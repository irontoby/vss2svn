// SSTests.cpp: implementation of the SSTests class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SSTests.h"
#include <SSPhysLib/SSVersionObject.h>
#include <SSPhysLib/SSItemInfoObject.h>
#include <SSPhysLib/SSNameObject.h>

//#include <boost/filesystem/path.hpp>
//namespace fs = boost::filesystem;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void TestComments (SSRecordFile& file)
{
  SSRecordPtr pRecord;
  int nCommentRecords = 0;
  int nComments = 0;
  for (pRecord = file.GetFirstRecord (); pRecord; )
  {
    if (pRecord->GetType () == eCommentRecord)
    {
      nCommentRecords++;
    }
    else if (pRecord->GetType () == eHistoryRecord)
    {
//      std::auto_ptr <SSVersionObject> pVersion (SSVersionObject::MakeVersion(pRecord));
//      SSLabeledAction* pAction = dynamic_cast <SSLabeledAction*> (pVersion.get());
      std::auto_ptr <SSVersionObject> pVersion (new SSVersionObject (file.GetFileImp(), pRecord));
      SSLabeledAction* pAction = dynamic_cast <SSLabeledAction*> (pVersion.get());
      
      // es gibt bishet anscheinend immer nur einen Comment oder einen LabelComment,
      // bzw. Comment und LabelComment sind identisch
      if (!pVersion->GetComment ().empty() || (pAction && pAction->GetLabelComment ().empty()))
      {
        nComments++;
      }
    }
    pRecord = file.GetNextRecord (pRecord);
  }

  assert (nComments == nCommentRecords);
}

void CheckOffsetsToComment (SSFile& file)
{
#if 0
  SSRecordPtr pRecord;
  for (pRecord = file.GetFirstRecord (); pRecord; )
  {
    if (pRecord->GetType () == eHistoryRecord)
    {
      std::auto_ptr<SSVersionObject> pVersion (SSVersionObject::MakeVersion(pRecord));
      const VERSION_RECORD* pV = reinterpret_cast<const VERSION_RECORD*> (pVersion->GetDataPtr ());
      
//      std::cout << std::hex << pV->offsetToLabelComment <<std::endl;
      // offsetToLabelComment zeigt immer auf einen MC record
      if (pV->offsetToLabelComment)
      {
        SSRecordPtr pNext = file.GetRecord (pV->offsetToLabelComment);
//        assert (pNext->GetType () == eCommentRecord);
      }

      // im Labeled Fall zeigt offsetToNextRecordOrComment immer auf einen MC record
      if (pVersion->GetActionId () == Labeled && pV->offsetToNextRecordOrComment)
      {
        SSRecordPtr pNext = file.GetRecord (pV->offsetToNextRecordOrComment);
//        assert (pNext->GetType () == eCommentRecord);
      }

      // offsetToNextRecordOrComment zeigt immer auf den nächsten in der List
      if (pV->offsetToNextRecordOrComment)
      {
//        assert (pV->offsetToNextRecordOrComment == pRecord->GetNextOffset ());
      }

    }
    pRecord = file.GetNextRecord (pRecord);
  }
#endif
}

void CheckLabelValid (SSFile& file)
{
#if 0
  SSRecordPtr pRecord;
  for (pRecord = file.GetFirstRecord (); pRecord; )
  {
    if (pRecord->GetType () == eHistoryRecord)
    {
      std::auto_ptr <SSVersionObject> pVersion (SSVersionObject::MakeVersion(pRecord));
      const VERSION_RECORD* pV = reinterpret_cast<const VERSION_RECORD*> (pVersion->GetDataPtr ());
      SSLabeledAction* pAction = dynamic_cast <SSLabeledAction*> (pVersion.get());

      if (pAction)
      {
        if (pV->offsetToLabelComment)
        {
          SSRecordPtr pComment1 = file.GetRecord (pV->offsetToLabelComment);
          assert (pV->lengthLabelComment == pComment1->GetLen ());
        }
        // BUG: Nach dem Löschen eines Labels wird eigentlich das erste char des Label auf 0 gesetzt und der
        //      Label comment gelöscht. (neuer MC eintrag mit Länge 1) der Comment wird nicht angefaßt. Anscheinend
        //      kann es aber passieren, daß die Länge des Comment auf 0 gesetzt wird, obwohl der Eintrag auf einen
        //      leeren 1 Byte langen Comment verweist.
        //      Ich bin mir noch nicht sicher, ob das ein Bug oder ein Feature ist
        if (pV->lengthComment != 0 &&
            pV->offsetToNextRecordOrComment && pV->offsetToNextRecordOrComment != pV->offsetToLabelComment )
        {
          SSRecordPtr pComment2 = file.GetRecord (pV->offsetToNextRecordOrComment);
          assert (pV->lengthComment == pComment2->GetLen ());
        }
      }
      
    }
    pRecord = file.GetNextRecord (pRecord);
  }
#endif
}

void CheckFileSize (SSHistoryFile& file)
{
  std::auto_ptr<SSItemInfoObject> pItem (file.GetItemInfo());
  assert (pItem.get () && "no item found in this file");
}

void IntegrityCheck (SSHistoryFile& file)
{
  // 1.) run the linear list of records, one record is succeeded by another record
  try {
    SSRecordPtr pRecord = file.GetFirstRecord ();
    while (pRecord)
    {
      pRecord = file.GetNextRecord (pRecord);
    }
  } 
  catch (SSException& ex)
  {
    std::cout << "Warning while testing linear: " << ex.what () << std::endl;
  }

  // 2.) run the linked list of records
  try {
    std::auto_ptr<SSItemInfoObject> pItem (file.GetItemInfo());
    if (pItem.get ())
    {
      for (int i = pItem->GetNumberOfActions (); i> 0; i--)
      {
        std::auto_ptr<SSVersionObject> pVersion (pItem->GetVersion (i));
        std::string pComment = pVersion->GetComment();
      }
    }
  }
  catch (SSException& ex) {
    std::cout << "Warning while testing linked history list: " << ex.what () << std::endl;
  }

  // 3.) run the parent project linked list
//  try {
//    SSItemInfo* pItem = file.GetItem ();
//    if (pItem)
//    {
//      for (int i = pItem->GetNumberOfParents (); i> 0; i--)
//      {
//        SSVersionObject* pVersion = pItem->GetParent (i);
//      }
//    }
//  }
//  catch (SSException& ex) {
//    std::cout << "Warning while testing linked parent project list: " << ex.what () << std::endl;
//  }
}


void DumpNamesCache (SSNamesCacheFile& namesCache)
{
  SSRecordPtr pRecord;
  for (pRecord = namesCache.GetFirstRecord (); pRecord; )
  {
    if (pRecord->GetType () == eNameCacheEntry)
    {
      SSNameObject name (namesCache.GetFileImp(), pRecord);
      name.Dump (std::cout);
    }
    pRecord = namesCache.GetNextRecord (pRecord);
  }
}


//void TestSrcSafeIni (const COptions& options)
//{
//  fs::path basePath = fs::path(options.GetSrcSafeIniPath (), fs::native);
//
//  std::string database = options.GetDatabase ();
//  if (!database.empty())
//  {
//    database = " (" + database + ")";
//  }
//
//  if (!basePath.empty ())
//  {
//    fs::path srcSafeIni = basePath / "srcsafe.ini";
//    fs::path dataPath  = basePath / CIniFile::GetValue("Data_Path" + database, "", srcSafeIni.string());
//    fs::path usersPath = basePath / CIniFile::GetValue("Users_Path"+ database, "", srcSafeIni.string());
//    fs::path usersText = basePath / CIniFile::GetValue("Users_Text"+ database, "", srcSafeIni.string());
//    fs::path tempPath  = basePath / CIniFile::GetValue("Temp_Path" + database, "", srcSafeIni.string());
//  }
//}
