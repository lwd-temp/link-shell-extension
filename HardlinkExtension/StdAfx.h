/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _CRT_SECURE_NO_DEPRECATE

#define ISOLATION_AWARE_ENABLED 1
#define SIDEBYSIDE_COMMONCONTROLS 1

// Disable checked iterators in release builds
#ifndef _DEBUG
// #  define _SECURE_SCL 0
#endif

// Used to enable an extra menue entry to delete junctions from LSE
// #define REMOVE_DELETE_JUNCTION



#include <windows.h>
#include <winnt.h>
#include <winioctl.h>
#include <tchar.h>

#define INITGUID
#include <initguid.h>
#include <shlguid.h>

#include <winnetwk.h>
#pragma comment(lib, "wsock32.lib")

#include <ShlObj.h>

#include <stdio.h>

//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
#include <vector>
#include <algorithm>
#include <list>
#include <atomic>

using namespace std;

#include "uxtheme.h"
#include "winuser.h"
#include "commctrl.h"

#include <sys/stat.h>
#include <shlwapi.h>
#include <strsafe.h>

#include "resource.h"

#include <..\..\shared\tre-0.8.0\lib\regex.h>



#include "hardlink_types.h"
#include "..\hardlink\src\mmfobject.h"
#include "localisation.h"
#include "multilang.h"

#include "AsyncContext.h"
#include <HardLink.h>
#include "Utils.h"


// {0A479751-02BC-11d3-A855-0004AC2568AA}
DEFINE_GUID(CLSID_HardLinkExtension, 0xa479751, 0x2bc, 0x11d3, 0xa8, 0x55, 0x0, 0x4, 0xac, 0x25, 0x68, 0xaa);

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


