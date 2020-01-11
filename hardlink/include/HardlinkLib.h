/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

// Provide all neccessesary files to compile hardlink.lib

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
#include <atomic>
#include <mutex>

#include <WinSock.h>
#pragma comment(lib, "wsock32.lib")

// regular expression parser
#include "..\..\shared\tre-0.8.0\lib\regex.h" 


#include "hardlink_types.h"
#include "DbgHelpers.h"

#include <ntsysteminfo.h>

#include "LSESettings.h"

#include "AsyncContext.h"
#include "mmfobject.h"
#include "Progressbar.h"
#include "hardlinks.h"
#include "HardlinkUtils.h"
#include "NtBase.h"


#include "UACReuse.h"

#include "multilang.h"
