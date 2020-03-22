/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define ISOLATION_AWARE_ENABLED 1
#define SIDEBYSIDE_COMMONCONTROLS 1

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include <iomanip>
#include <sstream>

#define INITGUID
#include <initguid.h>
#include <shlguid.h>

#include <shlobj.h>

#include "uxtheme.h"
#include "winuser.h"
#include "commctrl.h"

#include <shlwapi.h>

#include "resource.h"

using namespace std;

// Include interfaces from hardlink.lib
#include "HardlinkLib.h"

// Component specific includes
#include "localisation.h"


// {0A479751-02BC-11d3-A855-0004AC2568AA}
DEFINE_GUID(CLSID_HardLinkExtension, 0xa479751, 0x2bc, 0x11d3, 0xa8, 0x55, 0x0, 0x4, 0xac, 0x25, 0x68, 0xaa);

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


