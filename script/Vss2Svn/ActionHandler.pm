package Vss2Svn::ActionHandler;

use warnings;
use strict;

our %gHandlers =
    (
     ADD        => \&_add_handler,
     COMMIT     => \&_commit_handler,
     RENAME     => \&_rename_handler,
     SHARE      => \&_share_handler,
     BRANCH     => \&_branch_handler,
     MOVE       => \&_move_handler,
     RESTORE    => \&_restore_handler,
     RESTOREDPROJECT    => \&_restore_handler,
     DELETE     => \&_delete_handler,
     RECOVER    => \&_recover_handler,
     PIN        => \&_pin_handler,
     LABEL      => \&_label_handler,
    );

our(%gPhysInfo);
our(%gOrphanedInfo);

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $row) = @_;

    my $self =
        {
         row => $row,
         action => undef,
         info => undef,
         version => undef,
         errmsg => '',
         itempaths => undef,
         recursed => 0,
         physname_seen => '',
         verbose => 0,
         trunkdir => '',
        };

    return bless($self, $class);
}  #  End new

###############################################################################
#  handle
###############################################################################
sub handle {
    my($self, $action) = @_;

    $self->{action} = $action;
    my $handler = $gHandlers{$action};

    if (!defined($handler)) {
        $self->{errmsg} .= "Unknown action '$action'";
        return 0;
    }

    if ($self->{verbose}) {
        my $physprint = (defined $self->{row}->{physname})?
            $self->{row}->{physname} : '!UNDEF';
        my $parentprint = (defined $self->{row}->{parentphys})?
            $self->{row}->{parentphys} : '!UNDEF';
        print "$action: $physprint, $parentprint \@ $self->{row}->{timestamp}\n";
    }

    my $rv = $self->$handler;

    $self->{errmsg} =~ s/\n$//;
    return $rv;

}  #  End handle

###############################################################################
#  physinfo
###############################################################################
sub physinfo {
    my($self) = @_;

    return $gPhysInfo{ $self->{row}->{physname} };
}  #  End physinfo

###############################################################################
#  _add_handler
###############################################################################
sub _add_handler {
    my($self) = @_;
    my $row = $self->{row};

    # For each physical item, we store its "real" physical parent in the
    # 'parentphys' property, then keep a list of additional shared parents in
    # the 'sharedphys' array.

    my $parentphys = $row->{parentphys};
    my ($orphaned);
    
    if (!defined $parentphys) {
        # '_' is used as a magic marker for orphaned files
        $row->{parentphys} = '_' . $row->{physname};
#        $row->{itemname} = $row->{physname} . '_' . $row->{itemname};
        $orphaned = 1;
    }

    # the version number could have been changed by the share handler
    # or in the branch handler, this is the version we branch.
    my $version = defined $row->{version} ? $row->{version}
                    : $self->{version};
    
    # if the item to be added was destroyed, then we don't have a version
    # number here. So we don't need to add the item anyway.
    if (!defined $version ) {
        $self->{errmsg} .= "Attempt to add entry '$row->{physname}' with "
            . "unknown version number (probably destroyed)\n";
        
        $gOrphanedInfo {$row->{physname} } = 1;
        return 0;
    }
    
    $gPhysInfo{ $row->{physname} } =
        {
         type       => $row->{itemtype},
         name       => $row->{itemname},
#         parentphys => $row->{parentphys},
#         sharedphys => [],
         parents    => {},
         last_version => $version,
         orphaned   => $orphaned,
         was_binary => $row->{is_binary},
        };

    $self->_add_parent ($row->{physname}, $row->{parentphys});
    $self->_track_item_paths ($version);

    # File was just created so no need to look for shares
    $self->{itempaths} = [$self->_get_current_item_path()];

    # don't convert orphaned items
#    return $orphaned ? 0 : 1;
    return 1;
}  #  End _add_handler

###############################################################################
#  _commit_handler
###############################################################################
sub _commit_handler {
    my($self) = @_;
    my $row = $self->{row};

    my $physname = $row->{physname};
    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        $self->{errmsg} .= "Attempt to commit unknown item '$physname':\n"
            . "$self->{physname_seen}\n";

        return 0;
    }

    $physinfo->{was_binary} = $row->{is_binary};

    # We need to track at least the version number, even if there is no
    # active parent. This is necessary, if we later share this item, we need
    # to share from the latest seen version.
    
    # remember the last version, in which the file was modified
    $physinfo->{last_version} = $row->{version};
    
    # and track all itempaths for the new version
    $self->_track_item_paths ($row->{version});
    
    my $itempaths = $self->_get_active_item_paths();
    if (!defined $itempaths && defined $physinfo->{orphaned}) {
        $self->{errmsg} .= "No more active itempath to commit to orphaned item '$physname':\n"
            . "$self->{physname_seen}\n";

        return 0;
    }
        
    $self->{itempaths} = $itempaths;
    
    if (!defined $self->{itempaths}) {
        $self->{errmsg} .= "No more active itempath to commit to '$physname':\n"
            . "$self->{physname_seen}\n";

        return 0;
    }
    
    return 1;
}  #  End _commit_handler

###############################################################################
#  _rename_handler
###############################################################################
sub _rename_handler {
    my($self) = @_;
    my $row = $self->{row};

    my $physname = $row->{physname};
    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        # only report an error, if the file wasn't detected as orphaned. 
        if (!defined $gOrphanedInfo {$physname}) {
            $self->{errmsg} .= "Attempt to rename unknown item '$physname':\n"
                . "$self->{physname_seen}\n";
        }
        
        return 0;
    }

    # Get the existing paths before the rename; info will contain the new name
    my $itempaths = $self->_get_vivid_item_paths();

    # Renames on shares may show up once for each share, which we don't want
    # since one rename takes care of all locations. If the "new" name is
    # already the same as the old, just ignore it.
    if ($physinfo->{name} eq $row->{info}) {
        return 0;
    }

    # A rename of an item renames it in all its shares
    $physinfo->{name} = $row->{info};

    # no need to track the itempathes, since a rename doesn't create a new
    # item version

    $self->{itempaths} = $itempaths;
    $self->{info} = $row->{info};

    return 1;
}  #  End _rename_handler

###############################################################################
#  _share_handler
###############################################################################
sub _share_handler {
    my($self) = @_;
    my $row = $self->{row};

    my $physname = $row->{physname};
    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        $self->{errmsg} .= "Attempt to share unknown item '$physname':\n"
            . "$self->{physname_seen}\n";

        return 0;
    }

#    # if this is not a share+pin action, then add this item to the sharedphys
#    # list. Otherwise, this item is pinned to a specific version and does not
#    # participate in shared actions
#    if (!defined $row->{version}) {
#        push @{ $physinfo->{sharedphys} }, $row->{parentphys};
#    }
    
    my $version = $row->{version};
    $version = $physinfo->{last_version} if (!defined $version);

    $row->{is_binary} = $physinfo->{was_binary};

    # 'itempath' is the path for this new location (the share target);
    # note: since we can share from a orphaned item, we use the itemname that
    # is provided in the row information for the share target and not the
    # current name of the item. The orphaned name is mangeled to make it unique
    my $parentpath = $self->_get_current_parent_path ();
    my $itempath = $parentpath . $row->{itemname};

    # 'sourceinfo' contains the source path
    my $sourceinfo = $self->_get_valid_path ($physname, $row->{parentphys}, $version);

    if (!defined($sourceinfo)) {
        # We can't figure out the path for the parent that this share came from,
        # so it was either destroyed or corrupted. That means that this isn't
        # a share anymore; it's a new add.

        $self->{action} = 'ADD';
#        $self->{version} = $version;
#        return $self->_add_handler();
    }

    # track the addition of the new parent
    $self->_add_parent ($physname, $row->{parentphys});
    
    # if this is a share+pin action, then remember the pin version
    if (defined $row->{version}) {
        $physinfo->{parents}->{$row->{parentphys}}->{pinned} = $row->{version};
    } 

    $self->{itempaths} = [$itempath];
    $self->{info} = $sourceinfo;
    $self->{version} = $version;

    # the share target is now also a valid "copy from" itempath
    $self->_track_item_path ($physname, $row->{parentphys}, $version, $itempath);

    return 1;
}  #  End _share_handler

###############################################################################
#  _branch_handler
###############################################################################
sub _branch_handler {
    my($self) = @_;
    my $row = $self->{row};

    # Branching a file is actually a null action in SVN; it simply means we
    # stop duplicating checkins. Return the existing path, but internally
    # we'll remove this parent from the list of shared physical parents from
    # the old location, then create a new one with the pertinent info. The row's
    # 'physname' is that of the new file; 'info' is the formerly shared file.

    my $physname = $row->{physname};
    my $oldphysname = $row->{info};

    my $oldphysinfo = $gPhysInfo{$oldphysname};

    if (!defined $oldphysinfo) {
        $self->{errmsg} .= "Attempt to branch unknown item '$oldphysname':\n"
            . "$self->{physname_seen}\n";

        return 0;
    }
    
#    # First delete this parentphys from the old shared object; see
#    # _delete_handler for details
#    if ($oldphysinfo->{parentphys} eq $row->{parentphys}) {
#        $oldphysinfo->{parentphys} = shift( @{ $oldphysinfo->{sharedphys} } );
#    } else {
#        my $sharedphys = [];
#
#        foreach my $oldparent (@{ $oldphysinfo->{sharedphys} }) {
#            push @$sharedphys, $oldparent
#                unless $oldparent eq $row->{parentphys};
#        }
#
#        $oldphysinfo->{sharedphys} = $sharedphys;
#    }

    my $version = defined $row->{version} ? $row->{version}
                    : $self->{version};

    # if we branch into a destroyed object, delete is the logical choice
    if (!defined $version ) {
      $self->{errmsg} .= "Attempt to branch '$oldphysname' into "
			 . "'$physname' at an unknown version number "
                         . "('$physname' probably destroyed)\n";
      $gOrphanedInfo{$physname} = 1;
      $self->{action} = 'DELETE';
      $row->{physname} = $oldphysname;
      $row->{info} = undef;
      return $self->_delete_handler();
    }

    # treat the old path as deleted
    # we can't branch an item, that doesn't have a parent. This happens when the
    # parent was destroyed. 
    if (defined $row->{parentphys}) {
        $oldphysinfo->{parents}->{$row->{parentphys}}->{deleted} = 1;
    }
    else {
        # since we have the "orphaned" handling, we can map this action to an
        # addition, so that this item will show up in the orphaned cache.
        # TODO: To keep the history of the item we can try to ShareBranch
        # from original item if it is also somewhere accessible.
        # something like:
#        my $copypath = $self->_get_valid_path ($oldphysinfo, $row->{parentphys}, $row->{version});
        
        $self->{action} = 'ADD';
    }

    # Now treat the new entry as a new addition
    return $self->_add_handler();

#    # Now create a new entry for this branched item
#    $gPhysInfo{$physname} =
#        {
#         type       => $row->{itemtype},
#         name       => $row->{itemname},
##         parentphys => $row->{parentphys},
##         sharedphys => [],
#         parents    => {},
#        };

#    $self->_add_parent ($physname, $row->{parentphys});
#    $self->{itempaths} = $self->_get_current_item_paths(1);

#    return 1;

}  #  End _branch_handler

###############################################################################
#  _move_handler
###############################################################################
sub _move_handler {
    my($self) = @_;
    my $row = $self->{row};

    # Get the existing paths before the move; parent sub will get the new
    # name
    my $physname = $row->{physname};
    my $physinfo = $gPhysInfo{$physname};
  
    if (!defined $physinfo) {
        $self->{errmsg} .= "Attempt to move unknown item '$physname':\n"
            . "$self->{physname_seen}\n";

        return 0;
    }

    if (!defined $row->{parentphys}) {
      # Check if this is an orphaned item
      if (scalar @{$physinfo->{order}} == 1) {
        $row->{parentphys} = $physinfo->{order}[0];
      } else {
        # Don't know from where to move. Share it there instead
        $row->{parentphys} = $row->{info};
        $row->{info} = undef;
        $self->{action} = 'SHARE';
        return $self->_share_handler();
      }
    }

    # '$sourceinfo' is the path for the old location (the move source);
    my $parentpath = $self->_get_current_parent_path ();
    my $sourceinfo = $parentpath . $physinfo->{name}; # $row->{itemname};

    if (!defined ($row->{info})) {
        # the target directory was destroyed, so there is no apropriate move
        # target information. Fall back to a move to the orphaned cache
        $row->{info} = '_' . $row->{physname};
    }

    # '$itempath' contains the move target path
    my $itempath = $self->_get_parent_path ($row->{info}) . $row->{itemname};

    if (!defined($parentpath)) {
        # We can't figure out the path for the parent that this move came from,
        # so it was either destroyed or corrupted. That means that this isn't
        # a move anymore; it's a new add.

        $self->{action} = 'ADD';
#        $self->{version} = $version;
#        return $self->_add_handler();
        
        # we need to swap the source and the target path
        $sourceinfo = $itempath;
        undef $itempath;
    }
    else {
        # set the old parent inactive
        $physinfo->{parents}->{$row->{parentphys}}->{deleted} = 1;
    }

    # if the item mysteriously changed name during the move
    $physinfo->{name} = $row->{itemname};

    # track the addition of the new parent
    $self->_add_parent ($physname, $row->{info});
    
    $self->{itempaths} = [$sourceinfo];
    $self->{info} = $itempath;

    # the move target is now also a valid "copy from" itempath
    $self->_track_item_path ($physname, $row->{parentphys}, $physinfo->{last_version}, $itempath);

    return 1;
}  #  End _move_handler

###############################################################################
#  _restore_handler
###############################################################################
sub _restore_handler {
    my($self) = @_;
    my $row = $self->{row};
    
    $self->{action} = 'MOVE';
    $row->{actiontype} = 'MOVE';
    $row->{info} = $row->{parentphys};
    $row->{parentphys} = '_' . $row->{physname};
    return $self->_move_handler ();
}

###############################################################################
#  _delete_handler
###############################################################################
sub _delete_handler {
    my($self) = @_;
    my $row = $self->{row};

    # For a delete operation we return the path of the item to be deleted

    my $physname = $row->{physname};
    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        # only report an error, if the file wasn't detected as orphaned. 
        if (!defined $gOrphanedInfo {$physname}) {
            $self->{errmsg} .=  "Attempt to delete unknown item '$physname':\n"
                . "$self->{physname_seen}\n";
        }
        return 0;
    }

    my $parentpath = $self->_get_current_parent_path ();
    my $itempaths = [$parentpath . $physinfo->{name}];

#    if ($physinfo->{parentphys} eq $row->{parentphys}) {
#        # Deleting from the "main" parent; find a new one by shifting off the
#        # first shared path, if any; if none exists this will leave a null
#        # parent entry. We could probably just delete the whole node at this
#        # point.
#
#        $physinfo->{parentphys} = shift( @{ $physinfo->{sharedphys} } );
#
#    } else {
#        my $sharedphys = [];
#
#        foreach my $parent (@{ $physinfo->{sharedphys} }) {
#            push @$sharedphys, $parent
#                unless $parent eq $row->{parentphys};
#        }
#
#        $physinfo->{sharedphys} = $sharedphys;
#    }

    # protect for delete/purge cycles: if the parentphys isn't in the shares
    # anymore, the file was already deleted from the parent and is now purged
    if (defined $physinfo->{parents}->{$row->{parentphys}}->{deleted}) {
        return 0;
    }

    # set the parent inactive
    $physinfo->{parents}->{$row->{parentphys}}->{deleted} = 1;

    $self->{itempaths} = $itempaths;

    return 1;

}  #  End _delete_handler

###############################################################################
#  _recover_handler
###############################################################################
sub _recover_handler {
    my($self) = @_;
    my $row = $self->{row};

    my $physname = $row->{physname};
    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        # only report an error, if the file wasn't detected as orphaned. 
        if (!defined $gOrphanedInfo {$physname}) {
            $self->{errmsg} .= "Attempt to recover unknown item '$physname':\n"
                . "$self->{physname_seen}\n";
        }
        
        return 0;
    }

#    if (defined $physinfo->{parentphys}) {
#        # Item still has other shares, so recover it by pushing this parent
#        # onto its shared list
#
#        push( @{ $physinfo->{sharedphys} }, $row->{parentphys} );
#
#    } else {
#        # Recovering its only location; set the main parent back to this
#        $physinfo->{parentphys} = $row->{parentphys};
#    }

    # recover this item within the current parent
    my $parentinfo = $physinfo->{parents}->{$row->{parentphys}};
    if (!defined $parentinfo->{deleted}) {
        $self->{errmsg} .= "Attempt to recover an active item '$physname':\n"
            . "$self->{physname_seen}\n";

        return 0;
    }
    undef $parentinfo->{deleted};
    
    # We only recover the path explicitly set in this row, so build the path
    # ourself by taking the path of this parent and appending the name
    my $parentpath = $self->_get_current_parent_path();
    my $itempath = $parentpath . $physinfo->{name};

    # Since the item could be modified between the delete and the recovery,
    # we need to find a valid source for the recover
    $self->{info} =
        $self->_get_valid_path ($physname, $row->{parentphys}, $row->{version});
    $self->{itempaths} = [$itempath];
    
    # We only set the version number, if this item is a file item. If it is a
    # project item, we must recover from the last known revision, which is
    # determined in the dumpfile handler
    if ($row->{itemtype} == 2) {
        $self->{version} = $physinfo->{last_version};
    }

    return 1;
}  #  End _recover_handler

###############################################################################
#  _pin_handler
###############################################################################
sub _pin_handler {
    my($self) = @_;
    my $row = $self->{row};

    my $physname = $row->{physname};
    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        $self->{errmsg} .= "Attempt to pin unknown item '$physname':\n"
            . "$self->{physname_seen}\n";

        return 0;
    }

    my $parentpath = $self->_get_current_parent_path();
    my $itempath = $parentpath . $physinfo->{name};

    my $parentinfo = \%{$physinfo->{parents}->{$row->{parentphys}}};

    my $version = $row->{version};
    if (!defined $row->{version}) {
        # this is the unpin handler
        undef $parentinfo->{pinned};
        $version = $physinfo->{last_version};
    }
    else {
        $parentinfo->{pinned} = $row->{version};
    }
    
    $self->{itempaths} = [$itempath];
    $self->{info} =
        $self->_get_valid_path ($physname, $row->{parentphys}, $row->{version});
    $self->{version} = $version;
    
    # the unpinned target is now also a valid "copy from" itempath
    $self->_track_item_path ($physname, $row->{parentphys}, $version, $itempath);

    return 1;
}  #  End _pin_handler

###############################################################################
#  _label_handler
###############################################################################
sub _label_handler {
    # currently the handler only tracks labels that where assigned to files
    # we need this for the item name tracking
    my($self) = @_;
    my $row = $self->{row};

    my $itempaths = $self->_get_active_item_paths();

    $self->_track_item_paths ($row->{version});
    
    $self->{itempaths} = $itempaths;
    $self->{info} = $row->{label};
    
    return 1;
}  #  End _label_handler

###############################################################################
#  _get_current_parent_path
###############################################################################
sub _get_current_parent_path {
    my($self) = @_;

    return $self->_get_parent_path($self->{row}->{parentphys});
}  #  End _get_current_parent_path


###############################################################################
#  _get_parent_path
###############################################################################
sub _get_parent_path {
    my($self, $physname) = @_;

    # Uses recursion to determine the current full paths for an item based on
    # the name of its physical file. We can't cache this information because
    # a rename in a parent folder would not immediately trigger a rename in
    # all of the child items.

    # By default, we return an anonymous array of all paths in which the item
    # is shared, unless $mainonly is true. Luckily, only files can be shared,
    # not projects, so once we start recursing we can set $mainonly to true.

    if ($self->{verbose}) {
        my $physprint = (defined $physname)? $physname : '!UNDEF';
        my $space = ($self->{recursed})? '   ' : '';
        print "${space}_get_parent_path($physprint)\n";
    }

    if (++($self->{recursed}) >= 1000) {
        $self->{errmsg} .= "Infinite recursion detected while looking up "
            . "parent for '$physname':\n$self->{physname_seen}\n";

        return undef;
    }

    if (!defined($physname)) {
        return undef;
    }

    if ($physname eq '') {
        return '';
    }
    
    if ($physname eq 'AAAAAAAA') {
        # End of recursion; all items must go back to 'AAAAAAAA', which was so
        # named because that's what most VSS users yell after using it much. :-)
        return $self->{trunkdir} . '/';
    }

    if ($physname =~ m/^_.*/) {
        # End of recursion; this is the orphaned node
        # return the name of the orphaned directory + the name of the orphaned
        # file in order to make the path unique
        return '/orphaned/' . $physname . '/';
    }

    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        $self->{errmsg} .= "Could not determine real path for '$physname':\n"
            . "$self->{physname_seen}\n";

        return undef;
    }

    #todo: make the behavoir of orphaned file tracking configurable
#    if ($physinfo->{orphaned}) {
#        return undef;
#    }

    $self->{physname_seen} .= "$physname, ";

    # In a move szenario, we can have one deleted and one active parent. We
    # are only interested in the active ones here.
    my @pathstoget = $self->_get_active_parents ($physname);

    # TODO: For projects there should be only one active parent
    my $parent = $pathstoget[0];
    
    # if we don't have any active parents, the item path itself is deleted
    if (!defined ($parent)) {
        return undef;
    }

    my $result;

    $result = $self->_get_parent_path($pathstoget[0], 1);

    if(!defined($result)) {
        return undef;
    }

    return $result . $physinfo->{name};

}  #  End _get_parent_path

###############################################################################
#  _get_current_item_paths
###############################################################################
sub _get_current_item_paths {
    my($self, $mainonly) = @_;
    
    my @parents = $self->_get_parents ($self->{row}->{physname});
    return $self->_get_item_paths($self->{row}->{physname}, @parents);
}  #  End _get_current_item_paths

###############################################################################
#  _get_vivid_item_paths
###############################################################################
sub _get_vivid_item_paths {
    my($self, $mainonly) = @_;
    
    my @parents = $self->_get_vivid_parents ($self->{row}->{physname});
    return $self->_get_item_paths($self->{row}->{physname}, @parents);
}  #  End _get_vivid_item_paths

###############################################################################
#  _get_active_item_paths
###############################################################################
sub _get_active_item_paths {
    my($self, $mainonly) = @_;
    
    my @parents = $self->_get_active_parents ($self->{row}->{physname});
    return $self->_get_item_paths($self->{row}->{physname}, @parents);
}  #  End _get_active_item_paths

###############################################################################
#  _get_current_item_path
###############################################################################
sub _get_current_item_path {
    my($self) = @_;

    my @parents = $self->_get_parents ($self->{row}->{physname});
    
    if (scalar @parents == 0) {
        return undef;
    }
    
    my $physname = $self->{row}->{physname};
    my $paths = $self->_get_item_paths($physname, $parents[0]);
    
    if (!defined $paths) {
        $self->{errmsg} .=  "Could not retrieve item path for '$physname': "
            . "(probably bogous timestamp in parent and child action)\n";
        return undef;
    }
    
    return $paths->[0];
}  #  End _get_current_item_path

###############################################################################
#  _get_item_paths
###############################################################################
sub _get_item_paths {
    my($self, $physname, @parents) = @_;

    # Uses recursion to determine the current full paths for an item based on
    # the name of its physical file. We can't cache this information because
    # a rename in a parent folder would not immediately trigger a rename in
    # all of the child items.

    # By default, we return an anonymous array of all paths in which the item
    # is shared, unless $mainonly is true. Luckily, only files can be shared,
    # not projects, so once we start recursing we can set $mainonly to true.

    if ($self->{verbose}) {
        my $physprint = (defined $physname)? $physname : '!UNDEF';
        my $space = ($self->{recursed})? '   ' : '';
        print "${space}_get_item_paths($physprint)\n";
    }


    if (!defined($physname)) {
        return undef;
    }

    if ($physname eq 'AAAAAAAA') {
        # End of recursion; all items must go back to 'AAAAAAAA', which was so
        # named because that's what most VSS users yell after using it much. :-)
        return [$self->{trunkdir} . '/'];
    }

    if ($physname =~ m/^_.*/) {
        # End of recursion; this is the orphaned node
        # return the name of the orphaned directory + the name of the orphaned
        # file in order to make the path unique
        return '/orphaned/' . $physname . '/';
    }

    my $physinfo = $gPhysInfo{$physname};

    if (!defined $physinfo) {
        $self->{errmsg} .= "Could not determine real path for '$physname':\n"
            . "$self->{physname_seen}\n";

        return undef;
    }

    #todo: make the behavoir of orphaned file tracking configurable
#    if ($physinfo->{orphaned})
#    {
#        return undef;
#       }

    $self->{physname_seen} .= "$physname, ";

#    my @pathstoget =
#        ($physinfo->{parentphys}, @{ $physinfo->{sharedphys} } );
    my @pathstoget = @parents;
    
    my $paths;
    my $result;
#    if (defined $physinfo->{parents}->{$row->{parentphys}}->{deleted}) {
#        return 0;
#    }

PARENT:
    foreach my $parent (@pathstoget) {
        if (!defined $parent) {
            next PARENT;
        }
        $result = $self->_get_parent_path($parent);

        if(!defined($result)) {
            next PARENT;
        }

        push @$paths, $result . $physinfo->{name};
    }

    return $paths;

}  #  End _get_item_paths



###############################################################################
#  _track_item_paths:
# This function maintains a map that records the itempath that was valid for
# each version of the physical file in the context of the different parents.
# This map is needed, e.g. during pinning when a file is pinned to a previous
# version. Since the file, could have renamed in between, we need to know the
# previous itempath that was valid in the previous version.
#
# This map does not replace the recursive lookup of the itempath in teh function
# _get_item_paths. The itempathes stored here are "historic" item pathes.
# A rename e.g. is not reflectected in the version history of the physical file
# and therefor does not have a distinct version as in subversion. 
###############################################################################
sub _track_item_paths {
    my($self, $version) = @_;

    my $row = $self->{row};
    
    # we only need to track the path for actions that deal with a specific
    # version
    if (defined $version) {

        my $physinfo = $gPhysInfo{ $row->{physname} };
        
        my @parents = $self->_get_active_parents ($row->{physname});
        my $result;

PARENT:
        foreach my $parent (@parents) {

            my $parentpath = $self->_get_parent_path ($parent);
            if (!defined $parentpath) {
                next PARENT;
            }
            $result = $parentpath . $physinfo->{name};

            $self->_track_item_path ($row->{physname}, $parent, $row->{version}, $result);

#            my $versions = \@{$physinfo->{parents}->{$parent}->{versions}};
#            
#            # in the case of pinning and sharing with pinning, the version number
#            # denotes a version in the past. So if there is already an entry for
#            # this version number skip this parent.
#            if (exists $versions->[$row->{version}]) {
#                next PARENT;
#            }
            
#            # remember the last version, in which the file was modified
#            $physinfo->{last_version} = $row->{version};
            
#            $result = $self->_get_parent_path ($parent) . $physinfo->{name};
    
#            if(!defined($result)) {
#                next PARENT;
#            }
    
#            $versions->[$row->{version}] = $result;
        }
    }
}  #  End _track_item_paths


###############################################################################
#  _track_item_path:
###############################################################################
sub _track_item_path {
    my($self, $physname, $parent, $version, $itempath) = @_;

    if (defined $version && defined $itempath) {

        my $physinfo = $gPhysInfo{ $physname };
        
        my $versions = \@{$physinfo->{parents}->{$parent}->{versions}};
        
        # in the case of pinning and sharing with pinning, the version number
        # denotes a version in the past. So if there is already an entry for
        # this version number skip this parent.
        if (exists $versions->[$version]) {
            return;
        }

        $versions->[$version] = $itempath;
    }
}  #  End _track_item_path


###############################################################################
#  _get_vivid_parents
# This function returns all parents where the physical file is not deleted,
# r all active projects. If a file is deleted, the file
# does nor take place in any further rename activity, so it is
# inactive.
###############################################################################
sub _get_vivid_parents {
    my($self, $physname) = @_;

    my $physinfo = $gPhysInfo{$physname};

    my @parents;
    if (defined $physinfo) {

PARENT:
        foreach my $parentphys (@{$physinfo->{order}}) {

            # skip orphaned parents
#            if ($parentphys eq '99999999' ) {
#                next PARENT;
#            }

            my $parent = $physinfo->{parents}->{$parentphys};
            if (!defined $parent)
            {
                next PARENT;
            }
            
            # skip deleted parents, since these parents do not
            # participate in specific vss action
            if (defined $parent->{deleted} ) {
                next PARENT;
            }

            push @parents, $parentphys;
        }
    }
    
    return @parents
}  # End _get_vivid_parents

###############################################################################
#  _get_active_parents
# This function returns all parents where the physical file is not deleted
# or pinned, or all active projects. If a file is pinned or deleted, the file
# does nor take place in any further checkin or rename activity, so it is
# inactive.
###############################################################################
sub _get_active_parents {
    my($self, $physname) = @_;

    my $physinfo = $gPhysInfo{$physname};

    my @parents;
    if (defined $physinfo) {

PARENT:
        foreach my $parentphys (@{$physinfo->{order}}) {

            # skip orphaned parents
#            if ($parentphys eq '99999999' ) {
#                next PARENT;
#            }

            my $parent = $physinfo->{parents}->{$parentphys};
            if (!defined $parent)
            {
                next PARENT;
            }
            
            # skip deleted or pinned parents, since these parents do not
            # participate in any vss action
            if (defined $parent->{deleted} || defined $parent->{pinned} ) {
                next PARENT;
            }

            push @parents, $parentphys;
        }
    }
    
    return @parents
}  # End _get_active_parents

###############################################################################
#  _get_parents
# This function returns all parents for the physical file
###############################################################################
sub _get_parents {
    my($self, $physname) = @_;

    my $physinfo = $gPhysInfo{$physname};

    my @parents;
    if (defined $physinfo) {

PARENT:
        foreach my $parentphys (@{$physinfo->{order}}) {

            # skip orphaned parents
#            if ($parentphys eq '99999999' ) {
#                next PARENT;
#            }

            my $parent = $physinfo->{parents}->{$parentphys};
            if (!defined $parent)
            {
                next PARENT;
            }
            
            push @parents, $parentphys;
        }
    }
    
    return @parents
}  # End _get_active_parents

###############################################################################
#  _get_valid_path
# This function returns an itempath for the physical file, that was valid in
# the previous version. Since all activities that create a new version of a file
# must be done on at least one active path, there should be at least one valid
# item path for the version.
# If we can't find any valid itempath, we can not perform a "copy from" revision
# In this case, we need to recheckin the current content of the item
###############################################################################
sub _get_valid_path {
    my($self, $physname, $parentphys, $version) = @_;

    my $physinfo = $gPhysInfo{$physname};
    if (!defined $physinfo) {
        return undef;
    }

    if (!defined $version) {
        $version = $physinfo->{last_version};
    }
    
    # 1. check the parent requested, if there was an item name for this version
    # we can use this item name, since it was valid in that time
    my $parent = $physinfo->{parents}->{$parentphys};
    if (defined $parent &&
#        $parentphys ne '99999999' &&
        $parent->{versions}->[$version]) {
        return $parent->{versions}->[$version];
    }
    
    # 2. check all other parents in the order, the where added
    my @parents;

PARENT:
    foreach $parentphys (@{$physinfo->{order}}) {

        $parent = $physinfo->{parents}->{$parentphys};
        if (defined $parent &&
#            $parentphys ne '99999999' &&
            $parent->{versions}->[$version]) {
            return $parent->{versions}->[$version];
        }
    }
    
    return undef;
}  #  End _get_valid_path

###############################################################################
#  _add_parent
# Track the addition of a new parent to this itempath. This will also track the
# order, in which the parents where added to the physical file. The valid
# itempath lookup will search for valid pathes in the order the parents where
# added to the project.
###############################################################################
sub _add_parent {
    my($self, $physname, $parentphys) = @_;

    my $physinfo = $gPhysInfo{$physname};
    if (defined $physinfo) {
        $physinfo->{parents}->{$parentphys} = {};
        push @{ $physinfo->{order} }, $parentphys;
    }
}  #  End _add_parent

1;
