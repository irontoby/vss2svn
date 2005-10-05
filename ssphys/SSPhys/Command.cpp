// Command.cpp: implementation of the CCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Command.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommand::CCommand (std::string commandName)
  : m_CommandName (commandName)
{
}

CCommand::~CCommand ()
{
}

void CCommand::PrintUsage () const
{
  std::cout << GetCommandName() << ":" << std::endl;
  COptions::PrintUsage ();
}



