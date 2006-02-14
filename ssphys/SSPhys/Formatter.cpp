// IFormatter.cpp: implementation of the IFormatter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Formatter.h"

#include <boost\lexical_cast.hpp>

#include <SSPhysLib\SSFiles.h>
#include <SSPhysLib\SSCheckoutObject.h>
#include <SSPhysLib\SSNameObject.h>
#include <SSPhysLib\SSItemInfoObject.h>
#include <SSPhysLib\SSProjectObject.h>
#include <SSPhysLib\SSParentFolderObject.h>
#include <SSPhysLib\SSBranchFileObject.h>

class CSSObjectVisitor : public ISSObjectVisitor
{
  virtual void Apply(const SSVersionObject& object, const ISSContext* pCtx)       { Apply ((SSObject&) object, pCtx); }
  virtual void Apply(const SSCheckOutObject& object, const ISSContext* pCtx)      { Apply ((SSObject&) object, pCtx); }
  virtual void Apply(const SSNameObject& object, const ISSContext* pCtx)          { Apply ((SSObject&) object, pCtx); }
  virtual void Apply(const SSCommentObject& object, const ISSContext* pCtx)       { Apply ((SSObject&) object, pCtx); }
  virtual void Apply(const SSProjectObject& object, const ISSContext* pCtx)       { Apply ((SSObject&) object, pCtx); }
  virtual void Apply(const SSParentFolderObject& object, const ISSContext* pCtx)  { Apply ((SSObject&) object, pCtx); }
  virtual void Apply(const SSBranchFileObject& object, const ISSContext* pCtx)    { Apply ((SSObject&) object, pCtx); }

  virtual void Apply(const SSFileItem& object, const ISSContext* pCtx)            { Apply ((SSItemInfoObject&) object, pCtx); }
  virtual void Apply(const SSProjectItem& object, const ISSContext* pCtx)         { Apply ((SSItemInfoObject&) object, pCtx); }

protected:
  virtual void Apply(const SSItemInfoObject& object, const ISSContext* pCtx)      { Apply ((SSObject&) object, pCtx); }
  virtual void Apply(const SSObject& object, const ISSContext* pCtx) = 0;
};

/////////////////////////////////////////////////////////////////////
class CPhysFormatter : public CFormatter
{
public:
  void Format (const SSObject& object, const ISSContext* pCtx)
  {
    object.Dump (std::cout);
  }

protected:

};

//////////////////////////////////////////////////////////////////////
class CXMLFormatter : public CFormatter
{
public:
  CXMLFormatter ()
    : m_pCurrentFileNode (NULL)
  {
    TiXmlDeclaration decl ("1.0", "windows-1252", "");
    m_Document.InsertEndChild (decl);
  }
  ~CXMLFormatter ()
  {
    m_Document.Print ();
  }
  void BeginFile (std::string fileName)
  {
    TiXmlElement fileNode ("File");
    fileNode.SetAttribute("Name", fileName);
    m_pCurrentFileNode = m_Document.InsertEndChild (fileNode)->ToElement();
  }
  void EndFile ()
  {
    m_pCurrentFileNode = NULL;
  }

  void Format (const SSObject& object, const ISSContext* pCtx)
  {
    AttribMap map;
    map ["offset"] = boost::lexical_cast<std::string>(object.GetOffset ());
    XMLNode node (NULL, object.GetTypeName (), map);
    object.ToXml (&node);
    
    if (m_pCurrentFileNode)
    {
      m_pCurrentFileNode->InsertEndChild(node.m_Node);
    }
  }

protected:
  TiXmlDocument m_Document;
  TiXmlElement* m_pCurrentFileNode;
};

//////////////////////////////////////////////////////////////////////
void hexdump( const unsigned char *buffer, int size )
{
  int i = 0;

  while( i < size )
  {
    int j = 0;
    char str[256];
    char* p = str;

    for( j = 0; j < 16 && i + j < size; j++ )
      p += sprintf (p, "%02x ", buffer[i + j]);

    //if not entire line
    for( ; j < 16; j++ )
      p += sprintf (p, "   " );

    p += sprintf (p, " | " );

    for( j = 0; j < 16 && i + j < size; j++ )
      p += sprintf (p, "%c", buffer[i + j] < ' ' || buffer[i + j] >= 127 ? '.' : buffer[i + j] );

    //filler
    for( ; j < 16; j++ )
      p += sprintf (p, " " );

    p += sprintf (p, " |\n" );

    i += 16;

    std::cout << str;
  }
}


class CBinaryFormatter : public CFormatter
{
public:
  CBinaryFormatter (/*tristate value*/)
/*    : m_Value (value)*/
  {
  }

  void Format (const SSObject& object, const ISSContext* pCtx)
  {
    const SSRecordPtr pRecord = object.GetRecord ();
    std::cout << "Offset: " << pRecord->GetOffset ();
    std::cout << " Type: "  << SSRecord::TypeToString(pRecord->GetType());
    std::cout << " Len: "   << pRecord->GetLen();

/*    if (m_Value == set) */
    {
      std::cout << std::endl;
      hexdump (object.GetRecord()->GetBuffer(), object.GetRecord()->GetLen());
    }
  }

private:
/*  tristate m_Value;*/
};


//////////////////////////////////////////////////////////////////////
class CVssFormatter : public CFormatter, public CSSObjectVisitor
{
public:
  CVssFormatter ()
    : n_bPhysicalID (true)
  {
  }
  virtual void Apply (const SSVersionObject& object, const ISSContext* pCtx);
  virtual void Apply (const SSFileItem& object, const ISSContext* pCtx);
  virtual void Apply (const SSProjectItem& object, const ISSContext* pCtx);
  virtual void Apply (const SSProjectObject& object, const ISSContext* pCtx);
  virtual void Apply (const SSCommentObject& object, const ISSContext* pCtx);
  virtual void Apply (const SSNameObject& object, const ISSContext* pCtx);

  void Format (const SSObject& object, const ISSContext* pCtx)
  {
    object.Accept (*this, pCtx);
  }
protected:
  virtual void Apply(const class SSObject &object, const ISSContext* pCtx) 
  {
    std::cout << "not implemented" << std::endl;
  }

  bool n_bPhysicalID;
};

void CVssFormatter::Apply (const SSVersionObject& rObject, const ISSContext* pCtx)
{
  // *****************  Version 1   *****************
  // User: Admin        Date: 21.11.94   Time: 18:59
  // Created    
  const char* format1 = "*****************  Version %d   *****************";
  const char* format2 = "User: %-12s Date: %-8s   Time: %s";
  
  char line1[60]; _snprintf (line1, 60, format1, rObject.GetVersionNumber ());
  char date[12];
  char time[12];
  __time32_t versionDate = rObject.GetDate ();
  const tm* ttm = _gmtime32 (&versionDate);//localtime (&versionDate);
  strftime (date, countof (date), "%x", ttm);
  strftime (time, countof (time), "%X", ttm);
  char line2[60]; _snprintf (line2, 60, format2, rObject.GetUsername ().c_str (), date, time);

  std::cout << line1 << std::endl;
  if (rObject.GetActionID() == Labeled)
    std::cout << "Label: \"" << rObject.GetLabel() << "\"" << std::endl;
  std::cout << line2 << std::endl;
  std::cout << rObject.GetActionString() << std::endl;

  if (n_bPhysicalID)
  {
    ISSItemAction* pAction = dynamic_cast<ISSItemAction*> (rObject.GetAction());
    if (pAction)
      std::cout << "Physical: " << pAction->GetPhysical() << std::endl;
  }
  
  if (!rObject.GetComment ().empty())
    std::cout << rObject.GetComment () << std::endl;
  if (rObject.GetActionID() == Labeled)
    std::cout << "Label Comment: " << rObject.GetLabelComment () << std::endl;

  std::cout << std::endl;
}

void CVssFormatter::Apply (const SSFileItem& object, const ISSContext* pCtx)
{
  const IFormattingContext* pFormattingContext = dynamic_cast<const IFormattingContext*> (pCtx);
  if (pFormattingContext && pFormattingContext->GetExtendedOutput ())
  {
    //File:  $/Modules/bin/z.dll
    //Type:  Binary
    //Size:  92160 bytes
    //Store only latest version:  No
    //Latest:                        Last Label:  1.9.3
    //  Version:  1                     Version:  178
    //  Date:     8.11.02    17:03      Date:     15.11.04   17:13
    //Comment: doxygen and graphviz

    // TODO: output the parent, but which?
    std::cout << "File:  " << object.GetName () << std::endl;
    std::string type = object.GetFileType () == eFileTypeBinary ? "Binary" : "Text";
    std::cout << "Type:  " << type << std::endl;
    // TODO: calculate the size, is it in the record??
    std::cout << "Size:  " << "???" << std::endl;
    // TODO: 
    std::cout << "Store only latest version:  " << "???" << std::endl;
    std::cout << "Latest:                        Last Label:  " << "???" << std::endl;
//    std::cout << "  Version:  " << object.GetVersion () << "                     Version:  ???" << std::endl;
//    std::cout << "  Date:     " << object.GetDate ()    << "                     Date:     ???" << std::endl;
  }
  else
  {
    std::string type = object.GetFileType () == eFileTypeBinary ? "Binary" : "Text";
    std::cout << object.GetName () << "\t" << type << std::endl;
  }
}

void CVssFormatter::Apply (const SSProjectItem& object, const ISSContext* pCtx)
{
  const IFormattingContext* pFormattingContext = dynamic_cast<const IFormattingContext*> (pCtx);
  if (pFormattingContext && pFormattingContext->GetExtendedOutput ())
  {
    //Project:  $/Modules/bin
    //Contains:
    //   98 Files              ( +2 deleted )
    //    0 Subproject(s)
    //Latest:                        Last Label:  1.9.3
    //  Version:  113                   Version:  178
    //  Date:     8.10.04    16:10      Date:     15.11.04   17:13
    //Comment:
  }
  else
  {
    std::cout << "$" << object.GetName () << "\tProject" << std::endl;
  }
  
}

void CVssFormatter::Apply(const SSProjectObject& object, const ISSContext* pCtx)
{
  std::string type;
  if (object.GetType () == SSITEM_FILE)
    type = object.GetFileType () == eFileTypeBinary ? "Binary" : "Text";
  else
    type = "Project";
  
  std::cout << object.GetName () << "\t" << type << std::endl;
}

void CVssFormatter::Apply (const SSCommentObject& object, const ISSContext* pCtx)
{
  std::cout << object.GetComment () << std::endl;
}

void CVssFormatter::Apply (const SSNameObject& object, const ISSContext* pCtx)
{
  std::cout << "Entries: " << object.size () << std::endl;

  SSNameObject::const_iterator iter = object.begin ();
  for (; iter != object.end (); ++iter)
  {
    std::cout << "id(" << (*iter).first  << ") "/*, offset (" << pEntry->offset << ")*/ "= " << (*iter).second <<std::endl;
  }
}

//////////////////////////////////////////////////////////////////////
std::auto_ptr<CFormatter> CFormatterFactory::MakeFormatter (eStyle style, po::variables_map const& vm)
{
  if (style == eBinary)
    return std::auto_ptr<CFormatter> (new CBinaryFormatter (/*value*/));
  if (style == eXML)
    return std::auto_ptr<CFormatter> (new CXMLFormatter ());
  if (style == eVSS)
    return std::auto_ptr<CFormatter> (new CVssFormatter ());
  if (style == eDump)
    return std::auto_ptr<CFormatter> (new CPhysFormatter ());

  return std::auto_ptr<CFormatter> (NULL);
}

std::auto_ptr<CFormatter> CFormatterFactory::MakeFormatter (po::variables_map const& options)
{
  std::string style = options["style"].as<std::string>();
  for (size_t i = 0; i < style.size(); ++i)
    style[i] = char(tolower(style[i]));

  if (style == "binary")
    return std::auto_ptr<CFormatter> (new CBinaryFormatter (/*value*/));
  if (style == "xml")
    return std::auto_ptr<CFormatter> (new CXMLFormatter ());
  if (style == "vss")
    return std::auto_ptr<CFormatter> (new CVssFormatter ());
  if (style == "dump")
    return std::auto_ptr<CFormatter> (new CPhysFormatter ());

  throw SSException (std::string("invalid Formatter ").append (style));
  return std::auto_ptr<CFormatter> (NULL);
}

po::options_description CFormatterFactory::GetOptionsDescription ()
{
  po::options_description descr ("Formatter options");
  descr.add_options ()
    ("style,s", po::value<std::string>()->default_value("XML"), "output style {XML|binary|vss|dump}");
  return descr;
}

