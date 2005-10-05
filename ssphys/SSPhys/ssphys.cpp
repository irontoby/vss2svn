// ssphys.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "SSTests.h"
#include "CommandLine.h"
#include "Command.h"
#include "GlobalOptions.h"

//---------------------------------------------------------------------------
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


//---------------------------------------------------------------------------
void PrintUsage ()
{
  std::cout << "ssphys v0.16:" << std::endl;
  std::cout << std::endl;
  std::cout << "usage:" << std::endl;

  CGlobalOptions globalOptions;
  std::cout << "general options: "  << std::endl;
  globalOptions.PrintUsage ();

  CCommandFactory factory;
  factory.PrintUsage ();
}

//---------------------------------------------------------------------------
extern std::auto_ptr<CFormatter> g_pFormatter (NULL);

//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  CCommandLine commandline;
  CGlobalOptions globalOptions;
  CCommandFactory factory;

  commandline.SetOptionsInfo (globalOptions.GetOptionsInfo ());
  commandline.SetCommandFactory (&factory);

  try 
  {
    commandline.Parse (argc, argv);

    COptionsList options = commandline.GetOptions ();

    globalOptions.SetOptions (options);
    if (globalOptions.GetHelp ())
    {
      PrintUsage();
      return 0;
    }

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

    g_pFormatter = CFormatterFactory::MakeFormatter (globalOptions.GetStyle (), globalOptions.m_StyleValue);
//    g_pFormatter->SetOptions (options);

    std::auto_ptr <CCommand> pCommand (factory.MakeCommand (commandline.GetCommand ()));
    if (pCommand.get ())
    {
      pCommand->SetOptions (options);

      CArguments& args= commandline.GetArgs ();
      while (!args.empty ())
      {
        pCommand->SetArguments (args);
        pCommand->Execute();
      }
    }
  
    delete g_pFormatter.release();
  }
  catch (std::exception& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
    PrintUsage ();
//    exit (ex->GetError ());
    return 1;
  }

  return 0;
  
#if 0
  if (options.GetHelp () || options.GetCommand () == "help")
  {
    exit (0);
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

//  std::auto_ptr<IFormatter> pFormatter (CPhysFormatterFactory::MakeFormatter());

  StringVector physFiles = options.GetPhysFiles ();
  StringVector::const_iterator iter = physFiles.begin ();
  
  try 
  {
    for (; iter != physFiles.end (); ++iter)
    {
      if (options.GetIntegrityCheck ())
      {
        SSHistoryFile file ((*iter).c_str ());
        // IntegrityCheck (file);
        CheckOffsetsToComment (file);
        CheckLabelValid (file);
        CheckFileSize (file);
        // TestComments (file);
      }
      else if (options.GetCommand () == "get")
        pFormatter->DoGet (*iter, options);
      else if (options.GetCommand () == "validate")
        pFormatter->DoValidate (*iter, options);
      else
        throw std::runtime_error ("unknown command line argument");
    }
  }
  catch (std::exception& ex)
  {
    std::cerr << "error: " << ex.what() << std::endl;
    options.PrintUsage ();
  }
#endif
  return 0;
}

