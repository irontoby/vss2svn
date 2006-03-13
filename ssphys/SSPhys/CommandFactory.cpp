// CommandFactory.cpp: implementation of the CCommandFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
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
  for (size_t i = 0; i < command.size(); ++i)
    command[i] = char(tolower(command[i]));

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
    throw unknown_command (command);

  return pCommand;
}

void CCommandFactory::PrintUsage () const
{
  std::cout << "available commands: " << std::endl;

  CHelpCommand help;
  std::cout << help.GetCommandName () << ":\t\t" << help.GetCommandDescription () << std::endl;

  CInfoCommand info;
  std::cout << info.GetCommandName () << ":\t\t" << info.GetCommandDescription () << std::endl;
  
  CHistoryCommand history;
  std::cout << history.GetCommandName () << ":\t" << history.GetCommandDescription () << std::endl;
  
  CValidateCommand validate;
  std::cout << validate.GetCommandName () << ":\t" << validate.GetCommandDescription () << std::endl;
  
  CGetCommand get;
  std::cout << get.GetCommandName () << ":\t\t" << get.GetCommandDescription () << std::endl;

  CFileTypeCommand filetype;
  std::cout << filetype.GetCommandName () << ":\t" << filetype.GetCommandDescription () << std::endl;

  CPropertiesCommand properties;
  std::cout << properties.GetCommandName () << ":\t" << properties.GetCommandDescription () << std::endl;

}

