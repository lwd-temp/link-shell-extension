/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

#include "..\..\Shared\version\version.h"

#include <ShObjIdl.h>
#include <sddl.h>


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#if defined( UNICODE )	 && ! defined( _UNICODE )
#define _UNICODE
#endif

#if defined( _UNICODE ) && ! defined( UNICODE )
#define UNICODE
#endif

#include <windows.h>
#include <winbase.h>
#include <winnt.h>
#include <winioctl.h>
#include <tchar.h>
#include <assert.h>
#include <lm.h>

#include <ntsysteminfo.h>


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
#include <regex>
#include <atomic>


#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <stdio.h>

#include <ShlObj.h>
#include <shlwapi.h>
#include <shlguid.h>
using namespace std;

#include <..\..\shared\tre-0.8.0\lib\regex.h>

#include <..\..\shared\version\RegSchemaVersion.h>


#include "md5.h"

#include <psapi.h>


