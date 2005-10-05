// InfoCommand.cpp: implementation of the CInfoCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InfoCommand.h"
#include "Formatter.h"
#include <SSPhysLib\SSFiles.h>
#include <SSPhysLib\SSObject.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInfoCommand::CInfoCommand()
  : CCommand ("info"),
    m_bOnlyRecords (false),
    m_bAllRecords (false),
    m_bDisplayAtOffset (false),
    m_Offset (0)
{

}

COptionInfoList CInfoCommand::GetOptionsInfo () const
{
  COptionInfoList options = CCommand::GetOptionsInfo();
  options.push_back (COptionInfo ('a', 'a', "all", "display information about all records", COptionInfo::noArgument));
  options.push_back (COptionInfo ('r', 'r', "records", "display basic information about the records", COptionInfo::noArgument));
  options.push_back (COptionInfo ('o', 'o', "offset", "display record at specific offset", COptionInfo::requiredArgument));
  return options;
}

bool CInfoCommand::SetOption (const COption& option)
{
  switch (option.id)
  {
  case 'a':
    m_bAllRecords = true;
    break;
  case 'r':
    m_bOnlyRecords = true;
    break;
  case 'o':
   m_bDisplayAtOffset = true;
   m_Offset = atoi ((const char*) option.value);
    break;
  default:
    return false;
  }
  return true;
}

bool CInfoCommand::SetArguments (CArguments& args)
{
  if (args.empty ())
    throw SSException ("no argument");
  
  m_PhysFile = args.front ();
  args.pop ();
  return true;
}

void CInfoCommand::Execute ()
{
  std::auto_ptr <SSRecordFile> pFile (SSRecordFile::MakeFile (m_PhysFile));
  g_pFormatter->SetFileName (m_PhysFile);
//  std::auto_ptr<ISSObjectVisitor> pFormatter (CPhysFormatterFactory::MakeFormatter ());
  if (pFile.get ())
  {
    if (m_bAllRecords)
    {
      SSRecordPtr pRecord = pFile->GetFirstRecord();
      while (pRecord)
      {
        if (m_bOnlyRecords)
        {
          pRecord->Dump (std::cout);
        }
        else
        {
          std::auto_ptr<SSObject> pObject (SSObject::MakeObject(pRecord));
          g_pFormatter->Format (*pObject);
          std::cout << std::endl;
        }
        pRecord = pFile->FindNextRecord(pRecord);
      }
    }
    else if (m_bDisplayAtOffset)
    {
      SSRecordPtr pRecord = pFile->GetRecord(m_Offset);
      std::auto_ptr<SSObject> pObject (SSObject::MakeObject(pRecord));
      g_pFormatter->Format (*pObject);
    }
    else
      pFile->Dump (std::cout);
  }
}