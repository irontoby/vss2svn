// PropertiesCommand.h: interface for the CPropertiesCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROPERTIESCOMMAND_H__0F1C2110_04E9_4DC1_A999_D40711F4DE15__INCLUDED_)
#define AFX_PROPERTIESCOMMAND_H__0F1C2110_04E9_4DC1_A999_D40711F4DE15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"
#include "Formatter.h"

class CPropertiesCommand : public CCommand, public IFormattingContext
{
public:
  CPropertiesCommand ();

  virtual COptionInfoList GetOptionsInfo () const;
  virtual bool SetOption (const COption& option);
  virtual bool SetArguments (CArguments& args);
  void Execute ();

  // IFormattingContext
  virtual bool GetExtendedOutput () const { return true; }

protected:
  std::string m_PhysFile;
};

#endif // !defined(AFX_PROPERTIESCOMMAND_H__0F1C2110_04E9_4DC1_A999_D40711F4DE15__INCLUDED_)
