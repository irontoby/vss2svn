package Vss2Svn::Dumpfile;

use Vss2Svn::Dumpfile::Node;
use Encode qw(from_to);

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
     DELETE     => \&_delete_handler,
     RECOVER    => \&_recover_handler,
    );

# Keep track of when paths were modified or deleted, for subsequent copies
# or recovers.

our %gModified = ();
our %gDeleted = ();

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $fh) = @_;

    my $self =
        {
         fh => $fh,
         revision => 0,
         errors => [],
         modified_cache => {},
         deleted_cache => {},
         svn_items => {},
         junk_itempaths => {},
         need_junkdir => 0,
         need_missing_dirs => [],
        };

    # prevent perl from doing line-ending conversions
    binmode($fh);

    my $old = select($fh);
    $| = 1;
    select($old);

    print $fh "SVN-fs-dump-format-version: 2\n\n";

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

    my $props = [];
    my $fh = $self->{fh};

    print $fh "\nRevision-number: $revision\n";

    $comment = '' if !defined($comment);
    $author = '' if !defined($author);

    # convert to utf8
    from_to ($comment, "windows-1252", "utf8");
    from_to ($author, "windows-1252", "utf8");

    if ($revision > 0) {
        push @$props, ['svn:log', $comment];
        push @$props, ['svn:author', $author];
    }

    push @$props, ['svn:date', $self->svn_timestamp($timestamp)];

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
    $self->{modified_cache} = {};
    $self->{deleted_cache} = {};

    my($handler, $this_action);

    foreach my $itempath (split "\t", $data->{itempaths}) {
        $this_action = $action;

        if(defined($itempath)) {
            ($this_action, $itempath) =
                $self->_action_path_sanity_check($this_action, $itempath, $data);

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
                return 0;
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

        $handler = $gHandlers{$this_action};

        $self->$handler($itempath, $nodes, $data, $expdir);
        $self->{is_primary} = 0;
    }

    foreach my $node (@$nodes) {
        $self->output_node($node);
    }

    my($physname, $cache);
    while(($physname, $cache) = each %{ $self->{modified_cache} }) {
        $gModified{$physname} = $cache;
    }

    while(($physname, $cache) = each %{ $self->{deleted_cache} }) {
        $gDeleted{$physname} = $cache;
    }

}  #  End do_action

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

    $itempath =~ s:^/::;
    $newname =~ s:/$:: if defined($newname);
    my @subdirs = split '/', $itempath;

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

    foreach my $subdir (@subdirs) {
        $thispath .= "$subdir/";

        if(ref($ref) ne 'HASH') {
            return undef;
        }
        if(!defined($ref->{$subdir})) {
            return undef;
        }

        $ref = $ref->{$subdir};
    }

    if((ref($ref) ne 'HASH') || !defined($ref->{$item}) ||
       (ref($ref->{$item} ne 'HASH'))) {
        return undef;
    }

    return ($ref, $item);

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
#  _add_handler
###############################################################################
sub _add_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

    if ($data->{itemtype} == 2) {
        $self->get_export_contents($node, $data, $expdir);
    }

    $self->track_modified($data->{physname}, $data->{revision_id}, $itempath);

    push @$nodes, $node;

}  #  End _add_handler

###############################################################################
#  _commit_handler
###############################################################################
sub _commit_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'change';

    if ($data->{itemtype} == 2) {
        $self->get_export_contents($node, $data, $expdir);
    }

    $self->track_modified($data->{physname}, $data->{revision_id}, $itempath);

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

    $newpath =~ s:(.*/)?.*:$1$newname:;

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($newpath, $data);
    $node->{action} = 'add';

    my($copyrev, $copypath);

    # ideally, we should be finding the last time the file was modified and
    # copy it from there, but that becomes difficult to track...
    $copyrev = $data->{revision_id} - 1;
    $copypath = $itempath;

    $node->{copyrev} = $copyrev;
    $node->{copypath} = $copypath;

    push @$nodes, $node;

    $self->track_modified($data->{physname}, $data->{revision_id}, $newpath);

    $node = Vss2Svn::Dumpfile::Node->new();
    $node->{path} = $itempath;
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

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

    @{ $node }{ qw(copyrev copypath) }
        = $self->last_modified_rev_path($data->{physname});

    return unless defined($node->{copyrev});

    push @$nodes, $node;

}  #  End _share_handler

###############################################################################
#  _branch_handler
###############################################################################
sub _branch_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    # branching is a no-op in SVN

    # if the file is copied later, we need to track, the revision of this branch
    # see the shareBranchShareModify Test
    $self->track_modified($data->{physname}, $data->{revision_id}, $itempath);

}  #  End _branch_handler

###############################################################################
#  _move_handler
###############################################################################
sub _move_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    # moving in SVN is the same as renaming; add the new and delete the old

    my $newpath = $data->{info};

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($newpath, $data);
    $node->{action} = 'add';

    my($copyrev, $copypath);

    $copyrev = $data->{revision_id} - 1;
    $copypath = $itempath;

    $node->{copyrev} = $copyrev;
    $node->{copypath} = $copypath;

    push @$nodes, $node;

    $self->track_modified($data->{physname}, $data->{revision_id}, $newpath);

    $node = Vss2Svn::Dumpfile::Node->new();
    $node->{path} = $itempath;
    $node->{action} = 'delete';
    $node->{hideprops} = 1;

    push @$nodes, $node;

}  #  End _move_handler

###############################################################################
#  _delete_handler
###############################################################################
sub _delete_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->{path} = $itempath;
    $node->{action} = 'delete';
    $node->{hideprops} = 1;

    push @$nodes, $node;

    $self->track_deleted($data->{physname}, $data->{revision_id},
                         $itempath);

}  #  End _delete_handler

###############################################################################
#  _recover_handler
###############################################################################
sub _recover_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

    my($copyrev, $copypath) = $self->last_deleted_rev_path($data->{physname});

    if (!defined $copyrev) {
        $self->add_error(
            "Could not recover path $itempath at revision $data->{revision_id};"
            . " unable to determine deleted revision");
        return 0;
    }

    $node->{copyrev} = $copyrev - 1;
    $node->{copypath} = $copypath;

    push @$nodes, $node;

}  #  End _recover_handler

###############################################################################
#  track_modified
###############################################################################
sub track_modified {
    my($self, $physname, $revision, $path) = @_;

    return unless $self->{is_primary};

    $self->{modified_cache}->{$physname} =
        {
         revision => $revision,
         path => $path,
        };

}  #  End track_modified

###############################################################################
#  track_deleted
###############################################################################
sub track_deleted {
    my($self, $physname, $revision, $path) = @_;

    $self->{deleted_cache}->{$physname} =
        {
         revision => $revision,
         path => $path,
        };

}  #  End track_deleted

###############################################################################
#  last_modified_rev_path
###############################################################################
sub last_modified_rev_path {
    my($self, $physname) = @_;

    if (!defined($gModified{$physname})) {
        return (undef, undef);
    }

    return @{ $gModified{$physname} }{ qw(revision path) };
}  #  End last_modified_rev_path

###############################################################################
#  last_deleted_rev_path
###############################################################################
sub last_deleted_rev_path {
    my($self, $physname) = @_;

    if (!defined($gDeleted{$physname})) {
        return (undef, undef);
    }

    return @{ $gDeleted{$physname} }{ qw(revision path) };
}  #  End last_deleted_rev_path

###############################################################################
#  get_export_contents
###############################################################################
sub get_export_contents {
    my($self, $node, $data, $expdir) = @_;

    if (!defined($expdir)) {
        return 0;
    } elsif (!defined($data->{version})) {
        $self->add_error(
            "Attempt to retrieve file contents with unknown version number");
        return 0;
    }

    my $file = "$expdir\\$data->{physname}.$data->{version}";

    if (!open EXP, "$file") {
        $self->add_error("Could not open export file '$file'");
        return 0;
    }

    binmode(EXP);

#   $node->{text} = join('', <EXP>);
    $node->{text} = do { local( $/ ) ; <EXP> } ;

    close EXP;

    return 1;

}  #  End get_export_contents

###############################################################################
#  output_node
###############################################################################
sub output_node {
    my($self, $node) = @_;
    my $fh = $self->{fh};

    my $string = $node->get_headers();
    from_to ($string, "windows-1252", "utf8");
    print $fh $string;
    $self->output_content($node->{hideprops}? undef : $node->{props},
                          $node->{text});
}  #  End output_node

###############################################################################
#  output_content
###############################################################################
sub output_content {
    my($self, $props, $text) = @_;

    my $fh = $self->{fh};

    $text = '' unless defined $text;

    my $proplen = 0;
    my $textlen = 0;
    my($propout, $textout) = ('') x 2;

    my($key, $value);

    if (defined($props)) {
        foreach my $prop (@$props) {
            ($key, $value) = @$prop;
            $propout .= 'K ' . length($key) . "\n$key\nV " . length($value)
                . "\n$value\n";
        }

        $propout .= "PROPS-END\n";
        $proplen = length($propout);
    }

    $textlen = length($text);
    return if ($textlen + $proplen == 0);

    if ($proplen > 0) {
        print $fh "Prop-content-length: $proplen\n";
    }

    if ($textlen > 0) {
        print $fh "Text-content-length: $textlen\n";
    }

    print $fh "Content-length: " . ($proplen + $textlen)
        . "\n\n$propout$text\n";

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

    my($sec, $min, $hour, $day, $mon, $year) = gmtime($vss_timestamp);

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
