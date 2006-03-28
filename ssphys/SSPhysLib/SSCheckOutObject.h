// SSCheckOutObject.h: interface for the SSCheckOutObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSCHECKOUTOBJECT_H__D987E1D1_744F_4D0F_BB42_A2D3A52F0171__INCLUDED_)
#define AFX_SSCHECKOUTOBJECT_H__D987E1D1_744F_4D0F_BB42_A2D3A52F0171__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSObject.h"

class SSCheckOutObject : public SSObject  
{
public:
	SSCheckOutObject(SSRecordPtr pRecord);
	virtual ~SSCheckOutObject();
  
  // accessors
  std::string GetUser ()              const { return std::string (GetData()->user); }
  std::string GetCheckOutFolder ()    const { return std::string (GetData()->checkOutFolder); }
  std::string GetComputer ()          const { return std::string (GetData()->computer); }
  std::string GetParentSpec ()        const { return std::string (GetData()->parentSpec); }
  std::string GetComment ()           const { return std::string (GetData()->comment); }
  char        GetFlag1 ()             const { return GetData()->flag1; }
  char        GetFlag2 ()             const { return GetData()->flag2; }
  char        GetFlag3 ()             const { return GetData()->flag3; }
  int         GetNumberOfVersions ()  const { return GetData()->numberOfVersions; }

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }
  const CF* GetData () const { return reinterpret_cast <const CF*> (SSObject::GetData ()); }

  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os) const;
protected:
};


#endif // !defined(AFX_SSCHECKOUTOBJECT_H__D987E1D1_744F_4D0F_BB42_A2D3A52F0171__INCLUDED_)
