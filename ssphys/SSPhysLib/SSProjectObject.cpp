// SSProjectObject.cpp: implementation of the SSProjectObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSProjectObject.h"
#include "SSName.h"
#include <strstream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SSProjectObject::SSProjectObject(const PROJECT_ENTRY& pe)
  : SSObject(SSRecordPtr (new SSRecord (eProjectEntry, &pe, sizeof (PROJECT_ENTRY))), eProjectEntry)
{
  if (GetRecord()->GetLen() < sizeof (PROJECT_ENTRY))
    throw SSRecordException ("not enough data for project object");
  
  m_pProjectEntry = (PROJECT_ENTRY*) SSObject::GetData ();
}

SSProjectObject::SSProjectObject(SSRecordPtr pRecord)
  : SSObject(pRecord, eProjectEntry)
{
  if (pRecord->GetLen() < sizeof (PROJECT_ENTRY))
    throw SSRecordException ("not enough data for project object");
  
  m_pProjectEntry = (PROJECT_ENTRY*) SSObject::GetData ();
}

SSProjectObject::~SSProjectObject()
{

}

std::string SSProjectObject::GetName () const
{
  SSName name (m_pProjectEntry->name);
  std::string fullName (name.GetFullName());
//  if (IsShared())
  if (GetPinnedToVersion() != 0)
  {
    char buffer[66];
    fullName += ";";
    fullName += itoa (GetPinnedToVersion(), buffer, 10);
  }
  return fullName;
}

std::string SSProjectObject::GetPhysFile () const
{
  return std::string (m_pProjectEntry->phys, 8);
}

int SSProjectObject::GetType () const
{
  return (m_pProjectEntry->type);
}

eFileType SSProjectObject::GetFileType () const
{
  if (IsStoreBinaryDiff ())
    return eFileTypeBinary;

  return eFileTypeText;
}

bool SSProjectObject::IsDeleted () const
{
  return (m_pProjectEntry->flags & 0x01) != 0;
}

bool SSProjectObject::IsStoreBinaryDiff () const
{
  return (m_pProjectEntry->flags & 0x02) != 0;
}

bool SSProjectObject::IsStoreLatestRev () const
{
  return (m_pProjectEntry->flags & 0x04) != 0;
}

bool SSProjectObject::IsShared () const
{
  return (m_pProjectEntry->flags & 0x08) != 0;
}

int SSProjectObject::GetPinnedToVersion () const
{
  return (m_pProjectEntry->pinnedToVersion);
}

// 0x08 shared
// 0x02 store binary diffs
// 0x01 deleted
std::string FlagsToString (short flags)
{
  std::ostringstream ost;
  if (flags & 0x01)
    ost << "deleted";
  if (flags & 0x02)
    ost << " binary";
  if (flags & 0x08)
    ost << " shared";
  if (flags & 0xf4)
    ost << " unknown(" << flags << ")";
  
  ost << '\0';
  return ost.str ();
}

std::string TypeToString (short type)
{
  warn_if (type != 1 && type != 2);

  char* types[2] = { "project", "file" };
  if (type == 1 || type == 2)
    return types[type-1];

  std::ostringstream ost;
  ost << "unknown type 0x" << std::hex << type << std::endl;
  return ost.str();
}

//void SSProjectObject::Dump (std::ostream& os)
//{
//  oss << "Type:  " << TypeToString (m_pProjectEntry->type) << std::endl;
//  oss << "Flags: 0x" << std::hex << m_pProjectEntry->flags << std::dec << " " << FlagsToString (m_pProjectEntry->flags) << std::endl;
//  SSName ssName (m_pProjectEntry->name);
//  oss << "SSName: " << ssName << std::endl;
//  if (m_pProjectEntry->pinnedToVersion)
//    oss << "Pinned to version: " << m_pProjectEntry->pinnedToVersion << std::endl;
//  oss << "Phys file: " << m_pProjectEntry->phys << std::endl;
//}

void SSProjectObject::Delete ()
{
  m_pProjectEntry->flags &= 0x01;
}
void SSProjectObject::Recover ()
{
  m_pProjectEntry->flags &= ~0x01;
}
void SSProjectObject::Rename (SSNAME oldName, SSNAME newName)
{
  if (memcmp (&oldName, &m_pProjectEntry->name, sizeof (SSNAME)) != 0)
    throw SSException ("old name does not match");
  
  m_pProjectEntry->name = newName;
}

void SSProjectObject::Pin (int version)
{
  m_pProjectEntry->pinnedToVersion = version;
}

//void SSProjectObject::ToXml (XMLNode* pParent) const
//{
//}
void SSProjectObject::Dump (std::ostream& os) const
{
  SSObject::Dump (os);
  
}
