// SSCommentObject.h: interface for the SSCommentObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSCOMMENTOBJECT_H__CA1555B2_0C25_42AE_9720_D670E5E447D6__INCLUDED_)
#define AFX_SSCOMMENTOBJECT_H__CA1555B2_0C25_42AE_9720_D670E5E447D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSObject.h"

class SSCommentObject : public SSObject
{
public:
	SSCommentObject(SSRecordPtr pRecord);
	virtual ~SSCommentObject();

  std::string GetComment () const;

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }
  
  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os) const;

protected:
  std::string m_Comment;
};

#endif // !defined(AFX_SSCOMMENTOBJECT_H__CA1555B2_0C25_42AE_9720_D670E5E447D6__INCLUDED_)
