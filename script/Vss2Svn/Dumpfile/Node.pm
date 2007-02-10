package Vss2Svn::Dumpfile::Node;

use warnings;
use strict;

our @gHeaderInfo =
    (
     {property => 'path',
      header => 'Node-path'},
     {property => 'kind',
      header => 'Node-kind'},
     {property => 'action',
      header => 'Node-action'},
     {property => 'copyrev',
      header => 'Node-copyfrom-rev'},
     {property => 'copypath',
      header => 'Node-copyfrom-path'},
    );

###############################################################################
#  new
###############################################################################
sub new {
    my($class) = @_;

    my $self =
        {
         path => undef,
         kind => undef,
         action => undef,
         copyrev => undef,
         copypath => undef,
         props => undef,
         hideprops => 0,
         text => undef,
        };

    $self = bless($self, $class);
    return $self;

}  #  End new

###############################################################################
#  set_initial_props
###############################################################################
sub set_initial_props {
    my($self, $itempath, $data) = @_;

    $self->{kind} = ($data->{itemtype} == 1)? 'dir' : 'file';
    $self->{path} = $itempath;

#    if ($data->{is_binary}) {
#        $self->add_prop('svn:mime-type', 'application/octet-stream');
#    }

}  #  End set_initial_props

###############################################################################
#  add_prop
###############################################################################
sub add_prop {
    my($self, $key, $value) = @_;
    $self->{props}->{$key} = $value;
}  #  End add_prop

###############################################################################
#  add_props
###############################################################################
sub add_props {
    my($self, %props) = @_;
    foreach my $prop (keys %props) {
        $self->{props}->{$prop} = $props{$prop};
    }
}  #  End add_props

###############################################################################
#  get_headers
###############################################################################
sub get_headers {
    my($self) = @_;

    my $headers = "\n";
    my($property, $header, $value);

    foreach my $info (@gHeaderInfo) {
        ($property, $header) = @{ $info }{ qw(property header) };
        if (defined($value = $self->{$property})) {
            if ($header =~ m/-path$/) {
                $value =~ s:^/::;
                $value =~ s:/$::;
            }

            $headers .= "$header: $value\n";
        }
    }

    return $headers;
}  #  End get_headers



1;
