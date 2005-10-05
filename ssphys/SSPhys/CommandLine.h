// CommandLine.h: interface for the CCommandLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMANDLINE_H__094560AC_AAF6_4054_8536_4D6F421ACBCE__INCLUDED_)
#define AFX_COMMANDLINE_H__094560AC_AAF6_4054_8536_4D6F421ACBCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Arguments.h"
#include "Options.h"
#include "CommandFactory.h"
#include <list>

//---------------------------------------------------------------------------
class CCommandLine
{
public:
  CCommandLine ();
  virtual ~CCommandLine ();

  void Parse (int argc, char* argv[]);

  void SetCommandFactory (CCommandFactory* pFactory)  { m_pCommandFactory = pFactory; }
  void SetOptionsInfo (COptionInfoList optionsInfo)   { m_OptionsInfo = optionsInfo; } 
  
  COptionsList GetOptions ()                          { return m_Options; } 
  CArguments GetArgs ()                               { return m_Args; } 
  std::string GetCommand ()                           { return m_Command; }
  
protected:
  bool IsLongArgument (const char* ch, const char* arg);
  bool IsArgChar (const char ch, const char arg);
  
  CCommandFactory *m_pCommandFactory;
  CArguments m_Args;
  COptionsList m_Options;
  COptionInfoList m_OptionsInfo;
  std::string m_Command;
};

#endif // !defined(AFX_COMMANDLINE_H__094560AC_AAF6_4054_8536_4D6F421ACBCE__INCLUDED_)
