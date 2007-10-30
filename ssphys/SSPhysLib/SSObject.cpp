// SSObject.cpp: implementation of the SSObject class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SSObject.h"
#include "SSItemInfoObject.h"
#include "SSNameObject.h"
#include "SSCheckOutObject.h"
#include "SSProjectObject.h"
#include "SSCommentObject.h"
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

SSObject::SSObject (SSRecordPtr pRecord, eType type)
  : m_pRecord (pRecord)
{
  if (pRecord && pRecord->GetType () != type)
    throw SSRecordException ("wrong record type");
}

SSObject::SSObject (SSRecordPtr pRecord)
  : m_pRecord (pRecord)
{
}

SSObject::SSObject ()
{
}

SSObject::~SSObject ()
{
}

SSObject* SSObject::MakeObject (SSRecordPtr pRecord)
{
  assert (pRecord);
  switch (pRecord->GetType ())
  {
  case eItemRecord:
    return SSItemInfoObject::MakeItemInfo (pRecord);
  case eNameCacheEntry:
    return new SSNameObject (pRecord);
  case eHistoryRecord:
    return new SSVersionObject (pRecord);
  case eCheckOutRecord:
    return new SSCheckOutObject (pRecord);
  case eProjectEntry:
    return new SSProjectObject (pRecord);
  case eCommentRecord:
    return new SSCommentObject (pRecord);
  case eParentFolder:
    return new SSParentFolderObject (pRecord);
  case eBranchFile:
    return new SSBranchFileObject (pRecord);
  case eFileDelta:
  case eNamesCache:
  case eUsersHeader:
  case eUser:
    return new SSObject (pRecord, pRecord->GetType ());
  default:
    {
      std::ostringstream ost;
      ost << "unknown record type \"" << pRecord->GetType() << "\" detected ";
      ost << "(offset 0x" << std::hex << pRecord->GetOffset() << std::dec << ")";
      throw SSRecordException (ost.str());
    }
  }
  return static_cast<SSObject*> (NULL);
}

//const byte* SSObject::GetDataPtr () const
//{ 
//  assert (m_pRecord);
//  return m_pRecord->GetBuffer ();
//}
//
//byte* SSObject::GetDataPtr ()
//{ 
//  assert (m_pRecord);
//  return m_pRecord->GetBuffer ();
//}

std::string SSObject::GetTypeName () const
{
  assert (*this);
  switch (GetType ())
  {
  case eItemRecord:
    return "ItemInfo";
  case eNameCacheEntry:
    return "NameCacheEntry";
  case eHistoryRecord:
    return "Version";
  case eCheckOutRecord:
    return "CheckOut";
  case eProjectEntry:
    return "Project";
  case eCommentRecord:
    return "Comment";
  case eParentFolder:
    return "ParentFolder";
  case eBranchFile:
    return "BranchFile";
  case eFileDelta:
    return "FileDelta";
  case eNamesCache:
    return "NamesCache";
  case eUsersHeader:
    return "UsersHeader";
  case eUser:
    return "User";
  };

  std::ostringstream ost;
  ost << GetRecord ()->GetRecordType();
  return ost.str();
}

void SSObject::ToXml (XMLNode* pParent) const
{
}

void SSObject::Dump (std::ostream& os) const
{
  os << "*** ";
  os << "Type: " << SSRecord::TypeToString (GetType ());
  os << ", Offset: 0x" << std::hex << GetOffset () << std::dec;
  os << " ***" << std::endl;
}
