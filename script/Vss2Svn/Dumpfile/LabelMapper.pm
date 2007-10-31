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


    my $retval = $labeldir;

    # we need to reset the following each enumeration if we aborted the
    # previous one with a premature return
    keys %{$self->{labels}};

    my ($label_path, $remap);
    while (($label_path, $remap) = each %{ $self->{labels} }) {
        if ( $label =~ /$label_path/) {
            # N.B. This might be the root project, so $labeldir might need to be be present?
            $retval = ($remap->[0] =~ m:^/:) ? $remap->[0] : ($labeldir . "/" . $remap->[0]);
            last;
        }
    }

    return $retval;
}


1;
