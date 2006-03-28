// ssphys.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"
#include "../version.h"

//#include "SSTests.h"
#include "CommandLine.h"
#include "Command.h"
#include "GlobalOptions.h"
#include "Formatter.h"
#include "CommandFactory.h"

#include <boost/program_options/detail/cmdline.hpp>
using namespace boost::program_options::detail;
namespace pod = boost::program_options::detail;

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
po::options_description GetGlobalOptions ()
{
  po::options_description descr ("global options");
  descr.add_options()
    ("help,h", "produce help message")
    ("version,v", "print version string");
  
  return descr;
}


//---------------------------------------------------------------------------
void PrintUsage ()
{
  std::cout << "   ssphys [OPTIONS]" << std::endl;
  std::cout << "or ssphys command [OPTIONS] <file>" << std::endl;
  std::cout << std::endl;
  std::cout << GetGlobalOptions () << std::endl;

  CCommandFactory factory;
  factory.PrintUsage ();
}

//---------------------------------------------------------------------------
void PrintVersion ()
{
  std::cout << "ssphys " << STRFILEVER << std::endl;
}

bool HandleGlobalOptions (po::variables_map vm)
{
  if (vm.count("help")) {
    PrintUsage();
    return true;
  }
  if (vm.count("version")) {
    PrintVersion();
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
#if 0
  // TODO: set this to the correct locale
  tzset ();
  setlocale (LC_TIME, "German");
#endif

  int ret = -1;
  try 
  {
    // first argument is command, the rest are the arguments to the command
    // if no command given
    if (argc < 2)
      throw missing_command ();
    
    // if no command argument check for global options
    if (argv[1] && strlen (argv[1]) >= 1 && argv[1][0]=='-')
    {
      po::options_description descr = GetGlobalOptions ();

      po::variables_map vm;
      po::parsed_options opts = po::command_line_parser(argc, argv)
        .options(descr)
        .run();
      po::store (opts, vm);
      po::notify(vm);   

      if (HandleGlobalOptions (vm))
        return 0;
    }
      
    // otherwise get the command and execute it.
    std::string command (argv[1]);
    std::vector<std::string> arguments (argv + 2, argv+argc);
    
    CCommandFactory factory;
    std::auto_ptr <CCommand> pCommand (factory.MakeCommand (command));
    if (pCommand.get ())
      ret = pCommand->Execute(arguments);
  }
  catch (std::exception& ex)
  {
    std::cerr << argv[0] << ": " << ex.what() << std::endl;
    // std::cerr << "Try `" << argv[0] << " --help` for more information" << std::endl;
    return -1;
  }

  return ret;
}

