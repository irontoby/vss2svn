// SSVersion.h: interface for the SSVersion class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSVERSION_H__A783C194_B621_44E3_A58B_CE8E87BB85FF__INCLUDED_)
#define AFX_SSVERSION_H__A783C194_B621_44E3_A58B_CE8E87BB85FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <SSPhysLib\SSVersionObject.h>
#include "SSItem.h"

class SSVersion;
typedef boost::shared_ptr<SSVersion> SSVersionPtr;

class SSVersion  
{
public:
	SSVersion();
	virtual ~SSVersion();

//    _bstr_t GetUsername ( );
  std::string GetUsername ();

//    long GetVersionNumber ( );
  long GetVersionNumber ( );

//    _bstr_t GetAction ( );
  std::string GetAction ( );

//    DATE GetDate ( );
//  DATE GetDate ( );

//    _bstr_t GetComment ( );
  std::string GetComment ( );

//    _bstr_t GetLabel ( );
  std::string GetLabel ( );

//    _bstr_t GetLabelComment ( );
  std::string GetLabelComment ( );

//    IVSSItemPtr GetVSSItem ( );
  SSItemPtr GetVSSItem ( );

private:
  SSVersionObject* m_pVersion;
};

class SSVersions
{
public:
  SSVersions (SSItemPtr itemPtr, std::string physFile, long iFlags);
  ~SSVersions ();
  
  long GetCount ()          { return m_Items.size (); }
  SSVersionPtr GetVersion (long i);

private:

  std::vector<SSVersionPtr> m_Items;
};


#endif // !defined(AFX_SSVERSION_H__A783C194_B621_44E3_A58B_CE8E87BB85FF__INCLUDED_)
