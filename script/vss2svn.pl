#!/usr/bin/perl

use warnings;
use strict;

use Getopt::Long;
use DBI;
use DBD::SQLite2;
use XML::Simple;
use File::Find;
use File::Path;
use Time::CTime;
use Data::Dumper;

use lib '.';
use Vss2Svn::ActionHandler;
use Vss2Svn::DataCache;
use Vss2Svn::SvnRevHandler;
use Vss2svn::Dumpfile;

our(%gCfg, %gSth, @gErr, %gFh, $gSysOut, %gActionType, %gNameLookup, %gId);

our $VERSION = '0.10';

&Initialize;
&ConnectDatabase;

&SetupGlobals;
&ShowHeader;

&RunConversion;

&DisconnectDatabase;
&ShowSummary;

###############################################################################
#  RunConversion
###############################################################################
sub RunConversion {

    # store a hash of actions to take; allows restarting in case of failed
    # migration
    my %joblist =
        (
            INIT            => {handler => sub{ 1; },
                                next    => 'LOADVSSNAMES'},

            # Load the "real" names associated with the stored "short" names
            LOADVSSNAMES    => {handler => \&LoadVssNames,
                                next    => 'FINDDBFILES'},

            # Add a stub entry into the Physical table for each physical
            # file in the VSS DB
            FINDDBFILES     => {handler => \&FindPhysDbFiles,
                                next    => 'GETPHYSHIST'},

            # Load the history of what happened to the physical files. This
            # only gets us halfway there because we don't know what the real
            # filenames are yet
            GETPHYSHIST     => {handler => \&GetPhysVssHistory,
                                next    => 'MERGEPARENTDATA'},

            # Merge data from parent records into child records where possible
            MERGEPARENTDATA => {handler => \&MergeParentData,
                                next    => 'BUILDACTIONHIST'},

            # Take the history of physical actions and convert them to VSS
            # file actions
            BUILDACTIONHIST => {handler => \&BuildVssActionHistory,
                                next    => 'IMPORTSVN'},

            # Create a dumpfile or import to repository
            IMPORTSVN       => {handler => \&ImportToSvn,
                                next    => 'DONE'},
        );

    my $info;

    while ($gCfg{task} ne 'DONE') {
        $info = $joblist{ $gCfg{task} }
            or die "FATAL ERROR: Unknown task '$gCfg{task}'\n";

        print "TASK: $gCfg{task}\n";

        if ($gCfg{prompt}) {
            print "Press ENTER to continue...\n";
            my $temp = <STDIN>;
            die if $temp =~ m/^quit/i;
        }

        &{ $info->{handler} };
        &SetSystemTask( $info->{next} );
    }

}  #  End RunConversion

###############################################################################
#  LoadVssNames
###############################################################################
sub LoadVssNames {
    &DoSsCmd("info \"$gCfg{vssdatadir}\\names.dat\"");

    my $xs = XML::Simple->new(KeyAttr => [],
                              ForceArray => [qw(NameCacheEntry Entry)],);

    my $xml = $xs->XMLin($gSysOut);

    my $namesref = $xml->{NameCacheEntry} || return 1;

    my($entry, $count, $offset, $name);

    my $cache = Vss2Svn::DataCache->new('NameLookup')
        || &ThrowError("Could not create cache 'NameLookup'");

ENTRY:
    foreach $entry (@$namesref) {
        $count = $entry->{NrOfEntries};
        next ENTRY unless $count > 1;

        $offset = $entry->{offset};

        if ($count == 2) {
            $name = $entry->{Entry}->[1]->{content};
        } else {
            $name = $entry->{Entry}->[$count - 2]->{content};
        }

        $cache->add($offset, $name);
    }

    $cache->commit();

}  #  End LoadVssNames

###############################################################################
#  FindPhysDbFiles
###############################################################################
sub FindPhysDbFiles {

    my $cache = Vss2Svn::DataCache->new('Physical')
        || &ThrowError("Could not create cache 'Physical'");

    find(sub{ &FoundSsFile($cache) }, $gCfg{vssdatadir});

    $cache->commit();

}  #  End FindPhysDbFiles

###############################################################################
#  FoundSsFile
###############################################################################
sub FoundSsFile {
    my($cache) = @_;

    my $path = $File::Find::name;
    return if (-d $path);

    my $vssdatadir = quotemeta($gCfg{vssdatadir});

    if ($path =~ m:^$vssdatadir/./([a-z]{8})$:i) {
        $cache->add(uc($1));
    }

}  #  End FoundSsFile

###############################################################################
#  GetPhysVssHistory
###############################################################################
sub GetPhysVssHistory {
    my($sql, $sth, $row, $physname, $physdir);

    &LoadNameLookup;
    my $cache = Vss2Svn::DataCache->new('PhysicalAction', 1)
        || &ThrowError("Could not create cache 'PhysicalAction'");

    $sql = "SELECT * FROM Physical";
    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    my $xs = XML::Simple->new(ForceArray => [qw(Version)]);

    while (defined($row = $sth->fetchrow_hashref() )) {
        $physname = $row->{physname};

        $physdir = "$gCfg{vssdir}\\data\\" . substr($physname, 0, 1);

        &GetVssPhysInfo($cache, $physdir, $physname, $xs);
    }

    $cache->commit();

}  #  End GetPhysVssHistory

###############################################################################
#  GetVssPhysInfo
###############################################################################
sub GetVssPhysInfo {
    my($cache, $physdir, $physname, $xs) = @_;

    &DoSsCmd("info \"$physdir\\$physname\"");

    my $xml = $xs->XMLin($gSysOut);
    my $parentphys;

    my $iteminfo = $xml->{ItemInfo};

    if (!defined($iteminfo) || !defined($iteminfo->{Type}) ||
        ref($iteminfo->{Type})) {

        &ThrowWarning("Can't handle file '$physname'; not a project or file\n");
        return;
    }

    if ($iteminfo->{Type} == 1) {
        $parentphys = ($physname eq 'AAAAAAAA')?
            '' : &GetProjectParent($xml);
    } elsif ($iteminfo->{Type} == 2) {
        $parentphys = undef;
    } else {
        &ThrowWarning("Can't handle file '$physname'; not a project or file\n");
        return;
    }

    &GetVssItemVersions($cache, $physname, $parentphys, $xml);

}  #  End GetVssPhysInfo

###############################################################################
#  GetProjectParent
###############################################################################
sub GetProjectParent {
    my($xml) = @_;

    no warnings 'uninitialized';
    return $xml->{ItemInfo}->{ParentPhys} || undef;

}  #  End GetProjectParent

###############################################################################
#  GetVssItemVersions
###############################################################################
sub GetVssItemVersions {
    my($cache, $physname, $parentphys, $xml) = @_;

    return 0 unless defined $xml->{Version};

    my($parentdata, $version, $vernum, $action, $name, $actionid, $actiontype,
       $tphysname, $itemname, $itemtype, $parent, $user, $timestamp, $comment,
       $is_binary, $info, $priority, $sortkey, $cachename);

VERSION:
    foreach $version (@{ $xml->{Version} }) {
        $action = $version->{Action};
        $name = $action->{SSName};
        $tphysname = $action->{Physical} || $physname;
        $user = $version->{UserName};
        $timestamp = $version->{Date};

        $itemname = &GetItemName($name);

        $actionid = $action->{ActionId};
        $info = $gActionType{$actionid};

        if (!$info) {
            warn "\nWARNING: Unknown action '$actionid'\n";
            next VERSION;
        }

        $itemtype = $info->{type};
        $actiontype = $info->{action};

        if ($actiontype eq 'IGNORE') {
            next VERSION;
        }

        $comment = undef;
        $is_binary = 0;
        $info = undef;
        $parentdata = 0;
        $priority = 5;

        if ($version->{Comment} && !ref($version->{Comment})) {
            $comment = $version->{Comment} || undef;
        }

        if ($itemtype == 1 && $physname eq 'AAAAAAAA'
            && ref($tphysname)) {

            $tphysname = $physname;
            $itemname = '';
        } elsif ($physname ne $tphysname) {
            # If version's physical name and file's physical name are different,
            # this is a project describing an action on a child item. Most of
            # the time, this very same data will be in the child's physical
            # file and with more detail (such as check-in comment).

            # However, in some cases (such as renames, or when the child's
            # physical file was later purged), this is the only place we'll
            # have the data; also, sometimes the child record doesn't even
            # have enough information about itself (such as which project it
            # was created in and which project(s) it's shared in).

            # So, for a parent record describing a child action, we'll set a
            # flag, then combine them in the next phase.

            $parentdata = 1;

            # OK, since we're describing an action in the child, the parent is
            # actually this (project) item

            $parentphys = $physname;
        } else {
            $parentphys = undef;
        }

        if ($itemtype == 1) {
            $itemname .= '/';
        } elsif (defined($xml->{ItemInfo}) &&
            defined($xml->{ItemInfo}->{Binary}) &&
            $xml->{ItemInfo}->{Binary}) {

            $is_binary = 1;
        }

        if ($actiontype eq 'RENAME') {
            # if a rename, we store the new name in the action's 'info' field

            $info = &GetItemName($action->{NewSSName});

            if ($itemtype == 1) {
                $info .= '/';
            }
        } elsif ($actiontype eq 'BRANCH') {
            $info = $action->{Parent};
        }

        $vernum = ($parentdata)? undef : $version->{VersionNumber};

        $priority -= 4 if $actiontype eq 'ADD'; # Adds are always first
        $priority -= 3 if $actiontype eq 'SHARE';
        $priority -= 2 if $actiontype eq 'BRANCH';

        # store the reversed physname as a sortkey; a bit wasteful but makes
        # debugging easier for the time being...
        $sortkey = reverse($tphysname);

        $cache->add($tphysname, $vernum, $parentphys, $actiontype, $itemname,
                    $itemtype, $timestamp, $user, $is_binary, $info, $priority,
                    $sortkey, $parentdata, $comment);

    }

}  #  End GetVssItemVersions

###############################################################################
#  GetItemName
###############################################################################
sub GetItemName {
    my($nameelem) = @_;

    my $itemname = $nameelem->{content};

    if (defined($nameelem->{offset})) {
        # Might have a "better" name in the name cache, but sometimes the
        # original name is best.
        my $cachename = $gNameLookup{ $nameelem->{offset} };
        return $itemname unless defined($cachename);

        if (!defined($itemname) ||
            (length($cachename) >= length($itemname) &&
            !($itemname !~ m/~/ && $cachename =~ m/~/))) {

            print "Changing name of '$itemname' to '$cachename' from "
                . "name cache\n" if $gCfg{debug};

            $itemname = $cachename;
        } else {
            print "Found name '$cachename' in namecache, but kept original "
                . "'$itemname'\n" if $gCfg{debug};
        }
    }

    return $itemname;

}  #  End GetItemName

###############################################################################
#  LoadNameLookup
###############################################################################
sub LoadNameLookup {
    my($sth, $row);

    $sth = $gCfg{dbh}->prepare('SELECT offset, name FROM NameLookup');
    $sth->execute();

    while(defined($row = $sth->fetchrow_hashref() )) {
        $gNameLookup{ $row->{offset} } = $row->{name};
    }
}  #  End LoadNameLookup

###############################################################################
#  MergeParentData
###############################################################################
sub MergeParentData {
    # VSS has a funny way of not placing enough information to rebuild history
    # in one data file; for example, renames are stored in the parent project
    # rather than in that item's data file. Also, it's sometimes impossible to
    # tell from a child record which was eventually shared to multiple folders,
    # which folder it was originally created in.

    # So, at this stage we look for any parent records which described child
    # actions, then update those records with data from the child objects. We
    # then delete the separate child objects to avoid duplication.

    my($sth, $rows, $row);
    $sth = $gCfg{dbh}->prepare('SELECT * FROM PhysicalAction '
                               . 'WHERE parentdata = 1');
    $sth->execute();

    # need to pull in all recs at once, since we'll be updating/deleting data
    $rows = $sth->fetchall_arrayref( {} );

    my($childrecs, $child, $id);
    my @delchild = ();

    foreach $row (@$rows) {
        $childrecs = &GetChildRecs($row);

        if (scalar @$childrecs > 1) {
            &ThrowWarning("Multiple child recs for parent rec "
                          . "'$row->{action_id}'");
        }

        foreach $child (@$childrecs) {
            &UpdateParentRec($row, $child);
            push(@delchild, $child->{action_id});
        }
    }

    foreach $id (@delchild) {
        &DeleteChildRec($id);
    }

    1;

}  #  End MergeParentData

###############################################################################
#  GetChildRecs
###############################################################################
sub GetChildRecs {
    my($parentrec) = @_;

    # Here we need to find any child rows which give us additional info on the
    # parent rows. There's no definitive way to find matching rows, but joining
    # on physname, actiontype, timestamp, and author gets us close. The problem
    # is that the "two" actions may not have happened in the exact same second,
    # so we need to also look for any that are up to two seconds apart and hope
    # we don't get the wrong row.

    my $sql = <<"EOSQL";
SELECT
    *
FROM
    PhysicalAction
WHERE
    parentdata = 0
    AND physname = ?
    AND actiontype = ?
    AND (? - timestamp IN (0, 1, 2))
    AND author = ?
ORDER BY
    timestamp
EOSQL

    my $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute( @{ $parentrec }{qw(physname actiontype timestamp author)} );

    return $sth->fetchall_arrayref( {} );
}  #  End GetChildRecs

###############################################################################
#  UpdateParentRec
###############################################################################
sub UpdateParentRec {
    my($row, $child) = @_;

    # The child record has the "correct" version number (relative to the child
    # and not the parent), as well as the comment info and whether the file is
    # binary

    my $comment;

    {
        no warnings 'uninitialized';
        $comment = "$row->{comment}\n$child->{comment}";
        $comment =~ s/\n$//;
    }

    my $sql = <<"EOSQL";
UPDATE
    PhysicalAction
SET
    version = ?,
    is_binary = ?,
    comment = ?
WHERE
    action_id = ?
EOSQL

    my $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute( $child->{version}, $child->{is_binary}, $comment,
                  $row->{action_id} );

}  #  End UpdateParentRec

###############################################################################
#  DeleteChildRec
###############################################################################
sub DeleteChildRec {
    my($id) = @_;

    my $sql = "DELETE FROM PhysicalAction WHERE action_id = ?";

    my $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute($id);
}  #  End DeleteChildRec

###############################################################################
#  BuildVssActionHistory
###############################################################################
sub BuildVssActionHistory {
    my $vsscache = Vss2Svn::DataCache->new('VssAction', 1)
        || &ThrowError("Could not create cache 'VssAction'");

    my $joincache = Vss2Svn::DataCache->new('SvnRevisionVssAction')
        || &ThrowError("Could not create cache 'SvnRevisionVssAction'");

    # This will keep track of the current SVN revision, and increment it when
    # the author or comment changes, the timestamps span more than an hour
    # (by default), or the same physical file is affected twice

    my $svnrevs = Vss2Svn::SvnRevHandler->new()
        || &ThrowError("Could not create SVN revision handler");
    $svnrevs->{verbose} = $gCfg{verbose};

    my($sth, $row, $action, $handler, $physinfo, $itempaths, $allitempaths);

    my $sql = 'SELECT * FROM PhysicalAction ORDER BY timestamp ASC, '
                    . 'priority ASC, sortkey ASC';

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

ROW:
    while(defined($row = $sth->fetchrow_hashref() )) {
        $svnrevs->check($row);
        $action = $row->{actiontype};

        $handler = Vss2Svn::ActionHandler->new($row);
        $handler->{verbose} = $gCfg{verbose};
        $physinfo = $handler->physinfo();

        if (defined($physinfo) && $physinfo->{type} != $row->{itemtype} ) {
            &ThrowError("Inconsistent item type for '$row->{physname}'; "
                        . "'$row->{itemtype}' unexpected");
        }

        # The handler's job is to keep track of physical-to-real name mappings
        # and return the full item paths corresponding to the physical item. In
        # case of a rename, it will return the old name, so we then do another
        # lookup on the new name.

        # Commits and renames can apply to multiple items if that item is
        # shared; since SVN has no notion of such shares, we keep track of
        # those ourself and replicate the functionality using multiple actions.

        if (!$handler->handle($action)) {
            &ThrowWarning($handler->{errmsg})
                if $handler->{errmsg};
            next ROW;
        }

        $itempaths = $handler->{itempaths};

        if (!defined $itempaths) {
            &ThrowWarning($handler->{errmsg})
                if $handler->{errmsg};
            next ROW;
        }

        # In cases of a corrupted share source, the handler may change the
        # action from 'SHARE' to 'ADD'
        $row->{actiontype} = $handler->{action};

        # May contain add'l info for the action depending on type:
        # RENAME: the new name (without path)
        # SHARE: the source path which was shared
        # MOVE: the new path
        $row->{info} = $handler->{info};

        $allitempaths = join("\t", @$itempaths);
        $row->{itempaths} = $allitempaths;

        $vsscache->add(@$row{ qw(physname version actiontype itempaths
                             itemtype is_binary info) });
        $joincache->add( $svnrevs->{revnum}, $vsscache->{pkey} );

    }

    $vsscache->commit();
    $svnrevs->commit();
    $joincache->commit();

}  #  End BuildVssActionHistory

###############################################################################
#  ImportToSvn
###############################################################################
sub ImportToSvn {
    # For the time being, we support only creating a dumpfile and not directly
    # importing to SVN. We could perhaps add this functionality by making the
    # CreateSvnDumpfile logic more generic and using polymorphism to switch out
    # the Vss2Svn::Dumpfile object with one that handles imports.

    &CreateSvnDumpfile;
}  #  End ImportToSvn

###############################################################################
#  CreateSvnDumpfile
###############################################################################
sub CreateSvnDumpfile {
    my $fh;

    my $file = 'vss2svn-dumpfile.txt';
    open $fh, ">$file"
        or &ThrowError("Could not create dumpfile '$file'");

    my($sql, $sth, $row, $revision, $actions, $action, $physname, $itemtype);

    my %exported = ();

    $sql = 'SELECT * FROM SvnRevision ORDER BY revision_id ASC';

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    my $dumpfile = Vss2Svn::Dumpfile->new($fh);

REVISION:
    while(defined($row = $sth->fetchrow_hashref() )) {
        $revision = $row->{revision_id};
        $dumpfile->begin_revision($row);

        next REVISION if $revision == 0;

        $actions = &GetRevVssActions($revision);

ACTION:
        foreach $action(@$actions) {
            $physname = $action->{physname};
            $itemtype = $action->{itemtype};

            if (!exists $exported{$physname}) {
                if ($itemtype == 2) {
                    $exported{$physname} = &ExportVssPhysFile($physname);
                } else {
                    $exported{$physname} = undef;
                }
            }

            $dumpfile->do_action($action, $exported{$physname});
        }
    }

    my @err = @{ $dumpfile->{errors} };

    if (scalar @err > 0) {
        print "\nERRORS during dumpfile creation:\n   ";
        print join("\n   ", @err);
    }

    $dumpfile->finish();
    close $fh;

}  #  End CreateSvnDumpfile

###############################################################################
#  GetRevVssActions
###############################################################################
sub GetRevVssActions {
    my($revision) = @_;

    my($sql, $sth);
    $sql = <<"EOSQL";
SELECT * FROM
    VssAction v
INNER JOIN
    SvnRevisionVssAction sv ON sv.action_id = v.action_id
WHERE
    sv.revision_id = ?
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute($revision);

    return $sth->fetchall_arrayref( {} );
}  #  End GetRevVssActions

###############################################################################
#  ExportVssPhysFile
###############################################################################
sub ExportVssPhysFile {
    my($physname) = @_;

    $physname =~ m/^((.).)/;

    my $exportdir = "$gCfg{vssdata}\\$1";
    my $physpath = "$gCfg{vssdir}\\data\\$2\\$physname";

    if (! -e $physpath) {
        # physical file doesn't exist; it must have been destroyed later
        &ThrowWarning("Can't retrieve revisions from physical file "
                      . "'$physname'; it was either destroyed or corrupted");
        return undef;
    }

    mkpath($exportdir);

    &DoSsCmd("get -b -v1 --force-overwrite $physpath $exportdir\\$physname");

    return $exportdir;
}  #  End ExportVssPhysFile

###############################################################################
#  ShowHeader
###############################################################################
sub ShowHeader {
    my $info = $gCfg{task} eq 'INIT'? 'BEGINNING CONVERSION...' :
        "RESUMING CONVERSION FROM TASK '$gCfg{task}' AT STEP $gCfg{step}...";
    my $starttime = ctime($^T);

    my $ssversion = &GetSsVersion();

    print <<"EOTXT";
======== VSS2SVN ========
$info
Start Time   : $starttime

VSS Dir      : $gCfg{vssdir}
Temp Dir     : $gCfg{tempdir}

SSPHYS exe   : $gCfg{ssphys}
SSPHYS ver   : $ssversion

EOTXT

}  #  End ShowHeader

###############################################################################
#  ShowSummary
###############################################################################
sub ShowSummary {

    my $starttime = ctime($^T);
    chomp $starttime;
    my $endtime = ctime(time);
    chomp $endtime;
    my $elapsed;

    {
        use integer;
        my $secs = time - $^T;

        my $hours = $secs / 3600;
        $secs -= ($hours * 3600);

        my $mins = $secs / 60;
        $secs -= ($mins * 60);

        $elapsed = sprintf("%2.2i:%2.2i:%2.2i", $hours, $mins, $secs);
    }

    print <<"EOTXT";
Started at    : $starttime
Ended at      : $endtime
Elapsed time  : $elapsed (H:M:S)

EOTXT

}  #  End ShowSummary

###############################################################################
#  DoSsCmd
###############################################################################
sub DoSsCmd {
    my($cmd) = @_;

    my $ok = &DoSysCmd("\"$gCfg{ssphys}\" $cmd", 1);

    $gSysOut =~ s/\x00//g; # remove null bytes
    $gSysOut =~ s/.\x08//g; # yes, I've seen VSS store backspaces in names!
    $gSysOut =~ s/[\x00-\x09\x11\x12\x14-\x1F\x7F-\xFF]/_/g; # just to be sure

}  #  End DoSsCmd

###############################################################################
#  DoSysCmd
###############################################################################
sub DoSysCmd {
    my($cmd, $allowfail) = @_;

    print "$cmd\n" if $gCfg{verbose};
    $gSysOut = `$cmd`;

    print $gSysOut if $gCfg{debug};

    my $rv = 1;

    if ($? == -1) {
        &ThrowWarning("FAILED to execute: $!");
        die unless $allowfail;

        $rv = 0;
    } elsif ($?) {
        &ThrowWarning(sprintf "FAILED with non-zero exit status %d", $? >> 8);
        die unless $allowfail;

        $rv = 0;
    }

    return $rv;

}  #  End DoSysCmd

###############################################################################
#  GetSsVersion
###############################################################################
sub GetSsVersion {
    my $out = `\"$gCfg{ssphys}\" -v 2>&1`;
    $out =~ m/^(ssphys v.*?)[:\n]/m;

    return $1 || 'unknown';
}  #  End GetSsVersion

###############################################################################
#  ThrowWarning
###############################################################################
sub ThrowWarning {
    my($msg, $callinfo) = @_;

    $callinfo ||= [caller()];

    $msg .= "\nat $callinfo->[1] line $callinfo->[2]";

    warn "ERROR -- $msg\n";

    push @gErr, $msg;

}  #  End ThrowWarning

###############################################################################
#  ThrowError
###############################################################################
sub ThrowError {
    &ThrowWarning(@_, [caller()]);
    &StopConversion;
}  #  End ThrowError

###############################################################################
#  StopConversion
###############################################################################
sub StopConversion {
    &DisconnectDatabase;
    &CloseAllFiles;

    exit(1);
}  #  End StopConversion

###############################################################################
#  CloseAllFiles
###############################################################################
sub CloseAllFiles {

}  #  End CloseAllFiles

###############################################################################
#  SetSystemTask
###############################################################################
sub SetSystemTask {
    my($task, $leavestep) = @_;

    print "\nSETTING TASK $task\n" if $gCfg{verbose};

    my($sql, $sth);

    $sth = $gSth{'SYSTEMTASK'};

    if (!defined $sth) {
        $sql = <<"EOSQL";
UPDATE
    SystemInfo
SET
    task = ?
EOSQL

        $sth = $gSth{'SYSTEMTASK'} = $gCfg{dbh}->prepare($sql);
    }

    $sth->execute($task);

    $gCfg{task} = $task;

    &SetSystemStep(0) unless $leavestep;

}  #  End SetSystemTask

###############################################################################
#  SetSystemStep
###############################################################################
sub SetSystemStep {
    my($step) = @_;

    print "\nSETTING STEP $step\n" if $gCfg{verbose};

    my($sql, $sth);

    $sth = $gSth{'SYSTEMSTEP'};

    if (!defined $sth) {
        $sql = <<"EOSQL";
UPDATE
    SystemInfo
SET
    step = ?
EOSQL

        $sth = $gCfg{'SYSTEMSTEP'} = $gCfg{dbh}->prepare($sql);
    }

    $sth->execute($step);

    $gCfg{step} = $step;

}  #  End SetSystemStep

###############################################################################
#  ConnectDatabase
###############################################################################
sub ConnectDatabase {
    my $db = $gCfg{sqlitedb};

    if (-e $db && (!$gCfg{resume} ||
                   (defined($gCfg{task}) && $gCfg{task} eq 'INIT'))) {

        unlink $db or &ThrowError("Could not delete existing database "
                                  .$gCfg{sqlitedb});
    }

    print "Connecting to database $db\n\n";

    $gCfg{dbh} = DBI->connect("dbi:SQLite2:dbname=$db", '', '',
                              {RaiseError => 1, AutoCommit => 1})
        or die "Couldn't connect database $db: $DBI::errstr";

}  #  End ConnectDatabase

###############################################################################
#  DisconnectDatabase
###############################################################################
sub DisconnectDatabase {
    $gCfg{dbh}->disconnect if defined $gCfg{dbh};
}  #  End DisconnectDatabase

###############################################################################
#  SetupGlobals
###############################################################################
sub SetupGlobals {
    if (defined($gCfg{task}) && $gCfg{task} eq 'INIT') {
        &InitSysTables;
    } else {
        &ReloadSysTables;
    }

    $gCfg{ssphys} = 'SSPHYS.exe' if !defined($gCfg{ssphys});
    $gCfg{vssdatadir} = "$gCfg{vssdir}\\data";

    (-d "$gCfg{vssdatadir}") or &ThrowError("$gCfg{vssdir} does not appear "
                                            . "to be a valid VSS database");

    my($id, $type, $action);
    while(<DATA>) {
        chomp;
        ($id, $type, $action) = split "\t";
        $gActionType{$id} = {type => $type, action => $action};
    }

    Vss2Svn::DataCache->SetCacheDir($gCfg{tempdir});
    Vss2Svn::DataCache->SetDbHandle($gCfg{dbh});
    Vss2Svn::DataCache->SetVerbose($gCfg{verbose});

    Vss2Svn::SvnRevHandler->SetRevTimeRange($gCfg{revtimerange});

}  #  End SetupGlobals

###############################################################################
#  InitSysTables
###############################################################################
sub InitSysTables {
    my($sql, $sth);

    $sql = <<"EOSQL";
CREATE TABLE
    Physical (
        physname    VARCHAR
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    NameLookup (
        offset      INTEGER,
        name        VARCHAR
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    PhysicalAction (
        action_id   INTEGER PRIMARY KEY,
        physname    VARCHAR,
        version     INTEGER,
        parentphys  VARCHAR,
        actiontype  VARCHAR,
        itemname    VARCHAR,
        itemtype    INTEGER,
        timestamp   INTEGER,
        author      VARCHAR,
        is_binary   INTEGER,
        info        VARCHAR,
        priority    INTEGER,
        sortkey     VARCHAR,
        parentdata  INTEGER,
        comment     TEXT
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE INDEX
    PhysicalAction_IDX1 ON PhysicalAction (
        timestamp   ASC,
        priority    ASC,
        sortkey     ASC
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE INDEX
    PhysicalAction_IDX2 ON PhysicalAction (
        physname    ASC,
        parentphys  ASC,
        actiontype  ASC,
        timestamp   ASC,
        author      ASC
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    VssAction (
        action_id   INTEGER PRIMARY KEY,
        physname    VARCHAR,
        version     INTEGER,
        action      VARCHAR,
        itempaths   VARCHAR,
        itemtype    INTEGER,
        is_binary   INTEGER,
        info        VARCHAR
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    SvnRevision (
        revision_id INTEGER PRIMARY KEY,
        timestamp   INTEGER,
        author      VARCHAR,
        comment     TEXT
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    SvnRevisionVssAction (
        revision_id INTEGER,
        action_id   INTEGER
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    my @cfgitems = qw(task step vssdir svnurl svnuser svnpwd ssphys tempdir
        setsvndate starttime);

    my $fielddef = join(",\n        ",
                        map {sprintf('%-12.12s VARCHAR', $_)} @cfgitems);

    $sql = <<"EOSQL";
CREATE TABLE
    SystemInfo (
        $fielddef
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    my $fields = join(', ', @cfgitems);
    my $args = join(', ', map {'?'} @cfgitems);

    $sql = <<"EOSQL";
INSERT INTO
    SystemInfo ($fields)
VALUES
    ($args)
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute(map {$gCfg{$_}} @cfgitems);
    $sth->finish();

}  #  End InitSysTables

###############################################################################
#  ReloadSysTables
###############################################################################
sub ReloadSysTables {
    my($sql, $sth, $sthup, $row, $field, $val);

    $sql = "SELECT * FROM SystemInfo";

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    $row = $sth->fetchrow_hashref();

FIELD:
    while (($field, $val) = each %$row) {
        if (defined($gCfg{$field})) { # allow user to override saved vals
            $sql = "UPDATE SystemInfo SET $field = ?";
            $sthup = $gCfg{dbh}->prepare($sql);
            $sthup->execute($gCfg{$field});
        } else {
            $gCfg{$field} = $val;
        }
    }

    $sth->finish();
    &SetSystemTask($gCfg{task});

}  #  End ReloadSysTables

###############################################################################
#  Initialize
###############################################################################
sub Initialize {
    GetOptions(\%gCfg,'vssdir=s','tempdir=s','resume','verbose',
               'debug','task=s');

    &GiveHelp("Must specify --vssdir") if !defined($gCfg{vssdir});
    $gCfg{tempdir} = '.\\_vss2svn' if !defined($gCfg{tempdir});

    $gCfg{sqlitedb} = "$gCfg{tempdir}\\vss_data.db";

    # XML output from ssphysout placed here.
    $gCfg{ssphysout} = "$gCfg{tempdir}\\ssphysout";

    # Commit messages for SVN placed here.
    $gCfg{svncomment} = "$gCfg{tempdir}\\svncomment.tmp.txt";
    mkdir $gCfg{tempdir} unless (-d $gCfg{tempdir});

    # Directories for holding VSS revisions
    $gCfg{vssdata} = "$gCfg{tempdir}\\vssdata";

    if ($gCfg{resume} && !-e $gCfg{sqlitedb}) {
        warn "WARNING: --resume set but no database exists; starting new "
            . "conversion...";
        $gCfg{resume} = 0;
    }

    if ($gCfg{debug}) {
        $gCfg{verbose} = 1;
    }

    $gCfg{starttime} = scalar localtime($^T);

    ### Don't go past here if resuming a previous run ###
    if ($gCfg{resume}) {
        return 1;
    }

    rmtree($gCfg{vssdata}) if (-e $gCfg{vssdata});
    mkdir $gCfg{vssdata};

    #foreach my $check (qw(svnurl)) {
    #    &GiveHelp("ERROR: missing required parameter $check")
    #        unless defined $gCfg{$check};
    #}

    $gCfg{ssphys} ||= 'SSPHYS.exe';
    $gCfg{svn} ||= 'SVN.exe';

    $gCfg{task} = 'INIT';
    $gCfg{step} = 0;

    # number of seconds that can elapse between first and last action in an
    # SVN revision
    $gCfg{revtimerange} = 3600;

}  #  End Initialize

###############################################################################
#  GiveHelp
###############################################################################
sub GiveHelp {
    my($msg) = @_;

    $msg ||= 'Online Help';

    print <<"EOTXT";

$msg

USAGE: perl vss2svn.pl --vssdir <dir> [options]

REQUIRED PARAMETERS:
    --vssdir <dir>  : Directory where VSS database is located. This should be
                      the directory in which the "srcsafe.ini" file is located.

OPTIONAL PARAMETERS:
    --ssphys <path> : Full path to ssphys.exe program; uses PATH otherwise
    --tempdir <dir> : Temp directory to use during conversion;
                      default is .\\_vss2svn
    --resume        : Resume a failed or aborted previous run
    --verbose       : Print more info about the items being processed
    --debug         : Print lots of debugging info.
EOTXT

    exit(1);
}  #  End GiveHelp

# Following is the data for %gActionType. First field is the node type from
# ssphys; second field is item type (1=project, 2=file); third field is the
# generic action it should be mapped to (loosely mapped to SVN actions)

__DATA__
CreatedProject	1	ADD
AddedProject	1	ADD
RenamedProject	1	RENAME
MovedProjectTo	1	IGNORE
MovedProjectFrom	1	MOVE
DeletedProject	1	DELETE
DestroyedProject	1	IGNORE
RecoveredProject	1	RECOVER
CheckedIn	2	COMMIT
CreatedFile	2	ADD
AddedFile	2	ADD
RenamedFile	2	RENAME
DeletedFile	2	DELETE
DestroyedFile	2	IGNORE
RecoveredFile	2	RECOVER
SharedFile	2	SHARE
BranchFile	2	BRANCH
PinnedFile	2	IGNORE
RollBack	2	IGNORE
UnpinnedFile	2	IGNORE
Labeled	2	IGNORE
