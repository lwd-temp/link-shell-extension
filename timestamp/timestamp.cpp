
#include "stdafx.h"
#include "resource.h"
#include <winioctl.h>

#include "timestamp.h"


const char						exec_name[] = "timestamp";
bool                  gFileTimeFormat = false;
bool                  gRecursive = false;

static struct option	long_options[] =
{
  { "writetime", no_argument, NULL, 'w' },
  { "accesstime", no_argument, NULL, 'a' },
  { "creationtime", no_argument, NULL, 'c' }, 
  { "filetime", no_argument, NULL, 'f' }, 
  { "backup", required_argument, NULL, 'b' }, 
  { "ctime", required_argument, NULL, 't' }, 
  { "xattr", required_argument, NULL, 'x' }, 
  { "readfile", required_argument, NULL, 'r' }, 
  { "fileid", required_argument, NULL, 'i' }, 
  { "tell", no_argument, NULL, 'e' }, 
  { "eaprobe", required_argument, NULL, 'p' }, 
  { "eawrite", required_argument, NULL, 'P' }, 
  { "easave", required_argument, NULL, 'S' }, 
  { "streamprobe", required_argument, NULL, 'y' }, 
  { "streamwrite", required_argument, NULL, 'Y' }, 
  { "sparsemap", required_argument, NULL, 'R' }, 
  { "sparsecreate", required_argument, NULL, 'Q' },
  { "recursive", no_argument, NULL, 'X' }, 
  { "attr", required_argument, NULL, 'A' }, 
  { 0, 0, 0, 0 }
};


void
Usage()
{
  _tprintf (_T("Timestamp 1V37. Set a timestamp on one or more files\n"));
	_tprintf (_T("Usage. timestamp [options] [commands] file(s)\n\n"));
	_tprintf (_T("Commands\n"));
	_tprintf (_T("  -A, --attr [+|-(anshr)] SRCFILE                      Show/Set file attributes\n"));
	_tprintf (_T("  -b, --backup SRCFILE DSTFILE(s)    Copy all stamps from source to destination\n"));
	_tprintf (_T("  -t, --ctime HEXNUM FILE(s)               Specify time as CTIME in hexadecimal\n"));
	_tprintf (_T("  -p, --eaprobe SRCFILE                          Check if a file has EA records\n"));
	_tprintf (_T("  -S, --easave SRCFILE                             Save the EA records to files\n"));
	_tprintf (_T("  -P, --eawrite DSTFILE FILE1 FILE2 FILE3 ...    Check if a file has EA records\n"));
	_tprintf (_T("  -i, --fileid  SRCFILE                            Prints the file-id of a file\n"));
	_tprintf (_T("  -r, --readfile  SRCFILE                Read file to update hardlink timestamp\n"));
  _tprintf (_T("  -R, --sparsemap SRCFILE                                  Print the sparse map\n"));
  _tprintf (_T("  -Q, --sparsecreate SRCFILE                              Create  a sparse file\n"));
  _tprintf (_T("  -y, --streamprobe SRCFILE:StreamName  Check if a file has alternative streams\n"));
  _tprintf (_T("  -Y, --streamwrite text SRCFILE:StreamName    Write text to alternative stream\n"));
	_tprintf (_T("  -x, --xattr SRCFILE                                  Show extended attributes\n"));
	_tprintf (_T("\nOptions\n"));
	_tprintf (_T("  -a, --accesstime\n"));
	_tprintf (_T("  -c, --creationtime\n"));
	_tprintf (_T("  -f, --filetime                           Specify format of timestamp printout\n"));
	_tprintf (_T("  -e, --tell                                             Print the current time\n\n"));
	_tprintf (_T("  -w, --writetime\n"));
	_tprintf (_T("  -X, --recursive\n"));
	_tprintf (_T("Samples\n"));
	_tprintf (_T("  timestamp file.txt \n"));
	_tprintf (_T("  timestamp --accesstime file.txt\n"));
	_tprintf (_T("  timestamp --writetime file.txt\n"));
	_tprintf (_T("  timestamp --accesstime --writetime --creationtime file.txt\n"));
	_tprintf (_T("  timestamp --writetime --creationtime --backup sourcefile.txt file.txt\n"));
	_tprintf (_T("  timestamp --writetime --ctime 4e1402f1 file.txt\n"));
	_tprintf (_T("  timestamp --filetime --writetime file.txt\n"));
	_tprintf (_T("  timestamp --backup sourcefile.txt destfile.txt\n"));
	_tprintf (_T("  timestamp --attr +s+h+r destfile.txt\n"));
}

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
  // Note that LONGLONG is a 64-bit value
  LONGLONG ll;

  ll = Int32x32To64(t, 10000000) + 116444736000000000;
  pft->dwLowDateTime = (DWORD)ll;
  pft->dwHighDateTime = ll >> 32;
}

void
TimeStamp(
  wchar_t*  a_FileName, 
  wchar_t*  a_BackupFile, 
  wchar_t*  a_CtimeString, 
  int       a_SetStamps
)
{
  if (a_SetStamps & (SET_WRITETIME|SET_ACCESSTIME|SET_CREATIONTIME|SET_CTIME|SET_BACKUP))
  {
    SYSTEMTIME st;
    FILETIME WriteTime, AccessTime, CreationTime;
    BY_HANDLE_FILE_INFORMATION	FileInformation;

    if (a_SetStamps & SET_CTIME)
    {
      time_t  TimeVal;
      swscanf(a_CtimeString, L"%I64x", &TimeVal);
      UnixTimeToFileTime(TimeVal, &WriteTime);
      UnixTimeToFileTime(TimeVal, &AccessTime);
      UnixTimeToFileTime(TimeVal, &CreationTime);
    }
    else
    {
      if (a_SetStamps & SET_BACKUP)
      {
        wchar_t FullPath[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(a_BackupFile, HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE h = CreateFile(FullPath, // a_BackupFile,
          FILE_READ_ATTRIBUTES,
          FILE_SHARE_READ,
          NULL, 
          OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL,
          NULL);

        if (INVALID_HANDLE_VALUE != h)
        {
          GetFileInformationByHandle(h, &FileInformation);
          WriteTime = FileInformation.ftLastWriteTime;
          AccessTime = FileInformation.ftLastAccessTime;
          CreationTime = FileInformation.ftCreationTime;

          CloseHandle(h);
        }
        else
        {
          wprintf (L"Failed to read %s\n", FullPath);
        }
      }
      else
      {
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &WriteTime);
        SystemTimeToFileTime(&st, &AccessTime);
        SystemTimeToFileTime(&st, &CreationTime);
      }
    }

    HANDLE h = CreateFileW(a_FileName,
      FILE_WRITE_ATTRIBUTES,
      FILE_SHARE_READ,
      NULL, 
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL,
      NULL);

    if (INVALID_HANDLE_VALUE != h ) 
    {
      GetFileInformationByHandle(h, &FileInformation);

      if (a_SetStamps & SET_WRITETIME)
        FileInformation.ftLastWriteTime = WriteTime;

      if (a_SetStamps & SET_ACCESSTIME)
        FileInformation.ftLastAccessTime = AccessTime;

      if (a_SetStamps & SET_CREATIONTIME)
        FileInformation.ftCreationTime = CreationTime;

      SetFileTime(h,
        &FileInformation.ftCreationTime,
        &FileInformation.ftLastAccessTime,
        &FileInformation.ftLastWriteTime
      );

      CloseHandle(h);
    }
    else
    {
      wprintf (L"Failed to open %s\n", a_FileName);
    }


  }
  else // if (a_SetStamps)
  {
    // Just print the attributes
    WIN32_FILE_ATTRIBUTE_DATA FileInfo;
    BOOL b = GetFileAttributesExW(a_FileName, GetFileExInfoStandard, &FileInfo);

    if (b)
    {
      SYSTEMTIME stTimestampWriteUTC, stTimestampWriteLocal;
      BOOL bConv = FileTimeToSystemTime((LPFILETIME)&FileInfo.ftLastWriteTime, &stTimestampWriteUTC);
      SystemTimeToTzSpecificLocalTime(NULL, &stTimestampWriteUTC, &stTimestampWriteLocal);

      SYSTEMTIME stTimestampCreateUTC, stTimestampCreateLocal;
      bConv = FileTimeToSystemTime((LPFILETIME)&FileInfo.ftCreationTime, &stTimestampCreateUTC);
      SystemTimeToTzSpecificLocalTime(NULL, &stTimestampCreateUTC, &stTimestampCreateLocal);

      SYSTEMTIME stTimestampAccessUTC, stTimestampAccessLocal;
      bConv = FileTimeToSystemTime((LPFILETIME)&FileInfo.ftLastAccessTime, &stTimestampAccessUTC);
      SystemTimeToTzSpecificLocalTime(NULL, &stTimestampAccessUTC, &stTimestampAccessLocal);

      if (gFileTimeFormat)
      {
        if ( a_SetStamps & SET_PRINT_FILENAME)
          wprintf (L"\"%s\"  ", a_FileName);

        wprintf (L"W:%08x-%08x  C:%08x-%08x  A:%08x-%08x", 
          FileInfo.ftLastWriteTime.dwHighDateTime, 
          FileInfo.ftLastWriteTime.dwLowDateTime, 
          FileInfo.ftCreationTime.dwHighDateTime, 
          FileInfo.ftCreationTime.dwLowDateTime, 
          FileInfo.ftLastAccessTime.dwHighDateTime,
          FileInfo.ftLastAccessTime.dwLowDateTime);

        if ( a_SetStamps & SET_PRINT_FILENAME)
          wprintf (L"  %08x  %08x  %08x", FileInfo.dwFileAttributes, FileInfo.nFileSizeHigh, FileInfo.nFileSizeLow);

        wprintf (L"\n");
      }
      else
      {
        if ( a_SetStamps & SET_PRINT_FILENAME)
          wprintf (L"\"%s\"  ", a_FileName);

        wprintf (L"W:%04d-%02d-%02d %02d:%02d:%02d.%03d  C:%04d-%02d-%02d %02d:%02d:%02d.%03d  A:%04d-%02d-%02d %02d:%02d:%02d.%03d",
          stTimestampWriteLocal.wYear,
          stTimestampWriteLocal.wMonth,
          stTimestampWriteLocal.wDay,
          stTimestampWriteLocal.wHour,
          stTimestampWriteLocal.wMinute,
          stTimestampWriteLocal.wSecond,
          stTimestampWriteLocal.wMilliseconds,

          stTimestampCreateLocal.wYear,
          stTimestampCreateLocal.wMonth,
          stTimestampCreateLocal.wDay,
          stTimestampCreateLocal.wHour,
          stTimestampCreateLocal.wMinute,
          stTimestampCreateLocal.wSecond,
          stTimestampCreateLocal.wMilliseconds,

          stTimestampAccessLocal.wYear,
          stTimestampAccessLocal.wMonth,
          stTimestampAccessLocal.wDay,
          stTimestampAccessLocal.wHour,
          stTimestampAccessLocal.wMinute,
          stTimestampAccessLocal.wSecond,
          stTimestampAccessLocal.wMilliseconds
        );

        if ( a_SetStamps & SET_PRINT_FILENAME)
          wprintf (L"  %08x  %08x  %08x", FileInfo.dwFileAttributes, FileInfo.nFileSizeHigh, FileInfo.nFileSizeLow);

        wprintf (L"\n");
      }
    }
    else
    {
      wprintf (L"Failed to open %s", a_FileName);
    }
  } // if (a_SetStamps)
}


int XTimeStamp_recursive (
  wchar_t*          aSrcPath, 
  XDelStatistics&   rXDelStatistics,
  wchar_t*          a_BackupFile,
  wchar_t*          a_CtimeString,
  int               a_SetStamps
)
{
  WIN32_FIND_DATAW		wfind;

  size_t sSrcLen = wcslen(aSrcPath);
  wcscat (aSrcPath, L"\\*.*");
  HANDLE sh = FindFirstFileW (aSrcPath, &wfind);
  aSrcPath[sSrcLen] = 0x00;
  if (INVALID_HANDLE_VALUE != sh)
    do
    {
      if (wfind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (wcscmp(wfind.cFileName, L"."))
        {
          if (wcscmp(wfind.cFileName, L".."))
          {
            sSrcLen = wcslen(aSrcPath);
            wcscat (aSrcPath, L"\\");
            wcscat (aSrcPath, wfind.cFileName);

            int ReparseType = ProbeReparsePoint(aSrcPath, NULL);
            switch (ReparseType)
            {
            case REPARSE_POINT_SYMBOLICLINK:
              {
                rXDelStatistics.SymbolicLinks++;
              }
              break;

            case REPARSE_POINT_JUNCTION:
              {
                rXDelStatistics.Junctions++;
              }
              break;

            case REPARSE_POINT_FAIL:
              {
                // recursivley delete files
                XTimeStamp_recursive (aSrcPath, rXDelStatistics, a_BackupFile, a_CtimeString, a_SetStamps);
                rXDelStatistics.Directories++;
              }
              break;
            }
            aSrcPath[sSrcLen] = 0x00;
          }
        }
      }
      else
      {
        sSrcLen = wcslen(aSrcPath);
        wcscat (aSrcPath, L"\\");
        wcscat (aSrcPath, wfind.cFileName);

        TimeStamp(aSrcPath, a_BackupFile, a_CtimeString, a_SetStamps);

        rXDelStatistics.Count++;
        rXDelStatistics.Size += wfind.nFileSizeLow;

        aSrcPath[sSrcLen] = 0x00;
      }
    } while (FindNextFileW (sh, &wfind) > 0);
    FindClose(sh);

    return 42;
}

int XTimeStamp (
  wchar_t*          aSrcPath, 
  XDelStatistics&   rXDelStatistics,
  wchar_t*          a_BackupFile,
  wchar_t*          a_CtimeString,
  int               a_SetStamps
)
{
  a_SetStamps &= ~(SET_WRITETIME|SET_ACCESSTIME|SET_CREATIONTIME);
  a_SetStamps |= SET_PRINT_FILENAME;
  XTimeStamp_recursive (aSrcPath, rXDelStatistics, a_BackupFile, a_CtimeString, a_SetStamps);
	return 42;
}


#ifdef UNICODE
#  ifdef __cplusplus
extern "C"
{
#  endif // __cplusplus
  int
    wmain(int argc,
    TCHAR* argv[])
#else
int
main(int argc,
     TCHAR* argv[])
#endif
{
	InitCreateHardlink ();


  if (argc == 1)
    Usage();
  
  _CrtSetDebugFillThreshold(0);

  bool Done = false;
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
  wchar_t BackupFile[HUGE_PATH];
  wchar_t CtimeString[MAX_PATH];

  int SetStamps = 0;
  while (1)
  {
    char c = ultragetopt_tunable (
      argc, 
      a_argv, 
      "wacfb:t:ex:i:p:P:S:y:Y:R:Q:XA:", 
      long_options,
      (int*) 0,
      "=",
      "-",
      UGO_HYPHENARG | UGO_OPTIONPERMUTE | UGO_OPTIONALARG | UGO_SHORTOPTASSIGN | UGO_SEPARATEDOPTIONAL
      );
    if (c == EOF)
    {
      break;
    }

    switch (c)
    {
      // --writetime
      case 'w':
      {
        SetStamps |= SET_WRITETIME;
      }
      break;

      // --accesstime
      case 'a':
      {
        SetStamps |= SET_ACCESSTIME;
      }
      break;

      // --creationtime
      case 'c':
      {
        SetStamps |= SET_CREATIONTIME;
      }
      break;

      // --filetime
      case 'f':
      {
        gFileTimeFormat = true;
      }
      break;

      // --fileid
      case 'i':
      {
        HANDLE  ExistingFileHandle = CreateFile(
          argv[optind - 1], 
          FILE_READ_ATTRIBUTES,
          FILE_SHARE_READ, 
          NULL, 
          OPEN_EXISTING, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          BY_HANDLE_FILE_INFORMATION	FileInformation;
          GetFileInformationByHandle(ExistingFileHandle, &FileInformation);
          printf ("%08x-%08x\n", 
            FileInformation.nFileIndexHigh,
            FileInformation.nFileIndexLow
            );


          CloseHandle(ExistingFileHandle);
        }
      }
      break;

      // --eaprobe
      case 'p':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE  ExistingFileHandle = CreateFileW(
          FullPath, 
          FILE_READ_EA,
          FILE_SHARE_READ, 
          NULL, 
          OPEN_EXISTING, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          PFILE_FULL_EA_INFORMATION pFileEaInfo;
          DWORD dwFileEaInfoSize;
          int Result = GetEaRecords(ExistingFileHandle, &pFileEaInfo, &dwFileEaInfoSize);
          if ((ERROR_SUCCESS == Result) && dwFileEaInfoSize)
          {
            DumpEaRecords(pFileEaInfo);
            free(pFileEaInfo);
          }

          CloseHandle(ExistingFileHandle);
        }
        else
        {
          wprintf (L"Failed to open file for EA Record '%s'\n", argv[optind - 1]);
        }
        Done = true;
      }
      break;

      // --eawrite
      case 'P':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);
        HANDLE  ExistingFileHandle = CreateFileW(
          FullPath, 
          FILE_WRITE_EA,
          FILE_SHARE_READ, 
          NULL, 
          OPEN_EXISTING, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          // Calculate the size of the buffer
          //
          DWORD FileSize = 0;
          for (int ArgcIdx = optind; ArgcIdx < argc; ArgcIdx++)
          {
            HANDLE  PayLoad = CreateFile(
              argv[ArgcIdx], 
              FILE_READ_ATTRIBUTES,
              FILE_SHARE_READ, 
              NULL, 
              OPEN_EXISTING, 
              FILE_FLAG_BACKUP_SEMANTICS, 
              NULL
              );
            if (INVALID_HANDLE_VALUE != PayLoad)
            {
              DWORD fs = GetFileSize(PayLoad, NULL);
              if (fs > 0xffff) 
              {
                wprintf (L"Payload size '%s' exceeds 0xffff limit\n", argv[ArgcIdx]);
                CloseHandle(PayLoad);

                continue;
              }
              FileSize += fs;

              FileSize += wcslen(argv[ArgcIdx]) + 1;

              FileSize += sizeof(FILE_FULL_EA_INFORMATION);

              CloseHandle(PayLoad);
            }
            else
            {
              wprintf (L"Failed to open EA Record Payload '%s'\n", argv[ArgcIdx]);
            }
          }

          if (0 == FileSize)
          {
            printf ("EA Record Payloads missing\n");
            Done = true;
            break;
          }

          // Assemble EA Records
          //
          PFILE_FULL_EA_INFORMATION pCurrentEaFileInfo = (PFILE_FULL_EA_INFORMATION)malloc(FileSize);
          PFILE_FULL_EA_INFORMATION pEaFileInfo = pCurrentEaFileInfo;
          ZeroMemory(pCurrentEaFileInfo, FileSize);
          for (int ArgcIdx = optind; ArgcIdx < argc; ArgcIdx++)
          {
            HANDLE  PayLoad = CreateFile(
              argv[ArgcIdx], 
              FILE_READ_ATTRIBUTES | GENERIC_READ,
              FILE_SHARE_READ, 
              NULL, 
              OPEN_EXISTING, 
              FILE_FLAG_BACKUP_SEMANTICS, 
              NULL
              );
            if (INVALID_HANDLE_VALUE != PayLoad)
            {
              // Create FileMapping
              //
              DWORD FileSize;
              FileSize = GetFileSize(PayLoad, NULL);
              HANDLE hFilemap = CreateFileMapping(
                PayLoad,
                NULL,
                PAGE_READONLY,
                0,
                FileSize,
                NULL
                );

              LPVOID pMemoryView = MapViewOfFileEx(
                hFilemap, 
                FILE_MAP_READ, 
                0, 
                0, 
                FileSize,
                NULL
                );

              char FullPath[HUGE_PATH];
              char* Filename;
              GetFullPathNameA(a_argv[ArgcIdx], HUGE_PATH, FullPath, &Filename);
              pCurrentEaFileInfo = AddEaRecords(pCurrentEaFileInfo, Filename, pMemoryView, FileSize);

              UnmapViewOfFile(pMemoryView);
              CloseHandle(hFilemap);

              CloseHandle(PayLoad);
            }
          }

          int iResult = SetEaRecords(ExistingFileHandle, pEaFileInfo, FileSize);
          if (ERROR_SUCCESS != iResult)
            wprintf (L"Failed to write EA REcord for '%s'\n", argv[optind - 1]);
          else
            DumpEaRecords(pEaFileInfo);

          CloseHandle(ExistingFileHandle);
        }
        Done = true;
      }
      break;

      // --easave
      case 'S':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE  ExistingFileHandle = CreateFileW(
          FullPath, 
          FILE_READ_EA,
          FILE_SHARE_READ, 
          NULL, 
          OPEN_EXISTING, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          PFILE_FULL_EA_INFORMATION pFileEaInfo;
          DWORD dwFileEaInfoSize;
          int Result = GetEaRecords(ExistingFileHandle, &pFileEaInfo, &dwFileEaInfoSize);
          if ((ERROR_SUCCESS == Result) && dwFileEaInfoSize)
          {
            SaveEaRecords(pFileEaInfo);
            free(pFileEaInfo);
          }

          CloseHandle(ExistingFileHandle);
        }
        else
        {
          wprintf (L"Failed to open file for EA Record '%s'\n", argv[optind - 1]);
        }
        Done = true;
      }
      break;

      // --streamprobe
      case 'y':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE  ExistingFileHandle = CreateFileW(
          FullPath, 
          FILE_GENERIC_READ,
          FILE_SHARE_READ, 
          NULL, 
          OPEN_EXISTING, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          wchar_t StreamData[HUGE_PATH + 10];
          DWORD BytesRead;
          ReadFile(ExistingFileHandle, StreamData, HUGE_PATH, &BytesRead, NULL);
          StreamData[BytesRead/2 + 1] = 0x00;
          wprintf (L"%lS\n", StreamData);

          CloseHandle(ExistingFileHandle);
        }
        else
        {
          wprintf (L"Failed to open Alternative Stream(%08x) '%s'\n", GetLastError(), FullPath);
        }
        Done = true;
      }
      break;


      // --streamwrite
      case 'Y':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE  ExistingFileHandle = CreateFileW(
          FullPath, 
          FILE_GENERIC_WRITE,
          FILE_SHARE_READ,
          NULL, 
          OPEN_ALWAYS, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          DWORD BytesWritten;
          WriteFile(ExistingFileHandle, a_argv[optind - 1], strlen(a_argv[optind - 1]), &BytesWritten, NULL);

          CloseHandle(ExistingFileHandle);
        }
        else
        {
          wprintf (L"Failed to write to Alternative Stream(%08x) '%s'\n", GetLastError(), FullPath);
        }
        Done = true;
      }
      break;

      // --sparsemap
      case 'R':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE  ExistingFileHandle = CreateFileW(
          FullPath, 
          FILE_GENERIC_READ,
          FILE_SHARE_READ,
          NULL, 
          OPEN_ALWAYS, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {

          LARGE_INTEGER StreamTotalSize;
          BOOL bResult = GetFileSizeEx(ExistingFileHandle, &StreamTotalSize);
          if (FALSE == bResult)
            return ERROR_ACCESS_DENIED;

          // Copy the stream content
          // 
          FILE_ALLOCATED_RANGE_BUFFER QueryRange;
          QueryRange.FileOffset.QuadPart = 0;
          QueryRange.Length.QuadPart = StreamTotalSize.QuadPart;

          PFILE_ALLOCATED_RANGE_BUFFER pRanges;
          DWORD nBytes;
          DWORD BufferSize = 0;
          DWORD LastError;

          // Retrieve sparse map
          //
          do
          {
            BufferSize += 0x1000;
            pRanges = (PFILE_ALLOCATED_RANGE_BUFFER)malloc(BufferSize);

            BOOL br = ::DeviceIoControl(
              ExistingFileHandle, 
              FSCTL_QUERY_ALLOCATED_RANGES,
              &QueryRange, sizeof(QueryRange),
              pRanges, BufferSize,
              &nBytes,
              NULL
              );

            LastError = GetLastError();
            if (ERROR_MORE_DATA == LastError) 
              free(pRanges);

          } while (ERROR_MORE_DATA == LastError);

          int nRanges = nBytes / sizeof(FILE_ALLOCATED_RANGE_BUFFER);

          for (int range = 0; range < nRanges; range++)
            printf ("%08x: %I64x\n", range, pRanges[range]);

          free(pRanges);
          CloseHandle(ExistingFileHandle);
        }
        else
        {
          wprintf (L"Failed to open sparse file(%08x) '%s'\n", GetLastError(), FullPath);
        }

      }
      break;

      // --sparsecreate
      case 'Q':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE  ExistingFileHandle = CreateFileW(
          FullPath, 
          GENERIC_WRITE,
          FILE_SHARE_READ,
          NULL, 
          OPEN_ALWAYS, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          DWORD dwTemp;
          DeviceIoControl(ExistingFileHandle, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwTemp, NULL);

          const int SPARSE_MAX_NUMBEROFRANGES = 20;
          const int SPARSE_GAP_SIZE = 10000000;
          const int SPARSE_DATA_SIZE = 64000;

          wchar_t SparseDataBuffer[SPARSE_DATA_SIZE];
          for (int i = 0; i < SPARSE_DATA_SIZE; i++)
            SparseDataBuffer[i] = i;

          LARGE_INTEGER WritePosition;
          WritePosition.QuadPart = 0;

          FILE_ZERO_DATA_INFORMATION fzdi;
          for (int NumberOfRanges = 0; NumberOfRanges < SPARSE_MAX_NUMBEROFRANGES; NumberOfRanges++)
          {
            BOOL b = SetFilePointerEx(ExistingFileHandle, WritePosition, &WritePosition, FILE_BEGIN);

            fzdi.FileOffset.QuadPart = WritePosition.QuadPart + SPARSE_DATA_SIZE;
            WritePosition.QuadPart += SPARSE_GAP_SIZE;
            fzdi.BeyondFinalZero.QuadPart = WritePosition.QuadPart;

            DeviceIoControl(ExistingFileHandle, FSCTL_SET_ZERO_DATA, &fzdi, sizeof(fzdi), NULL, 0, &dwTemp, NULL);

            DWORD dwWritten;
            b = WriteFile(ExistingFileHandle, SparseDataBuffer, SPARSE_DATA_SIZE, &dwWritten, NULL);

            for (int i = 0; i < SPARSE_DATA_SIZE; i++)
              SparseDataBuffer[i]++;
          }

          CloseHandle(ExistingFileHandle);
        }
        else
        {
          wprintf (L"Failed to create sparse file(%08x) '%s'\n", GetLastError(), FullPath);
        }

      }
      break;

      // --backup
      case 'b':
      {
        SetStamps |= SET_BACKUP|SET_WRITETIME|SET_ACCESSTIME|SET_CREATIONTIME;
        wcscpy_s(BackupFile, HUGE_PATH, argv[optind - 1]);
      }
      break;

      // --ctime
      case 't':
      {
        SetStamps |= SET_CTIME;
        wcscpy_s(CtimeString, MAX_PATH, argv[optind - 1]);
      }
      break;

      // --tell
      case 'e':
      {
        SYSTEMTIME time;
        GetLocalTime(&time);
        printf ("%4d-%02d-%02d %02d:%02d:%02d.%03d\n", 
          time.wYear,
          time.wMonth,
          time.wDay,
          time.wHour,
          time.wMinute,
          time.wSecond,
          time.wMilliseconds
          );

      }
      break;

      // --xattr
      case 'x':
      {
        DWORD BytesReturned;
        USHORT CompressionMode = COMPRESSION_FORMAT_DEFAULT;

        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE  ExistingFileHandle = CreateFile(
          FullPath, // argv[optind - 1], 
          FILE_READ_ATTRIBUTES,
          FILE_SHARE_READ, 
          NULL, 
          OPEN_EXISTING, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
          );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          BOOL b = DeviceIoControl(ExistingFileHandle,
            FSCTL_GET_COMPRESSION,
            NULL,
            0,
            &CompressionMode,
            sizeof(CompressionMode),
            &BytesReturned,
            NULL
            );

          char CompState = '?';
          if (b)
          {
            if (CompressionMode != COMPRESSION_FORMAT_NONE)
              CompState = 'C';
            else
              CompState = ' ';
          }

          char EncryptionState = ' ';
          char SparseState = ' ';
          DWORD FileAttributes = GetFileAttributes(FullPath);
          if (INVALID_FILE_ATTRIBUTES != FileAttributes )
          {
            if (FileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
              SparseState = 'S';

            if (FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
              EncryptionState = 'E';
          }

          printf("%c%c%c %S\n", 
            CompState, 
            SparseState, 
            EncryptionState, 
            argv[optind - 1]
          );
          CloseHandle(ExistingFileHandle);
        }
        else
        {
          wprintf(L"Failed to open %s\n", argv[optind - 1]);
        }

        Done = true;
      }
      break;

      // --attr
      case 'A':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);

        if (optind == argc)
        {
          GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

          DWORD FileAttributes = GetFileAttributes(FullPath);
          if (INVALID_FILE_ATTRIBUTES != FileAttributes)
          {
            printf("%c%c%c%c%c%c%c%c %S\n", 
              FileAttributes & FILE_ATTRIBUTE_ARCHIVE ? 'A' : ' ', 
              FileAttributes & FILE_ATTRIBUTE_NORMAL ? 'N' : ' ', 
              FileAttributes & FILE_ATTRIBUTE_TEMPORARY ? 'T' : ' ', 
              FileAttributes & FILE_ATTRIBUTE_SYSTEM ? 'S' : ' ', 
              FileAttributes & FILE_ATTRIBUTE_HIDDEN ? 'H' : ' ', 
              FileAttributes & FILE_ATTRIBUTE_READONLY ? 'R' : ' ', 
              FileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED ? 'I' : ' ', 
              FileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 'D' : ' ', 
              argv[optind - 1]
            );
          }
          else
          {
            wprintf(L"Failed to open %s\n", argv[optind - 1]);
          }
        }
        else
        {
          GetFullPathName(argv[optind], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);
          DWORD FileAttributes = GetFileAttributes(FullPath);
          int plus = -1;
          if (INVALID_FILE_ATTRIBUTES != FileAttributes)
          {
            for (int i = 0; i < wcslen(argv[optind - 1]); i++)
            {
              switch (argv[optind - 1][i])
              {
                case '+':
                  plus = 1;
                break;

                case '-':
                  plus = 0;
                break;

                case 'a':
                case 'A':
                  switch (plus)
                  {
                    case 1:
                      FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
                    break;
                    case 0:
                      FileAttributes &= ~FILE_ATTRIBUTE_ARCHIVE;
                    break;
                  }
                  plus = -1;
                break;

                case 'n':
                case 'N':
                  switch (plus)
                  {
                    case 1:
                      FileAttributes |= FILE_ATTRIBUTE_NORMAL;
                    break;
                    case 0:
                      FileAttributes &= ~FILE_ATTRIBUTE_NORMAL;
                    break;
                  }
                  plus = -1;
                break;

                case 's':
                case 'S':
                  switch (plus)
                  {
                    case 1:
                      FileAttributes |= FILE_ATTRIBUTE_SYSTEM;
                    break;
                    case 0:
                      FileAttributes &= ~FILE_ATTRIBUTE_SYSTEM;
                    break;
                  }
                  plus = -1;
                break;

                case 'r':
                case 'R':
                  switch (plus)
                  {
                    case 1:
                      FileAttributes |= FILE_ATTRIBUTE_READONLY;
                    break;
                    case 0:
                      FileAttributes &= ~FILE_ATTRIBUTE_READONLY;
                    break;
                  }
                  plus = -1;
                break;

                case 'h':
                case 'H':
                  switch (plus)
                  {
                    case 1:
                      FileAttributes |= FILE_ATTRIBUTE_HIDDEN;
                    break;
                    case 0:
                      FileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
                    break;
                  }
                  plus = -1;
                break;

              }
            }
            SetFileAttributes(FullPath, FileAttributes);
          }
          else
          {
            wprintf(L"Failed to open %s\n", argv[optind]);
          }
        }
        Done = true;
      }
      break;

      // --readfile
      case 'r':
      {
        wchar_t FullPath[HUGE_PATH];
        wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], NULL);

        HANDLE h = CreateFile(FullPath, 
          GENERIC_READ,
          FILE_SHARE_READ,
          NULL, 
          OPEN_EXISTING,
          FILE_ATTRIBUTE_NORMAL,
          NULL);

        if (INVALID_HANDLE_VALUE != h)
        {
          char buffer[MAX_PATH];
          DWORD BytesRead;
          ReadFile(h, &buffer, MAX_PATH, &BytesRead, NULL);

          CloseHandle(h);

        }
        else
        {
          wprintf(L"Failed to open %s\n", argv[optind - 1]);
        }
        Done = true;
      }
      break;

      // --recursive
      case 'X':
      {
        gRecursive = true;
      }
      break;
    }
  }

  if (Done)
    exit(1);

  wchar_t Argv1[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
  wcscpy_s(Argv1, HUGE_PATH, PATH_PARSE_SWITCHOFF);
  // Apply this only if no options 'timestamp source dest' is applied
  for ( int ArgPos = optind; ArgPos < argc; ArgPos++)
  {
    int PathPos = 0;
    if (!IsVeryLongPath(argv[ArgPos]))
      PathPos = PATH_PARSE_SWITCHOFF_SIZE;
    GetFullPathName(argv[ArgPos], HUGE_PATH, &Argv1[PathPos], NULL);
    if (gRecursive)
    {
      XDelStatistics stat;
      XTimeStamp(Argv1, stat, BackupFile, CtimeString, SetStamps);
    }
    else
    {
      TimeStamp(Argv1, BackupFile, CtimeString, SetStamps);
    }
  }

  // Delete ANSI options
  for (i = 0; i < argc + 1; i++)
    delete[] a_argv[i];
  delete[] a_argv;

}


#ifdef __cplusplus
}
#endif // __cplusplus


