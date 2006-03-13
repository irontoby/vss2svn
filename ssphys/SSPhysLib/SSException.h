// SSException.h: interface for the SSException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSEXCEPTION_H__56E9DD10_4947_45AB_A7C5_F732DE0538A5__INCLUDED_)
#define AFX_SSEXCEPTION_H__56E9DD10_4947_45AB_A7C5_F732DE0538A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <sstream>
#include "SSRecord.h"

#if !defined(_RAISE)
#	define _RAISE(e) throw e
#endif

//---------------------------------------------------------------------------
class SSException : public std::exception
{
public:
  SSException(const std::string& str)
    : exception (), m_Str (str)
    {
    }
	virtual ~SSException() throw ()
		{}
	virtual const char *what() const throw ()
		{return (m_Str.c_str()); }
protected:
	virtual void _Doraise() const
		{_RAISE(*this); }

  std::string m_Str;
};



//---------------------------------------------------------------------------
class SSRecordException : public SSException
{
public:
  SSRecordException(const std::string& str)
    : SSException (str)
    {
    }
protected:
	virtual void _Doraise() const
		{_RAISE(*this); }
private:
};

//---------------------------------------------------------------------------
#include <strstream>
class SSUnknownActionException : public SSException
{
public:
  SSUnknownActionException (short action, SSRecordPtr record)
    : m_Action (action), m_record (record), SSException ("unknown action")
  {
  }
  virtual ~SSUnknownActionException() throw ()
  {}
  virtual const char *what() const throw ()
  { 
    std::ostringstream stream;
	stream << m_Str << " " << m_Action << " at offset 0x" << std::hex << m_record->GetOffset();
	m_Message = stream.str();
	return (m_Message.c_str ()); 
  }
private:
  short m_Action;
  mutable std::string m_Message;
  SSRecordPtr m_record;
};

#endif // !defined(AFX_SSEXCEPTION_H__56E9DD10_4947_45AB_A7C5_F732DE0538A5__INCLUDED_)
