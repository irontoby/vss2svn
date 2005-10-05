// FileName.h: interface for the CFileName class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILENAME_H__5778DD0A_6746_4301_A8C4_B82F7CC70FC9__INCLUDED_)
#define AFX_FILENAME_H__5778DD0A_6746_4301_A8C4_B82F7CC70FC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Siehe auch CFileSpec auf http://www.codeproject.com/cpp/cfilespec.asp
//       oder CPath     auf http://www.thecodeproject.com/file/cpath.asp

#include <windows.h>
#include <tchar.h>

class CFileName 
{
public:
  CFileName (LPCTSTR pFilename = NULL)
  {
    m_szDrive[0] = m_szDir[0] = m_szFileTitle[0] = m_szExt[0] = _T('\0');

    if (pFilename)
      SetFilePath (pFilename);
  }

//  CFileName (HMODULE hModule)
//  {
//	  TCHAR szPathName[_MAX_PATH];
//	  if (::GetModuleFileName(hModule, szPathName, _MAX_PATH))
//      SetFilePath (szPathName);
//    else
//    {
//      DWORD dwError = GetLastError();
//   		TRACE (_T("CFileName(HMODULE) failed: GetLastError returned %d\n"), dwError);
//    }
//      
//  }

  // set the full path name including filename and extension (e.g.: "c:\WinNT\system32\kernel32.sys")
  void SetFilePath (LPCTSTR pFilepath)
  {
	  assert(pFilepath != NULL);
//	  assert(AfxIsValidString(pFilepath));

    _tsplitpath (pFilepath, m_szDrive, m_szDir, m_szFileTitle, m_szExt);
  }
  // return full path name including filename and extension (e.g.: "c:\WinNT\system32\kernel32.sys")
  std::string GetFilePath () const
  {
    TCHAR str[_MAX_PATH];
    _tmakepath (str, m_szDrive, m_szDir, m_szFileTitle, m_szExt);
    return str;
  }
  
  // set the drive name (e.g.: "c:")
  void SetDrive (LPCTSTR pStr)
  { 
	  assert(pStr != NULL);
//	  assert(AfxIsValidString(pStr));

    _tcsncpy (m_szDrive, pStr, _MAX_DRIVE); 
  }
//  // return the drive name (e.g.: "c:")
//  std::string GetDrive () const
//  { 
//    CString str; 
//    str.Format(_T("%.*s"), _MAX_DRIVE, m_szDrive); 
//    return str; 
//  }

  // set the directory, _including_ the trailing slash (e.g.: "WinNT\system32\")
  void SetDir (LPCTSTR pStr)
  { 
    assert (IsTrailingSlash (pStr) || _tcslen (pStr) == 0);
    _tcsncpy (m_szDir, pStr, _MAX_DIR); 
  }
//  // return the directory, including a trailing slash (e.g.: "WinNT\system32\")
//  CString GetDir () const
//  { 
//    CString str; 
//    str.Format(_T("%.*s"), _MAX_DIR, m_szDir); 
//    return str; 
//  }

  // set the file title (e.g.: "kernel32")
  void SetFileTitle (LPCTSTR pStr) 
  { 
	  assert(pStr != NULL);
//	  assert(AfxIsValidString(pStr));

    _tcsncpy (m_szFileTitle, pStr, _MAX_FNAME); 
  }
//  // return the file title (e.g.: "kernel32")
//  CString GetFileTitle () const
//  { 
//    CString str;
//    str.Format(_T("%.*s"), _MAX_FNAME, m_szFileTitle);
//    return str; 
//  }
  
  // set the extension, with or without the leading dot (e.g.: "sys" or ".sys")
  void SetExt (LPCTSTR str)       
  { 
    if (IsLeadingDot (str)) 
      _tcsncpy (m_szExt, str, _MAX_EXT);
    else
    {
      TCHAR* lpsz = m_szExt;
      _tcsncpy (lpsz, _T("."), 1);
      lpsz = _tcsinc (lpsz);
      _tcsncpy (lpsz, str, _MAX_EXT-1);
    }
  }
//  // return the extension without the leading dot (e.g.: "sys")
//  CString GetExt () const
//  { 
//    CString str; str.Format(_T("%.*s"), _MAX_EXT, m_szExt); return StripLeadingDot (str); 
//  }
//  // return the extension including the leading dot (e.g.: ".sys")
//  CString GetDotExt () const
//  { 
//    CString str; str.Format(_T("%.*s"), _MAX_EXT, m_szExt); return str; 
//  }

  // set the full path to the file, _including_ a trailing slash (e.g.: "c:\WinNT\system32\")
  void SetPath (LPCTSTR pStr)
  {
    assert (IsTrailingSlash (pStr) || _tcslen (pStr) == 0);
    _tsplitpath (pStr, m_szDrive, m_szDir, 0, 0);
  }
//  // return the full path to the file, including a trailing slash (e.g.: "c:\WinNT\system32\")
//  CString GetPath () const
//  { 
//    CString str;
//    LPTSTR p = str.GetBuffer(_MAX_PATH);
//    _tmakepath (p, m_szDrive, m_szDir, NULL, NULL);
//    str.ReleaseBuffer();
//    return str;
//  }
//  // return the relative Path to another location
//  CString GetRelativePathFrom (LPCTSTR pszFrom) const;
//  CString GetRelativePathTo (LPCTSTR pszTo) const;

//  // return this as an absolute path
//  CString GetAbsolutePath () const;
  
  // set the file title including the extension (e.g.: "kernel32.sys")
  void SetFileName (LPCTSTR pStr)  
  { 
	  assert(pStr != NULL);
//	  assert(AfxIsValidString(pStr));

    _tsplitpath (pStr, 0, 0, m_szFileTitle, m_szExt);
  }
//  // return the file title including the extension (e.g.: "kernel32.sys")
  std::string GetFileName () const
  { 
    TCHAR str[_MAX_PATH];
    _tmakepath (str, NULL, NULL, m_szFileTitle, m_szExt);
    return str;
  }

  // Accept '/' or '\\' as a path separator on all platforms.
  static BOOL IsPathSeparator(TCHAR ch)
  {
    return (ch == _T('/') || ch == _T('\\'));
  }

  // returns wether the path is a UNC path (e.g.: \\computer\c$\windows\kernel32.sys)
  BOOL IsUNCPath() const
  {
	  return IsUNCPath(m_szDir);
  }

  // returns wether the path is a UNC path (e.g.: \\computer\c$\windows\kernel32.sys)
  static BOOL IsUNCPath(LPCTSTR szPath)
  {
	  assert(szPath != NULL);
//	  assert(AfxIsValidString(szPath));

	  if (_tcslen(szPath) < 2)
		  return FALSE;
	  
	  return (szPath[0] == '\\' && szPath[1] == '\\');
  }

  BOOL IsFullPath ()
  {
    std::string path = GetFilePath();

    return(IsPathSeparator(path[0])
#if !PLATFORM_UNIX
      || (_istalpha(path[0]) && path[1] == _T(':'))
#endif  // !PLATFORM_UNIX
      );
  }

 
  // remove all unnecessary elements from path like path/to/./subpath, or path/to//subpath
  void Normalize ();

protected:
  bool IsTrailingSlash (LPCTSTR lpstr) const
  {
	  assert(lpstr != NULL);
//	  assert(AfxIsValidString(lpstr));

    int len = _tcslen (lpstr);
    if (len == 0) 
      return false;
    
    LPCTSTR lpsz = _tcsninc(lpstr, len-1);
    return (_tcsncmp (lpsz, _T("\\"), 1) == 0 || _tcsncmp (lpsz, _T("/"), 1) == 0);
  }
  
  bool IsLeadingDot (LPCTSTR pStr) const
  {
	  assert(pStr != NULL);
//	  assert(AfxIsValidString(pStr));

    return (_tcsncmp (pStr, _T("."), 1) == 0);
  }

//  CString StripLeadingDot (CString str) const
//  {
//    while (IsLeadingDot (str))
//      str = str.Right (str.GetLength() - 1);
//    return str;
//  }

  void SplitComputer ()
  {
    // If UNC name
    if ( !_tcsncmp(m_szDir, _T("\\\\"), 2))
    {
      // Get the computername
      _tcsncpy(m_szComputer, _tcsstr(m_szDir, _T("\\\\"))+2, sizeof(m_szComputer)-1 );
      _tcsnset(_tcsstr(m_szComputer, _T("\\")), 0, 1);

      // Strip the computername from the directory
      _tcsncpy(m_szDir, _tcsstr(m_szDir, _T("\\\\"))+2, sizeof(m_szDir)-1 );
      _tcsncpy(m_szDir, _tcsstr(m_szDir, _T("\\")), sizeof(m_szDir)-1 );
    }
  }


  TCHAR m_szDrive[_MAX_DRIVE];
  TCHAR m_szDir[_MAX_DIR];
  TCHAR m_szFileTitle[_MAX_FNAME];
  TCHAR m_szExt[_MAX_EXT];
  TCHAR m_szComputer[_MAX_FNAME];
};

#endif // !defined(AFX_FILENAME_H__5778DD0A_6746_4301_A8C4_B82F7CC70FC9__INCLUDED_)
