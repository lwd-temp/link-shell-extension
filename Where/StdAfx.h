

#pragma once


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <tchar.h>

#include "time.h"


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
#include <mutex>


#define ULTRAGETOPT_REPLACE_GETOPT
#include "..\Shared\ultragetopt.h"

#include <conio.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <locale.h>

#include <fcntl.h>
#include <io.h>

#include <StrSafe.h>
#include "shlobj.h"

#define HUGE_PATH 8192

