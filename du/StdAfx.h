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

#include <list>
#include <vector>
#include <atomic>

using namespace std;

#include <..\..\shared\tre-0.8.0\lib\regex.h>

#include "resource.h"

#include "..\hardlink\include\hardlink_types.h"
#include "..\hardlink\src\MmfObject.h"
#include "AsyncContext.h"

#pragma comment(lib, "wsock32.lib")
