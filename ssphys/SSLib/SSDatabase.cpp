// SSDatabase.cpp: implementation of the SSDatabase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSDatabase.h"
#include "IniFile.h"
#include <SSPhysLib\SSProjectObject.h>
#include <SSPhysLib\crc.h>

#define ASH_NO_REGEX
#include "tokenarray.h"
typedef ash::tokenarray<char> tarray;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;

#include <locale>
#include <iostream>
#include <sstream>
#include <time.h>
using namespace std;

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

//---------------------------------------------------------------------------
SSDatabase* SSDatabase::m_pCurrentDatabase = NULL;
SSDatabase* SSDatabase::GetCurrentDatabase ()
{
  return m_pCurrentDatabase;
}

struct RootProjectFile
{
  RootProjectFile ()
  {
//    memset (this, sizeof (this), 0);
//    memcpy (FileHeader, "SourceSafe@Microsoft", 20);
    
    ProjectEntry.type = SSITEM_PROJECT;
    ProjectEntry.flags = 0;
    ProjectEntry.name.flags = 0;
    strncpy (ProjectEntry.name.name, "$/", 3); // the official name of the root project is $/ and not $
    ProjectEntry.name.nsmap = 0;
    ProjectEntry.type = SSITEM_PROJECT;
    strncpy (ProjectEntry.phys, "AAAAAAAA", 9);
    ProjectEntry.pinnedToVersion = 0;

    RecordHeader.checksum = calc_crc16 (&ProjectEntry, sizeof (PROJECT_ENTRY));
    strncpy (RecordHeader.type, "JP", 2);
    RecordHeader.size = sizeof (PROJECT_ENTRY);
  }
//  char FileHeader[52];
  RECORD_HEADER RecordHeader;
  PROJECT_ENTRY ProjectEntry;
};
static RootProjectFile sRootProjectFile;



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SSDatabase::SSDatabase()
: m_bOpen (false)
{

}

SSDatabase::~SSDatabase()
{

}

bool SSDatabase::Open (const std::string& srcsafeini, const std::string& user, const std::string& password)
{
  if (srcsafeini.empty ())
    return false;

  m_SrcSafeIni = srcsafeini;
  m_User = user;
  m_Password = password;
  m_bOpen = true;

  fs::path basePath = fs::path(m_SrcSafeIni, fs::native);
  if (fs::exists(basePath) && !fs::is_directory(basePath) && basePath.has_leaf())
  {
    basePath = basePath.branch_path ();
    m_SrcSafeIni = basePath.string ();
  }

  std::string database; // = options.GetDatabase ();
//  if (!database.empty())
//  {
//    database = " (" + database + ")";
//  }

  if (!basePath.empty ())
  {
    fs::path srcSafeIni = basePath / "srcsafe.ini";
    fs::path userIni    = basePath / "users" / m_User / "ss.ini";
    fs::path dataPath  = basePath / CIniFile::GetValue("Data_Path" + database, "", srcSafeIni.string());
    fs::path usersPath = basePath / CIniFile::GetValue("Users_Path"+ database, "", srcSafeIni.string());
    fs::path usersText = basePath / CIniFile::GetValue("Users_Text"+ database, "", srcSafeIni.string());
    fs::path tempPath  = basePath / CIniFile::GetValue("Temp_Path" + database, "", srcSafeIni.string());
    fs::path namesPath = basePath / "names.dat";

    m_DataPath= dataPath.string ();
    m_UserIni = userIni.string ();
    m_NamesCache.SetFile (new SSNamesCacheFile (namesPath.string ()));
    m_bOpen = true;
  }
  
  return m_bOpen;
}

bool SSDatabase::IsOpen()
{
  return m_bOpen;
}

std::string SSDatabase::GetSrcSafeIni ()
{
  return m_SrcSafeIni;
}

std::string SSDatabase::GetUsername()
{
  return m_User;
}

std::string SSDatabase::GetDatabaseName()
{
  if (!IsOpen())
    return "";
  
  return CIniFile::GetValue ("Database_Name", "", m_SrcSafeIni);
}

std::string SSDatabase::GetCurrentProject () const
{
  return CIniFile::GetValue ("Project", "", m_UserIni);
}

int SSDatabase::SetCurrentProject (std::string cp)
{
  if (cp.empty() && cp.length() < 2)
    return 0;

  if (cp[0] != '$' && cp[1] != '/')
    throw SSException ("only absolute project pathes are supported");

  return CIniFile::SetValue ("Project", cp, "", m_UserIni) ? 0 : 1;
}

time_t timeFromString (const std::string& str)
{
  using namespace std;
  locale loc ("German");
  basic_stringstream< char > pszGetF, pszPutF, pszGetI, pszPutI;
  ios_base::iostate st = 0;
  struct tm t;
  memset(&t, 0, sizeof(struct tm));

  pszGetF << str;
  pszGetF.imbue( loc );
  basic_istream<char>::_Iter i = _USE (loc, time_get<char>)
    .get_date(basic_istream<char>::_Iter(pszGetF.rdbuf( ) ), basic_istream<char>::_Iter(0), pszGetF, st, &t);

  if ( st & ios_base::failbit )
    cout << "time_get("<< pszGetF.rdbuf( )->str( )<< ") FAILED on char: " << *i << endl;
  else

    cout << "time_get("<< pszGetF.rdbuf( )->str( )<< ") ="
    << "\ntm_sec: " << t.tm_sec
    << "\ntm_min: " << t.tm_min
    << "\ntm_hour: " << t.tm_hour
    << "\ntm_mday: " << t.tm_mday
    << "\ntm_mon: " << t.tm_mon
    << "\ntm_year: " << t.tm_year
    << "\ntm_wday: " << t.tm_wday
    << "\ntm_yday: " << t.tm_yday
    << "\ntm_isdst: " << t.tm_isdst
    << endl;

  return mktime (&t);
}

time_t SSDatabase::GetVersionDate ()
{
  if (!m_VersionStr.empty ())
    return timeFromString(m_VersionStr);
  return (time_t)(LONG_MAX);
}


// Interpretation of the specification
// the specification is a little problematic, since the roots project spec is not '$' but '$/'
// This leads to the following behavoir, (name is any valid item name not starting with $)
// name or $name:  the item in the current project (file item or project item)
//  1.) $/: the root project
//  2.) $  or $. or . : the current project
//  3.) .. or $..: the project relative to the current project
//
// so the interpretation is, that the spec is always relative to the current project, except
// the specification starts with "$/"
// all "$" are removed and the relative path specifiers are flattened

SSItemPtr SSDatabase::GetSSItem (const std::string& spec, bool bDeleted)
{
  if (!IsOpen ())
    throw SSException ("database not open");

  if (spec.empty())
    return SSItemPtr();

  std::string localSpec = spec;
  if (spec.size () < 2 || spec.compare (0, 2, "$/") != 0)
  {
    localSpec = GetCurrentProject ();
    if (localSpec.empty())
      localSpec = "$/";

    if (!spec.empty ())
    {
      std::string tmpSpec (spec);
      if (tmpSpec[0] == '$')
        tmpSpec.erase(0, 1);

      if (!localSpec.empty() && *localSpec.rbegin() != '/' && !tmpSpec.empty () && * tmpSpec.begin () != '/')
        localSpec += '/';
      localSpec += tmpSpec;
    }
  }

  SSItemPtr pItem;

  tarray items (localSpec, "/\\");
  tarray::const_iterator ps;
  for (ps = items.begin (); ps != items.end (); ++ps)
  {
//    std::cout << *ps << std::endl;

    if ((*ps).empty ())
      continue;

    std::string project = (*ps);
    int pos = project.find (';');
    
    std::string name (project);
    std::string version;
    if (pos >= 0) 
    {
      name    = project.substr(0, pos);
      version = project.substr(pos+1);
    }

    if (!pItem)
    {
      if (name != "$")
        throw std::exception ("project must start with the root project $/");

      SSProjectFile projectFile (new CMemoryIO (&sRootProjectFile, sizeof (RootProjectFile)));
      SSRecordPtr recordPtr = projectFile.GetFirstRecord ();
      SSProjectObject project (recordPtr);
      pItem.reset (new SSItem (this, SSItemPtr(), project));
    }
    else if (!name.empty ())
    {
      std::auto_ptr<SSItems> pItems (pItem->GetItems (bDeleted));
      bool bFound = false;
      for (int i = 0; i < pItems->GetCount(); i++)
      {
        SSItemPtr pNewItem = pItems->GetItem(i);

#pragma message ("Was passiert, wenn nach einem Project gefragt wird, welches den selben Namen wie ein File hat?")
        if (pNewItem->GetName () == name && pNewItem->GetDeleted() == bDeleted)
        {
          if (bFound)
            throw SSException ("duplicate entry found");
          pItem = pNewItem;
          bFound = true;
          // continue searching to check for duplicate entries
        }
      }

      if (!bFound)
      {
        // throw SSException ("is not an existing filename or project");
        std::string spec = pItem->GetSpec ();
        if (*spec.rbegin() != '/')
          spec += "/";
        spec += name;

        std::cout << spec << " is not an existing filename or project" << std::endl;
        std::cout << "possible items are:" << std::endl;
        for (int i = 0; i < pItems->GetCount(); i++)
        {
          SSItemPtr pNewItem = pItems->GetItem(i);
          std::cout << pNewItem->GetName () << std::endl;
        }
        return SSItemPtr ();
      }
    }

    // retirieve the version here, since the version specification can stand befind the / like:
    // $;100 or $/;100 
    // $/subproject;100 or // $/subproject/;100
    if (!version.empty ())
      pItem = pItem->GetVersion(version);
  }

  return pItem;
}

SSHistoryFile* SSDatabase::GetDataFile (const std::string& phys)
{
  return new SSHistoryFile (MakePath (phys));
}

std::string SSDatabase::MakePath (std::string phys)
{
  if (phys.empty())
    return m_DataPath;
  
  return m_DataPath + "\\" + phys[0] + "\\" + phys;
}

SSNamesCache *SSDatabase::GetNamesService ()
{
  return &m_NamesCache;
}
