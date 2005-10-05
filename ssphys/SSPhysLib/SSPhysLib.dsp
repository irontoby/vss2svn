# Microsoft Developer Studio Project File - Name="SSPhysLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SSPhysLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SSPhysLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SSPhysLib.mak" CFG="SSPhysLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SSPhysLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "SSPhysLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SSPhysLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "..\Utils" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "SSPhysLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "..\Utils" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /Zm200 /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "SSPhysLib - Win32 Release"
# Name "SSPhysLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=crc.cpp
# End Source File
# Begin Source File

SOURCE=.\FileName.cpp
# End Source File
# Begin Source File

SOURCE=.\SSException.cpp
# End Source File
# Begin Source File

SOURCE=SSFiles.cpp
# End Source File
# Begin Source File

SOURCE=SSName.cpp
# End Source File
# Begin Source File

SOURCE=SSRecord.cpp
# End Source File
# Begin Source File

SOURCE=.\SSTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=crc.h
# End Source File
# Begin Source File

SOURCE=.\FileName.h
# End Source File
# Begin Source File

SOURCE=.\SSException.h
# End Source File
# Begin Source File

SOURCE=SSFiles.h
# End Source File
# Begin Source File

SOURCE=SSName.h
# End Source File
# Begin Source File

SOURCE=SSRecord.h
# End Source File
# Begin Source File

SOURCE=SSTypes.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\XML.h
# End Source File
# End Group
# Begin Group "SSObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SSBranchFileObject.cpp
# End Source File
# Begin Source File

SOURCE=.\SSBranchFileObject.h
# End Source File
# Begin Source File

SOURCE=SSCheckOutObject.cpp
# End Source File
# Begin Source File

SOURCE=SSCheckOutObject.h
# End Source File
# Begin Source File

SOURCE=SSCommentObject.cpp
# End Source File
# Begin Source File

SOURCE=SSCommentObject.h
# End Source File
# Begin Source File

SOURCE=SSItemInfoObject.cpp
# End Source File
# Begin Source File

SOURCE=SSItemInfoObject.h
# End Source File
# Begin Source File

SOURCE=SSNameObject.cpp
# End Source File
# Begin Source File

SOURCE=SSNameObject.h
# End Source File
# Begin Source File

SOURCE=SSObject.cpp
# End Source File
# Begin Source File

SOURCE=SSObject.h
# End Source File
# Begin Source File

SOURCE=SSParentFolderObject.cpp
# End Source File
# Begin Source File

SOURCE=SSParentFolderObject.h
# End Source File
# Begin Source File

SOURCE=SSProjectObject.cpp
# End Source File
# Begin Source File

SOURCE=SSProjectObject.h
# End Source File
# Begin Source File

SOURCE=SSVersionObject.cpp
# End Source File
# Begin Source File

SOURCE=SSVersionObject.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
