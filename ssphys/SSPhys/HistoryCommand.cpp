// HistoryCommand.cpp: implementation of the CHistoryCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "HistoryCommand.h"
#include "VersionFilter.h"
#include "Formatter.h"
#include <SSPhysLib/SSItemInfoObject.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
CHistoryCommand::CHistoryCommand ()
  : CMultiArgCommand ("history", "Shows the history of a VSS phyiscal file"),
    m_bIncludeDeadRecords (false),
    m_bIncludeLabels (false),
    m_bIncludeActions (true)
{
}

po::options_description CHistoryCommand::GetOptionsDescription () const
{
  po::options_description descr (CMultiArgCommand::GetOptionsDescription());
  descr.add_options ()
    ("dead,d",   "display only dead records\n"
                 "append [+|-] to include or exclude dead records in the output\n"
                 "by default dead records are not printed")
    ("labels,l", "display only label records\n"
                 "append [+|-] to include or exclude label records in the output\n"
                 "by default label records are not printed");
  return descr;
}

po::options_description CHistoryCommand::GetHiddenDescription () const
{ 
  po::options_description descr (CMultiArgCommand::GetHiddenDescription());
  descr.add_options ()
    ("show-dead",   po::value<std::string> ()->default_value ("exclude"), "internal option for --dead")
    ("show-labels", po::value<std::string> ()->default_value ("include"), "internal option for --labels");
  return descr;
}


void CHistoryCommand::Execute (po::variables_map const& options, std::string const& arg)
{
  m_bIncludeDeadRecords = false;
  bool m_bIncludeHealthyRecords = true;
  if (options.count("show-dead"))
  {
    if (options["show-dead"].as<std::string> () == "include")
      m_bIncludeDeadRecords = true;
    else if (options["show-dead"].as<std::string> () == "only")
    {
      m_bIncludeDeadRecords = true;
      m_bIncludeHealthyRecords = false;
    }
    else if (options["show-dead"].as<std::string> () == "exclude")
      m_bIncludeDeadRecords = true;
  }
  
  m_bIncludeActions = true;
  m_bIncludeLabels = true;
  if (options.count("show-label"))
  {
    if (options["show-dead"].as<std::string> () == "only")
      m_bIncludeActions = false;
    else if (options["show-dead"].as<std::string> () == "exclude")
      m_bIncludeLabels = false;
  }

  SSHistoryFile file(arg);

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
      GetFormatter()->Format (version);

    version = file.GetPrevVersion(version);
  }
}
