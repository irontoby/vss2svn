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
            svn_error            => undef,
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
    $self->svn("help", undef, -2) or
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

    return $self->svn("$cmd $url $args", undef, $silent);
}

###############################################################################
#  svn
###############################################################################
sub svn {
    my($self, $cmd, $path, $silent) = @_;
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

    if (defined $path) {
        $cmd .= " -- \"$path\"";
        $disp_cmd .= " -- \"$path\"";
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

    if ($success) {
        $self->{svn_error} = undef;
    } else {
        $self->{svn_error} = "$disp_cmd\n$output";
    }

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


1;
