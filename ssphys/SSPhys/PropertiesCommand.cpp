// PropertiesCommand.cpp: implementation of the CPropertiesCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PropertiesCommand.h"
#include "Formatter.h"
#include <SSPhysLib/SSFiles.h>
#include <SSPhysLib/SSItemInfoObject.h>
#include <SSPhysLib/SSProjectObject.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
CPropertiesCommand::CPropertiesCommand ()
  : CMultiArgCommand ("properties", "Shows basic properties about a VSS physical file")
{
}

void CPropertiesCommand::Execute (po::variables_map const& options, std::string const& arg)
{
  std::auto_ptr<SSRecordFile> file (SSRecordFile::MakeFile(arg));
  if (file.get ())
  {
    SSHistoryFile* pHistory = dynamic_cast<SSHistoryFile*> (file.get());
    SSProjectFile* pProject = dynamic_cast<SSProjectFile*> (file.get());

    if (pHistory)
    {
      std::auto_ptr<SSItemInfoObject> info (pHistory->GetItemInfo ());
      if (info.get())
        GetFormatter()->Format (*info, this);
    }
    else if (pProject)
    {
      SSRecordPtr record (pProject->GetFirstRecord());
      while (record)
      {
        SSProjectObject project (record);
        GetFormatter()->Format (project, this);
        record = pProject->GetNextRecord(record);
      }
        
    }
    else
      throw SSException ("xxx is not a history file, nor a project file");
  }
}
