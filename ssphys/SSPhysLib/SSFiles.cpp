// SSFiles.cpp: implementation of the SSFiles class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSFiles.h"
#include "SSItemInfoObject.h"

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CBaseIO::~CBaseIO ()
{
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CFileIO::CFileIO (const std::string& fileName)
  : m_pFile (NULL), m_FileName (fileName)
{
}
CFileIO::~CFileIO ()
{
  if (m_pFile)
    fclose (m_pFile);
}
bool CFileIO::Open (const char* mode)
{
  if (!m_pFile)
    m_pFile = fopen (m_FileName.c_str (), mode);
  
  return m_pFile != NULL;
}
void CFileIO::Close ()
{
  if (m_pFile)
  {
    fclose (m_pFile);
    m_pFile = NULL;
  }
}
size_t CFileIO::Read (void* ptr, size_t size, size_t count)
{
  bool bClose = !m_pFile;
  if (bClose && !Open ("rb"))
    throw ("could not open file for reading");

  size_t retval = fread (ptr, size, count, m_pFile);

  if (bClose)
    Close ();

  return retval;
}
size_t CFileIO::Write (const void* ptr, size_t size, size_t count)
{
  bool bClose = !m_pFile;
  if (bClose && !Open ("a+b"))
    throw ("could not open file for writing");

  size_t retval = fwrite (ptr, size, count, m_pFile);

  if (bClose)
    Close ();

  return retval;
}
bool CFileIO::Seek (size_t offset, int whence)
{
  bool bClose = !m_pFile;
  if (bClose && !Open ("rb"))
    throw SSException ("could not open file for reading");

  int retval = fseek (m_pFile, offset, whence);

  if (bClose)
    Close ();

  return retval == 0;
}
long CFileIO::Size ()
{
  bool bClose = !m_pFile;
  if (!Open ("rb"))
    throw SSException ("could not open file for reading");

  fseek (m_pFile, 0, SEEK_END);
  long retval = ftell (m_pFile);

  if (bClose)
    Close ();

  return retval;
}
std::string CFileIO::FileName ()
{
  return m_FileName;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMemoryIO::CMemoryIO (const void* ptr, long size)
  : m_Ptr (ptr), m_Size (size), m_CurrentPos (0)
{
}
CMemoryIO::~CMemoryIO ()
{
}
bool CMemoryIO::Open (const char*)
{
  return true;
}
void CMemoryIO::Close ()
{
}
size_t CMemoryIO::Read (void* ptr, size_t size, size_t count)
{
  for (size_t i = 0; i<count; i++)
  {
    if (m_Size - m_CurrentPos >= size)
    {
      memcpy (ptr, (byte*)m_Ptr + m_CurrentPos, size);
      m_CurrentPos += size;
      ptr = (byte*)ptr + size;
    }
  }

  return i;
}
size_t CMemoryIO::Write (const void* ptr, size_t size, size_t count)
{
  for (size_t i = 0; i<count; i++)
  {
    if (m_Size - m_CurrentPos >= size)
    {
      memcpy ((byte*)m_Ptr + m_CurrentPos, ptr, size);
      m_CurrentPos += size;
      ptr = (byte*)ptr + size;
    }
  }

  return i;
}

bool CMemoryIO::Seek (size_t offset, int whence)
{
  if (whence == SEEK_SET)
    m_CurrentPos = offset;
  else if (whence == SEEK_END)
    m_CurrentPos = m_Size-offset-1;
  else if (whence == SEEK_CUR)
    m_CurrentPos += offset;

  return m_CurrentPos < m_Size;
}
long CMemoryIO::Size ()
{
  return m_Size;
}
std::string CMemoryIO::FileName ()
{
  return ("");
}






SSFileImp::SSFileImp (const std::string& fileName, bool bOpen /* = false */)
  : m_pIO (NULL)
{
  m_pIO = new CFileIO (fileName);
  if (bOpen && !Open ("rb"))
    throw SSException  ("could not load file");
}


SSFileImp::SSFileImp (CBaseIO* pio, bool bOpen /* = false */)
  : m_pIO (pio)
{
  if (!m_pIO)
    throw SSException  ("no io specified");
  
  if (bOpen && !Open ("rb"))
    throw SSException ("could not load file");
}

SSFileImp::~SSFileImp ()
{
  Close ();
  delete m_pIO; m_pIO = NULL;

//  std::map <long, SSRecordPtr>::iterator itor = m_Records.begin ();
//  for (; itor != m_Records.end (); ++itor)
//  {
//    if ((*itor).second.use_count () != 1)
//      throw std::logic_error ("the reference count for this SSRecord should be one");
//  }
  
//  m_Records.empty ();
}

bool SSFileImp::Open (const char* mode) const
{
  assert (m_pIO);
//  m_pFile = fopen (filename.c_str (), "rb");
  if (m_pIO->Open (mode))
  {
    return true;
  }

  return false;
}

void SSFileImp::Close () const
{
  assert (m_pIO);
  m_pIO->Close ();
}

long SSFileImp::Size ()
{
  assert (m_pIO);
  return m_pIO->Size ();
}

std::string SSFileImp::GetFileName ()
{
  assert (m_pIO);
  return m_pIO->FileName ();
}

size_t SSFileImp::Read (void* ptr, size_t size, size_t count) const
{
  return m_pIO->Read (ptr, size, count);
}

bool SSFileImp::Read (long offset, void* ptr, int len) const
{
  bool bClose = true; // !IsOpen ();
  if (bClose && !Open ("rb"))
    throw SSException ("could not open file for reading");
    
  if (!m_pIO->Seek (offset, SEEK_SET))
  {
    if (bClose) Close ();
    return false;
  }
  if (1 != m_pIO->Read (ptr, len, 1))
  {
    if (bClose) Close ();
    return false;
  }

  if (bClose) Close ();
  return true;
}

bool SSFileImp::Seek (size_t offset, int pos) const
{
  assert (m_pIO);
  return m_pIO->Seek (offset, pos);
}

size_t SSFileImp::Write (const void* ptr, size_t size, size_t count) const
{
  assert (m_pIO);
  return m_pIO->Write (ptr, size, count);
}

SSRecordPtr SSFileImp::GetRecord (long offset) 
{
  if (offset >= m_pIO->Size ())
    return SSRecordPtr ();

  SSRecordPtr recordPtr;
//  std::map <long, SSRecordPtr >::iterator itor = m_Records.find (offset);
//  if (itor == m_Records.end())
//  {
    recordPtr.reset (ReadRecord (shared_from_this(), offset));
//    m_Records[offset] = recordPtr;
//  }
//  else
//  {
//    recordPtr = SSRecordPtr ((*itor).second);
//  }

  return recordPtr;
}

SSRecord* SSFileImp::ReadRecord (SSFileImpPtr fileImp, long offset)
{
  SSRecord* pRecord = new SSRecord (fileImp, offset);
  return pRecord;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//SSRecordFile::SSRecordFile ()
//  : m_pFile (NULL), m_FileName ("")
//{
//}
//
SSFile::SSFile (const std::string& fileName, bool bOpen /* = false */)
  : m_FileImpPtr (new SSFileImp (fileName, bOpen))
{
}

SSFile::SSFile (SSFileImpPtr filePtr)
  : m_FileImpPtr (filePtr)
{
}

SSFile::SSFile (CBaseIO* pio, bool bOpen /* = false */)
  : m_FileImpPtr (new SSFileImp (pio, bOpen))
{
}


SSFile::~SSFile ()
{
  Close ();
}

bool SSFile::Open ()
{
  assert (m_FileImpPtr);
  if (m_FileImpPtr->Open ("rb"))
  {
    assert (Validate ());
    return true;
  }
  return false;
}

void SSFile::Close ()
{
  if (m_FileImpPtr)
    m_FileImpPtr->Close ();
}

//long SSRecordFile::Size ()
//{
//  assert (m_FileImpPtr);
//  return m_FileImpPtr->Size ();
//}

std::string SSFile::GetFileName ()
{
  assert (m_FileImpPtr);
  return m_FileImpPtr->GetFileName ();
}

//SSFileImpPtr SSFile::GetFileImp ()
//{
//  return m_FileImpPtr;
//}

//////////////////////////////////////////////////////////////////////
SSBinaryFile::SSBinaryFile (const std::string& fileName, bool bOpen /* = false */)
  : SSFile (fileName, bOpen)
{
}

SSBinaryFile::SSBinaryFile (SSFileImpPtr filePtr)
  : SSFile (filePtr)
{
}

SSBinaryFile::SSBinaryFile (CBaseIO* pio)
  : SSFile (pio)
{
}

//////////////////////////////////////////////////////////////////////
SSRecordFile::SSRecordFile(const std::string& fileName, bool bOpen /* = false */)
  : SSBinaryFile (fileName, bOpen)
{
}

SSRecordFile::SSRecordFile (CBaseIO* pio)
  : SSBinaryFile (pio)
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
    return NULL;

  switch (fileTypeMap[type])
  {
  case 0: return new SSHistoryFile (fileName);
  case 1: return new SSNamesCacheFile (fileName);
  case 2: return new SSProjectFile (fileName);
  case 3: return new SSUserFile (fileName);
  default:
    throw SSException  ("unknown file type");
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

//bool SSRecordFile::Read (long offset, void* ptr, int len)
//{
//  assert (m_FileImpPtr);
//  return m_FileImpPtr->Read(offset, ptr, len);
//}

SSRecordPtr SSRecordFile::GetRecord (long offset)
{
  assert (m_FileImpPtr);
  return m_FileImpPtr->GetRecord (offset);
}

void SSRecordFile::Dump (std::ostream& os)
{
//  oss << "SourceSafe physical file: " << m_FileName << std::endl;
}

//void SSRecordFile::DumpRecords (std::ostream& oss)
//{
//  SSRecordPtr pRecord;
//  for (pRecord = GetFirstRecord (); pRecord; )
//  {
//    oss << ">>> offset: 0x" << std::hex << pRecord->GetOffset() << std::dec;
//    oss << " Type: " << pRecord->GetRecordType () << std::endl;
//    std::auto_ptr <SSObject> pObject (SSObject::MakeObject(GetFileImp (), pRecord));
//    pObject->Dump (oss);
//    oss << std::endl;
//
//    pRecord = GetNextRecord (pRecord);
//  }
//}

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
//SSHeaderFile::SSHeaderFile ()
//: SSRecordFile ()
//{
//}

SSHeaderFile::SSHeaderFile (const std::string& fileName, bool bOpen /*= false*/)
: SSRecordFile (fileName, bOpen)
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
//SSPlainFile::SSPlainFile ()
//: SSRecordFile ()
//{
//}

SSPlainFile::SSPlainFile (const std::string& fileName, bool bOpen /*= false*/)
: SSRecordFile (fileName, bOpen)
{
}

SSPlainFile::SSPlainFile (CBaseIO* pio)
: SSRecordFile (pio)
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
//SSHistoryFile::SSHistoryFile ()
//  : SSHeaderFile ()
//{
//}

SSHistoryFile::SSHistoryFile (const std::string& fileName, bool bOpen /*= false*/)
  : SSHeaderFile (fileName, false), m_pItemInfo (NULL)
{
  if (bOpen && !Open ())
    throw std::runtime_error ("could not load file");
}

SSHistoryFile::~SSHistoryFile ()
{
  delete m_pItemInfo;
}

bool SSHistoryFile::CheckHeader ()
{
  // Read SourceSafeHeader
  assert (m_FileImpPtr);
  int size = m_FileImpPtr->Read (m_Header, sizeof (m_Header), 1);
  if (size != 1)
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
//    std::list<SSRecord*>::iterator itor;
//    for (itor = m_Records.begin (); itor != m_Records.end (); ++itor)
//    {
//      if ((*itor)->GetType () == eItemRecord)
//        return *itor;
//    }
//    return NULL;
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
//  SSRecordFile::Dump (oss);
//  SSItemInfoObject* pItem = GetItemInfo ();
//  if (pItem)
//    pItem->Dump (oss);
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
//SSUserFile::SSUserFile ()
//  : SSHeaderFile ()
//{
//}

SSUserFile::SSUserFile (const std::string& fileName, bool bOpen /*= false*/)
  : SSHeaderFile (fileName, false)
{
  if (bOpen && !Open ())
    throw std::runtime_error ("could not load file");
}

bool SSUserFile::CheckHeader ()
{
  // Read SourceSafeHeader
  assert (m_FileImpPtr);
  int size = m_FileImpPtr->Read (m_Header, sizeof (m_Header), 1);
  if (size != 1)
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
//SSNamesCacheFile::SSNamesCacheFile()
//  : SSPlainFile ()
//{
//}

SSNamesCacheFile::SSNamesCacheFile (const std::string& fileName, bool bOpen /*= false*/)
  : SSPlainFile (fileName, false)
{
  if (bOpen && !Open ())
    throw std::runtime_error ("could not load file");
}


void SSNamesCacheFile::Dump (std::ostream& os)
{
  SSPlainFile::Dump (os);
}



//---------------------------------------------------------------------------
//SSProjectFile::SSProjectFile ()
//  : SSPlainFile ()
//{
//}

SSProjectFile::SSProjectFile (const std::string& fileName, bool bOpen /*= false*/)
  : SSPlainFile (fileName, false)
{
  if (bOpen && !Open ())
    throw std::runtime_error ("could not load file");
}

SSProjectFile::SSProjectFile (CBaseIO* pio)
: SSPlainFile (pio)
{
}

void SSProjectFile::Dump (std::ostream& os)
{
  SSPlainFile::Dump (os);
}
