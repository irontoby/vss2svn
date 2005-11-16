// GetCommand.h: interface for the GetCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETCOMMAND_H__26EB05F6_F335_4169_8754_A7BEEF5FB29E__INCLUDED_)
#define AFX_GETCOMMAND_H__26EB05F6_F335_4169_8754_A7BEEF5FB29E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"

//---------------------------------------------------------------------------
class CGetCommand : public CCommand
{
public:
  CGetCommand ();

  virtual po::options_description GetOptionsDescription () const;
  virtual po::options_description GetHiddenDescription () const;
  virtual po::positional_options_description GetPositionalOptionsDescription () const;

  virtual void Execute (po::variables_map const & options, std::vector<po::option> const & args);

protected:
  bool m_bForceOverwrite;
  bool m_bBulkGet;
  int m_Version;
};

#endif // !defined(AFX_GETCOMMAND_H__26EB05F6_F335_4169_8754_A7BEEF5FB29E__INCLUDED_)
