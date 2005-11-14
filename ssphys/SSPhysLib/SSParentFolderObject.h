// SSParentFolderObject.h: interface for the SSParentFolderObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSPARENTFOLDEROBJECT_H__B896F3FB_BA6B_4A5E_B7E4_0FEDB9B6B930__INCLUDED_)
#define AFX_SSPARENTFOLDEROBJECT_H__B896F3FB_BA6B_4A5E_B7E4_0FEDB9B6B930__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSObject.h"

class SSParentFolderObject : public SSObject  
{
public:
	SSParentFolderObject(SSRecordPtr pRecord);
	virtual ~SSParentFolderObject();

  ulong       GetPreviousOffset ()  const { return GetData ()->previousOffset; }
  std::string GetParentPhys ()      const { return GetData ()->parentPhys; }

  SSParentFolderObject* GetPrevious ();

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }
  const PF* GetData () const { return reinterpret_cast<const PF*> (SSObject::GetData ()); }
  
  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os) const;
private:
};

#endif // !defined(AFX_SSPARENTFOLDEROBJECT_H__B896F3FB_BA6B_4A5E_B7E4_0FEDB9B6B930__INCLUDED_)
