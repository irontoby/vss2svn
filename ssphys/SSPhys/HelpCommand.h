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
class CHelpCommand : public CCommand
{
public:
  CHelpCommand (CCommandFactory* pFactory = NULL)
    : CCommand ("help"), m_pFactory (pFactory)
  {
  }

  virtual bool SetOption (const COption& option);
  virtual bool SetArguments (CArguments& args);
  void Execute ();

protected:
  CCommandFactory* m_pFactory;
  std::auto_ptr<CCommand> m_pCommand;
};


#endif // !defined(AFX_HELPCOMMAND_H__653D5862_B13A_46B2_8AA1_55E8D414AE54__INCLUDED_)
