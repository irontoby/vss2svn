// SSDatabase.h: interface for the SSDatabase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSDATABASE_H__5C786BA0_F380_4FC0_BEC0_6D97821C43D8__INCLUDED_)
#define AFX_SSDATABASE_H__5C786BA0_F380_4FC0_BEC0_6D97821C43D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSItem.h"
#include <SSPhysLib\SSNameObject.h>

class SSDatabase  
{
public:
	SSDatabase ();
	virtual ~SSDatabase();

  static SSDatabase* GetCurrentDatabase ();

  bool Open (const std::string& srcsafeini, const std::string& user, const std::string& password);
  bool IsOpen ();
  
  std::string GetSrcSafeIni ();
  std::string GetUsername ();
  std::string GetDatabaseName ();

  time_t GetVersionDate ();
  void SetVersionDate (std::string versionDate) { m_VersionStr = versionDate; }

  SSItemPtr GetSSItem (const std::string& spec, bool bDeleted);

  SSHistoryFile* GetDataFile (const std::string& phys);
  SSNamesCache* GetNamesService ();

  std::string GetCurrentProject () const;
  int SetCurrentProject (std::string cp);

private:
  std::string MakePath (std::string phys);

  static SSDatabase* m_pCurrentDatabase;

  std::string m_SrcSafeIni;
  std::string m_UserIni;
  std::string m_User;
  std::string m_Password;

  std::string m_VersionStr;
  
  std::string m_DataPath;
  bool m_bOpen;

  SSNamesCache m_NamesCache;
};

#endif // !defined(AFX_SSDATABASE_H__5C786BA0_F380_4FC0_BEC0_6D97821C43D8__INCLUDED_)
