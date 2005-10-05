// HistoryCommand.cpp: implementation of the CHistoryCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HistoryCommand.h"
#include "VersionFilter.h"
#include "Formatter.h"
#include <SSPhysLib\SSItemInfoObject.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
CHistoryCommand::CHistoryCommand ()
  : CCommand ("history"),
    m_bIncludeDeadRecords (false),
    m_bIncludeLabels (false),
    m_bIncludeActions (true)
{
}

COptionInfoList CHistoryCommand::GetOptionsInfo () const
{
  COptionInfoList options = CCommand::GetOptionsInfo();
  options.push_back (COptionInfo ('d', 'd', "dead", "include dead records", COptionInfo::tristateArgument));
  options.push_back (COptionInfo ('l', 'l', "label", "include label records", COptionInfo::tristateArgument));
  return options;
}

bool CHistoryCommand::SetOption (const COption& option)
{
  switch (option.id)
  {
  case 'd':
    m_bIncludeDeadRecords = true;
    m_bOnlyDeadRecords &= option.value.pTristateValue;
    break;
  case 'l':
    m_bIncludeLabels = true;
    if (option.value.pTristateValue == set)
      m_bIncludeActions = false;
    else if (option.value.pTristateValue == cleared)
      m_bIncludeLabels = false;
    break;
  default:
    return false;
  }
  return true;
}

bool CHistoryCommand::SetArguments (CArguments& args)
{
  if (args.empty ())
    throw SSException ("no argument");
  
  m_PhysFile = args.front ();
  args.pop ();    
  return true;
}

void CHistoryCommand::Execute ()
{
  SSHistoryFile file(m_PhysFile);

  const CVersionFilter* pFilter = NULL; // m_pOptions->GetVersionFilter ();
//  std::auto_ptr <ISSObjectVisitor> pFormatter (CVssFormatterFactory::MakeFormatter());

  SSVersionObject version = file.GetLastVersion();
  while (version)
  {
    bool bFiltered = pFilter ? pFilter->Filter(&version) : false;

    if (!m_bIncludeLabels && version.GetActionID () == Labeled)
      bFiltered = true;
    
    if (!m_bIncludeActions && version.GetActionID () != Labeled)
      bFiltered = true;

    if (!bFiltered)
      g_pFormatter->Format (version);

    version = file.GetPrevVersion(version);
  }
}
