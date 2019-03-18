/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <afx.h>

#include <fcntl.h>
#include <io.h>
#include <conio.h>

#include <iostream>
#include <sstream>

#include "resource.h"
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

using namespace std;

#define ULTRAGETOPT_REPLACE_GETOPT
#include "..\Shared\ultragetopt.h"


#include <..\..\shared\tre-0.8.0\lib\regex.h>

#include "..\hardlink\include\hardlink_types.h"
#include "..\hardlink\include\AsyncContext.h"
#include "..\hardlink\src\mmfobject.h"

#pragma comment(lib, "wsock32.lib")


