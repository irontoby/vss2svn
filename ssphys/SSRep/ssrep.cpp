// ssphys.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Options.h"
#include <SSLib\SSDatabase.h>
#include <SSPhysLib\SSItemInfoObject.h>
#include <direct.h>     // _getcwd
#include "Formatter.h"
#include "fstream"

//---------------------------------------------------------------------------
#include "LeakWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 


//---------------------------------------------------------------------------
void Debug (const std::string& message)
{
//  std::cerr << "INFO: " << message << std::endl;
}

void Info (const char* message)
{
  std::cerr << "INFO: " << message << std::endl;
}

void Notice (const char* message)
{
  std::cerr << "NOTICE: " << message << std::endl;
}

void Warning (const char* message)
{
  std::cerr << "WARNING: " << message << std::endl;
}

void Error (const char* message)
{
  std::cerr << "ERROR: " << message << std::endl;
}






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

void VssGet (const std::string& physFile, const COptions& options)
{
  try
  {
    if (options.GetDestination ().empty ())
      throw SSException ("please specify a destination for get operation");

    if (fexists (options.GetDestination ().c_str ()) && !options.GetForceOverwrite ())
      throw SSException ("destination file exists. Please use overwrite flag");


    SSHistoryFile file (physFile);
#if 0
    {
      std::auto_ptr<SSItemInfoObject> pItem (file.GetItemInfo ());
      if (pItem->GetType() == SSITEM_PROJECT)
        throw SSException ("get does not work on project files");

      std::auto_ptr<SSVersionObject> pVersion (pItem->GetVersion (options.GetVersion ()));
      if (pVersion.get())
      {
        pItem->Get (options.GetVersion (), options.GetDestination ().c_str());
      }
    }
#endif
  }
  catch (SSException& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
  }
}

void VssInfo (const std::string& physFile, const COptions& options)
{
  SSRecordFile* pFile = NULL;
  try
  {
    pFile = SSRecordFile::MakeFile (physFile);
    if (pFile)
    {
      pFile->Dump (std::cout);
      if (options.GetAllRecords ())
        pFile->DumpRecords (std::cout);
    }
  }
  catch (SSException& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
  }
  
  DELETE (pFile);
}

void VssValidate (const std::string& physFile, const COptions& options)
{
  try
  {
    std::auto_ptr<SSRecordFile> pFile (SSRecordFile::MakeFile (physFile));
    if (pFile.get())
    {
      pFile->Validate ();
    }
  }
  catch (SSException& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
  }
}

int VssCurrentProject (SSDatabase& dataBase, IFormatter* pFormatter, const COptions& options)
{
  int errorVal = 0;

  StringVector items = options.GetItems ();
  if (!items.empty())
    errorVal = dataBase.SetCurrentProject (items.front());

  if (errorVal == 0)
    return pFormatter->DoProject (dataBase, options);
  
  return errorVal;
}

int main(int argc, char* argv[])
{
  COptions options;
  options.ParseCommandLine (argc, argv);

  // redirect stdout
  std::ofstream logfile;           // log file
  std::streambuf *sb = NULL;
  if (!options.GetLogFile().empty())
  {
    logfile.open(options.GetLogFile().c_str(), std::ios_base::app | std::ios_base::out); // log file
    sb = std::cout.rdbuf();                     // save old sb
    std::cout.rdbuf(logfile.rdbuf());           // redirect
  }

  if (options.GetHelp () || options.GetCommand () == "help")
  {
    options.PrintUsage (); 
    if (sb) std::cout.rdbuf(sb);
    return (0);
  }

  // TODO: set this to the correct locale
  tzset ();
  setlocale (LC_TIME, "German");

  // Load the names cache
//  SSNamesCacheFile namesCache;
//  if (!options.GetNamesCache ().empty ())
//  {
//    try {
//      if (namesCache.Open (options.GetNamesCache ()))
//        g_NamesCache.SetFile (&namesCache);
//    }
//    catch (SSException& ex)
//    {
//      std::cerr << "names cache error: " << ex.what() << std::endl;
//    }
//  }

  int exitCode = 0;
  try 
  {

    std::string iniPath = options.GetSrcSafeIniPath();
    if (iniPath.empty ())
    {
      // grab the path form the environment
      const char* p = getenv ("SSDIR");
      if (p)
        iniPath = p;
      
      if (!iniPath.empty ())
        Debug (std::string("SSDIR=") + iniPath);
    }

    if (iniPath.empty ())
    {
      // take the current directory
      char buffer[_MAX_PATH];
      if (_getcwd (buffer, _MAX_PATH) != NULL)
        iniPath = buffer;

      if (!iniPath.empty ())
        Debug (std::string("CWD=") + iniPath);
    }
  
    SSDatabase dataBase;
    if (!dataBase.Open (iniPath, options.GetUser(), options.GetPassword()))
    {
      throw SSException (std::string ("could not open database: ") + iniPath);
    }

//    dataBase.SetVersionDate (options.GetVersionDate ());
    
    std::auto_ptr<IFormatter> pFormatter (CVssFormatterFactory::MakeFormatter());

    if (options.GetCommand () == "whoami")
      exitCode = pFormatter->DoWhoAmI (options);
    else if (options.GetCommand () == "project")
      exitCode = pFormatter->DoProject (dataBase, options);
    else if (options.GetCommand () == "cp")
      exitCode = VssCurrentProject (dataBase, pFormatter.get(), options);
    else 
    {
      // get the list of work items
      StringVector items = options.GetItems ();

      // if now items are specified at the command line, we get the current project
      if (items.empty ())
        items.push_back (std::string ("."));

      StringVector::const_iterator iter;
      for (iter = items.begin (); iter != items.end (); ++iter)
      {
        SSItemPtr pItem = dataBase.GetSSItem (*iter, false);
        if (!pItem)
          continue;

  //      SSItemPtr pItem = pItem->GetVersion (1);
  //      assert (pItem->GetVersionNumber() == 1);
//        pItem = pItem->GetVersion(options.GetVersionDate ());
//        if (!pItem)
//          continue;

        if (options.GetCommand () == "history")
          exitCode = pFormatter->DoHistory (pItem, options);
        else if (options.GetCommand () == "properties")
          exitCode = pFormatter->DoProperties (pItem, options);
        else if (options.GetCommand () == "filetype")
          exitCode = pFormatter->DoFiletype (pItem, options);
        else if (options.GetCommand () == "dir")
          exitCode = pFormatter->DoDirectory (pItem, options);
        else if (options.GetCommand () == "get")
          VssGet (*iter, options);
        else if (options.GetCommand () == "info")
          VssInfo (*iter, options);
        else if (options.GetCommand () == "validate")
          VssValidate (*iter, options);
        else
          throw std::runtime_error (std::string ("unsupported command: ") + options.GetCommand());
      }
    }
  }
  catch (std::exception& ex)
  {
    std::cout << "error: " << ex.what() << std::endl;
    options.PrintUsage ();
    exitCode = 1;
  }

  if (sb)
  {
    std::cout.rdbuf(sb);                        // restore sb
    logfile.close ();
  }
  
  return exitCode;
}

