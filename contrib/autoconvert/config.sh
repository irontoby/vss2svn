#!/bin/bash
# vim:ts=2:

# configure this for your needs
# Path where the big dumpfiles will be placed (created by vss2svn and renaming actions)
PATH_DUMPBIG="Dumpfiles/big/"
# Path for dumpfiles that are created for each project
PATH_DUMPSPLIT="Dumpfiles/split/"
# Path where your vss2svn folder resides
PATH_TOOLS="tools/"
# temp path where logs and stuff like that goes
PATH_TEMP="temp/"
# Path where your VSS database resides
PATH_VSSREPOSITORY="\\\\dev_test\\"
# folder name of your VSS database (usually the one that contains srcsafe.ini and data folder)
FOLDER_VSSREPOSITORY="Repository"
# For subsequent runs, check Dumpfiles/big folder for string you have to place here
#VSSDUMPDATE="2007-03-13_09.47"
VSSDUMPDATE=`date +%Y-%m-%d_%H.%M`
# Name of dumpfile to be created by vss2svn
VSSORIGDUMP="$PATH_DUMPBIG$VSSDUMPDATE.dump"
# Name of dumpfile with renamed projects (due to svndumbfilter)
VSSRENAMEDDUMP="$PATH_DUMPBIG$VSSDUMPDATE.renamed.dump"
# Name of dumpfile with "nice" projects (no dependencies on other projects)
VSSNICEDUMP="$PATH_DUMPBIG$VSSDUMPDATE.nice.dump"
# Name of dumpfile for "ugly" projects (dependencies on other projects)
VSSUGLYDUMP="$PATH_DUMPBIG$VSSDUMPDATE.ugly.dump"
# You have 2 options:
# 0: use precompiled vss2svn.exe
# 1: use source script vss2svn.pl (ActivePerl needs to be installed for this option)
USE_PERL=0
# Full path to SS.EXE
SS=/cygdrive/c/Programme/Microsoft\ Visual\ Studio/Common/VSS/win32/SS.EXE
# Full path to ANALYZE.EXE
ANALYZE=/cygdrive/c/Programme/Microsoft\ Visual\ Studio/Common/VSS/win32/ANALYZE.EXE
# Export these to make SS.EXE work properly
export SSDIR=`cygpath -w "$PATH_TEMP$FOLDER_VSSREPOSITORY"`
export SSUSER="Admin"
export SSPWD="004"

