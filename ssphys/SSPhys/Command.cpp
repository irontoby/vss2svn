// Command.cpp: implementation of the CCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Command.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommand::CCommand (std::string commandName, std::string description)
  : m_CommandName (commandName),
    m_CommandDescription (description)
{
}

CCommand::~CCommand ()
{
}

po::options_description CCommand::GetOptionsDescription () const
{
  return po::options_description ("Command options");
}
po::options_description CCommand::GetHiddenDescription () const
{
  return po::options_description ("Hidden command options");
}

po::positional_options_description CCommand::GetPositionalOptionsDescription () const
{ 
  return po::positional_options_description (); 
}


void CCommand::PrintUsage () const
{
  po::options_description descr (GetOptionsDescription ());
  descr.add (CFormatterFactory::GetOptionsDescription());
  
  std::cout << descr << std::endl;
}

int CCommand::Execute (std::vector <std::string> const& args)
{
  po::options_description descr (GetOptionsDescription ());
  descr.add (GetHiddenDescription ());
  descr.add (CFormatterFactory::GetOptionsDescription());

  po::parsed_options opts = po::command_line_parser(args)
    .options(descr)
    .positional(GetPositionalOptionsDescription ())
    .extra_parser(cmd_line_utils::vss_option_parser (descr))
    .run();
  po::store (opts, m_VariablesMap);
  po::notify(m_VariablesMap);   

  // extract the arguments from the parsed command line
  std::vector<po::option> arguments;
  std::remove_copy_if(opts.options.begin(), opts.options.end(), 
    std::back_inserter(arguments), cmd_line_utils::is_argument());

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

  Execute(m_VariablesMap, arguments);

  return 0;
}

std::auto_ptr<CFormatter>& CCommand::GetFormatter ()
{
  if (!m_pFormatter.get ())
    m_pFormatter = CFormatterFactory::MakeFormatter (m_VariablesMap);    
  return m_pFormatter;
}

//////////////////////////////////////////////////////////////////////
CMultiArgCommand::CMultiArgCommand (std::string commandName, std::string description)
: CCommand (commandName, description)
{
}

po::options_description CMultiArgCommand::GetHiddenDescription () const
{
  po::options_description descr (CCommand::GetHiddenDescription());
  descr.add_options ()
    ("input", po::value <std::vector <std::string> > (), "input file");
  return descr;
}

po::positional_options_description CMultiArgCommand::GetPositionalOptionsDescription () const
{
  po::positional_options_description positional (CCommand::GetPositionalOptionsDescription());
  positional.add ("input", -1);
  return positional;
}

void CMultiArgCommand::Execute (po::variables_map const& options, std::vector<po::option> const& args)
{
  if (options.count("input") > 0)
  {
    std::vector <std::string> const& args = options["input"].as <std::vector<std::string> > ();
    std::vector <std::string>::const_iterator end = args.end ();
    for (std::vector <std::string>::const_iterator citor = args.begin (); citor != end; ++citor)
      Execute (options, *citor);
  }
//  std::vector<po::option>::const_iterator end = args.end();
//  for (std::vector<po::option>::const_iterator citor = args.begin (); citor != end; ++citor)
//  {
//    Execute (options, *citor);
//  }
}
