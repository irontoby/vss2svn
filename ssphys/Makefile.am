AUXDIR = $(top_srcdir)/config
HELPER_PROGS=   $(AUXDIR)/config.guess \
		$(AUXDIR)/config.sub \
		$(AUXDIR)/depcomp \
		$(AUXDIR)/install-sh \
		$(AUXDIR)/ltmain.sh \
		$(AUXDIR)/missing 
EXTRA_DIST = $(top_srcdir)/runauto.sh $(top_srcdir)/version.in $(HELPER_PROGS)

CLEANFILES=
DISTCLEANFILES=svnwcrev.sh
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure $(HELPER_PROGS)
AUTOMAKE_OPTIONS = 1.9 foreign

noinst_LTLIBRARIES=
bin_PROGRAMS=

include $(top_srcdir)/SSPhysLib/module.mk
include $(top_srcdir)/SSPhys/module.mk

# remake libtool in case it's out of date
libtool : $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
