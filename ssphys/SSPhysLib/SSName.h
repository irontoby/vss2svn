// SSName.h: interface for the SSName class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSNAME_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
#define AFX_SSNAME_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSTypes.h"
#include "SSNameObject.h"

class SSName  
{
public:
	SSName(SSNAME ssname, SSNamesCache* pNamesService = NULL);
	virtual ~SSName();

  std::string GetFullName () const;
  long GetOffset () const;
  std::string GetType () const;

  void ToXml (XMLNode* pParent, std::string name = "SSName");
  operator const SSNAME ()
  {
    return m_ssName;
  }

private:
  SSNAME m_ssName;
  SSNamesCache* m_pNamesService;
};

std::ostream& operator<<(std::ostream& os, const SSName& ssname);

#endif // !defined(AFX_SSNAME_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
