// stdafx.cpp : source file that includes just the standard includes
//	ssphys.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#ifndef _INC_CRTDBG
#include <crtdbg.h>
#endif

#pragma warning (disable:4073) // get rid of warning for putting initializers in library initialization area
#pragma init_seg(lib)

struct crt_mem_leak_checker
{
 enum
 {
  DBG_FLAGS = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF
 };

 crt_mem_leak_checker()
 {
  _CrtSetDbgFlag(DBG_FLAGS);
 }

 ~crt_mem_leak_checker()
 {
  if (_CrtDumpMemoryLeaks())
  {
//   ::MessageBeep(MB_ICONHAND);
  }
 }

} the_checker;

#pragma warning (default:4073) 