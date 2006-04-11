// GetCommand.cpp: implementation of the GetCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GetCommand.h"
#include <SSPhysLib/SSFiles.h>
#include <SSPhysLib/SSItemInfoObject.h>
#include <SSPhysLib/SSVersionObject.h>
#include <SSPhysLib/SSProjectObject.h>
#include <boost/integer/static_min_max.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp> // create_directories
#include <boost/filesystem/exception.hpp>
#include <fcntl.h>
#include <fstream>
#include <strstream>
#include <sys/stat.h>
#include <errno.h>
using namespace boost::filesystem;
namespace fs = boost::filesystem;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGetCommand::CGetCommand ()
  : CCommand ("get", "Retrieve old versions from a VSS physical file"),
    m_Version (-1),
    m_bForceOverwrite (false),
    m_bBulkGet (false)
{
}

po::options_description CGetCommand::GetOptionsDescription () const
{
  po::options_description descr (CCommand::GetOptionsDescription());
  descr.add_options ()
    ("version,v", po::value<int>(), "Get a specific old version")
    ("force-overwrite", "overwrite target file")
    ("bulk,b", "bulk operation: get all intermediate files with the same name as the source name")
    ("projects,p", "Experimental: also try to rebuild project files")
    ("input", po::value<std::string>(), "input physical file name")
    ("output", po::value<std::string>(), "target file name.\n"
                                         "\n"
                                         "You can also specify an oputput directory for the output target."
                                         "In that case the name of the input file will be used "
                                         "as the output file name. With this option, you can easily "
                                         "build a shadow directory of your data, e.g. with the following command\n"
                                         "\n"
                                         "  find data -name ???????? | xargs -n 1 ssphys get -b -v 1 -s 1 --output shadowdir/ \n"
                                         "\n"
                                         "If you specify a relative or absolute path to the physical file, all non "
                                         "directory elements will be appended to the output directory, e.g.\n"
                                         "\n"
                                         "  ssphys get data/b/baaaaaaa shadow/ \n"
                                         "\n"
                                         "will output all files to \"shadow/data/b/baaaaaaa\". You can control the number "
                                         "of directories appended with the --strip option" )
    ("strip", po::value<int> (), "Strip the smallest prefix containing num leading slashes from the input path "
                                   "A sequence of one or more adjacent slashes is counted as a single slash, e.g\n"
                                   "\n"
                                   "/path/to/soursafe/archive/data/a/abaaaaaa\n"
                                   "\n"
                                   "setting --strip 0 gives the entire file name unmodified, --strip 1 gives\n"
                                   "\n"
                                   "path/to/soursafe/archive/data/a/abaaaaaa\n"
                                   "\n"
                                   "without the leading slash, --strip 6 gives\n"
                                   "\n"
                                   "a/abaaaaaa\n"
                                   "\n"
                                   "and  not specifying --strip at all just gives you abaaaaaa.")
     ;
  return descr;
}

po::options_description CGetCommand::GetHiddenDescription () const
{
  po::options_description descr (CCommand::GetHiddenDescription());
  return descr;
}

po::positional_options_description CGetCommand::GetPositionalOptionsDescription () const
{
  po::positional_options_description positional (CCommand::GetPositionalOptionsDescription());
  positional.add ("input", 1);
  positional.add ("output", 2);
  return positional;
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
  virtual bool Apply (const SSRestoreAction& rAction)           { return Apply ((const SSAction&) rAction); }

  virtual bool Apply (const SSDestroyedProjectAction& rAction)  { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSDestroyedFileAction& rAction)     { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSRenamedProjectAction& rAction)    { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSRenamedFileAction& rAction)       { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSCheckedInAction& rAction)         { return Apply ((const SSAction&) rAction); }
  virtual bool Apply (const SSSharedAction& rAction)            { return Apply ((const SSAction&) rAction); }

  virtual bool Apply (const SSMovedProjectAction& rAction)      { return Apply ((const SSAction&) rAction); }

protected:
  virtual bool Apply (const SSAction& rAction) 
  { 
    return true;
  }
};

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
    for (size_t i = 0; i < m_length; )
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

          while (size > 0 && !input.fail () && !output.fail ())
          {
            long s = std::min (size, (long) sizeof (b));
            input.read (b, s);
            output.write (b, s);
            
            // how many bytes did we really read?
            s = input.gcount ();
            size -= s;
          }

          if (input.fail ())
            throw SSException ("reverse delta: failed to read necessary amount of data from input file");
          if (output.fail ())
            throw SSException ("reverse delta: failed to write necessary amount of data to the output file");
        }
        break;
      case 0:
        {
	    long s = std::min (pfd->end, (ulong)(m_length - i));
          output.write (m_pBuffer+i, s);

          if (s < pfd->end)
            throw SSException ("reverse delta: invalid patch length in delta record");

          i += s;
        }
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

  virtual void SaveAs (std::string name, bool overwrite = false) = 0;

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

  virtual bool Apply (const SSCreatedFileAction& rAction)
  {
    // create an empty file
    CAutoFile targetFile (tmpnam(NULL));
    m_File = targetFile;
    
    return true;
  }

  virtual bool Apply (const SSLabeledAction& rAction)
  {
    // nothing to do for a file labeling operation
    return true;
  }
  
  virtual bool Apply (const SSRollbackAction& rAction)
  {
    // nothing special to do for a rollback operation
    // The rollback action just marks the begin of this file
    return true;
  }

  virtual bool Apply (const SSCheckedInAction& rAction)
  {
    SSRecordPtr pRecord = rAction.GetFileDelta();
    if (!pRecord)
    {
      throw SSException ("no file delta record found for check-in action (probably item did not retained old versions of itself)");
    }

    CAutoFile targetFile (tmpnam(NULL));

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

  virtual void SaveAs (std::string name, bool overwrite = false)
  {
    fs::path fpath (name);
    if (overwrite && fs::exists (fpath))
      fs::remove (fpath);
    else if (!fs::exists (fpath.branch_path ()))
      fs::create_directories(fpath.branch_path ());
    
    fs::copy_file (GetPath (), fpath);
  }

  virtual bool Apply (const SSAction& rAction) 
  {
    throw SSException (std::string ("unsuported action in CReverseHistoryHandler: ").append (CAction::ActionToString(rAction.GetActionID())));
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

  virtual void SaveAs (std::string name, bool overwrite = false)
  {
    boost::throw_exception (std::logic_error ("get not yet implemented for project status files"));
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
//  virtual bool Apply (const SSMovedProjectAction& rAction);

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






void CGetCommand::Execute (po::variables_map const & options, std::vector<po::option> const & args)
{
  std::string physFile;
  std::string destFile;
  fs::path physPath;
  fs::path destPath;

  fs::path::default_name_check (native);

  if (options.count("version"))
    m_Version = options["version"].as<int>();
  if (options.count("force-overwrite"))
    m_bForceOverwrite = true;
  if (options.count("bulk"))
    m_bBulkGet = true;

  if (options.count("input"))
    physPath = physFile = options["input"].as<std::string>();
  if (options.count("output"))
    destPath = destFile = options["output"].as<std::string> ();



  if (physFile.empty ())
    throw SSException ("please specify a source file for the get operation");

  if (destFile.empty ())
    throw SSException ("please specify a destination file for the get operation");

  if (fs::exists (destPath) && fs::is_directory(destPath) || *destFile.rbegin() == '\\' || *destFile.rbegin() == '/')
  {
    fs::path::iterator itor = physPath.begin ();
    int strip = options.count ("strip") ? options["strip"].as<int> () : 0;

    while (strip > 0 && itor != physPath.end ())
    {
      ++itor; 
      --strip;
    }

    fs::path relPath;
    if (itor == physPath.end ())
      relPath = physPath.leaf ();
    else
    {
      while (itor != physPath.end ())
      {
        relPath /= *itor;
        ++itor;
      }
    }

    destPath = destFile / relPath;
  }

  if (fs::exists (destPath) && !m_bForceOverwrite)
    throw SSException ("destination file exists. Please use overwrite flag");


  SSHistoryFile file(physPath.string ());
  std::auto_ptr<SSItemInfoObject> pItem (file.GetItemInfo());
  if (!pItem.get ())
    throw SSException ("no information object found");

  std::auto_ptr<CHistoryHandler> pVisitor;
  std::string lastDataFileName = pItem->GetDataFileName ();
  
  if (pItem->GetType() == SSITEM_FILE)
    pVisitor = std::auto_ptr<CHistoryHandler> (new CReverseHistoryHandler (lastDataFileName));
  else if (options.count("projects"))
    pVisitor = std::auto_ptr<CHistoryHandler> (new CProjectHistoryHandler (lastDataFileName));
  else
    return;

  if (m_Version < 0)
    m_Version = pItem->GetNumberOfActions () - m_Version + 1;

  SSFileItem* pFileItem = dynamic_cast<SSFileItem*> (pItem.get());

  // This check isn't necessary, since the get code will fail, if not delta information is available
  // The code was only here to give a better error message. But in case, that the flag was added after
  // the last commit, it is possible, that parts of the file are still recoverable. Therefore I disabled
  // this code
  /*
  if (pFileItem && pFileItem->GetStoreOnlyLatestRev ())
  {
    if (m_Version < pItem->GetNumberOfActions ())
    {
      throw SSException ("item does not retain old versions of itself");
    }
  }
  */

  // we need to initialize the tmpnam once. The first file generated by tmpnam has a trailing dot, but no
  // extension, e.g "sesc.". And this is an invalid name for the boost::filesystem library
  CAutoFile tmpfile (tmpnam(NULL));


  SSVersionObject version (pItem->GetHistoryLast ());
  while (version && version.GetVersionNumber() > m_Version)
  {
    if (version.GetAction())
    {
      if (m_bBulkGet)
      {
        std::string bulkFile (destPath.string () + "." + boost::lexical_cast<std::string>(version.GetVersionNumber ()));

        pVisitor->SaveAs (bulkFile, m_bForceOverwrite);
//          throw SSException ("failed to create target file " + bulkFile);
      }

      version.GetAction ()->Accept (*pVisitor.get());
    }

    version = version.GetPreviousObject ();
  }

  if (m_bBulkGet)
    destPath = destPath.string () + "." + boost::lexical_cast<std::string>(version ? version.GetVersionNumber () : 0);

  pVisitor->SaveAs (destPath.string (), m_bForceOverwrite);
//    throw SSException (std::string ("failed to create target file ").append (m_DestFile));
}

