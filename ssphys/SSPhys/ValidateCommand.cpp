// ValidateCommand.cpp: implementation of the ValidateCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ValidateCommand.h"
#include <SSPhysLib\SSFiles.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
CValidateCommand::CValidateCommand ()
  : CCommand ("validate")
{
}

COptionInfoList CValidateCommand::GetOptionsInfo () const
{
  COptionInfoList options = CCommand::GetOptionsInfo();
  return options;
}

bool CValidateCommand::SetOption (const COption& option)
{
  return true;
}

bool CValidateCommand::SetArguments (CArguments& args)
{
  if (args.empty ())
    throw SSException ("no argument");
  
  m_PhysFile = args.front ();
  args.pop ();    
  return true;
}

void CValidateCommand::Execute ()
{
  try
  {
    std::auto_ptr<SSRecordFile> pFile (SSRecordFile::MakeFile (m_PhysFile));
    if (pFile.get())
    {
      pFile->Validate ();
    }
  }
  catch (SSException& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
  }
}
