// InfoCommand.cpp: implementation of the CInfoCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InfoCommand.h"
#include "Formatter.h"
#include <SSPhysLib\SSFiles.h>
#include <SSPhysLib\SSObject.h>
#include <SSPhysLib\SSItemInfoObject.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInfoCommand::CInfoCommand()
  : CMultiArgCommand ("info", "Shows basic information about a VSS phyiscal file")
{
}

po::options_description CInfoCommand::GetOptionsDescription () const
{
//  po::options_description descr ("info options:\nshow information about a physical (record structured) file");
  po::options_description descr (CMultiArgCommand::GetOptionsDescription());
  descr.add_options ()
    ("all,a", "show information about all records")
    ("basic,b", "show only basic information about the records")
    ("offset,o", po::value <int> (), "only display information of the record at a specific offset");
  return descr;
}

void CInfoCommand::Info (SSRecordPtr pRecord, bool bBasicInfo)
{
  if (bBasicInfo)
  {
    std::auto_ptr<SSObject> pObject (new SSObject(pRecord));
    GetFormatter()->Format (*pObject);
  }
  else
  {
    std::auto_ptr<SSObject> pObject (SSObject::MakeObject(pRecord));
    GetFormatter()->Format (*pObject);
  }
}

void CInfoCommand::Execute (po::variables_map const& options, std::string const& arg)
{
  bool bAllRecords = false;
  bool bBasicInfo = false;
  bool bDisplayAtOffset = false;
  int offset = 0;

  if (options.count("all"))
    bAllRecords = true;
  if (options.count("basic"))
    bBasicInfo = true;
  if (options.count("offset"))
  {
    bDisplayAtOffset = true;
    offset = options["offset"].as<int> ();
  }

  std::auto_ptr <SSRecordFile> pFile (SSRecordFile::MakeFile (arg));
  
  if (pFile.get ())
  {
    GetFormatter()->BeginFile (arg);
    if (bAllRecords)
    {
      SSRecordPtr pRecord = pFile->GetFirstRecord();
      while (pRecord)
      {
        Info (pRecord, bBasicInfo);
        pRecord = pFile->FindNextRecord(pRecord);
      }
    }
    else if (bDisplayAtOffset)
    {
      SSRecordPtr pRecord = pFile->GetRecord(offset);
      Info (pRecord, bBasicInfo);
    }
    else
    {
      SSRecordPtr pRecord = pFile->GetFirstRecord();

      // In a names.dat file, the first record "NamesCache" is also some kind of InfoObject
      // so simply report on the first record

      // try, wether the first record is an item info object?
      // std::auto_ptr<SSObject> pObject (SSItemInfoObject::MakeItemInfo(pRecord));

      Info (pRecord, bBasicInfo);
    }

    GetFormatter()->EndFile ();
  }
}