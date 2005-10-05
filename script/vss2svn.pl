#!/usr/bin/perl

use warnings;
use strict;

use Getopt::Long;
use DBI;
use DBD::SQLite2;
use XML::Simple;
use File::Find;
use Time::CTime;

our(%gCfg, %gSth, @gErr, %gFh, $gSysOut);

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
    my %joblist =
        (
            INIT            => {handler => sub{ 1; },
                                next    => 'LOADVSSNAMES'},
            LOADVSSNAMES    => {handler => \&LoadVssNames,
                                next    => 'FINDDBFILES'},
            FINDDBFILES     => {handler => \&FindPhysDbFiles,
                                next    => 'GETHIST'},
            GETHIST         => {handler => \&GetVssHistory,
                                next    => 'LOADHIST'},
            LOADHIST        => {handler => \&LoadVssHistory,
                                next    => 'BUILDREVS'},
            BUILDREVS       => {handler => \&BuildRevs,
                                next    => 'IMPORTSVN'},
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
    
    my $xs = XML::Simple->new();
    my $ref = $xs->XMLin($gSysOut);
    
    1; # TODO: load names entries
}  #  End LoadVssNames

###############################################################################
#  FindPhysDbFiles
###############################################################################
sub FindPhysDbFiles {
    
    &StartDataCache('Physical');
    
    find(\&FoundSsFile, $gCfg{datadir});
    
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
        &AddDataCache("$1\t0");
    }

}  #  End FoundSsFile

###############################################################################
#  GetVssHistory
###############################################################################
sub GetVssHistory {
    my($sql, $sth, $row, $physname, $dir, $physpath);
    
    $sql = "SELECT * FROM Physical WHERE status = 0";
    $sth = &PrepSql($sql);
    $sth->execute();
    
    # have to fetch whole table so we can update as we go
    my $data = $sth->fetchall_arrayref( {} );
    $sth->finish();
    
    my $xs = XML::Simple->new();
    
    foreach $row (@$data) {
        $physname = $row->{physname};

        $dir = substr($physname, 0, 1);
        ($physpath = "$gCfg{vssdir}\\data\\$dir\\$physname") =~ s:/:\\:g;

        &GetVssPhysInfo($physpath, $xs);
        &SetPhysStatus($physname, 1);
    }

}  #  End GetVssHistory

###############################################################################
#  GetVssPhysInfo
###############################################################################
sub GetVssPhysInfo {
    my($physpath, $xs) = @_;
    
    &DoSsCmd("info -a \"$physpath\" -s xml");
    
    my $ref = $xs->XMLin($gSysOut);
    1;  # TODO: load physical file info
}  #  End GetVssPhysInfo

###############################################################################
#  SetPhysStatus
###############################################################################
sub SetPhysStatus {
    my($physname, $status) = @_;
    
    my($sql, $sth);
    
    $sth = $gSth{'SETPHYSSTATUS'};
    
    if (!defined $sth) {
        $sql = <<"EOSQL";
UPDATE
    Physical
SET
    status = ?
WHERE
    physname = ?
EOSQL

        $sth = $gSth{'SETPHYSSTATUS'} = &PrepSql($sql);
    }
    
    $sth->execute($status, $physname);    

}  #  End SetPhysStatus

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
        
    print <<"EOTXT";
======== VSS2SVN ========
$info
Start Time   : $starttime

VSS Dir      : $gCfg{vssdir}
Temp Dir     : $gCfg{tempdir}

SSPHYS exe   : $gCfg{ssphys}

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
#  ThrowWarning
###############################################################################
sub ThrowWarning {
    my($msg) = @_;
    warn "ERROR -- $msg\n" if $gCfg{debug};
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
#  StartDataCache
###############################################################################
sub StartDataCache {
    my($table) = @_;
    $gCfg{cachetarget} = $table;
    unlink $gCfg{datacache};
    
    &OpenFile('DATACACHE', ">$gCfg{datacache}");
    
}  #  End StartDataCache

###############################################################################
#  AddDataCache
###############################################################################
sub AddDataCache {
    my(@data) = @_;
    
    my $fh = $gFh{DATACACHE};
    print $fh join("\t", map {defined $_? $_ : '\\N'} @data), "\n";
    
}  #  End AddDataCache

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
    
    print "...done\n";
    
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
    
    if (!$gCfg{resume} ||
        (defined($gCfg{task}) && $gCfg{task} eq 'INIT')) {
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
    
}  #  End SetupGlobals

###############################################################################
#  InitSysTables
###############################################################################
sub InitSysTables {
    my($sql, $sth);
    
    $sql = <<"EOSQL";
CREATE TABLE
    Physical (
        physname    VARCHAR,
        status      INTEGER
    )
EOSQL

    $sth = &PrepSql($sql);
    $sth->execute;
    
    $sql = <<"EOSQL";
CREATE TABLE
    Action (
        action_id   INTEGER PRIMARY KEY,
        physname    VARCHAR,
        type        VARCHAR,
        itemname    VARCHAR,
        itemtype    INTEGER,
        timestamp   VARCHAR,
        author      VARCHAR,
        info        VARCHAR,
        comment     TEXT
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

