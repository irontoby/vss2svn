package Vss2Svn::Dumpfile::AutoProps;

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

    my ($enabled) = $self->{config}->get (['miscellany', 'enable-auto-props']);
    if (defined $enabled && $enabled eq "yes")
    {
        $self->{autoprops} = $self->{config}->get (['auto-props']);
    }
   
    $self = bless($self, $class);
    return $self;

}  #  End new

###############################################################################
#  get_props
###############################################################################
sub get_props {
    my($self, $path) = @_;

    my (@newprops);

    $path =~ s:^/::;
    my @subdirs = split '/', $path;
    my $item = pop(@subdirs);

    my ($glob, $autoprops);
    while (($glob, $autoprops) = each %{ $self->{autoprops} }) {
        print $glob, $item, "\n";
        if (Text::Glob::match_glob($glob, $item)) {
            foreach my $autoprop (@$autoprops)
            {
                my @props = split ';', $autoprop;
                foreach my $prop (@props)
                {
                    my @keyvalue = split '=', $prop;
                    push @newprops, [@keyvalue];
                }
            }
        }
    }
    
    return @newprops;
}


1;
