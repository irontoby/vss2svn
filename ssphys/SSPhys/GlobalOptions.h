// GlobalOptions.h: interface for the CGlobalOptions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBALOPTIONS_H__AD5486F4_24B4_42E6_AB0A_0E11E9284F30__INCLUDED_)
#define AFX_GLOBALOPTIONS_H__AD5486F4_24B4_42E6_AB0A_0E11E9284F30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Options.h"
#include "VersionFilter.h"
#include "Formatter.h"


class CGlobalOptions : public COptions
{
public:
  CGlobalOptions ();
  virtual ~CGlobalOptions ();
  
  virtual COptionInfoList GetOptionsInfo () const;
  virtual bool SetOption (const COption& option);

  bool GetHelp () const                 { return m_bHelp; }
//  std::string GetNamesCache () const    { return m_NamesDat; }
  bool GetAllRecords () const           { return m_bAllRecords; }
  bool GetOnlyRecords () const          { return m_bOnlyRecords; }
  
  eStyle GetStyle () const              { return m_Style; }

  std::string m_Command;
  bool m_bIntegrityCheck;
  bool m_bHelp;
  bool m_bAllRecords;
  bool m_bOnlyRecords;
  std::string m_NamesDat;
  CVersionFilter* m_pVersionFilter;
  int m_Version;
  std::string m_Dest;
  bool m_bForceOverwrite;
  eStyle m_Style;
  tristate m_StyleValue;

};

#endif // !defined(AFX_GLOBALOPTIONS_H__AD5486F4_24B4_42E6_AB0A_0E11E9284F30__INCLUDED_)
