// Options.cpp: implementation of the COptions class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Options.h"
#include "strstream"

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// TODO: replace the complete COptions class with a better design

COptions::COptions ()
  : m_bIntegrityCheck (false),
    m_bIncludeLabels (true),
    m_bOnlyLabels (false),
    m_bIncludeDeleted (false),
    m_bOnlyDeleted (false),
    m_bIncludeFiles (true),
    m_bOnlyFiles (false),
    m_bHelp (false),
    m_bAllRecords (false),
    m_pVersionFilter (NULL),
    m_Version (0),
    m_bForceOverwrite (false),
    m_bRecursive (false),
    m_bExtended (false),
    m_bPhys (false)
{

}

COptions::~COptions ()
{
  DELETE (m_pVersionFilter);
}

void COptions::PrintUsage ()
{
  std::cout << "ssrep v0.8:" << std::endl;
  std::cout << std::endl;
  std::cout << "usage:" << std::endl;
  std::cout << "general options: "  << std::endl;
  std::cout << " -Y           user,password"  << std::endl;
  std::cout << " -d           include deleted files" << std::endl;
  std::cout << " -d+            only deleted files" << std::endl;
  std::cout << " -f           include files (default)" << std::endl;
  std::cout << " -f+            only files" << std::endl;
  std::cout << " -f-            only projects" << std::endl;
  std::cout << " -v#          specific version number"  << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "ssrep dir [file]      displays directory information about the project"  << std::endl;
//  std::cout << "ssrep info [file]      displays basic information about the item"  << std::endl;
//  std::cout << "options: "  << std::endl;
//  std::cout << " -a           process all records within the file"  << std::endl;
//  std::cout << "ssrep history [item]   displays the history of the specific item"  << std::endl;
//  std::cout << "options: "  << std::endl;
//  std::cout << " -L<-|+>      Filter versions by labels (-) all versions without labels"  << std::endl;
//  std::cout << "                                        (+) only versions that have labels"  << std::endl;
//  std::cout << " --del        include dead records"  << std::endl;
//  std::cout << std::endl;
//  std::cout << "ssrep get [item] [target]  retrieve a specific version of the item"  << std::endl;
//  std::cout << "options: "  << std::endl;
//  std::cout << " -f           force overwrite of target file" << std::endl;
//  std::cout << " -v#          specifies the version to get"  << std::endl;
  std::cout << std::endl;
  std::cout << " -h --help    print help text" << std::endl;
}

void COptions::ParseCommandLine (int argc, char* argv[])
{
  for (int i = 1; i< argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      if (IsArgChar (argv[i][1], '?'))
      {
        m_bHelp = true;
      }
      else if (IsArgChar (argv[i][1], 'a') || IsLongArgument (&argv[i][1], "all"))
      {
        m_bAllRecords = true;
      }
      else if (IsArgChar (argv[i][1], 'd'))
      {
        m_bIncludeDeleted = true;
        m_bOnlyDeleted = ToBool (argv[i][2], m_bOnlyDeleted);
      }
      else if (IsArgChar (argv[i][1], 'e'))
      {
        m_bExtended = true;
      }
      else if (IsArgChar (argv[i][1], 'f'))
      {
        m_bIncludeFiles = true;
        m_bOnlyFiles = ToBool (argv[i][2], m_bIncludeFiles );
      }
      else if (IsLongArgument (&argv[i][1], "force"))
      {
        m_bForceOverwrite = true;
      }
      else if (IsArgChar (argv[i][1], 'h') || IsLongArgument (&argv[i][1], "help"))
      {
        m_bHelp = true;
      }
      else if (IsArgChar (argv[i][1], 'i'))
      {
// Ignore: Do not ask for input under any circumstances.
//        m_bIgnore = true;
//        m_bIgnore = ToBool (argv[i][2], m_bIgnore);
      }
      else if (IsArgChar (argv[i][1], 'l'))
      {
        m_bOnlyLabels = true;
        m_bIncludeLabels = ToBool (argv[i][2], m_bOnlyLabels);
      }
      else if (IsArgChar (argv[i][1], 'n') || IsLongArgument (&argv[i][1], "names"))
      {
        if (++i < argc)
          m_NamesDat = argv[i];
      }
      else if (IsArgChar (argv[i][1], 'o'))
      {
        m_LogFile = &argv[i][2];
      }
      else if (IsArgChar (argv[i][1], 'p'))
      {
        m_bPhys = true;
        m_bPhys = ToBool (argv[i][2], m_bPhys);
      }
      
      else if (IsArgChar (argv[i][1], 'r') || IsLongArgument (&argv[i][1], "recursive"))
      {
        m_bRecursive = true;
        m_bRecursive = ToBool (argv[i][2], m_bRecursive);
      }
      else if (IsArgChar (argv[i][1], 's'))
      {
        m_SrcSafeIniPath = &argv[i][2];
      }
      else if (IsArgChar (argv[i][1], 'v') || IsLongArgument (&argv[i][1], "version"))
      {
        std::string filter; 
        if (IsArgChar (argv[i][1], 'v'))
          filter = &argv[i][2];
        else if (++i < argc)
         filter = argv[i];

        int start;
        char tilde;
        std::istrstream ist (filter.c_str (), filter.length());
        ist >> start;
        int end = start;
        ist >> tilde;
        ist >> end;
        
        DELETE (m_pVersionFilter);
        m_pVersionFilter = new CVersionRangeFilter (end, start);

        m_Version = start;
        m_VersionDate = filter;
      }
      else if (IsArgChar (argv[i][1], 'y'))
      {
        std::string userpw = &argv[i][2];
        m_User = userpw.substr(0, userpw.find (','));
        m_Password = userpw.substr(userpw.find (',')+1);
      }
    }
    else
    {
      if (m_Command.empty ())
      {
        m_Command = argv[i];
        // TODO: tolower bekommt ein (int) als parameter, std::string ist aber ein uchar --> Probleme mit negativen chars
        std::transform (m_Command.begin(), m_Command.end(), m_Command.begin(), tolower);
      }
      else
        m_Items.push_back (argv[i]);
    }
  }

  if (m_Command == "get" && m_Items.size () > 1)
  {
    m_Dest = m_Items.back();
    m_Items.pop_back();
  }
}


bool COptions::IsLongArgument (const char* ch, const char* arg)
{
  if (!ch)
    return false;
  
  std::string str = (ch[0] == '-') ? &ch[1] : &ch[0];
  if (str == arg)
    return true;
  
  return false;
}

bool COptions::IsArgChar (const char ch, const char arg)
{
  if (!ch)
    return false;
  else if (ch == arg)
    return true;
  else if (tolower(ch) == arg)
    return true;
  else if (toupper(ch) == arg)
    return true;
  return false;
}

bool COptions::ToBool (const char ch, bool& val)
{
  if (!ch)
    return val;
  if (ch == '-')
    val = false;
  else if (ch == '+')
    val = true;
  
  return val;
}
