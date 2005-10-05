// Formatter.h: interface for the CFormatter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMATTER_H__A5506F2B_A8AC_4232_A73E_3892242D51E3__INCLUDED_)
#define AFX_FORMATTER_H__A5506F2B_A8AC_4232_A73E_3892242D51E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Options.h"
#include <SSLib\SSItem.h>
#include <SSLib\SSDatabase.h>

//////////////////////////////////////////////////////////////////////
class IFormatter
{
public:
  virtual int DoWhoAmI     (const COptions& options) = 0;
  virtual int DoProject    (const SSDatabase& database, const COptions& options) = 0;

  virtual int DoDirectory  (const SSItemPtr pItem, const COptions& options) = 0;
  virtual int DoProperties (const SSItemPtr pItem, const COptions& options) = 0;
  virtual int DoFiletype   (const SSItemPtr pItem, const COptions& options) = 0;
  virtual int DoHistory    (const SSItemPtr pItem, const COptions& options) = 0;
};




//////////////////////////////////////////////////////////////////////
class CVssFormatterFactory
{
public:
  static IFormatter*        MakeFormatter ();
};

#endif // !defined(AFX_FORMATTER_H__A5506F2B_A8AC_4232_A73E_3892242D51E3__INCLUDED_)
