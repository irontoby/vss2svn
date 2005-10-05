// PropertiesCommand.cpp: implementation of the CPropertiesCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PropertiesCommand.h"
#include "Formatter.h"
#include <SSPhysLib\SSFiles.h>
#include <SSPhysLib\SSItemInfoObject.h>
#include <SSPhysLib\SSProjectObject.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
CPropertiesCommand::CPropertiesCommand ()
  : CCommand ("properties")
{
}

COptionInfoList CPropertiesCommand::GetOptionsInfo () const
{
  COptionInfoList options = CCommand::GetOptionsInfo();
  return options;
}

bool CPropertiesCommand::SetOption (const COption& option)
{
  return false;
}

bool CPropertiesCommand::SetArguments (CArguments& args)
{
  if (args.empty ())
    throw SSException ("no argument");
  
  m_PhysFile = args.front ();
  args.pop ();
  return true;
}

void CPropertiesCommand::Execute ()
{
  std::auto_ptr<SSRecordFile> file (SSRecordFile::MakeFile(m_PhysFile));
  if (file.get ())
  {
    SSHistoryFile* pHistory = dynamic_cast<SSHistoryFile*> (file.get());
    SSProjectFile* pProject = dynamic_cast<SSProjectFile*> (file.get());

    if (pHistory)
    {
      std::auto_ptr<SSItemInfoObject> info (pHistory->GetItemInfo ());
      if (info.get())
        g_pFormatter->Format (*info, this);
    }
    else if (pProject)
    {
      SSRecordPtr record (pProject->GetFirstRecord());
      while (record)
      {
        SSProjectObject project (record);
        g_pFormatter->Format (project, this);
        record = pProject->GetNextRecord(record);
      }
        
    }
    else
      throw SSException ("xxx is not a history file, nor a project file");
  }
}