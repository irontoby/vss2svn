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
use strict;

use Getopt::Long;
use Cwd;
use File::Path;

use DBD::SQLite;
use DBI;

our(%gCfg, $VSS, $SVN, $TREE, %USERS,);

# http://www.perl.com/tchrist/defop/defconfaq.html#What_is_the_proposed_operat
sub first(&@);

&Vss2Svn::Subversion::Initialize;
&Vss2Svn::VSS::Initialize;

sub PrintMsg; # defined later

warn "\n\n**** BUILDING INITIAL STRUCTURES; PLEASE WAIT... ****\n\n";

&Initialize;
&CreateDatabase;

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

$gCfg{dbh}->commit;

&SetupSvnProject;
&ImportSvnHistory;

&CloseDatabase;

close STDERR;
open STDERR, ">&THE_REAL_STDERR"; # yes, we're about to exit, but leaving
                                  # STDERR dangling always makes me nervous!
                                  
close THE_REAL_STDERR;
exit(0);


###############################################################################
#  GetProjectTree
###############################################################################
sub GetProjectTree {
    $TREE = $VSS->project_tree($gCfg{vssproject},1,1)
        or die "Couldn't create project tree for $gCfg{vssproject}";
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

    my($key, $val, $newproj);
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
        
        ($newproj = "$project/$subbranch->{project}") =~ s://:/:;

        &WalkTreeBranch($subbranch->{branch}, $newproj);

        chdir '..';
    }
}

###############################################################################
#  AddFileHistory
###############################################################################
sub AddFileHistory {
    my($project, $file) = @_;

    # build the revision history for this file

    (my $filepath = "$project/$file") =~ s://:/:;
    my $filehist = $VSS->file_history("$filepath");
    die if !defined $filehist;
    
    PrintMsg "   $filepath\n";

    foreach my $rev (@$filehist) {
        $gCfg{globalCount}++;
        
        $rev->{user} = lc( $rev->{user} );  # normalize usernames to lowercase
        $rev->{comment} .= "\n\n$gCfg{comment}" if defined $gCfg{comment};
        
        $rev->{date} =~ s/-//g;
        $rev->{time} =~ s/://;
        
        &InsertDatabaseRevision($filepath, $rev);
        
        $USERS{ $rev->{user} } = 1;
    }

}

###############################################################################
#  InsertDatabaseRevision
###############################################################################
sub InsertDatabaseRevision {
    my($filepath, $rev) = @_;
    
    my %data = %$rev; # don't pollute $rev
    
    #quote the text fields
    map { $data{$_} = $gCfg{dbh}->quote( $rev->{$_} ) }
        qw(date time user comment);

    $filepath = $gCfg{dbh}->quote($filepath);

    my $cmd = <<"EOSQL";
INSERT INTO
    history (
        date,
        time,
        file,
        version,
        user,
        comment,
        global_count
    )
VALUES (
    $data{date},
    $data{time},
    $filepath,
    $data{version},
    $data{user},
    $data{comment},
    $gCfg{globalCount}
)
EOSQL

    warn $cmd;

    $gCfg{dbh}->do($cmd) or die;

}  #End InsertDatabaseRevision

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
    # we will walk the history table in date/time order, GETting from VSS
    # as we go. VSS doesn't allow atomic multi-item commits, so we'll detect
    # these assuming if the user and comment are the same from one item to the
    # next, they were part of the "same" action.

    my($row, $upd, $commitinfo);
    
    my %prev = (user => '', comment => '', grain => 0);
    my %all = (); # hash of all files ever added
    my %thistime = (); # hash of files added on this commit
    
    my $multiple = 0;
    my $grain = 0.000001;

    PrintMsg "\n\n**** MIGRATING VSS HISTORY TO SUBVERSION ****\n\n";
    
    # date, time, and file fields are formatted to enable sorting numerically
    my $cmd = "SELECT * FROM history ORDER BY date, time, file";
    my $sth = $gCfg{dbh}->prepare($cmd) or die;
    $sth->execute or die;

ROW:
    while ($row = $sth->fetchrow_hashref) {
        $row->{date} =~ s/(....)(..)(..)/$1-$2-$3/;
        $row->{time} =~ s/(..)(..)/$1:$2/;
        $row->{comment} = ''
            if (!exists $row->{comment} || !defined $row->{comment});

        PrintMsg "   ($gCfg{commitNumber})File $row->{file}, "
            . "$row->{date} $row->{time}...\n";
        
        if (defined $prev{date} &&
            ($row->{date} eq $prev{date}) &&
            ($row->{user} eq $prev{user}) &&
            ($row->{comment} eq $prev{comment}) &&
            (!defined $thistime{ $row->{file} })) {
            
            # user and comment are same; this will be multi-item commit
            $multiple = 1;
            
        } elsif ($multiple) {
            # we're in a multi-item commit but user or comment changed;
            # commit previous action
            $multiple = 0;
            &CommitSvn(1, $prev{comment}, $commitinfo);
            undef $commitinfo;
            &SetSvnDates(\%prev) if $gCfg{setdates};
            %thistime = ();
            
        } elsif (defined $commitinfo) {
            # we're not in a multi-item commit and user or comment
            # changed; commit the single previous file
            $multiple = 0;
            
            &CommitSvn(0, $prev{comment}, $commitinfo);
            undef $commitinfo;
            &SetSvnDates(\%prev) if $gCfg{setdates};
            %thistime = ();
        }
        
        if (defined $prev{date} && ($row->{date} ne $prev{date})) {
            $grain = 0.000001;
            
            if (defined $commitinfo) {
                # done with this date, so commit what we have so far
                &CommitSvn($multiple, $prev{comment}, $commitinfo);
                undef $commitinfo;
    
                &SetSvnDates(\%prev) if $gCfg{setdates};
                %thistime = ();
    
                undef $commitinfo;
                $multiple = 0;
            }
        }
        
        $upd = $all{ $row->{file} }++;
        $commitinfo = &GetVssRevision($row, $upd, \%thistime,);
        
        %prev = (%$row, (grain => $grain));
        $grain += 0.000001;

    }
    
    if (defined $commitinfo) {
        &CommitSvn($multiple, $prev{comment}, $commitinfo);

        &SetSvnDates(\%prev) if $gCfg{setdates};
        %thistime = ();
    }

    $sth->finish;
    
}

###############################################################################
#  GetVssRevision
###############################################################################
sub GetVssRevision {
    my($row, $upd, $thisRef) = @_;
    # Gets a version of a file from VSS and adds it to SVN
    # $row is the row hash ref from the history SQLite table
    # $upd is true if this is an update rather than add
    
    my $vsspath = $row->{file};
    
    $row->{file} =~ m/^(.*\/)(.*)/ or die;
    my($path, $file) = ($1, $2);
    
    $path =~ s/$gCfg{vssprojmatch}// or die;
    $path =~ s/\/$//; # remove trailing slash
    
    (my $dospath = "$gCfg{workdir}/$path") =~ s/\//\\/g; # use backslashes
    $dospath =~ s/\\$//; # remove trailing backslash if $path was empty
    $dospath =~ s/\\\\/\\/g; # replace double backslashes with single
    
    my $cmd = "GET -GTM -W -GL\"$dospath\" -V$row->{version} \"$vsspath\"";
    $VSS->ss($cmd) or die;
    
    chdir $dospath or die;
        
    if (!$upd) {
        $SVN->svn("add \"$file\"") or die;
    }

    my $commitinfo =
        { file => $file,
          user => $row->{user},
          dospath => $dospath,};

    $thisRef->{ $row->{file} } = 1;

    return $commitinfo;    
}

###############################################################################
#  CommitSvn
###############################################################################
sub CommitSvn {
    my($multiple, $comment, $commitinfo) = @_;
    
    open COMMENTFILE, ">$gCfg{tmpfiledir}/comment.txt" or die;
    print COMMENTFILE $comment;
    close COMMENTFILE;
    
    $multiple? &CommitMultipleItems($commitinfo)
        : &CommitSingleItem($commitinfo);
        
    $gCfg{commitNumber}++;
    
}  #End CommitSvn

###############################################################################
#  CommitSingleItem
###############################################################################
sub CommitSingleItem {
    my($commitinfo) = @_;

    warn "SINGLE COMMIT\n";
    chdir $commitinfo->{dospath} or die;
    $SVN->{user} = $commitinfo->{user};
    $SVN->svn("commit --file \"$gCfg{tmpfiledir}/comment.txt\" "
              . "--non-recursive $commitinfo->{file}") or die;
}

###############################################################################
#  CommitMultipleItems
###############################################################################
sub CommitMultipleItems {
    my($commitinfo) = @_;

    warn "MULTIPLE COMMIT\n";
    chdir $gCfg{workdir} or die;
    $SVN->{user} = $commitinfo->{user};
    $SVN->svn("commit --file \"$gCfg{tmpfiledir}/comment.txt\" \".\"") or die;
}

###############################################################################
#  SetSvnDates
###############################################################################
sub SetSvnDates {
    my($info) = @_;
 
    my $grain = sprintf '%0.6f', $info->{grain};
    my $svn_date = "$info->{date}T$info->{time}:${grain}Z";

    my $cmd = "propset --revprop -rHEAD svn:date $svn_date $gCfg{svnrepo}";
    $SVN->svn($cmd) or die;

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
    
    $gCfg{vssproject} =~ s:/$:: unless $gCfg{vssproject} eq '$/';
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
    
    $gCfg{globalCount} = 1;
    $gCfg{commitNumber} = 1;

    $gCfg{workbase} = cwd() . "/_vss2svn";
    &RecursiveDelete( $gCfg{workbase} );
    mkdir $gCfg{workbase} or die "Couldn't create $gCfg{workbase}";

    $gCfg{workdir} = "$gCfg{workbase}/work";
    mkdir $gCfg{workdir} or die "Couldn't create $gCfg{workdir}";
    
    $gCfg{importdir} = "$gCfg{workbase}/import";
    mkdir $gCfg{importdir} or die "Couldn't create $gCfg{importdir}";
    
    $gCfg{tmpfiledir} = "$gCfg{workbase}/tmpfile";
    mkdir $gCfg{tmpfiledir} or die "Couldn't create $gCfg{tmpfiledir}";
    
    $gCfg{dbdir} = "$gCfg{workbase}/db";
    mkdir $gCfg{dbdir} or die "Couldn't create $gCfg{dbdir}";
    
}

###############################################################################
#  CreateDatabase
###############################################################################
sub CreateDatabase {
    $gCfg{dbh} = DBI->connect("dbi:SQLite(RaiseError=>1,AutoCommit=>0)"
                              . ":dbname=$gCfg{dbdir}/vss2svn.db","","");
    my $cmd;
    
    $cmd = <<"EOSQL";
CREATE TABLE history
(
    date    char(8)        NOT NULL,
    time    char(5)        NOT NULL,
    file    varchar(1024)  NOT NULL,
    version long           NOT NULL,
    user    varchar(256)   NOT NULL,
    comment blob           NOT NULL,
    global_count    long   NOT NULL
)
EOSQL

    $gCfg{dbh}->do($cmd) or die;
}  #End CreateDatabase

###############################################################################
#  CloseDatabase
###############################################################################
sub CloseDatabase {
    $gCfg{dbh}->commit;
    $gCfg{dbh}->disconnect;
}  #End CloseDatabase

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

    if (defined $project && $project ne ''
        && $project ne '$' && $project !~ /^\$\//) {
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

    $self->ss("CP \"$project\"", -2) or
        croak "Could not set current project to $project:\n"
            . "  $self->{last_ss_output}\n ";

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
    
    my $cmd = "HISTORY \"$file\"";
    my $tmpfile = '';

    if (defined $self->{use_tempfiles}) {
        $tmpfile = "$self->{use_tempfiles}/file_history.txt";
        unlink $tmpfile;
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

    # VSS has no decent way of determining whether an item is a project of
    # a file, so we do this in a somewhat roundabout way

    $file =~ s/[\/\\]$//;

    my $bare = $file;
    $bare =~ s/.*[\/\\]//;
    $bare = quotemeta($bare);

    $self->ss("PROPERTIES \"$file\" -R-", -3) or return -1;

    my $match_isproject = "^Project:.*$bare\\s*\$";
    my $match_notfound = "$bare\\s*is not an existing filename or project";

    if ($self->{last_ss_output} =~ m/$match_isproject/mi) {
        return 0;
    } elsif ($self->{last_ss_output} =~ m/$match_notfound/mi) {
        return -1;
     } else {
        $self->ss("FILETYPE \"$file\"", -3) or return -1;

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
                           version    => qr/^\*+\s*Version\s+(\d+)\s*\*+\s*$/,
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
