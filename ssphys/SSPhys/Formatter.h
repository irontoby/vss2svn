// IFormatter.h: interface for the IFormatter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IFORMATTER_H__41F6915F_65BC_4A55_B2C8_1C125237E7CB__INCLUDED_)
#define AFX_IFORMATTER_H__41F6915F_65BC_4A55_B2C8_1C125237E7CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Options.h"
#include <SSPhysLib\SSVersionObject.h>

enum eStyle {eBinary, eXML, eVSS, eDump};

class IFormattingContext : public ISSContext
{
public:
  virtual bool GetExtendedOutput () const = 0;
};

//class CFormatter : public COptions, public ISSObjectVisitor
//{
//public:
//  virtual void Apply(const SSVersionObject& object, const ISSContext* pCtx)       { Apply ((SSObject&) object, pCtx); }
//  virtual void Apply(const SSCheckOutObject& object, const ISSContext* pCtx)      { Apply ((SSObject&) object, pCtx); }
//  virtual void Apply(const SSNameObject& object, const ISSContext* pCtx)          { Apply ((SSObject&) object, pCtx); }
//  virtual void Apply(const SSCommentObject& object, const ISSContext* pCtx)       { Apply ((SSObject&) object, pCtx); }
//  virtual void Apply(const SSProjectObject& object, const ISSContext* pCtx)       { Apply ((SSObject&) object, pCtx); }
//  virtual void Apply(const SSParentFolderObject& object, const ISSContext* pCtx)  { Apply ((SSObject&) object, pCtx); }
//  virtual void Apply(const SSBranchFileObject& object, const ISSContext* pCtx)    { Apply ((SSObject&) object, pCtx); }
//
//  virtual void Apply(const SSFileItem& object, const ISSContext* pCtx)            { Apply ((SSItemInfoObject&) object, pCtx); }
//  virtual void Apply(const SSProjectItem& object, const ISSContext* pCtx)         { Apply ((SSItemInfoObject&) object, pCtx); }
//
//  // The xml formatter needs to output the physical filename
//  virtual void SetFileName (std::string fileName) {};
//
//protected:
//  virtual void Apply(const SSItemInfoObject& object, const ISSContext* pCtx)      { Apply ((SSObject&) object, pCtx); }
//  virtual void Apply(const SSObject& object, const ISSContext* pCtx) = 0;
//};

class CFormatter : public COptions
{
public:
  // The xml formatter needs to output the physical filename
  virtual void SetFileName (std::string fileName) {};

  virtual void Format (const SSObject& object, const ISSContext* pCtx = NULL) = 0;
};

//////////////////////////////////////////////////////////////////////
extern std::auto_ptr<CFormatter> g_pFormatter;

//////////////////////////////////////////////////////////////////////
class CFormatterFactory
{
public:
  static std::auto_ptr<CFormatter>        MakeFormatter (eStyle style, tristate value);
};


#endif // !defined(AFX_IFORMATTER_H__41F6915F_65BC_4A55_B2C8_1C125237E7CB__INCLUDED_)
