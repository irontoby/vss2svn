// Command.h: interface for the CCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMAND_H__A42CF41F_0E40_40BA_A36B_CAAEC8A6D34E__INCLUDED_)
#define AFX_COMMAND_H__A42CF41F_0E40_40BA_A36B_CAAEC8A6D34E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Options.h"
#include "Arguments.h"

//---------------------------------------------------------------------------
class CCommand : public COptions
{
public:
  CCommand (std::string commandName);
  virtual ~CCommand ();

  std::string GetCommandName () const
  {
    return m_CommandName;
  }
  
  virtual bool SetArguments (CArguments& args)
  {
    return false;
  }
  
  virtual void Execute () = 0;
  
  virtual void PrintUsage () const;

protected:
  std::string m_CommandName;
};




#endif // !defined(AFX_COMMAND_H__A42CF41F_0E40_40BA_A36B_CAAEC8A6D34E__INCLUDED_)
