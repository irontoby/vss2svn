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
  : CCommand ("filetype")
{
}

COptionInfoList CFileTypeCommand::GetOptionsInfo () const
{
  COptionInfoList options = CCommand::GetOptionsInfo();
  return options;
}

bool CFileTypeCommand::SetOption (const COption& option)
{
  return false;
}

bool CFileTypeCommand::SetArguments (CArguments& args)
{
  if (args.empty ())
    throw SSException ("no argument");
  
  m_PhysFile = args.front ();
  args.pop ();
  return true;
}

void CFileTypeCommand::Execute ()
{
  SSHistoryFile file (m_PhysFile);
  std::auto_ptr<SSItemInfoObject> info (file.GetItemInfo ());
  if (info.get())
    g_pFormatter->Format (*info);
}