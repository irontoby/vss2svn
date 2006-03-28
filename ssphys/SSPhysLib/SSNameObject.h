// SSNameObject.h: interface for the SSNameObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSNAMEOBJECT_H__8116FB34_17A5_40BB_829F_28413D2A00DA__INCLUDED_)
#define AFX_SSNAMEOBJECT_H__8116FB34_17A5_40BB_829F_28413D2A00DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SSObject.h"
#include "SSFiles.h"


//------------------------------------------------------------------------------
class SSNameObject : public SSObject
{
public:
  typedef std::map<short, std::string>::const_iterator const_iterator;
  typedef std::map<short, std::string>::size_type size_type;

  SSNameObject (SSRecordPtr pRecord);

  const_iterator begin () const { return m_NamesMap.begin (); }
  const_iterator end () const   { return m_NamesMap.end (); }
  size_type size () const       { return m_NamesMap.size (); }
  
  std::string GetName (short id);

  virtual void Accept (ISSObjectVisitor& rVisitor, const ISSContext* pCtx = NULL) const { rVisitor.Apply (*this, pCtx); }

  void ToXml (XMLNode* pParent) const;
  void Dump (std::ostream& os) const;

protected:
  void Init (SSRecordPtr pRecord);

  std::map<short, std::string> m_NamesMap;
};

//------------------------------------------------------------------------------
class SSNamesCache
{
public:
  SSNamesCache ()
    : m_pFile (NULL)
  {
  }
  
  ~SSNamesCache ()
  {
    delete m_pFile;
  }

  void SetFile (SSNamesCacheFile* pFile)
  {
    m_pFile = pFile;
  }

  std::string GetName (short flag, long offset, int cp=0)
  {
    if (!m_pFile)
      return "";

    SSRecordPtr pRecord = m_pFile->GetRecord (offset);
    SSNameObject names (pRecord);
    return names.GetName (flag << 3 | 2);
  }

private:
  SSNamesCacheFile* m_pFile;
};


#endif // !defined(AFX_SSNAMEOBJECT_H__8116FB34_17A5_40BB_829F_28413D2A00DA__INCLUDED_)
