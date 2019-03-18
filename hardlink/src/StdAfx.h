// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__498AEA3E_7942_4B6B_9CD2_641E880D1CD8__INCLUDED_)
#define AFX_STDAFX_H__498AEA3E_7942_4B6B_9CD2_641E880D1CD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define _CRT_SECURE_NO_DEPRECATE

#if defined( UNICODE )	 && ! defined( _UNICODE )
#define _UNICODE
#endif

#if defined( _UNICODE ) && ! defined( UNICODE )
#define UNICODE
#endif

// Disable checked iterators in release builds
#ifndef _DEBUG
// #  define _SECURE_SCL 0
#endif

#include <windows.h>
#include <winbase.h>
#include <winnt.h>
#include <winioctl.h>
#include <tchar.h>
#include <assert.h>
#include <lm.h>


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

#include <iostream>
#include <iomanip>
#include <sstream>

#include <ShlObj.h>
#include "shlwapi.h"

#include <..\..\shared\tre-0.8.0\lib\regex.h>

#if defined USE_ROCKALL_HEAPMANAGER
#  include "FastHeap.hpp"
#endif





#endif // !defined(AFX_STDAFX_H__498AEA3E_7942_4B6B_9CD2_641E880D1CD8__INCLUDED_)
