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
class CValidateCommand : public CMultiArgCommand
{
public:
  CValidateCommand ();

  virtual void Execute (po::variables_map const& options, std::string const& arg);

protected:
};


#endif // !defined(AFX_VALIDATECOMMAND_H__9B7719D1_E912_4A98_BD7A_A4581CBCEE9B__INCLUDED_)
