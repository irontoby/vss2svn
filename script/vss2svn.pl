#!/usr/bin/perl

use warnings;
use strict;

use Getopt::Long;
use DBI;
use DBD::SQLite2;
use XML::Simple;
use XML::Parser;
use File::Find;
use File::Path;
use Time::CTime;
use Data::Dumper;
use Benchmark ':hireswallclock';

use lib '.';
use Vss2Svn::ActionHandler;
use Vss2Svn::DataCache;
use Vss2Svn::SvnRevHandler;
use Vss2Svn::Dumpfile;

require Encode;

our(%gCfg, %gSth, %gErr, %gFh, $gSysOut, %gActionType, %gNameLookup, %gId);

our $VERSION = '0.11.0-nightly.$LastChangedRevision$';
$VERSION =~ s/\$.*?(\d+).*\$/$1/; # get only the number out of the svn revision

&Initialize;
&ConnectDatabase;

&SetupGlobals;
&ShowHeader;

&RunConversion;

&ShowSummary;
&DisconnectDatabase;

###############################################################################
#  Progress tracking
###############################################################################

our ($progress, $total, $progress_title);

sub init_progress($$) {
    $progress = 0;
    $progress_title = $_[0];
    $total = $_[1] + 1;
    print "\r$progress_title: 0% (0)                   ";
}

sub advance(;$) {
    my $m = $_[0]||'';
    print "\r$progress_title: ".int(100*$progress/$total)."% ($progress) $m         ";
}

sub end_progress() {
    advance '(done)';
    print "\n";
}

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
                                next    => 'MERGEMOVEDATA'},

            # Merge data from move actions
            MERGEMOVEDATA => {handler => \&MergeMoveData,
                                next    => 'REMOVETMPCHECKIN'},

            # Remove temporary check ins
            REMOVETMPCHECKIN => {handler => \&RemoveTemporaryCheckIns,
                                 next    => 'MERGEUNPINPIN'},

            # Remove unnecessary Unpin/pin activities
            MERGEUNPINPIN => {handler => \&MergeUnpinPinData,
                                 next    => 'BUILDCOMMENTS'},

            # Rebuild possible missing comments
            BUILDCOMMENTS => {handler => \&BuildComments,
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
        push @{ $gCfg{tasks} }, $gCfg{task};

        if ($gCfg{prompt}) {
            print "Press ENTER to continue...\n";
            my $temp = <STDIN>;
            die if $temp =~ m/^quit/i;
        }

        $gCfg{dbh}->begin_work();
        &{ $info->{handler} };
        $gCfg{dbh}->commit();
        &SetSystemTask( $info->{next} );
    }

}  #  End RunConversion

###############################################################################
#  LoadVssNames
###############################################################################
sub LoadVssNames {
    my $cache = Vss2Svn::DataCache->new('NameLookup', 0, -reuse_data => $gCfg{reuse_cache})
        || &ThrowError("Could not create cache 'NameLookup'");

    if ($cache->{reused}) {
        $cache->commit();
        return;
    }

    &DoSsCmd("info -e$gCfg{encoding} \"$gCfg{vssdatadir}/names.dat\"");

    my($entry, $count, $offset, $name);

    my $parser = new XML::Parser(Handlers => {
        Start => sub {
            my ($exp, $tag, %attrs) = @_;
            
            if ($tag eq 'NameCacheEntry') {
                $offset = $attrs{offset};
            } elsif ($tag eq 'Entry') {
                $entry = $attrs{id};
                $name = '';
            }
        },
        Char => sub {
            my ($exp, $str) = @_;
            $name .= $str;
        },
        End => sub {
            my ($exp, $tag) = @_;
            
            if ($tag eq 'Entry') {
                # The cache can contain 4 different entries:
                #   id=1: abbreviated DOS 8.3 name for file items
                #   id=2: full name for file items
                #   id=3: abbreviated 27.3 name for file items
                #   id=10: full name for project items
                # Both ids 1 and 3 are not of any interest for us, since they only
                # provide abbreviated names for different szenarios. We are only
                # interested if we have id=2 for file items, or id=10 for project
                # items.
                if ($entry == 10 || $entry == 2) {
                    $cache->add($offset, $name);
                }
            }
        },
    });
        
    $parser->parse($gSysOut);

    $cache->commit();
}  #  End LoadVssNames

###############################################################################
#  FindPhysDbFiles
###############################################################################
sub FindPhysDbFiles {
    my $cache = Vss2Svn::DataCache->new('Physical', 0, -reuse_data => $gCfg{reuse_cache})
        || &ThrowError("Could not create cache 'Physical'");

    if ($cache->{reused}) {
        $cache->commit();
        return;
    }

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
    my $cache = Vss2Svn::DataCache->new('PhysicalAction', 1, -reuse_data => $gCfg{reuse_cache})
        || &ThrowError("Could not create cache 'PhysicalAction'");

    if ($cache->{reused}) {
        $cache->commit();
        return;
    }

    $sql = "SELECT * FROM Physical ORDER BY physname";
    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    my $xs = XML::Simple->new(ForceArray => [qw(Version)]);

    $progress = 0;
    while (defined($row = $sth->fetchrow_hashref() )) {
        $physname = $row->{physname};
        
        print "\r${physname}...   " if !$gCfg{debug} && ($progress++ % 1000) == 0;

        $physdir = "$gCfg{vssdir}/data";
        my $physfolder = substr($physname, 0, 1);

        &GetVssPhysInfo($cache, $physdir, $physfolder, $physname, $xs);
    }
    
    print "\rCommitting...   \n" unless $gCfg{debug};

    $cache->commit();

}  #  End GetPhysVssHistory

###############################################################################
#  FindPhysnameFile
###############################################################################
sub FindPhysnameFile {
    my($physdir, $physfolder, $physname) = @_;

    # return it if we can find it without any alteration
    return ($physdir, $physfolder, $physname) if -f "$physdir/$physfolder/$physname";
    my $lcphysname = lc($physname);
    my $lcphysfolder = lc($physfolder);

    # try finding lowercase folder/filename
    return ($physdir, $lcphysfolder, $lcphysname) if -f "$physdir/$lcphysfolder/$lcphysname";

    # try finding lowercase folder/uppercase filename
    return ($physdir, $lcphysfolder, $physname) if -f "$physdir/$lcphysfolder/$physname";

    # haven't seen this one, but try it...
    return ($physdir, $physfolder, $lcphysname) if -f "$physdir/$physfolder/$lcphysname";

    # no idea what to return...
    return (undef, undef, undef);
}

###############################################################################
#  GetVssPhysInfo
###############################################################################
sub GetVssPhysInfo {
    my($cache, $physdir, $physfolder, $physname, $xs) = @_;

    my @filesegment = &FindPhysnameFile($physdir, $physfolder, $physname);

    print "physdir: \"$filesegment[0]\", physfolder: \"$filesegment[1]\" physname: \"$filesegment[2]\"\n" if $gCfg{debug};

    if (!defined $filesegment[0] || !defined $filesegment[1]
    || !defined $filesegment[2]) {
        # physical file doesn't exist; it must have been destroyed later
        &ThrowWarning("Can't retrieve info from physical file "
                      . "'$physname'; it was either destroyed or corrupted");
        return;
    }

    &DoSsCmd("info -e$gCfg{encoding} \"$filesegment[0]/$filesegment[1]/$filesegment[2]\"");

    my $xml = $xs->XMLin($gSysOut);
    my $parentphys;

    my $iteminfo = $xml->{ItemInfo};

    if (!defined($iteminfo) || !defined($iteminfo->{Type}) ||
        ref($iteminfo->{Type})) {

        &ThrowWarning("Can't handle file '$physname'; not a project or file\n");
        return;
    }

    if ($iteminfo->{Type} == 1) {
        $parentphys = (uc($physname) eq 'AAAAAAAA')?
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
       $is_binary, $info, $priority, $sortkey, $label, $cachename);

    my $last_timestamp = 0;

VERSION:
    foreach $version (@{ $xml->{Version} }) {
        $action = $version->{Action};
        $name = $action->{SSName};
        $tphysname = $action->{Physical} || $physname;
        $user = $version->{UserName};

        $itemname = &GetItemName($name);

        $actionid = $action->{ActionId};
        $info = $gActionType{$actionid};

        if (!$info) {
            &ThrowWarning ("'$physname': Unknown action '$actionid'\n");
            next VERSION;
        }

        # check the linear order of timestamps. It could be done better, for
        # example checking the next version and calculate the middle time stamp
        # but regardless of what we do here, the result is erroneous, since it
        # will mess up the labeling.
        $timestamp = $version->{Date};
        if ($timestamp < $last_timestamp) {
            $timestamp = $last_timestamp + 1;
            &ThrowWarning ("'$physname': wrong timestamp at version "
                           . "'$version->{VersionNumber}'; setting timestamp to "
                           . "'$timestamp'");
        }
        $last_timestamp = $timestamp;

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
        $label = undef;

        if ($version->{Comment} && !ref($version->{Comment})) {
            $comment = $version->{Comment} || undef;
        }

        # In case of Label the itemtype is the type of the item currently
        # under investigation
        if ($actiontype eq 'LABEL') {
            my $iteminfo = $xml->{ItemInfo};
            $itemtype = $iteminfo->{Type};

        }

        # we can have label actions and labes attached to versions
        if (defined $action->{Label} && !ref($action->{Label})) {
            $label = $action->{Label};

            # append the label comment to a possible version comment
            if ($action->{LabelComment} && !ref($action->{LabelComment})) {
                if (defined $comment) {
                    print "Merging LabelComment and Comment for "
                        . "'$tphysname;$version->{VersionNumber}'\n"; # if $gCfg{verbose};
                    $comment .= "\n";
                }

                $comment .= $action->{LabelComment} || undef;
            }
        }

        if (defined($comment)) {
            $comment =~ s/^\s+//s;
            $comment =~ s/\s+$//s;
        }

        if ($itemtype == 1 && uc($physname) eq 'AAAAAAAA'
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

        # since there is no corresponding client action for PIN, we need to
        # enter the concrete version number here manually
        # In a share action the pinnedToVersion attribute can also be set
#        if ($actiontype eq 'PIN') {
            $vernum = $action->{PinnedToVersion} if (defined $action->{PinnedToVersion});
#        }

        # for unpin actions also remeber the unpinned version
        $info = $action->{UnpinnedFromVersion} if (defined $action->{UnpinnedFromVersion});

        $priority -= 4 if $actiontype eq 'ADD'; # Adds are always first
        $priority -= 3 if $actiontype eq 'SHARE';
        $priority -= 3 if $actiontype eq 'PIN';
        $priority -= 2 if $actiontype eq 'BRANCH';

        # store the reversed physname as a sortkey; a bit wasteful but makes
        # debugging easier for the time being...
        $sortkey = reverse($tphysname);

        $cache->add($tphysname, $vernum, $parentphys, $actiontype, $itemname,
                    $itemtype, $timestamp, $user, $is_binary, $info, $priority,
                    $sortkey, $parentdata, $label, $comment);

        # Handle version labels as a secondary action for the same version
        # version labels and label action use the same location to store the
        # label. Therefore it is not possible to assign a version label to
        # version where the actiontype was LABEL. But ssphys will report the
        # same label twice. Therefore filter the Labeling versions here.
        if (defined $version->{Label} && !ref($version->{Label})
            && $actiontype ne 'LABEL') {
            my ($labelComment);

            if ($version->{LabelComment} && !ref($version->{LabelComment})) {
                $labelComment = $version->{LabelComment};
            }
            else {
                $labelComment = "assigned label '$version->{Label}' to version $vernum of physical file '$tphysname'";
            }
            $cache->add($tphysname, $vernum, $parentphys, 'LABEL', $itemname,
                        $itemtype, $timestamp, $user, $is_binary, $info, 5,
                        $sortkey, $parentdata, $version->{Label}, $labelComment);
        }
    }

}  #  End GetVssItemVersions

###############################################################################
#  GetItemName
###############################################################################
sub GetItemName {
    my($nameelem) = @_;

    my $itemname = $nameelem->{content};

    if (defined($nameelem->{offset})) {
        # see if we have a better name in the cache
        my $cachename = $gNameLookup{ $nameelem->{offset} };

        if (defined($cachename)) {
            print "Changing name of '$itemname' to '$cachename' from "
                  . "name cache\n" if $gCfg{debug};
            $itemname = $cachename;
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
        $gNameLookup{ $row->{offset} } = Encode::decode_utf8( $row->{name} );
    }
}  #  End LoadNameLookup

##############################################################################
#  Support for using array representation of PhysicalActions
###############################################################################

BEGIN {
    our @phys_sql_fields = qw(
            action_id physname version parentphys actiontype
            itemname itemtype timestamp author is_binary
            info priority sortkey parentdata label comment
    );
    our %phys_sql_map = map { 'PA_'.$phys_sql_fields[$_] => $_ } 0..$#phys_sql_fields;
    our $phys_sql_fieldspec = join(',',@phys_sql_fields);
}

our (@phys_sql_fields, %phys_sql_map, $phys_sql_fieldspec);
use constant \%phys_sql_map;

# Expand an array into a hash
sub expand_arr {
    my $arr = shift @_;
    return { map { $_[$_] => $arr->[$_] } (0..$#_) };
}

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
    $sth = $gCfg{dbh}->prepare('SELECT '.$phys_sql_fieldspec.' FROM PhysicalAction '
                               . 'WHERE parentdata > 0');
    $sth->execute();

    # need to pull in all recs at once, since we'll be updating/deleting data
    $rows = $sth->fetchall_arrayref();

    my($childrecs, $child, $id, $depth);
    my @delchild = ();

    init_progress 'Processing', scalar(@$rows);
    
    foreach my $arow (@$rows) {
        $row = expand_arr $arow, @phys_sql_fields;
        advance if ($progress++ % 1000) == 0;
    
        $childrecs = &GetChildRecs($row);

        if (scalar @$childrecs > 1) {
            &ThrowWarning("Multiple child recs for parent rec "
                          . "'$row->{action_id}'");
        }

        $depth = &GetPathDepth($row);

        foreach $child (@$childrecs) {
            &UpdateParentRec($row, $child);
            push(@delchild, $child->{action_id});
        }
    }

    &DeleteChildRecList(\@delchild);

    end_progress;
    
    1;

}  #  End MergeParentData

###############################################################################
#  GetPathDepth
###############################################################################
sub GetPathDepth {
    my($row) = @_;

    # If we've already worked out the depth of this row, return it immediately
    if ($row->{parentdata} > 1) {
        return $row->{parentdata};
    }

    my($maxParentDepth, $depth, $parents, $parent);

    # Get the row(s) corresponding to the parent(s) of this row, and work out
    # the maximum depth

    my $sql = <<"EOSQL";
SELECT
    *
FROM
    PhysicalAction
WHERE
    parentdata > 0
    AND physname = ?
    AND actiontype = ?
    AND timestamp <= ?
EOSQL

    my $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute( @{ $row }{qw(parentphys actiontype timestamp)} );

    $parents =  $sth->fetchall_arrayref( {} );
    $maxParentDepth = 0;
    foreach $parent (@$parents) {
        $depth = &GetPathDepth($parent);
        $maxParentDepth = ($depth > $maxParentDepth) ? $depth : $maxParentDepth;
    }

    # Depth of this path becomes one more than the maximum parent depth
    $depth = $maxParentDepth + 1;

    # Update the row for this record
    &UpdateDepth($row, $depth);

    return $depth;
}  #  End GetPathDepth

###############################################################################
#  UpdateDepth
###############################################################################
sub UpdateDepth {
    my($row, $depth) = @_;

    my $sql = <<"EOSQL";
UPDATE
    PhysicalAction
SET
    parentdata = ?
WHERE
    action_id = ?
EOSQL

    my $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute( $depth, $row->{action_id} );

}  #  End UpdateDepth

###############################################################################
#  GetChildRecs
###############################################################################
sub GetChildRecs {
    my($parentrec, $parentdata) = @_;

    # Here we need to find any child rows which give us additional info on the
    # parent rows. There's no definitive way to find matching rows, but joining
    # on physname, actiontype, timestamp, and author gets us close. The problem
    # is that the "two" actions may not have happened in the exact same second,
    # so we need to also look for any that are some time apart and hope
    # we don't get the wrong row.

    $parentdata = 0 unless defined $parentdata;
    $parentdata = 1 if $parentdata != 0;

    my $sql = <<"EOSQL";
SELECT
    *
FROM
    PhysicalAction
WHERE
    MIN(parentdata, 1) = ?
    AND physname = ?
    AND actiontype = ?
    AND author = ?
ORDER BY
    ABS(? - timestamp)
EOSQL

    my $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute( $parentdata, @{ $parentrec }{qw(physname actiontype author timestamp)} );

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
        $comment =~ s/^\n+//;
        $comment =~ s/\n+$//;
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
#  MergeMoveData
###############################################################################
sub MergeMoveData {
    # Similar to the MergeParentData, the MergeMove Data combines two the src
    # and target move actions into one move action. Since both items are parents
    # the MergeParentData function can not deal with this specific problem

    my($sth, $rows, $row);

    $sth = $gCfg{dbh}->prepare('SELECT * FROM PhysicalAction '
                               . 'WHERE actiontype = "MOVE_FROM"');
    $sth->execute();

    # need to pull in all recs at once, since we'll be updating/deleting data
    $rows = $sth->fetchall_arrayref( {} );

    my($childrecs, $child, $id);

    foreach $row (@$rows) {
        $row->{actiontype} = 'MOVE_TO';
        $childrecs = &GetChildRecs($row, 1);

        my $source = undef;
        my $target = $row->{parentphys};

        my $chosenChildRecord;
        my $childRecord;

        foreach $childRecord (@$childrecs) {
            if (!(defined $chosenChildRecord)
                && $childRecord->{timestamp} == $row->{timestamp}
                && !($childRecord->{parentphys} eq $row->{parentphys})) {

                $chosenChildRecord = $childRecord;
            }
        }

        if (defined $chosenChildRecord) {
            $source = $chosenChildRecord->{parentphys};
            &DeleteChildRec($chosenChildRecord->{action_id});

            my $sql = <<"EOSQL";
UPDATE
    PhysicalAction
SET
    actiontype = 'MOVE',
    parentphys = ?,
    info = ?
WHERE
    action_id = ?
EOSQL
            my $update;
            $update = $gCfg{dbh}->prepare($sql);

            $update->execute( $target, $source, $row->{action_id});
        } else {
            #the record did not have a matching MOVE_TO. call it a RESTORE
            print "Changing $row->{action_id} to a RESTORE\n";

            my $sql = <<"EOSQL";
UPDATE
    PhysicalAction
SET
    actiontype = 'RESTORE'
WHERE
    action_id = ?
EOSQL
            my $update;
            $update = $gCfg{dbh}->prepare($sql);

            $update->execute( $row->{action_id});
        }
    }


    # change all remaining MOVE_TO records into MOVE records and swap the src and target
    $sth = $gCfg{dbh}->prepare('SELECT * FROM PhysicalAction '
                               . 'WHERE actiontype = "MOVE_TO"');
    $sth->execute();
    $rows = $sth->fetchall_arrayref( {} );

    foreach $row (@$rows) {
        my $update;
        $update = $gCfg{dbh}->prepare('UPDATE PhysicalAction SET '
                                      . 'actiontype = "MOVE", '
                                      . 'parentphys = ?, '
                                      . 'info = ? '
                                      . 'WHERE action_id = ?');
        $update->execute($row->{info}, $row->{parentphys}, $row->{action_id});
    }

    $sth = $gCfg{dbh}->prepare('SELECT * FROM PhysicalAction WHERE actiontype = "RESTORE"');
    $sth->execute();
    $rows = $sth->fetchall_arrayref( {} );

    foreach $row (@$rows) {
        #calculate last name of this file. Store it in $info

        my $sql = "SELECT * FROM PhysicalAction WHERE physname = ? AND timestamp < ? AND actiontype <> 'LABEL' ORDER BY timestamp DESC";

        $sth = $gCfg{dbh}->prepare($sql);
        $sth->execute( $row->{physname}, $row->{timestamp} );

        my $myOlderRecords = $sth->fetchall_arrayref( {} );

        if (scalar @$myOlderRecords > 0) {
            my $update = $gCfg{dbh}->prepare('UPDATE PhysicalAction SET info = ? WHERE action_id = ?');
            $update->execute(@$myOlderRecords[0]->{itemname}, $row->{action_id});
        }
    }

    1;

}  #  End MergeMoveData

###############################################################################
# RemoveTemporaryCheckIns
# remove temporary checkins that where create to detect MS VSS capabilities
###############################################################################
sub RemoveTemporaryCheckIns {
    my($sth, $rows, $row);
    $sth = $gCfg{dbh}->prepare('SELECT * FROM PhysicalAction '
                               . 'WHERE comment = "Temporary file created by Visual Studio .NET to detect Microsoft Visual SourceSafe capabilities."'
                               . '      AND actiontype = "ADD"'
                               . '      AND itemtype = 2');		# only delete files, not projects
    $sth->execute();

    # need to pull in all recs at once, since we'll be updating/deleting data
    $rows = $sth->fetchall_arrayref( {} );

    foreach $row (@$rows) {
        my $physname = $row->{physname};

        my $sql = 'SELECT * FROM PhysicalAction WHERE physname = ?';
        my $update = $gCfg{dbh}->prepare($sql);

        $update->execute( $physname );

        # need to pull in all recs at once, since we'll be updating/deleting data
        my $recs = $update->fetchall_arrayref( {} );

        foreach my $rec (@$recs) {
            print "Remove action_id $rec->{action_id}, $rec->{physname}, $rec->{actiontype}, $rec->{itemname}\n";
            print "       $rec->{comment}\n" if defined ($rec->{comment});
            &DeleteChildRec($rec->{action_id});
        }
    }

    1;
}


###############################################################################
#  Sliding window support
###############################################################################
sub fetch_next_row(\@$) {
    my $row = $_[1]->fetchrow_hashref();
    push @{$_[0]}, $row if $row;
    return $row;
}

###############################################################################
#  MergeUnpinPinData
###############################################################################
sub MergeUnpinPinData {
    my($sth, $rows, $row, $r, $next_row);

    my $total_count = $gCfg{dbh}->selectrow_array('SELECT COUNT(*) FROM PhysicalAction');

    my $sql = 'SELECT * FROM PhysicalAction ORDER BY timestamp ASC, '
                . 'itemtype ASC, priority ASC, parentdata ASC, sortkey ASC, action_id ASC';
    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    $rows = [];
    fetch_next_row @$rows, $sth or return;
    fetch_next_row @$rows, $sth or return;

    my @delchild = ();
    my @updchild = ();
    
    init_progress 'Processing', $total_count;

    while (@$rows > 1) {
        $row = shift @$rows;
                
        advance if ($progress++ % 1000) == 0;

        if ($row->{actiontype} eq 'PIN' && !defined $row->{version}) # UNPIN
        {
            # Search for a matching pin action
            my $u = 0;
            while ($u <= $#$rows) {
                $next_row = $rows->[$u];

                # Bail out if the actions cannot get in one commit
                # to avoid quadratic performance and preserve limited window size
                last if ($next_row->{timestamp} - $row->{timestamp}) > ($gCfg{revtimerange}||3600);

                if (   $next_row->{actiontype} eq 'PIN'
                    && defined $next_row->{version}   # PIN
                    && $row->{physname} eq $next_row->{physname}
                    && $row->{parentphys} eq $next_row->{parentphys}
#                    && $next_row->{timestamp} - $row->{timestamp} < 60
#                    && $next_row->{action_id} - $row->{action_id} == 1
                    ) {
                        print "found UNPIN/PIN combination for $row->{parentphys}/$row->{physname}"
                            . "($row->{itemname}) @ ID $row->{action_id}\n"  if $gCfg{verbose};

                        # if we have a unpinFromVersion number copy this one to the PIN handler
                        if (defined $row->{info})
                        {
                            push (@updchild, [$row->{info}, $next_row->{action_id}]);
                        }

                        push (@delchild, $row->{action_id});
                        last;
                    }

                # if the next action is anything else than a pin stop the search
                last if ($next_row->{actiontype} ne 'PIN' );
            }
            continue {
                fetch_next_row @$rows, $sth unless ++$u <= $#$rows;
            }
        }
    } continue {
        fetch_next_row @$rows, $sth unless @$rows > 1;
    }

    init_progress 'Updating', scalar(@updchild);
    
    my $sql2 = "UPDATE PhysicalAction SET info = ? WHERE action_id = ?";
    my $sth2 = $gCfg{dbh}->prepare($sql2);
    
    foreach my $item (@updchild) {
        advance if ($progress++ % 1000) == 0;

        $sth2->execute(@$item);
    }
    
    &DeleteChildRecList(\@delchild);

    end_progress;
    1;

}  #  End MergeUnpinPinData

###############################################################################
#  BuildComments
###############################################################################
sub BuildComments {
    my($sth, $rows, $row, $r, $next_row);

    my $total_count = $gCfg{dbh}->selectrow_array('SELECT COUNT(*) FROM PhysicalAction');

    my $sql = 'SELECT * FROM PhysicalAction WHERE actiontype="PIN" AND itemtype=2 ORDER BY physname ASC';
    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    my @updchild = ();

    init_progress 'Processing', $total_count;
    
    while ($row = $sth->fetchrow_hashref()) {
        advance if ($progress++ % 1000) == 0;

        # technically we have the following situations:
        # PIN only: we come from the younger version and PIN to a older one: the
        #     younger version is the currenty version of the timestamp of the PIN action
        # UNPIN only: we unpin from a older version to the current version, the
        #     timestamp of the action will again define the younger version
        # UNPIN/PIN with known UNPIN version: we merge from UNPIN version to PIN version
        # UNPIN/PIN with unknown UNPIN version: we are lost in this case and we
        #     can not distinguish this case from the PIN only case.

        my $sql2;
        my $prefix;

        # PIN only
        if (    defined $row->{version}     # PIN version number
            && !defined $row->{info}) {     # no UNPIN version number
            $sql2 = 'SELECT * FROM PhysicalAction'
                    . ' WHERE physname="' . $row->{physname} . '"'
                    . '      AND parentphys ISNULL'
                    . '      AND itemtype=2'
                    . '      AND version>=' . $row->{version}
                    . '      AND timestamp<=' . $row->{timestamp}
                    . ' ORDER BY version DESC';
            $prefix = "reverted changes for: \n";
        }

        # UNPIN only
        if (   !defined $row->{version}     # no PIN version number
            &&  defined $row->{info}) {     # UNPIN version number
            $sql2 = 'SELECT * FROM PhysicalAction'
                    . ' WHERE physname="' .  $row->{physname} . '"'
                    . '      AND parentphys ISNULL'
                    . '      AND itemtype=2'
                    . '      AND timestamp<=' . $row->{timestamp}
                    . '      AND version>' . $row->{info}
                    . ' ORDER BY version ASC';
        }

        # UNPIN/PIN
        if (    defined $row->{version}     # PIN version number
            &&  defined $row->{info}) {     # UNPIN version number
            $sql2 = 'SELECT * FROM PhysicalAction'
                    . ' WHERE physname="' . $row->{physname} . '"'
                    . '      AND parentphys ISNULL'
                    . '      AND itemtype=2'
                    . '      AND version>' . $row->{info}
                    . '      AND version<=' . $row->{version}
                    . ' ORDER BY version ';

            if ($row->{info} > $row->{version}) {
                $sql2 .= "DESC";
                $prefix = "reverted changes for: \n";
            }
            else {
                $sql2 .= "ASC";
            }

        }

        next if !defined $sql2;

        my $sth2 = $gCfg{dbh}->prepare($sql2);
        $sth2->execute();

        my $comments = $sth2->fetchall_arrayref( {} );
        my $comment;
        print "merging comments for $row->{physname}" if $gCfg{verbose};
        print " from $row->{info}" if ($gCfg{verbose} && defined $row->{info});
        print " to $row->{version}" if ($gCfg{verbose} && defined $row->{version});
        print "\n" if $gCfg{verbose};

        foreach my $c(@$comments) {
            next unless $c->{comment};
    	    
            print " $c->{version}: $c->{comment}\n" if $gCfg{verbose};
            $comment .= $c->{comment} . "\n";
            $comment =~ s/\n+$/\n/;
        }

        if (defined $comment && !defined $row->{comment}) {
            $comment =~ s/^\n+//;
            $comment =~ s/\n+$//;

            $comment = $prefix . $comment if defined $prefix;
            push @updchild, [$comment, $row->{action_id}];
        }
    }

    init_progress 'Updating', @updchild;
    
    my $sql3 = 'UPDATE PhysicalAction SET comment = ? WHERE action_id = ?';
    my $sth3 = $gCfg{dbh}->prepare($sql3);

    foreach my $item (@updchild) {
        advance if ($progress++ % 1000) == 0;

        $sth3->execute(@$item);
    }

    end_progress;
    1;

}  #  End BuildComments

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
#  DeleteChildRecList
###############################################################################
sub DeleteChildRecList {
    my($idlst) = @_;

    my $sql = "DELETE FROM PhysicalAction WHERE action_id = ?";
    my $sth = $gCfg{dbh}->prepare($sql);
    
    init_progress 'Deleting', scalar(@$idlst);
    
    for my $id (@$idlst) {
        advance if ($progress++ % 1000) == 0;
        $sth->execute($id);
    }
}  #  End DeleteChildRecList

###############################################################################
#  FindForcedOrphans
###############################################################################
sub FindForcedOrphans {
    my ($forced_orphans, $total_count) = @_;

    my $sql = 'SELECT * FROM PhysicalAction ORDER BY timestamp ASC, '
            . 'itemtype ASC, priority ASC, parentdata ASC, sortkey ASC, action_id ASC';

    my $sth = $gCfg{dbh}->prepare($sql);

    $sth->execute();
    init_progress 'Collecting orphaned moves', $total_count;

    my @moves;

    ROW:
    while(my $row = $sth->fetchrow_hashref()) {
        advance if ($progress++ % 1000) == 0;

        my $action = $row->{actiontype};

        my $handler = Vss2Svn::ActionHandler->new($row);
        $handler->{verbose} = $gCfg{verbose};
        $handler->{trunkdir} = $gCfg{trunkdir};
        my $physinfo = $handler->physinfo();

        if (defined($physinfo) && $physinfo->{type} != $row->{itemtype} ) {
            next ROW;
        }

        $row->{itemname} = Encode::decode_utf8( $row->{itemname} );
        $row->{info} = Encode::decode_utf8( $row->{info} );
        $row->{comment} = Encode::decode_utf8( $row->{comment} );
        $row->{author} = Encode::decode_utf8( $row->{author} );
        $row->{label} = Encode::decode_utf8( $row->{label} );

        if (!$handler->handle($action)) {
            next ROW;
        }

        next ROW unless $handler->{action} eq 'MOVE';

        my $itempaths = $handler->{itempaths};
        my $info = $handler->{info};

        my $src_id;
        if ($info =~ /^\/orphaned\/_([A-Z]{8})/) {
            $src_id = $1;
        }

        my $tgt_id;
        if ($itempaths->[0] =~ /^\/orphaned\/_([A-Z]{8})/) {
            $tgt_id = $1;
        }

        if ($src_id || $tgt_id) {
            push @moves, [ $src_id, $info, $tgt_id, $itempaths->[0] ];
        }
    }

    end_progress;

    Vss2Svn::ActionHandler::ResetState();

    my @queue;
    for my $move (@moves) {
        print "Moved orphans $move->[1] to $move->[3]\n";

        push @queue, $move->[0]
            if defined $move->[0] && !defined $move->[2];
    }

    while (@queue) {
        my $id = shift @queue;
        next if $forced_orphans->{$id};

        print "Forcing inclusion of $id\n";
        $forced_orphans->{$id} = 1;

        for my $move (@moves) {
            push @queue, $move->[0]
                if defined $move->[0] && defined $move->[2] && $move->[2] eq $id;
        }
    }
}

###############################################################################
#  BuildVssActionHistory
###############################################################################
sub BuildVssActionHistory {
    my $total_count = $gCfg{dbh}->selectrow_array('SELECT COUNT(*) FROM PhysicalAction');

    my %forced_orphans;
    if ($gCfg{no_orphaned}) {
        FindForcedOrphans(\%forced_orphans, $total_count);

        if ($gCfg{prompt}) {
            print "Press ENTER to continue...\n";
            my $temp = <STDIN>;
            die if $temp =~ m/^quit/i;
        }
    }

    my $vsscache = Vss2Svn::DataCache->new('VssAction', 1)
        || &ThrowError("Could not create cache 'VssAction'");

    my $joincache = Vss2Svn::DataCache->new('SvnRevisionVssAction')
        || &ThrowError("Could not create cache 'SvnRevisionVssAction'");

    my $labelcache = Vss2Svn::DataCache->new('Label')
        || &ThrowError("Could not create cache 'Label'");

    # This will keep track of the current SVN revision, and increment it when
    # the author or comment changes, the timestamps span more than an hour
    # (by default), or the same physical file is affected twice

    my $svnrevs = Vss2Svn::SvnRevHandler->new()
        || &ThrowError("Could not create SVN revision handler");
    $svnrevs->{verbose} = $gCfg{verbose};

    my($sth, $row, $action, $handler, $physinfo, $itempaths, $allitempaths);

    my $sql = 'SELECT * FROM PhysicalAction ORDER BY timestamp ASC, '
            . 'itemtype ASC, priority ASC, parentdata ASC, sortkey ASC, action_id ASC';

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    init_progress 'Processing', $total_count;
    
ROW:
    while(defined($row = $sth->fetchrow_hashref() )) {
        advance if ($progress++ % 1000) == 0;
    
        $action = $row->{actiontype};

        $handler = Vss2Svn::ActionHandler->new($row);
        $handler->{verbose} = $gCfg{verbose};
        $handler->{trunkdir} = $gCfg{trunkdir};
        $physinfo = $handler->physinfo();

        if (defined($physinfo) && $physinfo->{type} != $row->{itemtype} ) {
            &ThrowWarning("Inconsistent item type for '$row->{physname}'; "
                        . "'$row->{itemtype}' unexpected");
            next ROW;
        }

        $row->{itemname} = Encode::decode_utf8( $row->{itemname} );
        $row->{info} = Encode::decode_utf8( $row->{info} );
        $row->{comment} = Encode::decode_utf8( $row->{comment} );
        $row->{author} = Encode::decode_utf8( $row->{author} );
        $row->{label} = Encode::decode_utf8( $row->{label} );

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

        # In cases of a corrupted share source, the handler may change the
        # action from 'SHARE' to 'ADD'
        $row->{actiontype} = $handler->{action};

        if (!defined $itempaths) {
            # Couldn't determine name of item
            &ThrowWarning($handler->{errmsg})
                if $handler->{errmsg};

            # If we were adding or modifying a file, commit it to lost+found;
            # otherwise give up on it
            if ($row->{itemtype} == 2 && ($row->{actiontype} eq 'ADD' ||
                $row->{actiontype} eq 'COMMIT')) {

                $itempaths = [undef];
            } else {
                next ROW;
            }
        }

        # May contain add'l info for the action depending on type:
        # RENAME: the new name (without path)
        # SHARE: the source path which was shared
        # MOVE: the old path
        # PIN: the path of the version that was pinned
        # LABEL: the name of the label
        $row->{info} = $handler->{info};

        # Drop labels if requested
        next ROW if $row->{actiontype} eq 'LABEL' && $gCfg{no_labels};

        # Drop orphaned files if requested
        if ($gCfg{no_orphaned} && @$itempaths) {
            $itempaths = [ grep { !($_ && /^\/orphaned\/_([A-Z]{8})/ && !$forced_orphans{$1}) } @$itempaths ];

            if ($row->{actiontype} =~ /^(ADD|COMMIT|RENAME|BRANCH|DELETE|RECOVER|LABEL)$/) {
                ;
            } elsif ($row->{actiontype} eq 'SHARE') {
                if ($row->{info} && $row->{info} =~ /^\/orphaned\/_([A-Z]{8})/ && !$forced_orphans{$1}) {
                    $row->{actiontype} = 'ADD';
                    undef $row->{info};
                }
            } elsif ($row->{actiontype} eq 'PIN') {
                if ($row->{info} && $row->{info} =~ /^\/orphaned\/_([A-Z]{8})/ && !$forced_orphans{$1}) {
                    $row->{actiontype} = 'COMMIT';
                    undef $row->{info};
                }
            } elsif ($row->{actiontype} eq 'MOVE') {
                if ($row->{info} && $row->{info} =~ /^\/orphaned\/_([A-Z]{8})/ && !$forced_orphans{$1}) {
                    print "WARNING: Converting orphaned MOVE into ADD - possible data loss.\n" if @$itempaths;
                    $row->{actiontype} = 'ADD';
                    undef $row->{info};
                } elsif (!@$itempaths) {
                    $row->{actiontype} = 'DELETE';
                    $itempaths = [ $row->{info} ];
                    undef $row->{info};
                }
            } else {
                die "Unknown action type $row->{actiontype}";
            }

            next ROW unless @$itempaths;
        }

        # we need to check for the next rev number, after all pathes that can
        # prematurally call the next row. Otherwise, we get an empty revision.
        $svnrevs->check($row, $itempaths);

        # The version may have changed
        if (defined $handler->{version}) {
            $row->{version} = $handler->{version};
        }

        $allitempaths = join("\t", @$itempaths);
        $row->{itempaths} = $allitempaths;

        $vsscache->add(@$row{ qw(parentphys physname version actiontype itempaths
                             itemtype is_binary info) });
        $joincache->add( $svnrevs->{revnum}, $vsscache->{pkey} );

        if (defined $row->{label}) {
            $labelcache->add(@$row{ qw(physname version label itempaths) });
        }

    }

    end_progress;

    Vss2Svn::ActionHandler::ResetState();

    $vsscache->commit();
    $svnrevs->commit();
    $joincache->commit();
    $labelcache->commit();

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

    my $file = $gCfg{dumpfile};
    open $fh, ">$file"
        or &ThrowError("Could not create dumpfile '$file'");

    my($sql, $sth, $action_sth, $row, $revision, $actions, $action, $physname, $itemtype);

    my %exported = ();
    my $total_count = $gCfg{dbh}->selectrow_array('SELECT COUNT(*) FROM SvnRevisionVssAction');

    $sql = 'SELECT * FROM SvnRevision ORDER BY revision_id ASC';

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    $sql = <<"EOSQL";
SELECT * FROM
    VssAction
WHERE action_id IN
    (SELECT action_id FROM SvnRevisionVssAction WHERE revision_id = ?)
ORDER BY action_id
EOSQL

    $action_sth = $gCfg{dbh}->prepare($sql);

    my $autoprops = Vss2Svn::Dumpfile::AutoProps->new($gCfg{auto_props}) if $gCfg{auto_props};
    my $labelmapper = Vss2Svn::Dumpfile::LabelMapper->new($gCfg{label_mapper}) if $gCfg{label_mapper};
    my $dumpfile = Vss2Svn::Dumpfile->new($fh, $autoprops, $gCfg{md5}, $labelmapper);
    Vss2Svn::Dumpfile->SetTempDir($gCfg{tempdir});

    init_progress 'Processing', $total_count;

REVISION:
    while(defined($row = $sth->fetchrow_hashref() )) {
        my $t0 = new Benchmark;

        $revision = $row->{revision_id};
        $dumpfile->begin_revision($row);

#        next REVISION if $revision == 0;

        $action_sth->execute($revision);
        $actions = $action_sth->fetchall_arrayref( {} );

ACTION:
        foreach $action(@$actions) {
    	    advance if ($progress++ % 200) == 0;

            $physname = $action->{physname};
            $itemtype = $action->{itemtype};

#            if (!exists $exported{$physname}) {
                my $version = $action->{version};
                if (   !defined $version
                    && (   $action->{action} eq 'ADD'
                        || $action->{action} eq 'COMMIT')) {
                    &ThrowWarning("'$physname': no version specified for retrieval");

                    # fall through and try with version 1.
                    $version = 1;
                }

                if ($itemtype == 2 && defined $version) {
                    $exported{$physname} = &ExportVssPhysFile($physname, $version);
                } else {
                    $exported{$physname} = undef;
                }
#            }

            # do_action needs to know the revision_id, so paste it on
            $action->{revision_id} = $revision;
            $dumpfile->do_action($action, $exported{$physname});
        }
        print "revision $revision: ", timestr(timediff(new Benchmark, $t0)),"\n"
            if $gCfg{timing};
    }

    end_progress;

    my @err = @{ $dumpfile->{errors} };

    if (scalar @err > 0) {
        map { &ThrowWarning($_) } @err;
    }

    $dumpfile->finish();
    close $fh;

}  #  End CreateSvnDumpfile

###############################################################################
#  ExportVssPhysFile
###############################################################################
sub ExportVssPhysFile {
    my($physname, $version) = @_;

    $physname =~ m/^((.).)/;

    my $exportdir = "$gCfg{vssdata}/$1";
    my @filesegment = &FindPhysnameFile("$gCfg{vssdir}/data", $2, $physname);

    if (!defined $filesegment[0] || !defined $filesegment[1] || !defined $filesegment[2]) {
        # physical file doesn't exist; it must have been destroyed later
        &ThrowWarning("Can't retrieve revisions from physical file "
                      . "'$physname'; it was either destroyed or corrupted");
        return undef;
    }
    my $physpath = "$filesegment[0]/$filesegment[1]/$filesegment[2]";

    if (! -f $physpath) {
        # physical file doesn't exist; it must have been destroyed later
        &ThrowWarning("Can't retrieve revisions from physical file "
                      . "'$physname'; it was either destroyed or corrupted");
        return undef;
    }

    mkpath($exportdir) if ! -e $exportdir;

    # MergeParentData normally will merge two corresponding item and parent
    # actions. But if the actions are more appart than the maximum allowed
    # timespan, we will end up with an undefined version in an ADD action here
    # As a hot fix, we define the version to 1, which will also revert to the
    # alpha 1 version behavoir.
    if (! defined $version) {
        &ThrowWarning("'$physname': no version specified for retrieval");

        # fall through and try with version 1.
        $version = 1;
    }

    if (! -e "$exportdir/$physname.$version" ) {
        &DoSsCmd("get -b -v$version --force-overwrite -e$gCfg{encoding} \"$physpath\" $exportdir/$physname");
    }

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
    my $auto_props = (!defined $gCfg{auto_props}) ? "" : $gCfg{auto_props};
    my $label_mapper = (!defined $gCfg{label_mapper}) ? "" : $gCfg{label_mapper};

    print <<"EOTXT";
======== VSS2SVN ========
$info
Start Time   : $starttime

VSS Dir      : $gCfg{vssdir}
Temp Dir     : $gCfg{tempdir}
Dumpfile     : $gCfg{dumpfile}
VSS Encoding : $gCfg{encoding}
Auto Props   : $auto_props
trunk dir    : $gCfg{trunkdir}
md5          : $gCfg{md5}
label dir    : $gCfg{labeldir}
label mapper : $label_mapper

VSS2SVN ver  : $VERSION
SSPHYS exe   : $gCfg{ssphys}
SSPHYS ver   : $ssversion
XML Parser   : $gCfg{xmlParser}

EOTXT

    my @version = split '\.', $ssversion;
    # we need at least ssphys 0.22
    if ($version[0] == 0 && $version[1] < 22) {
        &ThrowError("The conversion needs at least ssphys version 0.22");
    }

}  #  End ShowHeader

###############################################################################
#  ShowSummary
###############################################################################
sub ShowSummary {

    if (keys(%gErr) || $gCfg{resume}) {
       print <<"EOTXT";
=============================================================================
                               ERROR SUMMARY

EOTXT

        if($gCfg{resume}) {
            print <<"EOTXT";
**NOTICE** Because this run was resumed from a previous run, this may be only
a partial list; other errors may have been reported during previous run.

EOTXT
        }

        foreach my $task (@{ $gCfg{errortasks} }) {
            print "\n$task:\n   ";
            print join("\n   ", @{ $gErr{$task} }),"\n";
        }
    }

    print <<"EOTXT";
=============================================================================
                             END OF CONVERSION

The VSS to SVN conversion is complete. You should now use the "svnadmin load"
command to load the generated dumpfile '$gCfg{dumpfile}'. The "svnadmin"
utility is provided as part of the Subversion command-line toolset; use a
command such as the following:
    svnadmin load <repodir> < "$gCfg{dumpfile}"

You may need to precede this with "svnadmin create <repodir>" if you have not
yet created a repository. Type "svnadmin help <cmd>" for more information on
"create" and/or "load".

If any errors occurred during the conversion, they are summarized above.

For more information on the vss2svn project, see:
http://www.pumacode.org/projects/vss2svn/

EOTXT

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

    my($actions, $revisions, $mintime, $maxtime) = &GetStats();

    print <<"EOTXT";
Started at              : $starttime
Ended at                : $endtime
Elapsed time            : $elapsed (H:M:S)

VSS Actions read        : $actions
SVN Revisions converted : $revisions
Date range (YYYY/MM/DD) : $mintime to $maxtime

EOTXT

}  #  End ShowSummary

###############################################################################
#  GetStats
###############################################################################
sub GetStats {
    my($sql, $actions, $revisions, $mintime, $maxtime);

    $sql = <<"EOSQL";
SELECT
    COUNT(*)
FROM
    VssAction
EOSQL

    ($actions) = $gCfg{dbh}->selectrow_array($sql);

    $sql = <<"EOSQL";
SELECT
    COUNT(*)
FROM
    SvnRevision
EOSQL

    ($revisions) = $gCfg{dbh}->selectrow_array($sql);

    $sql = <<"EOSQL";
SELECT
    MIN(timestamp), MAX(timestamp)
FROM
    PhysicalAction
EOSQL

    ($mintime, $maxtime) = $gCfg{dbh}->selectrow_array($sql);

    foreach($mintime, $maxtime) {
        $_ = &Vss2Svn::Dumpfile::SvnTimestamp($_);
        s:T.*::;
        s:-:/:g;
    }

    # initial creation of the repo wasn't considered an action or revision
    return($actions - 1, $revisions - 1, $mintime, $maxtime);

}  #  End GetStats

###############################################################################
#  DoSsCmd
###############################################################################
sub DoSsCmd {
    my($cmd) = @_;

    my $ok = &DoSysCmd("\"$gCfg{ssphys}\" $cmd", 1);

    $gSysOut =~ s/\x00//g; # remove null bytes
    $gSysOut =~ s/.\x08//g; # yes, I've seen VSS store backspaces in names!
    # allow all characters in the windows-1252 codepage: see http://de.wikipedia.org/wiki/Windows-1252
    $gSysOut =~ s/[\x00-\x09\x11\x12\x14-\x1F\x81\x8D\x8F\x90\x9D]/_/g; # just to be sure

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
        &ThrowWarning(sprintf "FAILED with non-zero exit status %d (cmd: %s)", $? >> 8, $cmd);
        die unless $allowfail;

        $rv = 0;
    }

    return $rv;

}  #  End DoSysCmd

###############################################################################
#  GetSsVersion
###############################################################################
sub GetSsVersion {
    my $out = `\"$gCfg{ssphys}\" --version 2>&1`;
    # Build numbers look like:
    #  a.) ssphys 0.20.0, Build 123
    #  b.) ssphys 0.20.0, Build 123:150
    #  c.) ssphys 0.20.0, Build 123:150 (locally modified)
    $out =~ m/^ssphys (.*?), Build (.*?)[ \n]/m;

    # turn it into
    #  a.) 0.20.0.123
    #  b.) 0.20.0.123:150
    #  c.) 0.20.0.123:150
    return $1 . "." . $2 || 'unknown';
}  #  End GetSsVersion

###############################################################################
#  ThrowWarning
###############################################################################
sub ThrowWarning {
    my($msg, $callinfo) = @_;

    $callinfo ||= [caller()];

    $msg .= "\nat $callinfo->[1] line $callinfo->[2]";

    warn "ERROR -- $msg\n";

    my $task = $gCfg{task};

    if(!defined $gErr{$task}) {
        $gErr{$task} = [];
        push @{ $gCfg{errortasks} }, $task;
    }

    push @{ $gErr{$task} }, $msg;

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

    $gCfg{ssphys} = 'ssphys' if !defined($gCfg{ssphys});
    $gCfg{vssdatadir} = "$gCfg{vssdir}/data";

    (-d "$gCfg{vssdatadir}") or &ThrowError("$gCfg{vssdir} does not appear "
                                            . "to be a valid VSS database");

    &SetupActionTypes;

    Vss2Svn::DataCache->SetCacheDir($gCfg{tempdir});
    Vss2Svn::DataCache->SetDbHandle($gCfg{dbh});
    Vss2Svn::DataCache->SetVerbose($gCfg{verbose});

    Vss2Svn::SvnRevHandler->SetRevTimeRange($gCfg{revtimerange})
        if defined $gCfg{revtimerange};

}  #  End SetupGlobals

###############################################################################
#  SetupActionTypes
###############################################################################
sub SetupActionTypes {
    # RollBack is only seen in combiation with a BranchFile activity, so actually
    # RollBack is the item view on the activity and BranchFile is the parent side
    # ==> map RollBack to BRANCH, so that we can join the two actions in the
    # MergeParentData step
    # RestoredProject seems to act like CreatedProject, except that the
    # project was recreated from an archive file, and its timestamp is
    # the time of restoration. Timestamps of the child files retain
    # their original values.
    %gActionType = (
        CreatedProject => {type => 1, action => 'ADD'},
        AddedProject => {type => 1, action => 'ADD'},
        RestoredProject => {type => 1, action => 'RESTOREDPROJECT'},
        RenamedProject => {type => 1, action => 'RENAME'},
        MovedProjectTo => {type => 1, action => 'MOVE_TO'},
        MovedProjectFrom => {type => 1, action => 'MOVE_FROM'},
        DeletedProject => {type => 1, action => 'DELETE'},
        DestroyedProject => {type => 1, action => 'DELETE'},
        RecoveredProject => {type => 1, action => 'RECOVER'},
        ArchiveProject => {type => 1, action => 'DELETE'},
        RestoredProject => {type => 1, action => 'RESTORE'},
        CheckedIn => {type => 2, action => 'COMMIT'},
        CreatedFile => {type => 2, action => 'ADD'},
        AddedFile => {type => 2, action => 'ADD'},
        RenamedFile => {type => 2, action => 'RENAME'},
        DeletedFile => {type => 2, action => 'DELETE'},
        DestroyedFile => {type => 2, action => 'DELETE'},
        RecoveredFile => {type => 2, action => 'RECOVER'},
        ArchiveVersionsofFile => {type => 2, action => 'ADD'},
    ArchiveVersionsofProject => {type => 1, action => 'ADD'},
        ArchiveFile => {type => 2, action => 'DELETE'},
        RestoredFile => {type => 2, action => 'RESTORE'},
        SharedFile => {type => 2, action => 'SHARE'},
        BranchFile => {type => 2, action => 'BRANCH'},
        PinnedFile => {type => 2, action => 'PIN'},
        RollBack => {type => 2, action => 'BRANCH'},
        UnpinnedFile => {type => 2, action => 'PIN'},
        Labeled => {type => 2, action => 'LABEL'},
    );

}  #  End SetupActionTypes

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
        label       VARCHAR,
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
        parentphys  VARCHAR,
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
CREATE INDEX
    VssAction_IDX1 ON VssAction (
        action_id   ASC
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

    $sql = <<"EOSQL";
CREATE INDEX
    SvnRevisionVssAction_IDX1 ON SvnRevisionVssAction (
        revision_id ASC,
        action_id   ASC
    )
EOSQL

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute;

    $sql = <<"EOSQL";
CREATE TABLE
    Label (
        physical VARCHAR,
        version  INTEGER,
        label    VARCHAR,
        imtempaths  VARCHAR
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
    $| = 1;

    GetOptions(\%gCfg,'vssdir=s','tempdir=s','dumpfile=s','resume','verbose',
               'reuse_cache','prompt','no_orphaned','no_labels',
               'debug','timing+','task=s','revtimerange=i','ssphys=s',
               'encoding=s','trunkdir=s','auto_props=s', 'label_mapper=s', 'md5');

    &GiveHelp("Must specify --vssdir") if !defined($gCfg{vssdir});
    $gCfg{tempdir} = './_vss2svn' if !defined($gCfg{tempdir});
    $gCfg{dumpfile} = 'vss2svn-dumpfile.dat' if !defined($gCfg{dumpfile});

    if (defined($gCfg{auto_props}) && ! -r $gCfg{auto_props}) {
        die "auto_props file '$gCfg{auto_props}' is not readable";
    }

    if (defined($gCfg{label_mapper}) && ! -r $gCfg{label_mapper}) {
        die "label_mapper file '$gCfg{label_mapper}' is not readable";
    }

    $gCfg{sqlitedb} = "$gCfg{tempdir}/vss_data.db";

    # XML output from ssphysout placed here.
    $gCfg{ssphysout} = "$gCfg{tempdir}/ssphysout";
    $gCfg{encoding} = 'windows-1252' if !defined($gCfg{encoding});

    # Commit messages for SVN placed here.
    $gCfg{svncomment} = "$gCfg{tempdir}/svncomment.tmp.txt";
    mkdir $gCfg{tempdir} unless (-d $gCfg{tempdir});

    # Directories for holding VSS revisions
    $gCfg{vssdata} = "$gCfg{tempdir}/vssdata";

    if ($gCfg{resume} && !-e $gCfg{sqlitedb}) {
        warn "WARNING: --resume set but no database exists; starting new "
            . "conversion...";
        $gCfg{resume} = 0;
    }

    if ($gCfg{debug}) {
        $gCfg{verbose} = 1;
    }
    $gCfg{timing} = 0 unless defined $gCfg{timing};
    $gCfg{md5} = 0 unless defined $gCfg{md5};

    $gCfg{starttime} = scalar localtime($^T);

    # trunkdir should (must?) be without trailing slash
    $gCfg{trunkdir} = '' unless defined $gCfg{trunkdir};
    $gCfg{trunkdir} =~ s:\\:/:g;
    $gCfg{trunkdir} =~ s:/$::;

    $gCfg{junkdir} = '/lost+found';

    $gCfg{labeldir} = '/labels';

    $gCfg{errortasks} = [];

    {
        no warnings 'once';
        $gCfg{usingExe} = (defined($PerlApp::TOOL));
    }

    &ConfigureXmlParser();

    ### Don't go past here if resuming a previous run ###
    if ($gCfg{resume}) {
        return 1;
    }

    rmtree($gCfg{vssdata}) if (-e $gCfg{vssdata} && !$gCfg{reuse_cache});
    mkdir $gCfg{vssdata};

    $gCfg{ssphys} ||= 'ssphys';
    $gCfg{svn} ||= 'SVN.exe';

    $gCfg{task} = 'INIT';
    $gCfg{step} = 0;
}  #  End Initialize

###############################################################################
#  ConfigureXmlParser
###############################################################################
sub ConfigureXmlParser {

    if(defined($ENV{XML_SIMPLE_PREFERRED_PARSER})) {
        # user has defined a preferred parser; don't mess with it
        $gCfg{xmlParser} = $ENV{XML_SIMPLE_PREFERRED_PARSER};
        return 1;
    }

    $gCfg{xmlParser} = 'XML::Simple';

    eval { require XML::SAX; };

    if($@) {
        # no XML::SAX; let XML::Simple use its own parser
        return 1;
    }
    elsif($gCfg{usingExe}) {
        # Prevent the ParserDetails.ini error message when running from .exe
        XML::SAX->load_parsers($INC[1]);
    }

    $gCfg{xmlParser} = 'XML::SAX::Expat';
    $XML::SAX::ParserPackage = $gCfg{xmlParser};

    my $p;

    eval { $p = XML::SAX::ParserFactory->parser(); };

    if(!$@) {
        # XML::SAX::Expat installed; use it

        # for exe version, XML::Parser::Expat needs help finding its encmaps
        no warnings 'once';

        my $encdir;
        foreach my $dir (@INC) {
            $encdir = "$dir/encodings";
            $encdir =~ s:\\:/:g;
            $encdir =~ s://:/:g;
            if(-d $encdir) {
                print "Adding '$encdir' to encodings file path\n";
                push(@XML::Parser::Expat::Encoding_Path, $encdir);
            }
        }

        return 1;
    }

    undef $XML::SAX::ParserPackage;
    eval { $p = XML::SAX::ParserFactory->parser(); };

    if(!$@) {
        $gCfg{xmlParser} = ref $p;
        return 1;
    }

    # couldn't find a better package; go back to XML::Simple
    $gCfg{'xmlParser'} = 'XML::Simple';
    return 1;

}  #  End ConfigureXmlParser

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
    --ssphys <path>   : Full path to ssphys.exe program; uses PATH otherwise
    --tempdir <dir>   : Temp directory to use during conversion;
                        default is ./_vss2svn
    --dumpfile <file> : specify the subversion dumpfile to be created;
                        default is ./vss2svn-dumpfile.dat
    --revtimerange <sec> : specify the difference between two ss actions
                           that are treated as one subversion revision;
                           default is 3600 seconds (== 1hour)

    --resume          : Resume a failed or aborted previous run
    --task <task>     : specify the task to resume; task is one of the following
                        INIT, LOADVSSNAMES, FINDDBFILES, GETPHYSHIST,
                        MERGEPARENTDATA, MERGEMOVEDATA, REMOVETMPCHECKIN,
                        MERGEUNPINPIN, BUILDACTIONHIST, IMPORTSVN
    --reuse_cache     : Rebuild the database, but reuse text temporary files.
                        May be useful if the database becomes corrupt due to 
                        an unexpected power failure. Make sure to remove any
                        incomplete files beforehand.

    --verbose         : Print more info about the items being processed
    --debug           : Print lots of debugging info.
    --timing          : Show timing information during various steps
    --encoding        : Specify the encoding used in VSS;
                        Default is windows-1252
    --trunkdir        : Specify where to map the VSS Project Root in the
                        converted repository (default = "/")
    --auto_props      : Specify an autoprops ini file to use, e.g.
                        --auto_props="c:/Dokumente und Einstellungen/user/Anwendungsdaten/Subversion/config"
    --md5             : generate md5 checksums
    --label_mapper    : INI style file to map labels to different locataions
    --no_orphaned     : Do not generate the orphaned cache
    --no_labels       : Do not convert labels
EOTXT

    exit(1);
}  #  End GiveHelp
