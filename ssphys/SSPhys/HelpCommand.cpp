// HelpCommand.cpp: implementation of the CHelpCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HelpCommand.h"
#include "CommandFactory.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern void PrintUsage ();

bool CHelpCommand::SetArguments (CArguments& args)
{
  if (m_pFactory && !args.empty ())
  {
    m_pCommand = std::auto_ptr<CCommand> (m_pFactory->MakeCommand (args.front ()));
    args.pop ();
    return true;
  }
  return false;
}

void CHelpCommand::Execute ()
{
  if (m_pCommand.get ())
  {
    m_pCommand->PrintUsage ();
  }
  else if (m_pFactory)
  {
    ::PrintUsage ();
  }
}

bool CHelpCommand::SetOption (const COption& option)
{
  return false;
}
