noinst_LTLIBRARIES += libSSPhys.la

libSSPhys_la_SOURCES = \
$(top_srcdir)/SSPhysLib/SSBranchFileObject.cpp \
$(top_srcdir)/SSPhysLib/SSBranchFileObject.h \
$(top_srcdir)/SSPhysLib/SSCheckOutObject.cpp \
$(top_srcdir)/SSPhysLib/SSCheckOutObject.h \
$(top_srcdir)/SSPhysLib/SSCommentObject.cpp \
$(top_srcdir)/SSPhysLib/SSCommentObject.h \
$(top_srcdir)/SSPhysLib/SSException.cpp \
$(top_srcdir)/SSPhysLib/SSException.h \
$(top_srcdir)/SSPhysLib/SSFiles.cpp \
$(top_srcdir)/SSPhysLib/SSFiles.h \
$(top_srcdir)/SSPhysLib/SSItemInfoObject.cpp \
$(top_srcdir)/SSPhysLib/SSItemInfoObject.h \
$(top_srcdir)/SSPhysLib/SSName.cpp \
$(top_srcdir)/SSPhysLib/SSName.h \
$(top_srcdir)/SSPhysLib/SSNameObject.cpp \
$(top_srcdir)/SSPhysLib/SSNameObject.h \
$(top_srcdir)/SSPhysLib/SSObject.cpp \
$(top_srcdir)/SSPhysLib/SSObject.h \
$(top_srcdir)/SSPhysLib/SSParentFolderObject.cpp \
$(top_srcdir)/SSPhysLib/SSParentFolderObject.h \
$(top_srcdir)/SSPhysLib/SSProjectObject.cpp \
$(top_srcdir)/SSPhysLib/SSProjectObject.h \
$(top_srcdir)/SSPhysLib/SSRecord.cpp \
$(top_srcdir)/SSPhysLib/SSRecord.h \
$(top_srcdir)/SSPhysLib/SSTypes.cpp \
$(top_srcdir)/SSPhysLib/SSTypes.h \
$(top_srcdir)/SSPhysLib/SSVersionObject.cpp \
$(top_srcdir)/SSPhysLib/SSVersionObject.h \
$(top_srcdir)/SSPhysLib/StdAfx.cpp \
$(top_srcdir)/SSPhysLib/StdAfx.h \
$(top_srcdir)/SSPhysLib/XML.cpp \
$(top_srcdir)/SSPhysLib/XML.h \
$(top_srcdir)/SSPhysLib/crc.cpp \
$(top_srcdir)/SSPhysLib/crc.h \
$(top_srcdir)/utils/LeakWatcher.h \
$(top_srcdir)/utils/tinystr.cpp \
$(top_srcdir)/utils/tinystr.h \
$(top_srcdir)/utils/tinyxml.cpp \
$(top_srcdir)/utils/tinyxml.h \
$(top_srcdir)/utils/tinyxmlerror.cpp \
$(top_srcdir)/utils/tinyxmlparser.cpp
libSSPhys_la_CPPFLAGS = -I$(top_srcdir)/utils -I$(top_srcdir)/SSPhysLib
libSSPhys_la_CXXFLAGS = -Wno-deprecated

