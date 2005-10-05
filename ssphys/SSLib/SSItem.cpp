// SSItem.cpp: implementation of the SSItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSItem.h"
#include "SSVersion.h"
#include "SSDatabase.h"
#include <SSPhysLib\SSProjectObject.h>

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/*
SSItem::SSItem()
  : m_pDatabase (NULL),
    m_nVersionNumber(0)
{
}

SSItem::SSItem(SSHistoryFile& rFile, int nVersion)
  : m_pDatabase (NULL),
    m_nVersionNumber(nVersion)
{
}

SSItem::SSItem(SSDatabase* pDb, const std::string& spec, const std::string& phys)
  : m_pDatabase (pDb),
    m_MySpec (spec),
    m_Phys (phys),
    m_nVersionNumber(0)
{
}
*/

//SSItem::SSItem(SSDatabase* pDb)
//  : m_pDatabase (pDb),
//    m_pParent (NULL),
//    // m_ItemDescr (project),
//    m_nVersionNumber(0)
//{
//  m_ItemDescr.type = SSITEM_PROJECT;
//  m_ItemDescr.flags = 0;
//  m_ItemDescr.name.flags = 0;
//  strncpy (m_ItemDescr.name.name, "$", 2);
//  m_ItemDescr.name.nsmap = 0;
//  strncpy (m_ItemDescr.phys, "AAAAAAAA", 9);
//}

SSItem::SSItem(SSDatabase* pDb, SSItemPtr pParent, SSProjectObject project, int version)
  : m_pDatabase (pDb),
    m_pParent (pParent),
    m_ItemDescr (project)
{
  m_ItemDescr.Pin (version);
}

SSItem::SSItem(SSDatabase* pDb, SSItemPtr pParent, SSProjectObject project)
  : m_pDatabase (pDb),
    m_pParent (pParent),
    m_ItemDescr (project)
{
}

SSItem::~SSItem()
{
}

bool SSItem::GetBinary ()
{
  return m_ItemDescr.IsStoreBinaryDiff ();
}

bool SSItem::GetDeleted ()
{
  return m_ItemDescr.IsDeleted ();
}

int SSItem::GetType ()
{
  return m_ItemDescr.GetType ();
}

std::string SSItem::GetSpec ()
{
  // Here is a minor oddity in the naming of the specification
  // The spec of the root project is $/, but the name of any subproject is
  // $/subproject (note without the slash at the end)

  // The encoding of the path specification leads to the small problem, that
  // a pinned version of the root project is written in the form $/;10. Chaining
  // subproject to that form of the root project specification leads to 
  // $/;10project;20. To be unambigous in this way, we need to introduce another 
  // path sperator: $/;10/project;20, even if this looks strange

  // Due to the nature ss handles the specification $;10 is valid. It is the version 10
  // of the current project. 
  std::string spec;
  SSItemPtr pParent (m_pParent);
  
  spec = GetName ();
  if (!pParent)
  {
    if (spec.size() < 2 && spec.compare(0, 2, "$/") != 0)
      throw SSException ("internal error: the root project must start with $/");
  }

  while (pParent)
  {
    std::string parentSpec = pParent->GetSpec();

    if (*parentSpec.rbegin() != '/')
      parentSpec += "/";

    spec = parentSpec + spec;
    pParent = pParent->GetParent();
  }
  
  return spec;
}

std::string SSItem::GetLocalSpec ()
{
  return "";
}

std::string SSItem::GetName ()
{
  return m_ItemDescr.GetName ();
}

long SSItem::GetVersionNumber ( )
{
  int pin = m_ItemDescr.GetPinnedToVersion();
  if (pin) 
    return pin;

//  std::auto_ptr<SSHistoryFile> pFile (m_pDatabase->GetDataFile (m_ItemDescr.GetPhysFile()));
//  if (pFile.get())
//  {
//    pFile->Open ();
//    std::auto_ptr<SSVersionObject> version (pFile->GetLastVersion());
//    return version->GetVersionNumber ();
//  }
//  else
//    throw SSException (std::string ("Could not open File: ") + pFile->GetFileName());
  
  return 0;
}

long SSItem::GetIsCheckedOut ( )
{
  return 0;
}

SSItemPtr SSItem::GetParent ()
{
  return m_pParent;   
}

SSItems* SSItem::GetItems (bool bIncludeDeleted)
{
  // valid only for project items
  if (GetType () != SSITEM_PROJECT)
    return NULL;
  
  // get the correct phys file
  std::auto_ptr<SSHistoryFile> pFile (m_pDatabase->GetDataFile (m_ItemDescr.GetPhysFile()));
  if (pFile.get())
  {
    pFile->Open ();
    SSProjectFile projectFile (pFile->GetFileName () + pFile->GetLatestExt ());
    SSItems* pItems = new SSItems (shared_from_this(), projectFile, true);

    // reverse apply the history
    SSVersionObject version = pFile->GetLastVersion();
    int myVersion = GetVersionNumber ();
    while (version && ( (myVersion > 0 && version.GetVersionNumber() > myVersion) )
/*                       || (versionPtr->GetDate() > m_pDatabase->GetVersionDate ()) */)
    {
      SSAction* pAction = version.GetAction();
      if (!pAction->Accept (*pItems))
        throw SSException("unsupported action");

      version = pFile->GetPrevVersion (version);
    }
    return pItems;
  }
  else
    throw SSException (std::string ("Could not open File: ") + pFile->GetFileName());
  
  return NULL;
}

SSItemPtr SSItem::GetVersion (std::string version)
{
  int v = atoi (version.c_str());
  return SSItemPtr (new SSItem (m_pDatabase, m_pParent, m_ItemDescr, v));
}

SSVersions* SSItem::GetVersions (long iFlags)
{
  return new SSVersions (shared_from_this(), m_ItemDescr.GetPhysFile(), iFlags);
}

std::string SSItem::GetPhysical()
{
  return m_ItemDescr.GetPhysFile();
}


void SSItem::Recover()
{
  m_ItemDescr.Recover ();  
}

void SSItem::Delete()
{
  m_ItemDescr.Delete ();  
}

void SSItem::Rename(SSNAME oldName, SSNAME newName)
{
  m_ItemDescr.Rename (oldName, newName);  
}



SSItems::~SSItems ()
{
}

SSItems::SSItems (SSItemPtr pParentItem, SSProjectFile& rFile, bool bIncludeDeleted)
  : m_pParent (pParentItem)
{
  BuildList (pParentItem, rFile, bIncludeDeleted);
}

SSItemPtr SSItems::GetItem (long i)
{
  assert (i < GetCount ());
  if (i<GetCount ())
    return m_Items[i];
  return static_cast<SSItem*>(NULL);
}

void SSItems::BuildList (SSItemPtr pParentItem, SSProjectFile& rFile, bool bIncludeDeleted)
{
  // iterate all records and add the items to the collection
  SSRecordPtr recordPtr = rFile.GetFirstRecord ();
  while (recordPtr)
  {
    if (recordPtr->GetType() == eProjectEntry)
    {
      SSProjectObject project (recordPtr);
      if (!project.IsDeleted() || bIncludeDeleted)
      {
        m_Items.push_back (SSItemPtr(new SSItem (pParentItem->GetDatabase (), pParentItem, project)));
      }
    }

    recordPtr = rFile.GetNextRecord (recordPtr);
  }
}

bool SSItems::Apply (const SSLabeledAction& rAction)
{
  return true;
}

bool SSItems::Apply (const SSCreatedProjectAction& rAction)
{
  return true;
}

bool SSItems::Apply (const SSCreatedFileAction& rAction)
{
  return true;
}

bool SSItems::Apply (const SSDestroyedFileAction& rAction)
{
  // durch map wahrscheinlich besser zu lösen
  if (GetPhysicalItem (rAction.GetPhysical()))
    throw SSException ("adding already existing item");

  PROJECT_ENTRY pe;
//          pe.flags = ??;
  pe.name = rAction.GetSSName ();
  strncpy (pe.phys, rAction.GetPhysical().c_str(), 8);
  pe.phys[8] = '\0';
  pe.pinnedToVersion = 0;
//          pe.type = ??
  SSProjectObject pr (pe);
  SSItemPtr pItem (new SSItem (m_pParent->GetDatabase(), m_pParent, pr));

  m_Items.push_back (pItem);

  return true;
}
bool SSItems::Apply (const SSDestroyedProjectAction& rAction)
{
  // durch map wahrscheinlich besser zu lösen
  if (GetPhysicalItem (rAction.GetPhysical()))
    throw SSException ("adding already existing item");

  PROJECT_ENTRY pe;
//          pe.flags = ??;
  pe.name = rAction.GetSSName ();
  strncpy (pe.phys, rAction.GetPhysical().c_str(), 8);
  pe.phys[8] = '\0';
  pe.pinnedToVersion = 0;
//          pe.type = ??
  SSProjectObject pr (pe);
  SSItemPtr pItem (new SSItem (m_pParent->GetDatabase(), m_pParent, pr));

  m_Items.push_back (pItem);

  return true;
}
bool SSItems::Apply (const SSAddedFileAction& rAction)
{
  SSItemPtr pItem = GetPhysicalItem (rAction.GetPhysical());
  std::vector<SSItemPtr>::iterator itor = std::find (m_Items.begin(), m_Items.end(), pItem);
 
  if (itor != m_Items.end())
    m_Items.erase(itor);
  else
    throw SSException ("item not found");

  return true;
}
bool SSItems::Apply (const SSAddedProjectAction& rAction)
{
  SSItemPtr pItem = GetPhysicalItem (rAction.GetPhysical());
  std::vector<SSItemPtr>::iterator itor = std::find (m_Items.begin(), m_Items.end(), pItem);
 
  if (itor != m_Items.end())
    m_Items.erase(itor);
  else
    throw SSException ("item not found");

  return true;
}

bool SSItems::Apply (const SSDeletedFileAction& rAction)
{
  SSItemPtr pItem = GetPhysicalItem (rAction.GetPhysical());
  if (pItem)
    pItem->Recover ();
  else
    throw SSException ("item not found");

  return true;
}
bool SSItems::Apply (const SSDeletedProjectAction& rAction)
{
  SSItemPtr pItem = GetPhysicalItem (rAction.GetPhysical());
  if (pItem)
    pItem->Recover ();
  else
    throw SSException ("item not found");

  return true;
}

//bool SSItems::DeleteFile (SSAction& rAction)
//{
//  SSItemPtr pItem = GetPhysicalItem (rAction.GetPhysical());
//  if (pItem)
//    pItem->Delete ();
//  else
//    throw SSException ("item not found");
//
//  return true;
//}
//bool SSItems::DeleteProject (SSAction& rAction)
//{
//  return DeleteFile (pAction);
//}

bool SSItems::Apply (const SSRenamedProjectAction& rAction) //, SSNAME oldName, SSNAME newName)
{
  SSItemPtr pItem = GetPhysicalItem (rAction.GetPhysical());
  if (!pItem)
    throw SSException ("item not found");

  pItem->Rename (rAction.GetSSName(), rAction.GetNewSSName ());
  return true;
}

bool SSItems::Apply (const SSRenamedFileAction& rAction) //, SSNAME oldName, SSNAME newName)
{
  SSItemPtr pItem = GetPhysicalItem (rAction.GetPhysical());
  if (!pItem)
    throw SSException ("item not found");

  pItem->Rename (rAction.GetSSName(), rAction.GetNewSSName ());
  return true;
}

bool SSItems::Apply (const SSCheckedInAction& rAction)
{
  return true;
}

SSItemPtr SSItems::GetPhysicalItem (std::string physical)
{
#pragma message ("speed up by using a map and no full search")
  std::vector<SSItemPtr>::iterator itor;
  std::vector<SSItemPtr>::iterator end = m_Items.end();

  SSItemPtr foundItemPtr;
  for (itor = m_Items.begin(); itor != end; ++itor)
  {
    SSItemPtr itemPtr = *itor;
//    std::cout << itemPtr->GetPhysical () << std::endl;
    std::string current = itemPtr->GetPhysical ();
    if (current == physical)
    {
      if (foundItemPtr)
        throw SSException ("duplicate entry");
      
      foundItemPtr = itemPtr;
    }
  }

  return foundItemPtr;  
}

void SSItems::Dump (std::ostream& os) const
{
  std::vector<SSItemPtr>::iterator itor;
  std::vector<SSItemPtr>::iterator end = m_Items.end();

  for (itor = m_Items.begin(); itor != end; ++itor)
  {
    oss << (*itor)->GetPhysical () << ": "  << (*itor)->GetName () << std::endl;
  }
}