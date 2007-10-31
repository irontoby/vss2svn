package Vss2Svn::Dumpfile::LabelMapper;

use warnings;
use strict;
use Config::Ini;

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $conf) = @_;

    my $config = new Config::Ini( $conf, -commentdelim => "#" );
    my $self = ();

    $self->{labels} = $config->get (['labels']);
   
    $self = bless($self, $class);
    return $self;

}  #  End new

###############################################################################
#  remap
###############################################################################
sub remap {
    my($self, $labeldir, $label) = @_;


    my $retval = ();
    $retval->{is_labeldir} = 1;
    $retval->{replacement} = $labeldir;

    # we need to reset the following each enumeration if we aborted the
    # previous one with a premature return
    keys %{$self->{labels}};

    my ($label_path, $remap);
    while (($label_path, $remap) = each %{ $self->{labels} }) {
        if ( $label =~ /$label_path/) {
            $retval->{replacement} = $remap->[0];
            $retval->{is_labeldir} = ($remap->[0] =~ m:^/:);
            last;
        }
    }

    return $retval;
}


1;
