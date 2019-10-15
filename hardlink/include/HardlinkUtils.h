/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

class WaitQueue 
{
  protected:
    enum 
    {
      MaxHandleAlloc = MAXIMUM_WAIT_OBJECTS,
    };

  public:
    WaitQueue(int a_Max);
    void Insert(HANDLE a_Handle);
    void WaitSingle();
    void WaitMultiple();

protected:
    DWORD       m_Count;
    DWORD       m_MaxHandles;
    HANDLE    m_Handles[MaxHandleAlloc + 1];

  public:
    ~WaitQueue() 
    {
    };

};

// https://www.statisticshowto.datasciencecentral.com/probability-and-statistics/regression-analysis/find-a-linear-regression-equation/
// TODO: calculate the regression based prediction
class ProgressPrediction
{
private:
    const int           cMaxSamples = 5;
    const int           cAccuracy = 18;

    typedef pair<FILETIME64, Effort> _ProgressSample;
    typedef list<_ProgressSample>	_ProgressSamples;
  
    _ProgressSamples m_Values;
    _ProgressSample  m_Start;

    __int64 m_Increment;

public:
  ProgressPrediction();
  ~ProgressPrediction();

  int AddSample(const Effort& aProgress, const Effort* apProgressOffset = 0);
  void SetStart(const Effort& aMaxProgress);
  void Duration(SYSTEMTIME& aDuration, Effort& a_Effort);
  bool TimeLeft(SYSTEMTIME& a_TimeStamp, Effort& a_Effort);
};

class use_WSA {
  WSADATA d;
  WORD ver;
public:
  use_WSA() : ver(MAKEWORD(2, 2)) {
    if ((WSAStartup(ver, &d) != 0) || (ver != d.wVersion))
      throw(std::runtime_error("Error starting Winsock"));
  }
  ~use_WSA() { WSACleanup(); }
};


char* FormatNumber(
  char* aResult,
  int aLength, 
  ULONG64 aNumber
);

char* FormatG(
  char* aResult,
  int aLength,
  ULONG64 aNumber
);

int PrintTrueSizeCopyStats(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest,
  bool                                      a_Json
);

int PrintTrueSizeCopyStatsNormal(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest
);

int PrintTrueSizeCopyStatsJson(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest
);

int PrintDeloreanCopyStats(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest,
  bool                                      a_Json
);

int PrintDeloreanCopyStatsNormal(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest
);

int PrintDeloreanCopyStatsJson(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest
);

int PrintDupeMergeCopyStats(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  bool                                      a_ListOnly,
  bool                                      a_AutomatedTest
);

void PrintInternalCounters(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats
);

int AnalysePathNameStatus(
  FILE*                 a_OutputFile,
  _PathNameStatusList&  a_PathNameStatusList, 
  bool                  a_Verbose,
  int                   a_LogLevel, 
  bool                  gJson,
  bool*                 a_JsonWriterState
);

int AnalysePathNameStatusNormal(
  FILE*                 a_OutputFile,
  _PathNameStatusList&  a_PathNameStatusList, 
  bool                  a_Verbose,
  int                   a_LogLevel
);

int AnalysePathNameStatusJson(
  FILE*                 a_OutputFile,
  _PathNameStatusList&  a_PathNameStatusList, 
  bool                  a_Verbose,
  int                   a_LogLevel,
  bool*                 a_JsonWriterState
);

bool 
ElevationNeeded(
);

bool 
SymLinkAllowUnprivilegedCreation(
  __inout LPOSVERSIONINFO lpVersionInfo
);

int
SetAttributes (
  __in     HANDLE         lpExistingFileHandle,
  __in     DWORD          dwFileAttributes
);

int
SetFileBasicInformation (
  __in     HANDLE         lpExistingFileHandle,
  __in     DWORD          dwFileAttributes,
  __in     PFILETIME      ftCreationTime,
  __in     PFILETIME      ftLastAccessTime,
  __in     PFILETIME      ftLastWriteTime
);

//
// Get timestamps and attributes in one call
//
int
GetFileBasicInformation (
  __in       HANDLE       lpExistingFileHandle,
  __inout    PDWORD       dwFileAttributes,
  __inout    PFILETIME    ftCreationTime,
  __inout    PFILETIME    ftLastAccessTime,
  __inout    PFILETIME    ftLastWriteTime
);

int
GetFileSizeEx2 (
  __in     HANDLE         lpExistingFileHandle,
  __out    PLARGE_INTEGER lpFileSize
);

int
CopyCompression(
  __in     LPCWSTR lpExistingFileName,
  __in     LPCWSTR lpNewFileName,
  __in     DWORD dwExistingAttributes,
  __in     DWORD dwNewAttributes
);

int
GetEaRecords( 
  __in    HANDLE                      a_SrcFileHandle,
  __inout PFILE_FULL_EA_INFORMATION*  a_pFileEaInfo,
  __inout LPDWORD                     a_dwFileEaInfoSize
);

int
SetEaRecords(
  __in    HANDLE                      a_DstFileHandle,
  __in    PFILE_FULL_EA_INFORMATION   a_pFileEaInfo,
  __in    DWORD                       a_dwFileEaInfoSize
);

int
DumpEaRecords(
  __in    PFILE_FULL_EA_INFORMATION   a_pFileEaInfo
);

PFILE_FULL_EA_INFORMATION
AddEaRecords(
  __in    PFILE_FULL_EA_INFORMATION   a_pFileEaInfo,
  __in    char*                       a_EaRecordName,
  __in    LPVOID                      a_pPayLoad,
  __in    USHORT                      a_dwPayLoadSize
);

int
SaveEaRecords(
  __in    PFILE_FULL_EA_INFORMATION   a_pFileEaInfo
);

int
CopyEaRecords(
  __in     HANDLE                 a_ExistingFileHandle,
  __in     HANDLE                 a_NewFileHandle
);

int
GetSecurityAttributesByName( 
  __in    PCWSTR                aSrcName,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize,
  __in    const int             aFlagsAndAttributes
);

int
SetSecurityAttributesByName( 
  __in    PCWSTR                aDstName,
  __in    PSECURITY_DESCRIPTOR  a_pSecDesc,
  __in    const int             aFlagsAndAttributes
);

int
CopySecurityAttributes( 
  __in    HANDLE                a_SrcHandle,
  __in    HANDLE                a_DstHandle,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize
);

int
GetSecurityAttributes( 
  __in    HANDLE                a_SrcHandle,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize
);

int
SetSecurityAttributes( 
  __in    HANDLE                a_DstHandle,
  __in    PSECURITY_DESCRIPTOR  a_pSecDesc
);

int
CopySecurityAttributesByName( 
  __in    PCWSTR                aSrcName,
  __in    PCWSTR                aDstName,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize,
  __in    const int             aFlagsAndAttributes
);


int
CopyFileEx3( 
  __in     LPCWSTR              lpExistingFileName,
  __in     LPCWSTR              lpNewFileName,
  __in_opt LPPROGRESS_ROUTINE   lpProgressRoutine,
  __in_opt LPVOID               lpData,
  __in_opt LPBOOL               pbCancel,
  __in     DWORD                dwCopyFlags,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize,
  __in     __int64              a_DateTimeTolerance,
  __in     _PathNameStatusList* a_PathNameStatusList
);

struct _ReadWriteHandShake
{
  PVOID               ReadContext;
  ULONG               BytesWritten;
  PBYTE               ReadData;
  ULONG               ReadLength;
  HANDLE              DataReceived;
  HANDLE              DataAvailable;

  LPPROGRESS_ROUTINE  lpProgressRoutine;
  LPVOID              lpData;
  LARGE_INTEGER       TotalFileSize;

  LARGE_INTEGER       TotalBytesTransferred;
};

DWORD WINAPI WriteEncryptedStreamCallBack(
  PBYTE   a_WriteData, 
  PVOID   pvCallbackContext, 
  PULONG  a_WriteLength
);

DWORD WINAPI ReadEncryptedStreamCallBack(
  PBYTE a_ReadData, 
  PVOID pvCallbackContext, 
  ULONG a_ReadLength
);

int
CopyEncryptedStream( 
  __in     LPCWSTR            lpExistingFileName,
  __in     LPCWSTR            lpNewFileName,
  __in_opt LPPROGRESS_ROUTINE lpProgressRoutine,
  __in_opt LPVOID             lpData,
  __in_opt LPBOOL             pbCancel,
  __in     LARGE_INTEGER      a_TotalFileSize,
  __inout  LARGE_INTEGER&     a_TotalBytesTransferred,
  __in     DWORD              dwCopyFlags
);

int
CopyFileStream( 
  __in     HANDLE             a_ExistingFileHandle,
  __in     HANDLE             a_NewFileHandle,
  __in_opt LPPROGRESS_ROUTINE lpProgressRoutine,
  __in_opt LPVOID             lpData,
  __in_opt LPBOOL             pbCancel,
  __in     LARGE_INTEGER      a_TotalFileSize,
  __inout  LARGE_INTEGER&     a_TotalBytesTransferred,
  __in     DWORD              a_StreamNumber
);

int
CopyFileStreamRange( 
  __in     HANDLE             a_ExistingFileHandle,
  __in     HANDLE             a_NewFileHandle,
  __in_opt LPPROGRESS_ROUTINE lpProgressRoutine,
  __in_opt LPVOID             lpData,
  __in_opt LPBOOL             pbCancel,
  __in     LARGE_INTEGER      a_TotalFileSize,
  __inout  LARGE_INTEGER&     a_TotalBytesTransferred,
  __in     DWORD              a_StreamNumber,
  __in     LARGE_INTEGER      a_StreamOffset,
  __in     LARGE_INTEGER      a_StreamSize,
  __inout  LARGE_INTEGER&     a_StreamBytesTransferred,
  __in     LARGE_INTEGER      a_StreamTotalSize
);

int
CopyAlternativeStreams(
  __in     LPCWSTR              a_ExistingFileName,
  __in     HANDLE               a_ExistingFileHandle,
  __in     LPCWSTR              a_NewFileName,
  __in_opt LPPROGRESS_ROUTINE   lpProgressRoutine,
  __in_opt LPVOID               lpData,
  __in_opt LPBOOL               pbCancel,
  __in     LARGE_INTEGER        a_TotalFileSize,
  __inout  LARGE_INTEGER&       a_TotalBytesTransferred,
  __in     DWORD                a_StreamStart,
  __in     _PathNameStatusList* a_PathNameStatusList
);

BOOL
CopyDirectory(
  __in      LPCWSTR                a_ExistingFileName,
  __in      LPCWSTR                a_NewFileName,
  __in_opt  LPPROGRESS_ROUTINE     lpProgressRoutine,
  __in_opt  LPVOID                 lpData,
  __in      DWORD                  dwCopyFlags,
  __inout   PSECURITY_DESCRIPTOR*  a_pSecDesc,
  __inout   int*                   a_SecDescSize,
  __in      _PathNameStatusList*   a_PathNameStatusList
);

int 
CreateDirectoryHierarchy(
	__in     LPWSTR	              aPath,
	__in     size_t		            len
);

int 
CopyDirectoryHierarchy(
	__in     LPWSTR	              a_ExistingFileName,
	__in     LPWSTR	              a_NewFileName,
  __in     size_t		            a_NewFileNameLength,
  __in     DWORD                dwCopyFlags,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize
);

int
EnumerateVolumes(
  __inout   _StringList& aVolumeList
);

int 
MemCpy(
	__in     PVOID	              a_Dst,
	__in     PVOID	              a_Src,
	__in     size_t	              a_Size
);

bool
ContainsRemote(
  __inout _ArgvList & a_PathList
);

DWORD
GetFileNotFoundCacheLifetime(
);

void
FindGlobalRootFromPath(
  wchar_t*    aFullPath
);

bool GetShareNameFromUNC(
	const wchar_t* aPath,
	wchar_t* aShare
);

BOOL
DeleteSibling(
  __in     const wchar_t*    a_SrcPath,
  __in     const DWORD       a_FileAttribute
);

void
DbgOsPrint(
  wchar_t*    aTag
);

int 
ReadArgsFromFile(
  wchar_t*      aArgFileName,
  _StringList&  aArgumentList
);

void
WildCard2RegExp(
  ::wstring&  aString
);

bool 
IsDeveloperModeEnabled();

wchar_t*
ResolveUNCPath(
  wchar_t* aUNCPath,
  wchar_t* aResolvedUNCPath
);

void
MakeAnsiString(
  const wchar_t*  unistring,
  char*           ansistring
);

void
stringreplace(
  wstring& aThis,
  wstring& src,
  wstring& dest
);

// MapNtStatusToWinError
DWORD
MapNtStatusToWinError(
  NTSTATUS        aNtStatus
);

// Token privilege calls
enum PrivilegeModification_t
{
  eClearPrivilege,
  eSetPrivilege,
  eProbePrivilege
};

// EnableTokenPrivilege
BOOL
EnableTokenPrivilege(
  __in LPCWSTR	PrivilegeName
);

BOOL
ProbeTokenPrivilege(
  __in LPCWSTR PrivilegeName
);
