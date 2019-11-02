#include "stdafx.h"

#include "hardlink_types.h"
#include "NtBase.h"

#include "processes.h"
#include "resource.h"
#include "strsafe.h"

#if defined _DEBUG
#include "ProcessesTest.h"
#endif;

HINSTANCE					g_hInstance;
HWND						  g_hwndParent;
HINSTANCE					g_hInstLib;

//-------------------------------------------------------------------------------------------
// main DLL entry
STDAPI
DllMain( HINSTANCE hInstance, DWORD ul_reason_for_call, LPVOID lpReserved )

/*
BOOL WINAPI		_DllMainCRTStartup( HANDLE	hInst,
									ULONG	ul_reason_for_call,
									LPVOID	lpReserved )
*/
{
	switch( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
			g_hInstance		= hInstance;
		break;
	}

    return TRUE;
}



//-------------------------------------------------------------------------------------------
// find a process by name
// return value:	true	- process was found
//					false	- process not found
bool	FindProc( char *szProcess )
{
	wchar_t		szProcessNameUni[1024];
  PUINT_PTR dPID;
	ULONG		dPIDSize = 0;

	int szProcessNameLen = strlen(szProcess);
	int	szProcessNameUniLen = MultiByteToWideChar (
		CP_UTF8,					// code page
		0,							// performance and mapping flags
		(const char*)szProcess,		// input (char*)
		szProcessNameLen + 1,		// input len including 0-char
		(wchar_t*)szProcessNameUni,	// output buffer (wchar_t*)
		(szProcessNameLen + 1) * 2	// output buffer size
	);

	bool b = NtQueryProcessId(szProcessNameUni, &dPID, &dPIDSize);
	if (b)
	{
		if (dPIDSize > 0)
			return true;
		else
			return false;
	}
	else
		return false;
}


//-------------------------------------------------------------------------------------------
// kills a process by name
// return value:	true	- process was found
//					false	- process not found
bool	KillProc( char *szProcess )
{
	wchar_t		szProcessNameUni[1024];
  PUINT_PTR dPID;
  ULONG		dPIDSize;

	//
	// enumerate processes names
	//
	int szProcessNameLen = strlen(szProcess);
	int	szProcessNameUniLen = MultiByteToWideChar (
		CP_UTF8,					// code page
		0,							// performance and mapping flags
		(const char*)szProcess,		// input (char*)
		szProcessNameLen + 1,		// input len including 0-char
		(wchar_t*)szProcessNameUni,	// output buffer (wchar_t*)
		(szProcessNameLen + 1) * 2	// output buffer size
	);

  bool found = false;
  bool b = NtQueryProcessId(szProcessNameUni, &dPID, &dPIDSize);
	if (b)
	{
		HANDLE		hProcess;
		//
		// walk trough and compare see if the process is running
		//
		for (int k = 0; k < dPIDSize; k++)
		{
			if( NULL != ( hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dPID[ k ] ) ) )
			{
				//
				// kill process
				//
				if( false == TerminateProcess( hProcess, 0 ) )
					CloseHandle( hProcess );
				else
					found = true;

				//
				// refresh systray
				//
				UpdateWindow( FindWindow( NULL, "Shell_TrayWnd" ) );

				//
				// refresh desktop window
				//
				UpdateWindow( GetDesktopWindow() );
				CloseHandle( hProcess );
			}
		}
	}
  return found;
}



//-------------------------------------------------------------------------------------------
// Checks for a given plattform if the 
// correct ExtractAndFork is installed by forking a process
DWORD	
ExtractAndFork( 
  char* aExeName, 
  char* aPlattform, 
  int aOffset 
)
{
	char sl[MAX_PATH];

	int id_plattform = -1;
	if (!stricmp(aPlattform, "win32"))
		id_plattform = IDR_VCREDISTPROBE_WIN32 + aOffset;
	else
	{
		if (!stricmp(aPlattform, "x64"))
			id_plattform = IDR_VCREDISTPROBE_X64 + aOffset;
	}

// #define _PROCESSES_DEBUG

	HRSRC symlink_res = FindResourceW(g_hInstance, (LPCWSTR)id_plattform, L"EXE");
	if (!symlink_res)
		return ERROR_RESOURCE_NAME_NOT_FOUND;

	HANDLE Symlink_han = LoadResource(g_hInstance, symlink_res);
	if (!Symlink_han)
		return ERROR_RESOURCE_DATA_NOT_FOUND;

	LPVOID pSymlink = LockResource(Symlink_han);

#if defined _PROCESSES_DEBUG
  _SYSTEMTIME SysTime;
  GetLocalTime(&SysTime);

  char filename[MAX_PATH];
  sprintf(filename, "c:\\tmp\\Fork %04d-%02d-%02d %02d-%02d-%02d",
    SysTime.wYear,
    SysTime.wMonth,
    SysTime.wDay,
    SysTime.wHour,
    SysTime.wMinute,
    SysTime.wSecond
  );

  FILE* f = fopen(filename, "w");
#endif
	GetTempPath(MAX_PATH, sl);
	strcat(sl, aExeName);
	DeleteFile(sl);

	HANDLE hsl = CreateFile(sl,
		GENERIC_WRITE|GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hsl)
	{
		DWORD sl_size = SizeofResource(g_hInstance, symlink_res);
		DWORD sl_written;
		BOOL b = WriteFile(hsl,
			pSymlink,
			sl_size,
			&sl_written,
			NULL);
#if defined _PROCESSES_DEBUG
		if (!b)
		{
			fprintf(f, "WriteFile GetLastError() '%s' '%s' %d\n", sl, aPlattform, GetLastError());
		}
#endif

		CloseHandle(hsl);
	}
	else
	{
#if defined _PROCESSES_DEBUG
		fprintf(f, "CreateFile GetLastError() '%s' '%s' %d\n", sl, aPlattform, GetLastError());
#endif
		return ERROR_INSTALL_TEMP_UNWRITABLE;
	}

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );

  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;

  UINT errormode = GetErrorMode();
  SetErrorMode(SEM_FAILCRITICALERRORS);
	BOOL b = CreateProcess(sl,
		NULL, 
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS | CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&pi);
  SetErrorMode(errormode);

  // Wait until child process exits.
  WaitForSingleObject( pi.hProcess, INFINITE );

  DWORD ExitCode;
  GetExitCodeProcess(pi.hProcess, &ExitCode);

  CloseHandle( pi.hProcess );
  CloseHandle( pi.hThread );

#if !defined _PROCESSES_DEBUG
	DeleteFile(sl);
#else
  fprintf(f, "CreateProcess Result: '%s' '%s', LastError: %d, ExitCode %d\n", sl, aPlattform, ExitCode);
	fclose(f);
#endif

  return ExitCode;
}

//-------------------------------------------------------------------------------------------
// Checks which modules are loaded in which processes
bool	
ProcEnumModules( char* aPlattform, int ProcessListSize, char* ProcessList )
{
  bool RetVal = false;

  DWORD err = ExtractAndFork("ProcessEnumModules.exe", aPlattform, 3);

  wchar_t TempPath[MAX_PATH];
  GetTempPathW(MAX_PATH, TempPath);
  wcscat(TempPath, L"LseInstallerCollidingProcesses.txt");

  char  ProcessName[MAX_PATH];
  ProcessList[0] = 0x00;
  FILE* ProcessFile = _wfopen (TempPath, L"rb");
  if (ProcessFile)
  {
    while (!feof(ProcessFile))
    {
      int c = fwscanf (ProcessFile, L"%S\n", ProcessName);
      if (c != EOF)
      {
        if (stricmp(ProcessName, "explorer.exe"))
        {
          StringCchCat(ProcessList, ProcessListSize, ProcessName);
          StringCchCat(ProcessList, ProcessListSize, "\n");
          RetVal = true;
        }
      }
    }

    fclose(ProcessFile);
  }

  DeleteFileW(TempPath);

  return RetVal;
}

//-------------------------------------------------------------------------------------------
// Checks for a given plattform if the 
// correct vcredist is installed by forking a process
// return value:	true	- correct vcredist installed
//					false	- vcredist missing
bool	Vcredist( char* aPlattform )
{
  DWORD err = ExtractAndFork("VCRedistProbe.exe", aPlattform, 0);

	if (STATUS_DLL_NOT_FOUND == err)
		 return false;
	else
		return true;
}

//-------------------------------------------------------------------------------------------
// Checks if the plattform is correct
// correct vcredist is installed by forking a process
// return value:	true	- correct vcredist installed
//					false	- vcredist missing
bool	CheckPlatt( char* aPlattform )
{
    HKEY RegKey;
	DWORD aSize;
	DWORD aType;

	bool Retval = false;

    DWORD RetVal = ::RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
            0,
            KEY_READ,
            &RegKey
    );

	if (ERROR_SUCCESS == RetVal)
	{
		aType = REG_SZ;
		char ProcessorArchitecture[MAX_PATH];
		ProcessorArchitecture[0] = 0x00;
		aSize = MAX_PATH;
		RegQueryValueEx(
				RegKey,
				"PROCESSOR_ARCHITECTURE",
				0,
				&aType,
				(LPBYTE)ProcessorArchitecture,
				&aSize
		);

		if (!stricmp(aPlattform, "x64"))
		{
			if (!stricmp(ProcessorArchitecture, "AMD64"))
				Retval = true;
		}
		else
		{
			if (!stricmp(aPlattform, "win32"))
			{
				if (!stricmp(ProcessorArchitecture, "x86"))
					Retval = true;
			}
		}
		::RegCloseKey(RegKey);
	}
	return Retval;
}

//-------------------------------------------------------------------------------------------
// Retrieves the OS Version
bool	_GetOsVersion( char* aOsVersion)
{
    OSVERSIONINFO vi = { sizeof(OSVERSIONINFO) };
	  GetVersionEx(&vi);

    sprintf(aOsVersion, "%d.%d", vi.dwMajorVersion, vi.dwMinorVersion);

    return true;
}
//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	FindProcess( HWND		hwndParent,
													 int		string_size,
													 char		*variables,
													 stack_t	**stacktop )
{
	char		szParameter[ 1024 ];


	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
		popstring( szParameter );

		if( true == FindProc( szParameter ) )
			wsprintf( szParameter, "1" );
		else
			wsprintf( szParameter, "0" );

		setuservariable( INST_R0, szParameter );
	}
}




//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	KillProcess( HWND		hwndParent,
													 int		string_size,
													 char		*variables,
													 stack_t	**stacktop )
{
	char		szParameter[ 1024 ];


	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
		popstring( szParameter );

		if( true == KillProc( szParameter ) )
			wsprintf( szParameter, "1" );
		else
			wsprintf( szParameter, "0" );

		setuservariable( INST_R0, szParameter );
	}
}

//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	VcRedistLevel( HWND		hwndParent,
													 int		string_size,
													 char		*variables,
													 stack_t	**stacktop )
{
	char		szParameter[ 1024 ];


	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
		popstring( szParameter );

		if( true == Vcredist( szParameter ) )
			wsprintf( szParameter, "1" );
		else
			wsprintf( szParameter, "0" );

		setuservariable( INST_R0, szParameter );
	}
}

//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	CheckPlattform( HWND		hwndParent,
													 int		string_size,
													 char		*variables,
													 stack_t	**stacktop )
{
	char		szParameter[ 1024 ];

	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
		popstring( szParameter );

		if( true == CheckPlatt( szParameter ) )
			wsprintf( szParameter, "1" );
		else
			wsprintf( szParameter, "0" );

		setuservariable( INST_R0, szParameter );
	}
}

//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	ProcessEnumModules( HWND		hwndParent,
													 int		string_size,
													 char		*variables,
													 stack_t	**stacktop )
{
	const int ProcessListSize = 1024;

  char		szParameter[ 1024 ];
	char		szProcesses[ ProcessListSize ];

	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
		popstring( szParameter );

		if( true == ProcEnumModules( szParameter, ProcessListSize, szProcesses ) )
			wsprintf( szParameter, "1" );
		else
			wsprintf( szParameter, "0" );

		setuservariable( INST_R0, szParameter );
		setuservariable( INST_R1, szProcesses );
	}
}

//-------------------------------------------------------------------------------------------
extern "C" __declspec(dllexport) void	GetOsVersion( HWND		hwndParent,
													 int		string_size,
													 char		*variables,
													 stack_t	**stacktop )
{
	g_hwndParent	= hwndParent;

	EXDLL_INIT();
	{
    char		szParameter[ 1024 ];
		popstring( szParameter );
		wsprintf( szParameter, "1" );

    char		szOsVersion[MAX_PATH];
	  _GetOsVersion(szOsVersion);

		setuservariable( INST_R0, szParameter );
		setuservariable( INST_R1, szOsVersion );
	}
}
#if defined _DEBUG

PROCESS_API
void
KillProcessTest( char* aPlattform )
{
  char ProcessList[1024];
  ProcEnumModules(aPlattform, 1024, ProcessList);
}

PROCESS_API
void
VcRedistLevelTest( char* aPlattform )
{
  Vcredist(aPlattform);
}

PROCESS_API
void
GetOsVersionTest( )
{
  char		szOsVersion[MAX_PATH];
  _GetOsVersion(szOsVersion);
}
#endif

