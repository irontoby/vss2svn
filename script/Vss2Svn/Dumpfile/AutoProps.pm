package Vss2Svn::Dumpfile::AutoProps;

use warnings;
use strict;
use Config::Ini;
use Text::Glob 0.08;

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $conf) = @_;

    my $config = new Config::Ini( $conf, -commentdelim => "#" );

    my $self = ();
    $self->{entries} = ();

    my ($enabled) = $config->get (['miscellany', 'enable-auto-props']);
    if (defined $enabled && $enabled eq "yes") {
        my $autoprops_list = $config->get (['auto-props']);

        # see http://subversion.tigris.org/servlets/ReadMsg?list=svn&msgNo=29642
        # matches are performed in a case-insensitive manner

        my ($glob, $autoprops);
        while (($glob, $autoprops) = each %{ $autoprops_list }) {
            my $entry = ();
            my $regex = Text::Glob::glob_to_regex_string($glob);
            $entry->{glob} = qr/$regex/i;
            $entry->{props} = $autoprops;
            push @{$self->{entries}}, $entry; 
        }
    }
   
    $self = bless($self, $class);
    return $self;

}  #  End new

###############################################################################
#  get_props
###############################################################################
sub get_props {
    my($self, $path) = @_;

    my (%newprops);

    $path =~ s:^/::;
    my @subdirs = split '/', $path;
    my $item = pop(@subdirs);

    foreach my $entry (@{$self->{entries}}) {
        if ($item =~ /$entry->{glob}/) {
            foreach my $autoprop (@{$entry->{props}})
            {
                my @props = split ';', $autoprop;
                foreach my $prop (@props)
                {
                    my ($key, $value) = split '=', $prop;
                    $newprops{$key} = $value;
                }
            }
        }
    }
    
    return %newprops;
}


1;
