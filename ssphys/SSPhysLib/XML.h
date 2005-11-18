// XML.h:structure definitions for SourceSafe files
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
#define AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef std::map<std::string, std::string> AttribMap;
class XMLElement;
class XMLValue;

// ---------------------------------------------------------------
class XMLEntity
{
protected:
  static bool isSpecialChar (std::string::value_type v)
  {
    std::string punct = "&<>'\"";
    std::string::iterator it = punct.begin();

    while(it != punct.end())
      if(v == *it++)
          return true;

    return false; 
  }
  
  template <class T>
  void ToXml (const T& element)
  {
    std::cout << element; 
  }
  void ToXml (const std::string& element)
  {
    if (std::find_if (element.begin (), element.end (), isSpecialChar) == element.end ())
      std::cout << element;
    else
      std::cout << "<![CDATA[" << element << "]]>";
  }
  void ToXml (const char& element)
  {
    if (std::find_if (&element, &element + strlen (&element), isSpecialChar) == &element+ strlen (&element))
      std::cout << element;
    else
      std::cout << "<![CDATA[" << element << "]]>";
  }

};

// ---------------------------------------------------------------
class XMLNode : protected XMLEntity
{
public:
  XMLNode (XMLNode* pParent, std::string name)
    : m_pParent (pParent), m_Name (name), m_bClosed (false), m_Indent (0)
  {
    m_Indent = pParent ? pParent->AddChild (this) + 2 : 0;

    // closing bracket will be written in destructor or when elements are added
    std::cout << std::string (m_Indent, ' ') << "<" << name << /*">" << */ std::endl;
  }

  XMLNode (XMLNode* pParent, std::string name, AttribMap attrib)
    : m_pParent (pParent), m_Name (name), m_bClosed (false), m_Indent (0)
  {
    m_Indent = pParent ? pParent->AddChild (this) + 2 : 0;

    std::cout << std::string (m_Indent, ' ') << "<" << name;
    AttribMap::iterator itor = attrib.begin ();
    for (; itor != attrib.end (); ++itor)
    {
      std::cout << " " << itor->first << "=\"" << itor->second << "\"";
    }
    // closing bracket will be written in destructor or when elements are added
    // std::cout << ">" << std::endl;
  }

  int AddChild (XMLNode* pChild)
  {
    if (!m_bClosed)
      std::cout << ">" << std::endl;

    m_bClosed = true;
    return m_Indent;
  }

  int AddChild (XMLElement* pChild)
  {
    if (!m_bClosed)
      std::cout << ">" << std::endl;

    m_bClosed = true;
    return m_Indent;
  }

  void AddContent (XMLValue* pContent)
  {
    if (!m_bClosed)
      std::cout << ">";

    m_bClosed = true;
  }

  ~XMLNode ()
  {
    if (!m_bClosed)
      std::cout << "/>" << std::endl;
    else
      std::cout << std::string (m_Indent, ' ') << "</" << m_Name << ">" << std::endl;
  }

  int GetIndent () const { return m_Indent; }

protected:
  XMLNode* m_pParent;
  std::string m_Name;
  bool m_bClosed;
  int m_Indent;
};


class XMLElement : protected XMLEntity
{
public:
  template <class T>
  XMLElement (XMLNode* pParent, std::string name, const T& element)
  {
    int indent = pParent ? pParent->AddChild (this) + 2 : 0;

    std::cout << std::string (indent, ' ') << "<" << name << ">";
    ToXml(element);
    std::cout << "</" << name << ">" << std::endl;
  }

  template <class T>
  XMLElement (XMLNode* pParent, std::string name, AttribMap attrib, const T& element)
  {
    int indent = pParent ? pParent->AddChild (this) + 2 : 0;

    std::cout << std::string (indent, ' ')  << "<" << name;
    AttribMap::iterator itor = attrib.begin ();
    for (; itor != attrib.end (); ++itor)
    {
      std::cout << " " << itor->first << "=\"" << itor->second << "\"";
    }
    std::cout << ">";
    ToXml(element);
    std::cout << "</" << name << ">" << std::endl;
  }
};

class XMLValue : protected XMLEntity
{
public:
  template <class T>
  XMLValue (XMLNode* pParent, const T& element)
  {
    if (pParent)
      pParent->AddContent(this);

    ToXml(element);
  }

};

#endif // !defined(AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
