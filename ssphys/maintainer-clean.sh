#!/bin/sh
# Clean out all the autoconf/automake/libtool files before check-in.
[ -f Makefile ] && make maintainer-clean
# Only remove these for Subversion check-in.
# Leave them for distribution tarballs.
rm -Rf configure Makefile.in aclocal.m4 SSPhysLib/Makefile.in SSPhys/Makefile.in version.h
# TODO: These should be copied from the /usr/share/automake-* directory
# and edited as needed, then checked in. They're either documentation
# or canned installation scripts for end-users on stripped machines.
rm -Rf INSTALL COPYING
rm -Rf depcomp config.guess config.sub install-sh ltmain.sh missing
#!/bin/sh
# Clean out all the autoconf/automake/libtool files before check-in.
[ -f Makefile ] && make maintainer-clean
# Only remove these for Subversion check-in.
# Leave them for distribution tarballs.
rm -Rf configure Makefile.in aclocal.m4 SSPhysLib/Makefile.in SSPhys/Makefile.in version.h
# TODO: These should be copied from the /usr/share/automake-* directory
# and edited as needed, then checked in. They're either documentation
# or canned installation scripts for end-users on stripped machines.
rm -Rf INSTALL COPYING
rm -Rf depcomp config.guess config.sub install-sh ltmain.sh missing
#!/bin/sh
# Clean out all the autoconf/automake/libtool files before check-in.
[ -f Makefile ] && make maintainer-clean
# Only remove these for Subversion check-in.
# Leave them for distribution tarballs.
rm -Rf configure Makefile.in aclocal.m4 SSPhysLib/Makefile.in SSPhys/Makefile.in version.h
# TODO: These should be copied from the /usr/share/automake-* directory
# and edited as needed, then checked in. They're either documentation
# or canned installation scripts for end-users on stripped machines.
rm -Rf INSTALL COPYING
rm -Rf depcomp config.guess config.sub install-sh ltmain.sh missing
#!/bin/sh
# Clean out all the autoconf/automake/libtool files before check-in.
[ -f Makefile ] && make maintainer-clean
# Only remove these for Subversion check-in.
# Leave them for distribution tarballs.
rm -Rf configure Makefile.in aclocal.m4 SSPhysLib/Makefile.in SSPhys/Makefile.in version.h
# TODO: These should be copied from the /usr/share/automake-* directory
# and edited as needed, then checked in. They're either documentation
# or canned installation scripts for end-users on stripped machines.
rm -Rf INSTALL COPYING
rm -Rf depcomp config.guess config.sub install-sh ltmain.sh missing
