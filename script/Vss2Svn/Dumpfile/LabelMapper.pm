package Vss2Svn::Dumpfile::LabelMapper;

use warnings;
use strict;
use Config::Ini;
use Text::Glob;

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $conf) = @_;

    my $self =
        {
         config => new Config::Ini( $conf, -commentdelim => "#" ),
        };

    $self->{labels} = $self->{config}->get (['labels']);
   
    $self = bless($self, $class);
    return $self;

}  #  End new

###############################################################################
#  remap
###############################################################################
sub remap {
    my($self, $labeldir, $label) = @_;

    my ($glob, $remap);
    
    # we need to reset the following each enumeration if we aborted the
    # previous one with a premature return
    my $dummy = keys %{$self->{labels}};

    while (($glob, $remap) = each %{ $self->{labels} }) {
        if ( $label =~ /$glob/) {
            if ($remap->[0] =~ m:^/:) {
#                print "remap label: $label to $remap->[0]\n";
                return $remap->[0];
            }
            else {
#                print "remap label: $label to $labeldir/$remap->[0]\n";
                return $labeldir . "/" . $remap->[0];
            }
            
        }
    }
#    print "$label not remaped\n";
    return  $labeldir;
}


1;
