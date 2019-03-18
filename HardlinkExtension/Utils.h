/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

#define SYMLINKARGS L"symlink.args"

FILE*
OpenFileForExeHelper(
  wchar_t* curdir, 
  wchar_t* sla_quoted
);

int 
ForkExeHelper(
	wchar_t*	curdir,
	wchar_t*	sla_quoted
);
