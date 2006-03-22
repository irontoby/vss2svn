AC_INIT(ssphys, 0.18.3)
AC_PREREQ(2.59)
AC_CONFIG_SRCDIR(version.in)
AM_INIT_AUTOMAKE

# we will be using C++
AC_PROG_CXX
# we will build libraries in some subdirectories
AC_PROG_LIBTOOL
# add an install target to Makefile
AC_PROG_INSTALL

# test to see if this is a working copy
# if it's not a wc, use the included version.h
if test -d .svn; then
  # check for commandline tools to create version.h
  AC_CHECK_PROG([SVNWCREV], [svnwcrev], [svnwcrev])
  AC_CHECK_PROG([SVN], [svn], [svn])
  AC_CHECK_PROG([SVNVERSION], [svnversion], [svnversion])
  if test "x$SVNWCREV" != "x"; then
    $SVNWCREV . version.in version.h
  elif test "x$SVN" != "x" -a "x$SVNVERSION" != "x"; then
    vss2svn_wcrange=`$SVN info . | sed -ne  "s/^Revision: \(.*\)$/\1/p"`
    vss2svn_wcrev=`$SVN info . | sed -ne "s/^Revision: \(.*\:\)\{0,1\}\(.*\)$/\2/p"`
    vss2svn_wcdate=`$SVN info . | sed -ne "s/^Last Changed Date: \(....-..-.. ..:..:.. .....\).*$/\\1/p"`
    vss2svn_wcmods=`$SVNVERSION . | sed -ne "s/^\([[0-9]]*:\)\{0,1\}[[0-9]]*S\{0,1\}$/2/p; s/^\([[0-9]]*:\)\{0,1\}[[0-9]]*MS\{0,1\}$/1/p"`
    awk -v rev=$vss2svn_wcrev -v range=$vss2svn_wcrange  -v wcdate="$vss2svn_wcdate" '! /\$WCREV\$|\$WCRANGE\$|\$WCDATE\$/ { print $0; } /\$WCREV\$/ { gsub(/\$WCREV\$/, rev); print $0; } /\$WCRANGE\$/ { gsub(/\$WCRANGE\$/, range); print $0; } /\$WCDATE\$/ { gsub(/\$WCDATE\$/, wcdate); print $0; }' version.in | sed -e "s/\$WCMODS?\(.*\):\(.*\)\$/\\$vss2svn_wcmods/g;" > version.h 
    AC_MSG_NOTICE([rewrote version.h])
  else
    # this will be fatal, as version.h should never be committed
    AC_MSG_ERROR([in a working copy, but no commandline tools to recreate version.h!])
  fi
fi

# Check for the C++ Boost library with Program Options support
AC_CHECK_LIB(boost_program_options, main,, [
	AC_MSG_ERROR([You need the C++ boost Program Options library (1.32 or later).])])
AC_CHECK_LIB(boost_filesystem, main,, [
	AC_MSG_ERROR([You need the C++ boost filesystem library.])])
AC_OUTPUT(Makefile)
AC_OUTPUT(SSPhysLib/Makefile)
AC_OUTPUT(SSPhys/Makefile)

