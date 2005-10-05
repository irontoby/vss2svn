// SSObject.h: interface for the SSObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSOBJECT_H__C8DBBC42_EE5F_4F9B_8935_5831008AB2BF__INCLUDED_)
#define AFX_SSOBJECT_H__C8DBBC42_EE5F_4F9B_8935_5831008AB2BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSRecord.h"
#include "XML.h"
#include <list>

class SSObject;
class SSVersionObject;
class SSCheckOutObject;
class SSNameObject;
class SSCommentObject;
class SSProjectObject;
class SSParentFolderObject;
class SSBranchFileObject;
//class SSItemInfoObject;
class SSFileItem;
class SSProjectItem;

class ISSContext
{
public:
  virtual ~ISSContext () {}; 
};

class ISSObjectVisitor 
{
public:
  virtual void Apply(const SSObject& object, const ISSContext* pCtx) = 0;

  virtual void Apply(const SSVersionObject& object, const ISSContext* pCtx) = 0;
  virtual void Apply(const SSCheckOutObject& object, const ISSContext* pCtx) = 0;
  virtual void Apply(const SSNameObject& object, const ISSContext* pCtx) = 0;
  virtual void Apply(const SSCommentObject& object, const ISSContext* pCtx) = 0;
  virtual void Apply(const SSProjectObject& object, const ISSContext* pCtx) = 0;
  virtual void Apply(const SSParentFolderObject& object, const ISSContext* pCtx) = 0;
  virtual void Apply(const SSBranchFileObject& object, const ISSContext* pCtx) = 0;

//  virtual void Apply(const SSItemInfoObject& object, const ISSContext* pCtx) = 0;
  virtual void Apply(const SSFileItem& object, const ISSContext* pCtx) = 0;
  virtual void Apply(const SSProjectItem& object, const ISSContext* pCtx) = 0;
};



class SSObject
{
protected:
  SSObject (SSRecordPtr pRecord, eType type);
  SSObject ();

public:
  virtual ~SSObject ();

  static SSObject* MakeObject (SSRecordPtr pRecord);

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); };
  virtual bool Validate ()              { return true; }

  SSFileImpPtr GetFile ()  const { return m_pRecord->GetFileImp (); }
  eType GetType ()         const { return m_pRecord->GetType (); }
  long GetOffset ()        const { return m_pRecord->GetOffset(); }

  const byte* GetData ()   const { return m_pRecord->GetBuffer (); }

  SSRecordPtr GetRecord () const { return m_pRecord; }

  operator bool ()         const { return m_pRecord; }
  
  virtual std::string GetName ()        const;
  virtual void ToXml (XMLNode* pParent) const;
  virtual void Dump (std::ostream& os)  const;

protected:
//  const byte* GetDataPtr () const;
//  byte* GetDataPtr ();

private:

  SSRecordPtr m_pRecord;
};


#endif // !defined(AFX_SSOBJECT_H__C8DBBC42_EE5F_4F9B_8935_5831008AB2BF__INCLUDED_)
