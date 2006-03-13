# non-Gnu-style project
AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS = SSPhys
# Note that SSTests is not listed here as a source.
SSPhys_SOURCES = Arguments.cpp Arguments.h Command.cpp Command.h CommandFactory.cpp CommandFactory.h CommandLine.cpp CommandLine.h FileTypeCommand.cpp FileTypeCommand.h Formatter.cpp Formatter.h GetCommand.cpp GetCommand.h GlobalOptions.cpp GlobalOptions.h HelpCommand.cpp HelpCommand.h HistoryCommand.cpp HistoryCommand.h InfoCommand.cpp InfoCommand.h Options.cpp Options.h PropertiesCommand.cpp PropertiesCommand.h  StdAfx.cpp StdAfx.h ValidateCommand.cpp ValidateCommand.h VersionFilter.cpp VersionFilter.h ssphys.cpp version.in
INCLUDES = -I@top_srcdir@ -I@top_srcdir@/utils
# strstream is a deprecated header, sstream is the replacement
AM_CXXFLAGS = -Wno-deprecated
LDADD = ../SSPhysLib/libSSPhys.a
