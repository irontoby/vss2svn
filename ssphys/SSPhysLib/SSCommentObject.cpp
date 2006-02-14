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
  int len = std::min (strlen (reinterpret_cast <const char*> (pRecord->GetBuffer())), (size_t) pRecord->GetLen ());
  m_Comment = std::string (reinterpret_cast <const char*> (pRecord->GetBuffer()), len);
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
  XMLText commentValue (pParent, GetComment ());
}

void SSCommentObject::Dump (std::ostream& os) const
{
  SSObject::Dump (os);
  os << GetComment () << std::endl;
}