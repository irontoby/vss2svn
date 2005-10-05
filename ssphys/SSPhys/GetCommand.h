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

  virtual COptionInfoList GetOptionsInfo () const;
  virtual bool SetOption (const COption& option);
  virtual bool SetArguments (CArguments& args);
  void Execute ();

protected:
  std::string m_PhysFile;
  std::string m_DestFile;
  bool m_bForceOverwrite;
  bool m_bBulkGet;
  int m_Version;
};

#endif // !defined(AFX_GETCOMMAND_H__26EB05F6_F335_4169_8754_A7BEEF5FB29E__INCLUDED_)
