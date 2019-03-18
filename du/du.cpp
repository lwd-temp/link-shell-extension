#include "stdafx.h"

#include "hardlink.h"

ULONG64 gSize, gCount, gDirectories;

void Usage()
{
	wprintf(L"Prints Disk Usage of specified path recursively. (C) 1990-2019 Hermann Schinagl\n");
	wprintf(L"Parameter error. Usage : DU [[drive]:[searchpath]]. Version 5.10\n");
}

int du (
	PWCHAR	aSrcPath
)
{
	WIN32_FIND_DATAW		wfind;
	int	RetVal = ERROR_SUCCESS;

  _CrtSetDebugFillThreshold(0);

  size_t sSrcLen = wcslen(aSrcPath);
	wcscat (aSrcPath, L"\\*.*");
	HANDLE	sh = FindFirstFileW (aSrcPath, &wfind);
	aSrcPath[sSrcLen] = 0x00;
	if (INVALID_HANDLE_VALUE != sh)
	{
		do
		{
			if (wfind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (wcscmp (wfind.cFileName, L"."))
				{
					if (wcscmp (wfind.cFileName, L".."))
					{
						sSrcLen = wcslen(aSrcPath);
						wcscat (aSrcPath, L"\\");
						wcscat (aSrcPath, wfind.cFileName);

						gDirectories++;
						du(aSrcPath);
						aSrcPath[sSrcLen] = 0x00;
					}
				}
			}
			else
			{
				gCount++;
				gSize += wfind.nFileSizeLow;
			}
		}
		while (FindNextFileW (sh, &wfind) > 0);

		FindClose (sh);
	}

	return RetVal;
}

// There are two things to take care of when you want to use Unicode:
// 1) In the linker section define '/ENTRY:"wmainCRTStartup"'
// 2) Declare wmain via extern "C" if it is in a .cpp program

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int wmain (int argc, wchar_t* argv[])
{
	gSize = 0;
	gCount = 0;
	gDirectories = 0;
	int i = 1;
	while (i < argc)
	{
		wchar_t 	Path[HUGE_PATH];
		wcscpy(Path, PATH_PARSE_SWITCHOFF);
		wcscat (Path, argv[i]);
		du (Path);
		i++;
	}

	if (argc == 1)
	{
		Usage();
	}
	else
	{
		if (gCount)
		{
			std::locale localeDeu( "" );
			std::ostringstream oss;

			oss.imbue( localeDeu );
			oss << gCount << " matching file(s) in " << gDirectories << " directories found, using " << gSize << " bytes." << std::endl;
			std::cout << oss.str();
		}
		else
			wprintf(L"No matching files found.\n");
	}


	return 0;
}

#ifdef __cplusplus
}
#endif // __cplusplus
