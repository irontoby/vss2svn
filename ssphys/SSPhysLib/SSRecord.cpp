// SSRecord.cpp: implementation of the SSRecord class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSRecord.h"
#include "SSFiles.h"
#include "crc.h"

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


struct TypeMap {
  char  _string[3];
  eType _eType;
};

TypeMap g_TypeMap[] = {
  {"DH", eItemRecord},
  {"MC", eCommentRecord},
  {"EL", eHistoryRecord},
  {"CF", eCheckOutRecord},
  {"PF", eParentFolder},
  {"FD", eFileDelta},
  {"HN", eNamesCache},
  {"SN", eNameCacheEntry},
  {"JP", eProjectEntry},
  {"HU", eUsersHeader},
  {"UU", eUser},
  {"BF", eBranchFile},
};

eType SSRecord::StringToType (const char type[2])
{
  for (int i = 0; i< countof(g_TypeMap); ++i)
  {
    if (0 == memcmp (type, g_TypeMap[i]._string, 2))
      return g_TypeMap[i]._eType;
  }

  return eUnknown;
}

std::string SSRecord::TypeToString (eType type)
{
  for (int i = 0; i< countof(g_TypeMap); ++i)
  {
    if (type == g_TypeMap[i]._eType)
      return std::string (g_TypeMap[i]._string, 2);
  }

  if (type == eNone)
    return "none";

  return "unknown";
}

bool SSRecord::IsValid () const
{ 
  // TODO: check checksum
  return m_pBuffer != NULL;
}

eType SSRecord::GetType () const
{
  if (!m_pBuffer)
    return eNone;
  return SSRecord::StringToType (m_Header.type);
}

SSRecord::SSRecord (SSFileImpPtr filePtr, long offset)
  : m_FileImpPtr (filePtr), m_Offset(offset), m_pBuffer(NULL), m_Len (0)
{
  int fileLength = m_FileImpPtr->Size ();

  if (!m_FileImpPtr->Read (offset, &m_Header, sizeof(m_Header)))
    throw SSException ("could not read record header");

  // OPTIMIZE: We do not nead to read all the record payload in advance (esp. for FD records)
  if (m_Header.size > 0)
  {
    if (offset + sizeof(m_Header) + m_Header.size > fileLength)
      throw SSRecordException ("bad header: length variable exceeds file size");

    m_pBuffer = new byte[m_Header.size];
    if (!m_FileImpPtr->Read (/*offset + sizeof(m_Header), */ m_pBuffer, m_Header.size))
      throw SSException ("could not read record data");

    short crc = calc_crc16 (m_pBuffer, m_Header.size);
    if (m_Header.checksum != (short)crc && m_Header.checksum != 0)
    {
      SSRecordException ex("wrong checksum");
      Warning (ex.what());
      _RAISE (ex);
    }
  }
  
  m_Len = m_Header.size;
}

SSRecord::SSRecord (eType type, const void* buffer, int len)
  : m_Offset(0), m_pBuffer(NULL), m_Len (0)
{
  m_Header.checksum = calc_crc16 (buffer, len);
  strncpy (m_Header.type, TypeToString (type).c_str(), 2);
  m_Header.size = len;

  // OPTIMIZE: We do not nead to read all the record payload in advance (esp. for FD records)
  if (m_Header.size > 0)
  {
    m_pBuffer = new byte[m_Header.size];
    memcpy (m_pBuffer, buffer, m_Header.size);
  }
  
  m_Len = m_Header.size;
}

void SSRecord::Dump (std::ostream& os) const
{
  os << "Offset: " << GetOffset ();
  os << " Type: " << GetRecordType ();
  os << " Len: " << GetLen();
  std::string validity = IsValid() ? "valid" : "invalid";
  os << " crc: " << m_Header.checksum << " -> " << validity << std::endl; 
}

SSRecord::~SSRecord ()
{
  delete [] m_pBuffer;
}
