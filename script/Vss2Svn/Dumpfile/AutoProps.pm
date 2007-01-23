package Vss2Svn::Dumpfile::AutoProps;

use warnings;
use strict;
use Config::Ini;

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $conf) = @_;

    my $self =
        {
         config => new Config::Ini( $conf, -commentdelim => "#" ),
        };

    my ($enabled) = $self->{config}->get (['miscellany', 'enable-auto-props']);
    if (defined $enabled && $enabled eq "yes")
    {
        my ($autoprops) = $self->{config}->get (['auto-props']);
    }
   
    $self = bless($self, $class);
    return $self;

}  #  End new




1;
