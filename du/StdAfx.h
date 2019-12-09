/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */


#pragma once

#include <windows.h>
#include <winbase.h>

#include <wchar.h>
#include <errno.h>
#include <io.h>

#include <iostream>
#include <sstream>

 //disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
#include <list>
#include <vector>
#include <atomic>
#include <map>
#include <algorithm>
#include <set>
#include <mutex>

using namespace std;

#include "resource.h"

#include <ShObjIdl.h>

// regular expression parser
#include <..\..\shared\tre-0.8.0\lib\regex.h>

// Include interfaces from hardlink.lib
#include "HardlinkLib.h"

#pragma comment(lib, "wsock32.lib")
