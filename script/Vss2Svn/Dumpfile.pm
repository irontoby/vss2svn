package Vss2Svn::Dumpfile;

use Vss2Svn::Dumpfile::Node;

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
        };

    # prevent perl from doing line-ending conversions, but this means we'll
    # need to explicitly output DOS-style line endings between info lines
    binmode($fh);

    my $old = select($fh);
    $| = 1;
    select($old);

    #TODO: take out UUID
    print $fh "SVN-fs-dump-format-version: 2\n\nUUID: 2d233e98-0cb8-4f47-9081-4b0a55eb6c6b\n";

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
    my $handler = $gHandlers{$action};

    my $nodes = [];

    # Temporary hack to prevent shared files from stepping on the "modified"
    # flag for other than the first commit. Ideally, we should keep all paths
    # for a given physical file's last modified flags, and use the best match
    # if we need to copy or recover one.

    $self->{is_primary} = 1;
    $self->{modified_cache} = {};
    $self->{deleted_cache} = {};

    foreach my $itempath (split "\t", $data->{itempaths}) {
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

    if ($data->{itemtype} == 1) {
        $newpath =~ s:(.*/)?.+$:$1$newname/:;
    } else {
        $newpath =~ s:(.*/)?.*:$1$newname:;
    }

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
        push @{ $self->{errors} },
            "Could not recover path $itempath at revision $data->{revision_id};"
            . " unable to determine deleted revision";
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
        push @{ $self->{errors} },
            "Attempt to retrieve file contents with unknown version number";
        return 0;
    }

    my $file = "$expdir\\$data->{physname}.$data->{version}";

    if (!open EXP, "$file") {
        push @{ $self->{errors} },
            "Could not open export file '$file'";
        return 0;
    }

    binmode(EXP);

    $node->{text} = join('', <EXP>);

    close EXP;

    return 1;

}  #  End get_export_contents

###############################################################################
#  output_node
###############################################################################
sub output_node {
    my($self, $node) = @_;
    my $fh = $self->{fh};

    print $fh $node->get_headers();
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

    my($sec, $min, $hour, $day, $mon, $year) = gmtime($vss_timestamp);

    $year += 1900;
    $mon += 1;

    return sprintf("%4.4i-%2.2i-%2.2iT%2.2i:%2.2i:%2.2i.%6.6iZ",
        $year, $mon, $day, $hour, $min, $sec, 0);

}  #  End svn_timestamp


1;