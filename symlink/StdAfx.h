/*
Copyright (C) 1999-2001, Hermann Schinagl, Hermann.Schinagl@gmx.net


*/

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _WIN32_WINNT		0x500

#if defined( UNICODE )	 && ! defined( _UNICODE )
#define _UNICODE
#endif

#if defined( _UNICODE ) && ! defined( UNICODE )
#define UNICODE
#endif


//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
#include <vector>
#include <algorithm>
#include <list>


#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <tchar.h>
#include <lm.h>
#include <direct.h>
#include <string.h>
#include <shlwapi.h>
#include <sys/stat.h>

