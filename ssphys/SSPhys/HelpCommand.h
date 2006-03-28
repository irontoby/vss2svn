// HelpCommand.h: interface for the CHelpCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPCOMMAND_H__653D5862_B13A_46B2_8AA1_55E8D414AE54__INCLUDED_)
#define AFX_HELPCOMMAND_H__653D5862_B13A_46B2_8AA1_55E8D414AE54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"
#include <memory>

class CCommandFactory;
//---------------------------------------------------------------------------
class CHelpCommand : public CMultiArgCommand
{
public:
  CHelpCommand (CCommandFactory* pFactory = NULL);

  virtual void Execute (po::variables_map const& options, std::string const& arg);

protected:
  CCommandFactory* m_pFactory;
  std::auto_ptr<CCommand> m_pCommand;
};


#endif // !defined(AFX_HELPCOMMAND_H__653D5862_B13A_46B2_8AA1_55E8D414AE54__INCLUDED_)
