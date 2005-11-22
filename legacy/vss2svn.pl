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


use warnings;
use strict;

use Cwd;
use DBD::SQLite;
use DBI;
use Data::Dumper;
use Encode;
use File::Path;
use Getopt::Long;
use Pod::Usage;
use SVN::Client;
use SVN::Core;
use SVN::Wc;
use Text::Wrap;
use Time::ParseDate;
use URI::URL;
use Win32::TieRegistry (Delimiter => '/');
use Win32;

our(%gCfg, $VSS, $SVNClient, $SNAPTIME, $SUBSTMAP, $COMMENT, $REVISION, $STATE_CHANGED);

sub first(&@);
sub PrintMsg; # defined later

&Vss2Svn::VSS::Initialize;

&Regionalize;
&Initialize;
&GiveStartupMessage;
&SetupLogfile;

&CreateDatabase;

&GetProjectTree;
&BuildHistory;
&GiveSchemaAuthMessage unless $gCfg{noprompt};

# the SVNClient seems to need to be somewhere in the working copy in order to work
chdir $gCfg{workdir} or die "Could not change to directory $gCfg{workdir}";

if ($gCfg{globalCiCount} < 0) {
	&CheckoutSvnWC;
}
&ApplySvnActions;
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
    my $setrevprop = $gCfg{setrevprop} ? 'yes' : 'no';

    print <<"EOMSG";

         ss.exe Found: $VSS->{_ssexe}

          VSS Project: $gCfg{vssproject}
       Subversion URL: $gCfg{svnrepo}

    Local Date Format: $Vss2Svn::VSS::gCfg{dateString}
    Local Time Format: $Vss2Svn::VSS::gCfg{timeString}
 Time Bias To Get GMT: $gCfg{timebias} minutes
   Bias to use in DST: $gCfg{dstbias} minutes

     Set SVN revprops: $setrevprop
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
    open STDERR, ">>$gCfg{logfile}"
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
    my $tree;
    if (!defined $gCfg{restart} || (defined $gCfg{restart} && defined $gCfg{update})) {
        my $msg = (defined $gCfg{update}) ? "UPDATED" : "INITIAL";
        PrintMsg "\n\n**** BUILDING $msg STRUCTURES; PLEASE WAIT... ****\n\n";

        # grab the project tree from the DIR listing
        $tree = $VSS->project_tree($gCfg{vssproject},1,1,
                                   ("endlabel" => $gCfg{snaplabel}, "endtime" => $SNAPTIME))
            or die "Couldn't create project tree for $gCfg{vssproject}";

        my $projsth = $gCfg{dbh}->prepare("INSERT INTO treetable (tag, type, file, version)"
                                          . " VALUES('update', 'project', ?, 0)") 
            or die "Could not execute DBD::SQLite command";
        my $filesth = $gCfg{dbh}->prepare("INSERT INTO treetable (tag, type, file, version)"
                                          . " VALUES('update', 'file', ?, ?)") 
            or die "Could not execute DBD::SQLite command";

        # insert the project tree structure into the database in one transaction
        my $ac = $gCfg{dbh}->{AutoCommit};
        my $re = $gCfg{dbh}->{RaiseError};

        $gCfg{dbh}->{AutoCommit} = 0;
        $gCfg{dbh}->{RaiseError} = 1;
        eval {
            &WalkTree($tree, $gCfg{vssproject}, $projsth, $filesth);
            &PruneVssExcludes;
            # we must compare the update vs the old snapshot to add/remove files and directories
            if (defined $gCfg{update}) {
                PrintMsg "\n\n**** Comparing trees... ****\n\n";
                &CompareTrees;
            }
            $gCfg{dbh}->do("DELETE FROM treetable WHERE tag='current'");
            $gCfg{dbh}->do("UPDATE treetable SET tag='current' WHERE tag='update'");

            # remember to add the new projects later
            if ($gCfg{globalCiCount} < 0) {
                PrintMsg "\n\n**** Enqueueing projects... ****\n\n";
                &EnqueueSvnProjects;
            }
            $gCfg{dbh}->commit;
        };
        if ($@) {
            PrintMsg "Transaction aborted because $@";
            eval { $gCfg{dbh}->rollback };
            die "Transaction failed!";
        }      
        $projsth->finish();
        $filesth->finish();

        $gCfg{dbh}->{AutoCommit} = $ac;
        $gCfg{dbh}->{RaiseError} = $re;
    }
}

###############################################################################
#  PruneVssExcludes
###############################################################################
sub PruneVssExcludes {

    return unless defined $gCfg{vssexclude};
    return if defined $gCfg{restart};

    # By this point, we already have the entire "naked" directory structure
    # in the database and we prune off any branches that match exclude.

    my $prunesth = $gCfg{dbh}->prepare("DELETE FROM treetable WHERE tag='update' AND file LIKE ?") 
        or die "Could not execute DBD::SQLite command";

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
        } elsif ($exclude =~ m:^$:) {
            PrintMsg "**WARNING: Exclude path \"$exclude\" is entire project of "
                . "$gCfg{vssproject}; ignoring...\n";
            next EXCLUDE;
        }

        # so we are in the project at least
        $exclude = $gCfg{vssproject} . "/" . $exclude;
        my $esc = $gCfg{dbh}->get_info(14); # get the SQL escape char

        # this RE is for the `file' column
        my $pathsqlre = $exclude;
        $pathsqlre =~ s/([_%])/$esc$1/g; # quote any SQL re that happens to be in the path
        $pathsqlre .= "%"; # append the real SQL re that we are looking for
        $prunesth->execute($pathsqlre) or die "Could not execute DBD::SQLite command";
    }
    $prunesth->finish();

}  #  End PruneVssExcludes

###############################################################################
#  CompareTrees
###############################################################################
sub CompareTrees {

    my $deletedFileList = [];
    my $deletedProjectList = [];
    my $esc = $gCfg{dbh}->get_info(14); # get the SQL escape char

    # look for deleted projects first
    # remember these, since we may have to delete files, too
    my $sth = $gCfg{dbh}->prepare("SELECT file FROM treetable WHERE tag='current' AND type='project' "
                                  . "AND file NOT IN (SELECT file FROM treetable WHERE tag='update' AND type='project') ORDER BY file");

    $sth->execute();
    while (my $row = $sth->fetchrow_hashref()) {
        push @{$deletedProjectList}, $row->{file};
    }
    $sth->finish();

    # look for added projects
    # just go ahead and add these items
    $sth = $gCfg{dbh}->prepare("SELECT file FROM treetable WHERE tag='update' AND type='project' "
                               . "AND file NOT IN (SELECT file FROM treetable WHERE tag='current' AND type='project') ORDER BY file");

    my $insactionsth = $gCfg{dbh}->prepare("INSERT INTO svnaction (type, action, global_count, file) VALUES('project', 'add', 0, ?)");

    $sth->execute();
    while (my $row = $sth->fetchrow_hashref()) {
        $insactionsth->execute($row->{file});
    } 
    $sth->finish();
    $insactionsth->finish();

    # look for deleted files
    $sth = $gCfg{dbh}->prepare("SELECT file FROM treetable WHERE tag='current' AND type='file' "
                               . "AND file NOT IN (SELECT file FROM treetable WHERE tag='update' AND type='file') ORDER BY file");

    $sth->execute();
    while (my $row = $sth->fetchrow_hashref()) {
        push @{$deletedFileList}, $row->{file};
    }
    $sth->finish();

    # cull the deleted projects containing subdirectories and files
    if (scalar @{$deletedProjectList} > 0) {

        # mark references as complete in the database
        my $delprojsth = $gCfg{dbh}->prepare("UPDATE history SET deleted = 1 WHERE file LIKE ?");
        my $delcpsth = $gCfg{dbh}->prepare("DELETE FROM checkpoint WHERE file LIKE ?");

        # mark further action for SVNClient to take
        my $delactionsth = $gCfg{dbh}->prepare("INSERT INTO svnaction (type, action, global_count, file) VALUES('project', 'delete', 0, ?)");
        my $elem;
        while (defined ($elem = shift @{$deletedProjectList})) {
            # this RE is for subdirs and files in the lists
            my $pathre = quotemeta($elem) . "\/.*";

            # this RE is for the `file' column in the checkpoint relation
            my $pathsqlre = $elem;
            $pathsqlre =~ s/([_%])/$esc$1/g; # quote any SQL re that happens to be in the path
            $pathsqlre .= "/%"; # append the real SQL re that we are looking for

            # remove subdirectories of this directory from deleted projects list
            my $newList = [];
            foreach my $newElem (@{$deletedProjectList}) {
                if (!($newElem =~ m/$pathre/)) {
                    push @{$newList}, $newElem;
                }
            }
            $deletedProjectList = $newList;

            # remove files in this directory from the deleted files list
            $newList = [];
            foreach my $newElem (@{$deletedFileList}) {
                if (!($newElem =~ m/$pathre/)) {
                    push @{$newList}, $newElem;
                }
            }
            $deletedFileList = $newList;

            $delactionsth->execute($elem);
            $delprojsth->execute($pathsqlre);
            $delcpsth->execute($pathsqlre);
        }
        $delactionsth->finish();
        $delprojsth->finish();
        $delcpsth->finish();
    }

    if (scalar @{$deletedFileList} > 0) {
        # mark references as complete in the database
        my $delprojsth = $gCfg{dbh}->prepare("UPDATE history SET deleted = 1 WHERE file = ?");
        my $delcpsth = $gCfg{dbh}->prepare("DELETE FROM checkpoint WHERE file = ?");

        # mark further action for SVNClient to take
        my $delactionsth = $gCfg{dbh}->prepare("INSERT INTO svnaction (type, action, global_count, file) VALUES('file', 'delete', 0, ?)");
        my $elem;
        while (defined ($elem = shift @{$deletedFileList})) {
            $delactionsth->execute($elem);
            $delprojsth->execute($elem);
            $delcpsth->execute($elem);
        }
    }
}

###############################################################################
#  BuildHistory
###############################################################################
sub BuildHistory {
    PrintMsg "\n\n**** BUILDING VSS HISTORY ****\n\n";

    # There are two distinct groups in this list that qualify for new history:
    # 1) Files in the treetable that have old history that are getting updated
    #    (according to the version in treetable).  These files have already been checkpointed.
    # 2) New files in the treetable with no previous history.  These files have not been checkpointed.
    my $limsize = 512;
    my $cmd;

    if (!defined $gCfg{update} || $gCfg{update} eq "fast") {
        $cmd = "SELECT a.file, b.histversion, a.version AS version "
        . "   FROM (SELECT file, version FROM treetable WHERE tag='current' AND type='file' AND version > 0) AS a, "
        . "        (SELECT file, MAX(version) AS histversion FROM history WHERE deleted = 0 GROUP BY file) AS b "
        . "   WHERE a.file = b.file AND a.version <> b.histversion"
        . " UNION "
        . "SELECT file, 0 AS histversion, version "
        . "   FROM (SELECT file, version FROM treetable WHERE tag='current' AND type='file' AND version > 0) "
        . "   WHERE file NOT IN (SELECT file FROM checkpoint) "
        . "ORDER BY file LIMIT $limsize";
    } elsif (defined $gCfg{update} && $gCfg{update} eq "complete") {
        $cmd = "SELECT a.file, b.histversion, a.version AS version "
        . "   FROM (SELECT file, version FROM treetable WHERE tag='current' AND type='file' AND version > 0) AS a, "
        . "        (SELECT file, MAX(version) AS histversion FROM history WHERE deleted = 0 GROUP BY file) AS b "
        . "   WHERE a.file = b.file"
        . " UNION "
        . "SELECT file, 0 AS histversion, version "
        . "   FROM (SELECT file, version FROM treetable WHERE tag='current' AND type='file' AND version > 0) "
        . "   WHERE file NOT IN (SELECT file FROM checkpoint) "
        . "ORDER BY file LIMIT $limsize";
    }
    
    # get all new files in the tree
    my $fplsth = $gCfg{dbh}->prepare($cmd) 
        or die "Could not execute DBD::SQLite command";

    PrintMsg "\n\n**** IMPORTING FILE HISTORIES ****\n\n";

    # creates new checkpoint for a file, meaning that history has been added
    my $cptsth = $gCfg{dbh}->prepare("INSERT OR IGNORE INTO checkpoint (file, checked) VALUES (?, 1)")
        or die "Could not execute DBD::SQLite command";

    # query for recovered file
    my $recsth;

    if (defined $gCfg{update}) {
        # this checks to see if it was deleted in history at all
        $recsth = $gCfg{dbh}->prepare("SELECT file FROM history WHERE file = ? AND deleted = 1")
            or die "Could not execute DBD::SQLite command";
    }

    # create the command to insert history here
    my $inshistcmd = "INSERT INTO history (tstamp, file, version, user, comment, global_count, retrieved, deleted)"
        . " VALUES (?, ?, ?, ?, ?, ?, 0, 0)";
    my $inshiststh = $gCfg{dbh}->prepare($inshistcmd)
        or die "Could not execute DBD::SQLite command";

  BIGLOOP:
    while (1) {
        # copy the query results so we can start tranactions
        $fplsth->execute() or die "Could not execute DBD::SQLite command";
        my $filelst = [];
        while (my $fprow = $fplsth->fetchrow_hashref()) {
            push @{$filelst}, {file => $fprow->{file}, version => $fprow->{version}, histversion => $fprow->{histversion}};
        }
        $fplsth->finish();

        # exit the loop if there were no files to add
        if ((scalar @{$filelst}) == 0) {
            last BIGLOOP;
        }
        
        my $ac = $gCfg{dbh}->{AutoCommit};
        my $re = $gCfg{dbh}->{RaiseError};
        
        foreach my $fpr (@{$filelst}) {
            my $foo = \%{$fpr};
            my $filepath = $foo->{file};
            
            my %versionInfo = ("endversion" => $foo->{version}, "beginversion" => $foo->{histversion});
            
            my $deletedInHistory = 0;
            if (defined $recsth && $versionInfo{beginversion} == 0) {
                $recsth->execute($filepath) or die "Could not execute DBD::SQLite command";
                my $tmp;
                $recsth->bind_columns(\$tmp);
                while ($recsth->fetch()) {
                    $deletedInHistory = 1;
                }
            }

            $gCfg{dbh}->{AutoCommit} = 0;
            $gCfg{dbh}->{RaiseError} = 1;
            eval {
                &AddFileHistory($filepath, $inshiststh, $deletedInHistory, %versionInfo);
                $cptsth->execute($filepath);
                $gCfg{dbh}->do("DELETE FROM agenda WHERE iorder = 0"); # flag scheduler
                $gCfg{dbh}->commit;
            };
            if ($@) {
                PrintMsg "Transaction aborted because $@";
                eval { $gCfg{dbh}->rollback };
                die "Transaction failed!";
            }      
            $gCfg{dbh}->{AutoCommit} = $ac; # commit side effect
            $gCfg{dbh}->{RaiseError} = $re;
        }
    }
    $cptsth->finish();
    $recsth->finish() if defined $recsth;
    $inshiststh->finish();

    PrintMsg "\n\n**** DONE BUILDING VSS HISTORY ****\n\n";
}

###############################################################################
#  WalkTree
###############################################################################
sub WalkTree {
    my($branch, $project, $projsth, $filesth) = @_;

    my($key, $val, $newproj);
    my @branches = ();

    foreach $key (sort keys %$branch) {
        $val = $branch->{$key};

        if (ref($val) eq 'HASH') {
            # subproject; create a new branch of the tree

            push @branches, {branch => $val, project => "$key"};

        } elsif (!ref $val) {
            # a scalar, i.e. regular file

            my $filepath = &CreateFilepath($project, $key);

            # save it away
            $filesth->execute($filepath, $val);
        }
    }

    foreach my $subbranch (@branches) {
        # save the project away and keep looking
        ($newproj = "$project/$subbranch->{project}") =~ s://:/:;

        $projsth->execute($newproj);

        &WalkTree($subbranch->{branch}, $newproj, $projsth, $filesth);
    }
}

###############################################################################
#  CreateFilepath
###############################################################################
sub CreateFilepath {
    my($project, $file) = @_;
    (my $filepath = "$project/$file") =~ s://:/:;

    # SS.exe uses a semicolon to indicate a "pinned" file
    $filepath =~ s/;(.*)//;

    return $filepath;
}

###############################################################################
#  AddFileHistory
###############################################################################
sub AddFileHistory {
    my($filepath, $inshiststh, $deletedInHistory, %versionInfo) = @_;

    warn "AddFileHistory: filepath: " . $filepath . " deletedInHistory: " . $deletedInHistory
        . " start: " . $versionInfo{"beginversion"} . " end: " . $versionInfo{"endversion"};

    # build the revision history for this file
    my $historypath = $filepath;

    if (defined $SUBSTMAP->{$filepath}) {
        $historypath = $SUBSTMAP->{$filepath};
    }

    my $keepLabel = !(defined $gCfg{nolabel});
    my ($filehist, $qhiststh);
    my $redoHistory = !($versionInfo{"endversion"} > $versionInfo{"beginversion"});

    if ($redoHistory) {
        warn "History rolled back for " . $filepath . ": start:" . $versionInfo{"beginversion"} . " end:" . $versionInfo{"endversion"};

        # file has been rolled back, pinned, or replaced by a totally different file with the same name
        # we need to look at entire history with the last case

        # we must also invalidate at least the "future" versions
        my $delsth = $gCfg{dbh}->prepare("UPDATE history SET deleted = 1 WHERE file = ? AND version > ?");
        $delsth->execute($filepath, $versionInfo{"endversion"});
        $delsth->finish();
        $versionInfo{"beginversion"} = 0;
        $qhiststh = $gCfg{dbh}->prepare("SELECT MAX(global_count) AS global_count FROM history WHERE tstamp = ? AND file = ? AND version = ? AND user = ? AND comment = ? AND deleted = 0 GROUP BY file");
    } elsif ($deletedInHistory) {
        warn "Deleted history for " . $filepath . ": start:" . $versionInfo{"beginversion"} . " end:" . $versionInfo{"endversion"};
        # if the file has been deleted in history, it could have one or more deleted
        # revisions that may be made active, including being recovered
        $qhiststh = $gCfg{dbh}->prepare("SELECT MAX(global_count) AS global_count FROM history WHERE tstamp = ? AND file = ? AND version = ? AND user = ? AND comment = ? AND deleted = 1 GROUP BY file");
    }
    # otherwise, the file is new or updating 

    ++$versionInfo{"beginversion"}; # fix off by one before getting history
    $filehist = $VSS->file_history($historypath, $keepLabel, %versionInfo);
    die "Internal error while reading VSS file history for $filepath"
        if !defined $filehist;

    PrintMsg "   $filepath\n";

    # this is in case we just rolled back or pinned
    # read the history in reverse and find the first place we were there
    my $revversion;
    if ($redoHistory || $deletedInHistory) {
        my $revfilehist = [];
        foreach my $foo (@$filehist) {
            unshift @{$revfilehist}, $foo;
        }
        my $scheduledCopy = 0;
        my $fixsth = $gCfg{dbh}->prepare("UPDATE history SET deleted = 0 WHERE global_count = ?");
        my $fix2sth = $gCfg{dbh}->prepare("UPDATE history SET deleted = 1 WHERE tstamp = ? AND file = ? AND version = ? AND user = ? AND comment = ?");
        my $currhiststh = $gCfg{dbh}->prepare("SELECT file FROM history WHERE file = ? AND deleted = 0");
        my ($chrow, $fileInCurrentHistory);
        $fileInCurrentHistory = 0;
        $currhiststh->execute($filepath);
        while ($chrow = $currhiststh->fetchrow_hashref()) {
            $fileInCurrentHistory = 1;
        }
        $currhiststh->finish();
      REVWALK:
        foreach my $rev (@$revfilehist) {
            $rev->{comment} = "" unless defined $rev->{comment};
            warn "Looking for: tstamp " . $rev->{tstamp} . " " . $filepath . " " . $rev->{version} . " " . $rev->{user} . " " . $rev->{comment};
            warn "tstamp undefined" unless defined $rev->{tstamp};
            warn "filepath undefined" unless defined $filepath;
            warn "version undefined" unless defined $rev->{version};
            warn "user undefined" unless defined $rev->{user};
            warn "comment undefined" unless defined $rev->{comment};

            $qhiststh->execute($rev->{tstamp}, $filepath, $rev->{version}, $rev->{user}, $rev->{comment});
            my ($qrow, $gc, $revision);
            while ($qrow = $qhiststh->fetchrow_hashref()) {
                $gc = $qrow->{global_count};
            }
            if (defined $gc && !$scheduledCopy) {
                warn "Found global_count: " . $gc;

                $revversion = $rev->{version};
                my ($undelactionsth, $checkpointsth);

                # if the file is not there, it must be copied there
                # otherwise if the file is there, it must be rolled back
                warn "fileInCurrentHistory: " . $fileInCurrentHistory;
                if ($fileInCurrentHistory) {
                    $undelactionsth = $gCfg{dbh}->prepare("INSERT INTO svnaction (type, action, global_count, file, args) VALUES('file', 'merge', 0, ?, ?)");
                } else {
                    $undelactionsth = $gCfg{dbh}->prepare("INSERT INTO svnaction (type, action, global_count, file, args) VALUES('file', 'copy', 0, ?, ?)");
                    $checkpointsth =  $gCfg{dbh}->prepare("INSERT OR IGNORE INTO checkpoint (file, checked) VALUES(?, 3)");
                }

                # get the destination target
                my $dst_target = $filepath;
                $dst_target =~ m/^(.*\/)(.*)/;
                my($path, $file) = ($1, $2);
                $path =~ s/$gCfg{vssprojmatch}//;
                $path =~ s/\/$//; # remove trailing slash
                $dst_target = "$path/$file";
                $dst_target = "." . $dst_target if ($dst_target =~ m/^\//);

                # get the source URL
                my $src_target = $gCfg{svnrepo};
                $src_target =~ s/\/$//; # remove trailing slash
                $src_target .= "$path/$file";

                my $url = URI::URL->new($src_target);
                $src_target = $url->as_string;

                # get the revision number
                my $retactsth = $gCfg{dbh}->prepare("SELECT b.revision FROM agenda AS a, commitpoint AS b WHERE a.global_count = ? AND a.number = b.number");
                $retactsth->execute($gc);
                while ($qrow = $retactsth->fetchrow_hashref()) {
                    $revision = $qrow->{revision};
                }
                $retactsth->finish();

                # stuff it into args
                warn "src_target: " . $src_target if defined $src_target;
                warn "revision: " . $revision;
                warn "dst_target: " . $dst_target;
                my $tmpAry;
                my $encodedDstTarget = Encode::encode('utf8', $dst_target);
                if ($fileInCurrentHistory) {
                    $tmpAry = [$encodedDstTarget, 'BASE', $encodedDstTarget, $revision, $encodedDstTarget, 0, 0, 1, 0]; 
                } else {
                    $tmpAry = [$src_target, $revision, $encodedDstTarget]; 
                }
                my $args = Data::Dumper->Dump([$tmpAry], [qw(*myargs)]);
                warn "args: " . $args;
                $undelactionsth->execute($filepath, $args);
                $undelactionsth->finish();
                $checkpointsth->execute($filepath) if defined $checkpointsth;
                $checkpointsth->finish() if defined $checkpointsth;
                $scheduledCopy = 1;
            }
            # fix up the database
            if (defined $gc && $deletedInHistory) {
                # mark the history as current, as long as we have a match
                $fixsth->execute($gc);
            } elsif ($redoHistory && !$scheduledCopy) {
                # invalidate history, since we didn't have a match
                $fix2sth->execute($rev->{tstamp}, $filepath, $rev->{version}, $rev->{user}, $rev->{comment});
            }

        }
        $qhiststh->finish();
        $fixsth->finish();
        $fix2sth->finish();
    }

REV:
    foreach my $rev (@$filehist) {
        next REV if (defined $revversion && $rev->{version} <= $revversion);

        $rev->{comment} = "" unless defined $rev->{comment};
        $rev->{filepath} = $filepath;
        $rev->{retrieved} = 0;
        $rev->{deleted} = 0;

        $inshiststh->execute($rev->{tstamp}, $rev->{filepath}, $rev->{version}, $rev->{user},
                             $rev->{comment}, ($rev->{globalCount} = ++$gCfg{globalCount}));
        warn $inshiststh->dump_results;
    }

} # End AddFileHistory

###############################################################################
#  GiveSchemaAuthMessage
###############################################################################
sub GiveSchemaAuthMessage {
   my ($svnuser,$svnpw) = split(/:/, $gCfg{svnlogin}, -1);
   print THE_REAL_STDERR <<"EOTXT";


ATTENTION REQUIRED:

EOTXT

  if ($gCfg{svnrepo} =~ m/^http:/) {
      print THE_REAL_STDERR <<"EOTXT";

  The user '$svnuser' must be able to authenticate to
  the repository '$gCfg{svnrepo}'.

  Usually, this is done by adding the user '$svnuser' to the correct
  AuthUserFile (for Apache 2) for the Location of the subversion
  repository on the HTTP server.

  This is a basic authentication file, where each username is followed
  by ':' then the hashed password for that user.  A blank password
  is permissible.

  See <http://svnbook.red-bean.com/en/1.1/ch06s04.html#svn-ch-6-sect-4.3.1> for
  more information.

EOTXT

  } elsif ($gCfg{svnrepo} =~ m/^https:/) {
      print THE_REAL_STDERR <<"EOTXT";

  The user '$svnuser' must be able to authenticate to
  the repository '$gCfg{svnrepo}'.

  Some configuration may be required to manage client certificate
  files for the user '$svnuser'. The client certificate file location may 
  have to be added to %APPDATA%\\Subversion\\servers

  See <http://svnbook.red-bean.com/en/1.1/ch06s04.html#svn-ch-6-sect-4.3.2> for
  more information.

EOTXT

  } elsif ($gCfg{svnrepo} =~ m/^svn:/) {
   print THE_REAL_STDERR <<"EOTXT";

  The user '$svnuser' must be able to authenticate to
  the repository '$gCfg{svnrepo}'.

  This is done by adding the user '$svnuser' to the repository's conf/svnserve.conf
  file.  The user name is followed by '=', then the password.

  See <http://svnbook.red-bean.com/en/1.1/ch06s03.html#svn-ch-6-sect-3.2> for
  more information.

EOTXT
  } elsif ($gCfg{svnrepo} =~ m/^svn\+(\w+):/) {
      print THE_REAL_STDERR <<"EOTXT";

  The user '$svnuser' must be able to authenticate to
  the repository '$gCfg{svnrepo}'.

  You are tunneling authentication over the '$1' protocol.

  This is done by adding the user '$svnuser' to the repository's conf/svnserve.conf
  file.  The user name is followed by '=' then the password.

  You may have to perform some other kind of authentication caching for
  the '$1' tunnel.

  See <http://svnbook.red-bean.com/en/1.1/ch06s03.html#svn-ch-6-sect-3.4> for
  more information.

EOTXT
 } elsif ($gCfg{svnrepo} =~ m/^file:/) {
      print THE_REAL_STDERR <<"EOTXT";

  For this repository no authentication is available.
  Do NOT pass the --svnlogin parameter to this script.

  You will only need write permission for the repository's database files.

EOTXT
 } else {
      print THE_REAL_STDERR <<"EOTXT";

  I have no idea on how to help you authenticate the user '$svnuser' over
  the schema for the repository '$gCfg{svnrepo}'.

  Good luck, sport!

EOTXT
 }

  if ($gCfg{setrevprop}) {
      print THE_REAL_STDERR <<"EOTXT";
  Since you are setting revision properties, now would be a good time
  to check and see that the user '$svnuser' can set the "svn:date" and
  "svn:author" properties for the repository '$gCfg{svnrepo}'.

  This requires that the "pre-revprop-change" hook script be set.
  See <http://svnbook.red-bean.com/en/1.1/svn-book.html#svn-ch-5-sect-2.1> for
  more information.

EOTXT
  }

  print THE_REAL_STDERR "\n\nPRESS ENTER TO CONTINUE (or enter [q] to quit and start over)...";

    my $rep = <STDIN>;

    if ($rep =~ /^q/i) {
       print THE_REAL_STDERR "\n\nQuitting...\n";
       exit(0);
    }
}

###############################################################################
#  EnqueueSvnProjects
###############################################################################
sub EnqueueSvnProjects {
    my $insactionsth = $gCfg{dbh}->prepare("INSERT INTO svnaction (type, action, global_count, file) VALUES ('project', 'add', 0, ?)");
    
    my $pplsth = $gCfg{dbh}->prepare("SELECT file FROM treetable WHERE tag = 'current' AND type = 'project' ORDER BY file");

    $pplsth->execute();
    while (my $row = $pplsth->fetchrow_hashref()) {
        PrintMsg "        " . $row->{file} . "...\n";
        $insactionsth->execute($row->{file});
    }
    $pplsth->finish();
    $insactionsth->finish();
}

###############################################################################
#  CheckoutSvnWC
###############################################################################
sub CheckoutSvnWC {
    PrintMsg "\n\n**** SETTING UP SUBVERSION DIRECTORIES ****\n\n";

    PrintMsg "   Checking out working copy...\n";
    # pull down the working copy
    $SVNClient->checkout($gCfg{svnrepo}, Encode::encode('utf8', Cwd::getcwd()), 'HEAD', 1); 
}

###############################################################################
#  CreateAddedProjectsList
###############################################################################
sub CreateAddedProjectsList {
    my $svnprojects = [];

    # create the projects to be added
    my $pplsth = $gCfg{dbh}->prepare("SELECT file FROM svnaction WHERE type='project' AND action='add' AND global_count=0 ORDER BY file");
    $pplsth->execute();
    while (my $row = $pplsth->fetchrow_hashref()) {
        my $dosProject = $row->{file};
        $dosProject =~ s:^$gCfg{vssprojmatch}::;
        $dosProject =~ s:^/::;
        push @{$svnprojects}, Encode::encode('utf8', $dosProject);
    }
    $pplsth->finish();
    return $svnprojects;
}

###############################################################################
#  CreateDeletedList
###############################################################################
sub CreateDeletedList {
    my $projpathRoot = $gCfg{workdir};
    $projpathRoot =~ s/\\/\//g;
    my $svnprojects = [];

    # delete the files/projects needing deleteion
    my $pplsth = $gCfg{dbh}->prepare("SELECT file FROM svnaction WHERE action='delete' AND global_count=0 ORDER BY file");
    $pplsth->execute();
    while (my $row = $pplsth->fetchrow_hashref()) {
        my $dosProject = $row->{file};
        $dosProject =~ s:^$gCfg{vssprojmatch}::;
        $dosProject = '/' . $dosProject unless $dosProject =~ m/^\//;
        push @{$svnprojects}, Encode::encode('utf8', $projpathRoot . $dosProject);
    }
    $pplsth->finish();
    
    return $svnprojects;
}

###############################################################################
#  CreateCopiedList
###############################################################################
sub CreateCopiedList {
    my $projpathRoot = $gCfg{workdir};
    $projpathRoot =~ s/\\/\//g;
    my $svnprojects = [];

    # copy the files from 
    my $pplsth = $gCfg{dbh}->prepare("SELECT args FROM svnaction WHERE action='copy' AND global_count=0 ORDER BY file");
    $pplsth->execute();
    while (my $row = $pplsth->fetchrow_hashref()) {
        my $args = $row->{args};
        push @{$svnprojects}, $args;
    }
    $pplsth->finish();

    return $svnprojects;
}

###############################################################################
#  CreateMergedList
###############################################################################
sub CreateMergedList {
    my $projpathRoot = $gCfg{workdir};
    $projpathRoot =~ s/\\/\//g;
    my $svnprojects = [];

    # copy the files from 
    my $pplsth = $gCfg{dbh}->prepare("SELECT args FROM svnaction WHERE action='merge' AND global_count=0 ORDER BY file");
    $pplsth->execute();
    while (my $row = $pplsth->fetchrow_hashref()) {
        my $args = $row->{args};
        push @{$svnprojects}, $args;
    }
    $pplsth->finish();

    return $svnprojects;
}

###############################################################################
#  ApplySvnActions
###############################################################################
sub ApplySvnActions {
    PrintMsg "\n\n**** APPLYING SUBVERSION ACTIONS ****\n\n";

    $STATE_CHANGED = 0;
    my $ac = $gCfg{dbh}->{AutoCommit};
    my $re = $gCfg{dbh}->{RaiseError};

    $gCfg{dbh}->{AutoCommit} = 0;
    $gCfg{dbh}->{RaiseError} = 1;

    eval {
        $SVNClient->notify(\&wc_changed);
        my $svnprojects = &CreateAddedProjectsList;

        if (scalar @{$svnprojects} > 0) {
            PrintMsg "   Importing project directories into subversion working copy...\n";
            File::Path::mkpath($svnprojects, 1);
            
            foreach my $proj (@{$svnprojects}) {
                $SVNClient->add($proj, 0);
            }
        }
        
        $svnprojects = &CreateDeletedList;
        if (scalar @{$svnprojects} > 0) {
            PrintMsg "   Deleting from subversion working copy...\n";
            $SVNClient->delete($svnprojects, 1);
        }

        $svnprojects = &CreateCopiedList;
        my $copiedMsg = "";
        if (scalar @{$svnprojects} > 0) {
            PrintMsg "   Copying historical versions from repository...\n";
            
            foreach my $proj (@{$svnprojects}) {
                my @myargs;
                eval($proj); # reads into "myargs"
                $SVNClient->copy($myargs[0], $myargs[1], $myargs[2]);
                $copiedMsg .= $myargs[0] . ", revision " . $myargs[1] . ", was recovered.\n";
            }
        }

        $svnprojects = &CreateMergedList;
        if (scalar @{$svnprojects} > 0) {
            PrintMsg "   Merging historical versions from repository...\n";
            
            foreach my $proj (@{$svnprojects}) {
                my @myargs;
                eval($proj); # reads into "myargs"
                $SVNClient->merge($myargs[0], $myargs[1], $myargs[2], $myargs[3],
                                  $myargs[4], $myargs[5], $myargs[6], $myargs[7], $myargs[8]);
                $copiedMsg .= $myargs[0] . ", revision " . $myargs[3] . ", was recovered.\n";
            }
        }

        if ($STATE_CHANGED) {
            my $coderef;
            $SVNClient->notify($coderef);

            # write this into the agenda, even though it's "unscheduled"
            my $tmpCommitNumber = &GetScheduleCount;
            my $iorder = &GetIorder;

            $tmpCommitNumber = 0 unless (defined $tmpCommitNumber);
            $iorder = 0 unless (defined $iorder);
            ++$iorder;

            my $asth = $gCfg{dbh}->prepare("INSERT INTO agenda ( number, global_count, iorder ) VALUES ( ?, 0, ? )");
            $asth->execute(++$tmpCommitNumber, $iorder++);
            $asth->finish;

            # commit it
            PrintMsg "   Committing `" . $gCfg{workdir} . "'...\n";
            
            my $msg = ($gCfg{globalCiCount} < 0) ? "Initial " : "";
            my $eventMsg = "";
            if (defined $SNAPTIME) {
                $eventMsg .= " at ";
                my ($sec,$min,$hour,$mday,$mon,$year) = gmtime($SNAPTIME);
                ++$mon;
                $year += 1900;
                $eventMsg .= sprintf("%4.4i-%2.2i-%2.2iT%2.2i:%2.2i:%2.2iZ", $year, $mon, $mday, $hour, $min, $sec);
            } elsif (defined $gCfg{snaplabel}) {
                $eventMsg .= " at label `" . $gCfg{snaplabel} . "'";
            }
            my $finalMsg = $msg . "Import of " . $gCfg{vssproject} . $eventMsg . "\n";
            $finalMsg .= $copiedMsg;
            $COMMENT = Encode::encode('utf8', $finalMsg);
            $SVNClient->log_msg(\&log_comments); 
            my ($commit_val) = $SVNClient->commit(Encode::encode('utf8', $gCfg{workdir}), 0);
            
            my $ciRevision;
            if (!defined $commit_val || $commit_val->revision() == $SVN::Core::INVALID_REVNUM) {
                $ciRevision = GetSvnInfo(Cwd::getcwd());
                if (!defined $ciRevision) {
                    die "GetSvnInfo failed, bailing";
                }
            } else {
                $ciRevision = $commit_val->revision();
            }

            # update the database now that we have commited
            my $updsth = $gCfg{dbh}->prepare("UPDATE ciCount SET number = ?");
            $updsth->execute(++$gCfg{globalCiCount});   
            $updsth->finish;
        }

        $gCfg{dbh}->do("DELETE FROM svnaction WHERE global_count=0");
        $gCfg{dbh}->commit;
    };
    if ($@) {
        PrintMsg "Transaction aborted because $@";
        eval { $gCfg{dbh}->rollback };
        die "Transaction failed!";
    }      
    
    $gCfg{dbh}->{AutoCommit} = $ac;
    $gCfg{dbh}->{RaiseError} = $re;
}

###############################################################################
#  ScheduleCommits
###############################################################################
sub ScheduleCommits {
    # this is the flag to tell us if scheduling has completed
    my $sth = $gCfg{dbh}->prepare("SELECT number FROM agenda WHERE iorder = 0")
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";
    
    my $tmp;
    my $beenScheduled = 0;
    $sth->bind_columns(\$tmp);
    while ($sth->fetch()) {
        $beenScheduled = 1;
    }
    $sth->finish();

    if (!$beenScheduled) {

        # schedule the rows
        PrintMsg "   Scheduling ...";

        my ($row, $prevRow);
        my $tmpCommitNumber = &GetScheduleCount; # prevRow not set up, but we are are 1 based
        my $seenThisRev = {};
        my $iorder = &GetIorder;

        $tmpCommitNumber = 0 unless (defined $tmpCommitNumber);
        $iorder = 0 unless (defined $iorder);
        ++$iorder;

        my $asth = $gCfg{dbh}->prepare("INSERT INTO agenda ( number, global_count, iorder ) VALUES ( ?, ?, ? )")
            or die "Could not execute DBD::SQLite command";

        # timestamp and file fields are formatted to enable sorting numerically
        # we check the old copy of the agenda first, to make sure we aren't in it
        $sth = $gCfg{dbh}->prepare("SELECT * FROM history WHERE deleted = 0 "
                                   . " AND global_count NOT IN (SELECT global_count FROM agenda)"
                                   . " ORDER BY tstamp, file, version")
            or die "Could not execute DBD::SQLite command";

        $sth->execute
            or die "Could not execute DBD::SQLite command";

        while ($row = $sth->fetchrow_hashref) {
            $row->{comment} = ''
                if (!exists $row->{comment} || !defined $row->{comment});

            if (defined $prevRow->{tstamp}
                && ($row->{tstamp} < ($prevRow->{tstamp} + 43200)) # 12 hour window
                && ($row->{user} eq $prevRow->{user})
                && ($row->{comment} eq $prevRow->{comment})
                && (!defined $seenThisRev->{$row->{file}})) {
                # date, user and comment are same; this will be multi-item commit
            } else {
                ++$tmpCommitNumber;
                $seenThisRev = {};
            }
            
            $seenThisRev->{$row->{file}} = 1;
            $asth->execute($tmpCommitNumber, $row->{global_count}, $iorder++)
                or die "Could not execute DBD::SQLite command";
            $prevRow = $row;
        }
        # finished scheduling
        $asth->execute(0, 0, 0) or die "Could not execute DBD::SQLite command";
        $asth->finish;
        $sth->finish;
        PrintMsg "done -- $tmpCommitNumber revisions to commit\n";
    }
}
        
###############################################################################
#  GetScheduleCount
###############################################################################
sub GetScheduleCount {
    my $sth = $gCfg{dbh}->prepare("SELECT MAX(number) FROM agenda")
        or die "Could not execute DBD::SQLite command";
    
    $sth->execute
        or die "Could not execute DBD::SQLite command";

    my ($tmpCount, $count);
    $sth->bind_columns(\$tmpCount);
    while ($sth->fetch()) {
        $count = $tmpCount;
    }
    $sth->finish();
    return $count;
}

###############################################################################
#  GetIorder
###############################################################################
sub GetIorder {
    my $sth = $gCfg{dbh}->prepare("SELECT MAX(iorder) FROM agenda")
        or die "Could not execute DBD::SQLite command";
    
    $sth->execute
        or die "Could not execute DBD::SQLite command";

    my ($tmpCount, $count);
    $sth->bind_columns(\$tmpCount);
    while ($sth->fetch()) {
        $count = $tmpCount;
    }
    $sth->finish();
    return $count;
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

    PrintMsg "\n\n**** MIGRATING VSS HISTORY TO SUBVERSION ****\n\n";

    # a unique record
    my $xsth = $gCfg{dbh}->prepare("SELECT * FROM history WHERE global_count = ?") 
        or die "Could not execute DBD::SQLite command";

    # use checkpoint as next stage of when to add/update files for subversion
    my $qckpsth = $gCfg{dbh}->prepare("SELECT checked FROM checkpoint WHERE file = ?")
        or die "Could not execute DBD::SQLite command";

    # use to set added/add commited states
    my $donesth = $gCfg{dbh}->prepare("UPDATE checkpoint SET checked = ? WHERE file = ?")
        or die "Could not execute DBD::SQLite command";

    # add a commitpoint for a number
    my $acpsth = $gCfg{dbh}->prepare("INSERT INTO commitpoint (number, revision, stage) VALUES (?, -1, 0)")
        or die "Could not execute DBD::SQLite command";

    # query for commitpoint
    my $qcpsth = $gCfg{dbh}->prepare("SELECT stage, revision FROM commitpoint WHERE number = ?")
        or die "Could not execute DBD::SQLite command";

    # set the commitpoint to stage 1 for a number
    my $s1cpsth = $gCfg{dbh}->prepare("UPDATE commitpoint SET stage = 1, revision = ? WHERE number = ?")
        or die "Could not execute DBD::SQLite command";

    # set the commitpoint to stage 2 for a number
    my $s2cpsth = $gCfg{dbh}->prepare("UPDATE commitpoint SET stage = 2 WHERE number = ?")
        or die "Could not execute DBD::SQLite command";

    # set the commitpoint to stage 3 for a number
    my $s3cpsth = $gCfg{dbh}->prepare("UPDATE commitpoint SET stage = 3 WHERE number = ?")
        or die "Could not execute DBD::SQLite command";

    # set file retrieval state
    my $fretsth = $gCfg{dbh}->prepare("UPDATE history SET retrieved = 1 WHERE global_count = ?")
        or die "Could not execute DBD::SQLite command";

    &ScheduleCommits;
    my $tmpCommitNumber = &GetScheduleCount;

    my $agendasth = $gCfg{dbh}->prepare("SELECT global_count FROM agenda WHERE number = ? ORDER BY iorder DESC")
        or die "Could not execute DBD::SQLite command";

    my $gCiCountsth = $gCfg{dbh}->prepare("UPDATE ciCount SET number = ?")
        or die "Could not execute DBD::SQLite command";

    # walk down the agenda, getting from VSS and commiting
    for (my $rev = $gCfg{globalCiCount}+1; $rev <= $tmpCommitNumber; $rev++, $gCfg{globalCiCount}++) {
        warn "Revision $rev now being committed";

        # checkout all the files at this point in the agenda from VSS
        my $agendaitem;
        my $agendarow;
        my $agendalist = [];
        my $committedFiles = {};
        my $addedSvnFiles = [];
        my $committedDosFiles = [];
        my $comment = "";
        my $tstamp = 0;
        my $vssname = "";
        my $fileCount = 0;

        # build the list of file snapshots for this revision
        $agendasth->execute($rev) 
            or die "Could not execute DBD::SQLite command";
        while ($agendarow = $agendasth->fetchrow_hashref) {
            push @{$agendalist}, $agendarow->{global_count};
        }

      AGENDAITEM:
        while (defined ($agendaitem = pop @{$agendalist})) {
            my $tmpNum = $agendaitem + 0;
            warn "global_count item $tmpNum being fetched";

            $xsth->execute($tmpNum) 
                or die "Could not execute DBD::SQLite command";

            my $tmpRow = {};
            while ($row = $xsth->fetchrow_hashref) {
                # make a copy
                while ( my ($key, $value) = each(%{$row})) {
                    $tmpRow->{$key} = $value . '';
                }

                # print a localtime formatted version, so at least the user can check
                # versus the VSS entries
                my ($min,$hour,$mday,$mon,$year);
                (undef,$min,$hour,$mday,$mon,$year) = localtime($tmpRow->{tstamp});
                ++$mon;
                $year += 1900;
                $tmpRow->{localdatetime} = sprintf("%4.4i-%2.2i-%2.2i %2.2i:%2.2i", $year, $mon, $mday, $hour, $min);
                $tmpRow->{comment} = ''
                    if (!exists $row->{comment} || !defined $row->{comment});
            }
            $row = $tmpRow;

            # we could be here because there were no directories to add or something
            next AGENDAITEM unless (defined $row->{file});

            # see if we have been added to wc or repository
            $qckpsth->execute($row->{file}) or die "Could not execute DBD::SQLite command";
            
            $upd = 0;
            while (my $rowx = $qckpsth->fetchrow_hashref()) {
                $upd = $rowx->{checked} - 1; # added history to db, but not to wc or repo
            }

            $commitinfo = &GetVssRevision($row, $upd, $rev, $qcpsth, $fretsth, $committedDosFiles, $addedSvnFiles);
            $committedFiles->{$row->{file}} = 1;
            my $shouldskip = (defined $commitinfo && defined $commitinfo->{skipped});
            my $skipMsg = $shouldskip ? "skipped" : "";
            PrintMsg "   ($rev)File $row->{file}, $row->{localdatetime}..." . $skipMsg . "\n";
            $comment = $row->{comment};
            $comment .= "\n\n$gCfg{comment}" if defined $gCfg{comment};
            $tstamp = $row->{tstamp};
            $vssname = lc($row->{user});
            ++$fileCount;

            # roll these skipped files back
            if ($shouldskip) {
                --$fileCount;
                undef $committedFiles->{$row->{file}};
                undef $commitinfo;
            }
        }

        # commit the files
        if (scalar @{$committedDosFiles} > 0) {
            &AddSvnFiles($addedSvnFiles, $donesth);
            &CommitSvn($committedDosFiles, $comment, $rev, $qcpsth, $acpsth, $s1cpsth);
        }

        # now we are really at "revision $rev"
        # adjust the date/time, author
        if ($fileCount > 0) {
            foreach my $k (keys %{$committedFiles}) {
                $donesth->execute(3, $k) or die "Could not execute DBD::SQLite command";
            }
            
            if ($gCfg{setrevprop}) {
                &SetSvnDates($tstamp, $rev, $qcpsth, $s2cpsth);
                &SetSvnAuthor($vssname, $rev, $qcpsth, $s3cpsth);
            }
        }

        # we are finished with this revision
        $gCiCountsth->execute($rev) or die "Could not execute DBD::SQLite command";

        # clear the pool
        my $pool = $SVNClient->pool();
        $pool->clear();
    }
    $xsth->finish;
    $acpsth->finish;
    $qcpsth->finish;
    $s1cpsth->finish;
    $s2cpsth->finish;
    $s3cpsth->finish;
    $qckpsth->finish;
    $donesth->finish;
    $fretsth->finish;
    $agendasth->finish;
    $gCiCountsth->finish;
}

###############################################################################
#  GetVssRevision
###############################################################################
sub GetVssRevision {
    my($row, $upd, $commitNum, $qcpsth, $fretsth, $committedDosFiles, $addedSvnFiles) = @_;

    $qcpsth->execute($commitNum) or die "Could not execute DBD::SQLite command";

    my $rc = 0;
    my $stage = 0;
    while (my $row = $qcpsth->fetchrow_hashref()) {
        $rc++;
        $stage = $row->{stage};
    }

    # we have definitely checked this in, skip it
    if ($rc == 1 && $stage >= 1) {
        warn "GetVssRevision:  $row->{file} checked in\n";
        return undef;
    }

    # Gets a version of a file from VSS and adds it to SVN
    # $row is the row hash ref from the history SQLite table
    # $upd is true if this is an update rather than add

    my $vsspath = $row->{file};
    my $realpath = $vsspath;

    if (defined $SUBSTMAP->{$vsspath}) {
        $vsspath = $SUBSTMAP->{$vsspath};
    }

    $realpath =~ m/^(.*\/)(.*)/
        or die "Mangled VSS file path information", join("\n", %$row);
    my($path, $file) = ($1, $2);

    $path =~ s/$gCfg{vssprojmatch}//
        or die "Mangled VSS file path information", join("\n", %$row);
    $path =~ s/\/$//; # remove trailing slash

    (my $dospath = "$gCfg{workdir}/$path") =~ s/\//\\/g; # use backslashes
    $dospath =~ s/\\$//; # remove trailing backslash if $path was empty
    $dospath =~ s/\\\\/\\/g; # replace double backslashes with single

    my $cmd = "GET -GTM -W -GL\"$dospath\" -V$row->{version} \"$vsspath\"";

    # get it if we haven't already gotten it yet or it's not there
    my $cofile = $dospath . "\\" . $file;
    if (!(-e $cofile) || $row->{retrieved} == 0) {
        if (-e $cofile) {
            unlink $cofile; # delete it
        }
        $VSS->ss($cmd)
            or die "Could not issue ss.exe command";
        if (-e $cofile) {
            $fretsth->execute($row->{global_count}) or die "Could not execute DBD::SQLite command";
        } else {
            if ($VSS->{ss_output} =~ m/^(File|Project) \S+.* does not retain old versions of itself/) {
                # only the latest version of this file is being stored, just skip it
                warn "GetVssRevision:  $row->{file}, version $row->{version} is only stored in latest version\n";
                return { skipped => 1 };
            } else {
                die "ss.exe failed to retrieve $cofile";
            }
        }           
    }

    if (!$upd) {
        my $tmpfile = "$path/$file";
        $tmpfile =~ s/^\///; # remove leading slash
        unshift @{$addedSvnFiles}, {file => $tmpfile, path => $realpath};
    }

    my $commitinfo =
    { file => $file,
      user => $row->{user},
      dospath => $dospath,};

    unshift @{$committedDosFiles}, Encode::encode('utf8', $commitinfo->{dospath} . "\\" . $commitinfo->{file});

    return $commitinfo;
}

###############################################################################
#  AddSvnFiles
###############################################################################
sub AddSvnFiles {
    my($addedSvnFiles, $donesth) = @_;

    foreach my $file (@{$addedSvnFiles}) {
        $SVNClient->add(Encode::encode('utf8', $file->{file}), 0);
        $donesth->execute(2, $file->{path}) or die "Could not execute DBD::SQLite command";
    } 
}

###############################################################################
#  CommitSvn
###############################################################################
sub CommitSvn {
    my($committedDosFiles, $comment, $commitNum, $qcpsth, $acpsth, $s1cpsth) = @_;

    $qcpsth->execute($commitNum) or die "Could not execute DBD::SQLite command";

    my $rc = 0;
    my $stage = 0;
    my $revpoint = -1;
    while (my $row = $qcpsth->fetchrow_hashref()) {
        $rc++;
        $stage = $row->{stage};
        $revpoint = $row->{revision};
    }

    if ($rc == 1 && $stage >= 1) {
        # history completed without a hitch
        # skip it
        PrintMsg "   (ALREADY COMMITTED)\n";
    } elsif (($rc == 0 || $rc == 1) && $stage < 1) {
        $COMMENT = Encode::encode('utf8', $comment);
        
        PrintMsg $rc ? "   (RESTARTING COMMIT)\n" : "   (COMMITTING SVN...)\n";

        # add the commitpoint
        if ($rc == 0) {
            $acpsth->execute($commitNum)
                or die "Could not execute DBD::SQLite command";
            warn $acpsth->dump_results;
        }

        $SVNClient->log_msg(\&log_comments);
        my ($commit_val) = $SVNClient->commit($committedDosFiles, 1);

        my $ciRevision;
        if (!defined $commit_val || $commit_val->revision() == $SVN::Core::INVALID_REVNUM) {
            $ciRevision = GetSvnInfo(@{$committedDosFiles}[0]);
            if (!defined $ciRevision) {
                die "GetSvnInfo failed, bailing";
            }
        } else {
            $ciRevision = $commit_val->revision();
        }

        # set the commitpoint to stage 1
        $s1cpsth->execute($ciRevision, $commitNum)
            or die "Could not execute DBD::SQLite command";
        warn $s1cpsth->dump_results;
    } else {
        # error
        die "Illegal commitpoint value $rc:$stage";
    }

}  #End CommitSvn

###############################################################################
#  GetSvnInfo
#    return the revision number for a file in the working copy
###############################################################################
sub GetSvnInfo {
    my($target) = @_;
    my $ret;
    warn "GetSvnInfo called";
    # this code only needs to exist until SVN::Client->info gets implemented
    undef $REVISION;
    undef $ret;
    $target =~ s/\\\\/\\/g; # replace double backslashes with single
    $target =~ s/\\/\//g; # replace singles with forward slash
    $SVNClient->log(Encode::encode('utf8', $target), 'BASE', 'BASE', 0, 0, \&get_version);
    if (defined $REVISION) {
        $ret = $REVISION;
    }
    return $ret;
}

# callback for GetSvnInfo
sub get_version {
    my ($changed_paths,$revision,$author,$date,$message,$pool) = @_;
    if (!defined $REVISION) {
        $REVISION = $revision;
    }
}

###############################################################################
#  SetSvnDates
###############################################################################
sub SetSvnDates {
    my($tstamp, $commitNum, $qcpsth, $s2cpsth) = @_;

    my $propRev = -1;

    # see if we have checkpointed this date
    $qcpsth->execute($commitNum) or die "Could not execute DBD::SQLite command";

    my $rc = 0;
    my $stage = 0;
    while (my $row = $qcpsth->fetchrow_hashref()) {
        $rc++;
        $stage = $row->{stage};
        $propRev = $row->{revision};
    }

    if ($rc == 1 && $stage >= 2) {
        warn "Date already set for revision $propRev, skipping";
    } elsif ($rc == 1 && $stage == 1) {
        my ($min,$hour,$mday,$mon,$year);
        (undef,$min,$hour,$mday,$mon,$year) = gmtime($tstamp);
        ++$mon;
        $year += 1900;
        my $svn_date = sprintf("%4.4i-%2.2i-%2.2iT%2.2i:%2.2i:00.000000Z", $year, $mon, $mday, $hour, $min);
        my $encDate =  Encode::encode('utf8', $svn_date); # probably unnecessary

        $SVNClient->revprop_set('svn:date', $encDate, $gCfg{svnrepo}, $propRev, 0);

        # set the commitpoint to stage 2
        $s2cpsth->execute($commitNum)
            or die "Could not execute DBD::SQLite command";
        warn $s2cpsth->dump_results;

    } else {
        die "Wrong state $rc:$stage in SetSvnDates";
    }

}  #End SetSvnDates

###############################################################################
#  SetSvnAuthor
###############################################################################
sub SetSvnAuthor {
    my($author, $commitNum, $qcpsth, $s3cpsth) = @_;

    my $propRev = -1;

    # see if we have checkpointed this author
    $qcpsth->execute($commitNum) or die "Could not execute DBD::SQLite command";

    my $rc = 0;
    my $stage = 0;
    while (my $row = $qcpsth->fetchrow_hashref()) {
        $rc++;
        $stage = $row->{stage};
        $propRev = $row->{revision};
    }

    if ($rc == 1 && $stage == 3) {
        warn "Author already set for revision $propRev, skipping";
    } elsif ($rc == 1 && ($stage == 1 || $stage == 2)) {
        my $encAuthor =  Encode::encode('utf8', $author);
        
        $SVNClient->revprop_set('svn:author', $encAuthor, $gCfg{svnrepo}, $propRev, 0);

        # set the commitpoint to stage 3
        $s3cpsth->execute($commitNum)
            or die "Could not execute DBD::SQLite command";
        warn $s3cpsth->dump_results;

    } else {
        die "Wrong state $rc:$stage in SetSvnAuthor";
    }

}  #End SetSvnAuthor

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
               'vsslogin=s','norevprop','noprompt','nolabel','timebias=i','dstbias=i',
               'iconv=s','restart','svnlogin=s','snaptime=s','snaplabel=s','substfile=s',
               'update:s','debug','help',);

    &GiveHelp(undef, 1) if defined $gCfg{help};

    defined $gCfg{vssproject} or GiveHelp("must specify --vssproject\n");
    defined $gCfg{svnrepo} or GiveHelp("must specify --svnrepo\n");
    defined $ENV{SSDIR} or GiveHelp("\$SSDIR not defined\n");

    my $url = URI::URL->new($gCfg{svnrepo});
    $gCfg{svnrepo} = $url->as_string;
    
    GiveHelp("VSS project must start with '\$/'") unless $gCfg{vssproject} =~ m:^\$\/:; 
    $gCfg{vssproject} =~ s:\\:/:g; # flip all '\\' to '/'
    $gCfg{vssproject} =~ s:/+:/:g; # replace all '//' with '/'

    $gCfg{vssproject} =~ s:(\$/.*)/$:$1:;
    $gCfg{vssprojmatch} = quotemeta( $gCfg{vssproject} );

    @{ $gCfg{vssexclude} } = split(',', join(',' ,@{ $gCfg{vssexclude} } ))
        if defined $gCfg{vssexclude};

    my $vss_args = {
        interactive => 'Y',
        timebias    => $gCfg{timebias},
        dstbias     => $gCfg{dstbias},
    };

    # seting dates/authors is the default, unless --norevprop is entered
    $gCfg{setrevprop} = !defined $gCfg{norevprop};

    if (defined $gCfg{vsslogin}) {
        @{ $vss_args }{'user', 'passwd'} = split(':', $gCfg{vsslogin});
        warn "\nATTENTION: about to issue VSS login command; if program\n"
            . "hangs here, you have specified an invalid VSS username\n"
            . "or password. (Press CTRL+Break to kill hung script)\n\n";
    }

    $VSS = Vss2Svn::VSS->new($ENV{SSDIR}, $gCfg{vssproject}, $vss_args);
    $VSS->{_debug} = 1;

    my %svnClientOpts = ();
    if (defined $gCfg{svnlogin}) {
        $svnClientOpts{auth} = [SVN::Client::get_simple_prompt_provider(\&simple_prompt, 0)];
    }

    $SVNClient = new SVN::Client(%svnClientOpts);

    $gCfg{globalCiCount} = -1;
    $gCfg{globalCount} = 1;

    $SUBSTMAP = {};
    if (defined $gCfg{substfile}) {
        open(SUBSTFILE, $gCfg{substfile})
            or die "Could not open substitution file: $gCfg{substfile}";
        my @subst_data=<SUBSTFILE>;
        close SUBSTFILE;

        foreach my $substline (@subst_data) {
            chop($substline);
            my ($fname, $subst) = split(/:/, $substline);
            $SUBSTMAP->{$fname} = $subst;
        }
    }

    # users should define when they want to update so it can be restarted if necessary
    if (defined $gCfg{update} && (!defined $gCfg{snaptime} && !defined $gCfg{snaplabel})) {
        die "--update must be used with either --snaptime or --snaplabel";
    } elsif (defined $gCfg{update}) {
        if ($gCfg{update} eq "") {
            $gCfg{update} = "fast";
        }
        if (!($gCfg{update} eq "fast" || $gCfg{update} eq "complete")) {
            die "--update must be one of [fast|complete]";
        }
        $gCfg{restart} = 1;     
    }

    # check --snaptime and --snaplabel
    if (defined $gCfg{snaptime} && defined $gCfg{snaplabel}) {
        die "--snaptime and --snaplabel are mutually exclusive";
    } elsif (defined $gCfg{snaplabel}) {
        # nothing special to do here
    } elsif (defined $gCfg{snaptime}) {
        # get the time
        my $tmpMatch = $gCfg{snaptime};
        $tmpMatch =~ m/(\d\d\d\d)-(\d\d)-(\d\d)T(\d\d):(\d\d)(Z?)/
            or die "Mangled date/time: '" . $gCfg{snaptime} . "'"; 
        my ($year, $month, $date, $hour, $minute, $zulu) = ($1, $2, $3, $4, $5, $6);
        my $datestring = sprintf("%04d/%02d/%02d %02d:%02d",
                                 $year, $month, $date, $hour, $minute);
        my %options = (NO_RELATIVE => 1);
        $options{GMT} = 1 if (defined $zulu && $zulu eq "Z");
        
        $SNAPTIME = parsedate($datestring, %options);
        if (!defined $SNAPTIME) {
            die "Mangled date/time: '" . $gCfg{snaptime} . "'";
        }
    } else {
        # --snaptime defaults to now
        $SNAPTIME = time();
    }

    $gCfg{workbase} = cwd() . "/_vss2svn";

    if (!defined $gCfg{restart}) {
        print "\nCleaning up any previous vss2svn runs...\n\n";
        File::Path::rmtree($gCfg{workbase}, 0, 0);
        mkdir $gCfg{workbase} or die "Couldn't create $gCfg{workbase} (does "
            . "another program have a lock on this directory or its files?)";
        
        $gCfg{workdir} = "$gCfg{workbase}/work";
        mkdir $gCfg{workdir} or die "Couldn't create $gCfg{workdir}";
        
        $gCfg{tmpfiledir} = "$gCfg{workbase}/tmpfile";
        mkdir $gCfg{tmpfiledir} or die "Couldn't create $gCfg{tmpfiledir}";
        
        $gCfg{dbdir} = "$gCfg{workbase}/db";
        mkdir $gCfg{dbdir} or die "Couldn't create $gCfg{dbdir}";
    } else {
        $gCfg{workdir} = "$gCfg{workbase}/work";
        $gCfg{tmpfiledir} = "$gCfg{workbase}/tmpfile";
        $gCfg{dbdir} = "$gCfg{workbase}/db";
    }
    $VSS->{use_tempfiles} = "$gCfg{tmpfiledir}";
}

# callback for authentication
sub simple_prompt {
    my ($cred,$realm,$default_username,$may_save,$pool) = @_;

    my ($svnuser,$svnpw) = split(/:/, $gCfg{svnlogin}, -1);
    $cred->username($svnuser);
    $cred->password($svnpw);
}

# callback for comments
sub log_comments {
    my ($msg,$tmpFile,$commit_ary,$pool) = @_;
    $$msg = $COMMENT;
}

# callback for notifications
sub wc_changed {
    my ($path,$type,$node,$mimeType,$state,$revision) = @_;
#   warn "wc_changed: " . $path . " " . $type . " " . $node . " " . $mimeType . " " . $state . " " . $revision;
    if ($type == $SVN::Wc::Notify::Action::add 
        || $type == $SVN::Wc::Notify::Action::copy
        || $type == $SVN::Wc::Notify::Action::delete
        || $state == $SVN::Wc::Notify::State::changed) {
        $STATE_CHANGED = 1;
    }
}

###############################################################################
#  Regionalize
###############################################################################
sub Regionalize {
    my $bias = $Registry->{'HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/'
                               .'Control/TimeZoneInformation/ActiveTimeBias'} || 0;

    my $dstbias = $Registry->{'HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/'
                                  . 'Control/TimeZoneInformation/DaylightBias'} || 0;

    {
        use integer; # forces Perl to interpret two's-complement correctly
        $gCfg{timebias} = hex($bias) + 0;
        $gCfg{dstbias} = hex($dstbias) + 0;
    }

    # close the keys
    undef $bias;
    undef $dstbias;
}

###############################################################################
#  CreateDatabase
###############################################################################
sub CreateDatabase {
    $gCfg{dbh} = DBI->connect("dbi:SQLite(RaiseError=>1,AutoCommit=>1)"
                              . ":dbname=$gCfg{dbdir}/vss2svn.db","","");

    if (!defined $gCfg{restart}) {
        PrintMsg "\n\n**** Creating database... ****\n\n";

        my $hist =  "CREATE TABLE history("
            . "tstamp    long NOT NULL,"
            . "file    varchar(1024)  NOT NULL,"
            . "version long           NOT NULL,"
            . "user    varchar(256)   NOT NULL,"
            . "comment blob           NOT NULL,"
            . "global_count    long   NOT NULL,"
            . "retrieved integer      NOT NULL,"
            . "deleted integer      NOT NULL)";
		my $tree = "CREATE TABLE treetable("
			. "tag VARCHAR(8) NOT NULL,"
			. "type VARCHAR(8) NOT NULL,"
			. "file    varchar(1024)  NOT NULL,"
			. "version long           NOT NULL)";
		my $agenda = "CREATE TABLE agenda("
			. "number long NOT NULL,"
			. "global_count long NOT NULL,"
			. "iorder integer NOT NULL)";
		my $checkpt = "CREATE TABLE checkpoint("
			. "file varchar(1024) NOT NULL PRIMARY KEY,"
			. "checked integer NOT NULL)";
		my $commitpt = "CREATE TABLE commitpoint("
			. "number integer NOT NULL PRIMARY KEY,"
			. "revision integer NOT NULL,"
			. "stage integer NOT NULL)";
		my $svnaction = "CREATE TABLE svnaction("
			. "type VARCHAR(8) NOT NULL,"
			. "file varchar(1024) NOT NULL,"
			. "action VARCHAR(8) NOT NULL,"
			. "global_count long NOT NULL,"
			. "args blob)";

		# fire up a transaction to make all the database tables
		# and to set the persistent checkin count

        my $ac = $gCfg{dbh}->{AutoCommit};
        my $re = $gCfg{dbh}->{RaiseError};

        $gCfg{dbh}->{AutoCommit} = 0;
        $gCfg{dbh}->{RaiseError} = 1;
		
        eval {
			$gCfg{dbh}->do($hist);
			$gCfg{dbh}->do($tree);

			$gCfg{dbh}->do($agenda);
			$gCfg{dbh}->do($checkpt);

			$gCfg{dbh}->do($commitpt);
			$gCfg{dbh}->do($svnaction);

			$gCfg{dbh}->do("CREATE TABLE ciCount(number integer NOT NULL)");
			$gCfg{dbh}->do("INSERT INTO ciCount (number) VALUES (-1)");

            $gCfg{dbh}->commit;
		};
        if ($@) {
            PrintMsg "Transaction aborted because $@";
            eval { $gCfg{dbh}->rollback };
            die "Transaction failed!";
        }      
        $gCfg{dbh}->{AutoCommit} = $ac;
        $gCfg{dbh}->{RaiseError} = $re;
    } else {
		# read the persistent values from the database
        &ResetGlobalCiCountFromDatabase;
        &ResetGlobalCountFromDatabase;
    }
}  #End CreateDatabase

###############################################################################
#  DumpDatabase
###############################################################################
sub DumpDatabase {

    my $sth = $gCfg{dbh}->prepare("SELECT * FROM history")
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";

    PrintMsg "HISTORY:\n";
    while (my $row = $sth->fetchrow_hashref) {
        PrintMsg "tstamp: " . $row->{tstamp}
        . " file: " . $row->{file} . " version: " . $row->{version} 
        . " user: " . $row->{user} . " comment: " . $row->{comment}
        . " global_count: " . $row->{global_count} . " retrieved: " . $row->{retrieved}
        . " deleted: " . $row->{deleted} . "\n";
    }
    $sth->finish();

    $sth =  $gCfg{dbh}->prepare("SELECT * FROM treetable")
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";

    PrintMsg "TREETABLE:\n";
    while (my $row = $sth->fetchrow_hashref) {
        PrintMsg "tag: " . $row->{tag} . " type: " . $row->{type} . " file: " . $row->{file} . " version: " . $row->{version} . "\n";
    }
    $sth->finish();

    $sth =  $gCfg{dbh}->prepare("SELECT * FROM agenda")
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";

    PrintMsg "AGENDA:\n";
    while (my $row = $sth->fetchrow_hashref) {
        PrintMsg "number: " . $row->{number} . " global_count: " . $row->{global_count} . " iorder: " . $row->{iorder} . "\n";
    }
    $sth->finish();

    $sth =  $gCfg{dbh}->prepare("SELECT * FROM checkpoint")
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";

    PrintMsg "CHECKPOINT:\n";
    while (my $row = $sth->fetchrow_hashref) {
        PrintMsg "file: " . $row->{file} . " checked: " . $row->{checked} . "\n";
    }
    $sth->finish();

    $sth =  $gCfg{dbh}->prepare("SELECT * FROM commitpoint")
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";

    PrintMsg "COMMITPOINT:\n";
    while (my $row = $sth->fetchrow_hashref) {
        PrintMsg "number: " . $row->{number} . " revision: " . $row->{revision}
        . " stage: " . $row->{stage} . "\n";
    }
    $sth->finish();

    PrintMsg "CICOUNT:\n";
    $sth =  $gCfg{dbh}->prepare("SELECT * FROM ciCount");
    while (my $row = $sth->fetchrow_hashref) {
        PrintMsg "number: " . $row->{number} . "\n";
    }
    $sth->finish();

    $sth =  $gCfg{dbh}->prepare("SELECT * FROM svnaction")
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";

    PrintMsg "SVNACTION:\n";
    while (my $row = $sth->fetchrow_hashref) {
        PrintMsg "file: " . $row->{file} . " type: " . $row->{type} . " action: " . $row->{action}
		. " global_count: " . $row->{global_count} . "\n";
    }
    $sth->finish();

}  #End DumpDatabase

###############################################################################
#  ResetGlobalCountFromDatabase
###############################################################################
sub ResetGlobalCountFromDatabase {
    # reset the global_count to the correct value from the database
    my $cmd = "SELECT MAX(global_count) FROM history";
    my $sth = $gCfg{dbh}->prepare($cmd)
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";
    
    my $tmpCount;
    $sth->bind_columns(\$tmpCount);
    while ($sth->fetch()) {
        $gCfg{globalCount} = $tmpCount;
    }
    $sth->finish();
}  #End 

###############################################################################
#  ResetGlobalCiCountFromDatabase
###############################################################################
sub ResetGlobalCiCountFromDatabase {
    # reset the globalCiCount to the correct value from the database
    my $cmd = "SELECT number FROM ciCount";
    my $sth = $gCfg{dbh}->prepare($cmd)
        or die "Could not execute DBD::SQLite command";
    $sth->execute
        or die "Could not execute DBD::SQLite command";
    
    my $tmpCount;
    $sth->bind_columns(\$tmpCount);
    while ($sth->fetch()) {
        $gCfg{globalCiCount} = $tmpCount;
    }
    $sth->finish();
}  #End 

###############################################################################
#  CloseDatabase
###############################################################################
sub CloseDatabase {
    $gCfg{dbh}->disconnect;
}  #End CloseDatabase

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




###############################################################################
#                           package Vss2Svn                                   #
###############################################################################

package Vss2Svn;

require 5.005_62;
use strict;
use warnings;

use File::Path;
use File::Copy;
use Text::Wrap;

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

###############################################################################
#  check_for_exe
###############################################################################
sub check_for_exe {
    my($self, $exe, $desc) = @_;

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

    croak "$msg\n$ENV{PATH}";
}  # End check_for_exe


1;

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
        croak "Project path must be absolute (begin with \$/)";
    }

    $project = first {defined} $project, '\$/';
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
            use_tempfiles        => 0,
            timebias             => 0,
            dstbias              => 0,
            _tempdir             => undef,
            _debug               => 0,
            _whoami              => undef,
            %$args,
        }, $class;

    $self->{_ssexe} = $self->check_for_exe("ss.exe",
        "the Microsoft Visual SourceSafe client");

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

    # used in Daylight Savings Time offset calculations
    $self->{_is_dst_now} = (localtime)[8];

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
#  format_time_version
###############################################################################
sub format_time_version {
    my ($self, $time) = @_;
    my ($minute, $hour, $date, $month, $year, $meridian);
    (undef, $minute, $hour, $date, $month, $year) = localtime($time);
    $month++;
    $year %= 100;
    $meridian = ($hour < 12) ? "a" : "p";
    $hour = (($hour % 12) == 0) ? 12 : ($hour % 12);
    my $ret = sprintf("%02d/%02d/%02d;%d:%02d%s", $month, $date, $year, $hour,
                      $minute, $meridian);
    return $ret;
}

###############################################################################
#  compute_biases
###############################################################################
sub compute_biases {
    my ($self, $basis) = @_;

    if ($self->{timebias} != 0) {
        my $bias = $self->{timebias};
        my $was_dst_then = (localtime $basis)[8];
        
        if ($self->{_is_dst_now} && ! $was_dst_then) {
            $bias -= $self->{dstbias};
        } elsif (! $self->{_is_dst_now} && $was_dst_then) {
            $bias += $self->{dstbias};
        }
        
        # add '+' to front so parsedate adds # of minutes
        $bias =~ s/^(\d+)/+ $1/;
        $basis = parsedate("$bias minutes", NOW => $basis);
    }
    return $basis;
}

###############################################################################
#  format_version_string
###############################################################################
sub format_version_string {
    my($self, %versionHash) = @_;
    my $cmd = "";

    if (defined $versionHash{"endlabel"}) { 
        $cmd .= ' "-VL' . $versionHash{"endlabel"};
        if (defined $versionHash{"beginlabel"}) {
            $cmd .= '~' . $versionHash{"beginlabel"};
        }
        $cmd .= '"';
    } elsif (defined $versionHash{"endtime"}) {
        my $ld = $self->format_time_version($versionHash{"endtime"}); 
        $cmd .= ' -Vd' . $ld;
        if (defined $versionHash{"begintime"}) {
            my $bd = $self->format_time_version($versionHash{"begintime"}); 
            $cmd .= '~' . $bd;
        }
    } elsif (defined $versionHash{"endversion"}) {
        $cmd .= ' -V' . $versionHash{"endversion"};
        if (defined $versionHash{"beginversion"}) {
            $cmd .= '~' . $versionHash{"beginversion"};
        }
    }
    return $cmd;
}

###############################################################################
#  project_tree
###############################################################################
sub project_tree {
    my($self, $project, $recursive, $remove_dev, %versionHash) = @_;

    # returns a nested-hash "tree" of all subprojects and files below the given
    # project; the "leaves" of regular files are the value "1" or the version number.
    # if one of the version options is defined

    $project = $self->full_path($project);
    $recursive = 1 unless defined $recursive;
    $remove_dev = 0 unless defined $remove_dev;

    if ($self->filetype($project) < 0) { # projects are type 0
        carp "project_tree(): '$project' is not a valid project";
        return undef;
    }

    my $cmd = "DIR \"$project\"";
    $cmd .= ($recursive)? ' -R' : ' -R-';
    $cmd .= $self->format_version_string(%versionHash); 

    # versions get pasted on the ends of projects and files
    my $versionPasting = (defined $versionHash{"endlabel"} || defined $versionHash{"endtime"} || defined $versionHash{"endversion"});

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
        if ($self->{_debug}) {
            warn "\nDEBUG:<$line>\n";
        }
        $line =~ s/\s+$//;

        if ($line eq '') {
            if ($seen_blank_line) {
                carp "project_tree(): an internal error has occured -- 1";
                return undef;
            }

            $seen_blank_line = 1;
            next LINE;
        } elsif ($line =~ m/^(File|Project) \S+.* has been destroyed, and cannot be rebuilt\.$/) {
            next LINE;
        } elsif ($line =~ m/^Continue anyway\?\(Y\/N\)Y$/) {
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
            $current_project =~ s/;\d+$// if $versionPasting;

            $branch_ref = \%tree;

            if ($current_project ne '') {
                # get a reference to the end branch of subprojects
                my @ssplit = split /\//, $current_project;
                my @localmap = map {$branch_ref = $branch_ref->{$_}} @ssplit;
                ($branch_ref) = reverse(@localmap);
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
            my $subproject = $1;
            $subproject =~ s/;\d+$// if $versionPasting;

            $branch_ref->{$subproject} = {} unless defined($branch_ref->{$subproject});
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
            my $version;
            $line =~ s/;(\d+)$// if $versionPasting;
            $version = $1 if $versionPasting;

            $branch_ref->{$line} = !(defined $version) ? 1 : $version;
        }

    }

    return \%tree;

}  # End project_tree

###############################################################################
#  file_history
###############################################################################
sub file_history {
    my($self, $file, $keepLabel, %versionHash) = @_;
    # returns an array ref of hash refs from earliest to most recent;
    # each hash has the following items:
    #    version: version (revision) number
    #    user   : name of user who committed change
    #    date   : date in YYYY-MM-DD format
    #    time   : time in HH:MM (24h) format
    #    comment: checkin comment
    #    tstamp : time in time_t format

    $file = $self->full_path($file);

    my $cmd = "HISTORY \"$file\"";
    my $tmpfile = '';

    $cmd .= $self->format_version_string(%versionHash); 

    $self->ss($cmd, -2) or return undef;

    my $hist = [];
    my $labeltext = {};

    my $last = 0; # what type was the last line read?
    # 0=start;1=version line;2=user/date/time;3="Created";
    # 4=comment, 5=label, 6=initial label before version

    my $last_version = -1;

    my$rev = {}; # hash of info for the lastent revision
    my($year,$month,$day,$hour,$min,$ampm,$comment,$version);

HISTLINE:
    foreach my $line (split "\n", $self->{ss_output}) {
        if ($self->{_debug}) {
            warn "\nDEBUG:($last)<$line>\n";
        }

        if ($last == 0) {
            $comment = '';
            if ($line =~ m/$gHistLineMatch{version}/) {

                if ($last_version == 0 ||
                    (($last_version != -1) && ($1 != ($last_version - 1)))) {

                    # each version should be one less than the last
                    print "file_history(): internal consistency failure";
                    return undef;
                }

                $last = 1;
                $rev->{version} = $version = $1;
                if (!defined $labeltext->{$1}) {
                    $labeltext->{$1} = [];
                }
            } elsif ($line =~ m/$gHistLineMatch{labelheader}/) {
                $last = 6;
                if (!defined $labeltext->{"UNKNOWN"}) {
                    $labeltext->{"UNKNOWN"} = [];
                }
                unshift @{$labeltext->{"UNKNOWN"}}, $line;
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

                    my $epoch_secs = $self->compute_biases($basis);

                    (undef,$min,$hour,$day,$month,$year)
                        = localtime($epoch_secs);

                    $month++;
                    $year += 1900; #no, not a Y2K bug; $year = 100 in 2000
                }

                my $tmpDate = sprintf("%4.4i-%2.2i-%2.2i", $year, $month, $day);
                $rev->{date} = $tmpDate;
                $rev->{time} = sprintf("%2.2i:%2.2i", $hour, $min);
                $tmpDate =~ s/-/\//g;
                $tmpDate .= " " . $rev->{time};
                $rev->{tstamp} = parsedate($tmpDate, (NO_RELATIVE => 1, GMT => 1, WHOLE => 1));
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
                $comment .= "\n";
                next HISTLINE;
            } elsif ($line =~ m/$gHistLineMatch{created}/) {
                $last = 3;
                $comment = $line;
                $comment .= "\n";
                next HISTLINE;
            } elsif ($line =~ m/^$/) {
                $last = 4;
            }

        }

        if ($last == 3) {
            if ($line =~ s/$gHistLineMatch{comment}//) {
                $last = 4;
                $comment = $line;
                $comment .= "\n";
                next HISTLINE;
            }
        }

        if ($last == 4) {
            if ($line =~ m/$gHistLineMatch{version}/) {
                $last = 1;
                $version = $1;
                if (!defined $labeltext->{$version}) {
                    $labeltext->{$version} = [];
                }

                $comment =~ s/\s+$//;
                $comment =~ s/^\s+//;
                $rev->{comment} = $comment;

                my $tmplabel;
                if ($keepLabel) {
                    while (defined ($tmplabel = pop @{$labeltext->{$version+1}})) {
                        $rev->{comment} .= "\n$tmplabel";
                    }
                }
                unshift @$hist, $rev;

                $rev = {};
                $rev->{version} = $version;
            } elsif ($line =~ m/$gHistLineMatch{labelheader}/) {
                $last = 5;
                if (!defined $labeltext->{$version-1}) {
                    $labeltext->{$version-1} = [];
                }
                unshift @{$labeltext->{$version-1}}, $line;
            } else {
                $comment .= $line;
                $comment .= "\n";
            }

            next HISTLINE;
        }

        if ($last == 5) {
            if ($line =~ m/$gHistLineMatch{version}/) {
                $last = 1;
                $version = $1;

                $comment =~ s/\s+$//;
                $comment =~ s/^\s+//;
                $rev->{comment} = $comment;

                my $tmplabel;
                if ($keepLabel) {
                    while (defined ($tmplabel = pop @{$labeltext->{$version+1}})) {
                        $rev->{comment} .= "\n$tmplabel";
                    }
                }
                unshift @$hist, $rev;

                $rev = {};
                $rev->{version} = $version;
            } else {
                unshift @{$labeltext->{$version-1}}, $line;
            }
        }

        if ($last == 6) {
            if ($line =~ m/$gHistLineMatch{version}/) {
                $last = 1;
                $version = $1;

                if (!defined $labeltext->{$version}) {
                    $labeltext->{$version} = [];
                }
                my $tmplabel;
                while (defined ($tmplabel = pop @{$labeltext->{"UNKNOWN"}})) {
                    unshift @{$labeltext->{$version}}, $tmplabel;
                }
                undef $labeltext->{"UNKNOWN"};

                $rev = {};
                $rev->{version} = $version;
            } else {
                unshift @{$labeltext->{"UNKNOWN"}}, $line;
            }
        }

    }

    $comment =~ s/\s+$//;
    $comment =~ s/^\s+//;
    $rev->{comment} = $comment;

    if ($last == 4 || $last == 3) {
        my $tmplabel;
        if ($keepLabel) {
            while (defined ($tmplabel = pop @{$labeltext->{$version}})) {
                $rev->{comment} .= "\n$tmplabel";
            }
        }
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

    # special cases
    return 0 if $file eq '\$/';
    return -1 unless $file =~ m:^\$\/:;

    $file =~ s:\\:/:g; # flip all '\\' to '/'

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

    # kill leading and trailing whitespace
    $file =~ s/^\s+//;
    $file =~ s/\s+$//;

    # append the project part, unless it's already a project
    $file = "$self->{project}/$file" unless $file =~ m/^\$/;
    $file =~ s:\\:/:g; # flip all '\\' to '/'
    $file =~ s:/+:/:g; # replace all '//' with '/'
    $file =~ s:(\$/.*)/$:$1:; # remove any trailing slashes

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

    $cmd = "\"$self->{_ssexe}\" $cmd" unless ($cmd =~ m/^ss(\.exe)?\s/i);

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
    $gCfg{dateFormat} = $dateFormat . "";

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
        created    => qr/^Created$/,
        labelheader    => qr/^\*+$/,
        label      => qr/^Label:/,
    );

    # patterns to match development files that project_tree will ignore
    @gDevPatterns = (
                        qr/\.vspscc$/,
                        qr/\.vssscc$/,
                        qr/^vssver\.scc$/,
                    );

    # close the keys
    undef $dateFormat;
    undef $dateSep;
    undef $timeSep;

}  # End Initialize

sub first(&@) {
    my $code = shift;
    &$code && return $_ for @_;
    return undef;
}

package main;

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

=item --vssexclude <EXCLUDE_PROJECTS>:

Exclude the given projects from the migration. To list multiple projects,
separate with commas or use multiple --vssexclude commands.

Each project can be given as an absolute path (beginning with $/) or
relative to --vssproject.

=item --comment "MESSAGE":

add MESSAGE to end of every migrated comment

=item --norevprop:

By default, vss2svn sets the "svn:date" and "svn:author" 
revision properties on all commits to reflect the original
VSS commit date and author so that the original commit dates/authors
(and not today's date/svnlogin) show up in your new repository.

Using this flag turns off the default behavior.

The default behavior requires the "pre-revprop-change" hook script to be set; see
L<http://svnbook.red-bean.com/en/1.1/svn-book.html#svn-ch-5-sect-2.1>.

See README.TXT for more information on setting "svn:date".

=item --vsslogin "USER:PASSWD":

Set VSS username and password, separated by a colon.
B<WARNING --> if the username/password combo you provide is
incorrect, this program will hang as ss.exe prompts you for
a username! (This is an unavoidable Microsoft bug).

=item --svnlogin "USER:PASSWD":

The username and password vss2svn uses to authenticate to the target subversion repository.

=item --timebias <OFFSET_MINUTES>:

This script will examine your current Windows regional settings
by default in order to determine the number of minutes it should
add to your local time to get to GMT (for example, if you are
in Eastern Daylight Time [-0400], this should be 240). Use this
argument to override this value only if times are converted
incorrectly.

=item --dstbias <OFFSET_MINUTES>:

This script will examine your current Windows regional settings
by default in order to determine the number of minutes it should
add to convert from a time during Daylight Savings Time to a time
during Standard Time. Use this argument to override this value
only if times are converted incorrectly. This will ordinarily be
-60 in regions that use DST and 0 otherwise.

=item --noprompt:

Don't prompt to confirm settings or to create usernames after
the first stage.

=item --nolabel:

Don't include labels in comments.

=item --restart:

Add this flag to the rest of the commandline arguments if the migration session dies.

=item --update [value]:

Implies --restart.  Refreshes the database since the oldest item in the database.
until the time specified by --snaptime or --snaplabel.  

The --update flag may take an optional argument, either "fast" or "complete", depending on how file history
is to be processed.  If none is given, it defaults to "fast".  

The behavior of "fast" is to only examine file history if a file's version number is not equal to the version
number that was stored in history on the last run.  That is, if a file has been rolled back, pinned, or modified.

However, there may be exceptions to this rule, such as when a file is removed and replaced with a file
with the same history depth.  The "complete" history examines the history of each and every file.

=item --substfile <file>:

Adds `FILE:SUBSTITUTION' pairs contained in the file to try and work around
files that have been checked in to VSS that cannot be checked out without
using wildcarding inside VSS.

=item --snaptime <time>:

Defaults to time(). Use this to specify when to capture the particular structure of the VSS project. 
Local time is specified in the format "yyyy-mm-ddThh:mm". GMT time is specified in the format "yyyy-mm-ddThh:mmZ".

Mutually exclusive of --snaplabel.

=item --snaplabel <label>:

Use this to specify a label to capture the particular structure of the VSS project. 

Mutually exclusive of --snaptime.

=item --debug:

Print all program output to screen as well as logfile.

=back

