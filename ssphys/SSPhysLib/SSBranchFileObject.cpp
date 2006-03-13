// SSBranchFileObject.cpp: implementation of the SSBranchFileObject class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SSBranchFileObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SSBranchFileObject::SSBranchFileObject(SSRecordPtr pRecord)
: SSObject (pRecord, eBranchFile)
{
  if (pRecord->GetLen() < sizeof (BF))
    throw SSRecordException ("not enough data for branch file (BF) object");
}

SSBranchFileObject::~SSBranchFileObject()
{

}

SSBranchFileObject* SSBranchFileObject::GetPrevious ()
{ 
  SSRecordPtr pRecord = GetFile()->GetRecord(GetPreviousOffset ());
  return new SSBranchFileObject (pRecord);
}

void SSBranchFileObject::ToXml (XMLNode* pParent) const
{ 
  XMLElement previous  (pParent, "PreviousOffset",  GetPreviousOffset());
  XMLElement branch    (pParent, "BranchToPhys",    GetBranchToPhys());
}

void SSBranchFileObject::Dump (std::ostream& os) const
{ 
  SSObject::Dump (os);

  os << "Previous    : 0x" << std::hex << GetPreviousOffset() << std::dec << std::endl;
  os << "BranchToPhys:   " << GetBranchToPhys() << std::endl;
}
