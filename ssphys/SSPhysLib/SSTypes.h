// SSTypes.h:structure definitions for SourceSafe files
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSTYPES_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
#define AFX_SSTYPES_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "time.h"
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/control.hpp>
#include <boost/preprocessor/comparison/greater.hpp>

typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned short ushort;

inline std::string toString (const char* ch, int len)
{
  return std::string (ch, len);
}

inline std::string timeToString (const time_t& t, int len)
{
  char date[12];
  char time[12];
  const char* format2 = "Date: %-8s   Time: %s";
  const tm* ttm = gmtime (&t);//localtime (&versionDate);
  strftime (date, countof (date), "%x", ttm);
  strftime (time, countof (time), "%X", ttm);
  char line2[60]; _snprintf (line2, 60, format2, date, time);
  return line2;
}

//---------------------------------------------------------------------------
// OLE API documentation
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnvss/html/vssauto.asp

//---------------------------------------------------------------------------
//struct RECORD_HEADER {
//  ulong size;  
//  char type[2];
//  short checksum;
//};

#define SS_TYPE_I(T)        BOOST_PP_SEQ_ELEM(0, T)
#define SS_VARIABLE_I(T)    BOOST_PP_SEQ_ELEM(1, T)
#define SS_ARRAYSIZE_I(T)   BOOST_PP_SEQ_ELEM(2, T)
#define SS_RETURNTYPE_I(T)  BOOST_PP_SEQ_ELEM(3, T)
#define SS_CONVFUNC_I(T)    BOOST_PP_SEQ_ELEM(4, T)

#define SS_IS_ARRAY_I(T)    BOOST_PP_IF (BOOST_PP_GREATER(BOOST_PP_SEQ_SIZE(T),2), BOOST_PP_GREATER(SS_ARRAYSIZE_I(T),1), 0)
#define SS_ARRAY_DECL(T)    BOOST_PP_CAT([, BOOST_PP_CAT(SS_ARRAYSIZE_I(T), ]))

#define SS_IS_RETURNTYPE_I(T)  BOOST_PP_GREATER(BOOST_PP_SEQ_SIZE(T),3)
#define SS_IS_CONVFUNC_I(T)    BOOST_PP_GREATER(BOOST_PP_SEQ_SIZE(T),4)

#define SS_TYPE(T)          SS_TYPE_I(T)
#define SS_NAME(T)          SS_VARIABLE_I(T)
#define SS_VARIABLE_DECL(T) BOOST_PP_IF(SS_IS_ARRAY_I(T), BOOST_PP_CAT(SS_VARIABLE_I(T), SS_ARRAY_DECL(T)), SS_VARIABLE_I(T))
#define SS_RETURNTYPE(T)    BOOST_PP_IF(SS_IS_RETURNTYPE_I(T), SS_RETURNTYPE_I(T), SS_TYPE_I(T))

#define SS_ACCESSOR(DATA, T)        return (DATA->SS_NAME(T));
#define SS_CONVERTER(DATA, T, FUNC) return FUNC (DATA->SS_NAME(T), SS_ARRAYSIZE_I(T));


#define DEFINE_ELEMENTS(r, data, T) \
 SS_TYPE(T) SS_VARIABLE_DECL(T);

#define SS_STRUCT(name, fields) \
  struct name \
  { \
    BOOST_PP_SEQ_FOR_EACH(DEFINE_ELEMENTS, ~, fields) \
  };

#define DEFINE_ACCESSORS(R, DATA, T)                  \
  SS_RETURNTYPE(T) BOOST_PP_CAT (Get, SS_NAME(T)) () const\
  { \
    BOOST_PP_IF(SS_IS_CONVFUNC_I(T), SS_CONVERTER(DATA, T, SS_CONVFUNC_I(T)), SS_ACCESSOR(DATA, T)); \
  }


#define RECORD_HEADER_SEQ                               \
        ((ulong)  (size))                               \
        ((char)   (type) (2) (std::string) (toString))  \
        ((short)  (checksum))

SS_STRUCT(RECORD_HEADER, RECORD_HEADER_SEQ);


//struct SSNAME {
//  short flags;		// 00 = item, 01 == project
//  char name[34];        // short name
//  ulong nsmap;           // offset into the names.dat
//
//};

#define SSNAME_SEQ                                                                       \
        ((short) (flags))                               /* 00 = item, 01 == project  */  \
        ((char)  (name) (34) (std::string) (toString))  /* short name                */  \
        ((ulong) (nsmap))                               /* offset into the names.dat */

SS_STRUCT(SSNAME, SSNAME_SEQ);

// TODO: in the OLE API: VSSITEM_PROJECT = 0; VSSITEM_FILE = 1
#define SSITEM_PROJECT 1
#define SSITEM_FILE    2

enum eFileType
{
  eFileTypeBinary,
  eFileTypeText
};


//struct DH {
//  short type; // 1 Project, 2 File
//
//  short numberOfRecords;
//
//  // This is the last name that was given to the item
//  SSNAME name;	
//  char dummy2[2];
//  char fileExt[2]; // .A or .B
//
//  // offsets for records
//  ulong i1;  // first EL Header
//  ulong i2;  // last EL oder FD HEader
//  ulong i3;  // size of the file
//};

#define DH_SEQ                                                                \
        ((short)  (Type))                /* 1 Project, 2 File  */             \
        ((short)  (NumberOfActions))                                          \
                                                                              \
        /* This is the last name that was given to the item */                \
        ((SSNAME) (SSName))                                                   \
        ((char)   (Dummy)     (2) (std::string) (toString))                   \
        ((char)   (LatestExt) (2) (std::string) (toString))   /* .A or .B */  \
                                                                              \
        /* offsets for records */                                             \
        ((ulong)  (HistoryOffsetBegin)) /* first EL Header */                 \
        ((ulong)  (HistoryOffsetLast))  /* last EL oder FD HEader */          \
        ((ulong)  (HistoryOffsetEnd))   /* size of the file */

SS_STRUCT (DH, DH_SEQ);



struct DH_FILE : public DH {
  char dummy4[20];
  
  // 0x00 == initial
  // 0x02 == binary
  // 0x04 == store only latest revision
  // 0x41 == checked out
  // 0x20 == shared
  short Flag; 
  char ShareSrcSpec[10];

  ulong OffsetBFRecord;       // offset to the last BR record in the file
  ulong OffsetPFRecord;	      // offset to the last PF record in the file
  short NumberOfBranches;	    // number of the BF records
  short NumberOfReferences;	  // Reference count for the item

  ulong OffsetCFRecord1;  // file checked out, ptr to CF record
  ulong OffsetCFRecord2;  // file not checked out, ptr to CF record
  int unknown;	        // changes after checkin

  char dummy5[8];

  // diese scheinen immer Paare zu bilden, nach einem Checkin ändert sich
  // der d11, d21 
  // d12 == d22 == d32
  short d11;
  short d12;
  short d21;
  short d22;
  short d31;
  short d32;

  char dummy6[4];

  // dito wie dxx, jedoch zum ersten mal nach einem Checkin
  short e11;
  short e12;
  short e21;
  short e22;
  short e31;
  short e32;

  char dummy7[200];

  short NumberOfItems;        // including projects
  short NumberOfProjects;     // number of subprojects
};

struct DH_PROJECT : public DH {
  char dummy4[20];
  
  char ParentSpec[258];       // of last checkout
  short dummy7;

  char ParentPhys[10];
  char dummy8[2];

  short NumberOfItems;        // including projects
  short NumberOfProjects;
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
  // missing action 12,
  // missing action 13
  Shared_File = 14, // Share, Pin, Unpin	
  Branch_File = 15, // reported as Rollback im Parent Project
  Created_File = 16,
  Checked_in = 17,
  // missing action 18
  RollBack = 19,
  // missing known actions: archives, restores

  /// --- pseudo actions ---
  Pinned_File = 20,
  Unpinned_File = 21
};

inline eAction ushortToAction (const ushort& v, int)
{
  return static_cast <eAction> (v);
}

#define VERSION_RECORD_SEQ                                                        \
        ((ulong)  (Previous))     /* previous VERSION_RECORD  */                  \
        ((ushort) (ActionID) (1) (eAction)   (ushortToAction)) /* eAction action */ \
        ((short)  (VersionNumber))                                                \
        ((time_t) (Date)  (1)  (std::string) (timeToString))                      \
        ((char)   (Username)  (32) (std::string) (toString))                          \
                                                                                  \
        ((char)   (Label) (32) (std::string) (toString))                          \
                                                                                  \
        /* This seems to be always be a pointer to the next record                \
           If (lengthComment != 0) this next record is the comment record */      \
        ((ulong)  (OffsetToNextRecordOrComment))                                  \
                                                                                  \
        /* This offset seems to be NULL in most cases                             \
           In case of a LabelAction this is the offset to the so called LabelComment Record \
           In addition the lengthLabelComment is > 0 */                           \
        ((ulong)  (OffsetToLabelComment))                                         \
                                                                                  \
        /* Length of the comment strings */                                       \
        ((short)  (LengthComment))                                                \
        ((short)  (LengthLabelComment))

SS_STRUCT (VERSION_RECORD, VERSION_RECORD_SEQ);

//struct VERSION_RECORD {
//  ulong previous;	// previous VERSION_RECORD
//  ushort action; // eAction action;
//  short version;
//  time_t date;
//  char user[32];
//
//  char label[32];
//  
//  // This seems to be always be a pointer to the next record 
//  // If (lengthComment != 0) this next record is the comment record
//  ulong offsetToNextRecordOrComment;
//
//  // This offset seems to be NULL in most cases
//  // In case of a LabelAction this is the offset to the so called LabelComment Record
//  // In addition the lengthLabelComment is > 0
//  ulong offsetToLabelComment;
//
//  // Length of the comment strings
//  short lengthComment;
//  short lengthLabelComment;
//};

struct ITEM_ACTION {
  SSNAME name;
  char physical[10];
} ;

//struct CREATED_PROJECT_ACTION {
//  SSNAME name;
//  char physical[10];
//} ;

//struct ADDED_PROJECT_ACTION {
//  SSNAME name;
//  char physical[10];
//} ;

//struct ADDED_FILE_ACTION {
//  SSNAME name;
//  char physical[10];
//} ;

struct DESTROYED_ACTION {
  SSNAME name;
  short padding;
  char physical[10];
} ;

//struct DESTROYED_PROJECT_ACTION {
//  SSNAME name;
//  short padding;
//  char physical[10];
//} ;
//
//struct DESTROYED_FILE_ACTION {
//  SSNAME name;
//  short padding;
//  char physical[10];
//} ;

//struct DELETED_PROJECT_ACTION {
//  SSNAME name;
//  char physical[10];
//} ;
//
//struct DELETED_FILE_ACTION {
//  SSNAME name;
//  char physical[10];
//} ;
//
//struct RECOVERED_PROJECT_ACTION {
//  SSNAME name;
//  char physical[10];
//} ;
//
//struct RECOVERED_FILE_ACTION {
//  SSNAME name;
//  char physical[10];
//} ;

struct RENAMED_ACTION {
  SSNAME newName;
  SSNAME name;  // old name
  char physical[10];
} ;

struct SHARED_FILE_ACTION {
  char srcPathSpec[260];
  SSNAME name;
  short subActionAndVersion; // -1: shared, 0: pinned; >0 unpinned, number denotes the Version which was unpinned
  short pinnedToVersion;     // >0: pinned to version, ==0 unpinned
  short padding2;            // reference ins project file? Nr des shares?
  char  physical[10];
} ;

//struct CREATED_FILE_ACTION {
//  SSNAME name;
//  char physical[10];
//} ;

struct CHECKED_IN_ACTION {
  ulong offsetFileDelta;
  long padding;
  char checkInSpec[260];
};

struct ROLLBACK_ACTION {
  SSNAME name;
  char physical[10];
  char parent[10];
};

struct BRANCH_FILE_ACTION {
  SSNAME name;
  char physical[10];
  char parent[10];
};

struct CF {
  char User[32];
  char Padding[4];
  // zusammen 260?
  char CheckOutFolder[256];
  char Padding2[4];
  char Computer[32];
  // zusammen 260
//  char parentSpec[80];
//  char fileSpec2[60];
//  char padding3[120];
  char ParentSpec[260];

  char Comment[13];
  char Padding4[51];

  // initial alles 0
  // Check Out: flag1=01, flag2=40, flag3=00
  // Check In : flag1=00, flag2=00, flag3=10
  char Flag1;
  char Padding5;
  char Flag2;
  char Padding6[8];
  char Flag3;
  int NumberOfVersions;
} ;

struct FD {
  short command; // 01 copy, 00 replace, 02
  short dummy;
  ulong start;
  ulong end;
} ;

struct PF {
  ulong PreviousOffset;
  char  ParentPhys[10];
  short padding;
} ;

struct BF {
  ulong PreviousOffset;
  char  BranchToPhys[10];
  short padding;
} ;

//---------------------------------------------------------------------------
struct PROJECT_ENTRY{
  short type;
  // 0x01 deleted
  // 0x02 store binary diffs
  // 0x04 store only latest revision
  // 0x08 shared
  short flags;
  SSNAME name;
  short pinnedToVersion;
  char phys[10];
} ;

//---------------------------------------------------------------------------
struct HN {
  ulong size;  
  char type[2];
  short checksum;
  char unknown1[16];
  ulong fileLen;
  char unknown2[60];
};

struct NSENTRY{
  // 1: 8.3 Name
  // 2: voller name
  // 3: 28.3 Name //?
  // 10: Projekte
  short id; 
  short offset;
} ;

struct NSMAP{
  short num;
  short unknown;
} ;



//---------------------------------------------------------------------------
class CAction
{
public:
  static const char* ActionToString (eAction e);
};



#endif // !defined(AFX_SSTYPES_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
