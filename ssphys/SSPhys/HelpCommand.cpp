// HelpCommand.cpp: implementation of the CHelpCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "HelpCommand.h"
#include "CommandFactory.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern void PrintUsage ();

CHelpCommand::CHelpCommand (CCommandFactory* pFactory /*= NULL*/)
  : CMultiArgCommand ("help", "Displays help on using a specific command"), 
    m_pFactory (pFactory)
{

}

void CHelpCommand::Execute (po::variables_map const& options, std::string const& arg)
{
  std::auto_ptr<CCommand> pCommand(m_pFactory->MakeCommand (arg));
  pCommand->PrintUsage ();
}

