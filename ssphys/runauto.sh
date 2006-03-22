#!/bin/sh
# Run the auto tools
set -x
aclocal
libtoolize
autoconf
automake -a
