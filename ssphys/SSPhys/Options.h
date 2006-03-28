// Options.h: interface for the COptions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_)
#define AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <list>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

po::options_description GetGlobalOptions ();
bool HandleGlobalOptions (po::variables_map vm);

#endif // !defined(AFX_OPTIONS_H__8B59AD68_C12F_44E7_9236_3C800593E961__INCLUDED_)
