/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#include <stdafx.h>


#include "hardlink.h"
#include "HardlinkUtils.h"

using namespace std;

const char						exec_name[] = "DupeMerge";

FILE*             gStdOutFile;
intptr_t          gStdOutHandle;
bool              gAutomatedTest = false;
bool              gListOnly = false;
int               gSlurp = FileInfoContainer::eNoDupeDebug;
int							  gLogLevel = FileInfoContainer::eLogVerbose;
int               gWhichGroups = DupeGroup::eVoid;
bool              gJson = false;


enum  DupeMergeSortMode {
  eSortBySize = 1,
  eSortByCardinality = 2,
};
int               gSortMode = eSortBySize;


// There are two things to take care of when you want to use Unicode:
// 1) In the linker section define '/ENTRY:"wmainCRTStartup"'
// 2) Declare wmain via extern "C" if it is in a .cpp program

static struct option	long_options[] =
{
	{ "exclude", required_argument, NULL, 'e' },
	{ "excludedir", required_argument, NULL, 'X' },
	{ "maxsize", required_argument, NULL, 'a' },
	{ "minsize", required_argument, NULL, 'i' },
	{ "wildcard", required_argument, NULL, 'w' },
	{ "include", required_argument, NULL, 'n' },
	{ "includedir", required_argument, NULL, 'I' },
	{ "regexp", required_argument, NULL, 'r' },
	{ "sort", required_argument, NULL, 's' },
	{ "list", no_argument, NULL, 'l' }, 
	{ "quiet", optional_argument, NULL, 'q' }, 
	{ "help", no_argument, NULL, 'h' }, 
	{ "debug", no_argument, NULL, 'd' },
	{ "automated_test", no_argument, NULL, 'A' }, 
	{ "output", required_argument, NULL, 'o' },

  // Only Long Options. If a short opt is added cBaseJustLongOpts has to be increased
  { "traditional", no_argument, NULL, '\0' },
	{ "slurpafterfind", required_argument, NULL, '\0' },
	{ "dumpafterfind", required_argument, NULL, '\0' },
	{ "slurpaftercalc", required_argument, NULL, '\0' },
	{ "dumpaftercalc", required_argument, NULL, '\0' },
	{ "whichgroups", required_argument, NULL, '\0' },
	{ 0, 0, 0, 0 }
};

const int cBaseJustLongOpts = 0x0c;

#if defined _DEBUG
// this is a testimplementataion for DupeContainer::NumberOfHardLinkGroups
ULONG
NumberOfHardLinkGroups(
	int		start,
	int		end,
	PBYTE	data
	)
{
		int nHlGroups = 0;
		if (start != end)
		{
			// Go through the files which have at least one FileIndex twin
			int	iter = start;
			int	last = iter;

			bool							equal = false;
			while (++iter != end)
			{
				if (data[iter] != data[last] )
				{
					if (equal)
						nHlGroups++;
					else
						if ( 1 == data[last])
							nHlGroups++;


					last = iter;
					equal = false;
				}
				else
				{
					if ( 1 == data[iter])
						nHlGroups++;
					equal = true;
				}
			} 
			if (equal)
				nHlGroups++;
			else
				if ( 1 == data[last])
					nHlGroups++;
		}

		return nHlGroups;
}

int
MarkRep(int		start, 
		int		end,
		PBYTE	data
)
{
	int rVal = 0;
	
	if (start == end)
		return -1;

	int	iter = start;
	int	last = iter;

	data[start] |= 0x80;
	rVal++;
	while (++iter != end)
	{
		data[iter] |= 0x080;
		if (( data[iter] & 0x7f )== (data[last] & 0x7f))
			data[last] &= 0x07f;
		else
			rVal++;

		last = iter;
	} 
	data[end] |= 0x80;

	for (int ii = start; ii < end; ii++)
		printf("%02x ", data[ii]);
	printf("\n");
	
	return rVal;
}

#endif


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	int
	Usage()
	{
		printf ("DupeMerge %s Find indentical files below a specified path and hardlink it.\n", VERSION_NUMBER);
		printf ("Usage. dupemerge [options] [commands] path\n\n");
	  printf ("Commands\n");
		printf ("  -a, --maxsize=NUMBER                Process only files smaller than MAXSIZE\n");
		printf ("  -i, --minsize=NUMBER                Process only files larger than MINSIZE\n");
		printf ("  -o, --output file                   redirect output to file\n\n");
		printf ("  -r, --regexp REGEXP                 Process only files matching <regexp>.\n");
		printf ("  -s, --sort filesize                 Sort dupegroups by filesize\n");
		printf ("  -s, --sort cardinality              Sort dupegroups by cardinality\n");
		printf ("  -w, --include WILDCARD              Process only files matching <wildcard>.\n");
		printf ("  -e, --exclude WILDCARD              Exclude files matching <wildcard>.\n");
	  printf ("  -X, --excludedir WILDCARD           Exclude directory recursively <wildcard>\n");
	  printf ("\nOptions\n");
		printf ("  -h, --help                          This help\n\n");
		printf ("  -l, --list                          Only list the identical files. Do not hardlink\n");
		printf ("  -q, --quiet                         Suppres any output\n");

		printf ("Samples\n");
		printf ("  dupemerge c:\\test\n");
		printf ("  dupemerge c:\\data c:\\test\\42\n");
		printf ("  dupemerge --list c:\\data c:\\test\\42\n");
		printf ("  dupemerge --minsize 3000 c:\\data\n");
		printf ("  dupemerge --maxsize 5000000 c:\\data c:\\winnt\n");
		printf ("  dupemerge --include *.exe c:\\data\n");
		printf ("  dupemerge --sort cardinality c:\\data\n");

		return errOk;
	}

void 
Exit(
  int ReturnCode
)
{
  fclose(gStdOutFile);
  CloseHandle((HANDLE)gStdOutHandle);
  exit (ReturnCode);
}


int
wmain(
  int argc, 
  wchar_t* argv[]
)
{
    _CrtSetDebugFillThreshold(0);
    bool  Traditional = false;
    {
      // Take the default locale
      setlocale(LC_ALL,"");

      _ArgvList SourceDirList;
      _StringList IncludeFileList;
      _StringList IncludeDirList;
      _StringList ExcludeFileList;
      _StringList ExcludeDirList;

      std::locale localeDeu( "" );
      std::ostringstream oss;
      oss.imbue( localeDeu );

      gStdOutHandle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
      int StdOutDesc = _open_osfhandle(gStdOutHandle, _O_TEXT);
      gStdOutFile = _fdopen(StdOutDesc, "w");

      FileInfoContainer	dc;
/*/
//      ULONG64 CheckSize = 0x4000000000000000;
//      ULONG64 CheckSize =            0x400007321;
      ULONG64 CheckSize;
        CheckSize =            0x400007321;
        CheckSize =            0x400000000;
//      CheckSize =            0x40000;
      const unsigned int MAX_INT  =            0xffffffff;
      unsigned char*  ptr = (unsigned char*)0x30000000000;

      ULONG64 toBeChecked;
      while (CheckSize > (ULONG64)MAX_INT) 
      {
        if (CheckSize > MAX_INT)
          toBeChecked = MAX_INT;
        else
          toBeChecked = CheckSize;
        printf("ptr: %I64x, CheckSize: %I64x\n", ptr, toBeChecked);
        CheckSize -= 0x100000000;
        ptr += 0x100000000;
      } 
      
      if (CheckSize > 0)
        printf("#ptr: %I64x, CheckSize: %I64x\n", ptr, CheckSize);
/*/


#if 0
      // Test code for max mappsing size calculation.
      ULONG64 lastCheckOffset = 0;
      ULONG64 lastCheckSize = 0;
      for (int i = 4; i < 64;i++)
      {
        ULONG64 CheckSize = 0;
        ULONG64 CheckOffset = dc.CalcDataSize(i, CheckSize);
        HTRACE(L"O: %I64x, S:%I64x\n", CheckOffset, CheckSize);

        if (lastCheckOffset + lastCheckSize != CheckOffset)
          HTRACE(L"###FAIL\n");

        lastCheckOffset = CheckOffset;
        lastCheckSize = CheckSize;
      }
#endif

#if 0

      int rrr;

      BYTE d0[] = { 1, 2, 3, 4 };
      rrr = MarkRep (0, 4, d0); // rrr = 3

      BYTE d1[] = { 1, 1, 1, 2, 2, 3, 3, 3 };
      rrr = MarkRep (0, 8, d1); // rrr = 3

      BYTE d2[] = { 1, 1, 1, 2, 2, 3, 3, 5 };
      rrr = MarkRep (0, 8, d2); // rrr = 4

      BYTE d3[] = { 0, 1, 1, 2, 2, 3, 3, 3 };
      rrr = MarkRep (0, 8, d3); // rrr = 4

      BYTE d4[] = { 0, 1, 5, 2, 2, 3, 3, 3 };
      rrr = MarkRep (0, 8, d4); // rrr = 5

      BYTE d5[] = { 0, 1, 5, 2, 2, 3, 3, 4 };
      rrr = MarkRep (0, 8, d5); // rrr = 6

      BYTE d6[] = { 0, 1, 5, 2, 2, 6, 3, 4 };
      rrr = MarkRep (0, 8, d6); // rrr = 7

      BYTE d7[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
      rrr = MarkRep (0, 8, d7); // rrr = 1

      BYTE d8[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
      rrr = MarkRep (0, 8, d8); // rrr = 8

      BYTE d9[] = { 1 };
      rrr = MarkRep (0, 1, d9); // rrr = 1

#endif

#if 0
      int rrr;

      BYTE t1[] = { 1, 1, 1, 2, 2, 3, 3, 3 };
      rrr = NumberOfHardLinkGroups(0,8, t1); // rrr = 5

      BYTE t2[] = { 2, 2, 3, 3, 3, 4, 4, 4, 4 };
      rrr = NumberOfHardLinkGroups(0,9, t2); // rrr = 3

      BYTE t3[] = { 1, 2, 2, 3, 3, 3 };
      rrr = NumberOfHardLinkGroups(0,6, t3); // rrr = 3

      BYTE t4[] = { 1, 1 };
      rrr = NumberOfHardLinkGroups(0,2, t4); // rrr = 2

      BYTE t5[] = { 3, 3, 3 };
      rrr = NumberOfHardLinkGroups(0,3, t5); // rrr = 1

      BYTE t6[] = { 1, 2, 2 };
      rrr = NumberOfHardLinkGroups(0,3, t6); // rrr = 2

      BYTE t7[] = { 2, 2, 1 };
      rrr = NumberOfHardLinkGroups(0,3, t7); // rrr = 2
#endif

      // Convert the wide options to ansi options
      char**a_argv = new char*[argc + 1];
      int i;
      for (i = 0; i < argc; i++)
      {
			  int argv_length = wcslen (argv[i]) + 1;
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

      while (1)
      {
        int longind;
        char c = ultragetopt_tunable (
          argc, 
          a_argv, 
          "e:X:a:i:I:n:w:r:s:lqhdo:Z:", 
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
          // --wildcard
          // --include
          case 'n':
          case 'w':
          {
            if (optind == argc)
            {
              Usage ();
              Exit (ERR_LESS_CMD_ARGUMENTS);
            }

            _StringList RawArgs;
            if ('@' == argv[optind - 1][0])
              ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
            else
              RawArgs.push_back(wstring(argv[optind - 1]));

            for (_StringListIterator iter = RawArgs.begin(); iter != RawArgs.end(); ++iter)
            {
				      WildCard2RegExp(*iter);

              *iter = *iter + L"$";
              IncludeFileList.push_back(*iter);
            }
          }
          break;

          // --includedir
          case 'I':
          {
            if (optind == argc)
            {
              Usage ();
              Exit (ERR_LESS_CMD_ARGUMENTS);
            }

            _StringList RawArgs;
            if ('@' == argv[optind - 1][0])
              ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
            else
              RawArgs.push_back(wstring(argv[optind - 1]));

            for (_StringListIterator iter = RawArgs.begin(); iter != RawArgs.end(); ++iter)
            {
				      WildCard2RegExp(*iter);

              // Check if it is an absolute path, e.g e:\\bla...
              wchar_t ttt = iter->at(1);
              if (iter->length() > 1 && iter->at(1) != ':')
                *iter = L"\\\\" + *iter + L"$";
              else
                *iter += L"$";

              IncludeDirList.push_back(*iter);
            }
          }
          break;

          // --exclude 
          case 'e':
          {
            if (optind == argc)
            {
              Usage ();
              Exit (ERR_LESS_CMD_ARGUMENTS);
            }

            _StringList RawArgs;
            if ('@' == argv[optind - 1][0])
              ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
            else
              RawArgs.push_back(wstring(argv[optind - 1]));

            for (_StringListIterator iter = RawArgs.begin(); iter != RawArgs.end(); ++iter)
            {
			        WildCard2RegExp(*iter);

              *iter = *iter + L"$";

              ExcludeFileList.push_back(*iter);
            }
          }
          break;

          // --excludedir
          case 'X':
          {
            if (optind == argc)
            {
              Usage ();
              Exit (ERR_LESS_CMD_ARGUMENTS);
            }

            _StringList RawArgs;
            if ('@' == argv[optind - 1][0])
              ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
            else
              RawArgs.push_back(wstring(argv[optind - 1]));

//            for (_StringListIterator iter = RawArgs.begin(); iter != RawArgs.end(); ++iter)
            for (auto iter : RawArgs)
            {
			        WildCard2RegExp(iter);

              // Check if it is an absolute path, e.g e:\\bla...
              if (iter.length() > 1 && iter.at(1) != ':')
                iter = L"\\\\" + iter + L"$";
              else
                iter += L"$";
              ExcludeDirList.push_back(iter);
            }
          }
          break;

          // --maxsize
          case 'a':
            if (optarg)
            {
              dc.SetMaxSize (_atoi64 (optarg));
            }
            break;

          // --minsize
          case 'i':
            if (optarg)
            {
              dc.SetMinSize (_atoi64 (optarg));
            }
            break;

          // --sort
          case 's':
            if (optarg)
            {
              if (!stricmp (optarg, "cardinality"))
                gSortMode = eSortByCardinality;
            }
            break;

          // --regexp
          case 'r':
          {
            if (optind == argc)
            {
              Usage ();
              Exit (ERR_LESS_CMD_ARGUMENTS);
            }

            if ('@' == argv[optind - 1][0])
              ReadArgsFromFile(&argv[optind - 1][1], IncludeFileList);
            else
              IncludeFileList.push_back(wstring (argv[optind - 1]));
          }
          break;

          // --quiet
          // 
          case 'q': 
          {
            if (NULL == optarg)
            {
              gLogLevel = FileInfoContainer::eLogQuiet;
            }
            else
            {
              int LogLev = _wtoi(argv[optind - 1]);
              switch(LogLev)
              {
                case 0:
                  gLogLevel = FileInfoContainer::eLogQuiet;
                  break;

                case 1:
                  gLogLevel = FileInfoContainer::eLogError;
                  break;

                case 2:
                  gLogLevel = FileInfoContainer::eLogChanged;
                  break;

                case 3:
                  gLogLevel = FileInfoContainer::eLogVerbose;
                  break;

                default:
                  gLogLevel = FileInfoContainer::eLogQuiet;
                break;
              }
            }
          }
          break;

          case 'l':
            dc.SetFlags (FileInfoContainer::eListOnly);
            gListOnly = true;
          break;

          case 'h':
            Usage();
            Exit (1);
          break;

          case 'd':
            dc.SetFlags (FileInfoContainer::eDebugDupeMerge);
          break;

          case 'A':
          {
              gAutomatedTest = true;
          }
          break;

          // --output
          // 
          case 'o':
          {
            FILE* Output = _wfopen(argv[optind - 1], L"wb");
            if (Output)
            {
              fclose(gStdOutFile);
              CloseHandle((HANDLE) gStdOutHandle);

              gStdOutFile = Output;
            }
          }
          break;

				  // We end up here if it was just long opts
          case 0:
				  {
            switch (longind)
            {
              // --traditional
              // 
              case cBaseJustLongOpts + 0x00:
              {
                Traditional = true;
              }
              break;

              // --SlurpAfterFind
              // 
              case cBaseJustLongOpts + 0x01:
              {
                struct _stat	buf;
                int r = _wstat (argv[optind - 1], &buf);
                if (r >= 0)
                {
                  _ArgvPath       ArgvXPath;
                  ArgvXPath.ArgvOrg = argv[optind - 1];
                  ArgvXPath.Argv = argv[optind - 1];
                  SourceDirList.push_back(ArgvXPath);

                  gSlurp |= FileInfoContainer::eSlurpAfterFind;
                }
                else
                {
                  fwprintf (gStdOutFile, L"'%s' can not be slurped.\n", argv[optind - 1]);
                }
              }
              break;

              // --DumpAfterFind
              // 
              case cBaseJustLongOpts + 0x02:
              {
                _ArgvPath       ArgvXPath;
                ArgvXPath.ArgvOrg = argv[optind - 1];
                ArgvXPath.Argv = argv[optind - 1];
                SourceDirList.push_back(ArgvXPath);

                gSlurp |= FileInfoContainer::eDumpAfterFind;
              }
              break;

              // --SlurpAfterCalc
              // 
              case cBaseJustLongOpts + 0x03:
              {
                struct _stat	buf;
                int r = _wstat (argv[optind - 1], &buf);
                if (r >= 0)
                {
                  _ArgvPath       ArgvXPath;
                  ArgvXPath.ArgvOrg = argv[optind - 1];
                  ArgvXPath.Argv = argv[optind - 1];
                  SourceDirList.push_back(ArgvXPath);

                  gSlurp |= FileInfoContainer::eSlurpAfterCalc;
                }
                else
                {
                  fwprintf (gStdOutFile, L"'%s' can not be slurped.\n", argv[optind - 1]);
                }
              }
              break;

              // --DumpAfterCalc
              // 
              case cBaseJustLongOpts + 0x04:
              {
                _ArgvPath       ArgvXPath;
                ArgvXPath.ArgvOrg = argv[optind - 1];
                ArgvXPath.Argv = argv[optind - 1];
                SourceDirList.push_back(ArgvXPath);

                gSlurp |= FileInfoContainer::eDumpAfterCalc;
              }
              break;


              // --whichgroups
              // 
              case cBaseJustLongOpts + 0x05:
              {
                if (NULL == optarg)
                {
                  gWhichGroups = DupeGroup::eNew;
                }
                else
                {
                  int LogLev = _wtoi(argv[optind - 1]);
                  switch(LogLev)
                  {
                    case 0:
                      gLogLevel = DupeGroup::eNew;
                      break;

                    case 1:
                      gLogLevel = DupeGroup::eOld;
                      break;

                  }
                }
              }
              break;

#if 0
              case cBaseJustLongOpts + 0x00:
              {
              }
              break;
#endif
            }
          }

        }
      }

      // Delete wide options
      for (i = 0; i < argc; i++)
        delete[] a_argv[i];
      delete[] a_argv;


      dc.SetFlags(gLogLevel);
      dc.SetOutputFile(gStdOutFile);

      // dc.SetFlags(FileInfoContainer::eDebugDupeMerge);

      // By default always sort by filesize
      if (gSortMode == eSortBySize)
        dc.SetFlags (FileInfoContainer::eSortBySize);
      else
        dc.ClearFlags (FileInfoContainer::eSortBySize);

      if (Traditional)
        dc.SetFlags(FileInfoContainer::eTraditional);

      if (argc == 1)
      {
        Usage ();
        Exit (1);
      }

      if (!gAutomatedTest)
        fwprintf (gStdOutFile, L"DupeMerge %S\n", VERSION_NUMBER);

      // Check if one valid path is among the arguments
      i = optind;
      bool	PathValid = true;
      while (i < argc)
      {
        struct _stat	buf;
        int r = _wstat (argv[i], &buf);
        if (r >= 0)
        {
          _ArgvPath       ArgvXPath;
          ArgvXPath.ArgvOrg = argv[i];
          ArgvXPath.Argv = argv[i];


          // Check if path is NTFS based
          if (!CheckFileSystemType(argv[i], &ArgvXPath.DriveType))
          {
            fwprintf (gStdOutFile, L"%s is not a NTFS partition.\n", argv[i]);
            PathValid = false;
            break;
          }


          SourceDirList.push_back(ArgvXPath);
        }
        else
        {
          fwprintf (gStdOutFile, L"%s not found.\n", argv[i]);
          PathValid = false;
        }

        i++;
      }

      if (!PathValid)
        return -1;

      CopyStatistics	DupeMergeStatistics;

      _PathNameStatusList PathNameStatusList;
      AsyncContext  DupeMergeContext;

      dc.DupeMerge(
        SourceDirList, 
        &IncludeFileList, 
        &IncludeDirList, 
        &ExcludeFileList, 
        &ExcludeDirList, 
        gSlurp, 
        &DupeMergeStatistics, 
        &PathNameStatusList, 
        &DupeMergeContext
      );

      DWORD			r;
      CopyStatistics		SnapShot;
      int				CurrentState = CopyStatistics::eVoid;
      do
      {
        r = WaitForSingleObject (DupeMergeContext.m_WaitEvent, 500);
        DupeMergeStatistics.Update(SnapShot);

        double		percent = 0;
        if (SnapShot.m_DupeTotalBytesToHash > 0)
          percent = ((100 / (double) (signed __int64) SnapShot.m_DupeTotalBytesToHash)) * (double) (signed __int64) SnapShot.m_DupeCurrentBytesHashed;

        StatisticsEvent	de;
        int				r = DupeMergeStatistics.GetDupeEvent (de);

        // Time calculations
        FILETIME64 StartTime, EndTime;
        SystemTimeToFileTime(&DupeMergeStatistics.m_StartTime, &StartTime.FileTime);
        SystemTimeToFileTime(&de.m_time, &EndTime.FileTime);

        FILETIME64 TotalDuration;
        TotalDuration.ul64DateTime = EndTime.ul64DateTime - StartTime.ul64DateTime;

        SYSTEMTIME stTotalDuration;
        FileTimeToSystemTime(&TotalDuration.FileTime, &stTotalDuration);

        CurrentState = de.m_state;
        switch (CurrentState)
        {
        case CopyStatistics::eVoid:
          break;

        case CopyStatistics::eScanning:
          {
            if (!gAutomatedTest)
            {
              fwprintf (gStdOutFile,
                L"Scanning elapsed %02d:%02d:%02d.%03d, %18I64d matching files\r",
                stTotalDuration.wHour,
                stTotalDuration.wMinute,
                stTotalDuration.wSecond,
                stTotalDuration.wMilliseconds,
                SnapShot.m_FilesSelected
                );
              fflush(gStdOutFile);
            }

          }
          break;

        case CopyStatistics::eScanned:
          {
            FILETIME64 EventTime;
            SystemTimeToFileTime(&de.m_time, &EventTime.FileTime);
            FILETIME64 CurrentDuration;
            CurrentDuration.ul64DateTime = EventTime.ul64DateTime - StartTime.ul64DateTime;

            SYSTEMTIME stCurrentDuration;
            FileTimeToSystemTime(&CurrentDuration.FileTime, &stCurrentDuration);

            if (!gAutomatedTest)
              fwprintf (gStdOutFile,
              L"Scanning finished %02d:%02d:%02d.%03d, %18I64d matching files\n",
              stCurrentDuration.wHour,
              stCurrentDuration.wMinute,
              stCurrentDuration.wSecond,
              stCurrentDuration.wMilliseconds,
              SnapShot.m_FilesSelected
              );
            else
              fwprintf (gStdOutFile,
              L"Scanning finished %18I64d total files\n",
              SnapShot.m_FilesSelected
              );

            fflush(gStdOutFile);
          }
          break;

        case CopyStatistics::ePreparation:
          {
            if (!gAutomatedTest)
            {
              fwprintf (gStdOutFile,
                L"Preparation elapsed %02d:%02d:%02d.%03d,  % #3.2f%%                 \r",
                stTotalDuration.wHour,
                stTotalDuration.wMinute,
                stTotalDuration.wSecond,
                stTotalDuration.wMilliseconds,
                percent
                );
              fflush(gStdOutFile);
            }
          }
          break;

        case CopyStatistics::ePreparated:
          {
            FILETIME64 EventTime;
            SystemTimeToFileTime(&de.m_time, &EventTime.FileTime);
            FILETIME64 CurrentDuration;
            CurrentDuration.ul64DateTime = EventTime.ul64DateTime - StartTime.ul64DateTime;

            SYSTEMTIME stCurrentDuration;
            FileTimeToSystemTime(&CurrentDuration.FileTime, &stCurrentDuration);

            if (!gAutomatedTest)
            {
              fwprintf (gStdOutFile,
                L"Preparation finished %02d:%02d:%02d.%03d,                          \n",
                stCurrentDuration.wHour,
                stCurrentDuration.wMinute,
                stCurrentDuration.wSecond,
                stCurrentDuration.wMilliseconds
                );
            }

            fflush(gStdOutFile);
          }
          break;

        case CopyStatistics::eDupeMerge:
          {
            if (!gAutomatedTest && !gListOnly)
            {
              fwprintf (gStdOutFile,
                L"Merging dupes: elapsed %02d:%02d:%02d.%03d,                       \r",
                stTotalDuration.wHour,
                stTotalDuration.wMinute,
                stTotalDuration.wSecond,
                stTotalDuration.wMilliseconds
                );

              fflush(gStdOutFile);
            }
          }
          break;

        case CopyStatistics::eDupeMerged:
          {
            if (!gListOnly)
            {
              FILETIME64 EventTime;
              SystemTimeToFileTime(&de.m_time, &EventTime.FileTime);
              FILETIME64 CurrentDuration;
              CurrentDuration.ul64DateTime = EventTime.ul64DateTime - StartTime.ul64DateTime;

              SYSTEMTIME stCurrentDuration;
              FileTimeToSystemTime(&CurrentDuration.FileTime, &stCurrentDuration);

              if (!gAutomatedTest)
              {
                fwprintf (gStdOutFile,
                  L"Merging dupes finished %02d:%02d:%02d.%03d,                      \n",
                  stCurrentDuration.wHour,
                  stCurrentDuration.wMinute,
                  stCurrentDuration.wSecond,
                  stCurrentDuration.wMilliseconds
                );
              }
              else
              {
                fwprintf (gStdOutFile, L"Merging dupes finished\n");
              }

              fflush(gStdOutFile);
            }
          }
          break;
        }
      }
      while (CurrentState != CopyStatistics::eFinished);

      // Analyse the result of the whole process
      r = AnalysePathNameStatus(gStdOutFile, PathNameStatusList, true, gLogLevel, gJson, NULL);

      if (gLogLevel <= FileInfoContainer::eLogChanged)
      {
        dc.OutputDupes (DupeGroup::eNew);

        // optional more output
        if (gWhichGroups == DupeGroup::eOld)
        {
          fwprintf (gStdOutFile, L"-----\n");
          dc.OutputDupes (DupeGroup::eOld);
        }
      }

      PrintDupeMergeCopyStats(gStdOutFile, DupeMergeStatistics, gListOnly, gAutomatedTest);

      // Cleanup
      IncludeFileList.clear();
      ExcludeFileList.clear();
      SourceDirList.clear();
      DeletePathNameStatusList(PathNameStatusList);
      dc.Dispose(NULL, &DupeMergeStatistics);

    } 
    // This bracket is only here so that we can leave a scope and all the automatic variables 
    // are released before Exit(). Otherwise we will get memory leak warnings in the debugger
    //
    Exit(0);
  }

#ifdef __cplusplus
}
#endif // __cplusplus
