bin_PROGRAMS += ssphys

ssphys_SOURCES = \
$(top_srcdir)/SSPhys/Arguments.cpp \
$(top_srcdir)/SSPhys/Arguments.h \
$(top_srcdir)/SSPhys/Command.cpp \
$(top_srcdir)/SSPhys/Command.h \
$(top_srcdir)/SSPhys/CommandFactory.cpp \
$(top_srcdir)/SSPhys/CommandFactory.h \
$(top_srcdir)/SSPhys/CommandLine.cpp \
$(top_srcdir)/SSPhys/CommandLine.h \
$(top_srcdir)/SSPhys/FileTypeCommand.cpp \
$(top_srcdir)/SSPhys/FileTypeCommand.h \
$(top_srcdir)/SSPhys/Formatter.cpp \
$(top_srcdir)/SSPhys/Formatter.h \
$(top_srcdir)/SSPhys/GetCommand.cpp \
$(top_srcdir)/SSPhys/GetCommand.h \
$(top_srcdir)/SSPhys/GlobalOptions.cpp \
$(top_srcdir)/SSPhys/GlobalOptions.h \
$(top_srcdir)/SSPhys/HelpCommand.cpp \
$(top_srcdir)/SSPhys/HelpCommand.h \
$(top_srcdir)/SSPhys/HistoryCommand.cpp \
$(top_srcdir)/SSPhys/HistoryCommand.h \
$(top_srcdir)/SSPhys/InfoCommand.cpp \
$(top_srcdir)/SSPhys/InfoCommand.h \
$(top_srcdir)/SSPhys/Options.cpp \
$(top_srcdir)/SSPhys/Options.h \
$(top_srcdir)/SSPhys/PropertiesCommand.cpp \
$(top_srcdir)/SSPhys/PropertiesCommand.h \
$(top_srcdir)/SSPhys/StdAfx.cpp \
$(top_srcdir)/SSPhys/StdAfx.h \
$(top_srcdir)/SSPhys/ValidateCommand.cpp \
$(top_srcdir)/SSPhys/ValidateCommand.h \
$(top_srcdir)/SSPhys/VersionFilter.cpp \
$(top_srcdir)/SSPhys/VersionFilter.h \
$(top_srcdir)/SSPhys/ssphys.cpp \
$(top_builddir)/version.h
ssphys_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)/SSPhys -I$(top_srcdir)/utils
ssphys_CXXFLAGS = -Wno-deprecated
ssphys_LDADD = libSSPhys.la

$(top_builddir)/version.h : $(top_builddir)/svnwcrev.sh $(top_srcdir)/version.in
	./svnwcrev.sh $(top_srcdir) $(top_srcdir)/version.in $@

CLEANFILES += $(top_builddir)/version.h
EXTRA_DIST += $(top_builddir)/version.h
