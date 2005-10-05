// FileName.cpp: implementation of the CFileName class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileName.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CFileName::Normalize ()
{
//  int n = 0;
//  int isAbsolute = IsFullPath ();
//  int isUNCPath = IsUNCPath();
//
//  int minCount = 0;
//  if (isUNCPath)
//    minCount = 1;     // do not drop the computer during ".." resolution.
//
//  // break the path into peaces
//  CString dir (m_szDir);
//  CStringList elements;
//	do
//	{
//    CString element = SysTokenize (dir, _T("\\/"), n);
//    
//    if( n != -1 )
//		{
//			if (element == _T(".."))
//      { 
//        // drop the last directory if we are not a relative path
//        if (elements.GetCount () > minCount)
//          elements.RemoveTail ();
//        else if (!isAbsolute)
//          elements.AddTail (element);
//      }
//      else if (element != _T("."))
//        elements.AddTail (element);
//		}
//	} while( n != -1 );
//  
//  // resamble the Path
//  if (isUNCPath)
//    dir = _T("\\\\");
//  else if (isAbsolute)
//    dir = _T("\\");
//  else
//    dir.Empty();
//
//  POSITION pos = elements.GetHeadPosition ();
//	while (pos != NULL)
//	{
//    CString element = elements.GetNext(pos);
//    dir += element + _T("\\");
//  }
//
//  _tcsncpy (m_szDir, dir, _MAX_DIR);
}

