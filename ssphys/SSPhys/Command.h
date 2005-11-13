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
#include "CommandLine.h"
#include "Formatter.h"

//---------------------------------------------------------------------------
class CCommand
{
protected:
  CCommand (std::string commandName, std::string description);

public:
  virtual ~CCommand ();

  std::string GetCommandName () const           { return m_CommandName; }
  std::string GetCommandDescription () const    { return m_CommandDescription; }
  
  virtual po::options_description GetOptionsDescription () const;
  virtual po::options_description GetHiddenDescription () const;
  virtual po::positional_options_description GetPositionalOptionsDescription () const;
  
  int Execute (std::vector <std::string> const& args);
  virtual void Execute (po::variables_map const & options, std::vector<po::option> const & args) = 0;
  
  void PrintUsage () const;
protected:
  std::string m_CommandName;
  std::string m_CommandDescription;
  po::variables_map m_VariablesMap;
//  po::options_description m_OptionsDescription;
//  po::options_description m_HiddenDescription;

  std::auto_ptr<CFormatter>& GetFormatter ();
private:
  std::auto_ptr<CFormatter> m_pFormatter;
};

//---------------------------------------------------------------------------
class CMultiArgCommand : public CCommand
{
public:
  CMultiArgCommand (std::string commandName, std::string description);

  virtual po::positional_options_description GetPositionalOptionsDescription () const;
  virtual po::options_description GetHiddenDescription () const;

  virtual void Execute (po::variables_map const& options, std::vector<po::option> const& args);
  virtual void Execute (po::variables_map const& options, std::string const& arg) = 0;

protected:
};



#endif // !defined(AFX_COMMAND_H__A42CF41F_0E40_40BA_A36B_CAAEC8A6D34E__INCLUDED_)
