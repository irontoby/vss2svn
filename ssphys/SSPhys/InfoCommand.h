// InfoCommand.h: interface for the CInfoCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INFOCOMMAND_H__44ECA882_85A1_410E_B764_E2AF098446C7__INCLUDED_)
#define AFX_INFOCOMMAND_H__44ECA882_85A1_410E_B764_E2AF098446C7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"

//---------------------------------------------------------------------------
class CInfoCommand : public CCommand
{
public:
  CInfoCommand ();

  virtual COptionInfoList GetOptionsInfo () const;
  virtual bool SetOption (const COption& option);
  virtual bool SetArguments (CArguments& args);
  void Execute ();

protected:
  std::string m_PhysFile;
  bool m_bAllRecords;
  bool m_bOnlyRecords;
  bool m_bDisplayAtOffset;
  int m_Offset;
};

#endif // !defined(AFX_INFOCOMMAND_H__44ECA882_85A1_410E_B764_E2AF098446C7__INCLUDED_)
