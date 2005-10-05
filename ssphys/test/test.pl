use strict;

use Test::Harness;

# XXX does not work when run outside "src"!
my @tests = glob "t/*.t";

$Test::Harness::verbose=1;
runtests(@tests);

