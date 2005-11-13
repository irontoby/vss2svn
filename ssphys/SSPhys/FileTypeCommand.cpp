// FileTypeCommand.cpp: implementation of the CFileTypeCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileTypeCommand.h"
#include "Formatter.h"
#include <SSPhysLib\SSFiles.h>
#include <SSPhysLib\SSItemInfoObject.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
CFileTypeCommand::CFileTypeCommand ()
  : CMultiArgCommand ("filetype", "Displays the type (text or binary) of a VSS physical file")
{
}

void CFileTypeCommand::Execute (const po::variables_map& vm, std::string const& arg)
{
  SSHistoryFile file (arg);
  std::auto_ptr<SSItemInfoObject> info (file.GetItemInfo ());
  if (info.get())
    GetFormatter()->Format (*info);
}