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

# keep track of when paths were modified or deleted, for subsequent copies
# or recovers
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
    #physname    VARCHAR,
    #version     INTEGER,
    #action      VARCHAR,
    #itempaths   VARCHAR,
    #itemtype    INTEGER,
    #is_binary   INTEGER,
    #info        VARCHAR

    my $action = $data->{action};
    my $handler = $gHandlers{$action};

    my $nodes = [];

    foreach my $itempath (split "\t", $data->{itempaths}) {
        $self->$handler($itempath, $nodes, $data, $expdir);
    }

    foreach my $node (@$nodes) {
        $self->output_node($node);
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

    $gModified{$itempath} = $data->{revision_id};

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

    $gModified{$itempath} = $data->{revision_id};

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

    $node->{copypath} = $itempath;

    my $copyrev = ($data->{itemtype} == 1)?
        $data->{revision_id} - 1 :
        $gModified{ $itempath };

    $node->{copyrev} = $copyrev;

    $gModified{$newpath} = $data->{revision_id};

    push @$nodes, $node;

    $node = Vss2Svn::Dumpfile::Node->new();
    $node->{path} = $itempath;
    $node->{action} = 'delete';
    $node->{hideprops} = 1;

    push @$nodes, $node;

    # we don't add this to %gDeleted since VSS doesn't treat a rename as an
    # add/delete

}  #  End _rename_handler

###############################################################################
#  _share_handler
###############################################################################
sub _share_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

    $node->{copypath} = $data->{info};
    $node->{copyrev} = $gModified{ $data->{info} };

    $gModified{$itempath} = $data->{revision_id};

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

    push @{ $self->{errors} }, "Not yet implemented: $data->{action}"

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

    $gDeleted{$itempath} = $data->{revision_id};

}  #  End _delete_handler

###############################################################################
#  _recover_handler
###############################################################################
sub _recover_handler {
    my($self, $itempath, $nodes, $data, $expdir) = @_;

    my $node = Vss2Svn::Dumpfile::Node->new();
    $node->set_initial_props($itempath, $data);
    $node->{action} = 'add';

    if (!defined $gDeleted{$itempath}) {
        push @{ $self->{errors} },
            "Could not recover path $itempath at revision $data->{revision_id};"
            . " unable to determine deleted revision";
        return 0;
    }

    $node->{copypath} = $itempath;
    $node->{copyrev} = $gDeleted{$itempath} - 1;

    $gModified{$itempath} = $data->{revision_id};

    push @$nodes, $node;

}  #  End _recover_handler

###############################################################################
#  get_export_contents
###############################################################################
sub get_export_contents {
    my($self, $node, $data, $expdir) = @_;

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