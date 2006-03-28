// CommandLine.h: interface for the CCommandLine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMANDLINE_H__094560AC_AAF6_4054_8536_4D6F421ACBCE__INCLUDED_)
#define AFX_COMMANDLINE_H__094560AC_AAF6_4054_8536_4D6F421ACBCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace cmd_line_utils 
{
  ///////////////////////////////////////////////////////////////////////////
  // predicate to extract all positional arguments from the command line
  struct is_argument 
  {
    bool operator()(po::option const &opt)
    {
      return (opt.position_key == -1) ? true : false;
    }
    bool operator()(std::string const &arg)
    {
      return (arg.size () < 1 || arg[0] != '-') ? true : false;
    }
  };

  ///////////////////////////////////////////////////////////////////////////
  // Additional command line parser which interprets '-I{+|-}' as an option
  class vss_option_parser 
  {
  public:
    vss_option_parser (po::options_description& descr)
      : m_desc (&descr)
    {
    }

    inline std::pair<std::string, std::string> 
      operator ()(std::string const& s)
    {
      if (s[0] == '-')
      {
        std::string prefix;
        if (s.size () == 2)
          prefix = "only";        // e.g. -L
        else if (s.size () == 3)
        {
          if (s[2] == '-')        // e.g. -L-
            prefix = "exclude";
          else if (s[2] == '+')   // e.g. -L+
            prefix = "include";
        }

        if (!prefix.empty ())
        {
          std::string long_option;
          char opt = tolower (s[1]);

          if (opt == 'l')
            long_option = "show-labels";
          else if (opt == 'd')
            long_option = "show-dead";

          if (!long_option.empty ())
            return std::make_pair(long_option, prefix);
        }
      }
      return std::pair<std::string, std::string>();
    }
  protected:
    po::options_description* m_desc;
  };
};

#endif // !defined(AFX_COMMANDLINE_H__094560AC_AAF6_4054_8536_4D6F421ACBCE__INCLUDED_)
