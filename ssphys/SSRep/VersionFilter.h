// VersionFilter.h: interface for the CVersionFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VERSIONFILTER_H__66CEFF72_78D5_4F75_A69C_FCEC324CF8FD__INCLUDED_)
#define AFX_VERSIONFILTER_H__66CEFF72_78D5_4F75_A69C_FCEC324CF8FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <SSPhysLib\SSObject.h>
#include <SSPhysLib\SSVersionObject.h>

class CVersionFilter
{
public:
  virtual ~CVersionFilter () {}
  virtual bool Filter (SSObject* pObject) const = 0;
};

class CVersionRangeFilter : public CVersionFilter
{
public:
  CVersionRangeFilter (int lower, int upper)
    : m_Lower (lower), m_Upper (upper)
  {
  }

  virtual bool Filter (SSObject* pObject) const
  {
    SSVersionObject* pAction = dynamic_cast<SSVersionObject*> (pObject);
    if (pAction)
    {
      if (pAction->GetVersionNumber() >= m_Lower && pAction->GetVersionNumber() <= m_Upper)
        return true;
    }
    return false;
  }

private:
  int m_Lower, m_Upper;
};

class CVersionSingleFilter : public CVersionRangeFilter
{
public:
  CVersionSingleFilter (int version)
    : CVersionRangeFilter (version, version)
  {
  }
};

#endif // !defined(AFX_VERSIONFILTER_H__66CEFF72_78D5_4F75_A69C_FCEC324CF8FD__INCLUDED_)
