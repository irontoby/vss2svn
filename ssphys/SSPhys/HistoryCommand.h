// HistoryCommand.h: interface for the CHistoryCommand class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTORYCOMMAND_H__1B5F69F1_AF4A_40D2_A135_954A4DF610BC__INCLUDED_)
#define AFX_HISTORYCOMMAND_H__1B5F69F1_AF4A_40D2_A135_954A4DF610BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Command.h"

//---------------------------------------------------------------------------
class CHistoryCommand : public CMultiArgCommand
{
public:
  CHistoryCommand ();

  virtual po::options_description GetOptionsDescription () const;
  virtual po::options_description GetHiddenDescription () const;

  virtual void Execute (po::variables_map const& options, std::string const& arg);

protected:
  bool m_bIncludeDeadRecords;
  bool m_bOnlyDeadRecords;
  bool m_bIncludeLabels;
  bool m_bIncludeActions;
};

#endif // !defined(AFX_HISTORYCOMMAND_H__1B5F69F1_AF4A_40D2_A135_954A4DF610BC__INCLUDED_)
