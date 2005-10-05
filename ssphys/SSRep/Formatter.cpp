// Formatter.cpp: implementation of the CFormatter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Formatter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

class CVssFormatter : public IFormatter
{
public:
  virtual int DoWhoAmI     (const COptions& options);
  virtual int DoProject    (const SSDatabase& database, const COptions& options);

  virtual int DoDirectory  (const SSItemPtr pItem, const COptions& options);
  virtual int DoProperties (const SSItemPtr pItem, const COptions& options);
  virtual int DoFiletype   (const SSItemPtr pItem, const COptions& options);
  virtual int DoHistory    (const SSItemPtr pItem, const COptions& options);
};



int CVssFormatter::DoWhoAmI (const COptions& options)
{
  if (options.GetUser().empty ())
  {
    // TODO: get the current windows user name
    return 100; // unknown user 
  }
  std::cout << options.GetUser () << std::endl;

  return 0;
}


int CVssFormatter::DoProject (const SSDatabase& database, const COptions& options)
{
  std::string cp = database.GetCurrentProject();
  if (cp.empty ())
  {
    throw SSException ("could not read current project");
  }
  std::cout << "Current project is " << cp << std::endl;

  return 0;
}



int CVssFormatter::DoDirectory (SSItemPtr pItem, const COptions& options)
{
  assert (pItem);
  try
  {
    if (pItem->GetType () != SSITEM_PROJECT)
      throw SSException ("please specify a project item");

    // Header
    std::cout << pItem->GetSpec() << ":" << std::endl;

    std::auto_ptr<SSItems> pItems (pItem->GetItems (options.GetDeleted()));
    if (!pItems.get ())
    {
      std::cout << "No items found under " << pItem->GetSpec() << std::endl;
      return 1;
    }

    // filter items
    std::vector <SSItemPtr> filtered;
    for (long i = 0; i < pItems->GetCount (); ++i)
    {
      SSItemPtr pItem = pItems->GetItem(i);
      bool bInclude = true;
        
      // filter delted items
      if ( pItem->GetDeleted () && options.GetDeleted()
        || !pItem->GetDeleted() && !(options.GetOnlyDeleted() && options.GetDeleted()))
        bInclude &= true;
      else
        bInclude &= false;

      // filter file/project items
      if ( pItem->GetType() == SSITEM_FILE    && options.GetFiles()
        || pItem->GetType() == SSITEM_PROJECT && !(options.GetOnlyFiles() && options.GetFiles()))
        bInclude &= true;
      else
        bInclude &= false;

      if (bInclude)
      {
        filtered.push_back(pItem);
      }
    }

    // first print all project items
    std::vector <SSItemPtr>::iterator itor;
    std::vector <SSItemPtr>::iterator end = filtered.end ();
    for (itor = filtered.begin(); itor != end; ++itor)
    {
      SSItemPtr pItem = *itor;
      if (pItem->GetType () == SSITEM_PROJECT)
      {
        std::cout << "$" << pItem->GetName();
        if (options.GetPhysOutput())
          std::cout << "\t" << pItem->GetPhysical();
        std::cout << std::endl;
      }
    }

    // second print all file items
    for (itor = filtered.begin(); itor != end; ++itor)
    {
      SSItemPtr pItem = *itor;
      if (pItem->GetType () == SSITEM_FILE)
      {
        std::cout << pItem->GetName();
        if (options.GetExtendedOutput () && pItem->GetIsCheckedOut())
        {
          // User\tDate\Time\CheckouPath, z.B.:
          // ValidatingEdit.cpp  Mirco          4.06.04 13:07  C:\Mirco
          // ValidatingEdit.h    Mirco          4.06.04 13:07  C:\Mirco
          std::cout << "\tTODO: Display check out information";
        } else if (options.GetPhysOutput())
          std::cout << "\t" << pItem->GetPhysical();

        std::cout << std::endl;
      }
    }

    std::cout << std::endl;
    
    // recurse into all subdirectories
    if (options.GetRecursive())
    {
      for (itor = filtered.begin(); itor != end; ++itor)
      {
        SSItemPtr pItem = *itor;
        if (pItem->GetType () == SSITEM_PROJECT)
          DoDirectory (pItem, options);
      }
    }
  }
  catch (SSException& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
    return 1;
  }
  
  return 0;
}


int CVssFormatter::DoProperties (SSItemPtr pItem, const COptions& options)
{
  assert (pItem);
  try
  {
    if (pItem->GetType () == SSITEM_PROJECT)
    {
      std::auto_ptr<SSItems> pItems (pItem->GetItems (true));
      int files = 0;
      int projects = 0;
      int deleted = 0;
      for (long i = 0; i < pItems->GetCount (); ++i)
      {
        SSItemPtr pItem = pItems->GetItem(i);
        // filter delted items
        if ( pItem->GetDeleted ())
          ++deleted;
        else if ( pItem->GetType () == SSITEM_PROJECT)
          ++projects;
        else if ( pItem->GetType () == SSITEM_FILE)
          ++files;
      }

      //Project:  $/
      //Contains:
      //    2 Files              ( +30 deleted )
      //   12 Subproject(s)
      std::cout << "Project:  " << pItem->GetSpec() << std::endl;
      std::cout << "Contains:  " << std::endl;
      std::cout << files << " Files           ( +" << deleted << " deleted )" << std::endl;
      std::cout << projects << " Subproject(s)" << std::endl;
    }
    else if (pItem->GetType () == SSITEM_FILE)
    {
      //File:  $/ValidatingEdit.h
      //Type:  Text
      //Size:  624 bytes      31 lines
      //Store only latest version:  No
    }

    //Latest:                        Last Label:  1.9.3
    //  Version:  178                   Version:  178
    //  Date:     15.11.04   17:13      Date:     15.11.04   17:13

  }
  catch (SSException& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}


int CVssFormatter::DoFiletype   (const SSItemPtr pItem, const COptions& options)
{
  if (!pItem)
    return 1;
  
  try 
  {
    if (pItem->GetType () == SSITEM_PROJECT)
    {
      std::auto_ptr<SSItems> pItems (pItem->GetItems (options.GetDeleted()));
      for (long i = 0; i < pItems->GetCount (); ++i)
      {
        SSItemPtr pItem = pItems->GetItem(i);
        
        if ( pItem->GetType () == SSITEM_PROJECT)
          std::cout << "\n\n" << pItem->GetSpec () << std::endl;
        else if ( pItem->GetType () == SSITEM_FILE)
        {
          std::string binary = pItem->GetBinary() ? "Binary" : "Text";
          std::cout << pItem->GetName () << "\t" << binary << std::endl;
        }
          
      }
    }
    else if (pItem->GetType () == SSITEM_FILE)
    {
      std::string binary = pItem->GetBinary() ? "Binary" : "Text";
      std::cout << pItem->GetName () << "\t" << binary << std::endl;
    }
  }
  catch (SSException& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
    return 1;
  }
  return 0;
}


int CVssFormatter::DoHistory (const SSItemPtr pItem, const COptions& options)
{
  assert (pItem);
  try
  {
//    std::auto_ptr<SSVersions> pVersions = pItem->GetVersions(0);
//    for (long i = 0; i < pVersions->GetCount (); ++i)
//    {
//      SSVersionPtr pVersions (pVersions->GetVersion(i));
//      pVersions->ReportVssStyle
//    }
  }
  catch (SSException& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}


IFormatter* CVssFormatterFactory::MakeFormatter ()
{ 
  return new CVssFormatter (); 
}

