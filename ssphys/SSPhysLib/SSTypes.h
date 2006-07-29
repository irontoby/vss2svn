// SSTypes.h:structure definitions for SourceSafe files
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSTYPES_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
#define AFX_SSTYPES_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "time.h"

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned short ushort;

#if !defined(_MSC_VER)
// for non-Windows compilation, choose a 32-bit unsigned type here
#include <sys/types.h>
typedef u_int32_t __time32_t;
#endif

//---------------------------------------------------------------------------
// OLE API documentation
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnvss/html/vssauto.asp
//---------------------------------------------------------------------------

struct RECORD_HEADER {
  ulong   size;  
  char    type[2];
  short   checksum;
};

struct SSNAME {
  short   flags;		  // 00 = item, 01 == project
  char    name[34];    // short name
  ulong   nsmap;      // offset into the names.dat

};

// TODO: in the OLE API: VSSITEM_PROJECT = 0; VSSITEM_FILE = 1
#define SSITEM_PROJECT 1
#define SSITEM_FILE    2

enum eFileType
{
  eFileTypeBinary,
  eFileTypeText
};


struct DH {
  short   type;     // 1 Project, 2 File

  short   numberOfActions;

  // This is the last name that was given to the item
  SSNAME  name;	
  char    dummy[2];
  char    latestExt[2]; // .A or .B

  // offsets for records
  ulong   historyOffsetBegin;  // first EL Header
  ulong   historyOffsetLast;   // last EL oder FD HEader
  ulong   historyOffsetEnd;    // size of the file
};

struct DH_FILE : public DH {
  char    dummy4[20];
  
  // 0x00 == initial
  // 0x01 == locked
  // 0x02 == binary
  // 0x04 == store only latest revision
  // 0x20 == shared
  // 0x40 == checked out
  short   flag; 
  char    shareSrcSpec[10];

  ulong   offsetBFRecord;       // offset to the last BR record in the file
  ulong   offsetPFRecord;	      // offset to the last PF record in the file
  short   numberOfBranches;	    // number of the BF records
  short   numberOfReferences;	  // Reference count for the item

  ulong   offsetCFRecord1;  // file checked out, ptr to CF record
  ulong   offsetCFRecord2;  // file not checked out, ptr to CF record
  int     unknown;	        // changes after checkin

  char    dummy5[8];

  // diese scheinen immer Paare zu bilden, nach einem Checkin ändert sich
  // der d11, d21 
  // d12 == d22 == d32
  short   d11;
  short   d12;
  short   d21;
  short   d22;
  short   d31;
  short   d32;

  char    dummy6[4];

  // dito wie dxx, jedoch zum ersten mal nach einem Checkin
  short   e11;
  short   e12;
  short   e21;
  short   e22;
  short   e31;
  short   e32;

  char    dummy7[200];

  short   numberOfItems;        // including projects
  short   numberOfProjects;     // number of subprojects
};

struct DH_PROJECT : public DH {
  char    dummy4[20];
  
  char    parentSpec[258];       // of last checkout
  short   dummy7;

  char    parentPhys[10];
  char    dummy8[2];

  short   numberOfItems;        // including projects
  short   numberOfProjects;
};

enum eAction { 
  Labeled = 0,
  Created_Project = 1,
  Added_Project = 2,
  Added_File = 3,
  Destroyed_Project = 4,
  Destroyed_File = 5,
  Deleted_Project = 6,
  Deleted_File = 7,
  Recovered_Project = 8,
  Recovered_File = 9,
  Renamed_Project = 10,
  Renamed_File = 11,
  Moved_Project_From = 12,
  Moved_Project_To = 13,
  Shared_File = 14, // Share, Pin, Unpin	
  Branch_File = 15, // reported as Rollback im Parent Project
  Created_File = 16,
  Checked_in = 17,
  // missing action 18
  RollBack = 19,
  ArchiveVersion_File = 20, 
  // missing action 21, probably RestoreVersion_File
  Archive_File = 22,
  Archive_Project = 23,
  Restore_File = 24,
  Restore_Project = 25,

  /// --- pseudo actions ---
  Pinned_File = 26,
  Unpinned_File = 27


};

struct VERSION_RECORD {
  ulong   previous;	    // previous VERSION_RECORD
  ushort  actionID;     // eAction action;
  short   versionNumber;
  __time32_t  date;
  char    username[32];

  char    label[32];
  
  // This seems to be always be a pointer to the next record 
  // If (lengthComment != 0) this next record is the comment record
  ulong   offsetToNextRecordOrComment;

  // This offset seems to be NULL in most cases
  // In case of a LabelAction this is the offset to the so called LabelComment Record
  // In addition the lengthLabelComment is > 0
  ulong   offsetToLabelComment;

  // Length of the comment strings
  short   lengthComment;
  short   lengthLabelComment;
};

struct ITEM_ACTION {
  SSNAME  name;
  char    physical[10];
} ;

//struct CREATED_PROJECT_ACTION {
//  SSNAME  name;
//  char    physical[10];
//} ;

//struct ADDED_PROJECT_ACTION {
//  SSNAME  name;
//  char    physical[10];
//} ;

//struct ADDED_FILE_ACTION {
//  SSNAME  name;
//  char    physical[10];
//} ;

struct DESTROYED_ACTION {
  SSNAME  name;
  short   padding;
  char    physical[10];
} ;

//struct DESTROYED_PROJECT_ACTION {
//  SSNAME  name;
//  short   padding;
//  char    physical[10];
//} ;
//
//struct DESTROYED_FILE_ACTION {
//  SSNAME  name;
//  short   padding;
//  char    physical[10];
//} ;

//struct DELETED_PROJECT_ACTION {
//  SSNAME  name;
//  char    physical[10];
//} ;
//
//struct DELETED_FILE_ACTION {
//  SSNAME  name;
//  char    physical[10];
//} ;
//
//struct RECOVERED_PROJECT_ACTION {
//  SSNAME  name;
//  char    physical[10];
//} ;
//
//struct RECOVERED_FILE_ACTION {
//  SSNAME  name;
//  char    physical[10];
//} ;

struct RENAMED_ACTION {
  SSNAME  newName;
  SSNAME  name;  // old name
  char    physical[10];
} ;

struct SHARED_FILE_ACTION {
  char    srcPathSpec[260];
  SSNAME  name;
  short   subActionAndVersion; // -1: shared, 0: pinned; >0 unpinned, number denotes the Version which was unpinned
  short   pinnedToVersion;     // >0: pinned to version, ==0 unpinned
  short   padding2;            // reference ins project file? Nr des shares?
  char    physical[10];
} ;

struct MOVED_PROJECT_ACTION {
  char    pathSpec[260];
  SSNAME  name;
  char    physical[10];
} ;

//struct CREATED_FILE_ACTION {
//  SSNAME  name;
//  char    physical[10];
//} ;

struct CHECKED_IN_ACTION {
  ulong   offsetFileDelta;
  long    padding;
  char    checkInSpec[260];
};

struct ROLLBACK_ACTION {
  SSNAME  name;
  char    physical[10];
  char    parent[10];
};

struct ARCHIVE_VERSIONS_ACTION {
  SSNAME  name;
  char    physical[10];
  char    unknown2[14];
  short archiveVersion;
  char targetFile[252];
};

struct BRANCH_FILE_ACTION {
  SSNAME  name;
  char    physical[10];
  char    parent[10];
};

typedef struct {
  SSNAME name;
  char physical[10];
  short dummy;
  char filename[264];
} ARCHIVE_ACTION;

typedef struct {
  SSNAME name;
  char physical[10];
  short dummy;
  char filename[264];
} RESTORE_ACTION;

struct CF {
  char    user[32];
  char    padding[4];
  // zusammen 260?
  char    checkOutFolder[256];
  char    padding2[4];
  char    computer[32];
  // zusammen 260
//  char parentSpec[80];
//  char fileSpec2[60];
//  char padding3[120];
  char    parentSpec[260];

  char    comment[13];
  char    padding4[51];

  // initial alles 0
  // Check Out: flag1=01, flag2=40, flag3=00
  // Check In : flag1=00, flag2=00, flag3=10
  char    flag1;
  char    padding5;
  char    flag2;
  char    padding6[8];
  char    flag3;
  int     numberOfVersions;
} ;

struct FD {
  short   command; // 01 copy, 00 replace, 02
  short   dummy;
  ulong   start;
  ulong   end;
} ;

struct PF {
  ulong   previousOffset;
  char    parentPhys[10];
  short   padding;
} ;

struct BF {
  ulong   previousOffset;
  char    branchToPhys[10];
  short   padding;
} ;

//---------------------------------------------------------------------------
struct PROJECT_ENTRY{
  short   type;
  // 0x01 deleted
  // 0x02 store binary diffs
  // 0x04 store only latest revision
  // 0x08 shared
  // 0x100 ??, or only one byte??
  // 0x200 ??
  short   flags;
  SSNAME  name;
  short   pinnedToVersion;
  char    phys[10];
} ;

//---------------------------------------------------------------------------
struct HN {
  ulong   size;  
  char    type[2];
  short   checksum;
  char    unknown1[16];
  ulong   fileLen;
  char    unknown2[60];
};

struct NSENTRY{
  // 1: 8.3 Name
  // 2: voller name
  // 3: 28.3 Name //?
  // 10: Projekte
  short   id; 
  short   offset;
} ;

struct NSMAP{
  short   num;
  short   unknown;
} ;



//---------------------------------------------------------------------------
class CAction
{
public:
  static const char* ActionToString (eAction e);
};



#endif // !defined(AFX_SSTYPES_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
