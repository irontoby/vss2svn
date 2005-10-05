// ValidateCommand.h: interface for the ValidateCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VALIDATECOMMAND_H__9B7719D1_E912_4A98_BD7A_A4581CBCEE9B__INCLUDED_)
#define AFX_VALIDATECOMMAND_H__9B7719D1_E912_4A98_BD7A_A4581CBCEE9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"

//---------------------------------------------------------------------------
class CValidateCommand : public CCommand
{
public:
  CValidateCommand ();

  virtual COptionInfoList GetOptionsInfo () const;
  virtual bool SetOption (const COption& option);
  virtual bool SetArguments (CArguments& args);
  void Execute ();

protected:
  std::string m_PhysFile;
};


#endif // !defined(AFX_VALIDATECOMMAND_H__9B7719D1_E912_4A98_BD7A_A4581CBCEE9B__INCLUDED_)
