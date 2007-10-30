package Vss2Svn::Dumpfile::SanityChecker;

use warnings;
use strict;

###############################################################################
#  new
###############################################################################
sub new {
    my($class) = @_;

    my $self =
        {
         svn_items => {},
         junk_itempaths => {},
         need_junkdir => 0,
         need_missing_dirs => [],
         deleted => {},
        };

    $self = bless($self, $class);
    return $self;

}  #  End new


###############################################################################
#  sanity_check
###############################################################################
sub sanity_check {
    my($self, $data, $itempath, $nodes) = @_;
    my ($this_action);
        if (defined ($itempath)) {
#            ($this_action, $itempath) =
#                $self->_action_path_sanity_check($this_action, $itempath, $data);

            return 0 unless defined($itempath);

        } else {
            # if the item's path isn't defined, its real name was corrupted in
            # vss, so we'll check it in to the junk drawer as an add
            if (defined $main::gCfg{junkdir}) {
                $itempath = $self->_get_junk_itempath($main::gCfg{junkdir},
                    join('.', @$data{ qw(physname version revision_id) }));

                $self->add_error("Using filename '$itempath' for item with "
                    . "unrecoverable name at revision $data->{revision_id}");

                $this_action = 'ADD';
            } else {
                return undef;
            }
        }

        # if need_junkdir = 1, the first item is just about to be added to the
        # junk drawer, so create the dumpfile node to add this directory
        if ($self->{need_junkdir} == 1) {
            $self->_add_svn_dir($nodes, $main::gCfg{junkdir});
            $self->{need_junkdir} = -1;
        }

        foreach my $dir (@{ $self->{need_missing_dirs} }) {
            $self->_add_svn_dir($nodes, $dir);
            $self->add_error("Creating missing directory '$dir' for item "
                . "'$itempath' at revision $data->{revision_id}");
        }
}

###############################################################################
#  _get_junk_itempath
###############################################################################
sub _get_junk_itempath {
    my($self, $dir, $base) = @_;

    $base =~ s:.*/::;
    my $itempath = "$dir/$base";
    my $count = 1;

    if($self->{need_junkdir} == 0) {
        $self->{need_junkdir} = 1;
    }

    if(!defined($self->{junk_itempaths}->{$itempath})) {
        $self->{junk_itempaths}->{$itempath} = 1;
        return $itempath;
    }

    my($file, $ext);

    if($base =~ m/^(.*)\.(.*)/) {
        ($file, $ext) = ($1, ".$2");
    } else {
        ($file, $ext) = ($base, '');
    }

    while(defined($self->{junk_itempaths}->{$itempath})) {
        $itempath = "$dir/$file.$count$ext";
        $count++;
    }

    return $itempath;
}  #  End _get_junk_itempath


###############################################################################
#  exists
###############################################################################
sub exists {
    my($self, $itempath) = @_;

    my ($ref, $missing, $item) = $self->_get_svn_struct_ref_for_move ($itempath);

    if(!defined ($ref)) {
        # some strange problem
        return undef;
    }

    if(ref($ref) ne 'HASH') {
        # parent isn't a directory
        return 0;
    }

    if (!defined ($item)) {
        # Are we looking for the root node?
        if ($itempath eq '/') {
            return 1;
        }
        
        return 0; # or undef??
    }
    
    if(!defined ($missing) || (@$missing == 0)) {
        # all parent dirs exists
        return defined ($ref->{$item});
    }
    
    return 0;

}  #  End exists

###############################################################################
#  exists_parent
###############################################################################
sub exists_parent {
    my($self, $itempath) = @_;

    my ($ref, $missing, $item) = $self->_get_svn_struct_ref_for_move ($itempath);

    if(!defined ($ref)) {
        # some strange problem
        return undef;
    }

    if(ref($ref) ne 'HASH') {
        # parent isn't a directory
        return 0;
    }

   if(!defined ($missing) || (@$missing == 0)) {
        # all parent dirs exists
        return 1;
    }
    
    return 0;
}  #  End exists_parent

###############################################################################
#  get_missing_dirs
###############################################################################
sub get_missing_dirs {
    my($self, $itempath) = @_;

    my ($ref, $missing, $item) = $self->_get_svn_struct_ref_for_move ($itempath);

    if(!defined ($ref)) {
        # some strange problem
        return undef;
    }

    if(ref($ref) ne 'HASH') {
        # parent isn't a directory
        return 0;
    }

    return $missing;
}  #  End get_missing_dirs

###############################################################################
#  _get_svn_struct_ref_for_copy
###############################################################################
sub _get_svn_struct_ref_for_copy {
    my($self, $itempath) = @_;

    my ($ref, $missing, $item) = $self->_get_svn_struct_ref_for_move ($itempath);

    if(!defined ($ref)) {
        # some strange problem
        return undef;
    }

    if(ref($ref) ne 'HASH') {
        # parent isn't a directory
        return undef;
    }

    if (!defined ($item)) {
        # Are we looking for the root node?
        if ($itempath eq '/') {
            return $ref;
        }
        
        return undef;
    }
    
    if(!defined ($missing) || (@$missing == 0)) {
        # all parent dirs exists
        if (defined ($ref->{$item})) {
            return $ref->{$item};
        }
        elsif (defined ($self->{deleted}->{$itempath})) {
            $ref = $self->{deleted}->{$itempath};
            delete $self->{deleted}->{$itempath};
            return $ref;
        }
    }
    
    return undef;
}  #  End _get_svn_struct_ref_for_copy

###############################################################################
#  load
###############################################################################
sub load {
    my($self, $node) = @_;

    if($node->{action} eq 'add'){
        my $ref;
        if ($node->{kind} eq 'dir' && defined $node->{copypath}) {
            $ref = $self->_get_svn_struct_ref_for_copy ($node->{copypath});
        }
        $self->_add_svn_struct_item ($node->{path}, ($node->{kind} eq 'dir') ? 1 : 2, $ref);
    }
    elsif($node->{action} eq 'change'){
        # nothing to do
    }
    elsif($node->{action} eq 'delete'){
        $self->_delete_svn_struct_item ($node->{path}, ($node->{kind} eq 'dir') ? 1 : 2);
    }
}

###############################################################################
#  _action_path_sanity_check
###############################################################################
sub _action_path_sanity_check {
    my($self, $action, $itempath, $data) = @_;

    my($itemtype, $revision_id) = @{ $data }{qw(itemtype revision_id)};

    return($action, $itempath) if ($itempath eq '' || $itempath eq '/');

    my($newaction, $newpath) = ($action, $itempath);
    my $success;

    $self->{need_missing_dirs} = [];

    if($action eq 'ADD' || $action eq 'SHARE' || $action eq 'RECOVER') {
        $success = $self->_add_svn_struct_item($itempath, $itemtype);

        if(!defined($success)) {
            $newpath = undef;
            $self->add_error("Path consistency failure while trying to add "
                . "item '$itempath' at revision $revision_id; skipping");

        } elsif($success == 0) {
            # trying to re-add existing item; if file, change it to a commit
            if ($itemtype == 1) {

                $newpath = undef;
                $self->add_error("Attempt to re-add directory '$itempath' at "
                . "revision $revision_id; possibly missing delete");

            } else {

                $newaction = 'COMMIT';
                $self->add_error("Attempt to re-add file '$itempath' at "
                    . "revision $revision_id, changing to modify; possibly "
                    . "missing delete");

            }
        }

    } elsif ($action eq 'DELETE') {
        $success = $self->_delete_svn_struct_item($itempath, $itemtype);

        if(!$success) {
            $newpath = undef;
            $self->add_error("Attempt to delete non-existent item '$itempath' "
                . "at revision $revision_id; skipping...");
        }

    } elsif ($action eq 'RENAME') {
        $success = $self->_rename_svn_struct_item($itempath, $itemtype,
            $data->{info});

        if(!$success) {
            $newpath = undef;
            $self->add_error("Attempt to rename non-existent item '$itempath' "
                . "at revision $revision_id; skipping...");
        }
    } elsif ($action eq 'MOVE') {
        my ($ref, $item) = $self->_get_svn_struct_ref_for_move($itempath);

        if(!$ref) {
            $newpath = undef;
            $self->add_error("Attempt to move non-existent directory '$itempath' "
                . "at revision $revision_id; skipping...");
        }

        $success = $self->_add_svn_struct_item($data->{info}, 1, $ref->{$item});

        if(!$success) {
            $newpath = undef;
            $self->add_error("Error while attempting to move directory '$itempath' "
                . "at revision $revision_id; skipping...");
        }

        delete $ref->{$item};
    }

    return($newaction, $newpath);

}  #  End _action_path_sanity_check

###############################################################################
#  _add_svn_struct_item
###############################################################################
sub _add_svn_struct_item {
    my($self, $itempath, $itemtype, $newref) = @_;

    $itempath =~ s:^/::;
    my @subdirs = split '/', $itempath;

    my $item = pop(@subdirs);
    my $ref = $self->{svn_items};

    my $thispath = '';

    foreach my $subdir (@subdirs) {
        $thispath .= "$subdir/";

        if(ref($ref) ne 'HASH') {
            return undef;
        }
        if(!defined($ref->{$subdir})) {
            # parent directory doesn't exist; add it to list of missing dirs
            # to build up
            push @{ $self->{need_missing_dirs} }, $thispath;

            $ref->{$subdir} = {};
        }

        $ref = $ref->{$subdir};
    }

    if(ref($ref) ne 'HASH') {
        # parent "directory" is actually a file
        return undef;
    }

    if(defined($ref->{$item})) {
        # item already exists; can't add it
        return 0;
    }

    if(defined($newref)) {
        $ref->{$item} = $newref;
    } else {
        $ref->{$item} = ($itemtype == 1)? {} : 1;
    }

    return 1;

}  #  End _add_svn_struct_item

###############################################################################
#  _delete_svn_struct_item
###############################################################################
sub _delete_svn_struct_item {
    my($self, $itempath, $itemtype) = @_;

    return $self->_delete_rename_svn_struct_item($itempath, $itemtype);
}  #  End _delete_svn_struct_item

###############################################################################
#  _rename_svn_struct_item
###############################################################################
sub _rename_svn_struct_item {
    my($self, $itempath, $itemtype, $newname) = @_;

    return $self->_delete_rename_svn_struct_item($itempath, $itemtype, $newname);
}  #  End _rename_svn_struct_item

###############################################################################
#  _delete_rename_svn_struct_item
###############################################################################
sub _delete_rename_svn_struct_item {
    my($self, $itempath, $itemtype, $newname, $movedref) = @_;

    my ($path);
    $path = $itempath;
    $path =~ s:^/::;
    $newname =~ s:/$:: if defined($newname);
    my @subdirs = split '/', $path;

    my $item = pop(@subdirs);
    my $ref = $self->{svn_items};

    foreach my $subdir (@subdirs) {
        if(!(ref($ref) eq 'HASH') || !defined($ref->{$subdir})) {
            # can't get to item because a parent directory doesn't exist; give up
            return undef;
        }

        $ref = $ref->{$subdir};
    }

    if((ref($ref) ne 'HASH') || !defined($ref->{$item})) {
        # item doesn't exist; can't delete/rename it
        return 0;
    }

    if(defined $newname) {
        $ref->{$newname} = $ref->{$item};
    }

    if (defined ($ref->{$item}) && ref($ref->{$item}) eq 'HASH') {
        $self->{deleted}->{$itempath} = $ref->{$item};
    }
    
    delete $ref->{$item};

    return 1;

}  #  End _delete_rename_svn_struct_item

###############################################################################
#  _get_svn_struct_ref_for_move
###############################################################################
sub _get_svn_struct_ref_for_move {
    my($self, $itempath) = @_;

    $itempath =~ s:^/::;
    my @subdirs = split '/', $itempath;

    my $item = pop(@subdirs);
    my $ref = $self->{svn_items};

    my $thispath = '';

    while (@subdirs) {
#    foreach my $subdir (@subdirs) {
        my $subdir = shift @subdirs; 
        $thispath .= "$subdir/";

        if(ref($ref) ne 'HASH') {
            return ($ref, undef, $item);
        }
        
        if(!defined($ref->{$subdir})) {
            my @missing_dirs;
            push @missing_dirs, $thispath;

            foreach $subdir (@subdirs) {
                $thispath .= "$subdir/";
                push @missing_dirs, $thispath;
            }
            return ($ref, \@missing_dirs, $item);
            
        }

        $ref = $ref->{$subdir};
    }

    return ($ref, undef, $item);

}  #  End _get_svn_struct_ref_for_move

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
    $self->_add_svn_struct_item($dir, 1);

}  #  End _add_svn_dir

###############################################################################
#  _create_svn_path
###############################################################################
sub _create_svn_path {
    my($self, $nodes, $itempath) = @_;

    $self->{need_missing_dirs} = [];

    $self->_add_svn_struct_item($itempath, 1);

    foreach my $dir (@{ $self->{need_missing_dirs} }) {
        $self->_add_svn_dir($nodes, $dir);
    }

    $self->{need_missing_dirs} = [];
}  #  End _create_svn_path

1;
