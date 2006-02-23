// XML.cpp:
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "XML.h"

class CValidXMLChar
{
public:
  bool operator () (char c)
  {
    bool b = true;
    if ( (c < 0x20 && c != 0x09 && c != 0x0A && c != 0x0D)
      || (c >= 0x7f && c <= 0x84)
      || (c >= 0x86 && c <= 0x9f) )
    {
      m_bInvalidOccured = true;
      return true;
    }
    return false;
  }
  bool m_bInvalidOccured;
};

std::string sanitizeForXML (const std::string& input)
{
  std::string output (input);
  CValidXMLChar validXMLChar;
  std::replace_if (output.begin (), output.end (), validXMLChar, '_');
  return output;
}

// ---------------------------------------------------------------

XMLNode::XMLNode (XMLNode* pParent, std::string name, AttribMap attrib)
  : m_Node (name), m_pParent (pParent)
{
  SetAttributes (attrib);
}

void XMLNode::SetAttributes (AttribMap attrib)
{
  AttribMap::iterator itor = attrib.begin ();
  for (; itor != attrib.end (); ++itor)
  {
    m_Node.SetAttribute(itor->first, sanitizeForXML (itor->second));
  }
}

void XMLNode::AddChild (XMLNode* pChild)
{
  m_Node.InsertEndChild(pChild->m_Node);
}

void XMLNode::AddText (XMLText* pContent)
{
  m_Node.InsertEndChild(pContent->m_Text);
}

void XMLNode::SetText (std::string text)
{
  TiXmlText xmlText (sanitizeForXML (text));
  m_Node.InsertEndChild(xmlText);
}

XMLNode::~XMLNode ()
{
  if (m_pParent) 
    m_pParent->AddChild (this);
}

// ---------------------------------------------------------------
XMLText::~XMLText ()
{
  if (m_pParent)
    m_pParent->AddText(this);
}


void XMLText::SetValue (std::string value)
{
  m_Text.SetValue (sanitizeForXML (value));
}

