// SSParentFolderObject.cpp: implementation of the SSParentFolderObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSParentFolderObject.h"
#include "SSFiles.h"

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

SSParentFolderObject::SSParentFolderObject(SSRecordPtr pRecord)
: SSObject (pRecord, eParentFolder)
{
  if (pRecord->GetLen() < sizeof (PF))
    throw SSRecordException ("not enough data for parent folder (PF) object");
}

SSParentFolderObject::~SSParentFolderObject()
{

}

SSParentFolderObject* SSParentFolderObject::GetPrevious ()
{ 
  SSRecordPtr pRecord = GetFile()->GetRecord(GetPreviousOffset ());
  return new SSParentFolderObject (pRecord);
}

void SSParentFolderObject::ToXml (XMLNode* pParent) const
{
  XMLElement previous  (pParent, "PreviousOffset",  GetPreviousOffset());
  XMLElement parent    (pParent, "ParentPhys", GetParentPhys());
}

void SSParentFolderObject::Dump (std::ostream& os) const
{
  SSObject::Dump (os);

  os << "Previous: 0x" << std::hex << GetPreviousOffset() << std::dec << std::endl;
  os << "Parent  :   " << GetParentPhys() << std::endl;
}