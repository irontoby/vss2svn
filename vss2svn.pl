#!perl

# sorry to embed HTML, I'm too lazy to keep two versions of this!
our $USAGE = <<'EOUSAGE';
    vss2svn.pl, written by Toby Johnson, toby@etjohnson.us
    This program is free software; it is released to the public domain.

    <b>usage: vss2svn.pl [options] --vssproject $/vss/project --svnrepo http://svn/repository/url</b>

      <b>--vssproject</b> : full path to VSS project you want to migrate
         <b>--svnrepo</b> : URL to target Subversion repository

    OPTIONAL PARAMETERS:
     <b>--comment=...</b> : optional text to add to end of every migrated comment
        <b>--setdates</b> : Sets the "svn:date" property off all commits to reflect the
                     original VSS commit date, so that the original commit dates
                     (and not today's date) show up in your new SVN logs. This is
                     not the default, since setting svn:date could lead to
                     problems if not done correctly. Using this also requires the
                     "pre-revprop-change" Hook Script to be set; see
                     <a href="http://svnbook.red-bean.com/svnbook/ch05s02.html#svn-ch-5-sect-2.1">http://svnbook.red-bean.com/svnbook/ch05s02.html#svn-ch-5-sect-2.1</a>
  <b>--login=user:pwd</b> : Set VSS username and password, separated by a colon.
                     <b>WARNING --</b> if the username/password combo you provide is
                     incorrect, this program will hang as SS prompts you for
                     a username! Even after I set the <b>ss.exe -I-</b> option, which
                     MICROS~1 claims means "to ensure that VSS never asks for user
                     input"!
        <b>--noprompt</b> : Don't prompt user to create usernames after the first stage
                     of the migration (see last paragraph below)

    The URL you provide for "svnrepo" will become the base URL for all migrated
    files, so for the usage example above, <b>$/vss/project/foo.c</b> would become
    <b>http://svn/repository/url/foo.c</b>. Plan your migration accordingly so that you
    end up with the structure that you want. The URL also cannot contain any
    existing files; but as long as the "parent" of the URL is a Subversion
    repository, any non-existent directories in the URL will be created.

    The <b>$SSDIR</b> environment variable must be set to the directory where your
    system srcsafe.ini file is located; see the VSS online help for more info.
    The "svn" and "ss" command-line executables must also be in your PATH.

    This script is released into the public domain. In case you're wondering
    about why the Vss2Svn packages have unused methods, it's because they came
    from in-house modules which had more functionality than just this conversion.

    I recommend converting only a small branch at first to see how things go.
    This process takes a very long time for large databases. I have made liberal

    Partway through the migration, you will be presented with a list of all
    usernames which performed any checkin operations in the given VSS project.
    If you want these user names to be preserved, you must add this list
    (including a user "vss_migration" for creating directories and such) to your
    Apache AuthUserFile with *blank passwords*. Apache must also *require* that
    usernames be passed, otherwise SVN will use anonymous access and you lose
    the usernames. So you need an "AuthType Basic" line or the like, as well as
    an AuthUserFile. See <a href="http://svnbook.red-bean.com/svnbook/ch06s04.html#svn-ch-6-sect-4.3">http://svnbook.red-bean.com/svnbook/ch06s04.html#svn-ch-6-sect-4.3</a>
    for more info.
EOUSAGE

use warnings;
use diagnostics;
use strict;

use Getopt::Long;
use Cwd;
use File::Path;

our(%gCfg, $VSS, $SVN, $TREE, %HIST, %USERS,);

# http://www.perl.com/tchrist/defop/defconfaq.html#What_is_the_proposed_operat
sub first(&@);

&Vss2Svn::Subversion::Initialize;
&Vss2Svn::VSS::Initialize;

&Initialize;

# redirect STDERR to logfile
open THE_REAL_STDERR, ">&STDERR";
$gCfg{logfile} = "$gCfg{workbase}/logfile.txt";
open STDERR, ">$gCfg{logfile}"
    or die "Couldn't open logfile $gCfg{workbase}/logfile.txt";

# the svn client program outputs to STDOUT; redirect to STDERR instead
open STDOUT, ">&STDERR";

select THE_REAL_STDERR;
$| = 1;
select STDOUT;

# since we redirected STDERR, make sure user sees die() messages!
$SIG{__DIE__} = \&MyDie;

&GetProjectTree;
&BuildHistory;
&GiveUserMessage unless $gCfg{noprompt};

&SetupSvnProject;
&ImportSvnHistory;

sub PrintMsg; # defined later

close STDERR;
open STDERR, ">&THE_REAL_STDERR"; # yes, we're about to exit, but leaving
                                  # STDERR dangling always makes me nervous!
                                  
close THE_REAL_STDERR;
exit(0);


###############################################################################
#  GetProjectTree
###############################################################################
sub GetProjectTree {
    $TREE = $VSS->project_tree($gCfg{vssproject},1,1);
}

###############################################################################
#  BuildHistory
###############################################################################
sub BuildHistory {
    chdir "$gCfg{importdir}" or die;
    
    PrintMsg "\n\n**** BUILDING VSS HISTORY ****\n\n";
    $VSS->{use_tempfiles} = "$gCfg{tmpfiledir}";
    
    &WalkTreeBranch($TREE, $gCfg{vssproject});
}

###############################################################################
#  WalkTreeBranch
###############################################################################
sub WalkTreeBranch {
    my($branch, $project) = @_;
    PrintMsg "ENTERING PROJECT $project...\n";

    my($key, $val);
    my @branches = ();

    foreach $key (sort keys %$branch) {
        $val = $branch->{$key};

        if (ref($val) eq 'HASH') {
            # subproject; create a new branch of the tree

            push @branches, {branch => $val, project => "$key"};

        } elsif (!ref $val) {
            # a scalar, i.e. regular file

            &AddFileHistory($project, $key);

        }
    }

    foreach my $subbranch (@branches) {
        mkdir $subbranch->{project};
        chdir $subbranch->{project} or die;

        &WalkTreeBranch($subbranch->{branch},
                        "$project/" . $subbranch->{project});

        chdir '..';
    }
}

###############################################################################
#  AddFileHistory
###############################################################################
sub AddFileHistory {
    my($project, $file) = @_;

    # build the revision history for this file; %HIST is keyed on date, then
    # time, then filename; then is an array of hashes (each hash containing
    # version number, user, and comment), in order of increasing version #s,
    # for that file on that date at that time; unless someone made multiple
    # commits in the same minute, this array should have only one value!

    my $filepath = "$project/$file";
    my $filehist = $VSS->file_history($filepath);
    die if !defined $filehist;
    
    my($ref, $user, $commentfile, $midpath);

    PrintMsg "   $filepath\n";

    foreach my $rev (@$filehist) {
        $HIST{ $rev->{date} } = {} unless defined $HIST{ $rev->{date} };
        $ref = $HIST{ $rev->{date} };

        $ref->{ $rev->{time} } = {} unless defined $ref->{ $rev->{time} };
        $ref = $ref->{ $rev->{time} };

        $ref->{ $filepath } = [] unless defined $ref->{ $filepath };
        $ref = $ref->{ $filepath };
        
        $user = lc( $rev->{user} );
        
        # $midpath simply allows us to spread comment tempfiles out
        $file =~ m/^(.)(.)/;  # load $1 and $2, and get your mind out of the gutter!
        $midpath = (defined $1 && defined $2)? lc("$1/$1$2/") : '';
        
        $commentfile = "$midpath$file-comment.$gCfg{commentCount}.txt";
        $gCfg{commentCount}++;
        
        $rev->{comment} .= "\n\n$gCfg{comment}" if defined $gCfg{comment};
        
        mkpath "$gCfg{tmpfiledir}/$midpath";
        open COMMENTFILE, ">$gCfg{tmpfiledir}/$commentfile"
            or die "Couldn't open $gCfg{tmpfiledir}/$commentfile";
        print COMMENTFILE $rev->{comment} or die;
        close COMMENTFILE;

        push @$ref, {version => $rev->{version}, user => $user,
                     commentfile => $commentfile };
        
        $USERS{$user} = 1;
    }

}

###############################################################################
#  GiveUserMessage
###############################################################################
sub GiveUserMessage {
   print THE_REAL_STDERR <<"EOTXT";

ATTENTION REQUIRED:
  Following is a list of all VSS users who have made updates at any time in the
  specified project. In order to preserve the user history during migration to
  Subversion, these users must exist in the Subversion authentication file.
  
  Usually, this is done with an Apache "Basic" HTTP authorization file, where
  each username is followed by a colon and the hashed password for that user.
  A blank password is permissible. Copy and paste the following lines into this
  authorization file in order to allow this user history to be migrated.
  
EOTXT

    print THE_REAL_STDERR join("\n", map {"$_:"} sort keys %USERS),
    "\n\nPRESS ENTER TO CONTINUE (or enter [q] to quit and start over)...";
       
    my $rep = <STDIN>;
   
    if ($rep =~ /^q/i) {
       print THE_REAL_STDERR "\n\nQuitting...\n";
       exit(0);
    }
}

###############################################################################
#  SetupSvnProject
###############################################################################
sub SetupSvnProject {
    PrintMsg "\n\n**** SETTING UP SUBVERSION DIRECTORIES ****\n\n";

    chdir $gCfg{importdir} or die;
    $SVN->do('import', '.', '--message "Initial Import"', 0) or die;
    
    chdir $gCfg{workdir} or die;
    $SVN->do('checkout', '', '"."') or die;
}

###############################################################################
#  ImportSvnHistory
###############################################################################
sub ImportSvnHistory {
    # we will walk the %HIST tree in order of date and time, GETting from VSS
    # as we go. VSS doesn't allow atomic multi-item commits, so we'll detect
    # these assuming if the user and comment are the same from one item to the
    # next, they were part of the "same" action.

    my($date, $time, $file, $ref, $version, $user, $comment,
       $upd, $commitinfo);
    
    my %prev = (user => '', comment => '');
    my %all = (); # hash of all files ever added
    my %thistime = (); # hash of files added on this commit
    
    my $multiple = 0;
    my $grain = 0;

    PrintMsg "\n\n**** MIGRATING VSS HISTORY TO SUBVERSION ****\n\n";

DATE:
    foreach $date (sort keys %HIST) {
        foreach $time (sort keys %{ $HIST{$date} }) {
            foreach $file (sort keys %{ $HIST{$date}{$time} }) {
                PrintMsg "   File $file, $date $time...\n";
                foreach $ref ( @{ $HIST{$date}{$time}{$file} } ) {
                    
                    $comment = &GetCommentFromFile($ref->{commentfile});
                
                    if (($ref->{user} eq $prev{user}) &&
                        ($comment eq $prev{comment}) &&
                        (!defined $thistime{$file})) {
                        
                        # user and comment are same; this will be multi-item commit
                        $multiple = 1;
                        
                    } elsif ($multiple) {
                        # we're in a multi-item commit but user or comment changed;
                        # commit previous action
                        $multiple = 0;
                        &CommitMultipleItems($commitinfo);
                        &SetSvnDates(\%thistime) if $gCfg{setdates};
                        %thistime = ();
                        
                    } elsif (defined $commitinfo) {
                        # we're not in a multi-item commit and user or comment
                        # changed; commit the single file
                        $multiple = 0;
                        
                        &CommitSingleItem($commitinfo);
                        &SetSvnDates(\%thistime) if $gCfg{setdates};
                        %thistime = ();
                    }
                    
                    $upd = $all{$file}++;
                    
                    $commitinfo = &GetVssRevision($ref, $date, $time, $file, $upd,
                        \%thistime, sprintf('%09.6f',$grain));
                    
                    $grain += 0.000001;
                    %prev = %$ref;
                    $prev{comment} = $comment;
                
                }
            }
        }
        
        # done with this date, so commit what we have so far
        if (defined $commitinfo) {
            $multiple? &CommitMultipleItems($commitinfo)
                : &CommitSingleItem($commitinfo);

            &SetSvnDates(\%thistime) if $gCfg{setdates};
            %thistime = ();
        }
        
        undef $commitinfo;
        $multiple = 0;
    }
    
}

###############################################################################
#  GetCommentFromFile
###############################################################################
sub GetCommentFromFile {
    my($file) = @_;
   
    open COMMENTFILE, "$gCfg{tmpfiledir}/$file"
        or die "Couldn't open $gCfg{tmpfiledir}/$file";
        
    my $comment = join('', <COMMENTFILE>);
    close COMMENTFILE;
    
    return $comment;
}

###############################################################################
#  GetVssRevision
###############################################################################
sub GetVssRevision {
    my($ref, $date, $time, $file, $upd, $thisRef, $grain) = @_;
    # Gets a version of a file from VSS and adds it to SVN
    # $ref is a reference to a HIST "leaf" hash of rev#, user, comment
    # $upd is true if this is an update rather than add
    
    my $path;
    my $vsspath = $file;
    
    $file =~ m/^(.*\/)(.*)/ or die;
    ($path, $file) = ($1, $2);
    
    $path =~ s/$gCfg{vssprojmatch}// or die;
    $path =~ s/\/$//; # remove trailing slash
    
    (my $dospath = "$gCfg{workdir}/$path") =~ s/\//\\/g; # use backslashes
    $dospath =~ s/\\$//; # remove trailing backslash if $path was empty
    $dospath =~ s/\\\\/\\/g; # replace double backslashes with single
    
    my $cmd = "GET -GTM -W -GL\"$dospath\" -V$ref->{version} $vsspath";
    $VSS->ss($cmd) or die;
    
    chdir $dospath or die;
        
    if (!$upd) {
        $SVN->svn("add \"$file\"") or die;
    }

    my $commitinfo =
        { cmd => "commit --file \"$gCfg{tmpfiledir}/$ref->{commentfile}\"",
          file => $file,
          user => $ref->{user},
          dospath => $dospath,};
        
    $thisRef->{$file} = { date => "${date}T${time}:${grain}Z",
                          dospath => $dospath };
                        

    return $commitinfo;    
}

###############################################################################
#  CommitSingleItem
###############################################################################
sub CommitSingleItem {
    my($commitinfo) = @_;

    warn "SINGLE COMMIT\n";
    chdir $commitinfo->{dospath} or die;
    $SVN->{user} = $commitinfo->{user};
    $SVN->svn("$commitinfo->{cmd} --non-recursive $commitinfo->{file}") or die;
}

###############################################################################
#  CommitMultipleItems
###############################################################################
sub CommitMultipleItems {
    my($commitinfo) = @_;

    warn "MULTIPLE COMMIT\n";
    chdir $gCfg{workdir} or die;
    $SVN->{user} = $commitinfo->{user};
    $SVN->svn("$commitinfo->{cmd} \".\"") or die;
}

###############################################################################
#  SetSvnDates
###############################################################################
sub SetSvnDates {
    my($thisRef) = @_;
    
    my $cmd;
   
    foreach my $file (sort keys %$thisRef) {
        $cmd = "propset --revprop -rHEAD svn:date $thisRef->{$file}->{date} "
            . "\"$thisRef->{$file}->{dospath}\\$file\"";
        $SVN->svn($cmd) or die;
    }

}  #End SetSvnDates

###############################################################################
#  RecursiveDelete
###############################################################################
sub RecursiveDelete {
    my($parent) = @_;
    my(@dirs, $dir);

    opendir(DIR, $parent);
    @dirs = readdir(DIR);
    closedir(DIR);

    foreach $dir (@dirs) {
       if ($dir ne '.' && $dir ne '..') {
          &RecursiveDelete("$parent/$dir");
       }
    }

    if (-d $parent) {
        rmdir($parent);
    }
    elsif (-f $parent) {
        unlink($parent);
    }

}

###############################################################################
#  PrintMsg
###############################################################################
sub PrintMsg {
    # print to logfile (redirected STDERR) and screen (STDOUT)
    warn @_;
    print THE_REAL_STDERR @_;
}  #End PrintMsg

###############################################################################
#  Die
###############################################################################
sub MyDie {
    # any die() is trapped by $SIG{__DIE__} to ensure user sees fatal errors
    warn @_;
    print THE_REAL_STDERR "\n", @_;
    
    (my $logfile = $gCfg{logfile}) =~ s:/:\\:g;
    print THE_REAL_STDERR <<"EOERR";

A fatal error has occured. See $logfile for more information.
EOERR
    exit(255);
}  #End Die

###############################################################################
#  Initialize
###############################################################################
sub Initialize {
    GetOptions(\%gCfg,'vssproject=s','svnrepo=s','comment=s','login=s',
               'setdates','noprompt','help');

    &GiveHelp(undef, 1) if defined $gCfg{help};
    
    defined $gCfg{vssproject} or GiveHelp("must specify --vssproject\n");
    defined $gCfg{svnrepo} or GiveHelp("must specify --svnrepo\n");
    defined $ENV{SSDIR} or GiveHelp("\$SSDIR not defined");
    
    $gCfg{vssproject} =~ s/\/$// unless $gCfg{vssproject} eq '$/';
    $gCfg{vssprojmatch} = quotemeta( $gCfg{vssproject} );
    
    $VSS = Vss2Svn::VSS->new($ENV{SSDIR}, $gCfg{vssproject});
    $VSS->{interactive} = 0;
    $VSS->{_debug} = 1;
    
    if (defined $gCfg{login}) {
        @{$VSS}{'user', 'passwd'} = split ':', $gCfg{login};
    }
    
    $SVN = Vss2Svn::Subversion->new( $gCfg{svnrepo} );
    $SVN->{interactive} = 0;
    $SVN->{user} = 'vss_migration';
    $SVN->{passwd} = ''; # all passwords are blank
    $SVN->{_debug} = 1;
    
    %USERS = ( vss_migration => 1, );
    
    $gCfg{commentCount} = 1;

    $gCfg{workbase} = cwd() . "/_vss2svn";
    &RecursiveDelete( $gCfg{workbase} );
    mkdir $gCfg{workbase} or die "Couldn't create $gCfg{workbase}";

    $gCfg{workdir} = "$gCfg{workbase}/work";
    mkdir $gCfg{workdir} or die "Couldn't create $gCfg{workdir}";
    
    $gCfg{importdir} = "$gCfg{workbase}/import";
    mkdir $gCfg{importdir} or die "Couldn't create $gCfg{importdir}";
    
    $gCfg{tmpfiledir} = "$gCfg{workbase}/tmpfile";
    mkdir $gCfg{tmpfiledir} or die "Couldn't create $gCfg{tmpfiledir}";
    
}

###############################################################################
#  GiveHelp
###############################################################################
sub GiveHelp {
    my($msg, $full) = @_;
    $msg .= "\n" if defined $msg;
    
    warn <<"EOHELP";
$msg
 usage: vss2svn.pl [options] --vssproject \$/vss/project --svnrepo http://svn/repository/url

   --vssproject : full path to VSS project you want to migrate
      --svnrepo : URL to target Subversion repository
         --help : see full help info

   USE --help TO VIEW ALL OPTIONAL PARAMETERS
EOHELP

    exit(0) unless $full;
    
    # de-html; kinda kludgy but gets the job done
    $USAGE =~ s:</?(a( href=".*?")?|b)>::g;
    warn $USAGE;
    exit(0);
    
}  #End GiveHelp



sub first(&@) {
	my $code = shift;
	&$code && return $_ for @_;
	return undef;
}




###############################################################################
#                           package Vss2Svn                                   #
###############################################################################

package Vss2Svn;

require 5.005_62;
use strict;
use warnings;

use File::Path;
use File::Copy;

use Carp;

our $VERSION = '1.00';

###############################################################################
#  set_user
###############################################################################
sub set_user {
    my($self, $user, $passwd) = @_;
    
    $self->{user} = $user;
    
    {
        no warnings 'uninitialized'; # we want to undef passwd if none passed
        $self->{passwd} = $passwd unless $passwd eq '';
    }
    
}  # End set_user


1;



###############################################################################
#                        package Vss2Svn::Subversion                          #
###############################################################################

package Vss2Svn::Subversion;

require 5.005_62;
use strict;
use warnings;

use base 'Vss2Svn';

sub first(&@) {
	my $code = shift;
	&$code && return $_ for @_;
	return undef;
}

use File::Path;
use File::Copy;

use Cwd;
use Cwd 'chdir';

use Carp;

our(%gInteractiveCmds);

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $svnrep, $project) = @_;

    if (!defined $svnrep) {
        croak "Must specify Subversion repository URL";
    }

    $project = '' if ! defined $project;

    my $self = bless
        {
            repository           => $svnrep,
            project              => $project,
            interactive          => 0,
            user                 => undef,
            passwd               => undef,
            silent               => undef,
            verbose              => undef,
            paginate             => 0,
            svn_output           => undef,
            get_readonly         => 1,
            get_compare          => 1,
            get_eol_type         => 0,
            implicit_projects    => undef,
            use_tempfiles        => 0,
            _tempdir             => undef,
            _debug               => 0,
            _whoami              => undef,
        }, $class;

    # test to ensure 'svn' command is available
    $self->svn("help", -2) or
        croak "Could not run Subversion 'svn' command: "
            . "ensure it is in your PATH";

    $self->set_project($project);

    return $self;

}

###############################################################################
#  set_project
###############################################################################
sub set_project {
    my($self, $project) = @_;

    $project =~ s/\/$//;
    $self->{project} = $project;

}  # End set_project

###############################################################################
#  do
###############################################################################
sub do {
    my($self, $cmd, $file, $args, $silent) = @_;
    
    # basically a wrapper for "svn" to set current project and repository
    
    my $url = "$self->{repository}/";
    $url .= $self->{project}
        if defined $self->{project} && $self->{project} ne '';
    
    $url .= $file if defined $file;
    $args = '' unless defined $args;
    
    return $self->svn("$cmd $url $args", $silent);
}

###############################################################################
#  svn
###############################################################################
sub svn {
    my($self, $cmd, $silent) = @_;
    # "raw" svn client access.

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

    $cmd =~ s/^\s*(svn)?\s*//;  #take off "svn" if present; we'll add back later
    $cmd =~ s/\s+$//;

    $cmd =~ m/^(\w+)/;

    my $disp_cmd = $cmd;

    if (defined $gInteractiveCmds{$1} && !$self->{interactive}) {
        $cmd = "$cmd --non-interactive";

        if (defined $self->{user} && $cmd !~ /--username/) {
            if (defined $self->{passwd} && $cmd !~ /--password/) {
                $disp_cmd = "$cmd --username \"$self->{user}\" --password *****";
                $cmd = "$cmd --username \"$self->{user}\" "
                    . "--password \"$self->{passwd}\"";
            } else {
                $disp_cmd = $cmd = "$cmd --username \"$self->{user}\"";
            }
        }
    
    }

    $cmd = "svn $cmd";

    my($rv, $output);

    warn "DEBUG: $disp_cmd\n\n" if $self->{_debug};

    open CMDOUT, '-|', "$cmd 2>&1";

    while (<CMDOUT>) {
        $output .= $_;
    }

    close CMDOUT;
    $output =~ s/\s+$// if defined $output;

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
    my $success = !$ev;

    if (!$success && ($silent == 0 || $silent == 2)) {

        carp "\nERROR in Vss2Svn::Subversion-\>ss\n"
            . "Command was: $disp_cmd\n   "
            . "(Error $ev) $output\n ";
        warn "\n";

    }

    $self->{svn_output} = $output;
    return $success;

}

###############################################################################
#  Initialize
###############################################################################
sub Initialize {
    
    # commands which allow --non-interactive
    %gInteractiveCmds = ( map {$_,1 } 
                                qw(blame cat checkout co commit ci copy cp delete del
                                remove rm diff di export import list ls log merge
                                mkdir move rename rn propdel pdel pd propedit pedit pe
                                propget pget pg proplist plist pl propset pset ps
                                status stat st switch sw update up))
}


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

use Cwd;
use Cwd 'chdir';

sub first(&@);

use Carp;
our $VERSION = '1.05';
our $MAKE = $ENV{MAKE} || undef;

our(%gErrMatch, %gHistLineMatch, @gDevPatterns);

###############################################################################
#   new
###############################################################################
sub new {
    my($class, $db, $project) = @_;

    if (!defined $db) {
        croak "Must specify VSS database path";
    }

    $db =~ s/[\/\\]?(srcsafe.ini)?$//i;

    if (defined $project && $project ne '' && $project !~ /^\$\//) {
        croak "Project path must be absolute (begin with $/)";
    }

    $project = first {defined} $project, '$/';

    my $self = bless
        {
            database             => $db,
            interactive          => 0,
            user                 => undef,
            passwd               => undef,
            silent               => undef,
            verbose              => undef,
            paginate             => 0,
            last_ss_output       => undef,
            get_readonly         => 1,
            get_compare          => 1,
            get_eol_type         => 0,
            implicit_projects    => undef,
            use_tempfiles        => 0,
            _tempdir             => undef,
            _debug               => 0,
            _whoami              => undef,
        }, $class;

    # test to ensure 'ss' command is available
    $self->ss("WHOAMI", -2) or
        croak "Could not run VSS 'ss' command: ensure it is in your PATH";

    $self->{_whoami} = $self->{last_ss_output};
    $self->{_whoami} =~ s/\s*$//;
    $self->{_whoami} =~ s/^.*\n//;

    if ($self->{last_ss_output} =~ /changing project/im ||
         !$self->_check_ss_inifile) {
        croak "FATAL ERROR: You must not set the Force_Dir or Force_Prj VSS\n"
            . "variables when running SourceSync. These variables can be\n"
            . "cleared by unchecking the two \"Assume...\" boxes in SourceSafe\n"
            . "Explorer under Tools -> Options -> Command Line Options.\n ";
    }

    if ($project eq '') {
        $self->ss('PROJECT', -2);

        $project = $self->{last_ss_output};
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

    $self->ss("CP $project", -2) or
        croak "Could not set current project to $project:\n"
            . "  $self->{last_ss_output}\n ";

    $self->{project} = $project;

}  # End set_project

###############################################################################
#  get_project
###############################################################################
sub get_project {
    my($self, $target, $project, $recursive, $postprocess) = @_;

    $project = $self->full_path($project);

    if ($self->filetype($project) ) { # projects are type 0
        carp "get_project(): '$project' is not a valid project";
        return 0;
    }

    my($force, $cmd);

    # Force_Dir causes SS to use working folder instead of current
    if (defined $target && $target ne '') {
        unless (-d $target) {
            carp "get_project(): target directory '$target' is invalid";
            return 0;
        }

        $force = 0; # disable Force_Dir
    } else {
        $force = 1; # enable Force_Dir
    }

    my $compare_method = (qw(-GCK -GCK -GCD -GCC))[$self->{get_compare}];
    my $eol_type = ('', qw(-GN -GRN))[$self->{get_eol_type}];

    $recursive = 1 unless defined $recursive;

    $cmd = "GET $project -GTM";
    $cmd .= ' -W' unless $self->{get_readonly};
    $cmd .= ($force)? ' -GF' : " -GF- -GL$target";
    $cmd .= ($recursive)? ' -R' : ' -R-';
    $cmd .= " $compare_method $eol_type";

    $self->ss($cmd, -2) or return 0;

    my $gettree = $self->_parse_get_project($project);

    if ($postprocess) {
        if ($^O =~ m/win/i) {
            $self->_postprocess_get_win_project($gettree, $target, $project);
        } else {
            $self->_postprocess_get_unix_project($gettree, $target, $project);
        }
    }

    return $gettree;

}  # End get_project

###############################################################################
#  _postprocess_get_win_project
###############################################################################
sub _postprocess_get_win_project {
    return 1;
}  # End _postprocess_get_win_project

###############################################################################
#  _postprocess_get_unix_project
###############################################################################
sub _postprocess_get_unix_project {
    my($self, $gettree, $target, $project) = @_;

    $self->_set_project_unix_exec($gettree, $target);

    return 1;
}  # End _postprocess_get_unix_project

###############################################################################
#  _set_project_unix_exec
###############################################################################
sub _set_project_unix_exec {
    my($self, $branch, $target) = @_;

    my($key, $val);

    while(($key, $val) = each %$branch) {
        if (ref $val) {
            $self->_set_project_unix_exec($val, "$target/$key");
        } elsif ($key !~ m/\.(pm|txt|ini|def|fmt|com|env)$/i &&
                    $val =~ m/^FILE(NOEXIST|CHANGED)$/) {
            print "chmod $target/$key\n";
            chmod 0554, "$target/$key" or $self->
                _msg("\n**WARNING: Could not change permissions "
                . "on file $target/$key\n");
        } elsif ($key =~ m/\.(txt|ini|env|com)$/i &&
                    $val =~ m/^FILE(NOEXIST|CHANGED)$/) {

            #Providing Read and Write permissions to the files
	 #with the above mentioned extensions - 08/19/03

            print "chmod $target/$key\n";
            chmod 0755, "$target/$key" or $self->
                _msg("\n**WARNING: Could not change permissions "
                . "on file $target/$key\n");
        }
    }

}  # End _set_project_unix_exec

###############################################################################
#  _parse_get_project
###############################################################################
sub _parse_get_project {
    my($self, $project) = @_;

    my $gettree = {};
    my $branch = {};

    my($curr);
    $project = quotemeta($project);

LINE:
    foreach my $line (split "\n", $self->{last_ss_output}) {

        if ($line =~ /^$project\/?(.*):/) {
            # start of a new project, so finish up the last one

            if (defined $curr) {
                $self->_add_proj_diff_branch($gettree, $curr, $branch);
                $branch = {};
            }

            $curr = $1;
            next LINE;

        }

        next LINE unless defined $curr;

        if ($line =~ m/Getting\s+(.*)/) {
            $branch->{$1} = 'FILENOEXIST';
        } elsif ($line =~ m/Replacing local copy of\s+(.*)/) {
            $branch->{$1} = 'FILECHANGED';
        }

    }

    if (defined $curr && defined $branch) {
        $self->_add_proj_diff_branch($gettree, $curr, $branch);
    }

    return $gettree;

}  # End _parse_get_project

###############################################################################
#  label
###############################################################################
sub label {
    my($self, $item, $label, $recursive) = @_;

    $item = $self->full_path($item);
    $recursive = 0 unless defined $recursive;

    my $interactive = $self->{interactive};

    if ($recursive && $self->filetype($item) == 0) {

        my @subprojects = $self->project_subprojects($item)
            or return 0;

        my $temp = '';
        my $shares = '';

        foreach my $subproject (map {"$item/$_"} @subprojects) {
            if (length($temp) + length($subproject) > 2000) {
                $shares .= "$temp:";
                $temp = '';
            }

            $temp .= "$subproject ";
        }

        $shares .= "$temp";
        $shares =~ s/:$//;

        unless ($shares eq '') {
            # interactive=Y option instructs to remove the old label if present
            $self->{interactive} = 'Y';

            foreach (split ':', $shares) {
                $self->ss("LABEL $_ \"-L$label\" -C-", -2);
            }
        }

    } else {

        $self->{interactive} = 'Y';
        $self->ss("LABEL $item \"-L$label\" -C-");

    }

    $self->{interactive} = $interactive;

    return 1;

}  # End label

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

    my $cmd = "DIR $project";
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
    foreach my $line (split "\n", $self->{last_ss_output}) {
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
                    next LINE if $line =~ m/$pattern/;
                }
            }

            $branch_ref->{$line} = 1;
        }

    }

    return \%tree;

}  # End project_tree

###############################################################################
#  project_files
###############################################################################
sub project_files {
    my($self, $project) = @_;

    $project = $self->full_path($project);

    my $tree = $self->project_tree($project, 0) or return undef;
    return grep { !ref $tree->{$_} } keys(%$tree); # return only the scalars

}  # End project_files

###############################################################################
#  project_subprojects
###############################################################################
sub project_subprojects {
    my($self, $project, $recursive) = @_;

    $project = $self->full_path($project);

    my $tree = $self->project_tree($project, $recursive) or return undef;

    if ($recursive) {
        return sort $self->_project_subprojects_recursive($tree);
    } else {
         # return only the hash refs
        return sort grep { ref $tree->{$_} } keys(%$tree);
    }

}  # End project_subprojects

###############################################################################
#  _project_subprojects_recursive
###############################################################################
sub _project_subprojects_recursive {
    my($self, $tree) = @_;

    my @subprojects = ();

    # if it's a reference, it's a subproject
    foreach my $subproject (grep { ref $tree->{$_} } keys(%$tree)) {
        push @subprojects, $subproject;
        push @subprojects, map {"$subproject/$_"}
            $self->_project_subprojects_recursive($tree->{$subproject});
    }

    return @subprojects;

}  # End _project_subprojects_recursive

###############################################################################
#  project_diff
###############################################################################
sub project_diff {
    my($self, $project, $dir) = @_;

    $project = $self->full_path($project);

    if ($self->filetype($project) ) { # projects are type 0
        carp "project_diff(): project '$project' is not a valid project (arg 1)";
        return undef;
    }

    if (! -d $dir) {
        carp "project_diff(): '$dir' is not a valid directory (arg 2)";
        return undef;
    }

    my $cwd = cwd();

    unless (chdir $dir) {
        carp "project_diff(): Could not change to directory '$dir' (arg 2)";
        return undef;
    }

    $self->ss("DIFF $project -R");

    my $difftree = $self->_parse_project_diffs($project);

    return $difftree;

}  # End project_diff

###############################################################################
#  _parse_project_diffs
###############################################################################
sub _parse_project_diffs {
    my($self, $project) = @_;

    my $difftree = {};
    my $branch = {};

    my($curr, $status, $item);
    $project = quotemeta($project);

LINE:
    foreach my $line (split "\n", $self->{last_ss_output}) {

        next LINE if $line =~ /^Against:/;

        if ($line =~ /^Diffing:\s+$project\/?(.*)/) {
            # start of a new project, so finish up the last one

            if (defined $curr) {
                $self->_add_proj_diff_branch($difftree, $curr, $branch);
                $branch = {};
            }

            $curr = $1; # this will be relative to starting project

            next LINE;

        } elsif ($line =~
                    /^Project\s+$project\/?(.*)\s+has no corresponding folder/) {
            # same situation, but we alread know this project is empty

            if (defined $curr) {
                $self->_add_proj_diff_branch($difftree, $curr, $branch);
                $branch = {};
            }

            (undef, $curr, $item) = ($1 =~ m/((.*)\/)?(.*)/);
            $curr = '' unless defined $curr;

            $branch->{$item} = 'DIRNOEXIST';
            $self->_add_proj_diff_branch($difftree, $curr, $branch);

            $branch = {};
            $curr = undef;

            next LINE;

        }

        next LINE unless defined $curr;

        if ($line =~ /^SourceSafe files not in the current folder:/) {

            $status = 'FILENOEXIST';
            next LINE;

        } elsif ($line =~ /^SourceSafe files different from local files:/) {

            $status = 'FILECHANGED';
            next LINE;

        } elsif ($line =~ /^Local files not in the current project:/) {
            # we don't really care about these

            $status = undef;
            next LINE;

        }

        next LINE unless defined $status;

        foreach $item (split ' ', $line) {
            $branch->{$item} = $status;
        }

    }

    if (defined $curr && defined $branch) {
        $self->_add_proj_diff_branch($difftree, $curr, $branch);
    }

    return $difftree;

}  # End _parse_project_diffs

###############################################################################
#  _add_proj_diff_branch
###############################################################################
sub _add_proj_diff_branch {
    my($self, $difftree, $path, $branch) = @_;

    my $pathref = $difftree;
    my @dirs = split '/', $path;

    foreach my $subdir (@dirs) {
        $pathref->{$subdir} = {} unless defined $pathref->{$subdir};
        $pathref = $pathref->{$subdir};
    }

    # We can't simply stuff $branch onto $pathref, since there may have already
    # been other directories defined at the same level. So, we have to iterate
    # over %$branch, adding each item.

    my($key, $value);

    while(($key, $value) = each %$branch) {
        $pathref->{$key} = $value;
    }

}  # End _add_proj_diff_branch

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

    if ($self->filetype($file) < 1) { # regular files are type 1 or 2
        carp "file_history(): '$file' is not a valid regular file";
        return undef;
    }
    
    my $cmd = "HISTORY $file";
    my $tmpfile;

    if (defined $self->{use_tempfiles}) {
        $tmpfile = "$self->{use_tempfiles}/file_history.txt";
        $cmd = "$cmd -O\@$tmpfile";
    }

    $self->ss($cmd, -2) or return undef;
    
    if (defined $self->{use_tempfiles}) {
        open SS_OUTPUT, "$tmpfile" or die "Can't open HISTORY tempfile $tmpfile";
        $self->{last_ss_output} = join('', <SS_OUTPUT>);
        close SS_OUTPUT;
    }

    my $hist = [];

    my $last = 0; # what type was the last line read?
                      # 0=start;1=version line;2=user/date/time;3="Checked In";
                      # 4=comment

    my $last_version = -1;

    my$rev = {}; # hash of info for the lastent revision
    my($year,$month,$day,$hour,$min,$ampm,$comment,$version);

HISTLINE:
    foreach my $line (split "\n", $self->{last_ss_output}) {
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

# TIMEFORMAT: modify the code below to pull dates and times out according to your locale
                ($rev->{user}, $month, $day, $year, $hour, $min, $ampm)
                    = ($1, $2, $3, $4, $5, $6, $7);

                $month = sprintf "%2.2i", $month;
                $day = sprintf "%2.2i", $day;
                $year = ($year > 79)? "19$year" : "20$year";

                $hour += 12 if $ampm =~ /p/i;
                $hour = sprintf "%2.2i", $hour;

                $rev->{date} = "$year-$month-$day";
                $rev->{time} = "$hour:$min";
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
#  build_repositories
###############################################################################
sub build_repositories {
    my($self, $root, $project) = @_;
    # $root is where the repositories will end up
    # $project is where the module files come from (and where replist.xml is)

    $project = $self->full_path($project);
    $root = $self->full_path($root);

    if ($self->filetype($root) ) { # projects are type 0
        carp "build_repositories(): root '$root' is not a valid project (arg 1)";
        return 0;
    }

    if (!$self->ss("VIEW $project/replist.xml -YSourceSync", -2)) {
        carp "build_repositories(): Could not read replist.xml "
            . "in project $project";
        return 0;
    }

    my $reptree = $self->_parse_replist_xml($project);

    if (!defined $reptree) {
        carp "build_repositories: Aborting due to errors in $root/replist.xml";
        return 0;
    }

    my $vsstree = $self->project_tree($project) or return undef;

    $self->_verify_reptree($reptree, $vsstree, $project) or return undef;
    $self->_create_repository_links($root, $reptree, 0) or return undef;

}  # End build_repositories

###############################################################################
#  _parse_replist_xml
###############################################################################
sub _parse_replist_xml {
    my($self, $project) = @_;

    my $xml = new XML::Parser(Style => 'Tree');

    # XML::Parser->parse calls die() on error
    my $data = eval { $xml->parse($self->{last_ss_output}) };

    if ($@) {
        carp "  Invalid XML data";
        return undef;
    }

    my($name, $attribs, $elems, $contents);
    my $tree = {};

    $name = $data->[0];
    ($attribs, $elems) = $self->_get_xml_data($data->[1]);

    if ($name ne 'RepositoryList') {
        carp "  replist.xml root element must be <RepositoryList>";
        return undef;
    }

TAG:
    while (($name, $contents) = splice(@$elems, 0, 2)) {
        if ($name eq '0') {
            if ($contents =~ /S/) {
                carp "  replist.xml: character data '$contents' not allowed in "
                    . "<RepositoryList> element";
                return undef;
            }

            next TAG;  # ignore whitespace

        } elsif ($name ne 'Repository') {
            carp "  replist.xml: unregognized element <$name> in <RepositoryList> "
                . "element";
            return undef;
        }

        $self->_add_rep_to_tree($contents, $tree, $project) || return undef;
    }

    return $tree;

}  # End _parse_replist_xml

###############################################################################
#  _add_rep_to_tree
###############################################################################
sub _add_rep_to_tree {
    my($self, $data, $tree, $project) = @_;

    my($name, $attribs, $elems, $contents, $repname);

    ($attribs, $elems) = $self->_get_xml_data($data);

    if (!defined($repname = delete $attribs->{'name'})) {
        carp "  replist.xml: <Repository> element has no 'name' attribute";
        return 0;
    }

    my $replabel;
    $replabel = (delete $attribs->{'label'}) if defined $attribs->{'label'};

    if (keys %$attribs) {
        carp "  replist.xml: <Repository> element allows only 'name' attributes";
        return 0;
    }

    if (defined $tree->{$repname}) {
        carp "  replist.xml: <Repository> '$repname' redefined";
        return 0;
    }

    $tree->{$repname} = {};

TAG:
    while (($name, $contents) = splice(@$elems, 0, 2)) {
        if ($name eq '0') {
            if ($contents =~ /S/) {
                carp "  replist.xml: character data '$contents' not allowed in "
                    . "<Repository> element";
                return undef;
            }

            next TAG;  # ignore whitespace

        } elsif ($name ne 'PerlModule') {
            carp "  replist.xml: unregognized element <$name> in <Repository> "
                . "element";
            return undef;
        }

        $self->_add_rep_tree_module($contents, $tree->{$repname},
                                             $project, $replabel)
            || return undef;
    }

    return 1;

}  # End _add_rep_to_tree

###############################################################################
#  _add_rep_tree_module
###############################################################################
sub _add_rep_tree_module {
    my($self, $data, $branch, $project, $replabel) = @_;

    my($name, $attribs, $elems, $contents, $modname);

    ($attribs, $elems) = $self->_get_xml_data($data);

    if (!defined($modname = delete $attribs->{'name'})) {
        carp "  replist.xml: <PerlModule> element has no 'name' attribute";
        return 0;
    }

    my $modlabel = first {defined} (delete $attribs->{'label'}), $replabel, '';

    if (keys %$attribs) {
        carp "  replist.xml: <PerlModule> ($modname) element allows only "
            . " 'name' and 'label' attributes";
        return 0;
    }

    if (defined $branch->{$modname}) {
        carp "  replist.xml: <PerlModule> ($modname) redefined";
        return 0;
    }

    $branch->{"$project/$modname"} = $modlabel;

    return 1;

}  # End _add_rep_tree_module

###############################################################################
#  _get_xml_data
###############################################################################
sub _get_xml_data {
    # returns two values from an XML::Parser->parse() array ref object:
    # 1. hash ref of attributes
    # 2. array ref of tag/contents pairs
    my($self, $xml_parser_obj) = @_;

    my $attribs = shift @{ $xml_parser_obj };
    my $contents = $xml_parser_obj;

    return($attribs, $contents);
}  # End _get_xml_data

###############################################################################
#  _verify_reptree
###############################################################################
sub _verify_reptree {
    my($self, $reptree, $vsstree, $project) = @_;

    my $project_match = quotemeta($project);
    $project_match = qr/^$project_match\//;

    my $success = 1;
    my($pathref);

REPOSITORY:
    foreach my $repository (sort keys %$reptree) {
MODULE:
        foreach my $module (sort keys %{ $reptree->{$repository} }) {
            $module =~ s/$project_match//;

            $pathref = $vsstree;

            foreach my $dir (split('/', $module)) {
                if (!defined $pathref->{$dir}) {
                    carp "  replist.xml: project $project/$module for repository "
                        . "$repository does not exist";

                    $success = 0;
                    next MODULE;
                }

                $pathref = $pathref->{$dir};
            }
        }
    }

    return $success;

}  # End _verify_reptree

###############################################################################
#  _create_repository_links
###############################################################################
sub _create_repository_links {
    # Given a hash ref from _parse_replist_xml, build the necessary links in
    # the VSS database

    my($self, $root, $reptree, $createnew) = @_;

    my($item, $value, $msg, $cmd);

    my %subdirs = ();

    # There is no way to avoid the inevitable here: since the SS utility only
    # allows us to share one project at a time, we must issue a separate
    # command for each. This, obviously, can take a very long time.

    if ($createnew) {
        $self->_msg("\nNow creating project $root...\n");

        if ($self->filetype($root) > -1) {
            # if root already exists, destroy it
            $self->_msg("   Removing old project...\n");

            $self->ss("PURGE $root", -3);
            $self->ss("DELETE $root", -2);
        }

        $self->ss("CREATE $root -S -C\"Automatically created by "
                    . "DBAS::SourceSync::VSS\"", -2)
            or $self->ss("CP $root") or return 0;
    } else {
        $self->ss("CP $root") or return 0;

    }

    foreach $item (sort keys %$reptree) {
        $value = $reptree->{$item};

        if (ref $value) {
            # this is a subdir
            $subdirs{$item} = $value;
        } else {
            # regular file; share it
            # $value is the version label we want

            $msg = $item;
            $msg .= " (label $value)" unless $value eq '';

            $self->_msg("   $msg\n");

            $cmd = "SHARE $item -G- -C-";
            $cmd .= " \"-VL$value\"" unless $value eq '';

            $self->ss($cmd, -2)
        }
    }

    foreach $item (sort keys %subdirs) {
        $self->_create_repository_links("$root/$item", $subdirs{$item}, 1);
    }

    return 1;

}  # End _create_repository_links

###############################################################################
#  get_repository
###############################################################################
sub get_repository {
    my($self, $perldist, $project, $workdir) = @_;

    if (!defined $perldist || ! -d $perldist) {
        carp "get_repository(): perldist '$perldist' not a valid dist target "
            . "directory (arg 1)";
        return 0;
    }

    unless ($^O =~ /win/i) {
        carp "get_repository(): Method only supported on Windows systems";
        return 0;
    }

    ($workdir = first {defined} @ENV{qw(TEMP TMP)}, '.')
        unless defined $workdir;

    my $time = time;
    my $tempdir = "$workdir/sourcesync.$time.tmp~~";
    $tempdir =~ s/[\\\/]$//;
    $tempdir =~ s/\\/\//g;

    foreach my $path ($tempdir, "$perldist/doc", "$perldist/x86") {
        unless (-d $path || mkpath $path) {
            carp "get_repository(): Could not create directory $path";
            return 0;
        }
    }

    my $eol_type = $self->{get_eol_type};
    $self->{get_eol_type} = 1; # set EOL type to Unix
    my $gettree = $self->get_project($tempdir, $project, 1, 0) || return 0;
    $self->{get_eol_type} = $eol_type;

    my $orig_dir = cwd();
    if (!chdir $tempdir) {
        carp "get_repository(): Could not change to working dir $tempdir";
        return 0;
    }

    my(@success, @fail, $ppm);
    my $cwd = cwd();
    $cwd =~ s:/:\\:g;

    $self->_vm("TEMPDIR $tempdir\n");

MODULE:
    foreach my $module (grep {m/Trustee/} sort keys %$gettree) {
        if (!chdir "$cwd\\$module") {
            carp "get_repository(): Could not change to module working dir "
                . "$cwd\\$module";
            next MODULE;
        }

        $self->_vm("MODULE $module\nWORKDIR $cwd\\$module\n");

        $ppm = My::PPM::Make->new(binary => 'x86',
                                                make   => $MAKE,
                                              );
        eval '$ppm->make_ppm';

        if ($@) {
            print "\n\n           **** WARNING: FAILED MODULE $module ****\n\n\n";
            push @fail, $module;
        } else {
            map {$self->_vm("COPYING $_ to $perldist/x86");
                  copy $_, "$perldist/x86"} glob '*.tar.gz';

            map {$self->_vm("COPYING $_ to $perldist");
                  copy $_, $perldist} glob '*.PPD';

            push @success, $module;
        }

    }

    print "\n";

    if (@success) {
        print "\nREPOSITORY MODULES SUCCESSFULLY BUILT:\n   ";
        print join("\n   ", @success);
    }

    if (@fail) {
        print "\nFAILURES OCCURRED WHILE BUILDING THESE MODULES:\n   ";
        print join("\n   ", @fail);
    }

    print "\n";

    chdir $orig_dir;
    rmtree $tempdir;

}  # End get_repository

###############################################################################
#  build_prod_mirror
###############################################################################
sub build_prod_mirror {
    # This will be a two-step process. We call _add_prod_branch from the root,
    # which recursively builds the mirror structure in a hash. Then call
    # _create_prod_links, which uses that hash to create the VSS share links

    # $root is where the production mirror will go
    # $project is the source
    my($self, $root, $project, $client, $platform) = @_;

    $project = $self->full_path($project);
    $root = $self->full_path($root);

    if ($self->filetype($root) ) { # projects are type 0
        carp "build_prod_mirror(): root '$root' is not a valid project (arg 1)";
        return undef;
    }

    if(!defined $client) {
        carp "build_prod_mirror(): client name must be specified (arg 3)";
        return undef;
    }

    my $tree = $self->project_tree($project, 1, 1) or return undef;

    $platform ||= '';

    $root = "$root/$client";
    $root .= "-$platform" if $platform;

    if ($self->filetype($root) > -1) {
        # if root already exists, destroy it
        $self->ss("PURGE $root", -3);
        $self->ss("DELETE $root", -2);
    }

    my %mirror;

    # Here we build the hash '%mirror' that will contain pairs of production
    # filenames and corresponding VSS items. Nested hashes indicate subdirs.
    # We start at the trunk of the tree and branch outwards; every time we
    # find a file in a generic directory, a platform-specific directory, or
    # a client-specific directory, we add those files in. Since files added
    # later will "overwrite" those added initially, this has the effect of
    # a "depth-first" search. This means the actual order of precedence is
    # client, platform, generic. Client names begin with "_"; platforms use "+".
    # The values in %mirror indicate the full VSS path to the actual item.
    # For example, with client "abc" on platform "bc" we might get:
    #
    #   %mirror = (
    #              menu.pl => '$/menu/menu.pl',       # regular file
    #              preproc => {                       # 'preproc' subdir
    #                          preproc.pl => '$/preproc/_abc/preproc.pl',
    #                          db.pl      => '$/preproc/_abc/+bc/db.pl',
    #                         },
    #                ... etc
    #             )
    # Client- and platform-specific subdirs can be nested as deep as makes
    # sense for any particular implementation.

    $self->_msg("\nNow determining filesystem structure...\n");
    $self->_add_prod_branch($tree, \%mirror, $project, $client, $platform, 1);

    $self->_create_prod_links($root, \%mirror);

    return $root;

}  # End build_prod_mirror

###############################################################################
#  _add_prod_branch
###############################################################################
sub _add_prod_branch {
    my($self, $branch, $mirr_ref, $project, $client, $platform, $recurse) = @_;
    # branch is any hash ref in a project_tree
    # mirr_ref is where we will store the info for this branch, see desc above
    # project is the VSS project in which we will look for items to share
    # "_client" directories take precedence over platform
    # "+platform" directories take precedence over generic
    # recurse should be true unless we're in a client- or platform- specific dir

    my($item, $value);

    my %client_specific_dirs = ();
    my %platform_specific_dirs = ();

    # take care of all the regular files and subdirs first, storing client-
    # and platform-specific stuff to add afterwards

BRANCH_ITEM:
    while(($item, $value) = each %$branch) {
        if (ref $value) {  # a subdir, but what kind?

            if ($platform && ($item =~ m/^\+$platform/)) {
                # platform-specific subdirectory, save for later
                $platform_specific_dirs{$item} = $value;

            } elsif ($item =~ m/^_$client/) {
                # client-specific subdirectory, save for later
                $client_specific_dirs{$item} = $value;

            } elsif ($item =~ m/^~/) {
                # a regular subdirectory, but its contents should go into
                # this directory and not a subdir

                if (!$recurse) {
                    carp "build_prod_mirror(): client- and platform-specific "
                        . "directories\ncannot have generic subdirectories\n"
                        . "  Project: $project\n"
                        . "   Subdir: $item\n";

                    next BRANCH_ITEM;
                }

                $self->_add_prod_branch($value,
                                                $mirr_ref,
                                                "$project/$item",
                                                $client,
                                                $platform,
                                                1);

            } elsif ($item !~ m/^[+_]/) {
                # just a regular subdirectory, recurse as usual unless we're
                # already in a special dir
                if (!$recurse) {
                    carp "build_prod_mirror(): client- and platform-specific "
                        . "directories\ncannot have generic subdirectories\n"
                        . "  Project: $project\n"
                        . "   Subdir: $item\n";

                    next BRANCH_ITEM;
                }

                $mirr_ref->{$item} = {};
                $self->_add_prod_branch($value,
                                                $mirr_ref->{$item},
                                                "$project/$item",
                                                $client,
                                                $platform,
                                                1);

            }

        } else {
            # $item is just a regular file. Add it into the mirror tree.
            $mirr_ref->{$item} = "$project/$item";
        }
    }

    # now take care of platform- and client-specific stuff, in that order
    while(($item, $value) = each %platform_specific_dirs) {
        $self->_add_prod_branch($value,
                                        $mirr_ref,
                                        "$project/$item",
                                        $client,
                                        $platform,
                                        1);
    }

    while(($item, $value) = each %client_specific_dirs) {
        $self->_add_prod_branch($value,
                                        $mirr_ref,
                                        "$project/$item",
                                        $client,
                                        $platform,
                                        1);
    }

}  # End _add_prod_branch

###############################################################################
#  _create_prod_links
###############################################################################
sub _create_prod_links {
    # Given a hash ref from _add_prod_branch, build the necessary links in
    # the VSS database

    my($self, $root, $mirr_ref) = @_;

    my($item, $value);

    my $shares = '';
    my $temp = '';
    my %subdirs = ();

    # Since many calls to SS can get expensive, and there is no way to specify
    # which project to share an item into (it's always the "current project"),
    # we first do all the items in one project and then move on to its subdirs.
    # If the list o' files gets too long, we stick in colons -- which are
    # illegal in VSS item names and therefore safe to use here -- to split on.

    $self->_msg("\nNow creating project $root...\n");
    $self->ss("CREATE $root -S -C\"Automatically created by "
                 . "DBAS::SourceSync::VSS\"", -2)
        or $self->ss("CP $root") or return 0;

    foreach $item (sort keys %$mirr_ref) {
        $value = $mirr_ref->{$item};

        if (ref $value) {
            # this is a subdir
            $subdirs{$item} = $value;
        } else {
            # regular file; share it
            if (length($temp) + length($value) > 2000) {
                $shares .= "$temp:";
                $temp = '';
            }

            $temp .= "$value ";

            $self->_msg("   $value\n");
        }
    }

    $shares .= "$temp";
    $shares =~ s/:$//;

    unless ($shares eq '') {
        foreach (split ':', $shares) {
            $self->ss("SHARE $_ -G-", -2)
        }
    }

    foreach $item (sort keys %subdirs) {
        $self->_create_prod_links("$root/$item", $subdirs{$item});
    }

    return 1;

}  # End _create_prod_links

###############################################################################
#  build_perl_tree
###############################################################################
sub build_perl_tree {
    # Generally uses the same strategy as build_prod_mirror; see comments there
    my($self, $root, $project) = @_;

    $project = $self->full_path($project);
    $root = $self->full_path($root);

    if ($self->filetype($root) > 0 ) { # >1 means regular file
        carp "build_perl_tree(): root '$root' is not a valid project (arg 1)";
        return undef;
    }

    my $tree = $self->project_tree($project) or return undef;

    if ($self->filetype($root) > -1) {
        # if project already exists, destroy it
        $self->ss("PURGE $root", -3);
        $self->ss("DELETE $root", -2);
    }

    my %mirror;

    $self->_msg("\nNow determining Perl tree structure...\n");
    $self->_add_perl_branch($tree, \%mirror, $project);

    return $self->_create_perltree_links($root, \%mirror);

}  # End build_perl_tree

###############################################################################
#  _add_perl_branch
###############################################################################
sub _add_perl_branch {
    my($self, $branch, $mirr_ref, $project) = @_;

    my($item, $value);

BRANCH_ITEM:
    while(($item, $value) = each %$branch) {
        if (ref $value) {
            # A subdirectory; if it has dashes in it, turn it into a nested subdir
            # structure.

            my $subdir_mirr_ref = $mirr_ref;

            if ($item =~ /-/) {
                my @dirs = split('-', $item);
                pop @dirs;  # remove last entry; i.e. 'VSS' above doesn't become a dir

                foreach my $dir (@dirs) {
                    $subdir_mirr_ref->{$dir} = {}
                        unless defined $subdir_mirr_ref->{$dir};
                    $subdir_mirr_ref = $subdir_mirr_ref->{$dir};
                }

            } else {
                $subdir_mirr_ref = $mirr_ref->{$item};
            }

            $self->_add_perl_branch($value,
                                          $subdir_mirr_ref,
                                          "$project/$item");
        } elsif ($item =~ m/\.pm$/i) {
            # a Perl module; add it into the mirror tree.
            $mirr_ref->{$item} = "$project/$item";
        }

    }

}  # End _add_perl_branch

###############################################################################
#  _create_perltree_links
###############################################################################
sub _create_perltree_links {
    # Given a hash ref, build the necessary links in the VSS database
    my($self, $root, $mirr_ref) = @_;

    my($item, $value, $dirname);

    my $shares = '';
    my $temp = '';
    my %subdirs = ();

    $self->_msg("\nNow creating Perl tree project $root...\n");
    $self->ss("CREATE $root -S -C\"Automatically created by "
                 . "DBAS::SourceSync::VSS\"", -2)
        or $self->ss("CP $root") or return 0;

    while (($item, $value) = each %$mirr_ref) {
        if (ref $value) {
            # this is a subproject; save it for later
            $subdirs{$item} = $value;
        } else {
            # regular file; share it
            if (length($temp) + length($value) > 2000) {
                $shares .= "$temp:";
                $temp = '';
            }

            $temp .= "$value ";

            $self->_msg("   $value\n");
        }

    }

    $shares .= "$temp";
    $shares =~ s/:$//;

    unless ($shares eq '') {
        foreach (split ':', $shares) {
            $self->ss("SHARE $_ -G-", -2)
        }
    }

    foreach $item (sort keys %subdirs) {
        $self->_create_perltree_links("$root/$item", $subdirs{$item});
    }

    return 1;

}  # End _create_perltree_links

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

    $file =~ s/\s//g;

    # special cases
    return 0 if $file eq '$/';
    return -1 if $file eq '$';

    # VSS has no decent way of determining whether an item is a project of
    # a file, so we do this in a somewhat roundabout way

    $file =~ s/[\/\\]$//;

    my $bare = $file;
    $bare =~ s/.*[\/\\]//;
    $bare = quotemeta($bare);

    $self->ss("PROPERTIES $file -R-", -3) or return -1;

    my $match_isproject = "^Project:.*$bare\\s*\$";
    my $match_notfound = "$bare\\s*is not an existing filename or project";

    if ($self->{last_ss_output} =~ m/$match_isproject/mi) {
        return 0;
    } elsif ($self->{last_ss_output} =~ m/$match_notfound/mi) {
        return -1;
     } else {
        $self->ss("FILETYPE $file", -3) or return -1;

        if ($self->{last_ss_output} =~ m/^$bare\s*Text/mi) {
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
#  prepare_tempdir
###############################################################################
sub prepare_tempdir {
    my($self, $target, $project) = @_;

    $project = '' unless defined $project;
    $project = $self->full_path($project);

    my $project_path = $project;
    $project_path =~ s/^\$\///;

    $target = '.' unless defined $target;
    $self->{_tempdir} = "$target/.$$.SourceSync.tmp"
        unless defined $self->{_tempdir};

    my $make_target = "$self->{_tempdir}/$project_path";
    mkpath($make_target);

    $self->get_project($make_target, $project) or return 0;

}  # End prepare_tempdir

###############################################################################
#  release_tempdir
###############################################################################
sub release_tempdir {
    my($self) = @_;

    rmtree($self->{_tempdir}, 0, 0)
        or print "release_tempdir(): Could not remove directory $self->{_tempdir}";

}  # End release_tempdir

###############################################################################
#  _get_tempfile
###############################################################################
sub _get_tempfile {
    my($self, $target, $vss_file) = @_;

    $vss_file =~ s/^\$\///; # remove initial $/

    my $target_file = $vss_file;
    $target_file =~ s/.*[\/\\]//;

    my $copy_source = "$self->{_tempdir}/$vss_file";
    my $copy_target = "$target/$target_file";

    return copy($copy_source, $copy_target);

}  # End _get_tempfile

###############################################################################
#  is_valid_vss_name
###############################################################################
sub is_valid_vss_name {
    my($self, $name) = @_;

    return !($name =~ m/[$<>@\[\]{}():;=^|*!\/\\%?,'"\t]/);

}  # End is_valid_vss_name

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

    $cmd = "ss $cmd" unless ($cmd =~ m/^ss\s/i);

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
    open SSOUT, '-|', "$cmd 2>&1";

    while (<SSOUT>) {
        $output .= $_;
    }

    close SSOUT;
    $output =~ s/\s+$// if defined $output;

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

    if (!$success && ($silent == 0 || $silent == 2)) {

        carp "\nERROR in Vss2Svn::VSS-\>ss\n"
            . "Command was: $disp_cmd\n   "
            . "(Error $ev) $output\n ";
        warn "\n";

    }

    $self->{last_ss_output} = $output;
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
    # see ss method for explanation of this
    %gErrMatch = (
                    GET    => 'is not an existing filename or project',
                    CREATE => 'Cannot change project to',
                    CP     => 'Cannot change project to',
                 );

# TIMEFORMAT: modify "userdttm" below if necessary for your format.
    %gHistLineMatch = (
                           version    => qr/^\*+\s*Version\s+(\d)+\s*\*+\s*$/,
                           userdttm   => qr/^User:\s+([\S]+)\s+Date:\s+(\d+)\/(\d+)
                                         \/(\d+)\s+Time:\s+(\d+):(\d+)([ap]*)\s*$/x,
                           comment    => qr/^Comment:\s*/,
                           label      => qr/^Label:/,
                      );

    # patterns to match development files that project_tree will ignore
#    @gDevPatterns = (
#                        qr/\.perlproj$/,
#                        qr/\.vspscc$/,
#                        qr/\.vssscc$/,
#                        qr/\.sln$/,
#                    );

}  # End Initialize

sub first(&@) {
	my $code = shift;
	&$code && return $_ for @_;
	return undef;
}


1;
