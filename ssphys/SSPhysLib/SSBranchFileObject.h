// SSBranchFileObject.h: interface for the SSBranchFileObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSBRANCHFILEOBJECT_H__A0EACDCC_E735_4F39_B5F4_609876C5878B__INCLUDED_)
#define AFX_SSBRANCHFILEOBJECT_H__A0EACDCC_E735_4F39_B5F4_609876C5878B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSObject.h"

class SSBranchFileObject : public SSObject  
{
public:
	SSBranchFileObject(SSRecordPtr pRecord);
	virtual ~SSBranchFileObject();

  ulong       GetPreviousOffset ()  const { return GetData ()->PreviousOffset; }
  std::string GetBranchToPhys ()    const { return GetData ()->BranchToPhys; }

  SSBranchFileObject* GetPrevious ();

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }
  const BF* GetData () const { return reinterpret_cast<const BF*> (SSObject::GetData ()); }
  
  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os) const;
private:
};


#endif // !defined(AFX_SSBRANCHFILEOBJECT_H__A0EACDCC_E735_4F39_B5F4_609876C5878B__INCLUDED_)
