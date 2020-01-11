/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#if defined( _UNICODE ) && ! defined( UNICODE )
#define UNICODE
#endif


#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include <lm.h>

#include <shlwapi.h>

#include <shlobj.h>

#include "resource.h"

using namespace std;

// Include interfaces from hardlink.lib
#include "HardlinkLib.h"
