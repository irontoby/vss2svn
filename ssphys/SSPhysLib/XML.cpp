// XML.cpp:
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "XML.h"

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
    m_Node.SetAttribute(itor->first, itor->second);
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
  TiXmlText xmlText (text);
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
  m_Text.SetValue (value);
}
