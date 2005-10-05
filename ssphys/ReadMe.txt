========================================================================
       CONSOLE APPLICATION : ssphys
========================================================================


ssphys is a replacment application for ss.exe, a Visual Source Safe reporting
utility. ssphys reads and interpretes the physical files of an Source Safe
archive.


/////////////////////////////////////////////////////////////////////////////
Notes:

Source Safe stores its information in several file:

1.) physical data files: 
    these are all those file, that can be found in "data\[a..z]\[aaaaaaaa...zzzzzzzz]"
2.) project information files:
    special files in data\[a..z] that tell VSS which files make up a project
3.) names.dat
4.) um.dat
    User Management
5.) CRC.dat
6.) Rights.dat
7.) srcsafe.ini


Most these files are made from special records that contain a header with 
length, type and crc information. The type is a two character code like "EL",
"MC" or "FD". 

There are two types of these record based files. One with a 52 byte big file
header (SSHeaderFile), and others that only contain these special records
(SSPlainFile), and others that are simply binary files, without special records.

The basic building of ssphys is based on these records (SSRecords). The records
are read without any interpretation from a SSFile and can be wrapped by higher
level special objects, depending on the type. The SSObject::MakeObject factory 
function builds a special SSObject derived object depending on the type.

With this it is possible to build a basic history reporting. 

The binary layout for all special records can be found in the SSTypes.h header.

/////////////////////////////////////////////////////////////////////////////

Reverse Delta: 
VSS stores all checkins as reverse deltas in "FD" records. Reverse delta means,
that the last version of the file is kept and delta to the previous version is 
stored. The last version is stored in a file named from the physical item plus
an extension ".A" or ".B". These two extensions are used alternately. 

Rebuilding an older item is possible by retrieving the last version, and reverse
apply all deltas up to the required version.

The delta format is a very easy stream with only three commands with the following
layout
{<command> <start> <len> data} {<command> <start> <len> data} ... <stop>

command:
0 = take <len> bytes from the stream and append them to the output file
1 = take <len> bytes from the last file, starting at offset <start> and append
    them to the output file
2 = <stop>

/////////////////////////////////////////////////////////////////////////////

VSS Special notes:

-- Shares
SS support shares, but only for files. Shares are implemented in the way, that the
reference count for the item is increased and the same physical entry is refrenced 
in another project. Additionally a shared flag is set. For each share a special "PF"
record is appended tothe data file. There is no version information attached to 
these "PF" records. So the item can't tell when a share was made (only implicitly by 
the linear order of the history activity). 

Shares are only supported for files. If you recursivly share a project a new subproject
is created in the share target project and all subitems are really shared into this
new project. This means e.g. that the shared project does not have the same history. 

In each project record the ("last") specification of the parent record is stored and 
the physical item for the parent. The fact, that shares are not supported for projects
allows us to exactly specify the parent path specification.

-- Renames
Renaming an item in VSS mainly done in tne parent project. There is a name variable 
in the item physical file that always carries the last name, but all historic activities
are recorded in the parent. 

Sadly, the project structure is not recorded with FD records. As with files, the last 
state is stored in a special file, but no reverse deltas exits. To get the correct 
names for older items, one have to reverse apply all actions beginning from the last 
state.


Renaming a project item will have additional effects. Since all sub projects will 
exactly record the parent specification, all parent specifications within the subprojects
of the renamed parent have to be renamed also. Therefor the items will only know the last
valid full parent specification (and the physical name), but no historic information.

