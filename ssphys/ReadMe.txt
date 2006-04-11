== Introduction ==

ssphys is an application to read and ouput the content of the archive files 
created from Visual Source Safe 6.0. Additionally it can rebuild old versions of 
the archived files.

ssphys was created in order to fully convert the from a Visual Source Safe 
archive to other source control systems, esp. subversion. The reason for the 
exitence of this tool is due to the buggyness of the VSS tool chain provided 
from Microsoft. With the provided commandline tool ss.exe it was impossible to 
retrieve deleted or renamed files.


== Visual Source Safe ==

Visual Source Safe is a source control application provided from Microsoft. 

=== 5.0 ===

not tested yet

=== 6.0 === 

provided with Visual Studio. COmes in different version. The last known is V6.0d

=== 7.0 ===

provided with Whidebey Studio not tested yet.


=== Archive Format ===

VSS differntiates between two different types of '''items''': '''Files''' and 
'''Projects''' Project are like directories and can contain different other 
Projects or Files. To differntiate between Projects and Files, Projects are 
written with a leading dollar sign like $/Project. 

VSS stores both types of items (Projects and Files) in seperate physical files 
within the archive. During all activities the name of the physical never 
changes, even throughout renames or deletes. If two items with the same name are 
stored within one Project, these two items are represented in two different 
physical files. 

The name of the physical file used for an item is build from an increasing list 
of 8 characters (aaaaaaaa...zzzzzzzz). The name of the file has no relation to 
the name of the Item. The last name that was used is recorded in the file 
aaaaaaaa.cnt in the data directory of the archive.

The data directory of the archive is made up of several different files:

 1. physical data files: 

    these are all those files, that can be found in the data directory. Starting 
    from v6.0 the data directory is organized in different subfolders from "a" 
    to "z". The physical datafiles are sorted into these folders from their 
    first letter: data\[a..z]\[aaaaaaaa...zzzzzzzz]

 2. project information files:

    special files in data\[a..z] that tell VSS which Files make up a Project

 3. names.dat

    names cache to convert from a short representation of a File or Project name 
    to the real name.

 4. um.dat

    User Management

 5. CRC.dat  
 
 6. Rights.dat 
 
 7. srcsafe.ini


Most of these files are made from special records that contain a header with 
length, type and crc information. The type is a two character code like "EL", 
"MC" or "FD", e.g names.dat and all physical files. 

Others files are simply binary files, without a special record structure, e.g 
status.dat

Additionally sveral files have a specific file header (SSHeaderFile), and others 
that only contain these special records (SSPlainFile)

The basic building of ssphys is based on these records (SSRecord). The records 
are read without any interpretation from a SSFile and can be wrapped by higher 
level special objects, depending on the type. The SSObject::MakeObject factory 
function builds a special SSObject derived object depending on the type.

The binary layout for all special records can be found in the SSTypes.h header.


=== Reverse Delta ===

VSS stores all checkins as reverse deltas in "FD" records. Reverse delta means, 
that the last version of the file is kept and delta to the previous version is 
stored. The last version is stored in a file named from the physical item plus 
an extension ".A" or ".B". These two extensions are used alternately. 

Rebuilding an older item is possible by retrieving the last version, and reverse 
apply all deltas up to the required version.

The delta format is a very easy stream with only three commands with the 
following layout {<command> <start> <len> data} {<command> <start> <len> data} 
... <stop>

command: 0 = take <len> bytes from the stream and append them to the output file 
1 = take <len> bytes from the last file, starting at offset <start> and append 
them to the output file 2 = <stop>


== VSS Actions == 

=== Shares ===

SS support shares, but only for files. Shares are implemented in the way, that 
the reference count for the item is increased and the same physical entry is 
refrenced in another project. Additionally a shared flag is set. For each share 
a special "PF" record is appended tothe data file. There is no version 
information attached to these "PF" records. So the item can't tell when a share 
was made (only implicitly by the linear order of the history activity). 

Shares are only supported for files. If you recursivly share a project a new 
subproject is created in the share target project and all subitems are really 
shared into this new project. This means e.g. that the shared project does not 
have the same history. 

In each project record the ("last") specification of the parent record is stored 
and the physical item for the parent. The fact, that shares are not supported 
for projects allows us to exactly specify the parent path specification.

=== Renames ===

Renaming an item in VSS mainly done in tne parent project. There is a name 
variable in the item physical file that always carries the last name, but all 
historic activities are recorded in the parent. 

Sadly, the project structure is not recorded with FD records. As with files, the 
last state is stored in a special file, but no reverse deltas exits. To get the 
correct names for older items, one have to reverse apply all actions beginning 
from the last state.

Renaming a project item will have additional effects. Since all sub projects 
will exactly record the parent specification, all parent specifications within 
the subprojects of the renamed parent have to be renamed also. Therefor the 
items will only know the last valid full parent specification (and the physical 
name), but no historic information.

=== Branches ===

When branching a file VSS creates a new physical file and links it to the branch 
source file. The new branch is recorded in a special BF record in the branch 
source. This information is used in the properties dialog on the Paths page. 

A branch can only be performed on a file that was previously shared. During the 
branch the share is broken at the pinned version and a new physical file is 
created.

The following changes are made:

 branch source::

  addition of a new BF record with the information about the new physical files. 
  All BF records are linked from the last to the first. The last BF records and 
  the number of records is stored in the DH File information record.
 
 branch target::
 
  The branch target starts as a new physical file with the following excpetions:
  
  1. The Item Information Record (DH) records the branch source physical file
  
  2. The first version is a RollBack action (in contrast to a Created File action)
  
	3. version numbering starts with one greater the version number from where the 
	   branch was created
  
 parent folder::
   
   A new EL (History Record) with the ActionID ''Branch_File'' is created. The 
   name of the item, the old physical and the new physical files names are 
   recorded. No information about the branch version is recorded.
   
  path:: the source safe name for the history of exactly one branch.

example:

A file Readme.txt (INEAAAAA) was branched at version 19 and the newly created "path"
will live in the new physical file AJGAAAAA. The first version in this file is a 
RollBack action, that points back to the branch parent. The branchpoint can be 
calculated from the version number of the RollBack action, since the version number 
after the branch continues counting from the branchpoint, namely version 20.

Since the Rollback action is only for internal bookeeping, no file change is 
associated with this action, that means, that the file INEAAAAA;19 is the same as the
file AJGAAAAA;20. 

  <Version offset="1116">
    <VersionNumber>20</VersionNumber>
    <UserName>Dirk</UserName>
    <Date>1094815584</Date>
    <Comment> </Comment>
    <Action ActionId="RollBack">
      <Physical>AJGAAAAA</Physical>
      <SSName offset="89324" type="file">Readme.txt</SSName>
      <Parent>INEAAAAA</Parent>
    </Action>
  </Version>

The specifc RollBack version is reconstructable from the physical file AJGAAAAA. All
further versions must be taken from the branch parent INEAAAAA. The version history
for this branched file is therefor:

...
readme.txt;21 = AJGAAAAA;21
readme.txt;20 = AJGAAAAA;20
readme.txt;19 = INEAAAAA;19
readme.txt;18 = INEAAAAA;18
...


=== Rollback ===

==== Rollback Action vs. Rollback Activities ====

The term Rollback is a bit misleading in VSS. There is a properties command where 
you can perform "Rollback" activities and, there is the "Rollback action" reported
in the history. Both have nothing in common. 

The first one is available from the command line ot the history dialog of the 
SSExplorer. This one will let you "constantly" delete some content of your history 
by completly removing it out of the physical file. There are no remains left over 
and there is no entry in the item history, that you performed this action.

The second one is implicitly triggered by the branch command. The branch will force
the creation of a new physical file. In order to follow the history to the point
before the branch, the first action in this new file is a "RollBack Action", with
the information about the parent physical file and the branchpoint. See under Branches
for an example. It is like saying "from now on continue with the history in a different 
file at a specific version number"

If you see a "Roolback to version" action in the history of a file, this is always
due to a branch action. 

