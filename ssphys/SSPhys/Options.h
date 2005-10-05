// Options.h: interface for the COptions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_)
#define AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>


//---------------------------------------------------------------------------
enum tristate { cleared, undefined, set };

inline void operator &= (bool& _bool, tristate _tristate)
{
  _bool = _tristate == undefined ? _bool : _tristate == cleared ? false : true;
}


//---------------------------------------------------------------------------
struct CValue 
{
  CValue ()
    : m_Type (typeUndefined)
  { }
  CValue (tristate val)
    : m_Type (typeTristate), pTristateValue (val)
  { }
  CValue (const char* pVal)
    : m_Type (typeString), m_pStringValue (pVal)
  { }
  CValue (const bool val)
    : m_Type (typeBool), pBoolValue(val)
  { }

  enum { typeUndefined, typeTristate, typeBool, typeString} m_Type;
  union {
    tristate pTristateValue;
    bool pBoolValue;
    const char* m_pStringValue;
  };

  operator const char* () const
  {
    assert (m_Type == typeString);
    return m_pStringValue;
  }

  operator tristate () const
  {
    assert (m_Type == typeTristate);
    return pTristateValue;
  }

};

struct COption
{
  int id;
  CValue value;
};

typedef std::list<COption> COptionsList;

struct COptionInfo
{
public:
  enum  eArgType { noArgument, requiredArgument, optionalArgument, tristateArgument };

  COptionInfo (int id, char shortOption, std::string longOption, std::string descr, eArgType needArg);

  int m_Id;
  char m_shortOption;
  std::string m_longOption;
  std::string m_Description;
  eArgType m_needArg;
};

std::ostream& operator<<(std::ostream& os, const COptionInfo& info);

typedef std::list<COptionInfo> COptionInfoList;


class COptions
{
public:
  virtual ~COptions () {};

  virtual COptionInfoList GetOptionsInfo () const
  {
    return COptionInfoList ();
  }
  void SetOptions (const COptionsList& options)
  {
    COptionsList::const_iterator itor = options.begin ();
    while (itor != options.end())
    {
//      if (!SetOption (*itor))
//        throw SSException ("unknown option");
      SetOption (*itor);
      ++itor;
    }
  }
  virtual bool SetOption (const COption& option) = 0;

  virtual void PrintUsage () const;
};


#endif // !defined(AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_)
