#!/usr/bin/perl

use warnings;
use strict;

use Getopt::Long;
use DBI;
use DBD::SQLite2;
use XML::Simple;
use File::Find;
use Time::CTime;
use Data::Dumper;

our(%gCfg, %gSth, @gErr, %gFh, $gSysOut, %gActionType, %gNameLookup, $gId,
    %gPhysInfo,);

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
                                next    => 'BUILDREVS'},

            # Combine these individual actions into atomic actions a' la SVN
            BUILDREVS       => {handler => \&BuildRevs,
                                next    => 'IMPORTSVN'},

            # Create a dumpfile or import to repository
            IMPORTSVN       => {handler => \&ImportToSvn,
                                next    => 'DONE'},
        );

    my $info;

    while ($gCfg{task} ne 'DONE') {
        $info = $joblist{ $gCfg{task} }
            or die "FATAL ERROR: Unknown task '$gCfg{task}'\n";

        print "TASK: $gCfg{task}\n" if $gCfg{debug};

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
    &DoSsCmd("info -a \"$gCfg{vssdatadir}\\names.dat\" -s xml");

    my $xs = XML::Simple->new(KeyAttr => [],
                              ForceArray => [qw(Entry)],);

    my $xml = $xs->XMLin($gSysOut);

    my $namesref = $xml->{NameCacheEntry} || return 1;

    my($entry, $count, $offset, $name);

    &StartDataCache('NameLookup', 1);

ENTRY:
    foreach $entry (@$namesref) {
        $count = $entry->{NrOfEntries};
        next ENTRY unless $count > 1;

        $offset = $entry->{offset};

        $name = $entry->{Entry}->[$count - 1]->{content};

        &AddDataCache($offset, $name);
    }

    &CommitDataCache();

}  #  End LoadVssNames

###############################################################################
#  FindPhysDbFiles
###############################################################################
sub FindPhysDbFiles {

    &StartDataCache('Physical', 1);

    find(\&FoundSsFile, $gCfg{vssdatadir});

    &CommitDataCache();

}  #  End FindPhysDbFiles

###############################################################################
#  FoundSsFile
###############################################################################
sub FoundSsFile {

    my $path = $File::Find::name;
    return if (-d $path);

    my $vssdatadir = quotemeta($gCfg{vssdatadir});

    if ($path =~ m:^$vssdatadir/./([a-z]{8})$:i) {
        &AddDataCache(uc($1), 0);
    }

}  #  End FoundSsFile

###############################################################################
#  GetPhysVssHistory
###############################################################################
sub GetPhysVssHistory {
    my($sql, $sth, $row, $physname, $physdir);

    &LoadNameLookup;
    &StartDataCache('PhysicalAction', 1, 1);

    $sql = "SELECT * FROM Physical";
    $sth = &PrepSql($sql);
    $sth->execute();

    my $xs = XML::Simple->new(ForceArray => [qw(Version)]);

    while (defined($row = $sth->fetchrow_hashref() )) {
        $physname = $row->{physname};

        $physdir = "$gCfg{vssdir}\\data\\" . substr($physname, 0, 1);

        &GetVssPhysInfo($physdir, $physname, $xs);
    }

    &CommitDataCache();

}  #  End GetPhysVssHistory

###############################################################################
#  GetVssPhysInfo
###############################################################################
sub GetVssPhysInfo {
    my($physdir, $physname, $xs) = @_;

    &DoSsCmd("info -a \"$physdir\\$physname\" -s xml");

    my $xml = $xs->XMLin($gSysOut);
    my $parentphys;

    if (defined($xml->{ProjectItem})) {
        $parentphys = ($physname eq 'AAAAAAAA')?
            '' : &GetProjectParent($xml);
    } elsif (defined($xml->{FileItem})) {
        $parentphys = &GetFileParent($xml);
    } else {
        &ThrowWarning("Can't handle file '$physname'; not a project or file\n");
        return;
    }

    &GetVssItemInfo($physname, $parentphys, $xml);

}  #  End GetVssPhysInfo

###############################################################################
#  GetProjectParent
###############################################################################
sub GetProjectParent {
    my($xml) = @_;

    no warnings 'uninitialized';
    return $xml->{ProjectItem}->{ParentPhys} || undef;

}  #  End GetProjectParent

###############################################################################
#  GetFileParent
###############################################################################
sub GetFileParent {
    my($xml) = @_;

    no warnings 'uninitialized';

    my $parents = $xml->{ParentFolder};
    my $parentphys;

    if (ref $parents eq 'ARRAY') {
        # If there is more than one parent folder, this is a shared or branched
        # item. Since the child item has no way of knowing who its original
        # parent is, we'll leave it blank and expect it to be filled in by the
        # parent.
        $parentphys = undef;
    } else {
        $parentphys = $parents->{ParentPhys} || undef;
    }

    return $parentphys;

}  #  End GetFileParent

###############################################################################
#  GetVssItemInfo
###############################################################################
sub GetVssItemInfo {
    my($physname, $parentphys, $xml) = @_;

    return 0 unless defined $xml->{Version};

    my($parentdata, $version, $number, $action, $name, $actionid, $actiontype,
       $tphysname, $itemname, $itemtype, $parent, $user, $timestamp, $comment,
       $info);

VERSION:
    foreach $version (@{ $xml->{Version} }) {
        $action = $version->{Action};
        $name = $action->{SSName};
        $tphysname = $action->{Physical} || $physname;
        $user = $version->{UserName};
        $timestamp = $version->{Date};

        if (defined($name->{offset})) {
            $itemname = $gNameLookup{ $name->{offset} } || $name->{content};
        } else {
            $itemname = $name->{content} || undef;
        }

        $actionid = $action->{ActionId};
        $info = $gActionType{$actionid}; # || next VERSION;  # unknown action
        if (!$info) {
            next VERSION;
        }
        $itemtype = $info->{type};
        $actiontype = $info->{action};

        $comment = undef;
        $info = undef;
        $parentdata = 0;

        if ($version->{Comment} && !ref($version->{Comment})) {
            $comment = $version->{Comment} || undef;
        }

        if ($itemtype == 1 && $physname eq 'AAAAAAAA' && ref($tphysname)) {
            $tphysname = $physname;
            $itemname = '';
        } elsif ($physname ne $tphysname) {
            # If version's physical name and file physical name are different,
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
        }

        if ($itemtype == 1) {
            $itemname .= '/';
        }

        if ($actiontype eq 'RENAME') {
            # if a rename, we store the new name in the action's 'info' field
            no warnings 'uninitialized';

            $name = $action->{NewSSName};
            if (defined($name->{offset})) {
                $info = $gNameLookup{ $name->{offset} } || $name->{content};
            } else {
                $info = $name->{content} || undef;
            }

            if ($itemtype == 1) {
                $info .= '/';
            }
        }

        $number = ($parentdata)? undef : $version->{VersionNumber};

        &AddDataCache($tphysname, $number, $parentphys, $actiontype, $itemname,
                      $itemtype, $timestamp, $user, $info, $parentdata,
                      $comment);

    }

}  #  End GetVssItemInfo

###############################################################################
#  LoadNameLookup
###############################################################################
sub LoadNameLookup {
    my($sth, $row);

    $sth = &PrepSql('SELECT offset, name FROM NameLookup');
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
    $sth = &PrepSql('SELECT * FROM PhysicalAction WHERE parentdata = 1');
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

}  #  End MergeParentData

###############################################################################
#  GetChildRecs
###############################################################################
sub GetChildRecs {
    my($parentrec) = @_;

    my $sql = <<"EOSQL";
SELECT
    *
FROM
    PhysicalAction
WHERE
    parentdata = 0
    AND physname = ?
    AND actiontype = ?
    AND timestamp = ?
    AND author = ?
EOSQL

    my $sth = &PrepSql($sql);
    $sth->execute( @{ $parentrec }{qw(physname actiontype timestamp author)} );

    return $sth->fetchall_arrayref( {} );
}  #  End GetChildRecs

###############################################################################
#  UpdateParentRec
###############################################################################
sub UpdateParentRec {
    my($row, $child) = @_;

    # The child record has the "correct" version number (relative to the child
    # and not the parent), as well as the comment info

    my $sql = <<"EOSQL";
UPDATE
    PhysicalAction
SET
    version = ?,
    comment = ?
WHERE
    action_id = ?
EOSQL

    my $sth = &PrepSql($sql);
    $sth->execute( $child->{version}, $child->{comment}, $row->{action_id} );

}  #  End UpdateParentRec

###############################################################################
#  DeleteChildRec
###############################################################################
sub DeleteChildRec {
    my($id) = @_;

    my $sql = "DELETE FROM PhysicalAction WHERE action_id = ?";

    my $sth = &PrepSql($sql);
    $sth->execute($id);
}  #  End DeleteChildRec

###############################################################################
#  BuildVssActionHistory
###############################################################################
sub BuildVssActionHistory {
    &StartDataCache('VssAction', 1, 1);

    my($sth, $row, $action, $handler);

    $sth = &PrepSql('SELECT * FROM PhysicalAction ORDER BY timestamp');
    $sth->execute();

    #TODO: handle deletes
    my %handlers =
        (
         ADD        => \&VssAddHandler,
         RENAME     => \&VssRenameHandler,
        );

    while(defined($row = $sth->fetchrow_hashref() )) {
        $action = $row->{actiontype};

        $handler = $handlers{$action};

        if (defined($gPhysInfo{ $row->{physname}} ) &&
            $gPhysInfo{ $row->{physname} }->{type} != $row->{itemtype} ) {

            &ThrowError("Inconsistent item type for '$row->{physname}'; "
                        . "'$row->{itemtype}' unexpected");
        }

        # The handler's job is to keep %gPhysInfo up to date with physical-to-
        # real item name mappings and return the full item path of the physical
        # item. In case of a rename, it will return the old name, so we then do
        # another lookup on the new name.

        if (defined($handler)) {
            $row->{itempath} = &$handler($row);
        } else {
            $row->{itempath} = &GetCurrentItemPath($row->{physname});
        }

        if ($row->{actiontype} eq 'RENAME') {
            $row->{info} = &GetCurrentItemPath($row->{physname});
        }

        &AddDataCache(@$row{ qw(physname version actiontype itempath itemtype
            timestamp author info comment) });

    }

    &CommitDataCache();

}  #  End BuildVssActionHistory

###############################################################################
#  VssAddHandler
###############################################################################
sub VssAddHandler {
    my($row) = @_;

    $gPhysInfo{ $row->{physname} } =
        {
         type       => $row->{itemtype},
         name       => $row->{itemname},
         parentphys => $row->{parentphys},
        };

    return &GetCurrentItemPath($row->{physname});
}  #  End VssAddHandler

###############################################################################
#  VssRenameHandler
###############################################################################
sub VssRenameHandler {
    my($row) = @_;

    # get the existing path before the rename; parent sub will get the new
    # name in the next call

    my $itempath = &GetCurrentItemPath($row->{physname});

    $gPhysInfo{ $row->{physname} } =
        {
         type       => $row->{itemtype},
         name       => $row->{info},
         parentphys => $row->{parentphys},
        };

    return $itempath;
}  #  End VssRenameHandler

###############################################################################
#  GetCurrentItemPath
###############################################################################
sub GetCurrentItemPath {
    my($physname, $recursed) = @_;

    # Uses recursion to determine the current full path for an item based on
    # the name of its physical file. We can't cache this information because
    # a rename in a parent folder would not immediately trigger a rename in
    # all of the child items.

    if (!$recursed) {
        $gCfg{nameResolveRecurse} = 0;
        $gCfg{nameResolveSeen} = '';
    } elsif (++$gCfg{nameResolveRecurse} >= 1000) {
        &ThrowError("Infinite recursion detected while looking up parent for "
                    . "'$physname'");
    }

    if ($physname eq 'AAAAAAAA') {
        return '/';
    }

    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        &ThrowError("Could not determine real path for '$physname':\n"
                    . $gCfg{nameResolveSeen});
    }

    $gCfg{nameResolveSeen} .= "$physname, ";

    return &GetCurrentItemPath($physinfo->{parentphys}, 1) . $physinfo->{name};

}  #  End GetCurrentItemPath

###############################################################################
#  ImportToSvn
###############################################################################
sub ImportToSvn {
    defined($gCfg{svnurl})? &CheckinToSvn : &CreateSvnDumpfile;
}  #  End ImportToSvn

###############################################################################
#  CheckinToSvn
###############################################################################
sub CheckinToSvn {

}  #  End CheckinToSvn

###############################################################################
#  CreateSvnDumpfile
###############################################################################
sub CreateSvnDumpfile {

}  #  End CreateSvnDumpfile

###############################################################################
#  ShowHeader
###############################################################################
sub ShowHeader {
    if ($gCfg{log}) {
        my $prefix = $gCfg{pvcsproj} || $gCfg{svnurl} || "log-$$";
        $prefix =~ s:.*[\\/]::;
        $gCfg{logfile} = "./logs/$prefix.txt";
        print "All output will be logged to $gCfg{logfile}...\n";
        open LOG, ">>$gCfg{logfile}"
            or die "Couldn't append to logfile $gCfg{logfile}";
        open STDERR, ">&LOG";
        select STDERR; $| = 1;
        select LOG; $| = 1;
    }

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
    if (@gErr) {
        print "\n\n\n====ERROR SUMMARY====\n\n";
        foreach (@gErr) {
            print "$_\n";
        }
    } else {
        print "\n\n\n====NO ERRORS ENCOUNTERED THIS RUN====\n\n";
    }

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
SVN rev range : $gCfg{firstrev} - $gCfg{lastrev}
Started at    : $starttime
Ended at      : $endtime
Elapsed time  : $elapsed (H:M:S)

EOTXT

    &CloseFile('LOG');

}  #  End ShowSummary

###############################################################################
#  DoSsCmd
###############################################################################
sub DoSsCmd {
    my($cmd) = @_;

    &DoSysCmd("\"$gCfg{ssphys}\" $cmd", 0);

}  #  End DoSsCmd

###############################################################################
#  DoSysCmd
###############################################################################
sub DoSysCmd {
    my($cmd, $allowfail) = @_;

    print "$cmd\n" if $gCfg{debug};
    $gSysOut = `$cmd`;

    print $gSysOut if $gCfg{debug};

    my $rv = 1;

    if ($? == -1) {
        &ThrowError("FAILED to execute: $!");
        die unless $allowfail;

        $rv = 0;
    } elsif ($?) {
        &ThrowError(sprintf "FAILED with non-zero exit status %d", $? >> 8);
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
    my($msg) = @_;
    warn "ERROR -- $msg\n";
    print "ERROR -- $msg\n" if $gCfg{log};
    push @gErr, $msg;
}  #  End ThrowWarning

###############################################################################
#  ThrowError
###############################################################################
sub ThrowError {
    &ThrowWarning(@_);
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
#  OpenFile
###############################################################################
sub OpenFile {
    my($fhname, $target) = @_;

    (my $name = $target) =~ s/^>//;

    print "\nOPENING FILE $name\n" if $gCfg{debug};

    open $gFh{$fhname}, $target
        or &ThrowError("Could not open file $name");

}  #  End OpenFile

###############################################################################
#  CloseFile
###############################################################################
sub CloseFile {
    my($fhname) = @_;

    close $gFh{$fhname};
    delete $gFh{$fhname};

}  #  End CloseFile

###############################################################################
#  CloseAllFiles
###############################################################################
sub CloseAllFiles {
    map { &CloseFile($_) } values %gFh;
    close LOG;
}  #  End CloseAllFiles

###############################################################################
#  SetSystemTask
###############################################################################
sub SetSystemTask {
    my($task, $leavestep) = @_;

    print "\nSETTING TASK $task\n" if $gCfg{debug};

    my($sql, $sth);

    $sth = $gSth{'SYSTEMTASK'};

    if (!defined $sth) {
        $sql = <<"EOSQL";
UPDATE
    SystemInfo
SET
    task = ?
EOSQL

        $sth = $gSth{'SYSTEMTASK'} = &PrepSql($sql);
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

    print "\nSETTING STEP $step\n" if $gCfg{debug};

    my($sql, $sth);

    $sth = $gSth{'SYSTEMSTEP'};

    if (!defined $sth) {
        $sql = <<"EOSQL";
UPDATE
    SystemInfo
SET
    step = ?
EOSQL

        $sth = $gCfg{'SYSTEMSTEP'} = &PrepSql($sql);
    }

    $sth->execute($step);

    $gCfg{step} = $step;

}  #  End SetSystemStep

###############################################################################
#  DeleteTable
###############################################################################
sub DeleteTable {
    my($table) = @_;

    my $sth = &PrepSql("DELETE FROM $table");
    return $sth->execute;
}  #  End DeleteTable

###############################################################################
#  StartDataCache
###############################################################################
sub StartDataCache {
    my($table, $delete, $autoinc) = @_;

    if ($delete) {
        &DeleteTable($table);
    }

    if ($autoinc) {
        $gId = 0;
    } else {
        undef $gId;
    }

    $gCfg{cachetarget} = $table;
    unlink $gCfg{datacache};

    &OpenFile('DATACACHE', ">$gCfg{datacache}");

}  #  End StartDataCache

###############################################################################
#  AddDataCache
###############################################################################
sub AddDataCache {
    my(@data) = @_;

    if (ref($data[0]) eq 'ARRAY') {
        @data = @{ $data[0] };
    }

    if (defined $gId) {
        unshift(@data, $gId++);
    }

    my $fh = $gFh{DATACACHE};
    print $fh join("\t", map {&FormatCacheData($_)} @data), "\n";

}  #  End AddDataCache

###############################################################################
#  FormatCacheData
###############################################################################
sub FormatCacheData {
    my($data) = @_;
    return '\\N' if !defined($data);

    $data =~ s/([\t\n\\])/\\$1/g;

    return $data;
}  #  End FormatCacheData

###############################################################################
#  CommitDataCache
###############################################################################
sub CommitDataCache {
    my($sql, $sth);

    &CloseFile('DATACACHE');

    print "\n\nCOMMITTING $gCfg{cachetarget} CACHE TO DATABASE\n"
        if $gCfg{debug};
    $sql = "COPY $gCfg{cachetarget} FROM '$gCfg{datacache}'";

    $sth = &PrepSql($sql);
    $sth->execute();

    unlink $gCfg{datacache};

}  #  End CommitDataCache

###############################################################################
#  PrepSql
###############################################################################
sub PrepSql {
    my($sql) = @_;

    print "\nSQL:\n$sql\n" if $gCfg{debug};
    return $gCfg{dbh}->prepare($sql);

}  #  End PrepSql

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

    $sth = &PrepSql($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    NameLookup (
        offset      INTEGER,
        name        VARCHAR
    )
EOSQL

    $sth = &PrepSql($sql);
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
        info        VARCHAR,
        parentdata  INTEGER,
        comment     TEXT
    )
EOSQL

    $sth = &PrepSql($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE INDEX
    PhysicalAction_IDX1 ON PhysicalAction (
        timestamp   ASC
    )
EOSQL

    $sth = &PrepSql($sql);
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

    $sth = &PrepSql($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    VssAction (
        action_id   INTEGER PRIMARY KEY,
        physname    VARCHAR,
        version     INTEGER,
        action      VARCHAR,
        itempath    VARCHAR,
        itemtype    INTEGER,
        timestamp   INTEGER,
        author      VARCHAR,
        info        VARCHAR,
        comment     TEXT
    )
EOSQL

    $sth = &PrepSql($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    ItemNameHistory (
        physname    VARCHAR,
        timestamp   INTEGER,
        itemname    VARCHAR
    )
EOSQL

    $sth = &PrepSql($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    Revision (
        revision_id INTEGER PRIMARY KEY,
        svndate     VARCHAR,
        author      VARCHAR,
        comment     VARCHAR,
        status      INTEGER
    )
EOSQL

    $sth = &PrepSql($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    AtomRevision (
        atom_id     INTEGER,
        rev_id      INTEGER
    )
EOSQL

    $sth = &PrepSql($sql);
    $sth->execute;

    my @cfgitems = qw(task step vssdir svnurl svnuser svnpwd ssphys tempdir
        setsvndate debug starttime);

    my $fielddef = join(",\n        ",
                        map {sprintf('%-12.12s VARCHAR', $_)} @cfgitems);

    $sql = <<"EOSQL";
CREATE TABLE
    SystemInfo (
        $fielddef
    )
EOSQL

    $sth = &PrepSql($sql);
    $sth->execute;

    my $fields = join(', ', @cfgitems);
    my $args = join(', ', map {'?'} @cfgitems);

    $sql = <<"EOSQL";
INSERT INTO
    SystemInfo ($fields)
VALUES
    ($args)
EOSQL

    $sth = &PrepSql($sql);
    $sth->execute(map {$gCfg{$_}} @cfgitems);
    $sth->finish();

}  #  End InitSysTables

###############################################################################
#  ReloadSysTables
###############################################################################
sub ReloadSysTables {
    my($sql, $sth, $sthup, $row, $field, $val);

    $sql = "SELECT * FROM SystemInfo";

    $sth = &PrepSql($sql);
    $sth->execute();

    $row = $sth->fetchrow_hashref();

FIELD:
    while (($field, $val) = each %$row) {
        if (defined($gCfg{$field})) { # allow user to override saved vals
            $sql = "UPDATE SystemInfo SET $field = ?";
            $sthup = &PrepSql($sql);
            $sthup->execute($gCfg{$field});
        } else {
            $gCfg{$field} = $val;
        }
    }

    $sth->finish();
    &SetSystemTask($gCfg{task}, 1);

}  #  End ReloadSysTables

###############################################################################
#  Initialize
###############################################################################
sub Initialize {
    GetOptions(\%gCfg,'vssdir=s','tempdir=s','resume','debug','task=s');

    &GiveHelp("Must specify --vssdir") if !defined($gCfg{vssdir});
    $gCfg{tempdir} = '.\\_vss2svn' if !defined($gCfg{tempdir});

    $gCfg{sqlitedb} = "$gCfg{tempdir}\\vss_data.db";

    # XML output from ssphysout placed here.
    $gCfg{ssphysout} = "$gCfg{tempdir}\\ssphysout";

    # SQLite data cache placed here.
    $gCfg{datacache} = "$gCfg{tempdir}\\datacache.tmp.txt";

    # Commit messages for SVN placed here.
    $gCfg{svncomment} = "$gCfg{tempdir}\\svncomment.tmp.txt";
    mkdir $gCfg{tempdir} unless (-d $gCfg{tempdir});

    if ($gCfg{resume} && !-e $gCfg{sqlitedb}) {
        warn "WARNING: --resume set but no database exists; starting new "
            . "conversion...";
        $gCfg{resume} = 0;
    }

    ### Don't go past here if resuming a previous run ###
    if ($gCfg{resume}) {
        return 1;
    }

    #foreach my $check (qw(svnurl)) {
    #    &GiveHelp("ERROR: missing required parameter $check")
    #        unless defined $gCfg{$check};
    #}

    $gCfg{ssphys} ||= 'SSPHYS.exe';
    $gCfg{svn} ||= 'SVN.exe';

    $gCfg{task} = 'INIT';
    $gCfg{step} = 0;
    $gCfg{starttime} = scalar localtime($^T);

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
    --setsvndate    : Set svn:date property to original VSS checkin date
                      (see SVN:DATE WARNING in readme.txt)
    --log           : Log all output to <tempdir>\\vss2svn.log.txt
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
DeletedProject	1	DELETE
RecoveredProject	1	RECOVER
Checkedin	2	COMMIT
CreatedFile	2	ADD
AddedFile	2	ADD
RenamedFile	2	RENAME
DeletedFile	2	DELETE
RecoveredFile	2	RECOVER
SharedFile	2	COPY
PinnedFile	2	XXX
UnpinnedFile	2	XXX