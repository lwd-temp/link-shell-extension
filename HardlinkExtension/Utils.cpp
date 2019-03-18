/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */


#include "stdafx.h"
#include "Utils.h"

extern HINSTANCE g_hInstance;

FILE*
OpenFileForExeHelper(
  wchar_t* curdir, 
  wchar_t* sla_quoted
)
{
  // Get the current path of extension installation
  wchar_t* sla = &sla_quoted[1];
  sla_quoted[0] = '\"';

  GetTempPath(HUGE_PATH, curdir);
  wcscpy(sla, curdir);
  wcscat(sla, SYMLINKARGS);

  // Write the file for the args
#if defined _DEBUG
  DeleteFile(sla);
#endif

  return _wfopen (sla, L"wb");
}


int
ForkExeHelper(
	wchar_t*	curdir,
	wchar_t*	sla_quoted
)
{	
  // TODO: Das Übergabefile sollte nicht nur einen Namen haben, weil auch mehere Instanzen der LSE laufen können
  wchar_t symlinkexe[HUGE_PATH];	
	GetModuleFileNameW(g_hInstance, symlinkexe, HUGE_PATH);
	PathRemoveFileSpec(symlinkexe);

	wcscat(symlinkexe, L"\\symlink.exe");
	wcscat(sla_quoted, L"\"");

	// Start the process
	SHELLEXECUTEINFO se;
	ZeroMemory(&se, sizeof(se));

	se.cbSize = sizeof(SHELLEXECUTEINFO);
	se.fMask = SEE_MASK_NOCLOSEPROCESS;
	se.lpVerb = NULL;
	se.lpFile = symlinkexe;
	se.lpParameters = sla_quoted;
	se.lpDirectory = curdir;
	se.nShow = SW_HIDE;
	ShellExecuteEx(&se);
	
	// Wait for the process to finish
	WaitForSingleObject(se.hProcess, INFINITE);
	DWORD	ExitCode;
	GetExitCodeProcess(se.hProcess, &ExitCode);
	CloseHandle(se.hProcess);

#if !defined(_DEBUG)
	// Delete the tmp files afterwards

#if !defined DEBUG_DO_NOT_DELETE_SYMLINKS_ARGS
 	sla_quoted[wcslen(sla_quoted) - 1] = 0x00;
	DeleteFile(&sla_quoted[1]);
#endif
#endif
  HTRACE (L"%s\n", &sla_quoted[1]);
	
	return ExitCode;
}	

