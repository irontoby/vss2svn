// CommandFactory.h: interface for the CCommandFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMANDFACTORY_H__1ACA9B5D_0D2A_42A2_8C4A_A34C1D867A2A__INCLUDED_)
#define AFX_COMMANDFACTORY_H__1ACA9B5D_0D2A_42A2_8C4A_A34C1D867A2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"

class CCommandLine;

/** Class thrown when the command name is not recognized. */
class unknown_command : public SSException {
public:
  unknown_command(const std::string& name)
    : SSException(std::string("unknown command ").append(name)) 
  {}
};

/** Class thrown when no command name was specified. */
class missing_command : public SSException {
public:
  missing_command()
    : SSException(std::string("missing command ")) 
  {}
};

class CCommandFactory  
{
public:
	CCommandFactory();
	virtual ~CCommandFactory();

  CCommand* MakeCommand (std::string command);
  void PrintUsage () const;

protected:
};

#endif // !defined(AFX_COMMANDFACTORY_H__1ACA9B5D_0D2A_42A2_8C4A_A34C1D867A2A__INCLUDED_)
