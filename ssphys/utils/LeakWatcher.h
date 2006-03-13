//---------------------------------------------------------------------------
// LeakWatcher.h

#ifndef IMWATCHINGYOULEAK
#define IMWATCHINGYOULEAK

#ifdef _DEBUG

#include <crtdbg.h>

inline void* operator new(size_t nSize, const char * lpszFileName, int nLine)
{
    return ::operator new(nSize, 1, lpszFileName, nLine);
}
#define DEBUG_NEW new(THIS_FILE, __LINE__)

inline void operator delete(void* pMem, const char* pszFilename, int nLine)
{ 
  ::operator delete(pMem); 
}

#define MALLOC_DBG(x) _malloc_dbg(x, 1, THIS_FILE, __LINE__);
#define malloc(x) MALLOC_DBG(x)

#endif // _DEBUG

#endif // #include guard
