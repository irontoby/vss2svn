// SSVersion.cpp: implementation of the SSVersion class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSVersion.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SSVersion::SSVersion()
{

}

SSVersion::~SSVersion()
{

}

std::string SSVersion::GetUsername ()
{
  return m_pVersion->GetUsername();
}

long SSVersion::GetVersionNumber ( )
{
  return m_pVersion->GetVersionNumber();
}

std::string SSVersion::GetAction ( )
{
  return m_pVersion->GetActionString();
}

//  DATE SSVersion::GetDate ( );

std::string SSVersion::GetComment ( )
{
  return m_pVersion->GetComment();
}

std::string SSVersion::GetLabel ( )
{
#pragma message ("fix")
//  return m_pVersion->GetLabel();
  return "";
}

std::string SSVersion::GetLabelComment ( )
{
#pragma message ("fix")
//    return m_pVersion->GetLabelComment();
  return "";
}

SSItemPtr SSVersion::GetVSSItem ( )
{
  return SSItemPtr();
}


SSVersions::SSVersions (SSItemPtr itemPtr, std::string physFile, long iFlags)
{
}
