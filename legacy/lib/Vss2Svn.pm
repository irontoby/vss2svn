###############################################################################
#                           package Vss2Svn                                   #
###############################################################################

package Vss2Svn;

require 5.005_62;
use strict;
use warnings;

use File::Path;
use File::Copy;

use Carp;

our $VERSION = '1.00';

###############################################################################
#  set_user
###############################################################################
sub set_user {
    my($self, $user, $passwd) = @_;

    $self->{user} = $user;

    {
        no warnings 'uninitialized'; # we want to undef passwd if none passed
        $self->{passwd} = $passwd unless $passwd eq '';
    }

}  # End set_user


1;
