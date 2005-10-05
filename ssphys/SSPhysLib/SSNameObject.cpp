// SSNameObject.cpp: implementation of the SSNameObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning (disable: 4786)
#include <string>
#include <map>
#include <boost\lexical_cast.hpp>

#include "SSNameObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SSNameObject::SSNameObject (SSRecordPtr pRecord)
  : SSObject (pRecord, eNameCacheEntry)
{
  Init (pRecord);
}

void SSNameObject::Init (SSRecordPtr pRecord)
{
  if (pRecord->GetLen () < sizeof (NSMAP) + sizeof (NSMAP))  
    throw SSRecordException ("not enough data for name object");

  const NSMAP* pMap = (NSMAP*) pRecord->GetBuffer();
  const NSENTRY* pEntry = (NSENTRY*) ((byte*)pMap   + sizeof (NSMAP));
  const char* pNames = (const char*) ((byte*)pEntry + sizeof (NSENTRY) * pMap->num);

  for (int i = 0; i<pMap->num; ++i)
  {
    warn_if (pEntry->id != 1 && pEntry->id != 2 && pEntry->id != 3 && pEntry->id != 10);
    
    m_NamesMap[pEntry->id] = pNames + pEntry->offset;

    ++pEntry;
  }
}

std::string SSNameObject::GetName (short id)
{
  std::map<short, std::string>::iterator iter = m_NamesMap.find (id);
  if (iter != m_NamesMap.end ())
    return (*iter).second;

  return "";
}


void SSNameObject::ToXml (XMLNode* pParent) const
{
  XMLElement entries (pParent, "NrOfEntries", size ());

  std::map<short, std::string>::const_iterator iter = m_NamesMap.begin ();
  for (; iter != m_NamesMap.end (); ++iter)
  {
    AttribMap map;
    map["id"] = boost::lexical_cast<std::string> ((*iter).first);
    XMLElement name (pParent, "Entry", map, (*iter).second);
  }
}

void SSNameObject::Dump (std::ostream& os) const
{
  SSObject::Dump (os);

  os << "Entries: " << m_NamesMap.size () << std::endl;

  std::map<short, std::string>::const_iterator iter = m_NamesMap.begin ();
  for (; iter != m_NamesMap.end (); ++iter)
  {
    os << "id(" << (*iter).first  << ") "/*, offset (" << pEntry->offset << ")*/ "= " << (*iter).second <<std::endl;
  }
}
