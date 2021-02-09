/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <stdio.h>
#include <tchar.h>

#include <lm.h>
#include <shlwapi.h>

#include <iostream>

#include <locale.h>

#include <fcntl.h>
#include <io.h>

#include <shlobj.h>

#include "ln_version.h"
#include "..\Shared\version\version_assembly.h"

// Ultragetopt
#define ULTRAGETOPT_REPLACE_GETOPT
#include "..\Shared\ultragetopt.h"

using namespace std;

// Include interfaces from hardlink.lib
#include "HardlinkLib.h"

