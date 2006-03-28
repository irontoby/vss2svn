AC_INIT([ssphys], [0.20.1])
AC_PREREQ(2.59)
AC_CONFIG_SRCDIR([version.in])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE

# we will be using C++
AC_PROG_CXX
# we will build libraries in some subdirectories
AC_PROG_LIBTOOL
# add an install target to Makefile
AC_PROG_INSTALL

# we need some subversion tools if this is a working copy
AC_ARG_VAR([SVNWCREV], [tool for incorporating subversion information into source])
AC_ARG_VAR([SVN], [subversion commandline client])
AC_ARG_VAR([SVNVERSION], [subversion version number tool])

# test to see if this is a working copy
# if it's not a wc, use the included version.h
if test -d .svn; then
  # check for commandline tools to create version.h
  AC_CHECK_PROG([SVNWCREV], [svnwcrev], [svnwcrev])
  AC_CHECK_PROG([SVN], [svn], [svn])
  AC_CHECK_PROG([SVNVERSION], [svnversion], [svnversion])
  if test "x$SVNWCREV" != "x"; then
    cat > svnwcrev.sh <<EOF
#!/bin/sh
$SVNWCREV \$1 \$2 \$3
EOF
    chmod +x svnwcrev.sh
    AC_MSG_NOTICE([wrote svnwcrev.sh])
  elif test "x$SVN" != "x" -a "x$SVNVERSION" != "x"; then
    cat > svnwcrev.sh <<EOF
#!/bin/sh
vss2svn_wcrange=\`$SVN info \$1 | $SED -ne  "s/^Revision: \(.*\)\$/\1/p"\`
vss2svn_wcurl=\`$SVN info \$1 | $SED -ne  "s/^URL: \(.*\)\$/\1/p"\`
vss2svn_wcrev=\`$SVN info \$1 | $SED -ne "s/^Revision: \(.*\:\)\{0,1\}\(.*\)\$/\2/p"\`
vss2svn_wcmixed=\`$SVNVERSION \$1 | $SED -ne "s/^[[^:]]*\$/2/p; s/^\(.*\):\(.*\)\$/1/p"\`
vss2svn_wcdate=\`$SVN info \$1 | $SED -ne "s/^Last Changed Date: \(....-..-.. ..:..:..\).*\$/\\1/p"\`
vss2svn_wcmods=\`$SVNVERSION \$1 | $SED -ne "s/^\([[0-9]]*:\)\{0,1\}[[0-9]]*S\{0,1\}\$/2/p; s/^\([[0-9]]*:\)\{0,1\}[[0-9]]*MS\{0,1\}\$/1/p"\`
vss2svn_wcnow=`date "+%Y-%m-%d %H:%M:%S"`
$AWK -v rev=\$vss2svn_wcrev -v range=\$vss2svn_wcrange  -v wcdate="\$vss2svn_wcdate" -v url="\$vss2svn_wcurl" -v now="\$vss2svn_wcnow" '! /\\\$WCNOW\\$|\\\$WCURL\\$|\\\$WCREV\\$|\\\$WCRANGE\\$|\\\$WCDATE\\$/ { print \$0; } /\\\$WCNOW\\$/ { gsub(/\\\$WCNOW\\$/, now); /\\\$WCURL\\$/ { gsub(/\\\$WCURL\\$/, url); print \$0; } /\\\$WCREV\\$/ { gsub(/\\\$WCREV\\$/, rev); print \$0; } /\\\$WCRANGE\\$/ { gsub(/\\\$WCRANGE\\$/, range); print \$0; } /\\\$WCDATE\\$/ { gsub(/\\\$WCDATE\\$/, wcdate); print \$0; }' \$2 | $SED -e "s/\\\$WCMODS?\(.*\):\(.*\)\\$/\\\\\$vss2svn_wcmods/g;" | $SED -e  "s/\\\$WCMIXED?\(.*\):\(.*\)\\$/\\\\\$vss2svn_wcmixed/g;" > \$3
EOF
    chmod +x svnwcrev.sh
    AC_MSG_NOTICE([wrote svnwcrev.sh])
  else
    # this will be fatal, as version.h should never be committed
    AC_MSG_ERROR([in a working copy, but no commandline tools to recreate version.h!])
  fi
fi

# add libtool dependencies
AC_SUBST(LIBTOOL_DEPS)

# Check for the C++ Boost library with Program Options support
AC_CHECK_LIB(boost_program_options, main,, [
	AC_MSG_ERROR([You need the C++ boost Program Options library (1.32 or later).])])
AC_CHECK_LIB(boost_filesystem, main,, [
	AC_MSG_ERROR([You need the C++ boost filesystem library.])])
AC_OUTPUT([Makefile])
