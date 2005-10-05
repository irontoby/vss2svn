// CommandLine.cpp: implementation of the CCommandLine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandLine.h"
#include "Command.h"

//---------------------------------------------------------------------------
tristate ToTristate (const char ch)
{
  if (!ch)
    return undefined;
  if (ch == '-')
    return cleared;
  else if (ch == '+')
    return set;
  
  return undefined;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommandLine::CCommandLine ()
{
}

CCommandLine::~CCommandLine ()
{
}


void CCommandLine::Parse (int argc, char* argv[])
{
  for (int i = 1; i< argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      COptionInfoList::const_iterator itor = m_OptionsInfo.begin ();
      COption option;
      while (itor != m_OptionsInfo.end ())
      {
        const COptionInfo& info = *itor;
        if (IsArgChar (argv[i][1], info.m_shortOption) || IsLongArgument (&argv[i][1], info.m_longOption.c_str()))
        {
          option.id = info.m_Id;
          switch (info.m_needArg)
          {
          case COptionInfo::requiredArgument:
            if (++i < argc)
              option.value = argv[i];
            else
              throw SSException ("missing command line parameter");
            break;
          case COptionInfo::optionalArgument:
            if (++i < argc)
              option.value = argv[i];
            break;
          case COptionInfo::tristateArgument:
            option.value = ToTristate (argv[i][2]);
            break;
          }

          break;
        }
        ++itor;
      }

      if (itor == m_OptionsInfo.end ())
        throw SSException ("unknown command line parameter");

      m_Options.push_back (option);
    }
    else
    {
      if (m_Command.empty ())
      {
        m_Command = argv[i];
        std::auto_ptr <CCommand> pCommand (m_pCommandFactory->MakeCommand (m_Command));
        COptionInfoList commandOptions = pCommand->GetOptionsInfo ();
        m_OptionsInfo.insert (m_OptionsInfo.end (), commandOptions.begin (), commandOptions.end ());
      }
      else
      {
        m_Args.push (argv[i]);
      }
    }
  }
}


bool CCommandLine::IsLongArgument (const char* ch, const char* arg)
{
  if (!ch)
    return false;
  
  std::string str = (ch[0] == '-') ? &ch[1] : &ch[0];
  if (str == arg)
    return true;
  
  return false;
}

bool CCommandLine::IsArgChar (const char ch, const char arg)
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
