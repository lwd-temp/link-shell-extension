/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

// TODO ERROR: 'test\hardlinks' and '' are not on same volume, wenn der Pfad falsch ist

#include "stdafx.h"

#include "ln.h"

#pragma hdrstop
#pragma comment( lib, "advapi32.lib" )

extern _locale_t g_locale_t;

const char						exec_name[] = "ln";

static struct option	long_options[] =
{
  { "recursive", optional_argument, NULL, 'r' },
  { "enum", required_argument, NULL, 'e' },
  { "quiet", optional_argument, NULL, 'q' },
  { "copy", required_argument, NULL, 'c' },
  { "junction", required_argument, NULL, 'j' },
  { "noitcnuj", required_argument, NULL, 'J' },
  { "symbolic", no_argument, NULL, 's' },
  { "absolute", no_argument, NULL, 'a' },
  { "list", required_argument, NULL, 'l' },
  { "automated_test", no_argument, NULL, 'A' },
  { "move", required_argument, NULL, 'm' },
  { "mirror", required_argument, NULL, 'o' },
  { "deeppathcreate", required_argument, NULL, 'z' }, // undocumented, for automated test only
  { "deeppathdelete", required_argument, NULL, 'Z' }, // undocumented, for automated test only
  { "delorean", required_argument, NULL, 'd' },
  { "exclude", required_argument, NULL, 'x' },

  { "excludedir", required_argument, NULL, 'X' },
  { "excluderegexp", required_argument, NULL, 'p' },
  { "excluderegexpdir", required_argument, NULL, 'P' },
  { "unroll", optional_argument, NULL, 'u' },
  { "unrollregexp", optional_argument, NULL, 'U' },
  { "splice", optional_argument, NULL, 'i' },
  { "spliceregexp", optional_argument, NULL, 'I' },
  { "backup", no_argument, NULL, 'b' },
  { "help", no_argument, NULL, 'h' },

  { "source", required_argument, NULL, '\0' },
  { "deeppathcopy", required_argument, NULL, '\0' }, // undocumented, for automated test only
  { "skipfiles", no_argument, NULL, '\0' },
  { "traditional", no_argument, NULL, '\0' },
  { "smartrename", required_argument, NULL, '\0' },
  { "destination", required_argument, NULL, '\0' },
  { "datetime", no_argument, NULL, '\0' }, // undocumented

  { "output", required_argument, NULL, '\0' },
  { "truesize", required_argument, NULL, '\0' },
  { "adsdev", required_argument, NULL, '\0' }, // undocumented, for automated test only
  { "automated_traditional", no_argument, NULL, '\0' },// undocumented
  { "keepsymlinkrelation", no_argument, NULL, '\0' },
  { "timetolerance", required_argument, NULL, '\0' },
  { "include", required_argument, NULL, '\0' },
  { "includedir", required_argument, NULL, '\0' },
  { "includeregexp", required_argument, NULL, '\0' },
  { "includeregexpdir", required_argument, NULL, '\0' },
  { "dupemerge", no_argument, NULL, '\0' },
  { "probefs", required_argument, NULL, '\0' },
  { "enumvolumes", no_argument, NULL, '\0' },   // undocumented
  { "switchoffntfscheck", no_argument, NULL, '\0' },   // undocumented, for automated test only
  { "noads", no_argument, NULL, '\0' },
  { "noea", no_argument, NULL, '\0' },

  { "nosparsefile", no_argument, NULL, '\0' },   // undocumented
  { "nolocaluncresolve", no_argument, NULL, '\0' },   // undocumented, for automated test only
  { "anchor", required_argument, NULL, '\0' },
  { "utf", no_argument, NULL, '\0' },   // undocumented
  { "deloreanverbose", no_argument, NULL, '\0' },   // undocumented: Also prints the logging after the clone process
  { "deloreansleep", required_argument, NULL, '\0' }, // undocumented: Specify a time which should be waited after clone during delorean, due to ln.html#smb2redirectorcache
  { "hardlinklimit", required_argument, NULL, '\0' }, // undocumented: Specify the hardlink limit. By default 1023, for testing can be set seperatley. Used in Autoamted Test
  { "generatehardlinks", required_argument, NULL, '\0' }, // undocumented: Specify how many hardlinks are generated automatically. for automated test only
  { "1023safe", no_argument, NULL, '\0' },
  { "json", no_argument, NULL, '\0' }, // Output is in json format
  { "forcestats", no_argument, NULL, '\0' }, // undocumented, for automated test only. Show stats regardless of automated test
  { "progress", no_argument, NULL, '\0' }, // Shows the progress
  { "merge", required_argument, NULL, '\0' }, // Merges two delorean sets
  { "delete", required_argument, NULL, '\0' }, // Deletes a tree and take care of hardlinks
  { "supportfs", required_argument, NULL, '\0' },
  { "follow", optional_argument, NULL, '\0' },
 
  { "followregexp", optional_argument, NULL, '\0' },
  { 0, 0, 0, 0 }
};


// With symbolic options on the index of just long opts starts with
//
const int cBaseJustLongOpts = 0x19;

int							  gLogLevel = FileInfoContainer::eLogVerbose;
bool              gAutomatedTest{ false };
FILE*             gStdOutFile;
intptr_t          gStdOutHandle;
bool              gSwitchOffNtfsCheck{ false };
bool              gDupemerge{ false };
bool              gNoEa{ false };
bool              gNoSparseFile{ false };
bool              gNoAds{ false };
bool              gResolveUNC{ true };
bool              gDeloreanVerbose{ false };
int               gDeloreanSleep{ 0 };
bool              g1023safe{ false };
bool              gJson{ false };
bool              gProgress{ false };
const int         gUpdateIntervall{ 4 };

class	ln_EnumHardlinkSiblingsGlue : public EnumHardlinkSiblingsGlue
{
	public:
		virtual void Print(wchar_t* pSiblingFileName);
};

void 
ln_EnumHardlinkSiblingsGlue ::
Print(wchar_t* pSiblingFileName) 
{
	int LongPathIdx = 0;
  if (IsVeryLongPath(pSiblingFileName))
    LongPathIdx = PATH_PARSE_SWITCHOFF_SIZE;

  fwprintf (gStdOutFile, L"%s\n", &pSiblingFileName[LongPathIdx]);
}

int
HardlinkList(
  WCHAR*  fromFile,
  bool    aTradtional
)
{
  wchar_t	LinkName[HUGE_PATH + 2];
  DWORD LinkNameLength = HUGE_PATH;
  PWCHAR	FilePart;
  int RetVal = ERROR_SUCCESS;

  if (true == aTradtional || PathIsUNC(fromFile))
  {
    // Enumerate the home-grown way, but for UNC Path this is the only way to acchieve results
    FileInfoContainer	FileList;
    ln_EnumHardlinkSiblingsGlue	Glue;
    _PathNameStatusList PathNameStatusList;

    wchar_t uncPath[HUGE_PATH];
    wchar_t* pUncPath = nullptr;
    pUncPath = ResolveUNCPath(fromFile, uncPath);

    FileList.EnumHardlinkSiblings(pUncPath ? uncPath : fromFile, L"", &Glue, false, &PathNameStatusList, nullptr);

    DeletePathNameStatusList(PathNameStatusList);

    return RetVal;
  }

  // Enumerate with built-in functions
  int LinkNameIdx = 0;
  wchar_t FullName[HUGE_PATH];
  GetFullPathName(fromFile, HUGE_PATH, FullName, &FilePart);

  if (IsVeryLongPath(FullName))
    LinkNameIdx = PATH_PARSE_SWITCHOFF_SIZE;

  // Follow a possible subst chain, until we end up with e.g \Device\HardDisk\Volume4
  do
  {
    wcscpy_s(LinkName, HUGE_PATH, &FullName[LinkNameIdx]);
    LinkName[2] = 0x00;
    QueryDosDevice(LinkName, FullName, HUGE_PATH);
  }
  // a subst on a subst can be recognized by \??\ as prefix to e.g. \??\f:\tmp
  while (FullName[1] == '?');

  HANDLE FindHardLinkHandle = FindFirstFileNameW(fromFile, 0, &LinkNameLength, &LinkName[2]);
  if (INVALID_HANDLE_VALUE != FindHardLinkHandle)
  {
    do
    {
      fwprintf(gStdOutFile, L"%s\n", LinkName);
      LinkNameLength = HUGE_PATH;
    } while (FindNextFileNameW(FindHardLinkHandle, &LinkNameLength, &LinkName[2]));

    FindClose(FindHardLinkHandle);
  }
  else
  {
    // ERROR_INVALID_LEVEL is the error returned when not supported (a sym link to file on FAT32 or Samba server for example)
    DWORD LastError = GetLastError();
    size_t FromFileIdx = 0;
    if (IsVeryLongPath(fromFile))
      FromFileIdx = PATH_PARSE_SWITCHOFF_SIZE;
    fwprintf(gStdOutFile, L"!?d(0x%08x) %s\n", LastError, &fromFile[FromFileIdx]);
    RetVal = LastError;
  }

  return RetVal;
}

int
ShowJunction(
	_ArgvPath& aSource
)
{
	WCHAR	JunctionTarget[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
  wcscpy_s(JunctionTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);
  int ReparseType = ProbeReparsePoint(aSource.Argv.c_str(), &JunctionTarget[PATH_PARSE_SWITCHOFF_SIZE]);
  if (REPARSE_POINT_JUNCTION == ReparseType || REPARSE_POINT_MOUNTPOINT == ReparseType)
  {
    int PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
    // Check if the target was a junction or mountpoint
    if (IsVeryLongPath(&JunctionTarget[PATH_PARSE_SWITCHOFF_SIZE]))
      PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE * 2;

    DWORD SymlinkAttr = GetFileAttributes(&JunctionTarget[PathParseSwitchOffSize - PATH_PARSE_SWITCHOFF_SIZE]);
    if (SymlinkAttr == INVALID_FILE_ATTRIBUTES)
      fwprintf(gStdOutFile, L"%s -#-> %s\n", &aSource.Argv.at(PATH_PARSE_SWITCHOFF_SIZE), &JunctionTarget[PathParseSwitchOffSize]);
    else
      fwprintf(gStdOutFile, L"%s -> %s\n", &aSource.Argv.at(PATH_PARSE_SWITCHOFF_SIZE), &JunctionTarget[PathParseSwitchOffSize]);
  }
  else
    fwprintf(gStdOutFile, L"ERROR: %s is not a junction\n", aSource.ArgvOrg.c_str());
	return ReparseType;
}

int
DoJunction(
	PCWCH	aSource,
  PCWCH aSourceOrg,
	PCWCH	aTarget
)
{
  int r = CreateJunction(aSource, aTarget);
	fwprintf(gStdOutFile, L"%s -> %s\n", aSourceOrg, &aTarget[PATH_PARSE_SWITCHOFF_SIZE] );

	return r;
}

int
HardlinkEnum(
  _ArgvPath* fromFile
)
{
	FileInfoContainer	FileList;

	_PathNameStatusList PathNameStatusList;

  CopyStatistics	aStats;
  
  FileList.SetFlags(FileInfoContainer::eLogQuiet);
  FileList.SetOutputFile(gStdOutFile);

  _ArgvList EnumDestination;
  EnumDestination.push_back(*fromFile);
  FileList.FindHardLink (EnumDestination, 1, &aStats, &PathNameStatusList, nullptr);

  FileList.PrepareRefcounts();

  // TBD do the errorhandling based on PathNameStatusList

	fwprintf (gStdOutFile, L"Saturated Hardlinks\n---------------------\n");
	int	hardlinksgroups = FileList.FindDupes (0, PATH_PARSE_SWITCHOFF_SIZE);
	if (0 == hardlinksgroups)
		fwprintf (gStdOutFile, L"No saturated hardlinks found.\n\n");

	fwprintf (gStdOutFile, L"UnSaturated Hardlinks\n---------------------\n");
	int	pp = FileList.FindDupes (1, PATH_PARSE_SWITCHOFF_SIZE);
	pp += FileList.FindSingleUnsaturated (PATH_PARSE_SWITCHOFF_SIZE);

	if (0 == pp)
		fwprintf (gStdOutFile, L"No unsaturated hardlinks found.\n\n");

	hardlinksgroups += pp;


  DeletePathNameStatusList(PathNameStatusList);

	return hardlinksgroups;
}

#if defined _DEBUG
void TestPrintProgress()
{
  char nItems[MAX_PATH];
  FormatNumber(nItems, MAX_PATH, 73);

  SYSTEMTIME currentTime;
  GetLocalTime(&currentTime);

  FILETIME64 currentFileTime;
  SystemTimeToFileTime(&currentTime, &currentFileTime.FileTime);

  FILETIME64 pastFileTime;
  pastFileTime.ul64DateTime = currentFileTime.ul64DateTime - (320LL * 24 * 60 * 60 * 1000 * 1000 * 10);

  SYSTEMTIME timeLeft;
  FILETIME64 duationFileTime;

  duationFileTime.ul64DateTime = currentFileTime.ul64DateTime - pastFileTime.ul64DateTime;
  FileTimeToSystemTime(&duationFileTime.FileTime, &timeLeft);

  constexpr int DaysTillMonth[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
  int totalDays = timeLeft.wDay - 1 + (timeLeft.wMonth < 12 ? DaysTillMonth[timeLeft.wMonth - 1] : 0);

  // # 34%, Items 12345678901234, Time elapsed: 123d 02:03:04#
  // Max number of items: 99'999'999'999, which is far enough for the NTFS Limit, but ReFS might exceed 99Billions of files
  // For ReFs we would need 25 characters since it maximum is ‭9,223,372,036,854,775,807‬
  if (totalDays)
  {
    wprintf(L"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%3d%%, Items %14S, Time left: %3dd %02d:%02d:%02d",
      (int)48,
      nItems,
      totalDays,
      timeLeft.wHour,
      timeLeft.wMinute,
      timeLeft.wSecond
    );
  }
  else
  {
    wprintf(L"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%3d%%, Items %14S, Time left:      %02d:%02d:%02d",
      (int)48,
      nItems,
      timeLeft.wHour,
      timeLeft.wMinute,
      timeLeft.wSecond
    );
  }
}
#endif

void PrintProgress(__int64 aPercentage, ProgressPrediction& aProgressPrediction)
{
  SYSTEMTIME timeLeft;
  Effort  effort;
  // Only print progress if prediction was possible
  if (aProgressPrediction.TimeLeft(timeLeft, effort))
  {
    char nItems[MAX_PATH];
    FormatNumber(nItems, MAX_PATH, effort.m_Items.load());

    constexpr int DaysTillMonth[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    int totalDays = timeLeft.wDay - 1 + (timeLeft.wMonth < 12 ? DaysTillMonth[timeLeft.wMonth - 1] : 0);

    if (totalDays)
    {
      wprintf(L"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%3d%%, Items %14S, Time left: %3dd %02d:%02d:%02d",
        (int)aPercentage,
        nItems,
        totalDays,
        timeLeft.wHour,
        timeLeft.wMinute,
        timeLeft.wSecond
      );
    }
    else
    {
      wprintf(L"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%3d%%, Items %14S, Time left:      %02d:%02d:%02d",
        (int)aPercentage,
        nItems,
        timeLeft.wHour,
        timeLeft.wMinute,
        timeLeft.wSecond
      );
    }
  }
}

void PrintElapsed(ProgressPrediction& aProgressPrediction, __int64 aFakeEffort = -1)
{
  SYSTEMTIME timeLeft;
  Effort effort;
  aProgressPrediction.Duration(timeLeft, effort);

  char nItems[MAX_PATH];
  if (aFakeEffort < 0)
    FormatNumber(nItems, MAX_PATH, effort.m_Items.load());
  else
    FormatNumber(nItems, MAX_PATH, aFakeEffort);

  constexpr int DaysTillMonth[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
  int totalDays = timeLeft.wDay - 1 + (timeLeft.wMonth < 12 ? DaysTillMonth[timeLeft.wMonth - 1] : 0);

  if (totalDays)
  {
    wprintf(L"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b100%%, Items %14S, Time elapsed: %3dd %02d:%02d:%02d\n",
      nItems,
      totalDays,
      timeLeft.wHour,
      timeLeft.wMinute,
      timeLeft.wSecond
    );
  }
  else
  {
    wprintf(L"\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b100%%, Items %14S, Time elapsed:      %02d:%02d:%02d\n",
      nItems,
      timeLeft.wHour,
      timeLeft.wMinute,
      timeLeft.wSecond
    );
  }
}

void Enumerating(AsyncContext& aContext)
{
  // Print progress if async
  // 
  constexpr wchar_t     progressIndicator[] = { L'|', L'/', L'-', L'\\' };
  int ProgressCount = 0;
  if (gProgress)
    wprintf(L"Enumerating...   ");
  while (!aContext.Wait(250))
  {
    if (gProgress)
      wprintf(L"\b%c", progressIndicator[++ProgressCount % 4]);
  }

  wprintf(L"\b \n");
}

int
TrueSize(
  _ArgvList&                                aSourceDirList,
  _StringList*                              aIncludeFileList,
  _StringList*                              aIncludeDirList,
  _StringList*                              aExcludeFileList,
  _StringList*                              aExcludeDirList,
  bool                                      aSkipFiles,
  bool                                      aTraditional
)
{
  if (INVALID_FILE_ATTRIBUTES != aSourceDirList[0].FileAttribute)
  {
    _PathNameStatusList pathNameStatusList;
    CopyStatistics	stats;

    FileInfoContainer	FileList;
    FileList.SetOutputFile(gStdOutFile);
    GetLocalTime(&stats.m_StartTime);

    if (aTraditional)
      FileList.SetFlags(FileInfoContainer::eTraditional);

    FileList.SetIncludeList(aIncludeFileList);
    FileList.SetIncludeDirList(aIncludeDirList);
    FileList.SetExcludeList(aExcludeFileList);
    FileList.SetExcludeDirList(aExcludeDirList);

    if (aSkipFiles)
      FileList.SetFlags(FileInfoContainer::eSkipFiles);

    // Enumerate affected files
    AsyncContext    Context;
    AsyncContext*   pContext = nullptr;

    if (gProgress)
      pContext = &Context;
    int r = FileList.FindHardLink(aSourceDirList, 0, &stats, &pathNameStatusList, pContext);
    if (pContext)
      Enumerating(Context);

    // Calculate the true Size
    FileList.TrueSize(&stats, &pathNameStatusList, nullptr);

    GetLocalTime(&stats.m_EndTime);
    PrintTrueSizeCopyStats(gStdOutFile, stats, FileInfoContainer::eSmartCopy, gAutomatedTest, gJson);

#if defined PRINT_DISPOSE_DURATION
    // Free Memory - no need to go async as in mirror
    SYSTEMTIME DisposeStartTime, DisposeEndTime;
    GetSystemTime(&DisposeStartTime);
#endif

    DeletePathNameStatusList(pathNameStatusList);
    FileList.Dispose(nullptr, &stats);

#if defined PRINT_DISPOSE_DURATION
    GetSystemTime(&DisposeEndTime);

    // Time calculations for DisposeDuration
    FILETIME64 DisposeStart, DisposeEnd;
    SystemTimeToFileTime(&DisposeStartTime, &DisposeStart.FileTime);
    SystemTimeToFileTime(&DisposeEndTime, &DisposeEnd.FileTime);

    FILETIME64 DisposeDuration;
    DisposeDuration.ul64DateTime = DisposeEnd.ul64DateTime - DisposeStart.ul64DateTime;

    SYSTEMTIME stDisposeDuration;
    FileTimeToSystemTime(&DisposeDuration.FileTime, &stDisposeDuration);

    const int DisposeDurationStrLength = 64;
    char DisposeDurationStr[DisposeDurationStrLength];
    sprintf(DisposeDurationStr, DisposeDurationStrLength, "%02d:%02d:%02d.%03d",
      stDisposeDuration.wHour,
      stDisposeDuration.wMinute,
      stDisposeDuration.wSecond,
      stDisposeDuration.wMilliseconds
    );

    fprintf(gStdOutFile, "%s", DisposeDurationStr);
#endif

#if defined DEBUG_STOPWATCH
    PrintInternalCounters(gStdOutFile, stats);
#endif
    return ERR_SUCCESS;
  }
  else
  {
    fwprintf(gStdOutFile, L"ERROR: '%s' not found\n", aSourceDirList[0].ArgvOrg.c_str());
    return ERR_SOURCE_DIR_DOES_NOT_EXIST;
  }
}

int
LnSmartXXX(
  _ArgvList&                                aSourceDirList,
  _ArgvPath&                                aDestination,
  bool                                      aAbsolute,
  bool                                      aHardVsSymbolic,
  FileInfoContainer::CopyReparsePointFlags  aMode,
  _StringList*                              aIncludeFileList,
  _StringList*                              aIncludeDirList,
  _StringList*                              aExcludeFileList,
  _StringList*                              aExcludeDirList,
  _StringList*                              aUnrollDirList,
  _StringList*                              aSpliceDirList,
  bool                                      aSkipFiles,
  bool                                      aTraditional,
  bool                                      aBackup,
  bool                                      aKeepSymlinkRelation,
  int                                       aDateTimeTolerance,
  int                                       aHardlinkLimit
)
{
  int RetVal = ERROR_SUCCESS;

  CopyStatistics	aStats;
	
	GetLocalTime(&aStats.m_StartTime);
	FileInfoContainer	FileList;
  if (!aAbsolute)
    FileList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

  if (aTraditional)
    FileList.SetFlags(FileInfoContainer::eTraditional);

  if (aSkipFiles)
    FileList.SetFlags(FileInfoContainer::eSkipFiles);

  if (aKeepSymlinkRelation)
    FileList.SetFlags(FileInfoContainer::eKeepSymlinkRelation);

  FileList.SetFlags(gLogLevel);
  FileList.SetOutputFile(gStdOutFile);

  if (aBackup)
    FileList.SetFlags(FileInfoContainer::eBackupMode);

  if (gDupemerge)
    FileList.SetFlags2(FileInfoContainer::eDupemerge);

  if (gNoAds)
    FileList.SetFlags2(FileInfoContainer::eNoAds);

  if (gNoEa)
    FileList.SetFlags2(FileInfoContainer::eNoEa);

  if (gNoSparseFile)
    FileList.SetFlags2(FileInfoContainer::eNoSparseFile);

  bool JsonWriterState = false;
  if (gJson)
  {
    FileList.SetFlags(FileInfoContainer::eJson);
    FileList.SetJsonWriterState(&JsonWriterState);
    fwprintf (gStdOutFile, L"{\n\"Statistics\":\n[\n{\n\"Items\":[\n");
  }

  FileList.SetDateTimeTolerance(aDateTimeTolerance);
  FileList.SetHardlinkLimit(aHardlinkLimit);

  _PathNameStatusList PathNameStatusList;
  FileList.SetIncludeList(aIncludeFileList);
  FileList.SetIncludeDirList(aIncludeDirList);
  FileList.SetExcludeList(aExcludeFileList);
  FileList.SetExcludeDirList(aExcludeDirList);
  FileList.SetUnrollDirList(aUnrollDirList);
  FileList.SetSpliceDirList(aSpliceDirList);

  // Assign each source location the same destination if it is empty
  for (_ArgvListIterator iter = aSourceDirList.begin(); iter != aSourceDirList.end(); ++iter)
    if (iter->ArgvDest.empty())
      iter->ArgvDest = aDestination.Argv;

  AsyncContext    Context;
  AsyncContext*    pContext = nullptr;

  if (gProgress)
    pContext = &Context;
  FileList.FindHardLink (aSourceDirList, 0, &aStats, &PathNameStatusList, pContext);
  if (pContext)
    Enumerating(Context);

  // Check if enumerating worked
  //
  if (ERROR_SUCCESS != AnalysePathNameStatus(gStdOutFile, PathNameStatusList, false, gLogLevel, gJson, &JsonWriterState ) )
  {
    AnalysePathNameStatus(gStdOutFile, PathNameStatusList, true, gLogLevel, gJson, &JsonWriterState );
    if (gLogLevel <= FileInfoContainer::eLogError && !gJson)
      fwprintf (gStdOutFile, L"ERROR: Failed to enumerate files in %s\n", PathNameStatusList[0].m_PathName);

    return ERR_FAILED_TO_ENUMERATE_FILES;
  }

	GetLocalTime(&aStats.m_CopyTime);

  wchar_t ModeLiteral[MAX_PATH];
  Effort maxProgress;
  FileList.Prepare(aMode, &aStats, &maxProgress);

  // Since context was used during enumeration, reset it
  if (pContext)
    Context.Reset();
  switch (aMode)
  {
    case FileInfoContainer::eSmartCopy:
      FileList.SmartCopy (&aStats, &PathNameStatusList, pContext);
      wcscpy_s(ModeLiteral, MAX_PATH, L"Copying... ");
    break;
  
    case FileInfoContainer::eSmartClone:
      if (aHardVsSymbolic)
        FileList.SetFlags(FileInfoContainer::eSymboliclinkClone);
      FileList.SmartClone (&aStats, &PathNameStatusList, pContext);
      wcscpy_s(ModeLiteral, MAX_PATH, L"Cloning... ");
    break;

    case FileInfoContainer::eSmartClean:
      FileList.SmartClean(&aStats, &PathNameStatusList, pContext);
      wcscpy_s(ModeLiteral, MAX_PATH, L"Deleting...");
    break;
  
  }

  // ShowProgress(Effort& aMaxProgress, Context& aContext, wchar_t* aModeLiteral)

  // Print progress if async
  // 
  if (gProgress)
  {
    //                # 34%, Items 12345678901234, Time elapsed: 123d 02:03:04#
    //#sssssssssssbbbbppp%%, Items               , Time left: 123d 02:03:04#
    wprintf(L"%11s      0%%, Items               , Time left:              ", ModeLiteral);

    ProgressPrediction progressPrediction;
#if defined DEBUG_PREDICTION_RECORD
    progressPrediction.Start("c:\\tmp\\progress.log");
#endif
    progressPrediction.SetStart(maxProgress);
    int Percentage = 0;

    int UpdCnt = gUpdateIntervall;
    while (!Context.Wait(250))
    {
      int NewPercentage = progressPrediction.AddSample(Context.GetProgress());
      if (NewPercentage != Percentage)
      {
        Percentage = NewPercentage;
        PrintProgress(Percentage, progressPrediction);
      }
      if (!--UpdCnt)
      {
        UpdCnt = gUpdateIntervall;
        PrintProgress(Percentage, progressPrediction);
      }
    }
    PrintElapsed(progressPrediction);
#if defined DEBUG_PREDICTION_RECORD
    progressPrediction.Stop();
#endif

    if (gAutomatedTest)
    {
      __int64 copyOvershoot = maxProgress.m_Points.load() - Context.GetProgress().m_Points.load();
      if (copyOvershoot)
        // We do overshoot when run with --dupemerge, because the amount of files to be operated on changes during the copy operation.
        // Regression test thus contains one line with 'Overshoot'. All other overshoots shall be 0 and shall not show up.
        fwprintf(gStdOutFile, L"DEBUG: %s Overshoot: %I64d, Effort: %I64d\n", ModeLiteral, copyOvershoot, maxProgress.m_Points.load());
    }
  }

  if (aMode == FileInfoContainer::eSmartClean)
  {
    // Delete the anchor directories too
    for (auto iter : aSourceDirList)
    {
      if (iter.FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
      {
        BOOL removeDir = RemoveDir(iter.Argv.c_str(), TRUE);
        if (removeDir)
        {
          aStats.m_DirectoriesCleaned++;
          if (gLogLevel != FileInfoContainer::eLogQuiet)
            fwprintf(gStdOutFile, L"-d %s\n", &iter.Argv.c_str()[PATH_PARSE_SWITCHOFF_SIZE]);

          // TODO das geht mit json sicher nicht
        }
        else
        {
          aStats.m_DirectoriesCleanedFailed++;
          PathNameStatus pns(MinusD, &iter.Argv.c_str()[PATH_PARSE_SWITCHOFF_SIZE], GetLastError());
          PathNameStatusList.push_back(pns);
        }
      }
    }
  }
  GetLocalTime(&aStats.m_EndTime);

  // Analyse the result of the whole process
  int r = AnalysePathNameStatus(gStdOutFile, PathNameStatusList, true, gLogLevel, gJson, &JsonWriterState);
  if (ERROR_SUCCESS != r)
  {
    switch (aMode)
    {
      case FileInfoContainer::eSmartCopy:
        if (gLogLevel <= FileInfoContainer::eLogError && !gJson)
          fwprintf (gStdOutFile, L"WARNING: SmartCopy finished successfully but see output for errors.\n");
        RetVal = ERR_SMARTCOPY_FAILED;
      break;
    
      case FileInfoContainer::eSmartClone:
        if (gLogLevel <= FileInfoContainer::eLogError && !gJson)
          fwprintf (gStdOutFile, L"WARNING: SmartClone finished successfully but see output for errors.\n");
        RetVal = ERR_SMARTCLONE_FAILED;
      break;

      case FileInfoContainer::eSmartClean:
        if (gLogLevel <= FileInfoContainer::eLogError && !gJson)
          fwprintf (gStdOutFile, L"WARNING: Delete finished successfully but see output for errors.\n");
        RetVal = ERR_SMARTDELETE_FAILED;
      break;
    }
  }

  if (gJson)
    fwprintf (gStdOutFile, L"],\n");

  PrintDeloreanCopyStats(gStdOutFile, aStats, aMode, gAutomatedTest, gJson);

  if (gJson)
    fwprintf (gStdOutFile, L"}\n]\n}\n");

#if defined PRINT_DISPOSE_DURATION
  // Free Memory - no need to go async as in mirror
  SYSTEMTIME DisposeStartTime, DisposeEndTime;
  GetSystemTime(&DisposeStartTime);
#endif

  DeletePathNameStatusList(PathNameStatusList);
  FileList.Dispose(nullptr, &aStats);

#if defined PRINT_DISPOSE_DURATION
  GetSystemTime(&DisposeEndTime);

  // Time calculations for DisposeDuration
  FILETIME64 DisposeStart, DisposeEnd;
  SystemTimeToFileTime(&DisposeStartTime, &DisposeStart.FileTime);
  SystemTimeToFileTime(&DisposeEndTime, &DisposeEnd.FileTime);

  FILETIME64 DisposeDuration;
  DisposeDuration.ul64DateTime = DisposeEnd.ul64DateTime - DisposeStart.ul64DateTime;

  SYSTEMTIME stDisposeDuration;
  FileTimeToSystemTime(&DisposeDuration.FileTime, &stDisposeDuration);
  
  const int DisposeDurationStrLength = 64;
  char DisposeDurationStr[DisposeDurationStrLength];

  sprintf_s (DisposeDurationStr, DisposeDurationStrLength, "%02d:%02d:%02d.%03d",
    stDisposeDuration.wHour,
    stDisposeDuration.wMinute,
    stDisposeDuration.wSecond,
    stDisposeDuration.wMilliseconds
  );

  fprintf(gStdOutFile, "%s", DisposeDurationStr);
#endif

  return RetVal;
}

bool
QueryTooManyLinksJson(
  FILE*                     a_OutputFile,
  _PathNameStatusList&      a_PathNameStatusList, 
  bool                      a_Verbose,
  bool*                     a_JsonWriterState
)
{
  bool RetVal = false;

  PathNameStatus pns;
  for ( _PathNameStatusList::iterator iter = a_PathNameStatusList.begin(); iter != a_PathNameStatusList.end(); ++iter )
  {
    pns = *iter;
    if (pns.m_ErrorCode == ERROR_TOO_MANY_LINKS)
    {
      if (a_Verbose)
      {
        wchar_t JsonPath[HUGE_PATH];
        wcsesc_s(JsonPath, pns.m_PathName, HUGE_PATH, L'\\');

        if (*a_JsonWriterState)
          fwprintf (a_OutputFile, L",\n");
        else
          *a_JsonWriterState = true;

        fwprintf (a_OutputFile, L"{\"op\":\"+\",\"er\":1142,\"ty\":\"h\",\"pa\":\"%s\"}", 
          JsonPath
        );
      }
      RetVal = true;
    }
  }
      
  return RetVal;
}

bool
QueryTooManyLinksNormal(
  FILE*                     a_OutputFile,
  _PathNameStatusList&      a_PathNameStatusList, 
  bool                      a_Verbose
)
{
  bool HeaderPrinted = false;
  bool RetVal = false;
  
  HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
  GetConsoleScreenBufferInfo(output, &ConsoleScreenBufferInfo);

  PathNameStatus pns;
  for ( _PathNameStatusList::iterator iter = a_PathNameStatusList.begin(); iter != a_PathNameStatusList.end(); ++iter )
  {
    pns = *iter;
    if (pns.m_ErrorCode == ERROR_TOO_MANY_LINKS)
    {
      RetVal = true;
      if (a_Verbose)
      {
        if (!HeaderPrinted)
        {
          SetConsoleTextAttribute(output, FOREGROUND_RED|FOREGROUND_INTENSITY);
          fwprintf (a_OutputFile, L"\n\nDelorean Copy FAILED! The NTFS link limit of 1023 has been exceeded for:\n");

          SetConsoleTextAttribute(output, ConsoleScreenBufferInfo.wAttributes);

          HeaderPrinted = true;
        }

        fwprintf (a_OutputFile, L"!+h(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
      }
      else
        break;
    }
  }
  if (HeaderPrinted)
  {
    SetConsoleTextAttribute(output, FOREGROUND_RED|FOREGROUND_INTENSITY);
    fwprintf (a_OutputFile, L"\nDelorean Copy FAILED! The NTFS link limit of 1023 has been exceeded!");
    SetConsoleTextAttribute(output, ConsoleScreenBufferInfo.wAttributes);
  }

  return RetVal;
}

bool
QueryTooManyLinks(
  FILE*                     a_OutputFile,
  _PathNameStatusList&      a_PathNameStatusList, 
  bool                      a_Verbose,
  bool                      a_Json,
  bool*                     a_JsonWriterState
)
{
  if (a_Json)
    return QueryTooManyLinksJson(a_OutputFile, a_PathNameStatusList, a_Verbose, a_JsonWriterState);
  else
    return QueryTooManyLinksNormal(a_OutputFile, a_PathNameStatusList, a_Verbose);
}

void _Mirror(
  FileInfoContainer&    CloneList,
  FileInfoContainer&    MirrorList,
  CopyStatistics&       MirrorStatistics,
  _PathNameStatusList&  PathNameStatusList,
  bool                  aDeloreanMerge
)
{
  // Sequentially run Clean and Mirror, because it won't work in parallel.
  // Why: If we would just deal with items there or not, parallel would work, but unfortunatley 
  // items can also be renamed during mirror and this breaks the parallel approach

  AsyncContext  MirrorContext;
  AsyncContext* pMirrorContext = nullptr;
  if (gProgress)
    pMirrorContext = &MirrorContext;

  Effort maxMirrorEffort;
  MirrorList.Prepare(FileInfoContainer::eSmartMirror, &MirrorStatistics, &maxMirrorEffort);

  // Lets clean the cloned backup
  CloneList.SetLookAsideContainer(&MirrorList);
  CopyStatistics	CleanStatistics;
  Effort maxCleanEffort;
  if (!aDeloreanMerge)
  {
    CloneList.Prepare(FileInfoContainer::eSmartClean, &CleanStatistics, &maxCleanEffort);
    CloneList.SmartClean(&MirrorStatistics, &PathNameStatusList, pMirrorContext);
  }

  __int64 maxMirrorItems = maxMirrorEffort.m_Items.load();
  maxMirrorEffort += maxCleanEffort;

  // Print SmartClean progress
  //
  int Percentage = 0;
  Effort CleanOverallProgress;
  ProgressPrediction progressPrediction;
  if (gProgress)
  {
    wprintf(L"Mirroring...    0%%, Items               , Time left:              ");
    progressPrediction.SetStart(maxMirrorEffort);

    // Only if we are in normal mode, clean is run. With Delorean Merge we don't
    if (!aDeloreanMerge)
    {
      int UpdCnt = gUpdateIntervall;
      while (!MirrorContext.Wait(250))
      {
        int NewPercentage = progressPrediction.AddSample(MirrorContext.GetProgress());
        if (NewPercentage != Percentage)
        {
          Percentage = NewPercentage;
          PrintProgress(Percentage, progressPrediction);
        }
        if (!--UpdCnt)
        {
          UpdCnt = gUpdateIntervall;
          PrintProgress(Percentage, progressPrediction);
        }
      }
      CleanOverallProgress = MirrorContext.GetProgress();
      progressPrediction.AddSample(MirrorContext.GetProgress(), &CleanOverallProgress);
      PrintProgress(Percentage, progressPrediction);

      // During Debug we would like to know how good the effort prediction was. There should be almost no over or under shoots
      if (gAutomatedTest)
      {
        if (maxCleanEffort.m_Points.load() > 0)
        {
          __int64 cleanOvershoot = maxCleanEffort.m_Points.load() - CleanOverallProgress.m_Points.load();
          if (cleanOvershoot)
            fwprintf(gStdOutFile, L"DEBUG: Clean Overshoot: %I64d, Effort: %I64d\n", cleanOvershoot, maxCleanEffort.m_Points.load());
        }
      }
    }
    MirrorContext.Reset();
  }

  // Once cleaning is done start the mirror process
  MirrorList.SetLookAsideContainer(&CloneList);
  MirrorList.SmartMirror(&MirrorStatistics, &PathNameStatusList, pMirrorContext);

  // Print mirror progress 
  //
  if (gProgress)
  {
    int UpdCnt = gUpdateIntervall;
    while (!MirrorContext.Wait(250))
    {
      int NewPercentage = progressPrediction.AddSample(MirrorContext.GetProgress(), &CleanOverallProgress);
      if (NewPercentage != Percentage)
      {
        Percentage = NewPercentage;
        PrintProgress(Percentage, progressPrediction);
      }
      if (!--UpdCnt)
      {
        UpdCnt = gUpdateIntervall;
        PrintProgress(Percentage, progressPrediction);
      }
    }
    // With mirror we also have the items to be cleaned into account, because they can make up a lot of 
    // the progress. Think of mirroring against a destination, which contains 10 times the number of files
    // of the source. But at the very end we would like to show up only the numbers of items from the source,
    // so we have to pass this number to PrintElapsed()
    PrintElapsed(progressPrediction, maxMirrorItems);

    if (gAutomatedTest)
    {
      __int64 mirrorOvershoot = maxMirrorEffort.m_Points.load() - MirrorContext.GetProgress().m_Points.load() - maxCleanEffort.m_Points.load();
      if (mirrorOvershoot)
        fwprintf(gStdOutFile, L"DEBUG: Mirror Overshoot: %I64d, Effort: %I64d\n", mirrorOvershoot, maxMirrorEffort.m_Points.load() - maxCleanEffort.m_Points.load());
    }
  }

  GetLocalTime(&MirrorStatistics.m_EndTime);
}

int
Delorean(
	_ArgvList&                                aSourceDirList,
	_ArgvPath&                                aDestPath,
	_ArgvPath&                                aBackupPath,
  bool                                      aAbsolute,
  _StringList*                              aIncludeFileList,
  _StringList*                              aIncludeDirList,
  _StringList*                              aExcludeFileList,
  _StringList*                              aExcludeDirList,
  _StringList*                              aUnrollDirList,
  _StringList*                              aSpliceDirList,
  bool                                      aSkipFiles,
  bool                                      aTraditional,
  bool                                      aBackup,
  bool                                      aKeepSymlinkRelation,
  int                                       aDateTimeTolerance,
  int                                       aHardlinkLimit,
  bool                                      a1023safe
)
{
  CopyStatistics	CloneStatistics;
	
  // Create a Clone Destination -> Backup
	GetLocalTime(&CloneStatistics.m_StartTime);
	FileInfoContainer	CloneList;
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
  wcscpy(CloneList.m_ContainerName, L"-!C");
#endif
  CloneList.SetFlags(gDeloreanVerbose ? FileInfoContainer::eLogVerbose : FileInfoContainer::eLogQuiet);

  CloneList.SetOutputFile(gStdOutFile);

  CloneList.SetDateTimeTolerance(aDateTimeTolerance);
  CloneList.SetHardlinkLimit(aHardlinkLimit);


  _PathNameStatusList PathNameStatusList;
  _PathNameStatusList PathNameStatusListClone;

  // Mirror the clone: Source -> Backup
	FileInfoContainer	MirrorList;
  MirrorList.SetFlags(gLogLevel);
  MirrorList.SetOutputFile(gStdOutFile);

  MirrorList.SetDateTimeTolerance(aDateTimeTolerance);

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
  wcscpy(MirrorList.m_ContainerName, L"-!M");
#endif

  if (!aAbsolute)
  {
    MirrorList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);
    CloneList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);
  }

  if (aKeepSymlinkRelation)
  {
    MirrorList.SetFlags(FileInfoContainer::eKeepSymlinkRelation);
    CloneList.SetFlags(FileInfoContainer::eKeepSymlinkRelation);
  }

  if (aSkipFiles)
  {
    MirrorList.SetFlags(FileInfoContainer::eSkipFiles);
    CloneList.SetFlags(FileInfoContainer::eSkipFiles);
  }

  if (aBackup)
  {
    MirrorList.SetFlags(FileInfoContainer::eBackupMode);
    CloneList.SetFlags(FileInfoContainer::eBackupMode);
  }

  if (aTraditional)
  {
    MirrorList.SetFlags(FileInfoContainer::eTraditional);
    CloneList.SetFlags(FileInfoContainer::eTraditional);
  }

  if (gDupemerge)
    MirrorList.SetFlags2(FileInfoContainer::eDupemerge);

  if (gNoAds)
  {
    MirrorList.SetFlags2(FileInfoContainer::eNoAds);
    CloneList.SetFlags2(FileInfoContainer::eNoAds);
  }

  if (gNoEa)
  {
    MirrorList.SetFlags2(FileInfoContainer::eNoEa);
    CloneList.SetFlags2(FileInfoContainer::eNoEa);
  }

  if (gNoSparseFile)
  {
    MirrorList.SetFlags2(FileInfoContainer::eNoSparseFile);
    CloneList.SetFlags2(FileInfoContainer::eNoSparseFile);
  }

  if (a1023safe)
  {
    CloneList.SetFlags(FileInfoContainer::eTraditional);
  }

  bool JsonWriterState = false;
  if (gJson)
  {
    MirrorList.SetFlags(FileInfoContainer::eJson);
    CloneList.SetFlags(FileInfoContainer::eJson);
    MirrorList.SetJsonWriterState(&JsonWriterState);
    CloneList.SetJsonWriterState(&JsonWriterState);

    fwprintf (gStdOutFile, L"{\n\"Statistics\":\n[\n{\n\"Items\":[\n");
  }

  AsyncContext MirrorFindContext;
  GetLocalTime(&CloneStatistics.m_CopyTime);

  // Set Filters in the source
  MirrorList.SetIncludeList(aIncludeFileList);
  MirrorList.SetIncludeDirList(aIncludeDirList);
  MirrorList.SetExcludeList(aExcludeFileList);
  MirrorList.SetExcludeDirList(aExcludeDirList);

  // Set Filters in the destination
  CloneList.SetIncludeList(aIncludeFileList);
  CloneList.SetIncludeDirList(aIncludeDirList);
  CloneList.SetExcludeList(aExcludeFileList);
  CloneList.SetExcludeDirList(aExcludeDirList);

  MirrorList.SetUnrollDirList(aUnrollDirList);
  MirrorList.SetSpliceDirList(aSpliceDirList);

  CopyStatistics	MirrorStatistics;
  
  // Assign each source location the same destination
  for (_ArgvListIterator iter = aSourceDirList.begin(); iter != aSourceDirList.end(); ++iter)
    iter->ArgvDest = aBackupPath.Argv;

#if defined SEPERATED_CLONE_MIRROR
  MirrorList.FindHardLink (aSourceDirList, 0, &MirrorStatistics, &PathNameStatusList, nullptr);
#else
  MirrorList.FindHardLink (aSourceDirList, 0, &MirrorStatistics, &PathNameStatusList, &MirrorFindContext);
#endif


  // If junctions should be spliced, then already spliced
  // junctions from Backup0 should also be spliced in the clone
  // from Backup0 to Backup1
  CloneList.SetUnrollDirList(aUnrollDirList);
  CloneList.SetSpliceDirList(aSpliceDirList);
  
  _ArgvList CloneDestination;
  // Clone Backup0 -> Backup1
  aDestPath.ArgvDest = aBackupPath.Argv;
  CloneDestination.push_back(aDestPath);

  AsyncContext CloneContext;
  AsyncContext* pCloneContext = nullptr;

  if (gProgress)
    pCloneContext = &CloneContext;

  CloneList.FindHardLink (CloneDestination, 0, &CloneStatistics, &PathNameStatusListClone, pCloneContext);
  if (pCloneContext)
    Enumerating(CloneContext);

  GetLocalTime(&CloneStatistics.m_CopyTime);
  Effort MaxProgress;
  CloneList.Prepare(FileInfoContainer::eSmartClone, &CloneStatistics, &MaxProgress);

  if (gProgress)
    CloneContext.Reset();
  CloneList.SmartClone (&CloneStatistics, &PathNameStatusListClone, pCloneContext);

  // Print clone progress
  //
  if (gProgress)
  {
    wprintf(L"Cloning  ...    0%%, Items               , Time left:              ");
    int Percentage = 0;

    ProgressPrediction progressPrediction;
    progressPrediction.SetStart(MaxProgress);

    // Wait until Cloning is over
    int UpdCnt = gUpdateIntervall;
    while (!CloneContext.Wait(250))
    {
      int NewPercentage = progressPrediction.AddSample(CloneContext.GetProgress());
      if (NewPercentage != Percentage)
      {
        Percentage = NewPercentage;
        PrintProgress(Percentage, progressPrediction);
      }
      if (!--UpdCnt)
      {
        UpdCnt = gUpdateIntervall;
        PrintProgress(Percentage, progressPrediction);
      }
    }
    PrintElapsed(progressPrediction);
  }


#if defined SEPERATED_CLONE_MIRROR
  printf ("Press any key to do the mirror\n");
//  char c = getch();
#endif

  // It turned out, that creating hardlinks on remote drive is a bit crucial
  //     https://technet.microsoft.com/en-us/library/ff686200%28WS.10%29.aspx
  // In general working together with different SMB versions is a pain
  //     http://blogs.technet.com/b/josebda/archive/2013/10/02/windows-server-2012-r2-which-version-of-the-smb-protocol-smb-1-0-smb-2-0-smb-2-1-smb-3-0-or-smb-3-02-you-are-using.aspx
  if (ContainsRemote(CloneDestination))
  {
    if (0 == gDeloreanSleep)
      gDeloreanSleep = GetFileNotFoundCacheLifetime();
    if (gDeloreanSleep > 0 && !gJson)
    {
      fwprintf (gStdOutFile, L"'Sleeping %d sec: Destination is a remote drive.\n", gDeloreanSleep);
      fflush(gStdOutFile);
      if (!gAutomatedTest)
        Sleep(gDeloreanSleep * 1000 + 100);
    }
  }

  if (gDeloreanVerbose)
  // Print statistics of Clone for debugging purposes
  {
    int iResult = ERROR_SUCCESS;
    int r = AnalysePathNameStatus(gStdOutFile, PathNameStatusListClone, true, gLogLevel, gJson, &JsonWriterState);
    if (ERROR_SUCCESS != r)
    {
      if (gLogLevel <= FileInfoContainer::eLogError && !gJson)
        fwprintf (gStdOutFile, L"WARNING: Delorean Copy finished successfully but see output for errors.\n");
      iResult = ERR_DELOREANCOPY_FAILED;
    }
    CloneStatistics.m_EndTime = CloneStatistics.m_CopyTime;

    if (gJson)
      fwprintf (gStdOutFile, L"],\n");

    PrintDeloreanCopyStats(gStdOutFile, 
      CloneStatistics, 
      (FileInfoContainer::CopyReparsePointFlags)0, 
      gAutomatedTest, 
      gJson
    );

    if (gJson)
    {
      JsonWriterState = false;
      fwprintf (gStdOutFile, L"},\n{\n\"Items\":[\n");
    }

    fflush(gStdOutFile);
  }

  // Check if it the clone worked
  if (QueryTooManyLinks(gStdOutFile, PathNameStatusListClone, false, gJson, &JsonWriterState))
  {
    CloneStatistics.m_EndTime = CloneStatistics.m_CopyTime;
    QueryTooManyLinks(gStdOutFile, PathNameStatusListClone, true, gJson, &JsonWriterState);

    if (gJson)
      fwprintf (gStdOutFile, L"],\n");

    PrintDeloreanCopyStats(gStdOutFile, 
      CloneStatistics, 
      (FileInfoContainer::CopyReparsePointFlags)0, 
      gAutomatedTest, 
      gJson
    );

    DeletePathNameStatusList(PathNameStatusListClone);

    if (gJson)
      fwprintf (gStdOutFile, L"}\n]\n}\n");

    return ERR_TOO_MANY_LINKS;
  }


#if !defined SEPERATED_CLONE_MIRROR
  // Wait until the MirrorFindContext also has finished with Findhardlink. Normally Findhardlink on the Mirrorsource should take longer 
  // than Clone on the bkp. An the other way we also have to wait for CloneContext if no progress printing is active
  // Anyhow syncing at this point is crucial. 
  // Well for progress printing this is not optimal, because we are stuck showing nothing until FindHardlink on MirrorSource is done
  while (!MirrorFindContext.Wait(100));
#endif


  MirrorStatistics.m_StartTime = CloneStatistics.m_StartTime ;
	GetLocalTime(&MirrorStatistics.m_CopyTime);

  if (gLogLevel != FileInfoContainer::eLogQuiet)
  {
    CloneList.ClearFlags(FileInfoContainer::eLogMask);
    CloneList.SetFlags(gLogLevel);
  }

  // Iterate over all source path and replace all source path in Clonelist
  for (_ArgvListIterator iter = aSourceDirList.begin(); iter != aSourceDirList.end(); ++iter)
    iter->ArgvDest = iter->Argv;

  CloneList.ChangePath(aBackupPath.Argv.c_str(), aSourceDirList);

  _Mirror(CloneList, MirrorList, MirrorStatistics, PathNameStatusList, false);

  // Start releasing data async, because releasing data really takes time
  const int nHandles = 2;
  HANDLE  WaitEvents[nHandles];
  AsyncContext CloneDisposeContext;
  AsyncContext MirrorDisposeContext;
  MirrorList.Dispose(&MirrorDisposeContext, &MirrorStatistics);
  CopyStatistics	CleanStatistics;
  CloneList.Dispose(&CloneDisposeContext, &CleanStatistics);

  WaitEvents[0] = MirrorDisposeContext.m_WaitEvent;
  WaitEvents[1] = CloneDisposeContext.m_WaitEvent;

  // Join PathNameStatusList and PathNameStatusListClone
  PathNameStatusList.insert(PathNameStatusList.end(), PathNameStatusListClone.begin(), PathNameStatusListClone.end());

  int iResult = ERROR_SUCCESS;
  int r = AnalysePathNameStatus(gStdOutFile, PathNameStatusList, true, gLogLevel, gJson, &JsonWriterState);
  if (ERROR_SUCCESS != r)
  {
    if (gLogLevel <= FileInfoContainer::eLogError && !gJson)
      fwprintf (gStdOutFile, L"WARNING: Delorean Copy finished successfully but see output for errors.\n");
    iResult = ERR_DELOREANCOPY_FAILED;
  }

  if (gJson)
    fwprintf (gStdOutFile, L"],\n");

//  MirrorStatistics+= CloneStatistics;
  PrintDeloreanCopyStats(gStdOutFile, 
    MirrorStatistics, 
    (FileInfoContainer::CopyReparsePointFlags)(FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror), 
    gAutomatedTest, 
    gJson
  );

  if (QueryTooManyLinks(gStdOutFile, PathNameStatusList, true, gJson, &JsonWriterState))
    iResult = ERR_TOO_MANY_LINKS;

  if (gJson)
    fwprintf (gStdOutFile, L"}\n]\n}\n");

  DeletePathNameStatusList(PathNameStatusList);

  // Wait till releasing of resources has finished
  WaitForMultipleObjects(nHandles, WaitEvents, TRUE, INFINITE);

  return iResult;
}


int
Mirror(
	_ArgvList&                                aSourceDirList,
	_ArgvPath&                                aDestination,
  bool                                      aAbsolute,
  _StringList*                              aIncludeFileList,
  _StringList*                              aIncludeDirList,
  _StringList*                              aExcludeFileList,
  _StringList*                              aExcludeDirList,
  _StringList*                              aUnrollDirList,
  _StringList*                              aSpliceDirList,
  bool                                      aSkipFiles,
  bool                                      aTraditional,
  bool                                      aBackup,
  bool                                      aKeepSymlinkRelation,
  int                                       aDateTimeTolerance,
  bool                                      aSymbolic,
  bool                                      aHardlinkMirror,
  bool                                      aDeloreanMerge,
  int                                       aHardlinkLimitValue
)
{
  CopyStatistics	CloneStatistics;
	
  // Create a Clone Destination -> Backup
	GetLocalTime(&CloneStatistics.m_StartTime);
	FileInfoContainer	CloneList;

  CloneList.SetFlags(FileInfoContainer::eLogQuiet);
  CloneList.SetOutputFile(gStdOutFile);

  CloneList.SetDateTimeTolerance(aDateTimeTolerance);
  CloneList.SetHardlinkLimit(aHardlinkLimitValue);


  _PathNameStatusList PathNameStatusList;

  // Mirror the clone: Source -> Backup
	FileInfoContainer	MirrorList;
  MirrorList.SetHardlinkLimit(aHardlinkLimitValue);
  if (!aAbsolute)
  {
    MirrorList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);
    CloneList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);
  }

  if (aKeepSymlinkRelation)
  {
    MirrorList.SetFlags(FileInfoContainer::eKeepSymlinkRelation);
    CloneList.SetFlags(FileInfoContainer::eKeepSymlinkRelation);
  }

  if (aSkipFiles)
  {
    CloneList.SetFlags(FileInfoContainer::eSkipFiles);
    MirrorList.SetFlags(FileInfoContainer::eSkipFiles);
  }

  if (aBackup)
  {
    MirrorList.SetFlags(FileInfoContainer::eBackupMode);
    CloneList.SetFlags(FileInfoContainer::eBackupMode);
  }

  if (aTraditional)
  {
    MirrorList.SetFlags(FileInfoContainer::eTraditional);
    CloneList.SetFlags(FileInfoContainer::eTraditional);
  }

  if (gDupemerge)
    MirrorList.SetFlags2(FileInfoContainer::eDupemerge);

  if (gNoAds)
  {
    MirrorList.SetFlags2(FileInfoContainer::eNoAds);
    CloneList.SetFlags2(FileInfoContainer::eNoAds);
  }

  if (gNoEa)
  {
    MirrorList.SetFlags2(FileInfoContainer::eNoEa);
    CloneList.SetFlags2(FileInfoContainer::eNoEa);
  }

  if (gNoSparseFile)
  {
    MirrorList.SetFlags2(FileInfoContainer::eNoSparseFile);
    CloneList.SetFlags2(FileInfoContainer::eNoSparseFile);
  }

  bool JsonWriterState = false;
  if (gJson)
  {
    MirrorList.SetFlags(FileInfoContainer::eJson);
    MirrorList.SetJsonWriterState(&JsonWriterState);
    CloneList.SetFlags(FileInfoContainer::eJson);
    CloneList.SetJsonWriterState(&JsonWriterState);
    fwprintf (gStdOutFile, L"{\n\"Statistics\":\n[\n{\n\"Items\":[\n");
  }

  // This is --symbolic --mirror aka CloneSymbolicMirror
  if (aSymbolic)
  {
    MirrorList.SetFlags(FileInfoContainer::eSymboliclinkClone);
    MirrorList.SetFlags2(FileInfoContainer::eCloneMirror);
    CloneList.SetFlags(FileInfoContainer::eSymboliclinkClone);
    CloneList.SetFlags2(FileInfoContainer::eCloneMirror);
  }

  // This is --recursive --mirror aka CloneHardMirror
  if (aHardlinkMirror)
  {
    MirrorList.SetFlags2(FileInfoContainer::eCloneMirror);
    CloneList.SetFlags2(FileInfoContainer::eCloneMirror);
  }

  // This is --merge aka DeloreanMerge
  if (aDeloreanMerge)
  {
    MirrorList.SetFlags2(FileInfoContainer::eDeloreanMerge);
    CloneList.SetFlags2(FileInfoContainer::eDeloreanMerge);
  }

  MirrorList.SetFlags(gLogLevel);
  MirrorList.SetOutputFile(gStdOutFile);

  MirrorList.SetDateTimeTolerance(aDateTimeTolerance);

  AsyncContext MirrorContext;
  GetLocalTime(&CloneStatistics.m_CopyTime);
  
  // Set the filters on the source side
  MirrorList.SetIncludeList(aIncludeFileList);
  MirrorList.SetIncludeDirList(aIncludeDirList);
  MirrorList.SetExcludeList(aExcludeFileList);
  MirrorList.SetExcludeDirList(aExcludeDirList);

  // Set the filters on the destination side
  CloneList.SetIncludeList(aIncludeFileList);
  CloneList.SetIncludeDirList(aIncludeDirList);
  CloneList.SetExcludeList(aExcludeFileList);
  CloneList.SetExcludeDirList(aExcludeDirList);

  // But the unroll stuff only has to be set on the source side
  MirrorList.SetUnrollDirList(aUnrollDirList);
  MirrorList.SetSpliceDirList(aSpliceDirList);

#if 0
  _ArgvList CloneDestination;
  for (_ArgvListIterator iter = aSourceDirList.begin(); iter != aSourceDirList.end(); ++iter)
  {
    iter->ArgvDest = aDestination.Argv;
    if (!(iter->Flags & _ArgvPath::Anchor))
    {
      // Add the first non-anchor as 'destination', which in this case is the source location
      if (aDestination.ArgvDest.empty())
      {
        aDestination.Argv = iter->ArgvDest;
        aDestination.ArgvDest = iter->Argv;
      }
    }
  }
  // Provide a destination
  CloneDestination.push_back(aDestination);
#else
  // Assign each source location the same destination
  _ArgvList CloneDestination;
  for (_ArgvListIterator iter = aSourceDirList.begin(); iter != aSourceDirList.end(); ++iter)
  {
    // if no corresponding destination was given via --destination, assume there is only one destination
    
    bool isAnchor = iter->Flags & _ArgvPath::Anchor;
    
    if (iter->ArgvDest.empty())
    {
      // All attributes of destination are copied over
      iter->ArgvDest = aDestination.Argv;
      iter->ArgvOrg = aDestination.ArgvOrg;
      iter->DriveType = aDestination.DriveType;
      iter->FileAttribute = aDestination.FileAttribute;
      if (!isAnchor)
        iter->Flags = aDestination.Flags;
    }

    if (!isAnchor)
    {
      // for non-anchors, create a destination which is source-destination flipped 
      _ArgvPath destination;

      destination.Argv = iter->ArgvDest;
      destination.ArgvDest = iter->Argv;
      destination.ArgvOrg = iter->ArgvOrg;
      destination.DriveType = aDestination.DriveType;
      destination.FileAttribute = aDestination.FileAttribute;
      destination.Flags = aDestination.Flags;

      CloneDestination.push_back(destination);
    }
    else
      CloneDestination.push_back(aDestination);
  }
#endif  

  CopyStatistics	MirrorStatistics;
#if defined SEPERATED_CLONE_MIRROR
  MirrorList.FindHardLink (aSourceDirList, 0, &MirrorStatistics, &PathNameStatusList, nullptr);
#else
  MirrorList.FindHardLink (aSourceDirList, 0, &MirrorStatistics, &PathNameStatusList, &MirrorContext);
#endif

  // Query Files on the destination
  _PathNameStatusList ClonePathNameStatusList;
  CloneList.FindHardLink (CloneDestination, 0, &CloneStatistics, &ClonePathNameStatusList, nullptr);

#if !defined SEPERATED_CLONE_MIRROR
  // Wait until both enum threads have finished
  Enumerating(MirrorContext);
#endif

  MirrorStatistics.m_StartTime = CloneStatistics.m_StartTime;
  GetLocalTime(&MirrorStatistics.m_CopyTime);

  if (gLogLevel != FileInfoContainer::eLogQuiet)
  {
    CloneList.ClearFlags(FileInfoContainer::eLogMask);
    CloneList.SetFlags(gLogLevel);
  }

  _Mirror(CloneList, MirrorList, MirrorStatistics, PathNameStatusList, aDeloreanMerge);

#if defined PRINT_DISPOSE_DURATION
  // Start releasing data async, because releasing data really takes time
  SYSTEMTIME DisposeStartTime, DisposeEndTime;
  GetSystemTime(&DisposeStartTime);
#endif

  const int nHandles = 2;
  HANDLE  WaitEvents[nHandles];
  AsyncContext CloneDisposeContext;
  AsyncContext MirrorDisposeContext;
  MirrorList.Dispose(&MirrorDisposeContext, &MirrorStatistics);
  CopyStatistics	CleanStatistics;
  CloneList.Dispose(&CloneDisposeContext, &CleanStatistics);

  WaitEvents[0] = MirrorDisposeContext.m_WaitEvent;
  WaitEvents[1] = CloneDisposeContext.m_WaitEvent;


  int iResult = ERROR_SUCCESS;
  int r = AnalysePathNameStatus(gStdOutFile, PathNameStatusList, true, gLogLevel, gJson, &JsonWriterState);
  if (ERROR_SUCCESS != r)
  {
    if (gLogLevel <= FileInfoContainer::eLogError && !gJson)
      if (aDeloreanMerge)
        fwprintf (gStdOutFile, L"WARNING: Merge finished successfully but see output for errors.\n");
      else
        fwprintf (gStdOutFile, L"WARNING: SmartMirror finished successfully but see output for errors.\n");
    iResult = ERR_SMARTMIRROR_FAILED;
  }

  if (gJson)
    fwprintf (gStdOutFile, L"],\n");

  PrintDeloreanCopyStats(gStdOutFile, MirrorStatistics, FileInfoContainer::eSmartMirror, gAutomatedTest, gJson);

  if (QueryTooManyLinks(gStdOutFile, PathNameStatusList, true, gJson, &JsonWriterState))
    iResult = ERR_TOO_MANY_LINKS;

  if (gJson)
    fwprintf (gStdOutFile, L"}\n]\n}\n");

  DeletePathNameStatusList(PathNameStatusList);

  // Wait till releasing of resources has finished
  WaitForMultipleObjects(nHandles, WaitEvents, TRUE, INFINITE);

#if defined PRINT_DISPOSE_DURATION
  GetSystemTime(&DisposeEndTime);

  // Time calculations for DisposeDuration
  FILETIME64 DisposeStart, DisposeEnd;
  SystemTimeToFileTime(&DisposeStartTime, &DisposeStart.FileTime);
  SystemTimeToFileTime(&DisposeEndTime, &DisposeEnd.FileTime);

  FILETIME64 DisposeDuration;
  DisposeDuration.ul64DateTime = DisposeEnd.ul64DateTime - DisposeStart.ul64DateTime;

  SYSTEMTIME stDisposeDuration;
  FileTimeToSystemTime(&DisposeDuration.FileTime, &stDisposeDuration);
  
  const int DisposeDurationStrLength = 64;
  char DisposeDurationStr[DisposeDurationStrLength];

  sprintf (DisposeDurationStr, DisposeDurationStrLength, "%02d:%02d:%02d.%03d",
    stDisposeDuration.wHour,
    stDisposeDuration.wMinute,
    stDisposeDuration.wSecond,
    stDisposeDuration.wMilliseconds
  );

  fprintf(gStdOutFile, "%s", DisposeDurationStr);
#endif

  return iResult;
}



PTCHAR
ResolveUNC(
	const wchar_t*  aUNCPath,
	wchar_t*        dest
)
{
  // Check if path is UNC
  if (!PathIsUNC(aUNCPath))
  {
    return nullptr;
  }

  wchar_t src[HUGE_PATH];
  wcscpy_s(src, HUGE_PATH, aUNCPath);
  TCHAR		seps[] = _T("\\");
  PTCHAR	token;
  PTCHAR	share;
  PTCHAR	path;

  token = _wcstok(src, seps);
  share = _wcstok(NULL, seps);

  // Check if the share has a path appended \\quadtatz\vss_share\**
  path = _wcstok(NULL, seps);
  if (!path)
  {
    // This is an empty path after the 
    size_t	ShareLength = wcslen(share);
    path = share + ShareLength;
  }
  else
  {
    // Collect the remaining path e.g \\quadtatz\vss_share\*tmp\src*
    PTCHAR ppp;
    while (ppp = _wcstok(NULL, seps))
    {
      *--ppp = '\\';
    }
  }

  char	hostname[MAX_PATH] = { 0x00 };
  char	ansiSrc[MAX_PATH];
  MakeAnsiString(src, ansiSrc);

  // request local hostname
  WORD wVersionRequested;
  WSADATA wsaData;
  wVersionRequested = MAKEWORD(1, 1);

  if (WSAStartup(wVersionRequested, &wsaData) == 0)
  {
    gethostname(hostname, MAX_PATH);
    WSACleanup();
  }

  // Check if the share is local
  if (!_stricmp(hostname, &ansiSrc[2]) && gResolveUNC)
  {
    PSHARE_INFO_502	BufPtr;

    NET_API_STATUS	nas = NetShareGetInfo(src, share, 502, (LPBYTE*)&BufPtr);
    if (nas == NERR_Success)
    {
      wcscpy_s(dest, HUGE_PATH, BufPtr->shi502_path);

      // Only append a trailing slash if there is not already one
      size_t DestLength = wcslen(dest);
      if (dest[DestLength - 1] != '\\')
        wcscat_s(dest, HUGE_PATH, _T("\\"));

      wcscat_s(dest, HUGE_PATH, path);
      NetApiBufferFree(BufPtr);
    }
    else
    {
      dest = NULL;
    }
  }
  else
  {
    dest = NULL;
  }

  *(share - 1) = '\\';
  *(path - 1) = '\\';

  return dest;
}

int
CheckArgument(
  _ArgvPath*      aArgvPath,
  bool            aExpression
)
{
  int RetVal = ERR_SUCCESS;
  if (aArgvPath->Argv.length())
  {
    // Resolve a local UNC
    wchar_t PathTmp[HUGE_PATH];
    wchar_t* pUncResolved = ResolveUNC(aArgvPath->Argv.c_str(), PathTmp);
    if (!pUncResolved)
      wcscpy_s(PathTmp, HUGE_PATH, aArgvPath->Argv.c_str());

    wchar_t*  pArgvFilePart;
    wchar_t ArgvPath[HUGE_PATH];

    // See if path is still UNC
    if (!PathIsUNC(PathTmp))
    {
      // vshadow creates UNC names which are mounted into the Globalroot
      // e.g.: \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy4\tmp
      if (!IsVeryLongPath(PathTmp))
      {
        // we only need to prefix \\?\ if it not already there
        wcscpy_s(ArgvPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(PathTmp, HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &ArgvPath[PATH_PARSE_SWITCHOFF_SIZE], &pArgvFilePart);
      }
      else
      {
        // But there are situations e.g with vshadow, where \\?\ is already there.
        //
        // For testing: Persistent Snapshots are created by
        //     %VSHADOW% -p x:
        // They are deleted by
        //     %VSHADOW% -ds={SNAPSHOT ID}
        // One can map the snapshot onto a share
        //     %VSHADOW% -er={SNAPSHOT ID},sharename
        //     vshadow64 -er={c6107cb4-fdea-4470-8309-fe46af7ca2ee},2013-05-04
        GetFullPathName(PathTmp, HUGE_PATH, ArgvPath, &pArgvFilePart);
      }
    }
    else
    {
      // The path might still be UNC if it was not a local UNC path, which could not be resolved
      GetFullPathName(PathTmp, HUGE_PATH, ArgvPath, &pArgvFilePart);
    }

    // Check if NTFS
    bool b = CheckFileSystemType(ArgvPath, &aArgvPath->DriveType);
    if (!b && aExpression)
    {
      int PathParseSwitchOffSize = IsVeryLongPath(ArgvPath) ? PATH_PARSE_SWITCHOFF_SIZE : 0;
      if (DRIVE_UNKNOWN == aArgvPath->DriveType)
      {
        RetVal = ERR_SOURCE_DIR_DOES_NOT_EXIST;
        if (gLogLevel != FileInfoContainer::eLogQuiet)
          fwprintf(gStdOutFile, L"ERROR: Source directory '%s' does not exist\n", &ArgvPath[PathParseSwitchOffSize]);
      }
      else
      {
        // During automated test we need a filesystem to copy ADS to, but which does not support ADS. FAT is such 
        // a filesystem. So we need a switch to enable ln to copy to FAT, and thus cause errors.
        if (!gSwitchOffNtfsCheck)
        {
          RetVal = ERR_HARDLINKS_UNSUPPORTED;
          if (gLogLevel != FileInfoContainer::eLogQuiet)
            fwprintf(gStdOutFile, L"ERROR: volume type for '%s' does not support hardlinks\n", &ArgvPath[PathParseSwitchOffSize]);
        }
      }
    }

    // Prepare the path and get the attribute
    // By the end we should have a path which has no traling slash, except for x:\ 
    if (!wcsncmp(ArgvPath, PATH_LONG_GLOBALROOT, PATH_LONG_GLOBALROOT_SIZE))
    {
      // It is \\?\ we need the traling slash to get proper attributes
      size_t Len = wcslen(ArgvPath);
      if (ArgvPath[Len - 1] != '\\')
        wcscat_s(ArgvPath, HUGE_PATH, L"\\");

      aArgvPath->FileAttribute = GetFileAttributes(ArgvPath);

      // But afterwards cut off the trailing slash. Do it on our own, because
      // the PathXXX from shlwapi.dll function works differently on all plattforms 
      Len = wcslen(ArgvPath);
      if (ArgvPath[Len - 1] == '\\')
        ArgvPath[Len - 1] = 0x00;
    }
    else
    {
      // Do not chop of the traling \ if it is \\?Volume{
      if (wcsncmp(&ArgvPath[PATH_PARSE_SWITCHOFF_SIZE], PATH_VOLUME_GUID, PATH_VOLUME_GUID_SIZE))
      {
        // This is either x:\ or x:\bla\foo\ 
        if (!PathIsRoot(&ArgvPath[PATH_PARSE_SWITCHOFF_SIZE]))
        {
          // Remove a trailing slash. Do it on our own, because the PathXXX 
          // from shlwapi.dll function works differently on all plattforms 
          size_t Len = wcslen(ArgvPath);
          if (ArgvPath[Len - 1] == '\\')
            ArgvPath[Len - 1] = 0x00;
        }
        else
        {
          // \\?\Volume
        }
      }
      aArgvPath->FileAttribute = GetFileAttributes(ArgvPath);
    }

    aArgvPath->Argv = ArgvPath;
  }

  return RetVal;
}

int
Usage()
{
  if (gAutomatedTest)
    fwprintf(gStdOutFile, L"ln. (Recursively) creates hardlinks for files within NTFS partitions\n");
  else
    fwprintf(gStdOutFile, L"ln %S (Recursively) creates hardlinks for files within NTFS partitions\n", VERSION_NUMBER);
  fwprintf(gStdOutFile, L"Usage. ln [options] [commands] file|dir|UNCname [name|dir|UNCname [dir]]\n\n");
  fwprintf(gStdOutFile, L"Commands\n");

  fwprintf(gStdOutFile, L"      --anchor PATH\n");
  fwprintf(gStdOutFile, L"  -c, --copy SRCPATH DESTPATH\n");
  fwprintf(gStdOutFile, L"      --deeppathdelete SRCPATH        Delete path longer than 256 characters\n");
  fwprintf(gStdOutFile, L"  -d, --delorean SRCPATH BACKUPPATH_0 BACKUPPATH_1\n");
  fwprintf(gStdOutFile, L"  -e, --enum SRCPATH                  Enumerate corresponding files\n");

  fwprintf(gStdOutFile, L"  -x, --exclude WILDCARD              Exclude file <wildcard>\n");
  fwprintf(gStdOutFile, L"  -X, --excludedir WILDCARD           Exclude directory Recursively <wildcard>\n");
  fwprintf(gStdOutFile, L"  -p, --excluderegexp REGEXP          Exclude file <reg expr filename>\n");
  fwprintf(gStdOutFile, L"  -P, --excluderegexpdir REGEXP       Exclude directory <reg expr directory>\n");
  fwprintf(gStdOutFile, L"      --include WILDCARD              Include files <wildcard>\n");
  fwprintf(gStdOutFile, L"      --includedir WILDCARD           Include directory Recursively <wildcard>\n");
  fwprintf(gStdOutFile, L"      --includeregexp REGEXP          Include file <reg expr filename>\n");
  fwprintf(gStdOutFile, L"      --includeregexpdir REGEXP       Include directory <reg expr directory>\n");
  fwprintf(gStdOutFile, L"  -j, --junction JUNCTION [TARGETDIR] Show/Create a junction\n");
  fwprintf(gStdOutFile, L"  -l, --list FILENAME                 List hardlink siblings\n");

  fwprintf(gStdOutFile, L"  -o, --mirror SRCPATH DESTPATH\n");
  fwprintf(gStdOutFile, L"  -m, --move SRCPATH DESTPATH\n");
  fwprintf(gStdOutFile, L"      --output FILENAME               Redirect output to FILENAME\n");
  fwprintf(gStdOutFile, L"      --probefs PATH                  Show type of filesystem\n");
  fwprintf(gStdOutFile, L"  -r, --recursive SRCPATH DESTPATH    Create a clone\n");
  fwprintf(gStdOutFile, L"      --source SRCPATH                Specify additional source directories\n");
  fwprintf(gStdOutFile, L"  -i, --splice [WILDCARD]             Splice Junctions/Symlink dirs <wildcard>\n");
  fwprintf(gStdOutFile, L"  -s, --symbolic SYMBOLICLINK         Show symbolic link target\n");

  fwprintf(gStdOutFile, L"      --timetolerance MILLISECONDS    File comparison tolerance <miliseconds>\n");
  fwprintf(gStdOutFile, L"      --truesize SRCPATH              Used space taking into account hardlinks\n");
  fwprintf(gStdOutFile, L"  -u, --unroll [WILDCARD]             Unroll Junctions/Symlink dirs <wildcard>\n");
  fwprintf(gStdOutFile, L"\nOptions\n");
  fwprintf(gStdOutFile, L"  -b, --backup                        Backup mode copies also ACLs\n");
  fwprintf(gStdOutFile, L"  -a, --absolute                      Force symbolic links target to absolute\n");

  fwprintf(gStdOutFile, L"      --dupemerge                     Use Dupemerge' hashing to find hardlinks\n");
  fwprintf(gStdOutFile, L"  -h, --help                          This help\n");
  fwprintf(gStdOutFile, L"      --json                          Prints the output in json format\n");
  fwprintf(gStdOutFile, L"      --keepsymlinkrelation           Save absolute/relative state of symlinks\n");
  fwprintf(gStdOutFile, L"      --noea                          Do not copy EA Records\n");
  fwprintf(gStdOutFile, L"      --noads                         Do not copy Alternative Data Streams (ADS)\n");
  fwprintf(gStdOutFile, L"      --progress                      Show a progress indicator on the console\n");
  fwprintf(gStdOutFile, L"  -q, --quiet                         Operation with no output\n");
  fwprintf(gStdOutFile, L"      --skipfiles                     Don't operate on files, but only on dirs\n");
  fwprintf(gStdOutFile, L"  -s, --symbolic                      Create symbolic link\n");

  fwprintf(gStdOutFile, L"      --traditional                   Use compatibility mode for remote drives\n");
  fwprintf(gStdOutFile, L"      --1023safe                      Workaround the 1023 hardlink limit\n\n");

  fwprintf(gStdOutFile, L"Samples\n");
  fwprintf(gStdOutFile, L"  ln sourcefile.txt destination.txt\n");
  fwprintf(gStdOutFile, L"  ln --recursive x:\\dir\\dir2 x:\\dir\\newdir\n");
  fwprintf(gStdOutFile, L"  ln --recursive x:\\dir\\dir2 x:\\anotherdir\n");
  fwprintf(gStdOutFile, L"  ln \\\\local_computer\\sourcefile.txt x:\\dir_on_local_computer\\destination.txt\n");
  fwprintf(gStdOutFile, L"  ln \\\\local_computer\\sourcefile.txt \\\\local_computer\\destination.txt\n");
  fwprintf(gStdOutFile, L"  ln --recursive \\\\local_computer\\sourcedir x:\\dir\\dir2\n");
  fwprintf(gStdOutFile, L"  ln --recursive \\\\local_computer\\sourcedir \\\\local_computer\\destdir\\sample\n");
  fwprintf(gStdOutFile, L"  ln --enum x:\\dir\\dir2\n");
  fwprintf(gStdOutFile, L"  ln --copy x:\\source\\dir2 x:\\dest\\dir2\n");
  fwprintf(gStdOutFile, L"  ln --source x:\\src\\loc1 --copy x:\\source\\dir2 x:\\dest\\dir2\n");
  fwprintf(gStdOutFile, L"  ln --junction x:\\source\\junction x:\\dest\\junction target\n");
  fwprintf(gStdOutFile, L"  ln --junction x:\\source\\junction\n");
  fwprintf(gStdOutFile, L"  ln --symbolic sourcefile.txt destination.txt\n");
  fwprintf(gStdOutFile, L"  ln --list sourcefile.txt\n");

  return 42;
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

#include "Aclapi.h"

bool
IsProtectedFolder(wchar_t* aPath)
{
  PSID ppsidOwner;
  PSID ppsidGroup;
  PACL ppSacl= nullptr;
  PACL pOldDACL = nullptr, pNewDACL = nullptr;
  PSECURITY_DESCRIPTOR pSD = nullptr;


  DWORD dwRes = GetNamedSecurityInfo(aPath, SE_FILE_OBJECT,DACL_SECURITY_INFORMATION, &ppsidOwner, &ppsidGroup, &pOldDACL, &ppSacl, &pSD);
  if (ERROR_SUCCESS != dwRes)
  {
    fwprintf (gStdOutFile, L"GetNamedSecurityInfo Error: %d \n", dwRes);
  }


  BOOL lpbDaclPresent;
  PACL pDacl;
  BOOL lpbDaclDefaulted;

  if (! GetSecurityDescriptorDacl(pSD, &lpbDaclPresent, &pDacl, &lpbDaclDefaulted))
  {
    cout<<GetLastError();
  }

  if ( lpbDaclPresent == FALSE)
  {
    fwprintf (gStdOutFile, L"\n No DACL Present");
  }

  if ( lpbDaclPresent && pDacl == nullptr)
  {
    fwprintf (gStdOutFile, L"\n A NULL DACL implicitly allows all access to an object");
  }

  if ( lpbDaclDefaulted == TRUE)
  {
    fwprintf (gStdOutFile, L"\n the DACL was retrieved by a default mechanism");
  }
  else
  {
    fwprintf (gStdOutFile, L"\n the DACL was explicitly specified by a user.");
  }

  return true;
}

#  ifdef __cplusplus
extern "C"
{
#  endif // __cplusplus
int
wmain(
  int argc,
  TCHAR* argv[]
)
{
  bool	recursive = false;
  bool  RecursiveOptional = false;
  bool  HardlinkMirror = false;
  bool	enumerate = false;
  bool	Symbolic = false;
  bool	Absolute = false;
  bool	SmartCopy = false;
  bool	list = false;
  bool	junction = false;
  bool	noitcnuj = false;
  bool	SmartMove = false;
  bool  DeepPathDelete = false;
  bool  DeepPathCreate = false;
  bool  delorean = false;
  bool  AlwaysSplice = false;
  bool  AlwaysUnroll = false;
  bool  AlwaysFollow = false;
  bool  SmartMirror = false;
  bool  bSkipFiles = false;
  bool  Traditional = false;
  bool  SmartRename = false;
  bool  bTrueSize = false;
  bool  DeloreanDelete = false;
  bool  AdsDev = false;
  bool  KeepSymlinkRelation = false;
  bool  Backup = false;
  bool  TimeTolerance = false;
  int   TimeToleranceValue = 0;
  bool  ProbeFs = false;
  bool  EnumVolumes = false;
  int   HardlinkLimitValue = cMaxHardlinkLimit;
  int   gGenerateHardlinks = 0;
  bool  DeloreanMerge = false;

  _StringList IncludeFileList;
  _StringList IncludeDirList;
  _StringList ExcludeFileList;
  _StringList ExcludeDirList;
  _StringList UnrollDirList;
  _StringList FollowDirList;
  _StringList SpliceDirList;
  _ArgvList SourceDirList;

  // For some unknown reason the startup code in msvcrt does not
  // properly open up stderr, so geopt must be disabled to print extended
  // error message to stderr
  opterr = 0;
#if defined DEBUG_TEST_PRINT_PROGRESS
  TestPrintProgress();
#endif
  InitCreateHardlink();

#if 0
  wchar_t		szProcessNameUni[1024];
  PUINT_PTR dPID;
  ULONG		dPIDSize = 0;

  wcscpy_s(szProcessNameUni, 1024, _T("winfile.exe"));
  bool b = NtQueryProcessId(szProcessNameUni, &dPID, &dPIDSize);

  printf("42\n");
#endif

#if 0

  wchar_t TestFixVeryLongPath[HUGE_PATH];
  wchar_t* pTestFixVeryLongPath = TestFixVeryLongPath;
  wcscpy(TestFixVeryLongPath, L"\\\\?\\c:\\Gallia\\est\\omnis\\divisa\\in\\partes\\tres\\quarum\\unam\\incolunt\\Belgae\\aliam\\Aquitani\\tertiam\\qui\\ipsorum\\lingua\\Celtae\\nostra\\Galli\\appellantur.\\Hi\\omnes\\lingua\\institutis\\legibus\\inter\\se\\differunt.\\Gallos\\ab\\Aquitanis\\Garumna\\flumen\\a\\Belgis\\Matrona\\et\\Sequana\\dividit.\\Horum\\omnium\\fortissimi\\sunt\\Belgae\\propterea\\quod\\a\\cultu\\atque\\humanitate\\provinciae\\longissime\\absunt\\minimeque\\ad\\eos\\mercatores\\saepe\\commeant\\atque\\ea\\quae\\ad\\effeminandos\\animos\\pertinent\\important\\proximique\\sunt\\Germanis\\qui\\trans\\Rhenum\\incolunt\\quibuscum\\continenter\\bellum\\gerunt.\\Qua\\de\\causa\\Helvetii\\quoque\\reliquos\\Gallos\\virtute\\praecedunt\\quod\\fere\\cotidianis\\proeliis\\cum\\Germanis\\contendunt\\cum\\aut\\suis\\finibus\\eos\\prohibent\\aut\\ipsi\\in\\eorum\\finibus\\bellum\\gerunt");
  FixVeryLongPath(&pTestFixVeryLongPath);

  wcscpy(TestFixVeryLongPath, L"\\\\?\\c:\\Gallia\\est\\omnis\\divisa");
  pTestFixVeryLongPath = TestFixVeryLongPath;
  FixVeryLongPath(&pTestFixVeryLongPath);

  wcscpy(TestFixVeryLongPath, L"c:\\Gallia\\est\\omnis\\divisa\\in\\partes\\tres\\quarum\\unam\\incolunt\\Belgae\\aliam\\Aquitani\\tertiam\\qui\\ipsorum\\lingua\\Celtae\\nostra\\Galli\\appellantur.\\Hi\\omnes\\lingua\\institutis\\legibus\\inter\\se\\differunt.\\Gallos\\ab\\Aquitanis\\Garumna\\flumen\\a\\Belgis\\Matrona\\et\\Sequana\\dividit.\\Horum\\omnium\\fortissimi\\sunt\\Belgae\\propterea\\quod\\a\\cultu\\atque\\humanitate\\provinciae\\longissime\\absunt\\minimeque\\ad\\eos\\mercatores\\saepe\\commeant\\atque\\ea\\quae\\ad\\effeminandos\\animos\\pertinent\\important\\proximique\\sunt\\Germanis\\qui\\trans\\Rhenum\\incolunt\\quibuscum\\continenter\\bellum\\gerunt.\\Qua\\de\\causa\\Helvetii\\quoque\\reliquos\\Gallos\\virtute\\praecedunt\\quod\\fere\\cotidianis\\proeliis\\cum\\Germanis\\contendunt\\cum\\aut\\suis\\finibus\\eos\\prohibent\\aut\\ipsi\\in\\eorum\\finibus\\bellum\\gerunt");
  pTestFixVeryLongPath = TestFixVeryLongPath;
  FixVeryLongPath(&pTestFixVeryLongPath);

  wcscpy(TestFixVeryLongPath, L"c:\\Gallia\\est\\omnis\\divisa");
  pTestFixVeryLongPath = TestFixVeryLongPath;
  FixVeryLongPath(&pTestFixVeryLongPath);

  WCHAR	ps[MAX_PATH];
  //		WCHAR	rs[MAX_PATH] = L"D:\\1\\21\\hermann";
  //		WCHAR	rs[MAX_PATH] = L"D:\\1\\test\\1\\1\\1\\1\\1\\1";
  WCHAR	rs[MAX_PATH] = L"D:\\1\\x - SymbolicLink\\x";
  ReparseCanonicalize(rs, ps);


  IsVeryLongPath(L"\\\\?\\hallo");
  IsVeryLongPath(L"h");
  IsVeryLongPath(L"halo");
  IsVeryLongPath(L"hal");


  wchar_t r0[MAX_PATH];
  wchar_t r1[MAX_PATH];
  wchar_t r2[MAX_PATH];

  PathCanonicalize(r0, L"D:\\1\\AAa\\SmartCopyTest\\..\\..\\2_\\0000\\sym.txt");

  wcscpy(r0, L"D:\\1\\2_\\hermann - SymbolicLink");
  ProbeSymbolicLink(r0, r1);
  ResolveSymboliclink(r0, r1, r2);

  wcscpy(r0, L"D:\\TEMP - SymbolicLink");
  ProbeSymbolicLink(r0, r1);
  ResolveSymboliclink(r0, r1, r2);

  wcscpy(r0, L"d:\\Criss - SymbolicLink.bmp");
  ProbeSymbolicLink(r0, r1);
  ResolveSymboliclink(r0, r1, r2);

  wcscpy(r0, L"D:\\1\\2_\\gerhild - SymbolicLink.txt");
  ProbeSymbolicLink(r0, r1);
  ResolveSymboliclink(r0, r1, r2);

  wcscpy(r0, L"D:\\gerhild.txt");
  ProbeSymbolicLink(r0, r1);
  ResolveSymboliclink(r0, r1, r2);

  wchar_t a1[MAX_PATH];
  wchar_t a2[MAX_PATH];
  wcscpy(a1, L"D:\\1\\2\\hermann.txt");
  wcscpy(a2, L"D:\\1\\2\\gerhild.txt");
  CreateSymboliclink(a1, a2, 0);
  wcscpy(a1, L"D:\\1\\2\\hermann.txt");
  wcscpy(a2, L"D:\\1\\2\\gerhild.txt");
  CreateSymboliclink(a1, a2, SYMLINK_FLAG_RELATIVE);
  wcscpy(a1, L"D:\\1\\hallo\\hermann.txt");
  wcscpy(a2, L"D:\\1\\2\\gerhild.txt");
  CreateSymboliclink(a1, a2, SYMLINK_FLAG_RELATIVE);
  wcscpy(a1, L"E:\\1\\2\\hermann.txt");
  wcscpy(a2, L"D:\\1\\2\\gerhild.txt");
  CreateSymboliclink(a1, a2, SYMLINK_FLAG_RELATIVE);
  printf("Test Codign still active!!!\n");
#endif

#if 0
  wchar_t SourcePath[HUGE_PATH];
  wchar_t aBackup0Path[HUGE_PATH];
  wchar_t aBackup1Path[HUGE_PATH];

  wchar_t		dp[MAX_PATH];
  wcscpy(dp, L"lib");
  wchar_t*	pFilename = dp;

  CreateTimeStampedFileName(L"d:\\1\\tm\\dest", aBackup0Path, aBackup1Path, pFilename);
#endif

#if 0
  FileInfo* pf = new FileInfo;
  wchar_t ff[] = { L"d:\\test\\tmp\\OldSourcePath" };
  wchar_t fn[MAX_PATH];
  wcscpy(fn, ff);
  wcscat(fn, L"\\hermann\\testet\\vor\\sichhin.txt");

  pf->m_FileName = new wchar_t[wcslen(fn) + 1];

  wcscpy(pf->m_FileName, fn);
  pf->m_SourcePathLen = wcslen(ff);

  wchar_t ss[] = { L"d:\\tmpneu\\ganz\\neu\\nocheinmal\\neu\\NewSourcePath" };
  pf->ReplaceSourcePath(ss, wcslen(ss));
#endif


#if 0 // Regular Expression Test
  regex_t*  compiled = new regex_t;
  int			retval = regwcomp(compiled, L"\\\\AppData\\\\.+\\\\Microsoft\\\\?", REG_ICASE | REG_EXTENDED);

  if (REG_OK == retval)
  {
    regmatch_t pmatch[1];
    retval = regwexec(compiled, L"C:\\Users\\clanger\\AppDta\\Roaming\\Microsoft\\Vault", 1, pmatch, 0);
    retval = regwexec(compiled, L"C:\\Users\\clanger\\AppData\\Roaming\\Microsoft\\Vault", 1, pmatch, 0);
  }
  else
    printf("failed to compile regexp\n");

#endif


#if 0
  regex_t*  compiled = new regex_t;
  int			retval = regwcomp(compiled, L"^t.*p$", REG_ICASE | REG_EXTENDED);

  regmatch_t pmatch[32];
  retval = regwexec(compiled, L"tomp", 32, pmatch, 0);

  retval = regwexec(compiled, L"tempur", 32, pmatch, 0);

  retval = regwexec(compiled, L"sodltemp", 32, pmatch, 0);

  retval = regwexec(compiled, L"asdasdtempdssd", 32, pmatch, 0);

  retval = regwexec(compiled, L"temp", 32, pmatch, 0);
#endif

#if 0
  // std regex test
  wregex RegEx(_T("\\.*\\.ini$"));
  wcmatch match;
  regex_search(_T("\\\\?\\c:\\$RECYCLE.BIN\\S-1-5-21-1054053922-559824688-2072063007-3334\\desktop.ini"), match, RegEx);
#endif


#if 0
  wchar_t NewLocation[HUGE_PATH];
  wchar_t NewLocationSym[HUGE_PATH];

  wcscpy_s(NewLocation, HUGE_PATH, L"\\\\?\\E:\\data\\cpp\\hardlinks\\ln\\dp\\Gallia\\est\\omnis\\divisa\\in\\partes\\tres\\quarum\\unam\\incolunt\\Belgae\\aliam\\Aquitani\\tertiam\\qui\\ipsorum\\lingua\\Celtae\\nostra\\Galli\\appellantur\\Hi\\omnes\\lingua\\institutis\\legibus\\inter\\se\\differunt\\Gallos\\ab\\Aquitanis\\Garumna\\flumen\\a\\Belgis\\Matrona\\et\\Sequana\\dividit\\Horum\\omnium\\fortissimi\\sunt");
  wcscpy_s(NewLocationSym, HUGE_PATH, L"\\\\?\\E:\\data\\cpp\\hardlinks\\ln\\dp\\Gallia\\est\\omnis\\divisa\\in\\partes\\tres\\quarum\\unam\\incolunt\\Belgae\\aliam\\Aquitani\\tertiam\\qui\\ipsorum\\lingua\\Celtae\\nostra\\Galli\\appellantur\\Hi\\omnes\\lingua\\institutis\\legibus\\inter\\se\\differunt\\Gallos\\ab\\Aquitanis\\Garumna\\flumen\\a\\Belgis\\Matrona\\et\\Sequana\\dividit\\Horum\\omnium\\fortissimi\\sunt_s");

  wcscpy_s(NewLocation, HUGE_PATH, L"\\\\?\\E:\\data\\cpp\\hardlinks\\ln\\dp\\Gallia\\Hi\\omnes\\lingua\\institutis\\legibus\\inter\\se\\differunt\\omnium\\fortissimi\\sunt");
  wcscpy_s(NewLocationSym, HUGE_PATH, L"\\\\?\\E:\\data\\cpp\\hardlinks\\ln\\dp\\Gallia\\Hi\\omnes\\lingua\\institutis\\legibus\\inter\\se\\differunt\\omnium\\fortissimi\\sunt_s");

  wcscpy_s(NewLocation, HUGE_PATH, L"\\\\?\\E:\\data\\cpp\\hardlinks\\ln\\dp\\Gallia\\sunt_dead");
  wcscpy_s(NewLocationSym, HUGE_PATH, L"\\\\?\\E:\\data\\cpp\\hardlinks\\ln\\dp\\Gallia\\sunt_s");

  //    wchar_t result[HUGE_PATH];
  //    wchar_t* pp = GetRelativeFilename(result, NewLocation, NewLocationSym);
  //    PathRelativePathTo(result, &NewLocation[PATH_PARSE_SWITCHOFF_SIZE], FILE_ATTRIBUTE_DIRECTORY, &NewLocationSym[PATH_PARSE_SWITCHOFF_SIZE], FILE_ATTRIBUTE_DIRECTORY);
  DWORD ERR = CreateSymboliclink(NewLocationSym, NewLocation, SYMLINK_FLAG_DIRECTORY);
#endif

#if 0
  wchar_t NewLocation[HUGE_PATH];
  wchar_t NewLocationSym[HUGE_PATH];

  wcscpy_s(NewLocation, HUGE_PATH, L"\\\\?\\E:\\data\\cpp\\hardlinks\\ln\\dp\\Gallia\\sunt_dead");
  wcscpy_s(NewLocationSym, HUGE_PATH, L"\\\\?\\E:\\data\\cpp\\hardlinks\\ln\\dp\\Gallia\\sunt_s");

  CreateJunction(NewLocation, NewLocationSym);

#endif

#if 0
  wchar_t  TS[MAX_PATH];
  wcscpy_s(TS, MAX_PATH, L"f:\\test\\1\\2");
  wcscpy_s(TS, MAX_PATH, L"\\\\?\\C:\\Dokumente und Einstellungen\\ich\\Eigene Dateien\\Eigene Musik\\Alles\\Beatport Top100 - 2009\\02-Pryda - Miami To Atlanta (Original Mix).mp3");

  int l = wcslen(TS);
  int cr = CreateDirectoryHierarchy(TS, l);
#endif

#if 0
  bool admin = IsProtectedFolder(L"c:\\tmp");
  //    bool admin = IsProtectedFolder(L"c:\\windows");
  printf("Proteced Folder %d\n", admin);
  exit(ERR_ERROR);
#endif

#if 0
  bool SupportedFS = gSupportedFileSystems.IsSupportedFileSystem(L"btrfs");
#endif

  gStdOutHandle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
  int StdOutDesc = _open_osfhandle(gStdOutHandle, _O_TEXT);
  gStdOutFile = _fdopen(StdOutDesc, "w");

  // Change the buffer size so that outputing results does not cause stalls during printout
  // which is difficult, because the safest way would be to go with _IONBF, but this slows
  // down operation. If we go for buffering with _IOFBF/_IOLBF a big file might take so long 
  // to copy, that even the buffer runs out and again the output print stalls
  // Even if unfortunatley _IOFBF is the same as _IOLBF we take _IOLBF, because we really 
  // want line buffering
  int rr = setvbuf(gStdOutFile, nullptr, _IOLBF, 1'000'000);

  // Take the default locale
  setlocale(LC_ALL, "");
  g_locale_t = _get_current_locale();

  // Convert the wide options to ansi options
  char**a_argv = new char*[argc + 1];
  int i;
  for (i = 0; i < argc; i++)
  {
    int argv_length = (int)wcslen(argv[i]) + 1;
    a_argv[i] = new char[argv_length];
    WideCharToMultiByte(
      CP_ACP,
      0,
      argv[i],
      argv_length,
      a_argv[i],
      argv_length,
      nullptr,
      nullptr
    );
  }
  a_argv[argc] = NULL;

  TCHAR	Argv1[HUGE_PATH];
  TCHAR	Argv2[HUGE_PATH];
  TCHAR	Argv3[HUGE_PATH];
  Argv3[0] = 0x00;

  while (1)
  {
    int longind;
    char c = ultragetopt_tunable(
      argc,
      a_argv,
      "R:r:e:q::c:j:J:sal:Am:o:z:Z:d:x:X:p:P:u::U::i::I::bh",
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
      // --clone
    case 'R':
      // --recursive
    case 'r':
    {
      recursive = true;
      if (optind + 1 == argc)
      {
        wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
        wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
      }
      else
      {
        if (nullptr == optarg)
          RecursiveOptional = true;
      }
    }
    break;

    // --symbolic
    // 
    case 's':
      Symbolic = true;
      break;

    case 'a':
      Absolute = true;
      break;

    case 'q':
      if (nullptr == optarg)
        gLogLevel = FileInfoContainer::eLogQuiet;
      else
      {
        int LogLev = _wtoi(argv[optind - 1]);
        switch (LogLev)
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

        }
      }

      break;

    case 'h':
      Usage();
      Exit(0);
      break;


      // --enum
      // 
    case 'e':
    {
      wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
      Argv2[0] = 0x00;
      enumerate = true;
    }
    break;

    case 'c':
    {
      SmartCopy = true;
      if (optind + 1 == argc)
      {
        wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
        wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
      }
      else
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }
    }
    break;

    case 'l':
    {
      wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
      Argv2[0] = 0x00;
      list = true;
    }
    break;

    //
    // --junction
    // 
    case 'j':
    {
      junction = true;
      wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
      if (optind < argc)
        wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
      else
        Argv2[0] = 0x00;
    }
    break;

    //
    // --noitcnuj
    // 
    case 'J':
    {
      noitcnuj = true;
      wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
      if (optind < argc)
        wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
      else
        Argv2[0] = 0x00;
    }
    break;

    case 'A':
    {
      gAutomatedTest = true;
    }
    break;

    // --Smartmove
    case 'm':
    {
      SmartMove = true;
      if (optind + 1 == argc)
      {
        wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
        wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
      }
      else
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }
    }
    break;

    // --mirror
    case 'o':
    {
      SmartMirror = true;
      if (optind + 1 == argc)
      {
        wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
        wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
      }
      else
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }
    }
    break;

    case 'z':
    {
      wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
      Argv2[0] = 0x00;
      DeepPathCreate = true;
    }
    break;

    case 'Z':
    {
      wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
      Argv2[0] = 0x00;
      DeepPathDelete = true;
    }
    break;

    // --delorean
    // 
    case 'd':
    {
      delorean = true;
      if (optind + 1 >= argc)
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }
      wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
      wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
      wcscpy_s(Argv3, HUGE_PATH, argv[optind + 1]);
    }
    break;

    // --exclude
    // 
    case 'x':
    {
      if (optind == argc)
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }

      _StringList RawArgs;
      if ('@' == argv[optind - 1][0])
        ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
      else
        RawArgs.push_back(wstring(argv[optind - 1]));

      for (_StringListIterator iter = RawArgs.begin(); iter != RawArgs.end(); ++iter)
      {
        WildCard2RegExp(*iter);

        *iter = L"\\\\" + *iter + L"$";

        ExcludeFileList.push_back(*iter);
      }
    }
    break;

    // --excludedir
    // 
    case 'X':
    {
      if (optind == argc)
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
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
        if (iter->length() > 1 && iter->at(1) != ':')
          *iter = L"\\\\" + *iter + L"$";
        else
          *iter += L"$";
        ExcludeDirList.push_back(*iter);
      }
    }
    break;

    // --excluderegexp
    // 
    case 'p':
    {
      if (optind == argc)
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }

      if ('@' == argv[optind - 1][0])
        ReadArgsFromFile(&argv[optind - 1][1], ExcludeFileList);
      else
        ExcludeFileList.push_back(wstring(argv[optind - 1]));
    }
    break;

    // --excluderegexpdir
    // 
    case 'P':
    {
      if (optind == argc)
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }

      if ('@' == argv[optind - 1][0])
        ReadArgsFromFile(&argv[optind - 1][1], ExcludeDirList);
      else
        ExcludeDirList.push_back(wstring(argv[optind - 1]));
    }
    break;

    // --unroll
    case 'u':
    {
      if (nullptr == optarg)
        AlwaysUnroll = true;
      else
      {
        _StringList RawArgs;
        if ('@' == argv[optind - 1][0])
          ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
        else
          RawArgs.push_back(wstring(argv[optind - 1]));

        for (_StringListIterator iter = RawArgs.begin(); iter != RawArgs.end(); ++iter)
        {
          stringreplace(*iter, wstring(L"."), wstring(L"\\."));
          stringreplace(*iter, wstring(L"*"), wstring(L".*"));
          stringreplace(*iter, wstring(L"?"), wstring(L"."));
          UnrollDirList.push_back(*iter);
        }
      }
    }
    break;

    // --unrollregexp
    case 'U':
    {
      if (nullptr == optarg)
        AlwaysUnroll = true;
      else
      {
        if ('@' == argv[optind - 1][0])
          ReadArgsFromFile(&argv[optind - 1][1], UnrollDirList);
        else
          UnrollDirList.push_back(wstring(argv[optind - 1]));
      }
    }
    break;

    // --splice
    case 'i':
    {
      if (nullptr == optarg)
        AlwaysSplice = true;
      else
      {
        _StringList RawArgs;
        if ('@' == argv[optind - 1][0])
          ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
        else
          RawArgs.push_back(wstring(argv[optind - 1]));

        for (_StringListIterator iter = RawArgs.begin(); iter != RawArgs.end(); ++iter)
        {
          stringreplace(*iter, wstring(L"."), wstring(L"\\."));
          stringreplace(*iter, wstring(L"*"), wstring(L".*"));
          stringreplace(*iter, wstring(L"?"), wstring(L"."));
          SpliceDirList.push_back(*iter);
        }
      }
    }
    break;

    // --spliceregexp
    case 'I':
    {
      if (nullptr == optarg)
        AlwaysSplice = true;
      else
      {
        if ('@' == argv[optind - 1][0])
          ReadArgsFromFile(&argv[optind - 1][1], SpliceDirList);
        else
          SpliceDirList.push_back(wstring(argv[optind - 1]));
      }
    }
    break;

    // Copy Backup attributes
    // --backup
    case 'b':
    {
      Backup = true;
      KeepSymlinkRelation = true;
    }
    break;



    // We end up here if it was just long opts
    case 0:
    {
      switch (longind)
      {
        // --source
        // 
        // Additional source directories given for Smartcopy and SmartClone
      case cBaseJustLongOpts + 0x00:
      {
        // There can be more than one source directories so we store the source dirs in a list
        _ArgvPath       ArgvXPath;
        ArgvXPath.ArgvOrg = argv[optind - 1];
        ArgvXPath.Argv = argv[optind - 1];
        int r = CheckArgument(&ArgvXPath, true);

        SourceDirList.push_back(ArgvXPath);
      }
      break;

      // --deeppathcopy
      // 
      // Only for testing purposes
      case cBaseJustLongOpts + 0x01:
      {
        // 
        wstring Dest(argv[optind - 1]);

        wchar_t*  pSourceFilePart;
        wchar_t*  pFilePart;
        wchar_t SourceFullPath[HUGE_PATH];
        wchar_t DestFullPath[HUGE_PATH];
        wcscpy_s(SourceFullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        wcscpy_s(DestFullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(argv[optind - 1], HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &SourceFullPath[PATH_PARSE_SWITCHOFF_SIZE], &pSourceFilePart);
        GetFullPathName(argv[optind], HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &DestFullPath[PATH_PARSE_SWITCHOFF_SIZE], &pFilePart);

        if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(DestFullPath))
        {
          wcscat_s(DestFullPath, HUGE_PATH, L"\\");
          wcscat_s(DestFullPath, HUGE_PATH, pSourceFilePart);
        }

        BOOL r = CopyFile(SourceFullPath, DestFullPath, FALSE);
        if (r)
          fwprintf(gStdOutFile, L"+f %s\n", DestFullPath);
        else
          fwprintf(gStdOutFile, L"!f %s\n", DestFullPath);

        Exit(ERR_SUCCESS);

      }
      break;

      // --skipfiles
      // 
      case cBaseJustLongOpts + 0x02:
        bSkipFiles = true;
        break;

        // --traditional
        // 
      case cBaseJustLongOpts + 0x03:
        Traditional = true;
        break;

        // --SmartRename
        // 
      case cBaseJustLongOpts + 0x04:
      {
        SmartRename = true;
        if (optind + 1 == argc)
        {
          wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
          wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
        }
        else
        {
          Usage();
          Exit(ERR_LESS_CMD_ARGUMENTS);
        }
      }
      break;

      // --destination
      // 
      // Additional destination directories to simualte the behaviour from LSE. Make 
      // sure that after each --source a ---destination must follow, so that the 
      // source destination pairs match
      case cBaseJustLongOpts + 0x05:
      {
        // Check the argument and convert it to Fullpath
        _ArgvPath       ArgvDest;
        ArgvDest.Argv = argv[optind - 1];
        CheckArgument(&ArgvDest, true);

        // Since we use push_back() during --source we can assign the destination
        // to the last element in the list
        SourceDirList.back().ArgvDest = ArgvDest.Argv;
      }
      break;

      // --datetime
      // 
      // Simply print out the date and time for the deloreancopy.bat
      case cBaseJustLongOpts + 0x06:
      {
        _SYSTEMTIME SysTime;
        GetLocalTime(&SysTime);

        fwprintf(gStdOutFile, L"%04d-%02d-%02d %02d-%02d-%02d",
          SysTime.wYear,
          SysTime.wMonth,
          SysTime.wDay,
          SysTime.wHour,
          SysTime.wMinute,
          SysTime.wSecond
        );

        Exit(ERR_SUCCESS);
      }
      break;

      // --output
      // 
      case cBaseJustLongOpts + 0x07:
      {
        FILE* Output = _wfopen(argv[optind - 1], L"wt,ccs=UNICODE");
        if (Output)
        {
          fclose(gStdOutFile);
          CloseHandle((HANDLE)gStdOutHandle);

          HANDLE consoleHandle = CreateFile(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
          if (INVALID_HANDLE_VALUE != consoleHandle)
            SetStdHandle(STD_OUTPUT_HANDLE, consoleHandle);

          gStdOutFile = Output;
        }
      }
      break;

      // --truesize
      // 
      case cBaseJustLongOpts + 0x08:
      {
        wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
        Argv2[0] = 0x00;
        bTrueSize = true;
      }
      break;

      // --AdsDev
      // 
      case cBaseJustLongOpts + 0x09:
      {
        wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
        wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
        AdsDev = true;
      }
      break;

      // --automated_traditional
      // 
      // This is the same as --traditional, but during automated test an --traditional option is
      // needed which can be removed in the output gia GSAR, so that a full testrun produces the
      // same output with --automated_traditional and without
      case cBaseJustLongOpts + 0x0a:
        Traditional = true;
        break;

        // --KeepSymlinkRelation
        // 
      case cBaseJustLongOpts + 0x0b:
      {
        KeepSymlinkRelation = true;
      }
      break;

      // --timetolerance
      // 
      case cBaseJustLongOpts + 0x0c:
      {
        if (EOF == swscanf_s(argv[optind - 1], L"%d", &TimeToleranceValue))
        {
          fwprintf(gStdOutFile, L"Illegal TimeTolerance: '%s'\n", argv[optind - 1]);
          TimeToleranceValue = 0;
        }
        else
        {
          // Convert from msec to 100NanoSec units, which is used in FileTime calls
          TimeToleranceValue *= 10000;
          TimeTolerance = true;
        }
      }
      break;

      // --include
      // 
      case cBaseJustLongOpts + 0x0d:
      {
        if (optind == argc)
        {
          Usage();
          Exit(ERR_LESS_CMD_ARGUMENTS);
        }

        _StringList RawArgs;
        if ('@' == argv[optind - 1][0])
          ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
        else
          RawArgs.push_back(wstring(argv[optind - 1]));

        for (_StringListIterator iter = RawArgs.begin(); iter != RawArgs.end(); ++iter)
        {
          WildCard2RegExp(*iter);

          *iter = L"\\\\" + *iter + L"$";
          IncludeFileList.push_back(*iter);
        }
      }
      break;

      // --includedir
      // 
      case cBaseJustLongOpts + 0x0e:
      {
        if (optind == argc)
        {
          Usage();
          Exit(ERR_LESS_CMD_ARGUMENTS);
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

      // --includeregexp
      // 
      case cBaseJustLongOpts + 0x0f:
      {
        if (optind == argc)
        {
          Usage();
          Exit(ERR_LESS_CMD_ARGUMENTS);
        }

        if ('@' == argv[optind - 1][0])
          ReadArgsFromFile(&argv[optind - 1][1], IncludeFileList);
        else
          IncludeFileList.push_back(wstring(argv[optind - 1]));
      }
      break;

      // --includeregexpdir
      // 
      case cBaseJustLongOpts + 0x10:
      {
        if (optind == argc)
        {
          Usage();
          Exit(ERR_LESS_CMD_ARGUMENTS);
        }

        if ('@' == argv[optind - 1][0])
          ReadArgsFromFile(&argv[optind - 1][1], IncludeDirList);
        else
          IncludeDirList.push_back(wstring(argv[optind - 1]));
      }
      break;

      // --dupemerge
      // 
      case cBaseJustLongOpts + 0x11:
        gDupemerge = true;
        break;

        // --probefs
        // 
      case cBaseJustLongOpts + 0x12:
        wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
        Argv2[0] = 0x00;
        ProbeFs = true;
        break;

        // --enumvolumes
        // 
      case cBaseJustLongOpts + 0x13:
        EnumVolumes = true;
        break;

        // --switchoffntfscheck
        // 
      case cBaseJustLongOpts + 0x14:
        if (gAutomatedTest)
          gSwitchOffNtfsCheck = true;
        break;

        // --noads
        // 
      case cBaseJustLongOpts + 0x15:
        gNoAds = true;
        break;

        // --noea
        // 
      case cBaseJustLongOpts + 0x16:
        gNoEa = true;
        break;

        // --nosparsefile
        // 
      case cBaseJustLongOpts + 0x17:
        gNoSparseFile = true;
        break;

        // --nolocaluncresolve
        // 
      case cBaseJustLongOpts + 0x18:
        if (gAutomatedTest)
          gResolveUNC = false;
        break;

        // --anchor
        // 
      case cBaseJustLongOpts + 0x19:
      {
        // Create/fake a virtual source, so that junction and symlink resolvals work
        _ArgvPath       ArgvXPath;
        ArgvXPath.ArgvOrg = argv[optind - 1];
        ArgvXPath.Argv = argv[optind - 1];
        ArgvXPath.Flags = _ArgvPath::Anchor;
        int r = CheckArgument(&ArgvXPath, false);

        SourceDirList.push_back(ArgvXPath);
      }
      break;

      // --utf
      // 
      case cBaseJustLongOpts + 0x1a:
      {
        int StdOutDesc = _open_osfhandle(gStdOutHandle, _O_WTEXT);
        gStdOutFile = _fdopen(StdOutDesc, "w");
      }
      break;

      // --deloreanverbose
      // 
      case cBaseJustLongOpts + 0x1b:
      {
        gDeloreanVerbose = true;
      }
      break;

      // --deloreansleep
      // 
      case cBaseJustLongOpts + 0x1c:
      {
        if (EOF == swscanf_s(argv[optind - 1], L"%d", &gDeloreanSleep))
        {
          fwprintf(gStdOutFile, L"Illegal Delorean Sleep Time: '%s'\n", argv[optind - 1]);
          gDeloreanSleep = 0;
        }
      }
      break;


      // --hardlinklimit
      // 
      case cBaseJustLongOpts + 0x1d:
      {
        if (gAutomatedTest)
        {
          if (EOF == swscanf_s(argv[optind - 1], L"%d", &HardlinkLimitValue))
          {
            fwprintf(gStdOutFile, L"Illegal HardlinkLimit: '%s'\n", argv[optind - 1]);
            HardlinkLimitValue = cMaxHardlinkLimit;
          }
        }
      }
      break;

      // --generatehardlinks
      // 
      case cBaseJustLongOpts + 0x1e:
      {
        if (gAutomatedTest)
        {
          if (EOF == swscanf_s(argv[optind - 1], L"%d", &gGenerateHardlinks))
          {
            fwprintf(gStdOutFile, L"Illegal number of hardlinks to generate: '%s'\n", argv[optind - 1]);
            gGenerateHardlinks = 0;
          }
        }
      }
      break;

      // --1023safe
      // 
      case cBaseJustLongOpts + 0x1f:
      {
        g1023safe = true;
      }
      break;

      // --json
      // 
      case cBaseJustLongOpts + 0x20:
      {
        gJson = true;
      }
      break;

      // --forcestatts
      // 
      case cBaseJustLongOpts + 0x21:
        break;

        // --json
        // 
      case cBaseJustLongOpts + 0x22:
      {
        gProgress = true;
      }
      break;

      // --merge
      case cBaseJustLongOpts + 0x23:
      {
        DeloreanMerge = true;
        if (optind + 1 == argc)
        {
          wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
          wcscpy_s(Argv2, HUGE_PATH, argv[optind]);
        }
        else
        {
          Usage();
          Exit(ERR_LESS_CMD_ARGUMENTS);
        }
      }
      break;

      // --delete
      case cBaseJustLongOpts + 0x24:
      {
        wcscpy_s(Argv1, HUGE_PATH, argv[optind - 1]);
        Argv2[0] = 0x00;
        DeloreanDelete = true;
      }
      break;

      // --supportfs
      case cBaseJustLongOpts + 0x25:
      {
        gLSESettings.Add(argv[optind - 1]);
        Argv2[0] = 0x00;
      }
      break;

      // --follow
      case cBaseJustLongOpts + 0x26:
      {
        if (nullptr == optarg)
          AlwaysFollow = true;
        else
        {
          _StringList RawArgs;
          if ('@' == argv[optind - 1][0])
            ReadArgsFromFile(&argv[optind - 1][1], RawArgs);
          else
            RawArgs.push_back(wstring(argv[optind - 1]));

          for (auto iter : RawArgs)
          {
            stringreplace(iter, wstring(L"."), wstring(L"\\."));
            stringreplace(iter, wstring(L"*"), wstring(L".*"));
            stringreplace(iter, wstring(L"?"), wstring(L"."));
            FollowDirList.push_back(iter);
          }
        }
      }
      break;

      // --followregexp
      case cBaseJustLongOpts + 0x27:
      {
        if (nullptr == optarg)
          AlwaysFollow = true;
        else
        {
          if ('@' == argv[optind - 1][0])
            ReadArgsFromFile(&argv[optind - 1][1], FollowDirList);
          else
            FollowDirList.push_back(wstring(argv[optind - 1]));
        }
      }
      break;

      default:
        fwprintf(gStdOutFile, L"Unknown LongOpt %d:'%S'\n", longind, long_options[longind].name);
        break;
      }


    }
    break;

    default:
      break;
    }
  }

  // Internal options
  if (DeepPathCreate)
  {
    wchar_t*  pFilePart;
    wchar_t FullPath[HUGE_PATH];
    wcscpy_s(FullPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
    GetFullPathName(Argv1, HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &FullPath[PATH_PARSE_SWITCHOFF_SIZE], &pFilePart);

    size_t l = wcslen(FullPath);
    CreateDirectoryHierarchy(FullPath, l);
    fwprintf(gStdOutFile, L"+d %s\n", FullPath);
    Exit(ERR_SUCCESS);
  }

  // Recursive might also be optional if it is used with --recursive --mirror 
  if (recursive)
  {
    if (SmartMirror)
    {
      if (RecursiveOptional)
      {
        HardlinkMirror = true;
      }
      else
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }
    }
    else
    {
      if (optind + 1 != argc)
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }
    }
  }

  //
  // EnumVolumes
  //
  if (EnumVolumes)
  {
    _StringList VolumeList;
    EnumerateVolumes(VolumeList);

    for (_StringListIterator iter = VolumeList.begin(); iter != VolumeList.end(); ++iter)
      fwprintf(gStdOutFile, L"%s\n", (*iter).c_str());

    Exit(ERR_SUCCESS);
  }

  // All the listed option prepare Argv1 and Argv2 itself above, or are just switches
  if (!(recursive ^ enumerate ^ SmartCopy ^ list ^ junction ^ noitcnuj ^ SmartMove ^ DeepPathCreate ^ DeepPathDelete ^ delorean ^ SmartMirror ^ SmartRename ^ bTrueSize ^ AdsDev ^ ProbeFs ^ HardlinkMirror ^ DeloreanMerge ^ DeloreanDelete))
  {
    // Apply this only if no options 'ln source dest' is applied
    if (argc - optind == 2)
    {
      wcscpy_s(Argv1, HUGE_PATH, argv[optind]);
      wcscpy_s(Argv2, HUGE_PATH, argv[++optind]);
    }
    else
    {
      // There are options, which sometimes just have 1 argument
      if (Symbolic)
      {
        if (argc - optind == 1)
        {
          wcscpy_s(Argv1, HUGE_PATH, argv[optind]);
          Argv2[0] = 0x00;
        }
        else
        {
          Usage();
          Exit(ERR_LESS_CMD_ARGUMENTS);
        }
      }
      else
      {
        Usage();
        Exit(ERR_LESS_CMD_ARGUMENTS);
      }
    }

  }

  if (Backup == true)
  {
    BOOL b = EnableTokenPrivilege(SE_BACKUP_NAME);
    BOOL r = EnableTokenPrivilege(SE_RESTORE_NAME);
    EnableTokenPrivilege(SE_SECURITY_NAME);
    if (!(b && r))
    {
      fwprintf(gStdOutFile, L"Error can not set Backup priviliges\n");
      Backup = false;
    }
  }

  if (!gAutomatedTest)
    fwprintf(gStdOutFile, L"ln %S\n", VERSION_NUMBER);

  // General preperation of Arguments
  _ArgvPath       Argv1Path;
  Argv1Path.Argv = Argv1;
  Argv1Path.ArgvOrg = Argv1;
  int r = CheckArgument(&Argv1Path, !(noitcnuj || Symbolic || SmartCopy || SmartMirror || delorean || DeepPathDelete || DeloreanMerge));
  if (r < 0)
    Exit(r);

  wcscpy_s(Argv1, HUGE_PATH, Argv1Path.Argv.c_str());
  SourceDirList.push_back(Argv1Path);

  _ArgvPath       Argv2Path;
  Argv2Path.Argv = Argv2;
  Argv2Path.ArgvOrg = Argv2;
  r = CheckArgument(&Argv2Path, (!junction || Symbolic));
  if (r < 0)
    Exit(r);
  wcscpy_s(Argv2, HUGE_PATH, Argv2Path.Argv.c_str());

  _ArgvPath       Argv3Path;
  Argv3Path.Argv = Argv3;
  Argv3Path.ArgvOrg = Argv3;
  r = CheckArgument(&Argv3Path, true);
  if (r < 0)
    Exit(r);
  wcscpy_s(Argv3, HUGE_PATH, Argv3Path.Argv.c_str());


  int	RetVal = 0;

  //
  // --DeepPathDelete
  //
  if (DeepPathDelete)
  {
    if (INVALID_FILE_ATTRIBUTES == Argv1Path.FileAttribute)
      Exit (ERR_SOURCE_DIR_DOES_NOT_EXIST);

    if (Argv1Path.FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
    {
      XDelStatistics xs;
      XDel(Argv1, HUGE_PATH, xs, FileInfoContainer::eQuiet);
      RemoveDir(Argv1Path.Argv.c_str(), TRUE);
      if (gLogLevel != FileInfoContainer::eLogQuiet)
        fwprintf(gStdOutFile, L"-d %s\n", Argv1Path.ArgvOrg.c_str());
    }
    else
    {
      DeleteFile(Argv1Path.Argv.c_str());
      if (gLogLevel != FileInfoContainer::eLogQuiet)
        fwprintf(gStdOutFile, L"-f %s\n", Argv1Path.ArgvOrg.c_str());
    }
    Exit(ERR_SUCCESS);
  }

  // Symbolic is used here with one argument to show the target of the symbolic link
  if (Symbolic && !Argv2[0])
  {
    wchar_t SymlinkTarget[HUGE_PATH];
    wcscpy_s(SymlinkTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);
    BOOL b = ProbeSymbolicLink(Argv1, &SymlinkTarget[PATH_PARSE_SWITCHOFF_SIZE]);
    if (b)
    {
      DWORD SymlinkAttr;
      wchar_t SymlinkFullTarget[HUGE_PATH];

      int r = ResolveSymboliclink(Argv1, &SymlinkTarget[PATH_PARSE_SWITCHOFF_SIZE], SymlinkFullTarget);
      if (S_OK == r)
        SymlinkAttr = GetFileAttributes(SymlinkFullTarget);
      else
        SymlinkAttr = GetFileAttributes(SymlinkTarget);

      if (SymlinkAttr == INVALID_FILE_ATTRIBUTES)
        fwprintf(gStdOutFile, L"%s -#-> %s (%s)\n", Argv1Path.ArgvOrg.c_str(), &SymlinkTarget[PATH_PARSE_SWITCHOFF_SIZE], S_OK == r ? SymlinkFullTarget : &SymlinkTarget[PATH_PARSE_SWITCHOFF_SIZE]);
      else
        fwprintf(gStdOutFile, L"%s -> %s\n", Argv1Path.ArgvOrg.c_str(), &SymlinkTarget[PATH_PARSE_SWITCHOFF_SIZE]);

      Exit(ERR_SUCCESS);
    }
    else
    {
      // See if there was a file at last
      if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute)
      {
        //
        // If it was no symlink, then print out the number of references if it is a hardlink
        HANDLE FileHandle = CreateFile(Argv1, 0, 0, 0, OPEN_EXISTING, 0, 0);
        if (INVALID_HANDLE_VALUE != FileHandle)
        {
          BY_HANDLE_FILE_INFORMATION	FileInformation;
          BOOL	r = GetFileInformationByHandle(FileHandle, &FileInformation);
          if (r)
          {
            if (gLogLevel != FileInfoContainer::eLogQuiet)
              fwprintf(gStdOutFile, L"%s:%d\n", Argv1Path.ArgvOrg.c_str(), FileInformation.nNumberOfLinks);
          }
          CloseHandle(FileHandle);


          // If GenerateHardlinks was set abuse this option to create many hardlinks from one file
          // This is used during automated test
          wchar_t GeneratedHardlink[HUGE_PATH];

          for (int i = 0; i < gGenerateHardlinks; i++)
          {
            swprintf_s(GeneratedHardlink, HUGE_PATH, L"%s%04d", Argv1, i);
            CreateHardlink(Argv1, GeneratedHardlink);
          }

          Exit(ERR_SUCCESS);
        }
      }
      else
      {
        if (gLogLevel != FileInfoContainer::eLogQuiet)
          fwprintf(gStdOutFile, L"ERROR: symlink '%s' does not exist\n", Argv1Path.ArgvOrg.c_str());
        Exit(ERR_FILE_DOES_NOT_EXIST);
      }
    }
  }

  // SmartCopy
  if (SmartCopy)
  {
    int	dg = LnSmartXXX(
      SourceDirList,
      Argv2Path,
      Absolute,
      true,
      FileInfoContainer::eSmartCopy,
      &IncludeFileList,
      &IncludeDirList,
      &ExcludeFileList,
      &ExcludeDirList,
      AlwaysUnroll ? nullptr : &UnrollDirList,
      AlwaysSplice ? nullptr : &SpliceDirList,
      bSkipFiles,
      Traditional,
      Backup,
      KeepSymlinkRelation,
      TimeToleranceValue,
      HardlinkLimitValue
    );

    Exit(dg);
  }

  // Delorean Copy
  if (delorean)
  {
    if (INVALID_FILE_ATTRIBUTES == Argv2Path.FileAttribute)
      Exit(ERR_TARGET_DIR_DOES_NOT_EXIST);

    int	dg = Delorean(
      SourceDirList,
      Argv2Path,
      Argv3Path,
      Absolute,
      &IncludeFileList,
      &IncludeDirList,
      &ExcludeFileList,
      &ExcludeDirList,
      AlwaysUnroll ? nullptr : &UnrollDirList,
      AlwaysSplice ? nullptr : &SpliceDirList,
      bSkipFiles,
      Traditional,
      Backup,
      KeepSymlinkRelation,
      TimeToleranceValue,
      HardlinkLimitValue,
      g1023safe
    );
    Exit(dg);
  }

  // Delorean Merge
  if (DeloreanMerge)
  {
    if (DRIVE_REMOTE == Argv1Path.DriveType || DRIVE_REMOTE == Argv2Path.DriveType)
    {
      fwprintf(gStdOutFile, L"ERROR: Option --merge not supported on Remote Drives\n");
      Exit(ERR_OPERATION_NOT_SUPPORTED);
    }
  }

  // Mirror
  if (SmartMirror || DeloreanMerge)
  {
    int	dg = Mirror(
      SourceDirList,
      Argv2Path,
      Absolute,
      &IncludeFileList,
      &IncludeDirList,
      &ExcludeFileList,
      &ExcludeDirList,
      AlwaysUnroll ? nullptr : &UnrollDirList,
      AlwaysSplice ? nullptr : &SpliceDirList,
      bSkipFiles,
      Traditional,
      Backup,
      KeepSymlinkRelation,
      TimeToleranceValue,
      Symbolic,
      HardlinkMirror,
      DeloreanMerge,
      HardlinkLimitValue
    );

    Exit(dg);
  }

  // Enumerate
  if (enumerate)
  {
    int	dg = HardlinkEnum(&Argv1Path);
    if (dg)
    {
      fwprintf(gStdOutFile, L"%ld Hardlink Groups found.\n", dg);
      Exit(ERR_SUCCESS);
    }
    else
    {
      fwprintf(gStdOutFile, L"No Hardlink Groups found.\n");
      Exit(ERR_NO_HARDLINKGROUPS);
    }
  }

  // --list Hardlink sibblings
  if (list)
  {
    if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute)
    {
      int dg = HardlinkList(Argv1, Traditional);
      if (ERR_SUCCESS == dg)
        Exit(ERR_SUCCESS);
      else
        Exit(ERR_NO_HARDLINKGROUPS);
    }
    else
    {
      fwprintf(gStdOutFile, L"ERROR: '%s' not found\n", Argv1Path.ArgvOrg.c_str());
      Exit(ERR_SOURCE_DIR_DOES_NOT_EXIST);
    }
  }

  // Create/Show junctions
  if (junction)
  {
    int r;
    if (INVALID_FILE_ATTRIBUTES != Argv2Path.FileAttribute && Argv2[0] && Argv1Path.FileAttribute == INVALID_FILE_ATTRIBUTES)
    {
      r = DoJunction(Argv1, Argv1Path.ArgvOrg.c_str(), Argv2);
    }
    else
    {
      if (Argv2Path.FileAttribute == INVALID_FILE_ATTRIBUTES)
      {
        if (gLogLevel != FileInfoContainer::eLogQuiet)
        {
          fwprintf(gStdOutFile, L"ERROR: Source directory '%s' does not exist\n", Argv2Path.ArgvOrg.c_str());
        }
        Exit(ERR_SOURCE_DIR_DOES_NOT_EXIST);
      }
      else
      {
        // Do we have both attributes
        if (Argv1[0] && Argv2[0])
        {
          if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute)
          {
            if (gLogLevel != FileInfoContainer::eLogQuiet)
            {
              fwprintf(gStdOutFile, L"ERROR: '%s' already exists\n", Argv1Path.ArgvOrg.c_str());
            }
            Exit(ERR_FILE_ALREADY_EXISTS);
          }
        }
        else
        {
          // we have only one argument
          if (Argv1[0])
          {
            if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute)
            {
              // if it is there show the target of the junction
              ShowJunction(Argv1Path);
            }
            else
            {
              if (gLogLevel != FileInfoContainer::eLogQuiet)
              {
                fwprintf(gStdOutFile, L"ERROR: Source directory '%s' does not exist\n", Argv1Path.ArgvOrg.c_str());
              }
              Exit(ERR_SOURCE_DIR_DOES_NOT_EXIST);
            }
          }
        }
      }
    }
    Exit(ERR_SUCCESS);
  }

  // Create/Show junctions
  if (noitcnuj)
  {
    int r;
    if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute && INVALID_FILE_ATTRIBUTES == Argv2Path.FileAttribute)
    {
      r = DoJunction(Argv2, Argv2Path.ArgvOrg.c_str(), Argv1);
    }
    else
    {
      if (INVALID_FILE_ATTRIBUTES == Argv1Path.FileAttribute)
      {
        if (gLogLevel != FileInfoContainer::eLogQuiet)
        {
          fwprintf(gStdOutFile, L"ERROR: Source directory '%s' does not exist\n", Argv1Path.ArgvOrg.c_str());
        }
        Exit(ERR_SOURCE_DIR_DOES_NOT_EXIST);
      }
      else
      {
        // Do we have both attributes
        if (Argv1[0] && Argv2[0])
        {
          if (INVALID_FILE_ATTRIBUTES != Argv2Path.FileAttribute)
          {
            if (gLogLevel != FileInfoContainer::eLogQuiet)
            {
              fwprintf(gStdOutFile, L"ERROR: '%s' already exists\n", Argv2Path.ArgvOrg.c_str());
            }
            Exit(ERR_FILE_ALREADY_EXISTS);
          }
        }
        else
        {
          // we have only one argument
          if (Argv1[0])
          {
            if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute)
            {
              // if it is there show the target of the junction
              ShowJunction(Argv1Path);
            }
            else
            {
              if (gLogLevel != FileInfoContainer::eLogQuiet)
              {
                fwprintf(gStdOutFile, L"ERROR: Source directory '%s' does not exist\n", Argv1Path.ArgvOrg.c_str());
              }
              Exit(ERR_SOURCE_DIR_DOES_NOT_EXIST);
            }
          }
        }
      }
    }
    Exit(ERR_SUCCESS);
  }

  // --smartmove
  if (SmartMove)
  {
    int dg = 0;
    if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute)
    {
      // Check if just the case of a letter changed, then we don't have to do anything
      if (!_wcsicmp(Argv1, Argv2))
        Exit(ERR_SUCCESS);

      _PathNameStatusList PathNameStatusList;
      CopyStatistics	aStats;

      FileInfoContainer	FileList;
      FileList.SetOutputFile(gStdOutFile);

      GetLocalTime(&aStats.m_StartTime);

      _ArgvList MoveDestination;
      Argv1Path.ArgvDest = Argv2;
      MoveDestination.push_back(Argv1Path);
      int r = FileList.FindHardLink(MoveDestination, 0, &aStats, &PathNameStatusList, nullptr);

      if (!Absolute)
        FileList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

      dg = FileList.SmartMove(&aStats, &PathNameStatusList, nullptr);
      //        BOOL b = MoveFileEx(Argv1, Argv2,  MOVEFILE_DELAY_UNTIL_REBOOT);
      BOOL b = MoveFile(Argv1, Argv2);

      GetLocalTime(&aStats.m_EndTime);

      PrintDeloreanCopyStats(gStdOutFile, aStats, FileInfoContainer::eSmartCopy, gAutomatedTest, gJson);

      DeletePathNameStatusList(PathNameStatusList);

      if (b)
        Exit(ERR_SUCCESS);
      else
      {
        fwprintf(gStdOutFile, L"ERROR: Move failed '%s' -> '%s': %08x\n", Argv1, Argv2, GetLastError());
        Exit(ERR_ACCESS_DENIED);
      }
    }
    else
    {
      fwprintf(gStdOutFile, L"ERROR: '%s' not found\n", Argv1Path.ArgvOrg.c_str());
      Exit(ERR_SOURCE_DIR_DOES_NOT_EXIST);
    }
  }

  //
  // SmartRename option
  //
  if (SmartRename)
  {
    // Check if just the case of a letter changed, then we don't have to do anything
    if (!_wcsicmp(Argv1, Argv2))
      Exit(ERR_SUCCESS);

    _PathNameStatusList PathNameStatusList;
    CopyStatistics	aStats;

    FileInfoContainer	FileList;
    FileList.SetOutputFile(gStdOutFile);
    GetLocalTime(&aStats.m_StartTime);

    _ArgvList MoveDestination;
    Argv1Path.ArgvDest = Argv2;
    MoveDestination.push_back(Argv1Path);
    int r = FileList.FindHardLink(MoveDestination, 0, &aStats, &PathNameStatusList, nullptr);

    if (!Absolute)
      FileList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

    int dg = FileList.SmartMove(&aStats, &PathNameStatusList, nullptr);

    GetLocalTime(&aStats.m_EndTime);

    PrintDeloreanCopyStats(gStdOutFile, aStats, FileInfoContainer::eSmartCopy, gAutomatedTest, gJson);

    DeletePathNameStatusList(PathNameStatusList);

    Exit(ERR_SUCCESS);
  }

  //
  // --ProbeFs
  //
  if (ProbeFs)
  {
    if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute)
    {

      DWORD FileSystemFlags;
      if (PathIsUNC(Argv1Path.Argv.c_str()))
        Argv1Path.Argv += L"\\";

      WCHAR		FileSystemName[MAX_PATH];
      FileSystemName[0] = 0x00;
      GetVolumeInformation(
        Argv1Path.Argv.c_str(),
        nullptr, 0,
        nullptr,
        nullptr,
        &FileSystemFlags,
        FileSystemName,
        MAX_PATH
      );
      fwprintf(gStdOutFile, L"%s\n", FileSystemName);
      Exit(ERR_SUCCESS);
    }
    else
      Exit(ERR_FILE_DOES_NOT_EXIST);
  }

  //
  // --truesize
  //
  if (bTrueSize)
  {
    int returnCode = TrueSize(
      SourceDirList,
      &IncludeFileList,
      &IncludeDirList,
      &ExcludeFileList,
      &ExcludeDirList,
      bSkipFiles,
      false // Traditional
    );

    Exit(returnCode);
  }

  //
  // --delete
  //
  if (DeloreanDelete)
  {
    if (INVALID_FILE_ATTRIBUTES != Argv1Path.FileAttribute)
    {
      int	returnCode = LnSmartXXX(
        SourceDirList,
        Argv2Path,
        Absolute,
        true,
        FileInfoContainer::eSmartClean,
        &IncludeFileList,
        &IncludeDirList,
        &ExcludeFileList,
        &ExcludeDirList,
        AlwaysFollow ? nullptr : &FollowDirList,
        nullptr,
        false,      // bSkipfiles
        Traditional,
        Backup,
        KeepSymlinkRelation,
        TimeToleranceValue,
        HardlinkLimitValue
      );

      Exit(returnCode);
    }
  }


  //
  // AdsDev option
  //
  if (AdsDev)
  {
#if defined DEBUG_PREDICTION_REPLAY
    Effort MaxProgress;
    Effort Progress;

    ProgressPrediction progressPrediction;
    int Percentage = 0;
    progressPrediction.Open("D:\\data\\cpp\\hardlinks\\tmp\\progress_PumaOpen_second.log");
    progressPrediction.ReadSample(MaxProgress);
    progressPrediction.SetStart(MaxProgress);

    int UpdCnt = gUpdateIntervall;
    while (true)
    {
      Sleep(250);
      Effort fakeEffort;
      bool r = progressPrediction.ReadSample(fakeEffort);
      if (!r)
        break;
      int NewPercentage = progressPrediction.AddSample(fakeEffort);
      if (NewPercentage != Percentage)
      {
        Percentage = NewPercentage;
        PrintProgress(Percentage, progressPrediction);
      }
      if (!--UpdCnt)
      {
        UpdCnt = gUpdateIntervall;
        PrintProgress(Percentage, progressPrediction);
      }
    }
    PrintElapsed(progressPrediction);
#endif
    Exit(ERR_ERROR);
  }

  // for hardlinks check if source and destination are on same volume
  if (!Symbolic)
  {
    if (!CheckIfOnSameDrive(&Argv1[PATH_PARSE_SWITCHOFF_SIZE], &Argv2[PATH_PARSE_SWITCHOFF_SIZE])) // HHH
    {
      if (gLogLevel != FileInfoContainer::eLogQuiet)
      {
        fwprintf(gStdOutFile, L"ERROR: '%s' and '%s' are not on same volume\n", Argv1Path.ArgvOrg.c_str(), Argv2Path.ArgvOrg.c_str());
      }
      Exit(ERR_NOT_ON_SAME_VOLUME);
    }
  }

  // recursive
  if (recursive)
  {
    // Check first argument
    if (INVALID_FILE_ATTRIBUTES == Argv1Path.FileAttribute)
    {
      if (gLogLevel != FileInfoContainer::eLogQuiet)
      {
        fwprintf(gStdOutFile, L"ERROR: source directory '%s' does not exist\n", Argv1Path.ArgvOrg.c_str());
      }
      RetVal = ERR_SOURCE_DIR_DOES_NOT_EXIST;
    }
    else
    {
      if (!(Argv1Path.FileAttribute & FILE_ATTRIBUTE_DIRECTORY))
      {
        if (gLogLevel != FileInfoContainer::eLogQuiet)
        {
          fwprintf(gStdOutFile, L"ERROR: '%s' is not a directory\n", Argv1Path.ArgvOrg.c_str());
        }
        RetVal = ERR_ARG_IS_NOT_A_DIRECTORY;
      }
    }

    // Check second argument
    if ((INVALID_FILE_ATTRIBUTES == Argv2Path.FileAttribute) && (0 == RetVal))
    {
      CreateDirectoryHierarchy(Argv2, wcslen(Argv2));
      if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(Argv2))
      {
        if (gLogLevel != FileInfoContainer::eLogQuiet)
        {
          fwprintf(gStdOutFile, L"ERROR: failed to create '%s'\n", Argv2Path.ArgvOrg.c_str());
        }
        RetVal = ERR_FAILED_TO_CREATE_DIR;
      }
    }
    else
    {
      if (
        !(Argv2Path.FileAttribute & FILE_ATTRIBUTE_DIRECTORY) &&
        (Argv2Path.FileAttribute & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE))
        )
      {
        if (gLogLevel != FileInfoContainer::eLogQuiet)
        {
          fwprintf(gStdOutFile, L"ERROR: file '%s' already exists\n", Argv2Path.ArgvOrg.c_str());
        }
        RetVal = ERR_FILE_ALREADY_EXISTS;
      }
    }
    if (0 == RetVal)
    {
      RemoveDirectory(Argv2Path.Argv.c_str());

      int RetVal = LnSmartXXX(
        SourceDirList,
        Argv2Path,
        Absolute,
        Symbolic,
        FileInfoContainer::eSmartClone,
        &IncludeFileList,
        &IncludeDirList,
        &ExcludeFileList,
        &ExcludeDirList,
        AlwaysUnroll ? nullptr : &UnrollDirList,
        AlwaysSplice ? nullptr : &SpliceDirList,
        bSkipFiles,
        Traditional,
        Backup,
        KeepSymlinkRelation,
        TimeToleranceValue,
        HardlinkLimitValue
      );
      Exit(RetVal);
    }
  }
  else 
  // this is not recursive
  {
    // Check first argument
    if (INVALID_FILE_ATTRIBUTES == Argv1Path.FileAttribute)
    {
      if (gLogLevel != FileInfoContainer::eLogQuiet)
      {
        fwprintf(gStdOutFile, L"ERROR: file '%s' does not exist\n", Argv1Path.ArgvOrg.c_str());
      }
      RetVal = ERR_FILE_DOES_NOT_EXIST;
    }

    // Check second argument if the file to be linked is already there
    if (INVALID_FILE_ATTRIBUTES == Argv2Path.FileAttribute && ERROR_SUCCESS == RetVal && GetLastError() == ERROR_PATH_NOT_FOUND)
    {
      // The path to the destination file did not exist
      wchar_t* filename = PathFindFileName(Argv2);
      *--filename = 0x00;
      CreateDirectoryHierarchy(Argv2, wcslen(Argv2));
      *filename = '\\';
    }

    // lets create the link if everything was fine up till now
    if (ERROR_SUCCESS == RetVal)
    {
      int result = ERROR_SUCCESS;
      if (Symbolic)
      {
        // --symbolic
        //
        int SymlinkFlag = Absolute ? 0 : SYMLINK_FLAG_RELATIVE;

        // If the first argument is a directory we assume two directories get linked
        if (Argv1Path.FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
          SymlinkFlag |= SYMLINK_FLAG_DIRECTORY;

        // If the first argument is a file and the second is an existing directory, we concatenate the filename to that directory
        if (
          //          (Argv1Path.FileAttribute & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) &&
          (Argv2Path.FileAttribute != INVALID_FILE_ATTRIBUTES) &&
          (Argv2Path.FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
          )
        {
          size_t lenArgv2 = Argv2Path.ArgvOrg.length();
          if (
            // If Argv2 contains a traling slash ( \ or / ) and Argv1 is a directory 
            (lenArgv2 && (Argv2Path.ArgvOrg[lenArgv2 - 1] == '\\' || Argv2Path.ArgvOrg[lenArgv2 - 1] == '/') && (Argv1Path.FileAttribute & FILE_ATTRIBUTE_DIRECTORY)) ||
            // or Argv1 is a normal file ...
            (Argv1Path.FileAttribute & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL))
            )
          {
            // ... do the 'into linking' stuff
            wchar_t* filename = PathFindFileName(Argv1);
            if (filename != Argv1)
            {
              wcscat_s(Argv2, HUGE_PATH, L"\\");
              wcscat_s(Argv2, HUGE_PATH, filename);

              if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(Argv2))
              {
                // Prepare filename for error message
                if (Argv1Path.FileAttribute & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL))
                  Argv2Path.ArgvOrg += L"\\";
                Argv2Path.ArgvOrg += filename;
                result = ERROR_ALREADY_EXISTS;
              }
            }
          }
        }
        if (ERROR_SUCCESS == result)
        {
          if (SymLinkAllowUnprivilegedCreation(&gVersionInfo))
            SymlinkFlag |= SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

          result = CreateSymboliclink(Argv2, Argv1, SymlinkFlag);
        }
      }
      else
      {
        // Create a Hardlink
        // If the first argument is a file and the second is an existing directory, we concatenate the filename to that directory
        if (
          (Argv2Path.FileAttribute != INVALID_FILE_ATTRIBUTES) &&
          (Argv2Path.FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
          )
        {
          wchar_t* filename = PathFindFileName(Argv1);
          if (filename != Argv1)
          {
            wcscat_s(Argv2, HUGE_PATH, L"\\");
            wcscat_s(Argv2, HUGE_PATH, filename);

            if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(Argv2))
            {
              // Prepare filename for error message
              Argv2Path.ArgvOrg += L"\\";
              Argv2Path.ArgvOrg += filename;
              result = ERROR_ALREADY_EXISTS;
            }
          }
        }

        if (ERROR_SUCCESS == result)
          result = CreateHardlink(Argv1, Argv2);
      }

      switch (result)
      {
      case ERROR_SUCCESS:
        if (gLogLevel != FileInfoContainer::eLogQuiet)
        {
          fwprintf(gStdOutFile, L"%s\n", Argv1Path.ArgvOrg.c_str());
        }
        break;

      case ERROR_ALREADY_EXISTS:
      {
        if (gLogLevel != FileInfoContainer::eLogQuiet)
        {
          fwprintf(gStdOutFile, L"ERROR: '%s' already exists\n", Argv2Path.ArgvOrg.c_str());
        }
        RetVal = ERR_FILE_ALREADY_EXISTS;
      }
      break;

      case ERROR_ACCESS_DENIED:
      {
        if (gLogLevel != FileInfoContainer::eLogQuiet && Argv2Path.ArgvOrg.size())
        {
          fwprintf(gStdOutFile, L"ERROR: '%s' permission denied\n", Argv2Path.ArgvOrg.c_str());
        }
        RetVal = ERR_ACCESS_DENIED;
      }
      break;

      default:
        if (gLogLevel != FileInfoContainer::eLogQuiet && Argv2Path.ArgvOrg.size())
        {
          fwprintf (gStdOutFile, L"ERROR: failed to create '%s', (%08x)\n", Argv2Path.ArgvOrg.c_str(), result);
        }
        RetVal = ERR_CREATE_HARDLINK_FAILED;
        break;
      }
    }

    if (!gLogLevel && 0 == RetVal)
    {
      if (Argv1Path.FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
        fwprintf(gStdOutFile, L"1 directory linked\n");
      else
        fwprintf(gStdOutFile, L"1 file linked\n");
    }
  }


  // Delete ANSI options
  for (i = 0; i < argc + 1; i++)
    delete[] a_argv[i];
  delete[] a_argv;

  Exit(RetVal);
}


#ifdef __cplusplus
}
#endif // __cplusplus


