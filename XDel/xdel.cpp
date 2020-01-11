/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#include "stdafx.h"

// ULONG64 gSize, gCount, gDirectories, gJunctions;

void Usage()
{
	wprintf(L"Fast recursive directory tree delete utility. (C) 1990-2019 Hermann Schinagl.\n");
	wprintf(L"Parameter error. Usage: XDEL [drive:][path\\]directory. Version 5.302\n");
}

// There are two things to take care of when you want to use Unicode:
// 1) In the linker section define '/ENTRY:"wmainCRTStartup"'
// 2) Declare wmain via extern "C" if it is in a .cpp program

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define DEL_ONLY_REPARSE

static struct option	long_options[] =
{
	{ "reparse", no_argument, NULL, 'r' },
	{ 0, 0, 0, 0 }
};


int wmain (int argc, wchar_t* argv[])
{
  XDelStatistics aXDelStatistics;
  ZeroMemory(&aXDelStatistics, sizeof(aXDelStatistics));

  // Take the default locale
  setlocale(LC_ALL,"");

#if 0
	int i = 1;
  while (i < argc)
	{
		wchar_t 	Path[HUGE_PATH];
    wcscpy_s(Path, HUGE_PATH, PATH_PARSE_SWITCHOFF);

  	wchar_t*	filePart;
		GetFullPathNameW(argv[i], HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &Path[PATH_PARSE_SWITCHOFF_SIZE], &filePart);
    XDel (Path, aXDelStatistics, false);
#if !defined DEL_ONLY_REPARSE
		RemoveDir(Path, FALSE);
#endif
		i++;
	}
#else
    // Convert the wide options to ansi options
		char**a_argv = new char*[argc + 1];
		int i;
		for (i = 0; i < argc; i++)
		{
			int argv_length = (int)wcslen (argv[i]) + 1;
      a_argv[i] = new char[argv_length];
      WideCharToMultiByte(
        CP_ACP,
        0,
        argv[i],
        argv_length,
        a_argv[i],
        argv_length,
        NULL,
        NULL
      );
		}
    a_argv[argc] = NULL;

		int XdelOptions = 0;
    while (1)
		{
      int longind;
      char c = ultragetopt_tunable (
				argc, 
				a_argv, 
        "r", 
				long_options,
				&longind,
        "=",
        "-",
        UGO_HYPHENARG | UGO_OPTIONPERMUTE | UGO_OPTIONALARG | UGO_SHORTOPTASSIGN | UGO_SEPARATEDOPTIONAL
			);
			if (c == EOF)
				break;

			switch (c)
			{
        // --reparse
        case 'r':
        {
          XdelOptions |= FileInfoContainer::eDelOnlyReparsePoints;
          break;
        }
        default:
        break;
      }
      i++;
    }

    wchar_t 	Path[HUGE_PATH];
    wcscpy_s(Path, HUGE_PATH, PATH_PARSE_SWITCHOFF);

    wchar_t*	filePart;
    GetFullPathNameW(argv[optind], HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &Path[PATH_PARSE_SWITCHOFF_SIZE], &filePart);
    XDel (Path, HUGE_PATH, aXDelStatistics, XdelOptions);
    if (0 == (XdelOptions & ~FileInfoContainer::eDelOnlyReparsePoints))
      RemoveDir(Path, FALSE);

    // Delete ANSI options
    for (i = 0; i < argc + 1; i++)
      delete[] a_argv[i];
    delete[] a_argv;


#endif



	if (aXDelStatistics.Count)
	{
		std::locale localeDeu( "" );
		std::ostringstream oss;

		oss.imbue( localeDeu );
		oss << aXDelStatistics.Count << "  file(s) deleted in " << aXDelStatistics.Directories << " directories, skipping " << aXDelStatistics.Junctions << " Junctions, using " << aXDelStatistics.Size << " bytes." << std::endl;
		std::cout << oss.str();
	}


	if (i == 1)
		Usage();

	return 0;
}

#ifdef __cplusplus
}
#endif // __cplusplus
