// GetCommand.cpp: implementation of the GetCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GetCommand.h"
#include <SSPhysLib\SSFiles.h>
#include <SSPhysLib\SSItemInfoObject.h>
#include <SSPhysLib\SSVersionObject.h>
#include <SSPhysLib\FileName.h>
#include <SSPhysLib\SSProjectObject.h>
#include <io.h>
#include <fcntl.h>
#include <fstream>
#include <strstream>

#include "windows.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
bool fexists (const char* file)
{
  FILE* pFile = fopen (file, "r");
  if (pFile)
  {
    fclose (pFile);
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
CGetCommand::CGetCommand ()
  : CCommand ("get"),
    m_Version (-1),
    m_bForceOverwrite (false),
    m_bBulkGet (false)
{
}

COptionInfoList CGetCommand::GetOptionsInfo () const
{
  COptionInfoList options = CCommand::GetOptionsInfo();
  options.push_back (COptionInfo ('v', 'v', "version", "version to get", COptionInfo::requiredArgument));
  options.push_back (COptionInfo ('f', 'f', "force-overwrite", "overwrite target file", COptionInfo::noArgument));
  options.push_back (COptionInfo ('b', 'b', "bulk", "bulk operation: get all intermediate files with the same name as the source name", COptionInfo::noArgument));
  return options;
}

bool CGetCommand::SetOption (const COption& option)
{
  switch (option.id)
  {
  case 'v':
    m_Version = atoi ((const char*) option.value);
    break;
  case 'f':
    m_bForceOverwrite = true;
    break;
  case 'b':
    m_bBulkGet = true;
    break;
  default:
    return false;
  }
  return true;
}

bool CGetCommand::SetArguments (CArguments& args)
{
  if (args.empty ())
    throw SSException ("no argument");
  
  m_PhysFile = args.front ();
  args.pop ();

  if (args.empty ())
    throw SSException ("missing argument");
  
  m_DestFile = args.front ();
  args.pop ();
  
  return true;
  
}


class CActionVisitor : public ISSActionVisitor
{
public:
  virtual ~CActionVisitor () {}

  virtual bool Apply (const SSLabeledAction& rAction)           { return Apply ((const SSAction&) rAction); }
//  virtual bool Apply (const SSSingleFileAction& rAction)      { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSCreatedProjectAction& rAction)    { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSCreatedFileAction& rAction)       { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSAddedProjectAction& rAction)      { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSAddedFileAction& rAction)         { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSDeletedProjectAction& rAction)    { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSDeletedFileAction& rAction)       { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSRecoveredProjectAction& rAction)  { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSRecoveredFileAction& rAction)     { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSBranchFileAction& rAction)        { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSRollbackAction& rAction)          { return Apply ((const SSAction&) rAction); }

  virtual bool Apply (const SSDestroyedProjectAction& rAction)  { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSDestroyedFileAction& rAction)     { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSRenamedProjectAction& rAction)    { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSRenamedFileAction& rAction)       { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSCheckedInAction& rAction)         { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSSharedAction& rAction)            { return Apply ((const SSAction&) rAction); }

protected:
  virtual bool Apply (const SSAction& rAction) 
  { 
    return true;
  }
};


std::string CreateTempFile ( const char* pzPrefix = NULL, const char* pzPath = NULL)
{
  return tempnam( pzPath, pzPrefix );
//  while( true ) 
  {
//    std::string cPath = tempnam( pzPath, pzPrefix );
//	  if (Open (cPath, O_RDWR | O_CREAT | O_EXCL ) < 0) 
//    {
//	    if ( errno == EEXIST ) 
//      {
//		    continue;
//	    } 
//      else 
//      {
////		    throw errno_exception( "Failed to create file" );
//	    }
//	  } 
//    else 
//    {
//	    Close ();
//      break;
//	  }
  }
}

class CAutoFile 
{
public:
  CAutoFile (std::string name, bool bDelete = true)
    : m_FileName (name),
      m_bDeleteFile (bDelete)
  {
  }

  CAutoFile (CAutoFile const & r)
    : m_bDeleteFile (r.m_bDeleteFile),
      m_FileName (r.Detatch ())
  {
  }

  CAutoFile& operator= (CAutoFile const & r)
  {
    if (this != &r)
    {
      if (m_FileName != r.GetPath())
      {
        if (m_bDeleteFile)
          Unlink ();
        m_bDeleteFile = r.m_bDeleteFile;
      }
      else if (r.m_bDeleteFile)
        m_bDeleteFile = r.m_bDeleteFile;
      m_FileName = r.Detatch();
    }
    return (*this);
  }

  ~CAutoFile()
  {
    if (m_bDeleteFile)
      Unlink ();
  }

  std::string     GetPath () const  { return m_FileName; }

  std::string Detatch() const;
  int Unlink();

protected:
 
  std::string m_FileName;
  mutable bool m_bDeleteFile;
};

std::string CAutoFile::Detatch() const
{
  m_bDeleteFile = false;
  return m_FileName;
}

int CAutoFile::Unlink()
{
  if ( m_bDeleteFile ) 
  {
	  m_bDeleteFile = false;
	  return( unlink( m_FileName.c_str () ) );
  }
  else 
  {
	  return( 0 );
  }
}



class CReverseDelta 
{
public:
  CReverseDelta (const char* buffer, size_t length)
    : m_pBuffer (buffer), m_length (length)
  {
  }
  
  bool operator () (std::istream& input, std::ostream& output) const
  {
    for (long i = 0; i < m_length; )
    {
      const FD* pfd = (const FD*) (m_pBuffer+i);
      i += sizeof(FD);
//      printf ("fd: %d, start %d, len %d\n", pfd->command, pfd->start, pfd->end);

      switch (pfd->command)
      {
      case 2:
        // assert finito
        break;
      case 1:
        {
          char b[256];
          long size = pfd->end;
          input.seekg(pfd->start);
          if (input.fail ())
            throw SSException ("reverse delta: invalid seek beyond file size");

          while (size > 0)
          {
            long s = __min (size, sizeof (b));
            input.read (b, s);
            output.write (b, s);
            size -= s;
          }
        }
        break;
      case 0:
        output.write (m_pBuffer+i, pfd->end);
        i += pfd->end;
        break;
      default:
        std::strstream msg; 
        msg << "unknown reverse delta command " << pfd->command;
        throw SSException (msg.str());
        break;
      }
    }

    return true;
  }
protected:
  const char* m_pBuffer;
  size_t m_length;
};


class CHistoryHandler : public CActionVisitor
{
public:
  CHistoryHandler (std::string src)
    : m_File (src, false)
  {
  }

  std::string GetPath ()
  { 
    return m_File.GetPath();
  }

  virtual bool SaveAs (std::string name, bool overwrite = false) = 0;

protected:
  CAutoFile m_File;

};

class CReverseHistoryHandler : public CHistoryHandler
{
public:
  CReverseHistoryHandler (std::string src)
    : CHistoryHandler (src)
  {
  }

  virtual bool Apply (const SSCheckedInAction& rAction)
  {
    SSRecordPtr pRecord = rAction.GetFileDelta();
    CAutoFile targetFile (CreateTempFile());

    std::ifstream input (m_File.GetPath().c_str(), std::ios::in|std::ios::binary);
    if (!input.is_open())
      return false;
    std::ofstream output (targetFile.GetPath().c_str(), std::ios::out|std::ios::binary);
    if (!output.is_open())
      return false;

    CReverseDelta revDelta ((const char*)pRecord->GetBuffer(), pRecord->GetLen());
    bool ret = revDelta (input, output);

    input.close();
    output.close ();

    if (ret)
    {
      m_File = targetFile;
    }
    return ret;
  }

  virtual bool SaveAs (std::string name, bool overwrite = false)
  {
    return ::CopyFile (GetPath ().c_str(), name.c_str(), !overwrite) != 0;
  }

  virtual bool Apply (const SSAction& rAction) 
  {
    throw SSException ("unsuported action in CReverseHistoryHandler");
  }

};

class CProjectHistoryHandler : public CHistoryHandler
{
public:
  CProjectHistoryHandler (std::string src)
    : CHistoryHandler (src)
  {
    SSProjectFile projectFile (GetPath ());
    BuildList (projectFile);
  }

  virtual bool SaveAs (std::string name, bool overwrite = false)
  {
    return false;
  }

  virtual bool Apply (const SSLabeledAction& rAction)           { /* nothing to do */ return true; }
////  virtual bool Apply (const SSSingleFileAction& rAction)      { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSCreatedProjectAction& rAction)    { /* nothing to do */ return true; }
  virtual bool Apply (const SSCreatedFileAction& rAction)       { /* nothing to do */ return true; }
  virtual bool Apply (const SSAddedProjectAction& rAction);
  virtual bool Apply (const SSAddedFileAction& rAction);
  virtual bool Apply (const SSDeletedProjectAction& rAction);
  virtual bool Apply (const SSDeletedFileAction& rAction);
  virtual bool Apply (const SSRecoveredProjectAction& rAction);
  virtual bool Apply (const SSRecoveredFileAction& rAction);

  virtual bool Apply (const SSDestroyedProjectAction& rAction);
  virtual bool Apply (const SSDestroyedFileAction& rAction);
  virtual bool Apply (const SSRenamedProjectAction& rAction);
  virtual bool Apply (const SSRenamedFileAction& rAction);
//  virtual bool Apply (const SSCheckedInAction& rAction)         { return Apply ((const SSAction&) rAction); }
// TODO:
//  virtual bool Apply (const SSSharedFileAction& rAction)        { return Apply ((const SSAction&) rAction); }

protected:
  virtual bool Apply (const SSAction& rAction) 
  {
    throw SSException ("unsuported action in CProjectHistoryHandler");
  }

  typedef std::vector<SSProjectObject>::iterator iterator;
  std::vector<SSProjectObject> m_Items;

  void BuildList (SSProjectFile& rFile);
  iterator FindItem (std::string physFile);
  iterator InsertItem (SSProjectObject object);

};


void CProjectHistoryHandler::BuildList (SSProjectFile& rFile)
{
  // iterate all records and add them to the collection
  SSRecordPtr recordPtr = rFile.GetFirstRecord ();
  while (recordPtr)
  {
    if (recordPtr->GetType() == eProjectEntry)
    {
      SSProjectObject projectObject (recordPtr);
      m_Items.push_back (projectObject);
    }

    recordPtr = rFile.GetNextRecord (recordPtr);
  }
}

CProjectHistoryHandler::iterator CProjectHistoryHandler::FindItem (std::string physFile)
{
  iterator itor;
  iterator end = m_Items.end();
  iterator found = end;

  for (itor = m_Items.begin(); itor != end; ++itor)
  {
    SSProjectObject& po = *itor;
    //    std::cout << itemPtr->GetPhysical () << std::endl;
    if (physFile == po.GetPhysFile())
    {
      if (found != end)
        throw SSException ("duplicate entry");
      
      found = itor;
    }
  }

  return found;  
}

CProjectHistoryHandler::iterator CProjectHistoryHandler::InsertItem (SSProjectObject object)
{
  m_Items.push_back (object);
  iterator last = m_Items.end();
  return --last;
}

bool CProjectHistoryHandler::Apply (const SSAddedFileAction& rAction)
{
  iterator itor = FindItem (rAction.GetPhysical());
  if (itor != m_Items.end())
    m_Items.erase(itor);
  else
    throw SSException ("item not found");

  return true;
}

bool CProjectHistoryHandler::Apply (const SSAddedProjectAction& rAction)
{
  iterator itor = FindItem (rAction.GetPhysical());
  if (itor != m_Items.end())
    m_Items.erase(itor);
  else
    throw SSException ("item not found");

  return true;
}

bool CProjectHistoryHandler::Apply (const SSDeletedFileAction& rAction)
{
  iterator itor = FindItem (rAction.GetPhysical());
  if (itor != m_Items.end ())
  {
    SSProjectObject& po = *itor;
    po.Recover ();
  }
  else
    throw SSException ("item not found");

  return true;
}
bool CProjectHistoryHandler::Apply (const SSDeletedProjectAction& rAction)
{
  iterator itor = FindItem (rAction.GetPhysical());
  if (itor != m_Items.end ())
  {
    SSProjectObject& po = *itor;
    po.Recover ();
  }
  else
    throw SSException ("item not found");

  return true;
}
bool CProjectHistoryHandler::Apply (const SSRecoveredFileAction& rAction)
{
  iterator itor = FindItem (rAction.GetPhysical());
  if (itor != m_Items.end ())
  {
    SSProjectObject& po = *itor;
    po.Delete ();
  }
  else
    throw SSException ("item not found");

  return true;
}
bool CProjectHistoryHandler::Apply (const SSRecoveredProjectAction& rAction)
{
  iterator itor = FindItem (rAction.GetPhysical());
  if (itor != m_Items.end ())
  {
    SSProjectObject& po = *itor;
    po.Delete ();
  }
  else
    throw SSException ("item not found");

  return true;
}

bool CProjectHistoryHandler::Apply (const SSDestroyedProjectAction& rAction)
{
  if (FindItem (rAction.GetPhysical ()) != m_Items.end ())
    throw SSException ("adding already existing item");

  PROJECT_ENTRY pe;
// pe.flags = ??;
  pe.name = rAction.GetSSName ();
  strncpy (pe.phys, rAction.GetPhysical().c_str(), 8);
  pe.phys[8] = '\0';
  pe.pinnedToVersion = 0;
// pe.type = ??
  SSProjectObject pr (pe);
  InsertItem (pr);

  return true;
}

bool CProjectHistoryHandler::Apply (const SSDestroyedFileAction& rAction)
{
  // durch map wahrscheinlich besser zu lösen
  if (FindItem (rAction.GetPhysical()) != m_Items.end ())
    throw SSException ("adding already existing item");

  PROJECT_ENTRY pe;
// pe.flags = ??;
  pe.name = rAction.GetSSName ();
  strncpy (pe.phys, rAction.GetPhysical().c_str(), 8);
  pe.phys[8] = '\0';
  pe.pinnedToVersion = 0;
// pe.type = ??
  SSProjectObject pr (pe);
  InsertItem (pr);

  return true;
}

bool CProjectHistoryHandler::Apply (const SSRenamedProjectAction& rAction)
{
  iterator itor = FindItem (rAction.GetPhysical());
  if (itor == m_Items.end ())
    throw SSException ("item not found");

  SSProjectObject& po = *itor;
  po.Rename (rAction.GetNewSSName (), rAction.GetSSName());
  return true;
}

bool CProjectHistoryHandler::Apply (const SSRenamedFileAction& rAction)
{
  iterator itor = FindItem (rAction.GetPhysical());
  if (itor == m_Items.end ())
    throw SSException ("item not found");

  SSProjectObject& po = *itor;
  po.Rename (rAction.GetNewSSName (), rAction.GetSSName());
  return true;
}






void CGetCommand::Execute ()
{
  if (m_DestFile.empty ())
    throw SSException ("please specify a destination file for the get operation");

  if (fexists (m_DestFile.c_str ()) && !m_bForceOverwrite)
    throw SSException ("destination file exists. Please use overwrite flag");

  SSHistoryFile file(m_PhysFile);
  std::auto_ptr<SSItemInfoObject> pItem (file.GetItemInfo());
  if (!pItem.get ())
    throw SSException ("no information object found");

  std::auto_ptr<CHistoryHandler> pVisitor;
  std::string lastDataFileName = pItem->GetDataFileName ();
  
  if (pItem->GetType() == SSITEM_FILE)
    pVisitor = std::auto_ptr<CHistoryHandler> (new CReverseHistoryHandler (lastDataFileName));
  else
    pVisitor = std::auto_ptr<CHistoryHandler> (new CProjectHistoryHandler (lastDataFileName));


  SSVersionObject version (pItem->GetHistoryLast ());
  while (version && version.GetVersionNumber() > m_Version)
  {
    if (version.GetAction())
    {
      version.GetAction ()->Accept (*pVisitor.get());
      if (m_bBulkGet)
      {
        char buffer[66];
        CFileName fname (m_PhysFile.c_str ());
        fname.SetExt (itoa (version.GetVersionNumber (), buffer, 10));

        if (!pVisitor->SaveAs (fname.GetFileName ().c_str(), false))
          throw SSException ("failed to create target file" + fname.GetFileName ());
      }
    }

    version = version.GetPreviousObject ();
  }

  if (!pVisitor->SaveAs (m_DestFile.c_str(), m_bForceOverwrite))
    throw SSException ("failed to create target file");
}

