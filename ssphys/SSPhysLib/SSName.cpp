// SSName.cpp: implementation of the SSName class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSName.h"
//#include "SSDatabase.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SSName::SSName(SSNAME ssname, SSNamesCache* pNameService)
  : m_ssName (ssname),
    m_pNamesService (pNameService)
{
//  if (!m_pNamesService && SSDatabase::GetCurrentDatabase ())
//    m_pNamesService = SSDatabase::GetCurrentDatabase ()->GetNamesService ();
}

SSName::~SSName()
{

}

//---------------------------------------------------------------------------
long SSName::GetOffset () const
{
  return m_ssName.nsmap;
}

//---------------------------------------------------------------------------
std::string SSName::GetType () const
{
  switch (m_ssName.flags)
  {
  case 0:
    return "file";
  case 1:
    return "project";
  }
  return "type unknown";
}

//---------------------------------------------------------------------------
std::string SSName::GetFullName () const
{
  std::string name = m_ssName.name;
  if (m_pNamesService && m_ssName.nsmap != NULL)
  { 
    std::string altName = m_pNamesService->GetName (m_ssName.flags, m_ssName.nsmap);
    if (!altName.empty ())
      name = altName;
  }
  return name;
}

//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const SSName& ssname)
{
  os << ssname.GetFullName ();
  
  return os;
}

// ---------------------------------------------------------------
void SSName::ToXml (XMLNode* pParent, std::string name /*= "SSName"*/)
{
  std::stringstream stream;
  stream << GetOffset ();
  AttribMap map;
  map["type"] = GetType ();
  map["offset"] = stream.str();
  XMLElement node (pParent, name, map, GetFullName ());
};
