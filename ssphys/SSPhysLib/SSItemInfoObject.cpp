// SSItemInfoObject.cpp: implementation of the SSItems class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSFiles.h"
#include "SSName.h"
#include "SSItemInfoObject.h"
#include "SSParentFolderObject.h"
#include "SSBranchFileObject.h"

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SSItemInfoObject::SSItemInfoObject (SSRecordPtr pRecord)
  : SSObject (pRecord, eItemRecord)
{
  if (pRecord->GetLen() < sizeof (DH))
    throw SSRecordException ("not enough data for info object");
  
  m_pInfoItem = GetData ();
}

SSItemInfoObject::~SSItemInfoObject ()
{
}

SSItemInfoObject* SSItemInfoObject::MakeItemInfo (SSRecordPtr pRecord)
{
  if (pRecord->GetLen () < sizeof (DH))
    throw SSException ("not enough bytes for DH Header in record");

  const DH* pDh = reinterpret_cast<const DH*> (pRecord->GetBuffer ());
  if (pDh->type == 1)
    return new SSProjectItem (pRecord);
  else if (pDh->type == 2)
    return new SSFileItem (pRecord);
  else 
    throw SSException ("unsupported item type");

  return (SSItemInfoObject*) NULL;
}



SSVersionObject SSItemInfoObject::GetVersion (int i)
{
  SSFileImpPtr filePtr = GetFile ();

  // TODO: Cache versions
  SSVersionObject version (filePtr->GetRecord (GetHistoryOffsetLast ()));

  while (version && version.GetVersionNumber () != i) 
  {
    version = version.GetPreviousObject();
  }

  return version;
}


bool SSItemInfoObject::Get (int ver, const char* dest)
{
//  SSFileImpPtr filePtr = GetFile ();
//
//  std::string lastVersion = filePtr->GetFileName () + GetLatestExt ();
//  char tmrFile[2][255];
//  char* ptr[2] = {tmrFile[0], (char*)dest};
//  ptr[0] = tmpnam (ptr[0]);
//  
//  CopyFile (lastVersion.c_str(), ptr[0]);
//
//  SSVersionObject version (filePtr->GetRecord (GetHistoryOffsetLast ()));
//  
//  while (version && version.GetVersionNumber() > ver)
//  {
//    if (version.GetActionId() == Checked_in)
//    {
//      SSCheckedInAction* pAction = dynamic_cast <SSCheckedInAction*> (version.GetAction());
//      SSRecordPtr pRecord = pAction->GetFileDelta();
//      ReverseDelta (ptr[0], pRecord->GetBuffer(), pRecord->GetLen(), ptr[1]);
//      std::swap (ptr[0], ptr[1]);
//    }
//
//    version = version.GetPrevious ();
//  }
//
//  if (0 != strcmp (ptr[0], dest))
//  {
//    CopyFile (ptr[0], dest);
//    _unlink (ptr[0]);
//  }
//  else
//  {
//    _unlink (ptr[1]);
//  }
  return false;
}

bool SSItemInfoObject::Validate()
{
  SSFileImpPtr filePtr = GetFile ();

  bool retval = true;
  retval &= warn_if (GetLatestExt() != ".A" && GetLatestExt() != ".B");
  retval &= warn_if (GetHistoryOffsetEnd() != filePtr->Size());
  retval &= warn_if (filePtr->GetRecord (GetHistoryOffsetLast ())->GetType() != eHistoryRecord);
  retval &= warn_if (filePtr->GetRecord (GetHistoryOffsetBegin ())->GetType() != eHistoryRecord);

  int nCount = 0;
  long offset = GetHistoryOffsetLast ();
  eAction lastActionId = Labeled;

  do {
    SSRecordPtr pRecord = filePtr->GetRecord (offset);
    retval &= warn_if (pRecord->GetType() != eHistoryRecord);

    std::auto_ptr <SSObject> objectPtr (SSObject::MakeObject(pRecord));
    SSVersionObject* pVersion = dynamic_cast<SSVersionObject*> (objectPtr.get ());
    if (pVersion)
    {
      SSVersionObject previous (pVersion->GetPreviousObject ());
      offset = previous ? previous.GetOffset() : NULL; 
      if (pVersion->GetActionID() == RollBack)
        nCount += pVersion->GetVersionNumber() - 1;
      lastActionId = pVersion->GetActionID();
    }
    else
      offset = NULL;

    ++nCount;
  } while (offset != NULL /*GetHistoryOffsetBegin ()*/);
  
  retval &= warn_if (nCount != GetNumberOfActions ());
  retval &= warn_if (lastActionId != Created_File && lastActionId != Created_Project && lastActionId != RollBack);
  return retval;
}

void SSItemInfoObject::ToXml (XMLNode* pParent) const
{
  XMLElement type  (pParent, "Type", GetType());
  XMLElement data  (pParent, "DataFileName", GetDataFileName ());
  XMLElement ext   (pParent, "LatestExt", GetLatestExt());
  GetSSName().ToXml (pParent);
  XMLElement noActions (pParent, "NumberOfActions", GetNumberOfActions());
}

void SSItemInfoObject::Dump (std::ostream& os) const
{
  SSObject::Dump (os);

  SSFileImpPtr filePtr = GetFile ();

  os << "Item Type:                      ";
 
  assert (m_pInfoItem);
  switch (m_pInfoItem->type)
  {
  case 1: os << "Project" << std::endl; break;
  case 2: os << "File" << std::endl; break;
  default: os << "Unknown (" << m_pInfoItem->type << ")" << std::endl; break;
  }

  SSName ssName (GetSSName ());
  os << "Last Name:                      " << ssName << std::endl;
//  os << "Number Of Records:              " << GetNumberOfRecords () << std::endl;
  os << "LatestExt of last version:        " << m_pInfoItem->latestExt[0] << m_pInfoItem->latestExt[1] << std::endl;
  os << "Offset to first History record: 0x" << std::hex << GetHistoryOffsetBegin()<< std::dec << std::endl;
  os << "Offset to last History record:  0x" << std::hex << GetHistoryOffsetLast() << std::dec << std::endl;
  os << "Offset to the end of the file:  0x" << std::hex << GetHistoryOffsetEnd()  << std::dec << std::endl;
  os << "Size of the file:               0x" << std::hex << filePtr->Size() << std::dec << std::endl;
}





//---------------------------------------------------------------------------
SSProjectItem::SSProjectItem (SSRecordPtr pRecord)
  : SSItemInfoObject (pRecord)
{
  if (pRecord->GetLen() < sizeof (DH_PROJECT))
    throw SSRecordException ("not enough data for project info object");

  m_pProjectInfo = GetData ();
}

void SSProjectItem::ToXml (XMLNode* pParent) const
{
  SSItemInfoObject::ToXml (pParent);
  XMLElement spec        (pParent, "ParentSpec", GetParentSpec ());
  XMLElement phys        (pParent, "ParentPhys", GetParentPhys ());
  XMLElement noitems     (pParent, "NumberOfItems", GetNumberOfItems ());
  XMLElement noProjects  (pParent, "NumberOfProjects", GetNumberOfProjects ());
}

void SSProjectItem::Dump (std::ostream& os) const
{
  SSItemInfoObject::Dump (os);
  
//  Hexdump (oss, dummy4, 20);
  os << "last parent spec:      " << GetParentSpec () << std::endl;
  os << "parentPhys:            " << GetParentPhys () << std::endl;
  os << "Number of Items:       " << GetNumberOfItems () << std::endl;
  os << "Number of Projects:    " << GetNumberOfProjects () << std::endl;
}




//---------------------------------------------------------------------------
SSFileItem::SSFileItem (SSRecordPtr pRecord)
  : SSItemInfoObject (pRecord)
{
  if (pRecord->GetLen() < sizeof (DH_FILE))
    throw SSRecordException ("not enough data for file info object");

  m_pFileInfo = GetData ();
}

bool SSFileItem::Validate()
{
  SSItemInfoObject::Validate ();

  const DH_FILE* pFileInfoItem = GetData();

  bool retval = true;
  retval &= warn_if (pFileInfoItem->numberOfItems > 0);
  retval &= warn_if (pFileInfoItem->numberOfProjects > 0);

  byte knownFlags[] = 
  {
    0x00, // initial
//    0x01, // ??? locked
    0x02, // binary
    0x04, // store only latest revision
    0x20, // shared
    0x22, // shared binary
    0x41, // checked out, locked
//    0x42, // ???
    0x43,  // checked out binary, locked
    0x61, // shared, checked out, locked
    0x63, // shared, checked out, locked, binary
//    0x101, // ??
//    0x102 // ??
  };
  for (int i = 0; i < countof (knownFlags); i++)
    if (pFileInfoItem->flag == knownFlags[i])  
      break;
  
  if (i == countof (knownFlags))
  {
    std::strstream strm;
    strm << ("unknown combination of flags in the FileInfo record: 0x") << std::hex << pFileInfoItem->flag << std::ends;
    Warning (strm.str());
    retval = false;
  }
  
  return retval;
}

bool SSFileItem::GetLocked () const
{
  return ((GetFlag () & 0x01) == 0x01) ? true : false;
}

eFileType SSFileItem::GetFileType () const
{
  return ((GetFlag () & 0x02) == 0x02) ? eFileTypeBinary : eFileTypeText;
}

bool SSFileItem::GetStoreOnlyLatestRev () const
{
  return ((GetFlag () & 0x04) == 0x04) ? true : false;
}

bool SSFileItem::GetShared () const
{
  return ((GetFlag () & 0x20) == 0x20) ? true : false;
}

bool SSFileItem::GetCheckedOut () const
{
  return ((GetFlag () & 0x40) == 0x40) ? true : false;
}


SSParentFolderObject* SSFileItem::GetFirstParentFolder ()
{ 
  SSRecordPtr pRecord = GetFile()->GetRecord(GetOffsetPFRecord ());
  return new SSParentFolderObject (pRecord);
}

SSBranchFileObject* SSFileItem::GetFirstBranchFile ()
{ 
  SSRecordPtr pRecord = GetFile()->GetRecord(GetOffsetBFRecord ());
  return new SSBranchFileObject (pRecord);
}

void SSFileItem::ToXml (XMLNode* pParent) const
{
  SSItemInfoObject::ToXml (pParent);
  std::stringstream strm; strm << std::hex << "0x" << GetFlag ();
  XMLElement flags       (pParent, "Flags", strm.str());
  XMLElement locked      (pParent, "Locked", GetLocked ());
  XMLElement binary      (pParent, "Binary", GetFileType () == eFileTypeBinary ? true : false);
  XMLElement store       (pParent, "StoreOnlyLatestRev", GetStoreOnlyLatestRev ());
  XMLElement checkedOut  (pParent, "CheckedOut", GetCheckedOut ());
  XMLElement shared      (pParent, "Shared", GetShared ());
  XMLElement sharedSrc   (pParent, "ShareSrc", GetShareSrcPhys());
  XMLElement ref         (pParent, "NumberOfReferences", GetNumberOfReferences());
  XMLElement branch      (pParent, "NumberOfBranches", GetNumberOfBranches ()); 
}

void SSFileItem::Dump (std::ostream& os) const
{
  SSItemInfoObject::Dump (os);

//  Hexdump (oss, dummy4, 20);

  const DH_FILE* pFileInfoItem = GetData ();
  os << "Status:               0x" << std::hex << pFileInfoItem->flag << std::dec << ": ";
  if (pFileInfoItem->flag == 0x00)
    os << "normal";
  os << ((GetFileType () == eFileTypeBinary) ? "binary" : "text");
  os << (GetStoreOnlyLatestRev () ?  ", store only latest revision" : "");
  os << (GetLocked () ? ", locked" : "");
  os << (GetCheckedOut () ? ", checked out, " : "");
  os << (GetShared () ? ", shared" : "");
  os << std::endl;

  if (pFileInfoItem->flag & 0xff98)
    os << "unknown flags encountered" << std::endl;

  os << "Share source physical file:         " << pFileInfoItem->shareSrcSpec << std::endl;

  os << "Offset to first parent record:      0x" << std::hex << pFileInfoItem->offsetPFRecord << std::dec << std::endl;
  os << "Reference count:                    " << pFileInfoItem->numberOfReferences << std::endl;
  os << "Offset to 1st checkout record:      0x" << std::hex << pFileInfoItem->offsetCFRecord1 << std::dec << std::endl;
  os << "Offset to 2nd checkout record:      0x" << std::hex << pFileInfoItem->offsetCFRecord2 << std::dec << std::endl;

  os << "Number of Items:                    " << std::hex << pFileInfoItem->numberOfItems << std::dec << std::endl;
  os << "Number of Projects:                 " << std::hex << pFileInfoItem->numberOfProjects << std::dec << std::endl;
}

