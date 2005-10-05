// SSProjectObject.h: interface for the SSProjectObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSPROJECTOBJECT_H__77FEAD1D_3579_4375_BF98_912CB41B8F96__INCLUDED_)
#define AFX_SSPROJECTOBJECT_H__77FEAD1D_3579_4375_BF98_912CB41B8F96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSObject.h"

class SSProjectObject : public SSObject  
{
public:
	SSProjectObject(const PROJECT_ENTRY& pe);
	SSProjectObject(SSRecordPtr pRecord);
	virtual ~SSProjectObject();

  std::string GetName ()      const;
  std::string GetPhysFile ()  const;

  int GetType ()              const;
  eFileType GetFileType ()    const;
  int GetPinnedToVersion ()   const;

  bool IsDeleted ()           const;
  bool IsStoreBinaryDiff ()   const;
  bool IsStoreLatestRev ()    const;
  bool IsShared ()            const;

  void Delete ();
  void Recover ();
  void Rename (SSNAME oldName, SSNAME newName);
  void Pin (int version);
  
  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }

  const PROJECT_ENTRY* GetData () const { return (const PROJECT_ENTRY*) SSObject::GetData (); }

//  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os) const;

protected:
  // convinience pointer into the data buffer
  PROJECT_ENTRY* m_pProjectEntry;
};

#endif // !defined(AFX_SSPROJECTOBJECT_H__77FEAD1D_3579_4375_BF98_912CB41B8F96__INCLUDED_)
