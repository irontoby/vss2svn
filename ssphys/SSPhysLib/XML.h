// XML.h:structure definitions for SourceSafe files
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
#define AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tinyxml.h"
#include <boost/lexical_cast.hpp>

typedef std::map<std::string, std::string> AttribMap;
class XMLElement;
class XMLText;

// ---------------------------------------------------------------
class XMLNode // : protected XMLEntity
{
public:
  XMLNode (XMLNode* pParent, std::string name, AttribMap attrib);

  template <class T>
  XMLNode (XMLNode* pParent, std::string name, const T& element)
    : m_Node (name), m_pParent (pParent)
  {
    SetText (boost::lexical_cast<std::string>(element));
  }

  template <class T>
  XMLNode (XMLNode* pParent, std::string name, AttribMap attrib, const T& element)
    : m_Node (name), m_pParent (pParent)
  {
    SetAttributes (attrib);
    SetText (boost::lexical_cast<std::string>(element));
  }

  ~XMLNode ();

  void AddChild (XMLNode* pChild);
  void AddText (XMLText* pContent);

  void SetAttributes (AttribMap attrib);
  void SetText (std::string text);

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
    SetValue (boost::lexical_cast<std::string>(element));
  }

  ~XMLText ();

  void SetValue (std::string value);

public:
  XMLNode* m_pParent;
  TiXmlText m_Text;
};

#endif // !defined(AFX_XML_H__6602C07F_65ED_4FD7_A730_6D416805378A__INCLUDED_)
