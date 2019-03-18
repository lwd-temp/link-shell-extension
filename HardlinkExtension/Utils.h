/*
	Copyright (C) 1999_2006, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#ifndef _UTILS_F0DB6EF6_A37B_410a_929F_A4D55EDD864C
#define _UTILS_F0DB6EF6_A37B_410a_929F_A4D55EDD864C

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


#endif
