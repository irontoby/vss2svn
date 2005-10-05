// CommandFactory.cpp: implementation of the CCommandFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandFactory.h"

#include "GlobalOptions.h"
#include "HelpCommand.h"
#include "HistoryCommand.h"
#include "InfoCommand.h"
#include "ValidateCommand.h"
#include "GetCommand.h"
#include "FileTypeCommand.h"
#include "PropertiesCommand.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommandFactory::CCommandFactory()
{
}

CCommandFactory::~CCommandFactory()
{
}

CCommand* CCommandFactory::MakeCommand (std::string command)
{
  CCommand* pCommand = NULL;
  if (command == "help")
    pCommand = new CHelpCommand (this);
  else if (command == "info")
    pCommand = new CInfoCommand ();
  else if (command == "history")
    pCommand = new CHistoryCommand ();
  else if (command == "validate")
    pCommand = new CValidateCommand ();
  else if (command == "get")
    pCommand = new CGetCommand ();
  else if (command == "filetype")
    pCommand = new CFileTypeCommand ();
  else if (command == "properties")
    pCommand = new CPropertiesCommand ();
  else
    throw SSException ("unknown command");

  return pCommand;
}

void CCommandFactory::PrintUsage () const
{
  // foreach command
  CHelpCommand helpCommand;
  helpCommand.PrintUsage ();

  CHistoryCommand historyCommand;
  historyCommand.PrintUsage ();

  CInfoCommand infoCommand;
  infoCommand.PrintUsage ();

  CValidateCommand validateCommand;
  validateCommand.PrintUsage ();

  CGetCommand getCommand;
  getCommand.PrintUsage ();
}
