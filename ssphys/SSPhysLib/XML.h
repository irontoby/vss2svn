// XML.h:structure definitions for SourceSafe files
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
#define AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tinyxml.h"
#include <boost\lexical_cast.hpp>

typedef std::map<std::string, std::string> AttribMap;
class XMLElement;
class XMLText;

// ---------------------------------------------------------------
class XMLNode // : protected XMLEntity
{
public:
  template <class T>
  XMLNode (XMLNode* pParent, std::string name, const T& element)
    : m_Node (name), m_pParent (pParent)
  {
    TiXmlText text (boost::lexical_cast<std::string>(element));
    m_Node.InsertEndChild(text);
  }

  XMLNode (XMLNode* pParent, std::string name, AttribMap attrib)
    : m_Node (name), m_pParent (pParent)
  {
    AttribMap::iterator itor = attrib.begin ();
    for (; itor != attrib.end (); ++itor)
    {
      m_Node.SetAttribute(itor->first, itor->second);
    }

  }

  template <class T>
  XMLNode (XMLNode* pParent, std::string name, AttribMap attrib, const T& element)
    : m_Node (name), m_pParent (pParent)
  {
    AttribMap::iterator itor = attrib.begin ();
    for (; itor != attrib.end (); ++itor)
    {
      m_Node.SetAttribute(itor->first, itor->second);
    }
    TiXmlText text (boost::lexical_cast<std::string>(element));
    m_Node.InsertEndChild(text);
  }

  int AddChild (XMLNode* pChild)
  {
    m_Node.InsertEndChild(pChild->m_Node);
    return 0;
  }

  void AddText (XMLText* pContent);

  ~XMLNode ()
  {
    if (m_pParent) 
      m_pParent->AddChild (this);
  }

public:
  TiXmlElement m_Node;
  XMLNode* m_pParent;
};


class XMLElement : public XMLNode
{
public:
  template <class T>
  XMLElement (XMLNode* pParent, std::string name, const T& element)
    : XMLNode (pParent, name, element)
  {
  }

  template <class T>
  XMLElement (XMLNode* pParent, std::string name, AttribMap attrib, const T& element)
    : XMLNode (pParent, name, attrib, element) 
  {
  }
};

class XMLText // : protected XMLEntity
{
public:
  template <class T>
  XMLText (XMLNode* pParent, const T& element)
    : m_Text (""), m_pParent (pParent)
  {
    m_Text.SetValue (boost::lexical_cast<std::string>(element));
  }
  ~XMLText ()
  {
    if (m_pParent)
      m_pParent->AddText(this);
  }
public:
  XMLNode* m_pParent;
  TiXmlText m_Text;
};


inline void XMLNode::AddText (XMLText* pContent)
{
    m_Node.InsertEndChild(pContent->m_Text);
}

#endif // !defined(AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
