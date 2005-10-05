// GlobalOptions.cpp: implementation of the CGlobalOptions class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GlobalOptions.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGlobalOptions::CGlobalOptions ()
  : m_bIntegrityCheck (false),
//    m_bIncludeDeadRecords (false),
//    m_bOnlyLabels (false),
//    m_bIncludeLabels (true),
    m_bHelp (false),
    m_bAllRecords (false),
    m_bOnlyRecords (false),
    m_pVersionFilter (NULL),
    m_Version (0),
    m_bForceOverwrite (false),
    m_Style (eVSS)
{
}

CGlobalOptions::~CGlobalOptions ()
{
  DELETE (m_pVersionFilter);
}

COptionInfoList CGlobalOptions::GetOptionsInfo () const
{
  COptionInfoList options = COptions::GetOptionsInfo();
  options.push_back (COptionInfo ('h', 'h', "help", "print help information", COptionInfo::noArgument));
  options.push_back (COptionInfo ('n', 'n', "names", "filename of the the names.dat file to use", COptionInfo::requiredArgument));
  options.push_back (COptionInfo ('b', 'b', "binary", "binary dump the output", COptionInfo::tristateArgument));
  options.push_back (COptionInfo ('s', 's', "style", "define the output style {binary|xml|vss|dump}", COptionInfo::requiredArgument));
  return options;
//      if (IsArgChar (argv[i][1], 'l'))
//      {
//        m_bOnlyLabels = true;
//        m_bIncludeLabels = ToBool (argv[i][2]);
//      }
//      else if (IsLongArgument (&argv[i][1], "del"))
//      {
//        m_bIncludeDeadRecords = true;
//      }
//      else 
//      if (IsArgChar (argv[i][1], 'h') || IsLongArgument (&argv[i][1], "help"))
//      {
//        m_bHelp = true;
//      }
//      else if (IsArgChar (argv[i][1], 'n') || IsLongArgument (&argv[i][1], "names"))
//      {
//        if (++i < argc)
//          m_NamesDat = argv[i];
//      }
//      else if (IsArgChar (argv[i][1], 'a') || IsLongArgument (&argv[i][1], "all"))
//      {
//        m_bAllRecords = true;
//      }
//      else if (IsArgChar (argv[i][1], 'r'))
//      {
//        m_bOnlyRecords = true;
//      }
//      else if (IsArgChar (argv[i][1], 'f') || IsLongArgument (&argv[i][1], "force"))
//      {
//        m_bForceOverwrite = true;
//      }
//      else if (IsArgChar (argv[i][1], 'v') || IsLongArgument (&argv[i][1], "version"))
//      {
//        std::string filter; 
//        if (IsArgChar (argv[i][1], 'v'))
//          filter = &argv[i][2];
//        else if (++i < argc)
//         filter = argv[i];
//
//        int start;
//        char tilde;
//        std::istrstream ist (filter.c_str (), filter.length());
//        ist >> start;
//        int end = start;
//        ist >> tilde;
//        ist >> end;
//        
//        DELETE (m_pVersionFilter);
//        m_pVersionFilter = new CVersionRangeFilter (end, start);
//
//        m_Version = start;
//      }
}

bool CGlobalOptions::SetOption (const COption& option)
{
  switch (option.id)
  {
  case 'h':
    m_bHelp = true; break;
  case 'n':
    m_NamesDat = (const char*)option.value; break;
  case 'b':
    m_Style = eBinary; 
    m_StyleValue = option.value;
    break;
  case 's':
    if (stricmp ("binary", option.value) == 0)
      m_Style = eBinary; 
    else if (stricmp ("XML", option.value) == 0)
      m_Style = eXML; 
    else if (stricmp ("vss", option.value) == 0)
      m_Style = eVSS; 
    else if (stricmp ("dump", option.value) == 0)
      m_Style = eDump; 
    else 
      throw SSException ("unkown style");
    break;
  default:
    return false;
  }
  return true;
}
