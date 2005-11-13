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
class CInfoCommand : public CMultiArgCommand
{
public:
  CInfoCommand ();

  virtual po::options_description GetOptionsDescription () const;
  virtual void Execute (po::variables_map const& options, std::string const& arg);

protected:
  void Info (SSRecordPtr pRecord, bool bBasicInfo);
};

#endif // !defined(AFX_INFOCOMMAND_H__44ECA882_85A1_410E_B764_E2AF098446C7__INCLUDED_)
