###############################################################################
#                        package Vss2Svn::VSS                                 #
###############################################################################

package Vss2Svn::VSS;

require 5.005_62;
use strict;
use warnings;

use base 'Vss2Svn';
use File::Path;
use File::Copy;
use Win32::TieRegistry (Delimiter => '/');
use Time::ParseDate;

use Cwd;
use Cwd 'chdir';

sub first(&@);

use Carp;
our $VERSION = '1.05';

our(%gCfg, %gErrMatch, %gHistLineMatch, @gDevPatterns);

###############################################################################
#   new
###############################################################################
sub new {
    my($class, $db, $project, $args) = @_;

    if (!defined $db) {
        croak "Must specify VSS database path";
    }

    $db =~ s/[\/\\]?(srcsafe.ini)?$//i;

    if (defined $project && $project ne ''
        && $project ne '$' && $project !~ /^\$\//) {
        croak "Project path must be absolute (begin with $/)";
    }

    $project = first {defined} $project, '$/';
    $args = first {defined} $args, {};

    my $self = bless
        {
            database             => $db,
            interactive          => 0,
            user                 => undef,
            passwd               => undef,
            silent               => undef,
            verbose              => undef,
            paginate             => 0,
            ss_output            => undef,
            ss_error             => undef,
            get_readonly         => 1,
            get_compare          => 1,
            get_eol_type         => 0,
            implicit_projects    => undef,
            use_tempfiles        => 0,
            timebias             => 0,
            _tempdir             => undef,
            _debug               => 0,
            _whoami              => undef,
            %$args,
        }, $class;

    # test to ensure 'ss' command is available
    $self->ss("WHOAMI", -2) or
        croak "Could not run VSS 'ss' command: ensure it is in your PATH";

    $self->{_whoami} = $self->{ss_output};
    $self->{_whoami} =~ s/\s*$//;
    $self->{_whoami} =~ s/^.*\n//;

    if ($self->{ss_output} =~ /changing project/im ||
         !$self->_check_ss_inifile) {
        croak "FATAL ERROR: You must not set the Force_Dir or Force_Prj VSS\n"
            . "variables when running SourceSync. These variables can be\n"
            . "cleared by unchecking the two \"Assume...\" boxes in SourceSafe\n"
            . "Explorer under Tools -> Options -> Command Line Options.\n ";
    }

    if ($project eq '') {
        $self->ss('PROJECT', -2);

        $project = $self->{ss_output};
        $project =~ s/^Current project is *//i;
        $project .= '/' unless $project =~ m/\/$/;

        $self->{project} = $project;
    } else {
        $self->set_project($project);
    }

    return $self;

}  #End new

###############################################################################
#  _check_ss_inifile
###############################################################################
sub _check_ss_inifile {
    my($self) = @_;

    my $user = lc($self->{_whoami});
    my $path = "$self->{database}/users/$user/ss.ini";

    open SSINI, $path or croak "Could not open user init file $path";
    my $success = 1;

LINE:
    while (<SSINI>) {
        if (m/Force_/i) {
            $success = 0;
            last LINE;
        }
    }

    close SSINI;
    return $success;

}  # End _check_ss_inifile

###############################################################################
#  set_project
###############################################################################
sub set_project {
    my($self, $project) = @_;

    $project .= '/' unless $project =~ m/\/$/;

    $self->ss("CP \"$project\"", -2) or
        croak "Could not set current project to $project:\n"
            . "  $self->{ss_output}\n ";

    $self->{project} = $project;

}  # End set_project

###############################################################################
#  project_tree
###############################################################################
sub project_tree {
    my($self, $project, $recursive, $remove_dev) = @_;

    # returns a nested-hash "tree" of all subprojects and files below the given
    # project; the "leaves" of regular files are the value "1".

    $project = $self->full_path($project);
    $recursive = 1 unless defined $recursive;
    $remove_dev = 0 unless defined $remove_dev;

    if ($self->filetype($project) ) { # projects are type 0
        carp "project_tree(): '$project' is not a valid project";
        return undef;
    }

    my $cmd = "DIR \"$project\"";
    $cmd .= ($recursive)? ' -R' : ' -R-';

    $self->ss($cmd, -2) or return undef;

    # It would be nice if Microsoft made it easy for scripts to pick useful
    # information out of the project 'DIR' listings, but unfortunately that's
    # not the case. It appears that project listings always follow blank
    # lines, and begin with the full project path with a colon appended.
    # Within a listing, subprojects come first and begin with a dollar sign,
    # then files are listed alphabetically. If there are no items in a project,
    # it prints out a message saying so. And at the end of it all, you get
    # a statement like "7 item(s)".

    my %tree = ();
    my $branch_ref = \%tree;

    my $seen_blank_line = 0;
    my($current_project);
    my $match_project = quotemeta($project);

LINE:
    foreach my $line (split "\n", $self->{ss_output}) {
        $line =~ s/\s+$//;

        if ($line eq '') {
            if ($seen_blank_line) {
                carp "project_tree(): an internal error has occured -- 1";
                return undef;
            }

            $seen_blank_line = 1;
            next LINE;
        }

        $seen_blank_line = 0;

        if ($line =~ m/^\d+\s+item\(s\)$/i) {
            # this is a count of # of items found; ignore
            next LINE;

        } elsif ($line =~ m/^No items found under/i) {
            # extraneous info
            next LINE;

        } elsif ($line =~ m/^(\$\/.*):$/) {
            # this is the beginning of a project's listing
            $current_project = $1;
            # make current project relative to initial
            $current_project =~ s/^$match_project\/?//i;
            $current_project =~ s/^\$\///; # take off initial $/ if still there

            $branch_ref = \%tree;

            if ($current_project ne '') {
                # get a reference to the end branch of subprojects
                ($branch_ref) = reverse(map {$branch_ref = $branch_ref->{$_}}
                                                split('/', $current_project));
            }

            if (!defined $branch_ref) {
                carp "project_tree(): an internal error has occured -- 2";
                return undef;
            }

            next LINE;
        } elsif ($line =~ m/^\$(.*)/) {
            # this is a subproject; create empty hash if not already there
            if (!defined $current_project) {
                carp "project_tree(): an internal error has occured -- 3";
                return undef;
            }

            $branch_ref->{$1} = {} unless defined($branch_ref->{$1});
        } else {
            # just a regular file
            if (!defined $current_project) {
                carp "project_tree(): an internal error has occured -- 4";
                return undef;
            }

            if ($remove_dev) {
                foreach my $pattern (@gDevPatterns) {
                    next LINE if $line =~ m/$pattern/i;
                }
            }

            $branch_ref->{$line} = 1;
        }

    }

    return \%tree;

}  # End project_tree

###############################################################################
#  file_history
###############################################################################
sub file_history {
    my($self, $file) = @_;
    # returns an array ref of hash refs from earliest to most recent;
    # each hash has the following items:
    #    version: version (revision) number
    #    user   : name of user who committed change
    #    date   : date in YYYYMMDD format
    #    time   : time in HH:MM (24h) format
    #    comment: checkin comment

    $file = $self->full_path($file);

    my $cmd = "HISTORY \"$file\"";
    my $tmpfile = '';

    $self->ss($cmd, -2) or return undef;

    my $hist = [];

    my $last = 0; # what type was the last line read?
                      # 0=start;1=version line;2=user/date/time;3="Checked In";
                      # 4=comment

    my $last_version = -1;

    my$rev = {}; # hash of info for the lastent revision
    my($year,$month,$day,$hour,$min,$ampm,$comment,$version);

HISTLINE:
    foreach my $line (split "\n", $self->{ss_output}) {
        if ($self->{_debug}) {
            warn "\nDEBUG:($last)<$line>\n";
        }

        if ($last == 0) {
            if ($line =~ m/$gHistLineMatch{version}/) {

                if ($last_version == 0 ||
                     (($last_version != -1) && ($1 != ($last_version - 1)))) {

                    # each version should be one less than the last
                    print "file_history(): internal consistency failure";
                    return undef;
                }

                $last = 1;
                $rev->{version} = $1;
            }

            next HISTLINE;
        } # if $last == 0

        if ($last == 1) {
            if ($line =~ m/$gHistLineMatch{userdttm}/) {
                $last = 2;
                $comment = '';

                if ($gCfg{dateFormat} == 1) {
                    # DD-MM-YY
                    ($rev->{user}, $day, $month, $year, $hour, $min, $ampm)
                        = ($1, $2, $3, $4, $5, $6, $7);
                } elsif ($gCfg{dateFormat} == 2) {
                    # YY-MM-DD
                    ($rev->{user}, $year, $month, $day, $hour, $min, $ampm)
                        = ($1, $2, $3, $4, $5, $6, $7);
                } else {
                    # MM-DD-YY
                    ($rev->{user}, $month, $day, $year, $hour, $min, $ampm)
                        = ($1, $2, $3, $4, $5, $6, $7);
                }

                $year = ($year > 79)? "19$year" : "20$year";

                if ($ampm =~ /p/i && $hour < 12) {
                    $hour += 12;
                } elsif ($ampm =~ /a/i && $hour == 12) {
                    $hour = 0;
                }

                if ($self->{timebias} != 0) {
                    my $basis = parsedate("$year/$month/$day $hour:$min");
                    (my $bias = $self->{timebias}) =~ s/^(\d+)/+ $1/;
                    my $epoch_secs = parsedate("$bias minutes",
                                               NOW => $basis);

                    (undef,$min,$hour,$day,$month,$year)
                        = localtime($epoch_secs);

                    $month += 1;
                    $year += 1900; #no, not a Y2K bug; $year = 100 in 2000
                }

                $rev->{date} = sprintf("%4.4i-%2.2i-%2.2i",
                                       $year, $month, $day);
                $rev->{time} = sprintf("%2.2i:%2.2i", $hour, $min);
            } elsif ($line =~ m/$gHistLineMatch{label}/) {
                # this is an inherited Label; ignore it

            } else {
                # user, date, and time should always come after header line
                print "file_history(): internal consistency failure";
                return undef;
            }

            next HISTLINE;
        } # if $last == 1

        if ($last == 2) {
            if ($line =~ s/$gHistLineMatch{comment}//) {
                $last = 4;
                $comment = $line;
            }

            next HISTLINE;
        }

        if ($last == 4) {
            if ($line =~ m/$gHistLineMatch{version}/) {
                $last = 1;
                $version = $1;

                $comment =~ s/\s+$//;
                $comment =~ s/^\s+//;
                $rev->{comment} = $comment;

                unshift @$hist, $rev;

                $rev = {};
                $rev->{version} = $version;
            } else {
                $comment .= "\n$line";
            }

        next HISTLINE;
        }
    }

    if ($last == 4) {
        $comment =~ s/\n/ /g;
        $comment =~ s/\s+$//;
        $comment =~ s/^\s+//;
        $rev->{comment} = $comment;
    } else {
        # last line of history should always be part of a comment, but
        # sometimes VSS doesn't include the final comment line
        $rev->{comment} = '(no comment)';
    }

    unshift @$hist, $rev;
    return $hist;
}

###############################################################################
#  filetype
###############################################################################
sub filetype {
    # -1: error
    #  0: project
    #  1: text
    #  2: binary

    my($self, $file) = @_;
    return -1 unless defined $file;

    #$file =~ s/\s//g;

    # special cases
    return 0 if $file eq '$/';
    return -1 if $file eq '$';

    # VSS has no decent way of determining whether an item is a project or
    # a file, so we do this in a somewhat roundabout way

    $file =~ s/[\/\\]$//;

    my $bare = $file;
    $bare =~ s/.*[\/\\]//;
    $bare = quotemeta($bare);

    $self->ss("PROPERTIES \"$file\" -R-", -3) or return -1;

    my $match_isproject = "^Project:.*$bare\\s*\$";
    my $match_notfound = "$bare\\s*is not an existing filename or project";

    if ($self->{ss_output} =~ m/$match_isproject/mi) {
        return 0;
    } elsif ($self->{ss_output} =~ m/$match_notfound/mi) {
        return -1;
     } else {
        $self->ss("FILETYPE \"$file\"", -3) or return -1;

        if ($self->{ss_output} =~ m/^$bare\s*Text/mi) {
            return 1;
        } else {
            return 2;
        }

    }

}  # End filetype

###############################################################################
#  full_path
###############################################################################
sub full_path {
    # returns the full VSS path to a given project file.

    my($self, $file) = @_;

    $file =~ s/^\s+//;
    $file =~ s/\s+$//;
    $file =~ s/\/$// unless $file eq '$/';

    return $file if $self->{implicit_projects};

    $file = "$self->{project}$file" unless $file =~ m/^\$/;
    $file =~ s/\/$// unless $file eq '$/'; # in case empty string was passed

    return $file;
}  # End full_path

###############################################################################
#  ss
###############################################################################
sub ss {
    my($self, $cmd, $silent) = @_;

    # SS command-line tool access.

    # silent values:
    #  0: print everything
    #  1: print program output only
    #  2: print err msgs only
    #  3: print nothing
    # -n: use 'n' only if 'silent' attribute not set

    if (defined($silent) && $silent < 0) {
        $silent = first {defined} $self->{silent}, $silent;
    } else {
        $silent = first {defined} $silent, $self->{silent}, 0;
    }

    $silent = abs($silent);

    $cmd =~ s/^\s+//;
    $cmd =~ s/\s+$//;

    (my $cmd_word = lc($cmd)) =~ s/^(ss(\.exe)?\s+)?(\S+).*/$3/i;

    $cmd = "ss $cmd" unless ($cmd =~ m/^ss(\.exe)?\s/i);

    if ($self->{interactive} =~ m/^y/i) {
        $cmd = "$cmd -I-Y";
    } elsif ($self->{interactive} =~ m/^n/i) {
        $cmd = "$cmd -I-N";
    } elsif (!$self->{interactive}) {
        $cmd = "$cmd -I-"
    }

    my $disp_cmd = $cmd;

    if (defined $self->{user} && $cmd !~ /\s-Y/i) {
        if (defined $self->{passwd}) {
            $disp_cmd = "$cmd -Y$self->{user},******";
            $cmd = "$cmd -Y$self->{user},$self->{passwd}";
        } else {
            $disp_cmd = $cmd = "$cmd -Y$self->{user}";
        }
    }

    my($rv, $output);

    warn "DEBUG: $disp_cmd\n\n" if $self->{_debug};

    $ENV{SSDIR} = $self->{database};

    if ($self->{use_tempfiles} &&
        $cmd_word =~ /^(dir|filetype|history|properties)$/) {
        my $tmpfile = "$self->{use_tempfiles}/${cmd_word}_cmd.txt";
        unlink $tmpfile;
        $cmd = "$cmd \"-O\&$tmpfile\"";
        system $cmd;

        if (open SS_OUTPUT, "$tmpfile") {
            local $/;
            $output = scalar <SS_OUTPUT>;
            close SS_OUTPUT;
            unlink $tmpfile;
        } else {
            warn "Can't open '$cmd_word' tempfile $tmpfile";
            undef $output;
        }

    } else {
        open SS_OUTPUT, '-|', "$cmd 2>&1";

        while (<SS_OUTPUT>) {
            $output .= $_;
        }

        close SS_OUTPUT;
        $output =~ s/\s+$// if defined $output;
    }

    if ($silent <= 1) {
        if ($self->{paginate}) {
            my $linecount = 1;

            foreach my $line (split "\n", $output) {
                print "$line\n";

                unless ($linecount++ % $self->{paginate}) {
                    print "Hit ENTER to continue...\r";
                    <STDIN>;

                    print "                        \r";

                }

            }

        } else {
            print "$output\n";
        }

    }

    my $ev = $? >> 8;

    # SourceSafe returns 1 to indicate warnings, such as no results returned
    # from a 'DIR'. We don't want to consider these an error.
    my $success = !($ev > 1);

    if ($success) {
        # This is interesting. If a command only partially fails (such as GET-ing
        # multiple files), that's apparently considered a success. So we have to
        # try to fix that.
        my $base_cmd = uc($cmd);
        $base_cmd =~ s/^(ss\s*)?(\w+).*/$2/i;

        my $err_match;

        if (defined($err_match = $gErrMatch{$base_cmd}) &&
                        $output =~ m/$err_match/m) {
            $success = 0;
        }

    }

    if ($success) {
        $self->{ss_error} = undef;
    } else {
        $self->{ss_error} = "$disp_cmd\n$output";
    }

    if (!$success && ($silent == 0 || $silent == 2)) {

        carp "\nERROR in Vss2Svn::VSS-\>ss\n"
            . "Command was: $disp_cmd\n   "
            . "(Error $ev) $output\n ";
        warn "\n";

    }

    $self->{ss_output} = $output;
    return $success;

}  # End ss

###############################################################################
#  _msg
###############################################################################
sub _msg {
    my $self = shift;
    print @_ unless $self->{silent};
}  # End _msg

###############################################################################
#  _vm  -- "verbose message"
###############################################################################
sub _vm {
    my $self = shift;
    print @_ if $self->{verbose};
}  # End _vm

###############################################################################
#  Initialize
###############################################################################
sub Initialize {
    my $dateFormat = $Registry->{'HKEY_CURRENT_USER/Control Panel/'
                             . 'International/iDate'} || 0;
    my $dateSep = $Registry->{'HKEY_CURRENT_USER/Control Panel/'
                             . 'International/sDate'} || '/';
    my $timeSep = $Registry->{'HKEY_CURRENT_USER/Control Panel/'
                             . 'International/sTime'} || ':';
    $gCfg{dateFormat} = $dateFormat;

    if ($dateFormat == 1) {
        $gCfg{dateString} = "DD${dateSep}MM${dateSep}YY";
    } elsif ($dateFormat == 2) {
        $gCfg{dateString} = "YY${dateSep}MM${dateSep}DD";
    } else {
        $gCfg{dateString} = "MM${dateSep}DD${dateSep}YY";
    }

    $gCfg{timeString} = "HH${timeSep}MM";

    # see ss method for explanation of this
    %gErrMatch = (
                    GET    => 'is not an existing filename or project',
                    CREATE => 'Cannot change project to',
                    CP     => 'Cannot change project to',
                 );

    %gHistLineMatch = (
        version    => qr/^\*+\s*Version\s+(\d+)\s*\*+\s*$/,
        userdttm   => qr/^User:\s+(.*?)\s+
                          Date:\s+(\d+)$dateSep(\d+)$dateSep(\d+)\s+
                          Time:\s+(\d+)$timeSep(\d+)([ap]*)\s*$/x,
        comment    => qr/^Comment:\s*/,
        label      => qr/^Label:/,
    );

    # patterns to match development files that project_tree will ignore
    @gDevPatterns = (
                        qr/\.vspscc$/,
                        qr/\.vssscc$/,
                        qr/^vssver\.scc$/,
                    );

}  # End Initialize

sub first(&@) {
	my $code = shift;
	&$code && return $_ for @_;
	return undef;
}

1;