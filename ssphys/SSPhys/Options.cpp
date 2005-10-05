// Options.cpp: implementation of the COptions class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Options.h"
#include <strstream>

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 




//---------------------------------------------------------------------------
COptionInfo::COptionInfo (int id, char shortOption, std::string longOption, std::string descr, eArgType needArg)
: m_Id (id),
  m_shortOption (shortOption),
  m_longOption (longOption),
  m_Description (descr),
  m_needArg (needArg)
{
}

std::ostream& operator<<(std::ostream& os, const COptionInfo& info)
{
  os << " ";
  if (info.m_shortOption)
  {
    os << " -" << info.m_shortOption;
    switch (info.m_needArg)
    {
    case COptionInfo::requiredArgument:
      os << " ARG";
      break;
    case COptionInfo::optionalArgument:
      os << " [ARG]";
      break;
    case COptionInfo::tristateArgument:
      os << "[+|-]";
      break;
    }

    if (!info.m_longOption.empty ())
      os << ",";
  }
  else
    os << "    ";

  if (!info.m_longOption.empty ())
    os << " --" << info.m_longOption;

  switch (info.m_needArg)
  {
  case COptionInfo::requiredArgument:
    os << " ARG";
    break;
  case COptionInfo::optionalArgument:
    os << " [ARG]";
    break;
  case COptionInfo::tristateArgument:
    os << "[+|-]";
    break;
  }

  os << "\t" << info.m_Description << std::endl;

  return os;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void COptions::PrintUsage () const
{
  COptionInfoList optionsList (GetOptionsInfo ());
  COptionInfoList::const_iterator itor = optionsList.begin ();
  while (itor != optionsList.end ())
  {
    const COptionInfo& info = *itor++;
    std::cout << info;
  }
  std::cout << std::endl;
}