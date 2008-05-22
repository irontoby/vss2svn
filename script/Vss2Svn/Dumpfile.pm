package Vss2Svn::Dumpfile;

use Vss2Svn::Dumpfile::Node;
use Vss2Svn::Dumpfile::SanityChecker;
use Vss2Svn::Dumpfile::AutoProps;
use Vss2Svn::Dumpfile::LabelMapper;

require Time::Local;

use warnings;
use strict;

use File::Copy;
use Digest::MD5;
use Data::UUID;

our %gHandlers =
    (
     ADD        => \&_add_handler,
     COMMIT     => \&_commit_handler,
     RENAME     => \&_rename_handler,
     SHARE      => \&_share_handler,
     BRANCH     => \&_branch_handler,
     ROLLBACK   => \&_branch_handler,
     MOVE       => \&_move_handler,
     DELETE     => \&_delete_handler,
     RECOVER    => \&_recover_handler,
     PIN        => \&_pin_handler,
     LABEL      => \&_label_handler,
    );

# Keep track of when paths were modified or deleted, for subsequent copies
# or recovers.

#our %gModified = ();
our %gDeleted = ();
our %gVersion = ();
our $gTmpDir;

###############################################################################
#  SetTempDir
###############################################################################
sub SetTempDir {
    my($class, $dir) = @_;

    $gTmpDir = $dir;
}  #  End SetTempDir

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $fh, $autoprops, $md5, $labelmapper) = @_;

    my $self =
        {
         fh => $fh,
         revision => 0,
         errors => [],
         deleted_cache => {},
         version_cache => [],
         repository => Vss2Svn::Dumpfile::SanityChecker->new(),
         auto_props => $autoprops,
         do_md5 => $md5,
         label_mapper => $labelmapper,
        };

    # prevent perl from doing line-ending conversions
    binmode($fh);

    my $old = select($fh);
    $| = 1;
    select($old);

    print $fh "SVN-fs-dump-format-version: 2\n\n";
    
    my $ug    = new Data::UUID;
    my $uuid = $ug->to_string( $ug->create() );
    
    print $fh "UUID: $uuid\n\n";

    $self = bless($self, $class);
    return $self;

}  #  End new

###############################################################################
#  finish
###############################################################################
sub finish {
    my($self) = @_;

    my $fh = $self->{fh};

    print $fh "\n\n";

}  #  End finish

###############################################################################
#  begin_revision
###############################################################################
sub begin_revision {
    my($self, $data) = @_;
    my($revision, $author, $timestamp, $comment) =
        @{ $data }{qw(revision_id author timestamp comment)};

    my $props = undef;
    my $fh = $self->{fh};

    print $fh "\nRevision-number: $revision\n";

    $comment = '' if !defined($comment);
    $author = '' if !defined($author);

    if ($revision > 0) {
        $props = { 'svn:log' => $comment,
                   'svn:author' => $author,
                 };
    }

    $props->{'svn:date'} = $self->svn_timestamp($timestamp);

    $self->output_content($props);
    $self->{revision} = $revision;

}  #  End begin_revision

###############################################################################
#  do_action
###############################################################################
sub do_action {
    my($self, $data, $expdir) = @_;

    my $action = $data->{action};

    my $nodes = [];

    # Temporary hack to prevent shared files from stepping on the "modified"
    # flag for other than the first commit. Ideally, we should keep all paths
    # for a given physical file's last modified flags, and use the best match
    # if we need to copy or recover one.

    $self->{is_primary} = 1;
    $self->{deleted_cache} = {};
    $self->{version_cache} = [];

    my($handler, $this_action);

    foreach my $itempath (split "\t", $data->{itempaths}) {
        $this_action = $action;

#        $this_action = $self->sanity_checker->check ($data, $itempath, $nodes);
#        if (!defined ($this_action)) {
#            return 0;
#        }

        $handler = $gHandlers{$this_action};

        my $thisnodes = [];
        $self->$handler($itempath, $thisnodes, $data, $expdir);

        # we need to apply all local changes to our repository directly: if we
        # have an action that operates on multiple items, e.g labeling, the
        # necessary missing directories are created for the first item
        foreach my $node (@$thisnodes) {
            $self->{repository}->load($node);
            push @$nodes, $node;
        }
        
        $self->{is_primary} = 0;
    }

    foreach my $node (@$nodes) {
        $self->output_node($node);
    }

    my($physname, $cache);
 
    my ($parentphys, $physnames);
    while(($parentphys, $physnames) = each %{ $self->{deleted_cache} }) {
        while(($physname, $cache) = each %{ $physnames }) {
            $gDeleted{$parentphys}->{$physname} = $cache;
        }
    }

    # track the version -> revision mapping for the file
    foreach my $record (@{$self->{version_cache}}) {
        my $version = \%{$gVersion{$record->{physname}}->[$record->{version}]};
        $version->{$record->{itempath}} = $record->{revision};
    }

}  #  End do_action


###############################################################################
#  _add_handler
###############################################################################
sub _add_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    if ($self->{repository}->exists ($itempath)) {
        if ($data->{itemtype} == 2) {
            $self->add_error("Attempt to re-add file '$itempath' at "
                . "revision $data->{revision_id}, changing to modify; possibly "
                . "missing delete");
            return $self->_commit_handler ($itempath, $nodes, $data, $expdir);
        }
        else {
            #creating a new VSS database can cause a "ADD" for a "/" item which will fail.
            if (!($itempath eq "/")) {
        	$self->add_error("Attempt to re-add directory '$itempath' at "
                    . "revision $data->{revision_id}, skipping action: possibly "
                    . "missing delete");
            }
            
            return 0;
        }
    }

    my $success = $self->{repository}->exists_parent ($itempath);
    if(!defined($success)) {
        $self->add_error("Path consistency failure while trying to add "
            . "item '$itempath' at revision $data->{revision_id}; skipping");
        return 0;
    }
    elsif ($success == 0) {
    	if (!($itempath =~ m/^\/orphaned\/_.*/))
    	{
	    $self->add_error("Parent path missing while trying to add "
	        . "item '$itempath' at revision $data->{revision_id}: adding missing "
	        . "parents");
	}
        $self->_create_svn_path ($nodes, $itempath);
    }
    
    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    if ($data->{is_binary}) {
        $node->add_prop('svn:mime-type', 'application/octet-stream');
    }
    if (defined $self->{auto_props}) {
        $node->add_props ($self->{auto_props}->get_props ($itempath));
    }

    $node->{action} = 'add';

    if ($data->{itemtype} == 2) {
        $self->get_export_file($node, $data, $expdir);
    }

#    $self->track_modified($data->{physname}, $data->{revision_id}, $itempath);
    $self->track_version ($data->{physname}, $data->{version}, $itempath);

    push @$nodes, $node;

}  #  End _add_handler

###############################################################################
#  _commit_handler
###############################################################################
sub _commit_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    if (!$self->{repository}->exists ($itempath)) {
        $self->add_error("Attempt to commit to non-existant file '$itempath' at "
            . "revision $data->{revision_id}, changing to add; possibly "
            . "missing recover");
        return $self->_add_handler ($itempath, $nodes, $data, $expdir);
    }
    
    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'change';

    if ($data->{itemtype} == 2) {
        $self->get_export_file($node, $data, $expdir);
    }

#    $self->track_modified($data->{physname}, $data->{revision_id}, $itempath);
    $self->track_version ($data->{physname}, $data->{version}, $itempath);

    push @$nodes, $node;

}  #  End _commit_handler

###############################################################################
#  _rename_handler
###############################################################################
sub _rename_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    # to rename a file in SVN, we must add "with history" then delete the orig.

    my $newname = $data->{info};
    my $newpath = $itempath;

    if ($data->{itemtype} == 1) {
        $newpath =~ s:(.*/)?.+$:$1$newname:;
    } else {
        $newpath =~ s:(.*/)?.*:$1$newname:;
    }

    if ($self->{repository}->exists ($newpath)) {
        $self->add_error("Attempt to rename item '$itempath' to '$newpath' at "
            . "revision $data->{revision_id}, but destination already exists: possibly "
            . "missing delete; skipping");
        return 0;
    }

    if (!$self->{repository}->exists ($itempath)) {
        $self->add_error("Attempt to rename item '$itempath' to '$newpath' at "
            . "revision $data->{revision_id}, but source doesn't exists: possibly "
            . "missing recover; skipping");
        return 0;
    }

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($newpath, $data);
    # change the properties according to the new name
    if (defined $self->{auto_props}) {
        $node->add_props ($self->{auto_props}->get_props ($newpath));
    }
    $node->{action} = 'add';

    my($copyrev, $copypath);

    # ideally, we should be finding the last time the file was modified and
    # copy it from there, but that becomes difficult to track...
    $copyrev = $data->{revision_id} - 1;
    $copypath = $itempath;

    $node->{copyrev} = $copyrev;
    $node->{copypath} = $copypath;

    push @$nodes, $node;

#    $self->track_modified($data->{physname}, $data->{revision_id}, $newpath);
#    $self->track_version ($data->{physname}, $data->{version}, $newpath);

    $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'delete';
    $node->{hideprops} = 1;

    push @$nodes, $node;

    # We don't add this to %gDeleted since VSS doesn't treat a rename as an
    # add/delete and therefore we wouldn't recover from this point

}  #  End _rename_handler

###############################################################################
#  _share_handler
###############################################################################
sub _share_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    if ($self->{repository}->exists ($itempath)) {
        $self->add_error("Attempt to share item '$data->{info}' to '$itempath' at "
            . "revision $data->{revision_id}, but destination already exists: possibly "
            . "missing delete; skipping");
        return 0;
    }

#   It could be possible that we share from a historically renamed item, so we don't check the source
#    if ($self->{repository}->exists ($data->{info})) {
#        $self->add_error("Attempt to share item '$itempath' to '$newpath' at "
#            . "revision $data->{revision_id}, but destination already exists: possibly "
#            . "missing delete; skipping");
#        return 0;
#    }

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

#    @{ $node }{ qw(copyrev copypath) }
#        = $self->last_modified_rev_path($data->{physname});
    $node->{copyrev} =
        $self->get_revision ($data->{physname}, $data->{version}, $data->{info});
    $node->{copypath} = $data->{info};

    if (!defined $node->{copyrev} || !defined $node->{copypath}) {
        return $self->_commit_handler ($itempath, $nodes, $data, $expdir);
    }

    $self->track_version ($data->{physname}, $data->{version}, $itempath);

    push @$nodes, $node;

}  #  End _share_handler

###############################################################################
#  _branch_handler
###############################################################################
sub _branch_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    # branching is a no-op in SVN
    #   - unless it is a ROLLBACK

    # since it is possible, that we refer to version prior to the branch later, we
    # need to copy all internal information about the ancestor to the child.
    if (defined $data->{info}) {
        # only copy versions, that are common between the branch source and the branch.
        my $copy_version=$data->{version};
        while(--$copy_version > 0) {
            if (defined $gVersion{$data->{info}}->[$copy_version]) {
                $gVersion{$data->{physname}}->[$copy_version] =
                   $gVersion{$data->{info}}->[$copy_version];
           }
       }
    }

    # handle rollback, which changes active revision simultaneously with branching
    if ($data->{action} eq 'ROLLBACK') {
        return $self->_commit_handler ($itempath, $nodes, $data, $expdir);
    }

#    # if the file is copied later, we need to track, the revision of this branch
#    # see the shareBranchShareModify Test
#    $self->track_modified($data->{physname}, $data->{revision_id}, $itempath);
    $self->track_version ($data->{physname}, $data->{version}, $itempath);

}  #  End _branch_handler

###############################################################################
#  _move_handler
###############################################################################
sub _move_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    # moving in SVN is the same as renaming; add the new and delete the old

    my $oldpath = $data->{info};

    if ($self->{repository}->exists ($itempath)) {
        $self->add_error("Attempt to move item '$oldpath' to '$itempath' at "
            . "revision $data->{revision_id}, but destination already exists: possibly "
            . "missing delete; skipping");
        return 0;
    }

    if (!$self->{repository}->exists ($oldpath)) {
        $self->add_error("Attempt to move item '$oldpath' to '$itempath' at "
            . "revision $data->{revision_id}, but source doesn't exists: possibly "
            . "missing recover; skipping");
        return 0;
    }

    my $success = $self->{repository}->exists_parent($itempath);
    if(!defined($success)) {
        $self->add_error("Attempt to move item '$oldpath' to '$itempath' at "
            . "revision $data->{revision_id}, but path consistency failure at dest");
        return 0;
    }
    elsif ($success == 0) {
        $self->add_error("Parent path missing while trying to move "
            . "item '$oldpath' to '$itempath' at "
            . "revision $data->{revision_id}: adding missing parents");
        $self->_create_svn_path ($nodes, $itempath);
    }

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

    my($copyrev, $copypath);

    $copyrev = $data->{revision_id} - 1;
    $copypath = $oldpath;

    $node->{copyrev} = $copyrev;
    $node->{copypath} = $copypath;

    push @$nodes, $node;

    # the new move target is a valid path.
    $self->track_version ($data->{physname}, $data->{version}, $itempath);

    $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($oldpath, $data);
    $node->{action} = 'delete';
    $node->{hideprops} = 1;

#   Deleted tracking is only necessary to be able to recover the item. But a move
#   does not set a recover point, so we don't need to track the delete here. Additionally
#   we do not have enough information for this operation.
#   $self->track_deleted($data->{oldparentphys}, $data->{physname},
#                        $data->{revision_id}, $oldpath);

    push @$nodes, $node;

}  #  End _move_handler

###############################################################################
#  _delete_handler
###############################################################################
sub _delete_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    if (!$self->{repository}->exists ($itempath)) {
        $self->add_error("Attempt to delete non-existent item '$itempath' at "
            . "revision $data->{revision_id}: possibly "
            . "missing recover/add/share; skipping");
        return 0;
    }

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'delete';
    $node->{hideprops} = 1;

    push @$nodes, $node;

    $self->track_deleted($data->{parentphys}, $data->{physname},
                         $data->{revision_id}, $itempath);

}  #  End _delete_handler

###############################################################################
#  _recover_handler
###############################################################################
sub _recover_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    if ($self->{repository}->exists ($itempath)) {
        $self->add_error("Attempt to recover existing item '$itempath' at "
            . "revision $data->{revision_id}: possibly "
            . "missing delete; change to commit");
        return $self->_commit_handler ($itempath, $nodes, $data, $expdir);
    }

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

    # for projects we want to go back to the revision just one before the deleted
    # revision. For files, we need to go back to the specified revision, since
    # the file could have been modified via a share.
    my($copyrev, $copypath);
    if (!defined ($data->{version})) {
        ($copyrev, $copypath)= $self->last_deleted_rev_path($data->{parentphys},
                                                            $data->{physname});
        $copyrev -= 1;
    }
    else {
        $copyrev =
            $self->get_revision ($data->{physname}, $data->{version}, $data->{info});
        $copypath = $data->{info};
    }
    
    if (!defined $copyrev || !defined $copypath) {
        $self->add_error(
            "Could not recover path $itempath at revision $data->{revision_id};"
            . " unable to determine deleted revision or path");
        return 0;
    }

    $node->{copyrev} = $copyrev;
    $node->{copypath} = $copypath;

    if (defined ($data->{version})) {
        $self->track_version ($data->{physname}, $data->{version}, $itempath);
    }

    push @$nodes, $node;

}  #  End _recover_handler

###############################################################################
#  _pin_handler
###############################################################################
sub _pin_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    if (!$self->{repository}->exists ($itempath)) {
        $self->add_error("Attempt to pin non-existing item '$itempath' at "
            . "revision $data->{revision_id}: possibly "
            . "missing recover; skipping");
        return 0;
    }

    my $copyrev = 
        $self->get_revision ($data->{physname}, $data->{version}, $data->{info});
    my $copypath = $data->{info};
    
    # if one of the necessary copy from attributes are unavailable we fall back
    # to a complete checkin
    if (defined $copyrev && defined $copypath) {
        $data->{comment} = "ported from $copypath r$copyrev";
    }
#    if (!defined $copyrev || !defined $copypath) {
        return $self->_commit_handler ($itempath, $nodes, $data, $expdir);
#    }
    
    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

    $node->{copyrev} = $copyrev;
    $node->{copypath} = $copypath;

    $self->track_version ($data->{physname}, $data->{version}, $itempath);

    push @$nodes, $node;

}  #  End _pin_handler

###############################################################################
#  _label_handler
###############################################################################
sub _label_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    if (!$self->{repository}->exists ($itempath)) {
        $self->add_error("Attempt to label non-existing item '$itempath' at "
            . "revision $data->{revision_id}: possibly "
            . "missing recover; skipping");
        return 0;
    }

    my $label = $data->{info};

    # It is possible that the label was deleted later, so we see here a label
    # action, but no label was assigned. In this case, we only need to track
    # the version->revision mapping, since the version could have been used
    # as a valid share source.
    if (defined ($label)) {
        my $labeldir = $main::gCfg{labeldir};
        
        if (defined $self->{label_mapper}) {
            $labeldir = $self->{label_mapper}->remap ($main::gCfg{labeldir}, $label);
        }
        $labeldir =~ s:\\:/:g;
        $labeldir =~ s:/$::;
        
        $label =~ s![\\/:*?"<>|]!_!g;
        
        my $vssitempath = $itempath;
        $vssitempath =~ s/^$main::gCfg{trunkdir}//;
        my $labelpath = "$labeldir/$label$vssitempath";

        $self->_create_svn_path ($nodes, $labelpath);

        my $node = Vss2Svn::Dumpfile::Node->new();
        $node->set_initial_props($labelpath, $data);
        $node->{action} = 'add';
    
        my $copyrev = $data->{revision_id} - 1;
        my $copypath = $itempath;

        $node->{copyrev} = $copyrev;
        $node->{copypath} = $copypath;

        push @$nodes, $node;
        
    }

    $self->track_version ($data->{physname}, $data->{version}, $itempath);
}  #  End _label_handler

###############################################################################
#  _add_svn_dir
###############################################################################
sub _add_svn_dir {
    my($self, $nodes, $dir) = @_;

    my $node = Vss2Svn::Dumpfile::Node->new();
    my $data = { itemtype => 1, is_binary => 0 };

    $node->set_initial_props($dir, $data);
    $node->{action} = 'add';

    push @$nodes, $node;
}  #  End _add_svn_dir


###############################################################################
#  _create_svn_path
###############################################################################
sub _create_svn_path {
    my($self, $nodes, $itempath) = @_;

    my $missing_dirs = $self->{repository}->get_missing_dirs($itempath);

    foreach my $dir (@$missing_dirs) {
        $self->_add_svn_dir($nodes, $dir);
    }
}  #  End _create_svn_path

###############################################################################
#  track_version
###############################################################################
sub track_version {
    my($self, $physname, $version, $itempath) = @_;

    my $record = 
        {
         physname => $physname,
         version => $version,
         revision => $self->{revision},
         itempath => $itempath,
        };
    push @{$self->{version_cache}}, $record;

}  #  End track_version


###############################################################################
#  get_revision
###############################################################################
sub get_revision {
    my($self, $physname, $version, $itempath) = @_;

    if (!defined($gVersion{$physname})) {
        return (undef);
    }

    if (!exists($gVersion{$physname}->[$version])) {
        return (undef);
    }
    
    return $gVersion{$physname}->[$version]->{$itempath};

}  #  End get_revision

###############################################################################
#  track_deleted
###############################################################################
sub track_deleted {
    my($self, $parentphys, $physname, $revision, $path) = @_;

    $self->{deleted_cache}->{$parentphys}->{$physname} =
        {
         revision => $revision,
         path => $path,
        };

}  #  End track_deleted

###############################################################################
#  last_deleted_rev_path
###############################################################################
sub last_deleted_rev_path {
    my($self, $parentphys, $physname) = @_;

    if (!defined($gDeleted{$parentphys})) {
        return (undef, undef);
    }

    if (!defined($gDeleted{$parentphys}->{$physname})) {
        return (undef, undef);
    }

    return @{ $gDeleted{$parentphys}->{$physname} }{ qw(revision path) };
}  #  End last_deleted_rev_path

###############################################################################
#  get_export_file
###############################################################################
sub get_export_file {
    my($self, $node, $data, $expdir) = @_;

    if (!defined($expdir)) {
        return 0;
    } elsif (!defined($data->{version})) {
        $self->add_error(
            "Attempt to retrieve file contents with unknown version number");
        return 0;
    }

    $node->{file} = "$expdir/$data->{physname}.$data->{version}";
    return 1;

}  #  End get_export_file

###############################################################################
#  output_node
###############################################################################
sub output_node {
    my($self, $node) = @_;
    my $fh = $self->{fh};

    # only in an add or rename action the propery array is set. So we have
    # to lookup the eol-style flag again. The best thing is to query the
    # property always temporarirly
    my %tmpProps = ();
    if (defined $self->{auto_props}) {
        %tmpProps = $self->{auto_props}->get_props ($node->{path});
    }
    my $eolStyle = $tmpProps{'svn:eol-style'};
    my $isNative = (defined $eolStyle && $eolStyle eq 'native') ? 1 : 0;

    my $string = $node->get_headers();
    print $fh $string;
    $self->output_content($node->{hideprops}? undef : $node->{props},
                          $node->{text}, $node->{file}, $isNative);
}  #  End output_node

###############################################################################
#  output_content
###############################################################################
sub output_content {
    my($self, $props, $text, $file, $isNative) = @_;

    my $fh = $self->{fh};

    $text = '' unless defined $text || defined $file;

    my $proplen = 0;
    my $textlen = 0;
    my($propout, $textout) = ('') x 2;

    if (defined($props)) {
        foreach my $key (keys %$props) {
            my $value = $props->{$key};
            $propout .= 'K ' . length($key) . "\n$key\n";
            if (defined $value) {
                $propout .= 'V ' . length($value) . "\n$value\n";
            }
            else {
                $propout .= "V 0\n\n";
            }
        }

        $propout .= "PROPS-END\n";
        $proplen = length($propout);
    }

    my $md5;
    $md5 = Digest::MD5->new if $self->{do_md5};

    # prevent errors due to non existing files
    if(!defined $text && defined $file && !-e $file) {
        $text = "";
    }

    # convert CRLF -> LF before calculating the size and compute the md5
    if(!defined $text && defined $file) {
            
        my ($input, $output);
        if (defined $isNative && $isNative) {
            open ($input, "<:crlf", $file);
            my $tmpFile = "$gTmpDir/crlf_to_lf.tmp.txt";
            open ($output, ">", $tmpFile);
            binmode ($output);

            while(<$input>) {
                $md5->add($_) if $self->{do_md5};
                print $output $_;
            }
            
            close $input;
            close $output;
            $file = $tmpFile;
        }
        else {
            open ($input, "<", $file);
            binmode ($input);
            $md5->addfile($input) if $self->{do_md5};
            close $input;            
        }
    } else {
        $md5->add($text) if $self->{do_md5};
    }       
    
    my $digest = $md5->hexdigest if $self->{do_md5};
#    print "digest: $digest\n";

    if(!defined $text && defined $file) {
        $textlen = -s $file;
    } else {
        $textlen = length($text);
    }
    return if ($textlen + $proplen == 0 && !defined $file);

    if ($proplen > 0) {
        print $fh "Prop-content-length: $proplen\n";
    }

    if (defined $file || $textlen > 0) {
        print $fh "Text-content-length: $textlen\n";
        print $fh "Text-content-md5: $digest\n" if $self->{do_md5};
    }

    print $fh "Content-length: " . ($proplen + $textlen)
        . "\n\n$propout";

    if(!defined $text && defined $file) {
        copy($file, $fh);
        print $fh "\n";
    } else {
        print $fh "$text\n";
    }

}  #  End output_content

###############################################################################
#  svn_timestamp
###############################################################################
sub svn_timestamp {
    my($self, $vss_timestamp) = @_;

    return &SvnTimestamp($vss_timestamp);

}  #  End svn_timestamp

###############################################################################
#  SvnTimestamp
###############################################################################
sub SvnTimestamp {
    my($vss_timestamp) = @_;

    # set the correct time: VSS stores the local time as the timestamp, but subversion
    # needs a gmtime. So we need to reverse adjust the timestamp in order to turn back
    # the clock.
    my($sec, $min, $hour, $day, $mon, $year) = gmtime($vss_timestamp);
    my($faketime) = Time::Local::timelocal ($sec, $min, $hour, $day, $mon, $year);
    ($sec, $min, $hour, $day, $mon, $year) = gmtime($faketime);

    $year += 1900;
    $mon += 1;

    return sprintf("%4.4i-%2.2i-%2.2iT%2.2i:%2.2i:%2.2i.%6.6iZ",
        $year, $mon, $day, $hour, $min, $sec, 0);

}  #  End SvnTimestamp

###############################################################################
#  add_error
###############################################################################
sub add_error {
    my($self, $msg) = @_;

    push @{ $self->{errors} }, $msg;
}  #  End add_error



1;
