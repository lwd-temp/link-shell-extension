/*
  Copyright (C) 1999 - 2020, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"

#include "hardlink_types.h"


#if defined _HTRACE_DEBUG
void 
HTRACE (wchar_t* aFormat ...)
{
  va_list args;
  wchar_t msg[HUGE_PATH];

  va_start(args, aFormat);
  vswprintf(msg, HUGE_PATH, aFormat, args);
#if defined _HTRACE_OUTPUT_DEBUG_STRING
  OutputDebugStringW(msg);
#else
  wprintf(msg);
#endif
  va_end(args);
}
#else
void
HTRACE (wchar_t* aFormat ...) 
{
}
#endif

