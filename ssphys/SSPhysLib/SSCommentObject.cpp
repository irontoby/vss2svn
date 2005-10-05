// SSCommentObject.cpp: implementation of the SSCommentObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSCommentObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SSCommentObject::SSCommentObject(SSRecordPtr pRecord)
  : SSObject (pRecord, eCommentRecord)
{
  m_Comment = std::string (reinterpret_cast <const char*> (pRecord->GetBuffer()), pRecord->GetLen ());
}

SSCommentObject::~SSCommentObject()
{

}

std::string SSCommentObject::GetComment () const
{
  return m_Comment;
}

void SSCommentObject::ToXml (XMLNode* pParent) const
{
  XMLValue commentValue (pParent, GetComment ());
}

void SSCommentObject::Dump (std::ostream& os) const
{
  SSObject::Dump (os);
  os << GetComment () << std::endl;
}