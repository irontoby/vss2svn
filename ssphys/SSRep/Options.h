// Options.h: interface for the COptions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_)
#define AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VersionFilter.h"

class COptions
{
public:
  COptions ();
  virtual ~COptions ();

  void PrintUsage ();
  void ParseCommandLine (int argc, char* argv[]);

  StringVector GetItems () const        { return m_Items; }
  std::string GetCommand () const       { return m_Command; }
  bool GetIntegrityCheck () const       { return m_bIntegrityCheck; }
  bool GetIncludeLabels () const        { return m_bIncludeLabels; }
  bool GetOnlyLabels () const           { return m_bOnlyLabels; }
  bool GetDeleted () const              { return m_bIncludeDeleted; }
  bool GetOnlyDeleted () const          { return m_bOnlyDeleted; }
  bool GetFiles () const                { return m_bIncludeFiles; }
  bool GetOnlyFiles () const            { return m_bOnlyFiles; }
  bool GetHelp () const                 { return m_bHelp; }
  bool GetRecursive () const            { return m_bRecursive; }
  std::string GetSrcSafeIniPath () const{ return m_SrcSafeIniPath; }
  std::string GetNamesCache () const    { return m_NamesDat; }
  bool GetAllRecords () const           { return m_bAllRecords; }
  CVersionFilter* GetVersionFilter () const  { return m_pVersionFilter; }
  int GetVersion () const               { return m_Version; }
  std::string GetVersionDate () const   { return m_VersionDate; }
  std::string GetDestination () const   { return m_Dest; }
  bool GetForceOverwrite () const       { return m_bForceOverwrite; }
  bool GetExtendedOutput () const       { return m_bExtended; }
  bool GetPhysOutput () const           { return m_bPhys; }
  
  std::string GetUser () const          { return m_User; }
  std::string GetPassword () const      { return m_Password; }

  std::string GetLogFile () const       { return m_LogFile; }
protected:
  bool IsLongArgument (const char* ch, const char* arg);
  bool IsArgChar (const char ch, const char arg);
  bool ToBool (const char ch, bool& val);
  
  StringVector m_Items;
  std::string m_Command;
  bool m_bIntegrityCheck;
  bool m_bIncludeLabels;
  bool m_bOnlyLabels;
  bool m_bIncludeDeleted;
  bool m_bOnlyDeleted;
  bool m_bIncludeFiles;
  bool m_bOnlyFiles;
  bool m_bHelp;
  bool m_bRecursive;
  bool m_bAllRecords;
  std::string m_NamesDat;
  std::string m_SrcSafeIniPath;
  CVersionFilter* m_pVersionFilter;
  int m_Version;
  std::string m_VersionDate;
  std::string m_Dest;
  bool m_bForceOverwrite;
  std::string m_User;
  std::string m_Password;
  std::string m_LogFile;
  bool m_bExtended;
  bool m_bPhys;
};

#endif // !defined(AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_)
