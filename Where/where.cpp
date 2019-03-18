#include "stdafx.h"


#define _A_SUBDIR 	0x10
#define PATH_PARSE_SWITCHOFF L"\\\\?\\" 
#define PATH_PARSE_SWITCHOFF_SIZE (sizeof(PATH_PARSE_SWITCHOFF) - 1) / sizeof(wchar_t)

ULONG64 gSize, gCount;

void Usage()
{
	wprintf(L"Searches whole drive recursively for file(s). (C) 1990-2012 Hermann Schinagl.\n");
	wprintf(L"Parameter error. Usage : WHERE filename [[drive]:[searchpath]]. Version 4.10\n");
}

#if defined _DEBUG
char* wstr2astr (wchar_t* pwStr, char* paStr)
{
	int len = wcslen(pwStr);
	for (int i = 0; i < len; i++)
		paStr[i] = _mbctombb (pwStr[i]);
	paStr[len] = 0x0;
	return paStr;	
}
#endif

int where (wchar_t* aRegExpr, wchar_t* aPath)
{
	wchar_t 				PathWid[8192];
	struct _wfinddata_t		wfind;

	wcscpy (PathWid, aPath);
	wcscat (PathWid, L"*.*");
	long sh = _wfindfirst (PathWid, &wfind);
	if (sh > 0)
		do
		{
			if (wcscmp(wfind.name, L"."))
			{
				if (wcscmp(wfind.name, L".."))
				{
					if (wfind.attrib & _A_SUBDIR )
					{
						wcscpy (PathWid, aPath);
						wcscat (PathWid, wfind.name);
						wcscat (PathWid, L"\\");
						where (aRegExpr, PathWid);
						
					}
				}
			}
		} while (_wfindnext (sh, &wfind) >= 0);
		_findclose(sh);

	// Search for files
	wcscpy (PathWid, aPath);
	wcscat (PathWid, aRegExpr);

	sh = _wfindfirst (PathWid, &wfind);
	if (sh > 0)
		do
		{
			if (!(wfind.attrib & _A_SUBDIR) )
			{
				wprintf(L"%s%s\n", &aPath[PATH_PARSE_SWITCHOFF_SIZE], wfind.name);

				gCount++;
				gSize += wfind.size;
			}
		} while (_wfindnext (sh, &wfind) >= 0);
		_findclose(sh);

	return 42;
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
	int i = 2;
	while (i < argc)
	{
		wchar_t 	Path[8192];
		wcscpy(Path, PATH_PARSE_SWITCHOFF );
    wchar_t*  pFilePart;     
    GetFullPathNameW(argv[i], 8192 - PATH_PARSE_SWITCHOFF_SIZE, &Path[PATH_PARSE_SWITCHOFF_SIZE], &pFilePart);

    WCHAR u = Path[wcslen(Path) - 1];
		if (u != L'\\')
			if (u != L':')
				wcscat(Path, L"\\");

    where (argv[1], Path);
		i++;
	}

	if (argc == 1)
	{
		Usage();
	}
	else
	{
		if (gCount)
			wprintf(L"%I64d matching file(s) found, using %I64d bytes.\n", gCount, gSize);
		else
			wprintf(L"No matching files found.\n");
	}


	return 0;
}

#ifdef __cplusplus
}
#endif // __cplusplus
