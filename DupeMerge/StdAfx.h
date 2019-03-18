// stdafx.h : include file for standard system include files,
//	or project specific include files that are used frequently, but
//			are changed infrequently
//

#if !defined(AFX_STDAFX_H__B32CE74F_7C36_4741_BE3B_050C80CB9A14__INCLUDED_)
#define AFX_STDAFX_H__B32CE74F_7C36_4741_BE3B_050C80CB9A14__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define _WIN32_WINNT		0x500

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>

//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <algorithm>

#include <..\..\shared\tre-0.8.0\lib\regex.h>


const	int HUGE_PATH = 8192;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B32CE74F_7C36_4741_BE3B_050C80CB9A14__INCLUDED_)
