#!perl

# vss2svn.pl, Copyright (C) 2004 by Toby Johnson.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# http://www.gnu.org/copyleft/gpl.html

BEGIN {
    # ensure script can find lib directory, regardless where it's run from
    my $path = $0;
    $path =~ s:^(.*)[/\\].*:$1: or $path = '.';
    eval "use lib '$path/lib'";

    if ($@) {
        die "Could not load Vss2Svn libraries: $@";
    }
}

use warnings;
use strict;

use Getopt::Long;
use Cwd;
use File::Path;
use Text::Wrap;
use Pod::Usage;

use Vss2Svn::Subversion;
use Vss2Svn::VSS;

use DBD::SQLite;
use DBI;

use Win32::TieRegistry (Delimiter => '/');

our(%gCfg, $VSS, $SVN, $TREE, %USERS,);

eval "use Encode";
$gCfg{allowUtf8} = !$@;

# http://www.perl.com/tchrist/defop/defconfaq.html#What_is_the_proposed_operat
sub first(&@);
sub PrintMsg; # defined later

&Vss2Svn::Subversion::Initialize;
&Vss2Svn::VSS::Initialize;

&Regionalize;
&Initialize;
&GiveStartupMessage;
&SetupLogfile;

&CreateDatabase;

&GetProjectTree;
&PruneVssExcludes;
&BuildHistory;
&GiveHttpdAuthMessage unless $gCfg{noprompt};

$gCfg{dbh}->commit;

&SetupSvnProject;
&ImportSvnHistory;

&CloseDatabase;
PrintMsg "\n\n**** VSS MIGRATION COMPLETED SUCCESSFULLY!! ****\n";

close STDERR;
open STDERR, ">&THE_REAL_STDERR"; # yes, we're about to exit, but leaving
                                  # STDERR dangling always makes me nervous!

$gCfg{hooray} = 1; # to suppress Win32::TieRegistry global destruction errors
exit(0);


###############################################################################
#  GiveStartupMessage
###############################################################################
sub GiveStartupMessage {

    my $setdates;
    my $datemsg = '';

    if ($gCfg{setdates}) {
        $setdates = 'yes';
        $datemsg = <<"EOMSG";


WARNING: Commit dates can be migrated to a NEW SUBVERSION REPOSITORY only.
You WILL CORRUPT your data if you migrate dates to an existing repository
which is at any other Revision than 0!
EOMSG
    } else {
        $setdates = 'no';
    }

    print <<"EOMSG";

         ss.exe Found: $gCfg{ssbin}
        svn.exe Found: $gCfg{svnbin}

          VSS Project: $gCfg{vssproject}
       Subversion URL: $gCfg{svnrepo}

    Local Date Format: $Vss2Svn::VSS::gCfg{dateString}
    Local Time Format: $Vss2Svn::VSS::gCfg{timeString}
 Time Bias To Get GMT: $gCfg{timebias} minutes

 Set SVN Commit Dates: $setdates$datemsg
EOMSG

    return if $gCfg{noprompt};

    print "Continue with these settings? [Y/n]";
    my $reply = <STDIN>;
    exit(1) if ($reply =~ m/\S/ && $reply !~ m/^y/i);
}

###############################################################################
#  SetupLogfile
###############################################################################
sub SetupLogfile {
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
    $SIG{__WARN__} = \&PrintMsg if $gCfg{debug};
}

###############################################################################
#  GetProjectTree
###############################################################################
sub GetProjectTree {
    PrintMsg "\n\n**** BUILDING INITIAL STRUCTURES; PLEASE WAIT... ****\n\n";

    &SetStatus(0,"Building initial structures");

    $TREE = $VSS->project_tree($gCfg{vssproject},1,1,1)
        or die "Couldn't create project tree for $gCfg{vssproject}";
}

###############################################################################
#  PruneVssExcludes
###############################################################################
sub PruneVssExcludes {

    return unless defined $gCfg{vssexclude};

    # By this point, we already have the entire "naked" directory structure
    # in $TREE, and we prune off any branches that match exclude. It may seem
    # wasteful to go to the trouble of building $TREE if we're just gonna
    # cut large chunks off now, but since we had to parse the entire output of
    # "ss DIR" on "vssproject" anyway, we wouldn't have saved much time by
    # using these at that stage.

    my($ref, $parent, $subdir, $last);

EXCLUDE:
    foreach my $exclude ( sort @{ $gCfg{vssexclude} }) {
        # by sorting, we get parents before their subdirectories, to give more
        # meaningful warning messages

        $exclude =~ s/^\s*(.*?)\s*$/$1/;
        $exclude =~ s:^$gCfg{vssprojmatch}/?::;

        if ($exclude =~ m:^\$/:) {
            PrintMsg "**WARNING: Exclude path \"$exclude\" is not underneath "
                . "$gCfg{vssproject}; ignoring...\n";
            next EXCLUDE;
        }

        # Perl doesn't allow us to delete() a hash ref, so we must also keep
        # track of the parent to fully get rid of the entry
        $ref = $parent = $TREE;

        foreach $subdir (split '\/', $exclude) {
            if (!exists $ref->{$subdir}) {
                PrintMsg "**WARNING: Exclude path \"$exclude\" not found in "
                    . "$gCfg{vssproject} (or a parent directory was already "
                    . "excluded); ignoring...\n";
                next EXCLUDE;
            }

            # can't use foreach() iterator outside of loop, so keep track of it
            $last = $subdir;
            $parent = $ref;
            $ref = $ref->{$subdir};
        }

        delete $parent->{$last};
        1;

    }

}  #  End PruneVssExcludes

###############################################################################
#  BuildHistory
###############################################################################
sub BuildHistory {
    chdir "$gCfg{importdir}"
        or die "Couldn't create working directory $gCfg{importdir}";

    PrintMsg "\n\n**** BUILDING VSS HISTORY ****\n\n";

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
        chdir $subbranch->{project}
            or die "Could not change to working directory $subbranch->{project}";

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

    # SS.exe uses a semicolon to indicate a "pinned" file
    $filepath =~ s/;(.*)//;

    my $filehist = $VSS->file_history("$filepath");
    die "Internal error while reading VSS file history for $filepath"
        if !defined $filehist;

    PrintMsg "   $filepath\n";

REV:
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
        qw(user comment);

    $filepath = $gCfg{dbh}->quote($filepath);

    my $cmd = <<"EOSQL";
INSERT INTO
    vss2svn_history (
        date,
        time,
        file,
        version,
        user,
        comment,
        imported,
        global_count
    )
VALUES (
    $data{date},
    $data{time},
    $filepath,
    $data{version},
    $data{user},
    $data{comment},
    0,
    $gCfg{globalCount}
)
EOSQL

    warn $cmd;

    $gCfg{dbh}->do($cmd)
        or die "Could not execute DBD::SQLite command";

}  #End InsertDatabaseRevision

###############################################################################
#  GiveHttpdAuthMessage
###############################################################################
sub GiveHttpdAuthMessage {
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

    &SetStatus(1,"Setting up Subversion directories");

    chdir $gCfg{importdir}
        or die "Could not change to directory $gCfg{importdir}";

    PrintMsg "   Importing directory structure from Subversion...\n";
    $SVN->do('import', '.', '--message "Initial Import"', 0)
        or die "Could not perform SVN import of $gCfg{importdir}. Have you "
        . "set your httpd authorization file correctly?";

    chdir $gCfg{workdir}
        or die "Could not change to directory $gCfg{workdir}";

    PrintMsg "   Checking out working copy...\n";
    $SVN->do('checkout', '', '"."')
        or die "Could not perform SVN checkout of $gCfg{importdir}";
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

    &SetStatus(2,"Migrating VSS history to Subversion");

    # date, time, and file fields are formatted to enable sorting numerically
    my $cmd = "SELECT * FROM vss2svn_history WHERE imported = 0 "
        . "ORDER BY date, time, file";

    my $sth = $gCfg{dbh}->prepare($cmd)
        or die "Could not prepare DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";

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

    $row->{file} =~ m/^(.*\/)(.*)/
        or die "Mangled VSS file path information", join("\n", %$row);
    my($path, $file) = ($1, $2);

    $path =~ s/$gCfg{vssprojmatch}//
        or die "Mangled VSS file path information", join("\n", %$row);
    $path =~ s/\/$//; # remove trailing slash

    (my $dospath = "$gCfg{workdir}/$path") =~ s/\//\\/g; # use backslashes
    $dospath =~ s/\\$//; # remove trailing backslash if $path was empty
    $dospath =~ s/\\\\/\\/g; # replace double backslashes with single

    my $cmd = "GET -GTM -W -GL\"$dospath\" -V$row->{version} \"$vsspath\"";
    $VSS->ss($cmd)
        or die "Could not issue ss.exe command";

    chdir $dospath
        or die "Could not switch to directory $dospath";

    if (!$upd) {
        $SVN->svn("add", $file)
            or die "Could not perform SVN add of $file";
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

    $comment = Encode::encode('utf8', $comment) if $gCfg{utf8};

    open COMMENTFILE, ">$gCfg{tmpfiledir}/comment.txt"
        or die "Could not open $gCfg{tmpfiledir}/comment.txt for writing";
    print COMMENTFILE $comment;
    close COMMENTFILE;

    PrintMsg "   (COMMITTING SVN...)\n";

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
    chdir $commitinfo->{dospath}
        or die "Could not change to directory $commitinfo->{dospath}";

    my $enc = $gCfg{utf8}? ' --encoding UTF-8' : '';

    $SVN->{user} = $commitinfo->{user};
    $SVN->svn("commit$enc --file \"$gCfg{tmpfiledir}/comment.txt\" "
              . "--non-recursive", $commitinfo->{file})
        or die "Could not perform SVN commit on \"$commitinfo->{file}\". "
        . "Have you set your httpd authorization file correctly?";
}

###############################################################################
#  CommitMultipleItems
###############################################################################
sub CommitMultipleItems {
    my($commitinfo) = @_;

    warn "MULTIPLE COMMIT\n";
    chdir $gCfg{workdir}
        or die "Could not change to directory $gCfg{workdir}";

    my $enc = $gCfg{utf8}? ' --encoding UTF-8' : '';

    $SVN->{user} = $commitinfo->{user};
    $SVN->svn("commit$enc --file \"$gCfg{tmpfiledir}/comment.txt\" \".\"")
        or die "Could not perform SVN commit. "
        . "Have you set your httpd authorization file correctly?";
}

###############################################################################
#  SetSvnDates
###############################################################################
sub SetSvnDates {
    my($info) = @_;

    my $grain = sprintf '%0.6f', $info->{grain};
    my $svn_date = "$info->{date}T$info->{time}:${grain}Z";

    my $cmd = "propset --revprop -rHEAD svn:date $svn_date $gCfg{svnrepo}";
    $SVN->svn($cmd)
        or die "Could not perform SVN propset of $svn_date on $gCfg{svnrepo}";

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
    print STDERR @_;
    print THE_REAL_STDERR @_;
}  #End PrintMsg

###############################################################################
#  MyDie
###############################################################################
sub MyDie {
    # any die() is trapped by $SIG{__DIE__} to ensure user sees fatal errors
    exit(255) if $gCfg{died}; # don't die 2x if fatal error in global cleanup
    exit(0) if $gCfg{hooray};

    warn @_;
    print THE_REAL_STDERR "\n", @_;

    (my $logfile = $gCfg{logfile}) =~ s:/:\\:g;

    my ($vsserr, $svnerr) = ('') x 2;

    if ((defined $VSS) && (defined $VSS->{ss_error})) {
        $vsserr = "\nLAST VSS COMMAND:\n$VSS->{ss_error}\n\n(You may find "
        . "more info on this error at the following website:\n"
        . "http://msdn.microsoft.com/library/default.asp?url=/library/"
        . "en-us/guides/html/vsorierrormessages.asp )";
    }

    if ((defined $SVN) && (defined $SVN->{svn_error})) {
        $svnerr = "\nLAST SVN COMMAND:\n$SVN->{svn_error}\n";
    }

    print THE_REAL_STDERR <<"EOERR";

******************************FATAL ERROR********************************
*************************************************************************

A fatal error has occured. The output from the last VSS or SVN command is
below, if available.

See $logfile for more information.
$vsserr$svnerr
EOERR
    $gCfg{died} = 1;
    exit(255);
}  #End MyDie

###############################################################################
#  Initialize
###############################################################################
sub Initialize {
    GetOptions(\%gCfg,'vssproject=s','vssexclude=s@','svnrepo=s','comment=s',
               'vsslogin=s','setdates','noprompt','timebias=i','restart',
               'utf8','debug','help',);

    &GiveHelp(undef, 1) if defined $gCfg{help};

    defined $gCfg{vssproject} or GiveHelp("must specify --vssproject\n");
    defined $gCfg{svnrepo} or GiveHelp("must specify --svnrepo\n");
    defined $ENV{SSDIR} or GiveHelp("\$SSDIR not defined\n");

    if ($gCfg{utf8} && ! $gCfg{allowUtf8}) {
        my $msg = <<"EOMSG";
ERROR: UTF-8 support is only available with the "Encoding" module, which
requires Perl 5.7.3 or higher. You must either install a newer version of Perl
or use the statically-compiled version of vss2svn to get UTF-8 support.
EOMSG
        $msg = fill('', '', $msg);
        die "\n$msg\n";
    }

    $gCfg{vssproject} =~ s:/$:: unless $gCfg{vssproject} eq '$/';
    $gCfg{vssprojmatch} = quotemeta( $gCfg{vssproject} );

    @{ $gCfg{vssexclude} } = split(',', join(',' ,@{ $gCfg{vssexclude} } ))
        if defined $gCfg{vssexclude};

    $gCfg{ssbin} = &CheckForExe
        ("ss.exe", "the Microsoft Visual SourceSafe client");

    $gCfg{svnbin} = &CheckForExe("svn.exe", "the Subversion client");

    my $vss_args = {
                    interactive => 'Y',
                    timebias    => $gCfg{timebias},
                   };

    if (defined $gCfg{vsslogin}) {
        @{ $vss_args }{'user', 'passwd'} = split(':', $gCfg{vsslogin});
        warn "\nATTENTION: about to issue VSS login command; if program\n"
            . "hangs here, you have specified an invalid VSS username\n"
            . "or password. (Press CTRL+Break to kill hung script)\n\n";
    }

    $VSS = Vss2Svn::VSS->new($ENV{SSDIR}, $gCfg{vssproject}, $vss_args);
    $VSS->{_debug} = 1;

    $SVN = Vss2Svn::Subversion->new( $gCfg{svnrepo} );
    $SVN->{interactive} = 0;
    $SVN->{user} = 'vss_migration';
    $SVN->{passwd} = ''; # all passwords are blank
    $SVN->{_debug} = 1;

    %USERS = ( vss_migration => 1, );

    $gCfg{globalCount} = 1;
    $gCfg{commitNumber} = 1;

    $gCfg{workbase} = cwd() . "/_vss2svn";

    print "\nCleaning up any previous vss2svn runs...\n\n";
    &RecursiveDelete( $gCfg{workbase} );
    mkdir $gCfg{workbase} or die "Couldn't create $gCfg{workbase} (does "
        . "another program have a lock on this directory or its files?)";

    $gCfg{workdir} = "$gCfg{workbase}/work";
    mkdir $gCfg{workdir} or die "Couldn't create $gCfg{workdir}";

    $gCfg{importdir} = "$gCfg{workbase}/import";
    mkdir $gCfg{importdir} or die "Couldn't create $gCfg{importdir}";

    $gCfg{tmpfiledir} = "$gCfg{workbase}/tmpfile";
    mkdir $gCfg{tmpfiledir} or die "Couldn't create $gCfg{tmpfiledir}";

    $gCfg{dbdir} = "$gCfg{workbase}/db";
    mkdir $gCfg{dbdir} or die "Couldn't create $gCfg{dbdir}";

    $VSS->{use_tempfiles} = "$gCfg{tmpfiledir}";

}

###############################################################################
#  Regionalize
###############################################################################
sub Regionalize {
    my $bias = $Registry->{'HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/'
                           .'Control/TimeZoneInformation/ActiveTimeBias'} || 0;
    {
        use integer; # forces Perl to interpret two's-complement correctly
        $gCfg{timebias} = hex($bias) + 0;
    }

}

###############################################################################
#  CheckForExe
###############################################################################
sub CheckForExe {
    my($exe, $desc) = @_;

    foreach my $dir (split ';', ".;$ENV{PATH}") {
        $dir =~ s/"//g;
        if (-f "$dir\\$exe") {
            return "$dir\\$exe";
        }
    }

    my $msg = fill('', '', <<"EOMSG");
Could not find executable '$exe' in your \%PATH\%. Ensure $desc is properly
installed on this computer, and manually add the directory in which '$exe' is
located to your path if necessary.

\%PATH\% currently contains:
EOMSG

    die "$msg\n$ENV{PATH}\n";
}

###############################################################################
#  CreateDatabase
###############################################################################
sub CreateDatabase {
    $gCfg{dbh} = DBI->connect("dbi:SQLite(RaiseError=>1,AutoCommit=>0)"
                              . ":dbname=$gCfg{dbdir}/vss2svn.db","","");
    my $cmd;

    $cmd = <<"EOSQL";
CREATE TABLE vss2svn_history
(
    date            long            NOT NULL,
    time            long            NOT NULL,
    file    varchar(1024)  NOT NULL,
    version long           NOT NULL,
    user    varchar(256)   NOT NULL,
    comment blob           NOT NULL,
    imported        integer         NOT NULL,
    global_count    long   NOT NULL
)
EOSQL

    $gCfg{dbh}->do($cmd) or die;

    $cmd = <<"EOSQL";
CREATE TABLE vss2svn_status
(
    code            long           NOT NULL,
    desc            varchar(1024)  NOT NULL,
    datestamp       long           NOT NULL
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
#   SetStatus
###############################################################################
sub SetStatus {
    my($status, $desc) = @_;
    $desc = $gCfg{dbh}->quote($desc);
    my $now = time;

    my $cmd = <<"EOSQL";
INSERT INTO
    vss2svn_status (
        code,
        desc,
        datestamp
    )
VALUES (
    $status,
    $desc,
    $now
)
EOSQL

    $gCfg{dbh}->do($cmd) or die;
    $gCfg{dbh}->commit;
}  # End SetStatus

###############################################################################
#  GiveHelp
###############################################################################
sub GiveHelp {
    my($msg, $verbose) = @_;
    $msg .= "\n" if defined $msg;

    $msg .= "USE --help TO VIEW FULL HELP INFORMATION\n" unless $verbose;

    if ($0 =~ /exe$/) {
        &GiveExeHelp($msg, $verbose); # will be created by .exe build script
    }

    pod2usage(
              {
                -message => $msg,
                -verbose => $verbose,
                -exitval => $verbose, # if user requested --help, go to STDOUT
              }
             );

}  #End GiveHelp



sub first(&@) {
	my $code = shift;
	&$code && return $_ for @_;
	return undef;
}


## EXE PRECOMPILE HERE

1;

__END__
=pod

=head1 LICENSE

vss2svn.pl, Copyright (C) 2004 by Toby Johnson.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
L<http://www.gnu.org/copyleft/gpl.html>

=head1 SYNOPSIS

vss2svn.pl S<--vssproject $/vss/project> S<--svnrepo http://svn/repo/url>

=over 4

=item --vssproject:

full path to VSS project you want to migrate

=item --svnrepo:

URL to target Subversion repository

=back

=head1 OPTIONS

=over 4

=item --exclude <EXCLUDE_PROJECTS>:

Exclude the given projects from the migration. To list multiple projects,
separate with commas or use multiple --exclude commands.

Each project can be given as an absolute path (beginning with $/) or
relative to --vssproject.

=item --comment "MESSAGE":

add MESSAGE to end of every migrated comment

=item --setdates:

Sets the "svn:date" property off all commits to reflect the
original VSS commit date, so that the original commit dates
(and not today's date) show up in your new SVN logs. This is
not the default, since setting svn:date could lead to
problems if not done correctly. Using this also requires the
"pre-revprop-change" Hook Script to be set; see
L<http://svnbook.red-bean.com/svnbook/ch05s02.html#svn-ch-5-sect-2.1>

=item --vsslogin "USER:PASSWD":

Set VSS username and password, separated by a colon.
B<WARNING --> if the username/password combo you provide is
incorrect, this program will hang as ss.exe prompts you for
a username! (This is an unavoidable Microsoft bug).

=item --timebias <OFFSET_MINUTES>:

Override the script's guess as to the number of minutes it should
add to your local time to get to GMT (for example, if you are
in Eastern Daylight Time [-0400], this should be 240).

=item --utf8:

Some users with non-English locales may find that the svn client
causes errors when importing comments containing non-English
characters. If this is the case with you, use this switch to
explicitly convert all comment messages to UTF-8 before importing
to Subversion.

=item --noprompt:

Don't prompt to confirm settings or to create usernames after
the first stage.

=item --debug:

Print all program output to screen as well as logfile.

=back

