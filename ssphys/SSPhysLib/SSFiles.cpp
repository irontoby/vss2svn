 // SSFiles.cpp: implementation of the SSFiles class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSFiles.h"
#include "SSItemInfoObject.h"
#include <fstream>

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 


SSFileImp::SSFileImp (const std::string& fileName)
  : m_pInput (NULL), m_FileName(fileName)
{
  std::ifstream* pfstr;
  m_pInput = pfstr = new std::ifstream (fileName.c_str(), std::ios_base::in | std::ios_base::binary);

  if (!m_pInput)
    throw SSException  (std::string ("failed to create filestream: ").append (fileName));

  if (pfstr->fail())
    throw SSException  (std::string ("failed to open file: ").append (fileName));
}


SSFileImp::SSFileImp (std::istream* pInput)
  : m_pInput (pInput)
{
  if (!m_pInput)
    throw SSException  ("no input stream specified");
}

SSFileImp::~SSFileImp ()
{
  assert (m_pInput);
  std::ifstream* pfstr = dynamic_cast <std::ifstream*> (m_pInput);
  if (pfstr)
    pfstr->close ();

  delete m_pInput; m_pInput = NULL;

//  std::map <long, SSRecordPtr>::iterator itor = m_Records.begin ();
//  for (; itor != m_Records.end (); ++itor)
//  {
//    if ((*itor).second.use_count () != 1)
//      throw std::logic_error ("the reference count for this SSRecord should be one");
//  }
  
//  m_Records.empty ();
}

long SSFileImp::Size ()
{
  assert (m_pInput);
  m_pInput->seekg(0, std::ios_base::end);
  return m_pInput->tellg();
}

std::string SSFileImp::GetFileName () const
{
  return m_FileName;
}

bool SSFileImp::Read (void* ptr, size_t size)
{
  m_pInput->read (reinterpret_cast<char*>(ptr), size);
  return m_pInput->gcount () == size;
}

bool SSFileImp::Read (long offset, void* ptr, size_t len)
{
//  bool bClose = true; // !IsOpen ();
//  if (bClose && !Open ("rb"))
//    throw SSException ("could not open file for reading");
    
  if (!Seek (offset, std::ios_base::beg))
  {
//    if (bClose) Close ();
    return false;
  }
  if (!Read (ptr, len))
  {
//    if (bClose) Close ();
    return false;
  }

//  if (bClose) Close ();
  return true;
}

bool SSFileImp::Seek (size_t offset, std::ios_base::seekdir way)
{
  assert (m_pInput);
  m_pInput->seekg (offset, way);
  return !m_pInput->fail();
}

//size_t SSFileImp::Write (const void* ptr, size_t size, size_t count) const
//{
//  assert (m_pInput);
//  return m_pInput->Write (ptr, size, count);
//}

SSRecordPtr SSFileImp::GetRecord (long offset)
{
  if (offset >= Size ())
    throw SSException ("could not read record at offset behind file size");

  SSRecordPtr recordPtr;
//  std::map <long, SSRecordPtr >::iterator itor = m_Records.find (offset);
//  if (itor == m_Records.end())
//  {
    recordPtr.reset (ReadRecord (offset));
//    m_Records[offset] = recordPtr;
//  }
//  else
//  {
//    recordPtr = SSRecordPtr ((*itor).second);
//  }

  return recordPtr;
}

SSRecord* SSFileImp::ReadRecord (long offset)
{
  return new SSRecord (shared_from_this(), offset);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SSFile::SSFile (const std::string& fileName)
  : m_FileImpPtr (new SSFileImp (fileName))
{
}

SSFile::SSFile (std::istream* pInput)
  : m_FileImpPtr (new SSFileImp (pInput))
{
}


SSFile::~SSFile ()
{
}

//////////////////////////////////////////////////////////////////////
SSBinaryFile::SSBinaryFile (const std::string& fileName)
  : SSFile (fileName)
{
}

SSBinaryFile::SSBinaryFile (std::istream* pInput)
  : SSFile (pInput)
{
}

//////////////////////////////////////////////////////////////////////
SSRecordFile::SSRecordFile(const std::string& fileName)
  : SSBinaryFile (fileName)
{
}

SSRecordFile::SSRecordFile (std::istream* pInput)
  : SSBinaryFile (pInput)
{
}

SSRecordFile::~SSRecordFile ()
{
}

SSRecordFile* SSRecordFile::MakeFile (const std::string& fileName)
{
  std::map <eType, int> fileTypeMap;
  fileTypeMap[eItemRecord] = 0;
  fileTypeMap[eNamesCache] = 1;
  fileTypeMap[eProjectEntry] = 2;
  fileTypeMap[eUsersHeader] = 3;

  eType type = eNone;
  
  try {
    SSPlainFile plainFile (fileName);
    SSRecordPtr pRecord = plainFile.GetFirstRecord();
    if (pRecord)
      type = pRecord->GetType ();
  }
  catch (SSException&)
  {
  }

  if (fileTypeMap.find (type) == fileTypeMap.end ())
  {
    try {
      SSHeaderFile headerFile (fileName);
      SSRecordPtr pRecord = headerFile.GetFirstRecord();
      if (pRecord)
        type = pRecord->GetType ();
    }
    catch (SSException&)
    {
    }
  }
  
  if (fileTypeMap.find (type) == fileTypeMap.end ())
    throw SSException  ("unrecognized file");

  switch (fileTypeMap[type])
  {
  case 0: return new SSHistoryFile (fileName);
  case 1: return new SSNamesCacheFile (fileName);
  case 2: return new SSProjectFile (fileName);
  case 3: return new SSUserFile (fileName);
  default:
    throw SSException  (std::string ("unknown file type: ").append(SSRecord::TypeToString(type)));
  }

  return NULL;
}

SSRecordPtr SSRecordFile::GetFirstRecord ()
{
  return GetRecord (GetHeaderLength ());
}

SSRecordPtr SSRecordFile::GetNextRecord (SSRecordPtr pRecord)
{
  SSRecordPtr pNext;
  if (pRecord && pRecord->IsValid ())
  {
    pNext = GetRecord (pRecord->GetNextOffset ());

//    if (pNext && (pNext->GetType () == eNone || pNext->GetType () == eUnknown))
//      throw SSException ("unknwon record type" + pNext->GetRecordType ());
  }
  return pNext;
}

SSRecordPtr SSRecordFile::FindNextRecord (SSRecordPtr pRecord)
{
  SSRecordPtr pNext;
  if (pRecord && pRecord->IsValid ())
  {
    long offset = pRecord->GetNextOffset ();
    while (!pNext && offset < m_FileImpPtr->Size () - sizeof (RECORD_HEADER))
    {
      try{
        pNext = GetRecord (offset);
      }
      catch (SSRecordException&)
      {
        ++offset;
      }
    }
  }
  return pNext;
}

SSRecordPtr SSRecordFile::GetRecord (long offset)
{
  assert (m_FileImpPtr);
  return m_FileImpPtr->GetRecord (offset);
}

void SSRecordFile::Dump (std::ostream& os)
{
//  oss << "SourceSafe physical file: " << m_FileName << std::endl;
}

bool SSRecordFile::Validate ()
{ 
  bool retval = true;

  SSRecordPtr pRecord;
  for (pRecord = GetFirstRecord (); pRecord; )
  {
    std::auto_ptr <SSObject> pObject (SSObject::MakeObject(pRecord));
    retval &= pObject->Validate ();

    pRecord = GetNextRecord (pRecord);
  }
  return retval; 
}

//---------------------------------------------------------------------------
SSHeaderFile::SSHeaderFile (const std::string& fileName)
: SSRecordFile (fileName)
{
}

long SSHeaderFile::GetHeaderLength ()
{
  return (sizeof (m_Header));
}

void SSHeaderFile::Dump (std::ostream& os)
{
  SSRecordFile::Dump (os);
}

//---------------------------------------------------------------------------
SSPlainFile::SSPlainFile (const std::string& fileName)
: SSRecordFile (fileName)
{
}

SSPlainFile::SSPlainFile (std::istream* pInput)
: SSRecordFile (pInput)
{
}

long SSPlainFile::GetHeaderLength ()
{
  return (0);
}

void SSPlainFile::Dump (std::ostream& os)
{
  SSRecordFile::Dump (os);
}

//---------------------------------------------------------------------------
SSHistoryFile::SSHistoryFile (const std::string& fileName)
  : SSHeaderFile (fileName), m_pItemInfo (NULL)
{
}

SSHistoryFile::~SSHistoryFile ()
{
  delete m_pItemInfo;
}

bool SSHistoryFile::CheckHeader ()
{
  // Read SourceSafeHeader
  assert (m_FileImpPtr);
  int size = m_FileImpPtr->Read (m_Header, sizeof (m_Header));
  if (size != sizeof(m_Header))
    throw SSException ("Could not read the SourceSafe header");

  // Check SourceSafeHeader
  warn_if (strncmp (m_Header, "SourceSafe@Microsoft", 20) != 0);

  for (int i = 21; i < countof (m_Header); i++)
  {
    // folgende Beobachtungen bisher gemacht
    if (i==32)
    {
      // File oder Project?
      warn_if (m_Header[i] != 1 && m_Header[i] != 2);
    }
    else if (i==34) 
    {
      // Archive version nummer
      warn_if (m_Header[i] != 6);
    }
    else
      warn_if (m_Header[i] != 0);
  }
  return true;
}

std::auto_ptr<SSItemInfoObject> SSHistoryFile::GetItemInfo ()
{
  return std::auto_ptr<SSItemInfoObject> (SSItemInfoObject::MakeItemInfo (GetRecord (GetHeaderLength ())));
}

SSVersionObject SSHistoryFile::GetLastVersion ()
{
  std::auto_ptr<SSItemInfoObject> pItemInfo (GetItemInfo ());
  SSRecordPtr pRecord = GetRecord (pItemInfo->GetHistoryOffsetLast());
  return SSVersionObject (pRecord);
}

SSVersionObject SSHistoryFile::GetPrevVersion (const SSVersionObject& version)
{
  return version.GetPreviousObject();
}

void SSHistoryFile::Dump (std::ostream& os)
{
  SSHeaderFile::Dump (os);
}

bool SSHistoryFile::IsProject ()
{
  return GetItemInfo ()->GetType() == SSITEM_PROJECT;
}

bool SSHistoryFile::IsFile ()
{
  return GetItemInfo ()->GetType() == SSITEM_FILE;
}

std::string SSHistoryFile::GetLatestExt()
{
  return GetItemInfo ()->GetLatestExt();
}

//---------------------------------------------------------------------------
SSUserFile::SSUserFile (const std::string& fileName)
  : SSHeaderFile (fileName)
{
}

bool SSUserFile::CheckHeader ()
{
  // Read SourceSafeHeader
  assert (m_FileImpPtr);
  int size = m_FileImpPtr->Read (m_Header, sizeof (m_Header));
  if (size != sizeof (m_Header))
    throw SSException ("Could not read the SourceSafe header");

  // Check SourceSafeHeader
  warn_if (strncmp (m_Header, "UserManagement@Microsoft", 25) != 0);

  for (int i = 26; i < countof (m_Header); i++)
  {
    // folgende Beobachtungen bisher gemacht
    if (i==32)
    {
      warn_if (m_Header[i] != 8);
    }
    else
      warn_if (m_Header[i] != 0);
  }
  return true;
}

void SSUserFile::Dump (std::ostream& os)
{
  SSHeaderFile::Dump (os);
}



//---------------------------------------------------------------------------
SSNamesCacheFile::SSNamesCacheFile (const std::string& fileName)
  : SSPlainFile (fileName)
{
}


void SSNamesCacheFile::Dump (std::ostream& os)
{
  SSPlainFile::Dump (os);
}



//---------------------------------------------------------------------------
SSProjectFile::SSProjectFile (const std::string& fileName)
  : SSPlainFile (fileName)
{
}

SSProjectFile::SSProjectFile (std::istream* pInput)
: SSPlainFile (pInput)
{
}

void SSProjectFile::Dump (std::ostream& os)
{
  SSPlainFile::Dump (os);
}
