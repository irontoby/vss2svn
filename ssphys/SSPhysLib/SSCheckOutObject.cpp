// SSCheckOutObject.cpp: implementation of the SSCheckOutObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSCheckOutObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SSCheckOutObject::SSCheckOutObject (SSRecordPtr pRecord)
  : SSObject (pRecord, eCheckOutRecord)
{
  if (pRecord->GetLen() < sizeof (CF))
    throw SSRecordException ("not enough data for checkout (CF) object");
}

SSCheckOutObject::~SSCheckOutObject ()
{
}

void SSCheckOutObject::ToXml (XMLNode* pParent) const
{
  XMLElement User           (pParent, "User", GetUser());
  XMLElement CheckOutFolder (pParent, "Folder", GetCheckOutFolder());
  XMLElement Computer       (pParent, "Computer", GetComputer());
  XMLElement ParentSpec     (pParent, "ParentSpec", GetParentSpec());
  XMLElement Comment        (pParent, "Comment", GetComment());
}
  
void SSCheckOutObject::Dump (std::ostream& os) const
{
  SSObject::Dump (os);

  os << "User: " << GetUser () << std::endl;
  os << "CheckOutFolder: " << GetCheckOutFolder() << std::endl;
  os << "Computer: " << GetComputer() << std::endl;
  os << "ParentSpec: " << GetParentSpec() << std::endl;
  os << "Comment: " << GetComment() << std::endl;
}
