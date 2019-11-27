/*
	Copyright (C) 1999 - 2019, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"

#include "hardlink_types.h"
#include "LSESettings.h"
#include "DbgHelpers.h"


#include "MmfObject.h"
#include "AsyncContext.h"
#include "hardlinks.h"

#include "moduleversion.h"

#include "HardlinkUtils.h"
#include "NtBase.h"

extern FILE* gStdOutFile;

WaitQueue::
WaitQueue(int a_Max) : 
    m_Count(0),
    m_MaxHandles(0)
{
  if (a_Max < MAXIMUM_WAIT_OBJECTS)
    m_MaxHandles = a_Max;
  else
    m_MaxHandles = MAXIMUM_WAIT_OBJECTS;
  
  for (DWORD i = 0; i < m_MaxHandles; ++i)
    m_Handles[i] = NULL;

  m_Handles[m_MaxHandles] = (HANDLE)0xCAFEBABE;
};

void 
WaitQueue::
Insert(HANDLE a_Handle)
{
  // If we are full wait until a slot gets free
  if (m_MaxHandles == m_Count)
    WaitSingle();

  m_Handles[m_Count++] = a_Handle;
};

void 
WaitQueue::
WaitSingle()
{
  DWORD dwWait = WaitForMultipleObjects(m_Count, m_Handles, FALSE, INFINITE);
  if (WAIT_OBJECT_0 + dwWait < m_Count)
  {
    CloseHandle(m_Handles[dwWait]);
    
    // compact the table
    for (DWORD d = dwWait; d < m_Count; ++d)
      m_Handles[d] = m_Handles[d+1];

    m_Count--;
  }
};

void 
WaitQueue::
WaitMultiple()
{
    DWORD dwWait = WaitForMultipleObjects(m_Count, m_Handles, TRUE, INFINITE);
    for (DWORD d = 0; d < m_Count; ++d)
      CloseHandle(m_Handles[d]);
};

// ProgressPrediction
//
ProgressPrediction::ProgressPrediction() : m_Increment{ 0 }
{
}

ProgressPrediction::~ProgressPrediction()
{
}

// ProgressPrediction
//
void
ProgressPrediction::
SetStart(const Effort& aMaxProgress) 
{ 
  SYSTEMTIME  ct;
  GetLocalTime(&ct);

  SystemTimeToFileTime(&ct, &m_Start.first.FileTime);

  // Furthermore reduce accuracy to 1/4 of a second
  m_Start.first.ul64DateTime >>= cAccuracy;
  m_Start.second = aMaxProgress;

  m_Increment = aMaxProgress.m_Points.load() / 100;

};

void
ProgressPrediction::
Duration(SYSTEMTIME& aDuration, Effort& a_Effort)
{
  SYSTEMTIME  currentTime;
  GetLocalTime(&currentTime);

  FILETIME64  currentFileTime;
  SystemTimeToFileTime(&currentTime, &currentFileTime.FileTime);

  FILETIME64 duration;
  duration.ul64DateTime = currentFileTime.ul64DateTime - (m_Start.first.ul64DateTime << cAccuracy);

  FileTimeToSystemTime(&duration.FileTime, &aDuration);
  a_Effort = m_Start.second;
}


int
ProgressPrediction::
AddSample(
  const Effort&   aProgress, 
  const Effort*   apProgressOffset
)
{
  if (m_Values.size() >= cMaxSamples)
    m_Values.pop_front();

  // Invert the progress, because this works by counting down
  _ProgressSample sample;
  sample.second = m_Start.second - aProgress;

  // Retrieve time and convert to Filetime
  SYSTEMTIME    ct;
  GetLocalTime(&ct);
  FILETIME64    CurrentTime;
  SystemTimeToFileTime(&ct, &CurrentTime.FileTime);

  // Also offset the time to m_Start and To stay in the range of __int64 in belows calculation, we shift 
  // by 18bit which is a factor of 262144 and brings us to an accuracy of ~1/4 a second because FILETIME is
  // in 100ns
  sample.first.ul64DateTime = CurrentTime.ul64DateTime >> cAccuracy;
  sample.first.ul64DateTime -= m_Start.first.ul64DateTime;

  m_Values.push_back(sample);

  __int64 currentPoints = aProgress.m_Points;
  if (apProgressOffset)
    currentPoints += apProgressOffset->m_Points;

  return (int)(m_Increment ? currentPoints / m_Increment : 100);
}

// Calculates the progress and returns and estimate how long progress will last.
// returns false if prediction is not possible due to unknown endpoint
bool
ProgressPrediction::
TimeLeft(SYSTEMTIME& a_TimeLeft, Effort& a_Effort)
{
// #define NEW_PROGRESS_PREDICTION 1
#if defined NEW_PROGRESS_PREDICTION
  __int64 SumX = 0;
  __int64 SumY = m_Start.second.m_Points;
  __int64 SumXX = 0; 
  __int64 SumXY = 0; 
  for (auto sample : m_Values)
  {
    SumX += sample.first.ul64DateTime;
    SumY += sample.second.m_Points;
    SumXX += sample.first.ul64DateTime * sample.first.ul64DateTime;
    SumXY += sample.first.ul64DateTime * sample.second.m_Points;
  }

  // This is the magic math 
  FILETIME64 EndTime;
  EndTime.l64DateTime = - (SumY * SumXX - SumX * SumXY) / (m_Values.size() * SumXY - SumX * SumY ) + m_Start.first.ul64DateTime;
//  EndTime.l64DateTime =  (SumY * SumXX - SumX * SumXY) / (SumX * SumY - m_Values.size() * SumXY ) + m_Start.first.ul64DateTime;
  
  // Shift back in the range of 100ns
  EndTime.ul64DateTime <<= cAccuracy;

  // Get current time
  SYSTEMTIME    CurrentTime;
  GetLocalTime(&CurrentTime);
  FILETIME64    CurrentFileTime;
  SystemTimeToFileTime(&CurrentTime, &CurrentFileTime.FileTime);

  FILETIME64 TimeLeft;
  TimeLeft.ul64DateTime = EndTime.ul64DateTime - CurrentFileTime.ul64DateTime;
//  TimeLeft.ul64DateTime = CurrentFileTime.ul64DateTime - EndTime.ul64DateTime;
  FileTimeToSystemTime(&TimeLeft.FileTime, &a_TimeLeft);

  return true;
#else
  SYSTEMTIME    CurrentTime;
  GetLocalTime(&CurrentTime);
  FILETIME64    CurrentFileTime;
  SystemTimeToFileTime(&CurrentTime, &CurrentFileTime.FileTime);

  CurrentFileTime.ul64DateTime >>= cAccuracy;
  __int64 dX = CurrentFileTime.ul64DateTime - m_Start.first.ul64DateTime;
  __int64 dY = m_Start.second.m_Points - m_Values.back().second.m_Points;
  __int64 k;
  
  // If things are very fast, we might enter this in no time, thus ....
  if (dX == 0)
    k = 0;
  else
    k = dY / dX;

  FILETIME64 EndTime;
  // Check if prediction is unsure
  if (k > 0)
  {
    EndTime.l64DateTime = m_Start.second.m_Points.load() / k;

    FILETIME64 TimeLeft;
    TimeLeft.ul64DateTime = m_Start.first.ul64DateTime + EndTime.ul64DateTime - CurrentFileTime.ul64DateTime;
    // Shift back in the range of 100ns
    TimeLeft.ul64DateTime <<= cAccuracy;

    FileTimeToSystemTime(&TimeLeft.FileTime, &a_TimeLeft);
    a_Effort = m_Values.back().second;
    return true;
  }
  else
    return false;
#endif
}

char* FormatNumber(char* aResult, int aLength, ULONG64 aNumber)
{
  locale::global(locale(""));

  // Make a stringstreams for output
  //
  ostringstream oss;

  oss << aNumber;
  strcpy_s(aResult, aLength, oss.str().c_str());
  return aResult;
}

char* FormatG(char* aResult,int aLength, ULONG64 aNumber)
{
  locale::global(locale(""));

  //
	// Make a stringstreams for output
	//
	ostringstream oss;

  if (aNumber < 1024 * 1024)
  {
    oss << aNumber;
    strcpy_s(aResult, aLength, oss.str().c_str());
  }
  else
  {
    aNumber /= 1024;
    if (aNumber < 1024)
    {
      oss << aNumber << " k";
      strcpy_s(aResult, aLength, oss.str().c_str());
    }
    else
    {
      aNumber /= 1024;
      if (aNumber < 1024)
      {
        oss << aNumber << " m";
        strcpy_s(aResult, aLength, oss.str().c_str());
      }
      else
      {
        aNumber /= 1024;
        if (aNumber < 1024)
        {
          oss << aNumber * 1024 << " m";
          strcpy_s(aResult, aLength, oss.str().c_str());
        }
        else
        {
          oss << aNumber << " g";
          strcpy_s(aResult, aLength, oss.str().c_str());
        }
      }
    }
  }

  return aResult;
}


int PrintTrueSizeCopyStats(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest,
  bool                                      a_Json
)
{
  if (a_Json)
    return PrintTrueSizeCopyStatsJson(a_OutputFile, aStats, aFlags, a_AutomatedTest);
  else
    return PrintTrueSizeCopyStatsNormal(a_OutputFile, aStats, aFlags, a_AutomatedTest);
}


int PrintTrueSizeCopyStatsNormal(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest
)
{
	//
	// Make some locales
	//

	//
	// Make a stringstreams for output
	//
	locale localeDeu( "" );
	ostringstream oss;
	oss.imbue( localeDeu );

	oss << endl;

  oss << "                        Total               Bytes" << endl; 

  oss << "    File:";
  oss << setfill(' ') << setw(20) << aStats.m_FilesTotal;
  oss << setfill(' ') << setw(20) << aStats.m_BytesTotal;
	oss << endl;

  oss << "Hardlink:";
  oss << setfill(' ') << setw(20) << aStats.m_FilesTotal - aStats.m_HardlinksTotal;
  oss << setfill(' ') << setw(20) << aStats.m_BytesTotal - aStats.m_HardlinksTotalBytes;
  oss << endl;

  oss << "   Total:";
  oss << setfill(' ') << setw(20) << aStats.m_FilesTotal - (aStats.m_FilesTotal - aStats.m_HardlinksTotal);
  oss << setfill(' ') << setw(20) << aStats.m_BytesTotal - (aStats.m_BytesTotal - aStats.m_HardlinksTotalBytes);
  oss << endl;

  oss << "  Folder:";
  oss << setfill(' ') << setw(20) << aStats.m_DirectoryTotal;
  oss << setfill(' ') << setw(20) << "-";
	oss << endl;

  oss << "Junction:";
  oss << setfill(' ') << setw(20) << aStats.m_JunctionsTotal;
  oss << setfill(' ') << setw(20) << "-";
	oss << endl;

  oss << " Symlink:";
  oss << setfill(' ') << setw(20) << aStats.m_SymlinksTotal;
  oss << setfill(' ') << setw(20) << "-";
  oss << endl;
  oss << endl;
    
  if (!a_AutomatedTest)
  {
    // Time calculations
    FILETIME64 StartTime, EndTime;

    SystemTimeToFileTime(&aStats.m_StartTime, &StartTime.FileTime);
    SystemTimeToFileTime(&aStats.m_EndTime, &EndTime.FileTime);

    FILETIME64 TotalDuration;
    TotalDuration.ul64DateTime = EndTime.ul64DateTime - StartTime.ul64DateTime;

    SYSTEMTIME stTotalDuration;
    FileTimeToSystemTime(&TotalDuration.FileTime, &stTotalDuration);
    
    const int TotalDurationStrLength = 64;
    char TotalDurationStr[TotalDurationStrLength];

    // The NT date starts at 1.1.1601
    if (stTotalDuration.wDay != 1)
    {
      sprintf_s (TotalDurationStr, TotalDurationStrLength, "%dd %02d:%02d:%02d.%03d",
        stTotalDuration.wDay - 1,
        stTotalDuration.wHour,
        stTotalDuration.wMinute,
        stTotalDuration.wSecond,
        stTotalDuration.wMilliseconds
      );
    }
    else
    {
      sprintf_s (TotalDurationStr, TotalDurationStrLength, "%02d:%02d:%02d.%03d",
        stTotalDuration.wHour,
        stTotalDuration.wMinute,
        stTotalDuration.wSecond,
        stTotalDuration.wMilliseconds
      );
    }


    oss << "                      Overall" << endl;

    oss << "   Times:";
    oss << setfill(' ') << setw(20) << TotalDurationStr;
    oss << endl;
  }

  fwprintf(a_OutputFile, L"%S", oss.str().c_str());
 
  return ERROR_SUCCESS;
}

int PrintTrueSizeCopyStatsJson(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest
)
{
  fwprintf (a_OutputFile, L"{\n\"Summary\":{\n");

  fwprintf (a_OutputFile, L"\"Total\":{\"File\":%I64d,\"Hardlink\":%I64d,\"Total\":%I64d,\"Folder\":%I64d,\"Junction\":%I64d,\"Symlink\":%I64d},\n",
    aStats.m_FilesTotal,
    aStats.m_FilesTotal - aStats.m_HardlinksTotal,
    aStats.m_FilesTotal - (aStats.m_FilesTotal - aStats.m_HardlinksTotal),
    aStats.m_DirectoryTotal,
    aStats.m_JunctionsTotal,
    aStats.m_SymlinksTotal
  );

  fwprintf (a_OutputFile, L"\"Bytes\":{\"File\":%I64d,\"Hardlink\":%I64d,\"Total\":%I64d,\"Folder\":\"-1\",\"Junction\":\"-1\",\"Symlink\":\"-1\"}}",
    aStats.m_BytesTotal,
    aStats.m_BytesTotal - aStats.m_HardlinksTotalBytes,
    aStats.m_BytesTotal - (aStats.m_BytesTotal - aStats.m_HardlinksTotalBytes)
  );

  // Time calculations
  FILETIME64 StartTime, EndTime;
  SystemTimeToFileTime(&aStats.m_StartTime, &StartTime.FileTime);
  SystemTimeToFileTime(&aStats.m_EndTime, &EndTime.FileTime);

  FILETIME64 TotalDuration;
  if (a_AutomatedTest)
    TotalDuration.ul64DateTime = 191267;
  else
    TotalDuration.ul64DateTime = EndTime.ul64DateTime - StartTime.ul64DateTime;

  fwprintf (a_OutputFile, L",\n\"Times\":{\n");
  fwprintf (a_OutputFile, L"\"Overall\":%I64d}\n}\n", TotalDuration.ul64DateTime);

  return ERROR_SUCCESS;
}

int PrintDeloreanCopyStats(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest,
  bool                                      a_Json
)
{
  if (a_Json)
    return PrintDeloreanCopyStatsJson(a_OutputFile, aStats, aFlags, a_AutomatedTest);
  else
    return PrintDeloreanCopyStatsNormal(a_OutputFile, aStats, aFlags, a_AutomatedTest);
}

int PrintDeloreanCopyStatsNormal(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest
)
{
	//
	// Make some locales
	//

	//
	// Make a stringstreams for output
	//
	ostringstream oss;

  char  tmpstr[MAX_PATH];


//	oss.imbue( localeDeu );
	oss << endl;

  oss << "              Total    Copied    Linked   Skipped";
  if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror )
    oss << "   Removed";
  oss << "  Excluded    Failed" << endl; 

  oss << "  Folder:";
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_DirectoryTotal);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_DirectoryCreated);
  oss << setfill(' ') << setw(10) << "-";
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_DirectoryCreateSkipped);
  if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror)
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_DirectoriesCleaned);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_DirectoriesExcluded);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_DirectoryCreateFailed + aStats.m_DirectoriesCleanedFailed);
	oss << endl;

  oss << "    File:";
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_FilesTotal);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_FilesCopied);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_FilesLinked);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_FilesCopySkipped + aStats.m_FilesLinkSkipped); 
  if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror)
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_FilesCleaned);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_FilesExcluded);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_FilesCopyFailed + aStats.m_FilesLinkFailed + aStats.m_FilesCleanedFailed);
	oss << endl;

  oss << "Junction:";
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_JunctionsTotal);
  oss << setfill(' ') << setw(10) << "-";
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_JunctionsRestored + aStats.m_JunctionsDangling);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_JunctionsRestoreSkipped);
  if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror)
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_JunctionsCleaned);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_JunctionsExcluded + aStats.m_JunctionsCropped);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_JunctionsRestoreFailed + aStats.m_JunctionsCleanedFailed);
	oss << endl;

  // Mountpoints, but only if they were part of the operation
  if (
      aStats.m_MountpointsTotal || 
      aStats.m_MountpointsRestored || 
      aStats.m_MountpointsRestoreSkipped || 
      aStats.m_MountpointsRestoreFailed || 
      aStats.m_MountpointsCleaned || 
      aStats.m_MountpointsCleanedFailed || 
      aStats.m_MountpointsExcluded || 
      aStats.m_MountpointsCropped || 
      aStats.m_MountpointsDangling
  )
  {
    oss << "MountPnt:";
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_MountpointsTotal);
    oss << setfill(' ') << setw(10) << "-";
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_MountpointsRestored + aStats.m_MountpointsDangling);
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_MountpointsRestoreSkipped);
    if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror)
      oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_MountpointsCleaned);
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_MountpointsExcluded + aStats.m_MountpointsCropped);
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_MountpointsRestoreFailed + aStats.m_MountpointsCleanedFailed);
	  oss << endl;
  }

  // Unknown Reparsepoints, but only if they were part of the operation
  if (
      aStats.m_ReparsePointTotal || 
      aStats.m_ReparsePointRestored || 
      aStats.m_ReparsePointRestoreSkipped || 
      aStats.m_ReparsePointRestoreFailed || 
      aStats.m_ReparsePointCleaned || 
      aStats.m_ReparsePointCleanedFailed || 
      aStats.m_ReparsePointExcluded || 
      aStats.m_ReparsePointCropped || 
      aStats.m_ReparsePointDangling
  )
  {
    oss << "RparsUkn:";
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_ReparsePointTotal);
    oss << setfill(' ') << setw(10) << "-";
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_ReparsePointRestored + aStats.m_ReparsePointDangling);
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_ReparsePointRestoreSkipped);
    if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror)
      oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_ReparsePointCleaned);
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_ReparsePointExcluded + aStats.m_ReparsePointCropped);
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_ReparsePointRestoreFailed + aStats.m_ReparsePointCleanedFailed);
	  oss << endl;
  }

  oss << " Symlink:";
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_SymlinksTotal);
  oss << setfill(' ') << setw(10) << "-";
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_SymlinksRestored + aStats.m_SymlinksDangling);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_SymlinksRestoreSkipped);
  if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror)
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_SymlinksCleaned);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_SymlinksExcluded + aStats.m_SymlinksCropped);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_SymlinksRestoreFailed + aStats.m_SymlinksCleanedFailed);
	oss << endl;
    
  oss << "    Byte:";
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_BytesTotal);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_BytesCopied);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_BytesLinked);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_BytesCopySkippped + aStats.m_BytesLinkSkipped);
  if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror)
    oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_BytesCleaned);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_BytesExcluded);
  oss << setfill(' ') << setw(10) << FormatG(tmpstr, MAX_PATH, aStats.m_BytesCopyFailed + aStats.m_BytesLinkFailed);
	oss << endl;
	oss << endl;

  // Time calculations
  FILETIME64 StartTime, EndTime;
  FILETIME64 CopyTime;
  SystemTimeToFileTime(&aStats.m_StartTime, &StartTime.FileTime);
  SystemTimeToFileTime(&aStats.m_EndTime, &EndTime.FileTime);

  FILETIME64 TotalDuration;
  FILETIME64 CloneDuration, MirrorDuration;
  if (a_AutomatedTest)
    TotalDuration.ul64DateTime = 0x6789;
  else
    TotalDuration.ul64DateTime = EndTime.ul64DateTime - StartTime.ul64DateTime;

  SYSTEMTIME stTotalDuration;
  SYSTEMTIME stCloneDuration, stMirrorDuration;
  FileTimeToSystemTime(&TotalDuration.FileTime, &stTotalDuration);
  
  const int DurationStrLength = 64;
  char TotalDurationStr[DurationStrLength];
  char CloneDurationStr[DurationStrLength];
  char MirrorDurationStr[DurationStrLength];

  sprintf_s (TotalDurationStr, DurationStrLength, "%02d:%02d:%02d.%03d",
    stTotalDuration.wHour,
    stTotalDuration.wMinute,
    stTotalDuration.wSecond,
    stTotalDuration.wMilliseconds
  );

  // Only show the times for cloning if caled from DeloreanCopy
  if ( (aFlags & (FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror)) == (FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror) )
  {
    SystemTimeToFileTime(&aStats.m_CopyTime, &CopyTime.FileTime);

    if (a_AutomatedTest)
    {
      CloneDuration.ul64DateTime = 0x12345;
      MirrorDuration.ul64DateTime = 0x54321;
    }
    else
    {
      CloneDuration.ul64DateTime = CopyTime.ul64DateTime - StartTime.ul64DateTime;
      MirrorDuration.ul64DateTime = EndTime.ul64DateTime - CopyTime.ul64DateTime;
    }

    FileTimeToSystemTime(&CloneDuration.FileTime, &stCloneDuration);
    FileTimeToSystemTime(&MirrorDuration.FileTime, &stMirrorDuration);

    // The NT date starts at 1.1.1601
    if (stCloneDuration.wDay != 1)
    {
      sprintf_s (CloneDurationStr, DurationStrLength, "%dd %02d:%02d:%02d.%03d",
        stCloneDuration.wDay - 1,
        stCloneDuration.wHour,
        stCloneDuration.wMinute,
        stCloneDuration.wSecond,
        stCloneDuration.wMilliseconds
      );
    }
    else
    {
      sprintf_s (CloneDurationStr, DurationStrLength, "%02d:%02d:%02d.%03d",
        stCloneDuration.wHour,
        stCloneDuration.wMinute,
        stCloneDuration.wSecond,
        stCloneDuration.wMilliseconds
      );
    }

    // The NT date starts at 1.1.1601
    if (stMirrorDuration.wDay != 1)
    {
      sprintf_s (MirrorDurationStr, DurationStrLength, "%dd %02d:%02d:%02d.%03d",
        stMirrorDuration.wDay - 1,
        stMirrorDuration.wHour,
        stMirrorDuration.wMinute,
        stMirrorDuration.wSecond,
        stMirrorDuration.wMilliseconds
      );
    }
    else
    {
      sprintf_s (MirrorDurationStr, DurationStrLength, "%02d:%02d:%02d.%03d",
        stMirrorDuration.wHour,
        stMirrorDuration.wMinute,
        stMirrorDuration.wSecond,
        stMirrorDuration.wMilliseconds
      );
    }
  }

  if ( (aFlags & (FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror)) == (FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror) )
    oss << "                      Overall               Clone              Mirror" << endl;
  else
    oss << "                      Overall" << endl;

  oss << "   Times:";
  oss << setfill(' ') << setw(20) << TotalDurationStr;
  if ( (aFlags & (FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror)) == (FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror) )
  {
    oss << setfill(' ') << setw(20) << CloneDurationStr;
    oss << setfill(' ') << setw(20) << MirrorDurationStr;
  }
  oss << endl;

  fwprintf(a_OutputFile, L"%S", oss.str().c_str());
 
  return 42;
}

int PrintDeloreanCopyStatsJson(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  FileInfoContainer::CopyReparsePointFlags  aFlags,
  bool                                      a_AutomatedTest
)
{
  fwprintf (a_OutputFile, L"\"Summary\":{\n");

  fwprintf (a_OutputFile, L"\"Total\":{\"Folder\":%I64d,\"File\":%I64d,\"Junction\":%I64d,\"Mountpoint\":%I64d,\"ReparseUnknown\":%I64d,\"Symlink\":%I64d,\"Byte\":%I64d},\n",
    aStats.m_DirectoryTotal,
    aStats.m_FilesTotal,
    aStats.m_JunctionsTotal,
    aStats.m_MountpointsTotal,
    aStats.m_ReparsePointTotal,
    aStats.m_SymlinksTotal,
    aStats.m_BytesTotal
  );

  fwprintf (a_OutputFile, L"\"Copied\":{\"Folder\":%I64d,\"File\":%I64d,\"Junction\":\"-1\",\"Mountpoint\":\"-1\",\"ReparseUnknown\":\"-1\",\"Symlink\":\"-1\",\"Byte\":%I64d},\n",
    aStats.m_DirectoryCreated,
    aStats.m_FilesCopied,
    aStats.m_BytesCopied
  );

  fwprintf (a_OutputFile, L"\"Linked\":{\"Folder\":\"-1\",\"File\":%I64d,\"Junction\":%I64d,\"Mountpoint\":%I64d,\"ReparseUnknown\":%I64d,\"Symlink\":%I64d,\"Byte\":%I64d},\n",
    aStats.m_FilesLinked,
    aStats.m_JunctionsRestored + aStats.m_JunctionsDangling,
    aStats.m_MountpointsRestored + aStats.m_MountpointsDangling,
    aStats.m_ReparsePointRestored + aStats.m_ReparsePointDangling,
    aStats.m_SymlinksRestored + aStats.m_SymlinksDangling,
    aStats.m_BytesLinked
  );

  fwprintf (a_OutputFile, L"\"Skipped\":{\"Folder\":%I64d,\"File\":%I64d,\"Junction\":%I64d,\"Mountpoint\":%I64d,\"ReparseUnknown\":%I64d,\"Symlink\":%I64d,\"Byte\":%I64d},\n",
    aStats.m_DirectoryCreateSkipped,
    aStats.m_FilesCopySkipped + aStats.m_FilesLinkSkipped,
    aStats.m_JunctionsRestoreSkipped,
    aStats.m_MountpointsRestoreSkipped,
    aStats.m_ReparsePointRestoreSkipped,
    aStats.m_SymlinksRestoreSkipped,
    aStats.m_BytesCopySkippped + aStats.m_BytesLinkSkipped
  );
  if ((aFlags & FileInfoContainer::eSmartMirror) == FileInfoContainer::eSmartMirror)
    fwprintf (a_OutputFile, L"\"Removed\":{\"Folder\":%I64d,\"File\":%I64d,\"Junction\":%I64d,\"Mountpoint\":%I64d,\"ReparseUnknown\":%I64d,\"Symlink\":%I64d,\"Byte\":%I64d},\n",
      aStats.m_DirectoriesCleaned,
      aStats.m_FilesCleaned,
      aStats.m_JunctionsCleaned,
      aStats.m_MountpointsCleaned,
      aStats.m_ReparsePointCleaned,
      aStats.m_SymlinksCleaned,
      aStats.m_BytesCleaned
    );

  fwprintf (a_OutputFile, L"\"Excluded\":{\"Folder\":%I64d,\"File\":%I64d,\"Junction\":%I64d,\"Mountpoint\":%I64d,\"ReparseUnknown\":%I64d,\"Symlink\":%I64d,\"Byte\":%I64d},\n",
    aStats.m_DirectoriesExcluded,
    aStats.m_FilesExcluded,
    aStats.m_JunctionsExcluded + aStats.m_JunctionsCropped,
    aStats.m_MountpointsExcluded + aStats.m_MountpointsCropped,
    aStats.m_ReparsePointExcluded + aStats.m_ReparsePointCropped,
    aStats.m_SymlinksExcluded + aStats.m_SymlinksCropped,
    aStats.m_BytesExcluded
  );

  fwprintf (a_OutputFile, L"\"Failed\":{\"Folder\":%I64d,\"File\":%I64d,\"Junction\":%I64d,\"Mountpoint\":%I64d,\"ReparseUnknown\":%I64d,\"Symlink\":%I64d,\"Byte\":%I64d}}",
    aStats.m_DirectoryCreateFailed + aStats.m_DirectoriesCleanedFailed,
    aStats.m_FilesLinkFailed + aStats.m_FilesCleanedFailed,
    aStats.m_JunctionsRestoreFailed + aStats.m_JunctionsCleanedFailed,
    aStats.m_MountpointsRestoreFailed + aStats.m_MountpointsCleanedFailed,
    aStats.m_ReparsePointRestoreFailed + aStats.m_ReparsePointCleanedFailed,
    aStats.m_SymlinksRestoreFailed + aStats.m_SymlinksCleanedFailed,
    aStats.m_BytesCopyFailed + aStats.m_BytesLinkFailed
  );

  // Time calculations
  FILETIME64 StartTime, EndTime;
  SystemTimeToFileTime(&aStats.m_StartTime, &StartTime.FileTime);
  SystemTimeToFileTime(&aStats.m_EndTime, &EndTime.FileTime);

  FILETIME64 TotalDuration;
  if (a_AutomatedTest)
    TotalDuration.ul64DateTime = 191267;
  else
    TotalDuration.ul64DateTime = EndTime.ul64DateTime - StartTime.ul64DateTime;

  fwprintf (a_OutputFile, L",\n\"Times\":{\n");
  fwprintf (a_OutputFile, L"\"Overall\":%I64d", TotalDuration.ul64DateTime);


  if ( (aFlags & (FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror)) == (FileInfoContainer::eSmartCopy | FileInfoContainer::eSmartMirror) )
  {
    FILETIME64 CopyTime;
    FILETIME64 CloneDuration, MirrorDuration;
    SystemTimeToFileTime(&aStats.m_CopyTime, &CopyTime.FileTime);

    if (a_AutomatedTest)
    {
      CloneDuration.ul64DateTime = 30769;
      MirrorDuration.ul64DateTime = 220802;
    }
    else
    {
      CloneDuration.ul64DateTime = CopyTime.ul64DateTime - StartTime.ul64DateTime;
      MirrorDuration.ul64DateTime = EndTime.ul64DateTime - CopyTime.ul64DateTime;
    }

    fwprintf (a_OutputFile, L",\"Clone\":%I64d,\"Mirror\":%I64d",
      CloneDuration.ul64DateTime,
      MirrorDuration.ul64DateTime
    );
  }
  fwprintf (a_OutputFile, L"}\n");

  return ERROR_SUCCESS;
}

int PrintDupeMergeCopyStats(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats, 
  bool                                      a_ListOnly,
  bool                                      a_AutomatedTest
)
{
	//
	// Make a stringstreams for output
	//
	locale localeDeu( "" );
	ostringstream oss;
	oss.imbue( localeDeu );

	oss << endl;

  if (a_ListOnly)
    oss << "                         Total     Estimated Dupes" << endl; 
  else
    oss << "                         Total             Deduped        Dedup Failed" << endl; 

  oss << "     File:";
  oss << setfill(' ') << setw(20) << aStats.m_FilesSelected;
  oss << setfill(' ') << setw(20) << aStats.m_DupeFilesHardlinked;
  if (!a_ListOnly)
    oss << setfill(' ') << setw(20) << aStats.m_DupeFilesHardlinkFailed;
	oss << endl;

  oss << "    Bytes:";
  oss << setfill(' ') << setw(20) << aStats.m_BytesTotal;
  oss << setfill(' ') << setw(20) << aStats.m_DupeBytesSaved;
  if (!a_ListOnly)
    oss << setfill(' ') << setw(20) << "-";
	oss << endl;

	oss << endl;
	oss << endl;
  if (a_ListOnly)
    oss << "                         Total       Estimated New                 Old" << endl; 
  else
    oss << "                         Total                 New                 Old" << endl; 

  oss << "DupeGroup:";
  oss << setfill(' ') << setw(20) << aStats.m_DupeGroupsTotal;
  oss << setfill(' ') << setw(20) << aStats.m_DupeGroupsNew;
  oss << setfill(' ') << setw(20) << aStats.m_DupeGroupsOld;
	oss << endl;
	oss << endl;
	oss << endl;
  fwprintf(a_OutputFile, L"%S", oss.str().c_str());
 
  return 42;
}

void PrintInternalCounters(
  FILE*                                     a_OutputFile,
  CopyStatistics&                           aStats
)
{
  locale localeDeu("");
  ostringstream oss;
  oss.imbue(localeDeu);


  oss << "Allocation Time[uS]: " << aStats.m_HeapAllocTime.Get() << endl;
  oss << "Deletion Time[uS]: " << aStats.m_HeapDeletionTime.Get() << endl;
  oss << "RegExpMatch Time[uS]: " << aStats.m_RegExpMatchTime.Get() << endl;

  fwprintf(a_OutputFile, L"%S", oss.str().c_str());
}


int AnalysePathNameStatus(
  FILE*                 a_OutputFile,
  _PathNameStatusList&  a_PathNameStatusList, 
  bool                  a_Verbose,
  int                   a_LogLevel,
  bool                  a_Json,
  bool*                 a_JsonWriterState
)
{
  if (a_Json)
    return AnalysePathNameStatusJson(a_OutputFile, a_PathNameStatusList, a_Verbose, a_LogLevel, a_JsonWriterState);
  else
    return AnalysePathNameStatusNormal(a_OutputFile, a_PathNameStatusList, a_Verbose, a_LogLevel);
}

int AnalysePathNameStatusNormal(
  FILE*                 a_OutputFile,
  _PathNameStatusList&  a_PathNameStatusList, 
  bool                  a_Verbose,
  int                   a_LogLevel
)
{
  int RetVal = ERROR_SUCCESS;
  
  // Die Idee ist es die PathnameStatusLIst auszuwerten
  PathNameStatus pns;
  for ( _PathNameStatusList::iterator iter = a_PathNameStatusList.begin(); iter != a_PathNameStatusList.end(); ++iter)
  {
    pns = *iter;
    if (a_Verbose && a_LogLevel <= FileInfoContainer::eLogError)
    {
      switch (pns.m_ErrorCode)
      {
        case ERROR_ALREADY_EXISTS:
        break;

        default:
        {
          switch (pns.m_Operation)
          {
            case PlusF:
              fwprintf (a_OutputFile, L"!+f(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case StarH:
              fwprintf (a_OutputFile, L"!*h(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case PlusD:
              fwprintf (a_OutputFile, L"!+d(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case PlusJ:
              fwprintf (a_OutputFile, L"!+j(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case PlusS:
              fwprintf (a_OutputFile, L"!+s(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case PlusH:
              fwprintf (a_OutputFile, L"!+h(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case PlusR:
              fwprintf (a_OutputFile, L"!+r(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case MinusJ:
              fwprintf (a_OutputFile, L"!-j(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case MinusM:
              fwprintf (a_OutputFile, L"!-m(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case MinusS:
              fwprintf (a_OutputFile, L"!-s(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case MinusD:
              fwprintf (a_OutputFile, L"!-d(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case MinusF:
              fwprintf (a_OutputFile, L"!-f(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case EqualF:
              fwprintf (a_OutputFile, L"!=f(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case EqualD:
              fwprintf (a_OutputFile, L"!=d(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case EqualJ:
              fwprintf (a_OutputFile, L"!=j(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case EqualS:
              fwprintf (a_OutputFile, L"!=s(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case EqualH:
              fwprintf (a_OutputFile, L"!=h(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case QuestionD:
              fwprintf (a_OutputFile, L"!?d(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case QuestionJ:
              fwprintf (a_OutputFile, L"!?j(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case QuestionM:
              fwprintf (a_OutputFile, L"!?m(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case QuestionS:
              fwprintf (a_OutputFile, L"!?s(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case QuestionF:
              fwprintf (a_OutputFile, L"!?f(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case QuestionR:
              fwprintf (a_OutputFile, L"!?r(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case DangleJ:
              fwprintf (a_OutputFile, L"!&j(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case DangleM:
              fwprintf (a_OutputFile, L"!&m(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case BackSlashF:
              fwprintf (a_OutputFile, L"!\\f(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case MemoryMapF:
              fwprintf (a_OutputFile, L"!/f(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case CompressionF:
              fwprintf (a_OutputFile, L"!%%f(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case CompressionD:
              fwprintf (a_OutputFile, L"!%%d(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case PlusT:
              fwprintf (a_OutputFile, L"!+t(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case PlusE:
              fwprintf (a_OutputFile, L"!+e(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case PlusP:
              fwprintf (a_OutputFile, L"!+p(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case MergeF:
              fwprintf (a_OutputFile, L"!°f(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            case Nop:
              fwprintf (a_OutputFile, L"!nop(0x%08x) %s\n", pns.m_ErrorCode, pns.m_PathName);
            break;

            default:
              fwprintf (a_OutputFile, L"!%02x(0x%08x) %s\n", pns.m_Operation, pns.m_ErrorCode, pns.m_PathName);
            break;
          }

          // Return the first error code in the list
          if (ERROR_SUCCESS == RetVal)
            RetVal = pns.m_ErrorCode;
        }
        break;
      }
    }
  }
  return RetVal;
}

int AnalysePathNameStatusJson(
  FILE*                 a_OutputFile,
  _PathNameStatusList&  a_PathNameStatusList, 
  bool                  a_Verbose,
  int                   a_LogLevel,
  bool*                 a_JsonWriterState
)
{
  int RetVal = ERROR_SUCCESS;
  
  // Die Idee ist es die PathnameStatusLIst auszuwerten
  PathNameStatus pns;
  for ( _PathNameStatusList::iterator iter = a_PathNameStatusList.begin(); iter != a_PathNameStatusList.end(); ++iter)
  {
    pns = *iter;
    if (a_Verbose && a_LogLevel <= FileInfoContainer::eLogError)
    {
      const int StrLength = 4;
      wchar_t Type[StrLength];
      wchar_t Operation[StrLength];
      switch (pns.m_ErrorCode)
      {
        case ERROR_ALREADY_EXISTS:
        break;

        default:
        {
          switch (pns.m_Operation)
          {
            case PlusF:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"f");
            break;

            case StarH:
              wcscpy_s(Operation, StrLength, L"*");
              wcscpy_s(Type, StrLength, L"h");
            break;

            case PlusD:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"d");
            break;

            case PlusJ:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"j");
            break;

            case PlusS:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"s");
            break;

            case PlusH:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"h");
            break;

            case PlusR:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"r");
            break;

            case MinusJ:
              wcscpy_s(Operation, StrLength, L"-");
              wcscpy_s(Type, StrLength, L"j");
            break;

            case MinusM:
              wcscpy_s(Operation, StrLength, L"-");
              wcscpy_s(Type, StrLength, L"m");
            break;

            case MinusS:
              wcscpy_s(Operation, StrLength, L"-");
              wcscpy_s(Type, StrLength, L"s");
            break;

            case MinusD:
              wcscpy_s(Operation, StrLength, L"-");
              wcscpy_s(Type, StrLength, L"d");
            break;

            case MinusF:
              wcscpy_s(Operation, StrLength, L"-");
              wcscpy_s(Type, StrLength, L"f");
            break;

            case EqualF:
              wcscpy_s(Operation, StrLength, L"=");
              wcscpy_s(Type, StrLength, L"f");
            break;

            case EqualD:
              wcscpy_s(Operation, StrLength, L"=");
              wcscpy_s(Type, StrLength, L"d");
            break;

            case EqualJ:
              wcscpy_s(Operation, StrLength, L"=");
              wcscpy_s(Type, StrLength, L"j");
            break;

            case EqualS:
              wcscpy_s(Operation, StrLength, L"=");
              wcscpy_s(Type, StrLength, L"s");
            break;

            case EqualH:
              wcscpy_s(Operation, StrLength, L"=");
              wcscpy_s(Type, StrLength, L"h");
            break;

            case QuestionD:
              wcscpy_s(Operation, StrLength, L"?");
              wcscpy_s(Type, StrLength, L"d");
            break;

            case QuestionJ:
              wcscpy_s(Operation, StrLength, L"?");
              wcscpy_s(Type, StrLength, L"j");
            break;

            case QuestionM:
              wcscpy_s(Operation, StrLength, L"?");
              wcscpy_s(Type, StrLength, L"m");
            break;

            case QuestionS:
              wcscpy_s(Operation, StrLength, L"?");
              wcscpy_s(Type, StrLength, L"s");
            break;

            case QuestionF:
              wcscpy_s(Operation, StrLength, L"?");
              wcscpy_s(Type, StrLength, L"f");
            break;

            case QuestionR:
              wcscpy_s(Operation, StrLength, L"?");
              wcscpy_s(Type, StrLength, L"r");
            break;

            case DangleJ:
              wcscpy_s(Operation, StrLength, L"&");
              wcscpy_s(Type, StrLength, L"j");
            break;

            case DangleM:
              wcscpy_s(Operation, StrLength, L"&");
              wcscpy_s(Type, StrLength, L"m");
            break;

            case BackSlashF:
              wcscpy_s(Operation, StrLength, L"\\");
              wcscpy_s(Type, StrLength, L"f");
            break;

            case MemoryMapF:
              wcscpy_s(Operation, StrLength, L"/");
              wcscpy_s(Type, StrLength, L"f");
            break;

            case CompressionF:
              wcscpy_s(Operation, StrLength, L"%");
              wcscpy_s(Type, StrLength, L"f");
            break;
            
            case CompressionD:
              wcscpy_s(Operation, StrLength, L"%");
              wcscpy_s(Type, StrLength, L"d");
            break;

            case PlusT:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"t");
            break;

            case PlusE:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"e");
            break;

            case PlusP:
              wcscpy_s(Operation, StrLength, L"+");
              wcscpy_s(Type, StrLength, L"p");
            break;

            case MergeF:
              wcscpy_s(Operation, StrLength, L"°");
              wcscpy_s(Type, StrLength, L"f");
            break;

            case Nop:
              wcscpy_s(Operation, StrLength, L"nop");
              wcscpy_s(Type, StrLength, L"nop");
            break;

            default:
              swprintf_s(Operation, StrLength, L"%02x", pns.m_Operation);
              wcscpy_s(Type, StrLength, L"nop");
            break;
          }

          // Return the first error code in the list
          if (ERROR_SUCCESS == RetVal)
            RetVal = pns.m_ErrorCode;
        }
        break;
      }
      wchar_t JsonPath[HUGE_PATH];
      wcsesc_s(JsonPath, pns.m_PathName, HUGE_PATH, L'\\');

      if (*a_JsonWriterState)
        fwprintf (a_OutputFile, L",\n");
      else
        *a_JsonWriterState = true;

      fwprintf (a_OutputFile, L"{\"op\":\"%s\",\"er\":%d,\"ty\":\"%s\",\"pa\":\"%s\"}", Operation, pns.m_ErrorCode, Type, JsonPath);
    }
  }
  return RetVal;
}


/*
typedef enum  {
  TokenElevationTypeDefault   = 1,
  TokenElevationTypeFull,
  TokenElevationTypeLimited 
} TOKEN_ELEVATION_TYPE , *PTOKEN_ELEVATION_TYPE;
*/
/*
typedef enum _TOKEN_INFORMATION_CLASS {
  TokenUser                    = 1,
  TokenGroups,
  TokenPrivileges,
  TokenOwner,
  TokenPrimaryGroup,
  TokenDefaultDacl,
  TokenSource,
  TokenType,
  TokenImpersonationLevel,
  TokenStatistics,
  TokenRestrictedSids,
  TokenSessionId,
  TokenGroupsAndPrivileges,
  TokenSessionReference,
  TokenSandBoxInert,
  TokenAuditPolicy,
  TokenOrigin,
  TokenElevationType,
  TokenLinkedToken,
  TokenElevation,
  TokenHasRestrictions,
  TokenAccessInformation,
  TokenVirtualizationAllowed,
  TokenVirtualizationEnabled,
  TokenIntegrityLevel,
  TokenUIAccess,
  TokenMandatoryPolicy,
  TokenLogonSid,
  MaxTokenInfoClass 
} TOKEN_INFORMATION_CLASS, *PTOKEN_INFORMATION_CLASS;
*/

bool 
ElevationNeeded(
)
{
  bool Elevation = false;
  
  HANDLE hToken;
  TOKEN_ELEVATION_TYPE elevationType;
  DWORD dwSize;

  OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
  GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)18 /*TokenElevationType*/, &elevationType, sizeof(elevationType), &dwSize);

  if (hToken)
  {
    switch (elevationType) 
    {
      case TokenElevationTypeDefault:
        // User is either admin with UAC off, or a normal user with UAC on
        HTRACE(TEXT("\nTokenElevationTypeDefault - User is not using a split token.\n"));
      break;

      case TokenElevationTypeFull:
        // UAC is on, but process is elevated, e.g from administrative prompt
        HTRACE(TEXT("\nTokenElevationTypeFull - User has a split token, and the process is running elevated.\n"));
      break;

      case TokenElevationTypeLimited:
        Elevation = true;
        // UAC is on, elevation prompt needed
        HTRACE(L"\nTokenElevationTypeLimited - User has a split token, but the process is not running elevated.\n");
      break;

      default:
        // We are on an OS < Windows Vista
        HTRACE(L"\nToken Unknown - %d.\n", elevationType);
      break;
    }
    CloseHandle(hToken);
  }
  else
    HTRACE(L"\nGetTokenInformation failed.\n");


  // if elevation is needed, there might be user who have SeCreateSymbolicLink enabled 
  // for all processes, so we have to check if this is on.
  if (Elevation)
  {
    // But this check will only detect SeCreateSymbolicLink, and we might to elevate for
    // other reasons
    if (ProbeTokenPrivilege(SE_CREATE_SYMBOLICLINK_PRIVILEGE))
      Elevation = false;
  }
  else
  {
    // Elevation is not needed, due to the split token, but this could also be a normal user
    // and not an Admin, so overrule this
    if (!IsUserAnAdmin())
    {
      if (!ProbeTokenPrivilege(SE_CREATE_SYMBOLICLINK_PRIVILEGE))
        Elevation = true;
    }
  }

  return Elevation;
}

bool 
SymLinkAllowUnprivilegedCreation(
  __inout LPOSVERSIONINFO lpVersionInfo
)
{
  if (lpVersionInfo->dwMajorVersion >= 10 && lpVersionInfo->dwBuildNumber >= 14972 && IsDeveloperModeEnabled())
    return true;
  else
    return false;
}

//
// Set attributes via handle
//
int
SetAttributes (
  __in     HANDLE         lpExistingFileHandle,
  __in     DWORD          dwFileAttributes
)
{
  IO_STATUS_BLOCK iosb;
  NTSTATUS  Status;

  FILE_BASIC_INFORMATION fbi;

  fbi.FileAttributes = dwFileAttributes;
  fbi.CreationTime.QuadPart = 0;
  fbi.LastAccessTime.QuadPart = 0;
  fbi.LastWriteTime.QuadPart = 0;
  fbi.ChangeTime.QuadPart = 0;

  Status = NtSetInformationFile(
    lpExistingFileHandle,
    &iosb,
    &fbi,
    sizeof(fbi),
    FileBasicInformation
  );

  if (NT_SUCCESS(Status))
    return ERROR_SUCCESS;
  else
    return ERROR_FILE_NOT_FOUND;
}

//
// Set timestamps and attributes in one call
//
int
SetFileBasicInformation (
  __in     HANDLE         lpExistingFileHandle,
  __in     DWORD          dwFileAttributes,
  __in     PFILETIME      ftCreationTime,
  __in     PFILETIME      ftLastAccessTime,
  __in     PFILETIME      ftLastWriteTime
)
{
  IO_STATUS_BLOCK iosb;
  NTSTATUS  Status;

  FILE_BASIC_INFORMATION fbi;

  fbi.FileAttributes = dwFileAttributes;
  fbi.CreationTime.LowPart = ftCreationTime->dwLowDateTime;
  fbi.CreationTime.HighPart = ftCreationTime->dwHighDateTime;

  fbi.LastAccessTime.LowPart = ftLastAccessTime->dwLowDateTime;
  fbi.LastAccessTime.HighPart = ftLastAccessTime->dwHighDateTime;

  fbi.LastWriteTime.LowPart = ftLastWriteTime->dwLowDateTime;
  fbi.LastWriteTime.HighPart = ftLastWriteTime->dwHighDateTime;

  fbi.ChangeTime.QuadPart = 0;

  Status = NtSetInformationFile(
    lpExistingFileHandle,
    &iosb,
    &fbi,
    sizeof(fbi),
    FileBasicInformation
  );

  if (NT_SUCCESS(Status))
    return ERROR_SUCCESS;
  else
    return ERROR_FILE_NOT_FOUND;
}

//
// Get timestamps and attributes in one call
//
int
GetFileBasicInformation (
  __in     HANDLE         lpExistingFileHandle,
  __inout  PDWORD         dwFileAttributes,
  __inout    PFILETIME    ftCreationTime,
  __inout    PFILETIME    ftLastAccessTime,
  __inout    PFILETIME    ftLastWriteTime
)
{
  IO_STATUS_BLOCK iosb;
  NTSTATUS  Status;

  FILE_BASIC_INFORMATION fbi;

  Status = NtQueryInformationFile(
    lpExistingFileHandle,
    &iosb,
    &fbi,
    sizeof(fbi),
    FileBasicInformation
  );

  *dwFileAttributes = fbi.FileAttributes;

  
  (*ftCreationTime).dwLowDateTime = fbi.CreationTime.LowPart;
  (*ftCreationTime).dwHighDateTime = fbi.CreationTime.HighPart;

  (*ftLastAccessTime).dwLowDateTime = fbi.LastAccessTime.LowPart;
  (*ftLastAccessTime).dwHighDateTime = fbi.LastAccessTime.HighPart;

  (*ftLastWriteTime).dwLowDateTime = fbi.LastWriteTime.LowPart;
  (*ftLastWriteTime).dwHighDateTime = fbi.LastWriteTime.HighPart;

  if (NT_SUCCESS(Status))
    return ERROR_SUCCESS;
  else
    return ERROR_FILE_NOT_FOUND;
}


//
// GetFileSizeEx2 retrieves the size of a file including all its collateral 
// sizes, like alternative streams 
// This is needed in Backup applications to properly calculate the progress
// because it could be that a file has just a 4 byte $DATA stream but a 4GB
// alternative stream
//
int
GetFileSizeEx2 (
  __in     HANDLE         lpExistingFileHandle,
  __out    PLARGE_INTEGER lpFileSize
)
{
  IO_STATUS_BLOCK iosb;
  NTSTATUS  Status;

  PVOID		pBuffer;
  PFILE_STREAM_INFORMATION pFileStream;
  int			BufferSize = 0;

  do
  {
    BufferSize += 0x400;
    pBuffer = (LPBYTE)malloc(BufferSize);

    Status = NtQueryInformationFile(
      lpExistingFileHandle,
      &iosb,
      pBuffer,
      BufferSize,
      FileStreamInformation
      );

    if (Status == STATUS_INFO_LENGTH_MISMATCH) 
      // Grow the buffer, which means delete the already allocated buffer
      free(pBuffer);

  } while (Status == STATUS_INFO_LENGTH_MISMATCH);

  pFileStream = (PFILE_STREAM_INFORMATION)pBuffer;
  lpFileSize->QuadPart = 0;

  // Check if we received the stream info
  if (NT_SUCCESS(Status) && iosb.Information != 0 && pFileStream->StreamNameLength != 0) 
  {
    // Run through the available filestreams
    UINT Offset;
    do
    {
      lpFileSize->QuadPart += pFileStream->EndOfStream.QuadPart;

      Offset = pFileStream->NextEntryOffset;
      pFileStream = (PFILE_STREAM_INFORMATION)((PBYTE)pFileStream + Offset);

    } while (Offset);

    free(pBuffer);
    return ERROR_SUCCESS;
  }
  else
  {
    // We might end up here because we had no access on the streams
    // return a proper error code
    free(pBuffer);
    return ERROR_FILE_NOT_FOUND;
  }
}


//--------------------------------------------------------------------
//
// CopyCompression
//
// Copies the compression status
// ERROR_FILE_EXISTS if compression of file didn't change
// ERROR_SUCCESS if compression status changed
//
//--------------------------------------------------------------------
int
CopyCompression(
  __in     LPCWSTR  lpExistingFileName,
  __in     LPCWSTR  lpNewFileName,
  __in     DWORD    dwExistingAttributes,
  __in     DWORD    dwNewAttributes
)
{
  int RetVal = ERROR_SUCCESS;
  
  if (INVALID_FILE_ATTRIBUTES == dwNewAttributes)
    return ERROR_FILE_NOT_FOUND;

  // Handle the compression bit: If the existing file and the new file have a different 
  // compression status, change the compression accordingly on the new file
  //
  if (dwExistingAttributes & FILE_ATTRIBUTE_COMPRESSED ^ dwNewAttributes & FILE_ATTRIBUTE_COMPRESSED)
  {
    HANDLE  ExistingFileHandle = CreateFile(
      lpExistingFileName, 
      FILE_READ_ATTRIBUTES,
      FILE_SHARE_READ, 
      NULL, 
      OPEN_EXISTING, 
      FILE_FLAG_BACKUP_SEMANTICS, 
      NULL
    );

    if (INVALID_HANDLE_VALUE != ExistingFileHandle)
    {
      HANDLE  NewFileHandle = CreateFile(
        lpNewFileName, 
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL, 
        OPEN_EXISTING, 
        FILE_FLAG_BACKUP_SEMANTICS, 
        NULL
        );

      if (INVALID_HANDLE_VALUE != NewFileHandle)
      {
        // Change the compression
        //
        DWORD BytesReturned;
        USHORT CompressionMode = COMPRESSION_FORMAT_DEFAULT;

        BOOL b = DeviceIoControl(ExistingFileHandle,
          FSCTL_GET_COMPRESSION,
          NULL,
          0,
          &CompressionMode,
          sizeof(CompressionMode),
          &BytesReturned,
          NULL
          );
        if (!b)
        {
          RetVal = GetLastError();
        }
        else
        {
          b = DeviceIoControl(NewFileHandle,
            FSCTL_SET_COMPRESSION,
            &CompressionMode,
            sizeof(CompressionMode),
            NULL,
            0,
            &BytesReturned,
            NULL
            );
          if (!b)
          {
            RetVal = GetLastError();
          }
        }
      
        CloseHandle(NewFileHandle);

      }
      else
      {
        // We failed on opening the new file
        RetVal = GetLastError();
      }
      CloseHandle(ExistingFileHandle);
    }
    else
    {
      RetVal = GetLastError();
    }
  }

  return RetVal;
}

//--------------------------------------------------------------------
//
// GetEaRecords
//
// Reads the EA Records of a file object into *a_pFileEaInfo
//
//--------------------------------------------------------------------
int
GetEaRecords( 
  __in    HANDLE                      a_SrcFileHandle,
  __inout PFILE_FULL_EA_INFORMATION*  a_pFileEaInfo,
  __inout LPDWORD                     a_dwFileEaInfoSize
)
{
  IO_STATUS_BLOCK       iosb;
	int			              BufferSize = 0x10000;
	NTSTATUS	            Status;
  FILE_EA_INFORMATION   FileEaInfo;

  //  Loop trying to read all EA data into the buffer and
  //  resize the buffer if necessary
  //

  *a_dwFileEaInfoSize = 0;
  while (TRUE) 
  {
	  *a_pFileEaInfo = (PFILE_FULL_EA_INFORMATION)malloc(BufferSize);

    Status = NtQueryEaFile(
      a_SrcFileHandle,
      &iosb,
      *a_pFileEaInfo,
      BufferSize,
      FALSE,
      NULL,
      0,
      0,
      (BOOLEAN) TRUE 
    );

    if (STATUS_NO_EAS_ON_FILE == Status)
      return ERROR_SUCCESS;

    //  If we successfully read all the data finish the loop
    //
    if (NT_SUCCESS( Status ) && iosb.Information != 0)
      break;

    //  If we received a status other than buffer overflow then skip EA's altogether
    //  
    if (!( (Status == STATUS_BUFFER_OVERFLOW) || (Status == STATUS_BUFFER_TOO_SMALL) ) )
      return ERROR_SUCCESS;

    //  Ok, there were EA records, but the buffer was too small. So get the EA Size and add 20%
    //
    Status = NtQueryInformationFile(
      a_dwFileEaInfoSize,
      &iosb,
      &FileEaInfo,
      sizeof(FileEaInfo),
      FileEaInformation
    );

    //  This call should never have failed. However, if it does, skip EAs altogether
    //
    if (!NT_SUCCESS(Status)) 
      return ERROR_SUCCESS;

    //  Resize the buffer to something that seems reasonable
    //
    free (*a_pFileEaInfo);

    BufferSize = FileEaInfo.EaSize * 5 / 4;

    // And with the next loop try to retrieve EA records again.
  }

  *a_dwFileEaInfoSize = BufferSize;

  return ERROR_SUCCESS;
}

//--------------------------------------------------------------------
//
// SetEaRecords
//
// Writes the EA Records in a_pFileEaInfo to a file
//
//--------------------------------------------------------------------
int
SetEaRecords(
  __in    HANDLE                      a_DstFileHandle,
  __in    PFILE_FULL_EA_INFORMATION   a_pFileEaInfo,
  __in    DWORD                       a_dwFileEaInfoSize
)
{
  IO_STATUS_BLOCK       iosb;
	NTSTATUS	            Status;

  // We were able to read EA records, so write it back
  Status = NtSetEaFile(
    a_DstFileHandle,
    &iosb,
    a_pFileEaInfo,
    a_dwFileEaInfoSize
  );

  if (!NT_SUCCESS( Status )) 
    return ERROR_ACCESS_DENIED;

  return ERROR_SUCCESS;
}

//--------------------------------------------------------------------
//
// DumpEaRecords
//
// Dumps the content of an EA Records to stdout
//
//--------------------------------------------------------------------
int
DumpEaRecords(
  __in    PFILE_FULL_EA_INFORMATION   a_pFileEaInfo
)
{
  PFILE_FULL_EA_INFORMATION pEI = a_pFileEaInfo;
  PFILE_FULL_EA_INFORMATION pCurrentEI;
  do
  {
    pCurrentEI = pEI;
    char EaName[MAX_PATH];
    memcpy(EaName, pEI->EaName, pEI->EaNameLength);
    EaName[pCurrentEI->EaNameLength] = 0x00;
    printf("%08x: '%s'\n", pCurrentEI->EaValueLength, EaName);

    UINT	Offset = pEI->NextEntryOffset;
    pEI = (PFILE_FULL_EA_INFORMATION)((PBYTE)pEI + Offset);
  }
  while (pCurrentEI->NextEntryOffset != 0);

  return ERROR_SUCCESS;
}

//--------------------------------------------------------------------
//
// AddEaRecords
//
// Add data to an EA record in memory
//
//--------------------------------------------------------------------
PFILE_FULL_EA_INFORMATION
AddEaRecords(
  __in    PFILE_FULL_EA_INFORMATION   a_pFileEaInfo,
  __in    char*                       a_EaRecordName,
  __in    LPVOID                      a_pPayLoad,
  __in    USHORT                      a_dwPayLoadSize
)
{
  // Scan till end of EA record
  ULONG Offset = 0;
  if (a_pFileEaInfo->EaNameLength != 0 || a_pFileEaInfo->EaValueLength != 0)
  {
    Offset = a_pFileEaInfo->EaNameLength + a_pFileEaInfo->EaValueLength + sizeof(FILE_FULL_EA_INFORMATION) - 3;
    Offset = Offset & 0xfffffffc;
    Offset += 4;
  }

  a_pFileEaInfo->NextEntryOffset = Offset;
  PFILE_FULL_EA_INFORMATION pEaCurrent = (PFILE_FULL_EA_INFORMATION)((PBYTE)a_pFileEaInfo + Offset);

  pEaCurrent->NextEntryOffset = 0;
  pEaCurrent->Flags = 0;
  pEaCurrent->EaNameLength = (UCHAR)strlen(a_EaRecordName);
  strcpy(pEaCurrent->EaName, a_EaRecordName);

  pEaCurrent->EaValueLength = a_dwPayLoadSize;
  PBYTE pPayLoad = (PBYTE)(pEaCurrent->EaName) + pEaCurrent->EaNameLength + 1;
  memcpy(pPayLoad, a_pPayLoad, a_dwPayLoadSize);
 
  return pEaCurrent;
}

//--------------------------------------------------------------------
//
// SaveEaRecords
//
// Save the content of an EA Records to files, which are named like
// the name of the EA
//
//--------------------------------------------------------------------
int
SaveEaRecords(
  __in    PFILE_FULL_EA_INFORMATION   a_pFileEaInfo
)
{
  int iResult = ERROR_SUCCESS;

  PFILE_FULL_EA_INFORMATION pEI = a_pFileEaInfo;
  PFILE_FULL_EA_INFORMATION pEaCurrent;
  do
  {
    pEaCurrent = pEI;
    char EaName[MAX_PATH];
    memcpy(EaName, pEI->EaName, pEI->EaNameLength);
    EaName[pEaCurrent->EaNameLength] = 0x00;
    
    HANDLE PayLoad = CreateFileA(
      EaName, 
      FILE_GENERIC_READ | FILE_GENERIC_WRITE,
      FILE_SHARE_READ, 
      NULL, 
      OPEN_ALWAYS, 
      FILE_FLAG_BACKUP_SEMANTICS, 
      NULL
    );

    if (INVALID_HANDLE_VALUE != PayLoad)
    {
      DWORD BytesWritten;
      PBYTE pPayLoad = (PBYTE)(pEaCurrent->EaName) + pEaCurrent->EaNameLength + 1;
      BOOL bResult = WriteFile(PayLoad, pPayLoad, pEaCurrent->EaValueLength, &BytesWritten, NULL);
      CloseHandle(PayLoad);
      if (!( bResult || pEaCurrent->EaValueLength != BytesWritten) )
      {
        iResult = ERROR_ACCESS_DENIED;
        continue;
      }
    }
    else
      iResult = ERROR_FILE_NOT_FOUND;

    UINT	Offset = pEI->NextEntryOffset;
    pEI = (PFILE_FULL_EA_INFORMATION)((PBYTE)pEI + Offset);
  }
  while (pEaCurrent->NextEntryOffset != 0);

  return iResult;
}

//--------------------------------------------------------------------
//
// CopyEaRecords
//
// Copy EA Records from source to destination
//
//--------------------------------------------------------------------
int
CopyEaRecords(
  __in     HANDLE                 a_ExistingFileHandle,
  __in     HANDLE                 a_NewFileHandle
)
{
  PFILE_FULL_EA_INFORMATION pFileEaInfo{ nullptr };
  DWORD dwFileEaInfoSize{ 0 };

  int iResult = GetEaRecords(a_ExistingFileHandle, &pFileEaInfo, &dwFileEaInfoSize);
  if ((ERROR_SUCCESS == iResult) && dwFileEaInfoSize)
    iResult = SetEaRecords(a_NewFileHandle, pFileEaInfo, dwFileEaInfoSize);

  free(pFileEaInfo);

  return iResult;
}

//--------------------------------------------------------------------
//
// GetSecurityAttributes
//
// Copies the Security Attributes of a file object to m_SecDesc
//
//--------------------------------------------------------------------
inline
int
GetSecurityAttributes( 
  __in    HANDLE                a_SrcHandle,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize
)
{
  // Propably the security descriptor already fits
  DWORD SecDescSize;
  NTSTATUS ntstatus = NtQuerySecurityObject(
    a_SrcHandle, 
    OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, 
    *a_pSecDesc, 
    *a_SecDescSize,
    &SecDescSize 
  ); 

  if(STATUS_BUFFER_TOO_SMALL == ntstatus) 
  { 
    free(*a_pSecDesc);
    *a_pSecDesc = (PSECURITY_DESCRIPTOR)malloc(SecDescSize);
    *a_SecDescSize = SecDescSize;

    if(!*a_pSecDesc) 
      return STATUS_NO_MEMORY; 
  } 

  ntstatus = NtQuerySecurityObject(
    a_SrcHandle, 
    OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, 
    *a_pSecDesc, 
    SecDescSize,
    &SecDescSize
  ); 

  return ntstatus;
}

//--------------------------------------------------------------------
//
// SetSecurityAttributes
//
// Stores the Security Attributes from m_SecDesc to a file object
//
//--------------------------------------------------------------------
inline
int
SetSecurityAttributes( 
  __in    HANDLE                a_DstHandle,
  __in    PSECURITY_DESCRIPTOR  a_pSecDesc
)
{
    NTSTATUS ntstatus = NtSetSecurityObject(
      a_DstHandle, 
      OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, 
      a_pSecDesc
    ); 

  return ntstatus;
}


//--------------------------------------------------------------------
//
// CopySecurityAttributes
//
// Copies the Security Attributes of a file object
//
//--------------------------------------------------------------------
int
CopySecurityAttributes( 
  __in    HANDLE                a_SrcHandle,
  __in    HANDLE                a_DstHandle,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize
)
{
  NTSTATUS ntstatus = GetSecurityAttributes(a_SrcHandle, a_pSecDesc, a_SecDescSize);
  if(NT_SUCCESS(ntstatus)) 
    ntstatus = SetSecurityAttributes(a_DstHandle, *a_pSecDesc);

  if(NT_SUCCESS(ntstatus)) 
    return ERROR_SUCCESS;
  else
    return GetLastError();
}

//--------------------------------------------------------------------
//
// CopySecurityAttributesByName
//
// Copies the Security Attributes of a directory
//
//--------------------------------------------------------------------
int
CopySecurityAttributesByName( 
  __in    PCWSTR                aSrcName,
  __in    PCWSTR                aDstName,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize,
  __in    const int             aFlagsAndAttributes
)
{
  int iResult = ERROR_INVALID_FUNCTION;
  HANDLE  SrcHandle = CreateFile(
    aSrcName, 
    GENERIC_READ | READ_CONTROL | ACCESS_SYSTEM_SECURITY,
    FILE_SHARE_READ, 
    NULL, 
    OPEN_EXISTING, 
    aFlagsAndAttributes | FILE_FLAG_BACKUP_SEMANTICS, 
    NULL
  );

  if (INVALID_HANDLE_VALUE != SrcHandle)
  {
    HANDLE  DstHandle = CreateFile(
      aDstName, 
      GENERIC_WRITE | WRITE_OWNER | WRITE_DAC | ACCESS_SYSTEM_SECURITY,
      FILE_SHARE_READ, 
      NULL, 
      OPEN_ALWAYS, 
      aFlagsAndAttributes | FILE_FLAG_BACKUP_SEMANTICS, 
      NULL
    );

    if (INVALID_HANDLE_VALUE != DstHandle)
    {
      iResult = CopySecurityAttributes(SrcHandle, DstHandle, a_pSecDesc, a_SecDescSize);
      CloseHandle(DstHandle);
    }
    CloseHandle(SrcHandle);
  }

  return iResult;
}

//--------------------------------------------------------------------
//
// GetSecurityAttributesByName
//
// Fetches the security attributes of a file object into m_SecDesc
//
//--------------------------------------------------------------------
int
GetSecurityAttributesByName( 
  __in    PCWSTR                aSrcName,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize,
  __in    const int             aFlagsAndAttributes
)
{
  int iResult = ERROR_INVALID_FUNCTION;
  HANDLE  SrcHandle = CreateFile(
    aSrcName, 
    GENERIC_READ | READ_CONTROL | ACCESS_SYSTEM_SECURITY,
    FILE_SHARE_READ, 
    NULL, 
    OPEN_EXISTING, 
    aFlagsAndAttributes | FILE_FLAG_BACKUP_SEMANTICS, 
    NULL
  );

  if (INVALID_HANDLE_VALUE != SrcHandle)
  {
    iResult = GetSecurityAttributes(SrcHandle, a_pSecDesc, a_SecDescSize);
    CloseHandle(SrcHandle);
  }

  return iResult;
}

//--------------------------------------------------------------------
//
// SetSecurityAttributesByName
//
// Stores the security attributes from m_SecDesc to a file object
//
//--------------------------------------------------------------------
int
SetSecurityAttributesByName( 
  __in    PCWSTR                aDstName,
  __in    PSECURITY_DESCRIPTOR  a_pSecDesc,
  __in    const int             aFlagsAndAttributes
)
{
  int iResult = ERROR_INVALID_FUNCTION;
  HANDLE  DstHandle = CreateFile(
    aDstName, 
    GENERIC_WRITE | WRITE_OWNER | WRITE_DAC | ACCESS_SYSTEM_SECURITY,
    FILE_SHARE_READ, 
    NULL, 
    OPEN_ALWAYS, 
    aFlagsAndAttributes | FILE_FLAG_BACKUP_SEMANTICS, 
    NULL
  );

  if (INVALID_HANDLE_VALUE != DstHandle)
  {
    iResult = SetSecurityAttributes(DstHandle, a_pSecDesc);
    CloseHandle(DstHandle);
  }

  return iResult;
}


//--------------------------------------------------------------------
//
// WriteEncryptedStreamCallBack
//
// Copies a data stream of a file
//
//--------------------------------------------------------------------
DWORD WINAPI WriteEncryptedStreamCallBack(
  __in     PBYTE   a_WriteData, 
  __in     PVOID   pvCallbackContext, 
  __inout  PULONG  a_WriteLength
)
{
  DWORD dwResult = ERROR_SUCCESS;

  _ReadWriteHandShake* pRwHs = (_ReadWriteHandShake*)pvCallbackContext;

  // Do we have to work off data from a Write round before?
  if (0 == pRwHs->BytesWritten)
  {
    // No we need fresh data and wait for it
    DWORD dwWait = WaitForSingleObject(pRwHs->DataAvailable, 10000);
    if (WAIT_OBJECT_0 != dwWait)
      dwResult = dwWait;
  }

  // See if all data from the reader fits in the writebuffer
  if (pRwHs->ReadLength < *a_WriteLength)
    // No we have to chop off data, and process it in subsequent WritecallBacks
    *a_WriteLength = pRwHs->ReadLength;

  // Calculate how many data is left. If the read buffer and the write buffer is 
  // of same size, ReadLength will be 0, but if ReadLength > 0 we then know
  // what is left to write
  pRwHs->ReadLength -= *a_WriteLength;

  if (*a_WriteLength > 0)
  {
    memcpy(a_WriteData, &pRwHs->ReadData[pRwHs->BytesWritten], *a_WriteLength);

    pRwHs->TotalBytesTransferred.QuadPart += *a_WriteLength;

    // Handle the callback methods
    if (pRwHs->lpProgressRoutine)
    {
      DWORD ret = pRwHs->lpProgressRoutine(
        pRwHs->TotalFileSize,
        pRwHs->TotalBytesTransferred,
        pRwHs->TotalFileSize,
        pRwHs->TotalBytesTransferred,
        0,                                      // Encrypted Streams are always on the maintream and never on the alternative stream
        CALLBACK_CHUNK_FINISHED,
        0,
        0,
        pRwHs->lpData
      );

      switch (ret)
      {
        case PROGRESS_CANCEL:
          dwResult = ERROR_OPERATION_ABORTED;
          // ADV_TBD Delete the file which has been created
        break;

        case PROGRESS_STOP:
          dwResult = ERROR_OPERATION_ABORTED;
        break;

      }
    } // if (lpProgressRoutine)

  }

  // If the buffer, which was read in the ReadCallBack was too big, we have 
  // to work off the data in more than one WriteCallBacks. In order to keep track
  // where to continue during the next round of writing, save the position in the buffer
  pRwHs->BytesWritten += *a_WriteLength;

  // Check if we could write all of the buffer
  if (0 == pRwHs->ReadLength)
  {
    // We were able to work off the whole data of the readbuffer, either in the first
    // round or in a subsequent round. We show this by setting BytesWritten to 0
    pRwHs->BytesWritten = 0;

    // Request more data from the reader
    SetEvent(pRwHs->DataReceived);
  }

  return dwResult;
}

//--------------------------------------------------------------------
//
// ReadEncryptedStream
//
//--------------------------------------------------------------------
DWORD
__stdcall
ReadEncryptedStream(
  __in     _ReadWriteHandShake*  pRwHs
)
{
  DWORD dwResult = ReadEncryptedFileRaw(&ReadEncryptedStreamCallBack, (PVOID)pRwHs, pRwHs->ReadContext);

  return 1;
}
//--------------------------------------------------------------------
//
// ReadEncryptedStreamCallBack
//
//--------------------------------------------------------------------
DWORD WINAPI ReadEncryptedStreamCallBack(
  __in     PBYTE a_ReadData, 
  __in     PVOID pvCallbackContext, 
  __in     ULONG a_ReadLength
)
{
  DWORD dwResult = ERROR_SUCCESS;
  
  _ReadWriteHandShake* pRwHs = (_ReadWriteHandShake*)pvCallbackContext;

  pRwHs->ReadData = a_ReadData;
  pRwHs->ReadLength = a_ReadLength;

  SetEvent(pRwHs->DataAvailable);

  DWORD WaitStatus = WaitForSingleObject(pRwHs->DataReceived, 10000);
  if (WAIT_OBJECT_0 != WaitStatus)
    dwResult = WaitStatus;

  return dwResult;
}

//--------------------------------------------------------------------
//
// CopyEncryptedStream
//
//--------------------------------------------------------------------
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
)
{
  PVOID pReadContext;	
  PVOID pWriteContext;	

  DWORD dwResult = OpenEncryptedFileRaw(lpExistingFileName, 0, &pReadContext);
  if(ERROR_SUCCESS == dwResult)
  {
    ULONG Flags = CREATE_FOR_IMPORT;
    if (dwCopyFlags & COPY_FILE_COPY_DIRECTORY)
      Flags |= CREATE_FOR_DIR;

    dwResult = OpenEncryptedFileRaw(lpNewFileName, Flags, &pWriteContext);

    if(ERROR_SUCCESS == dwResult)
    {
      _ReadWriteHandShake* pRwHs = new _ReadWriteHandShake;
      pRwHs->ReadLength = 0;
      pRwHs->BytesWritten = 0;
      pRwHs->ReadContext = pReadContext;
      pRwHs->DataReceived = CreateEvent (NULL, FALSE, FALSE, NULL);
      pRwHs->DataAvailable = CreateEvent (NULL, FALSE, FALSE, NULL);

      pRwHs->lpProgressRoutine = lpProgressRoutine;
      pRwHs->lpData = lpData;
      pRwHs->TotalFileSize = a_TotalFileSize;

      pRwHs->TotalBytesTransferred.QuadPart = 0;

      DWORD ThreadId;
      HANDLE WaitEvent = CreateThread (
        NULL, 
        STACKSIZE,
        (LPTHREAD_START_ROUTINE) & (ReadEncryptedStream), 
        pRwHs,
        0, 
        &ThreadId
        );

      dwResult = WriteEncryptedFileRaw(&WriteEncryptedStreamCallBack, pRwHs,  pWriteContext);		

      CloseEncryptedFileRaw(pReadContext);	
      CloseEncryptedFileRaw(pWriteContext);	
      CloseHandle(pRwHs->DataReceived);
      CloseHandle(pRwHs->DataAvailable);

      delete pRwHs;
    }
  }

  return dwResult;
}

//--------------------------------------------------------------------
//
// CopySparseStream
//
// Copies a sparse data stream of a file
//
//--------------------------------------------------------------------
int
CopySparseStream( 
  __in     HANDLE             a_ExistingFileHandle,
  __in     HANDLE             a_NewFileHandle,
  __in_opt LPPROGRESS_ROUTINE lpProgressRoutine,
  __in_opt LPVOID             lpData,
  __in_opt LPBOOL             pbCancel,
  __in     LARGE_INTEGER      a_TotalFileSize,
  __inout  LARGE_INTEGER&     a_TotalBytesTransferred,
  __in     DWORD              a_StreamNumber
)
{
  int iResult = ERROR_SUCCESS;

  LARGE_INTEGER StreamTotalSize;
  BOOL bResult = GetFileSizeEx(a_ExistingFileHandle, &StreamTotalSize);
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
      a_ExistingFileHandle, 
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

  LARGE_INTEGER StreamBytesTransferred;
  StreamBytesTransferred.QuadPart = 0;

  DWORD dwTemp;
  DeviceIoControl(a_NewFileHandle, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwTemp, NULL);

  BOOL bCancel = FALSE;

  // Copy all ranges of the file
  //
  for (int range = 0; range < nRanges; ++range)
  {
    if (ERROR_SUCCESS == iResult)
      iResult = CopyFileStreamRange(
        a_ExistingFileHandle,
        a_NewFileHandle,
        lpProgressRoutine,
        lpData,
        pbCancel,
        a_TotalFileSize,
        a_TotalBytesTransferred,
        a_StreamNumber,
        pRanges[range].FileOffset,
        pRanges[range].Length,
        StreamBytesTransferred,
        StreamTotalSize
      );
    else
      break;
  }

  free(pRanges);

  return iResult;
}

//--------------------------------------------------------------------
//
// CopyFileStream
//
// Copies a data stream of a file
//
//--------------------------------------------------------------------
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
)
{
  int iResult = ERROR_SUCCESS;

  // Copy the stream content
  // 
  LARGE_INTEGER StreamTotalSize, StreamBytesTransferred, StreamBegin;
  StreamBytesTransferred.QuadPart = 0;
  BOOL bResult = GetFileSizeEx(a_ExistingFileHandle, &StreamTotalSize);
  if (FALSE == bResult)
    return ERROR_ACCESS_DENIED;

  LARGE_INTEGER StreamOffset;
  StreamOffset.QuadPart = 0;

  // Preallocate data, so that NTFS fragmentation is low
  //
#if 1
  SetFilePointerEx(a_NewFileHandle, StreamTotalSize, NULL, FILE_BEGIN);
  SetEndOfFile(a_NewFileHandle);
  StreamBegin.QuadPart = 0;
  SetFilePointerEx(a_NewFileHandle, StreamBegin, NULL, FILE_BEGIN);
#endif

  return CopyFileStreamRange(
    a_ExistingFileHandle,
    a_NewFileHandle,
    lpProgressRoutine,
    lpData,
    pbCancel,
    a_TotalFileSize,
    a_TotalBytesTransferred,
    a_StreamNumber,
    StreamOffset,
    StreamTotalSize,
    StreamBytesTransferred,
    StreamTotalSize
  );
}

//--------------------------------------------------------------------
//
// CopyFileStreamRange
//
// Copies a data stream of a file
//
//--------------------------------------------------------------------
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
)
{
  int iResult = ERROR_SUCCESS;

  // Save initial Offset
  LARGE_INTEGER      StreamSizeAbsolute;
  StreamSizeAbsolute.QuadPart = a_StreamOffset.QuadPart + a_StreamSize.QuadPart;

  // Check for files with zero byte length
  //
  if (0 == a_StreamSize.QuadPart)
    return iResult;

  // Create FileMapping
  //
  HANDLE hFilemap = CreateFileMapping(
		      a_ExistingFileHandle,
		      NULL,
		      PAGE_READONLY,
          a_StreamTotalSize.HighPart,
          a_StreamTotalSize.LowPart,
          NULL
  );
  if (NULL == hFilemap) 
    return GetLastError();

  enum { COPY_BUFFER_SIZE = 0x1000000 };
  LARGE_INTEGER Count;
  Count.QuadPart = COPY_BUFFER_SIZE;
  
  if (a_StreamSize.QuadPart != a_StreamTotalSize.QuadPart)
  {
    // If we copy a Sparse File we have to skip the empty areas
    LARGE_INTEGER NewStreamOffset;
    SetFilePointerEx(a_NewFileHandle, a_StreamOffset, &NewStreamOffset, FILE_BEGIN);
  }

  bool CopyDataDone = false;
  do 
  {
    // Map file into adress space
    if (a_StreamOffset.QuadPart + Count.QuadPart >  StreamSizeAbsolute.QuadPart)
    {
      Count.QuadPart = StreamSizeAbsolute.QuadPart - a_StreamOffset.QuadPart;
      CopyDataDone = true;
    }

    if (Count.QuadPart != 0)
    {
      LPVOID pMemoryView = MapViewOfFileEx(hFilemap, 
        FILE_MAP_READ, 
        a_StreamOffset.HighPart, 
        a_StreamOffset.LowPart, 
        Count.QuadPart,
        NULL
      );
      if (NULL == pMemoryView) 
      {
		    CloseHandle(hFilemap);
        hFilemap = NULL;
        return ERROR_ACCESS_DENIED;
      }

      DWORD BytesWritten;
      DWORD dwLength;
      __try
      {
        dwLength = *((LPDWORD) pMemoryView);

        BOOL bResult = WriteFile(a_NewFileHandle, pMemoryView, Count.LowPart, &BytesWritten, NULL);
        if ((!bResult || Count.LowPart != BytesWritten) || (TRUE == *pbCancel))
        {
          DWORD LastError = GetLastError();
          UnmapViewOfFile(pMemoryView);
          
          CloseHandle(hFilemap);

          CloseHandle(a_NewFileHandle);
          CloseHandle(a_ExistingFileHandle);

          return LastError;
        }
      }
      __except(GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ?
        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
      {
        DWORD LastError = GetLastError();
        UnmapViewOfFile(pMemoryView);

        CloseHandle(hFilemap);

        CloseHandle(a_NewFileHandle);
        CloseHandle(a_ExistingFileHandle);

        return LastError;
      }

      // Handle the callback methods
      if (lpProgressRoutine)
      {
        a_TotalBytesTransferred.QuadPart += BytesWritten;
        a_StreamBytesTransferred.QuadPart += BytesWritten;

        DWORD ret = lpProgressRoutine(
          a_TotalFileSize,
          a_TotalBytesTransferred,
          a_StreamTotalSize,
          a_StreamBytesTransferred,
          a_StreamNumber,
          CALLBACK_CHUNK_FINISHED,
          a_ExistingFileHandle,
          a_NewFileHandle,
          lpData
        );

        switch (ret)
        {
          case PROGRESS_CANCEL:
            iResult = ERROR_OPERATION_ABORTED;
            // ADV_TBD Delete the file which has been created
          break;

          case PROGRESS_STOP:
            iResult = ERROR_OPERATION_ABORTED;
          break;

        }
      } // if (lpProgressRoutine)

      UnmapViewOfFile(pMemoryView);

      a_StreamOffset.QuadPart += Count.QuadPart;

    }
  } while (!CopyDataDone);

  CloseHandle(hFilemap);

  return iResult;
}
//--------------------------------------------------------------------
//
// CopyFileEx3
//
// Copies all data of a file
//
//--------------------------------------------------------------------
int
CopyFileEx3( 
  __in     LPCWSTR                lpExistingFileName,
  __in     LPCWSTR                lpNewFileName,
  __in_opt LPPROGRESS_ROUTINE     lpProgressRoutine,
  __in_opt LPVOID                 lpData,
  __in_opt LPBOOL                 pbCancel,
  __in     DWORD                  dwCopyFlags,
  __inout  PSECURITY_DESCRIPTOR*   a_pSecDesc,
  __inout  int*                   a_SecDescSize,
  __in     __int64                a_DateTimeTolerance,
  __in     _PathNameStatusList*   a_PathNameStatusList
)
{
  int iResult = ERROR_SUCCESS;

  DWORD ExistingFileDesiredAccess = GENERIC_READ;
  DWORD ExistingFileFlagsAndAttributes = 0;
  if (dwCopyFlags & COPY_FILE_COPY_SACL)
  {
    ExistingFileDesiredAccess |= READ_CONTROL | ACCESS_SYSTEM_SECURITY | GENERIC_READ;
    ExistingFileFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
  }

  if (dwCopyFlags & COPY_FILE_COPY_DIRECTORY)
  {
    ExistingFileDesiredAccess |= FILE_READ_ATTRIBUTES;
    ExistingFileFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
  }

  if (dwCopyFlags & COPY_FILE_COPY_REPARSE_POINT)
    ExistingFileFlagsAndAttributes |= FILE_FLAG_OPEN_REPARSE_POINT;

  HANDLE  ExistingFileHandle = CreateFile(
    lpExistingFileName, 
    ExistingFileDesiredAccess,
    FILE_SHARE_READ, 
    NULL, 
    OPEN_EXISTING, 
    ExistingFileFlagsAndAttributes, 
    NULL
  );

  // Try again if we failed to to logon problems. This turned out to be a showstopper with Windows10
  //
#if 0
  if (INVALID_HANDLE_VALUE == ExistingFileHandle && GetLastError() == ERROR_LOGON_FAILURE )
  {
    wchar_t UNCPath[MAX_PATH];
    GetShareNameFromUNC(lpExistingFileName, UNCPath);
    NETRESOURCE nr;

    // Delete trailing slash
    size_t Len = wcslen(UNCPath);
    if (UNCPath[Len - 1] == '\\')
      UNCPath[Len - 1] = 0x00;

    // Logon to remote drive
    nr.dwType = RESOURCETYPE_ANY;
    nr.lpLocalName = NULL;
    nr.lpRemoteName = UNCPath;
    nr.lpProvider = NULL;

    DWORD dw = WNetAddConnection3(NULL, &nr, NULL, NULL, CONNECT_INTERACTIVE | CONNECT_PROMPT);  

    // Try again to open the file
    if (ERROR_SUCCESS == dw)
      ExistingFileHandle = CreateFile(
        lpExistingFileName, 
        ExistingFileDesiredAccess,
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        ExistingFileFlagsAndAttributes, 
        NULL
        );
  }
#endif

  if (INVALID_HANDLE_VALUE != ExistingFileHandle)
  {
    // Try to retrieve basic attributes
    FILETIME64 ExistingCreationTime, ExistingLastAccessTime, ExistingLastWriteTime;
    DWORD ExistingFileAttributes{ 0 };
    iResult = GetFileBasicInformation(
      ExistingFileHandle, 
      &ExistingFileAttributes, 
      &ExistingCreationTime.FileTime, 
      &ExistingLastAccessTime.FileTime, 
      &ExistingLastWriteTime.FileTime
    );

    // Retrieve Filesize of existing file
    LARGE_INTEGER TotalFileSize;
    BOOL bResult = GetFileSizeEx2(ExistingFileHandle, &TotalFileSize);

    // Prepare flags for opening the destination
    DWORD NewFileDesiredAccess = GENERIC_WRITE;
    DWORD NewFileFlagsAndAttributes = 0;
    if (dwCopyFlags & COPY_FILE_COPY_SACL)
    {
      NewFileDesiredAccess |= WRITE_OWNER | WRITE_DAC | ACCESS_SYSTEM_SECURITY;
      NewFileFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
    }

    if (dwCopyFlags & COPY_FILE_COPY_DIRECTORY)
    {
      NewFileDesiredAccess |= FILE_WRITE_ATTRIBUTES;
      NewFileFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
    }

    if (dwCopyFlags & COPY_FILE_COPY_REPARSE_POINT)
      NewFileFlagsAndAttributes |= FILE_FLAG_OPEN_REPARSE_POINT;

    DWORD NewFileCreationDisposition = OPEN_ALWAYS;
    HANDLE  NewFileHandle = CreateFile(
      lpNewFileName, 
      NewFileDesiredAccess,
      FILE_SHARE_READ, 
      NULL, 
      NewFileCreationDisposition,
      NewFileFlagsAndAttributes, 
      NULL
    );

    if (INVALID_HANDLE_VALUE != NewFileHandle)
    {
      // Handle the COPY_FILE_FAIL_IF_EXISTS option
      if (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS)
      {
        int iResult = GetLastError();
        if (ERROR_ALREADY_EXISTS == iResult)
        {
          // Retrieve Filesize of new but also already existing file
          LARGE_INTEGER TotalNewFileSize;
          bResult = GetFileSizeEx2(NewFileHandle, &TotalNewFileSize);

          // Check if they are the same
          FILETIME64 NewCreationTime, NewLastAccessTime, NewLastWriteTime;
          DWORD NewFileAttributes;
          int NewBasicInfoStatus = GetFileBasicInformation(
            NewFileHandle, 
            &NewFileAttributes, 
            &NewCreationTime.FileTime,
            &NewLastAccessTime.FileTime,
            &NewLastWriteTime.FileTime
          );


          // See if the files are the same. This involves filesize and fileage with in a timetolerance
          if (TotalNewFileSize.QuadPart == TotalFileSize.QuadPart)
          {
            if (_abs64(NewLastWriteTime.l64DateTime - ExistingLastWriteTime.l64DateTime) <= a_DateTimeTolerance ) 
            {
              CloseHandle(ExistingFileHandle);
              CloseHandle(NewFileHandle);

              // The new and exitsing files are within the time tolerance, thus are treated as the same
              
              // But it can happen that just the compression is different, and then we treat the file as 
              // different, and also return the status of the CopyCompression operation
              //
              // Unfortunatley the compression can only be copied by reopening the files, which takes time
              // but it would be great to just pass the handles and and set the compression.
              //
              iResult = CopyCompression(
                lpExistingFileName, 
                lpNewFileName, 
                ExistingFileAttributes, 
                NewFileAttributes
              );
              if (ERROR_SUCCESS == iResult)
                iResult = ERROR_ALREADY_EXISTS;

              SetLastError(iResult);

              return iResult;
            }
          }
          else
            // nope, not the same
            iResult = ERROR_POTENTIAL_FILE_FOUND;
        }
      
        if (ERROR_POTENTIAL_FILE_FOUND == iResult)
        {
          // The file was already there but it defintiley was not the same, so we have
          // to close it and reopen it 
          
          CloseHandle(NewFileHandle);

          NewFileCreationDisposition = CREATE_ALWAYS;
          NewFileHandle = CreateFile(
            lpNewFileName, 
            NewFileDesiredAccess,
            FILE_SHARE_READ,
            NULL, 
            NewFileCreationDisposition,
            NewFileFlagsAndAttributes, 
            NULL
          );
        }
      }
      else // if (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS)
      {
        // Copy over regardless if there or not
      }

      // Copy the main data stream
      //
      LARGE_INTEGER TotalBytesTransferred;
      TotalBytesTransferred.QuadPart = 0;
      
      // Copy Data only from files. Directories do not have a main data stream
      //
      if (!(dwCopyFlags & COPY_FILE_COPY_DIRECTORY))
      {
        if (ExistingFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
          iResult = CopySparseStream(
            ExistingFileHandle, 
            NewFileHandle, 
            lpProgressRoutine, 
            lpData,
            pbCancel,
            TotalFileSize,  
            TotalBytesTransferred,
            0
          );
        else
        {
          if (!(ExistingFileAttributes & FILE_ATTRIBUTE_ENCRYPTED))
            iResult = CopyFileStream(
              ExistingFileHandle, 
              NewFileHandle, 
              lpProgressRoutine, 
              lpData,
              pbCancel,
              TotalFileSize, 
              TotalBytesTransferred,
              0
            );
        }
      }

      // Copy Encrypted Streams for directories and files
      if (ExistingFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
      {
        if (dwCopyFlags & COPY_FILE_COPY_SACL)
        {
          // Copying encrypted streams goes a totally different way than any other Backup
          // So we have to close the handles, and afterwards open them again
          CloseHandle(ExistingFileHandle);
          CloseHandle(NewFileHandle);

          iResult = CopyEncryptedStream(
            lpExistingFileName, 
            lpNewFileName, 
            lpProgressRoutine, 
            lpData,
            pbCancel,
            TotalFileSize, 
            TotalBytesTransferred,
            dwCopyFlags
          );

          // Reopen the Handles
          ExistingFileHandle = CreateFile(
            lpExistingFileName, 
            ExistingFileDesiredAccess,
            FILE_SHARE_READ, 
            NULL, 
            OPEN_EXISTING, 
            ExistingFileFlagsAndAttributes, 
            NULL
          );

          NewFileHandle = CreateFile(
            lpNewFileName, 
            NewFileDesiredAccess,
            FILE_SHARE_READ,
            NULL, 
            NewFileCreationDisposition,
            NewFileFlagsAndAttributes, 
            NULL
          );
        }
        else
        {
          // Problem: Encrypted stream but not in backup mode
          iResult = ERROR_ACCESS_DENIED;
          CloseHandle(NewFileHandle);
          NewFileHandle = INVALID_HANDLE_VALUE;
          if (dwCopyFlags & COPY_FILE_COPY_DIRECTORY)
            RemoveDirectory(lpNewFileName);
          else
            DeleteFile(lpNewFileName);
        }
      }

      // Copy the alternative data streams
      //
      if (!(dwCopyFlags & COPY_FILE_COPY_SKIP_ADS))
        if (ERROR_SUCCESS == iResult)
          iResult = CopyAlternativeStreams(
            lpExistingFileName, 
            ExistingFileHandle, 
            lpNewFileName, 
            lpProgressRoutine, 
            lpData,
            pbCancel,
            TotalFileSize, 
            TotalBytesTransferred,
            dwCopyFlags & COPY_FILE_COPY_DIRECTORY ? 0 : 1,
            a_PathNameStatusList
          );
      
      // Copy EA records 
      //
      if (!(dwCopyFlags & COPY_FILE_COPY_SKIP_EA))
        if (ERROR_SUCCESS == iResult)
        {
          iResult = CopyEaRecords(ExistingFileHandle, NewFileHandle);
          if (ERROR_SUCCESS != iResult)
          {
            PathNameStatus pns(PlusE, &lpNewFileName[PATH_PARSE_SWITCHOFF_SIZE], iResult);
            a_PathNameStatusList->push_back(pns);
          }
        }

      // Copy Security Attributes
      //
      if (dwCopyFlags & COPY_FILE_COPY_SACL)
        if (ERROR_SUCCESS == iResult)
          iResult = CopySecurityAttributes(
            ExistingFileHandle, 
            NewFileHandle, 
            a_pSecDesc,
            a_SecDescSize
          );

      // Copy compression needed?
      if (ExistingFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
      {
        // The compression bit can not be set during copying. The fileHandle during copying
        // has to be closed and opened again to change the compression attribute. What a pitty.
        if (INVALID_HANDLE_VALUE != NewFileHandle)
          CloseHandle(NewFileHandle);

        if (ERROR_SUCCESS == iResult)
        {
          iResult = CopyCompression(
            lpExistingFileName, 
            lpNewFileName, 
            ExistingFileAttributes, 
            0
          );

          // Reopen the NewFile so that we can restore the attributes
          NewFileHandle = CreateFile(
            lpNewFileName, 
            NewFileDesiredAccess | FILE_WRITE_ATTRIBUTES,
            FILE_SHARE_READ,
            NULL, 
            OPEN_ALWAYS, 
            NewFileFlagsAndAttributes, 
            NULL
          );

          // Proceed with the iResult from CopyCompression()
        }
      }


      // Finally copy Normal attributes & timestamps. This has to be done at the end, because 
      // e.g. copying the compression needs write access to the file, and if the destination
      // would be ReadOnly the compression could not be copied
      if (ERROR_SUCCESS == iResult)
      {
        if (!(dwCopyFlags & COPY_FILE_COPY_CREATION_TIME))
          ExistingCreationTime.ul64DateTime = 0;

        if (!(dwCopyFlags & COPY_FILE_COPY_ACCESS_TIME))
          ExistingLastAccessTime.ul64DateTime = 0;

        if (!(dwCopyFlags & COPY_FILE_COPY_WRITE_TIME))
          ExistingLastWriteTime.ul64DateTime = 0;

        if ( (ExistingFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)) == (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY))
        {
          //
          // Retrieving the attribute seems so easy, but when it comes to a root directory or mountpoint
          // we get 0x16, which is directory hidden and system but a nonsense, if you at the very end
          // exactly restore these attributes.
          //
          // We need to find out if it is a volume mount point, and if yes fix the attribute to 
          // FILE_ATTRIBUTE_DIRECTORY
          //
          wchar_t VolumeName[MAX_PATH];
          BOOL IsVolume = GetVolumeNameForVolumeMountPoint(lpExistingFileName, VolumeName, MAX_PATH);

          if (IsVolume)
            ExistingFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        }
        
        iResult = SetFileBasicInformation(NewFileHandle, 
          ExistingFileAttributes, 
          &ExistingCreationTime.FileTime, 
          &ExistingLastAccessTime.FileTime, 
          &ExistingLastWriteTime.FileTime
        );
      }
      else
      {
      }

      if (INVALID_HANDLE_VALUE != NewFileHandle)
        CloseHandle(NewFileHandle);
    }
    else // if (INVALID_HANDLE_VALUE != NewFileHandle) ------------------------------------------------------
    {
      // We failed on opening the NewFile for write, because it was e.g. there and could not be opened
      iResult = GetLastError();

      // But it might turn out that the file in the destination is the same, so there
      // is no need to bail out. Try to open it just for attributes
      DWORD NewFileDesiredAccess = STANDARD_RIGHTS_READ | FILE_READ_ATTRIBUTES | SYNCHRONIZE;
      DWORD NewFileFlagsAndAttributes = FILE_FLAG_BACKUP_SEMANTICS;

      if (dwCopyFlags & COPY_FILE_COPY_REPARSE_POINT)
        NewFileFlagsAndAttributes |= FILE_FLAG_OPEN_REPARSE_POINT;

      HANDLE  NewFileHandle = CreateFile(
        lpNewFileName, 
        NewFileDesiredAccess,
        FILE_SHARE_READ,
        NULL, 
        OPEN_ALWAYS, 
        NewFileFlagsAndAttributes, 
        NULL
        );

      if (INVALID_HANDLE_VALUE != NewFileHandle)
      {
        bool CopyCompressionOnly = false;
        // Check if they are the same
        FILETIME64 NewCreationTime, NewLastAccessTime, NewLastWriteTime;
        DWORD NewFileAttributes;
        int NewBasicInfoStatus = GetFileBasicInformation(
          NewFileHandle, 
          &NewFileAttributes, 
          &NewCreationTime.FileTime,
          &NewLastAccessTime.FileTime,
          &NewLastWriteTime.FileTime
        );

        // Retrieve Filesize of new but also already existing file
        LARGE_INTEGER TotalNewFileSize;
        bResult = GetFileSizeEx2(NewFileHandle, &TotalNewFileSize);

        // See if the files are the same. This involves filesize and file-age with in a timetolerance
        if (TotalNewFileSize.QuadPart == TotalFileSize.QuadPart)
        {
          if (_abs64(NewLastWriteTime.l64DateTime - ExistingLastWriteTime.l64DateTime) <= a_DateTimeTolerance ) 
          {
            // The new and exitsing files are within the time tolerance, thus are treated as the same
            iResult = CopyCompression(
              lpExistingFileName, 
              lpNewFileName, 
              ExistingFileAttributes, 
              NewFileAttributes
            );
            if (ERROR_SUCCESS == iResult)
              iResult = ERROR_ALREADY_EXISTS;

            CopyCompressionOnly = true;
          }
          // else ... give up TBD
        }
        // else ... give up TBD

        // The file could be opened for attribute read access, but not for write.
        // Maybe it was just caused by the attributes
        if (ERROR_ACCESS_DENIED == iResult && (NewFileAttributes & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)) )
        {
          // Close File, but try to open with access to just write to the attributes 
          if (INVALID_HANDLE_VALUE != NewFileHandle)
            CloseHandle(NewFileHandle);

          NewFileHandle = CreateFile(
            lpNewFileName, 
            NewFileDesiredAccess | FILE_WRITE_ATTRIBUTES,
            FILE_SHARE_READ,
            NULL, 
            OPEN_ALWAYS, 
            NewFileFlagsAndAttributes, 
            NULL
          );

          if (INVALID_HANDLE_VALUE != NewFileHandle)
          {
            // Unlock the file attributes
            // 
            FILETIME64 NewCreationTime;
            NewCreationTime.ul64DateTime = 0;
            FILETIME64 NewLastAccessTime;
            NewLastAccessTime.ul64DateTime = 0;
            FILETIME64 NewLastWriteTime;
            NewLastWriteTime.ul64DateTime = 0;

            int AttrResult = SetFileBasicInformation(NewFileHandle, 
              NewFileAttributes & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM), 
              &NewCreationTime.FileTime, 
              &NewLastAccessTime.FileTime, 
              &NewLastWriteTime.FileTime
            );
            if (ERROR_SUCCESS == AttrResult)
            {
              // Check if just the compression status shall be changed
              if (CopyCompressionOnly)
              {
                iResult = CopyCompression(
                  lpExistingFileName, 
                  lpNewFileName, 
                  ExistingFileAttributes, 
                  NewFileAttributes
                );
  //              if (ERROR_SUCCESS == iResult)
  //                iResult = ERROR_ALREADY_EXISTS;

                // Restore attributes
                // Do we really need to copy the attributes and timestamps?
                if ( ExistingFileAttributes != NewFileAttributes || dwCopyFlags & (COPY_FILE_COPY_CREATION_TIME|COPY_FILE_COPY_ACCESS_TIME|COPY_FILE_COPY_WRITE_TIME) )
                {
                  if (!(dwCopyFlags & COPY_FILE_COPY_CREATION_TIME))
                    ExistingCreationTime.ul64DateTime = 0;

                  if (!(dwCopyFlags & COPY_FILE_COPY_ACCESS_TIME))
                    ExistingLastAccessTime.ul64DateTime = 0;

                  if (!(dwCopyFlags & COPY_FILE_COPY_WRITE_TIME))
                    ExistingLastWriteTime.ul64DateTime = 0;

                  SetFileBasicInformation(NewFileHandle, 
                    ExistingFileAttributes, 
                    &ExistingCreationTime.FileTime, 
                    &ExistingLastAccessTime.FileTime, 
                    &ExistingLastWriteTime.FileTime
                  );
                }
                CloseHandle(NewFileHandle);
                NewFileHandle = INVALID_HANDLE_VALUE;
              }
              else // if (CopyCompressionOnly)
              {
                CloseHandle(NewFileHandle);

                // Prepare flags for opening the destination
                DWORD NewFileDesiredAccess = GENERIC_WRITE;
                DWORD NewFileFlagsAndAttributes = 0;
                if (dwCopyFlags & COPY_FILE_COPY_SACL)
                {
                  NewFileDesiredAccess |= WRITE_OWNER | WRITE_DAC | ACCESS_SYSTEM_SECURITY;
                  NewFileFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
                }

                if (dwCopyFlags & COPY_FILE_COPY_DIRECTORY)
                {
                  NewFileDesiredAccess |= FILE_WRITE_ATTRIBUTES;
                  NewFileFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
                }

                if (dwCopyFlags & COPY_FILE_COPY_REPARSE_POINT)
                  NewFileFlagsAndAttributes |= FILE_FLAG_OPEN_REPARSE_POINT;

                // Second try to copy things with changed attributes
                //
                NewFileCreationDisposition = OPEN_ALWAYS;
                NewFileHandle = CreateFile(
                  lpNewFileName, 
                  NewFileDesiredAccess,
                  FILE_SHARE_READ, 
                  NULL, 
                  NewFileCreationDisposition, 
                  NewFileFlagsAndAttributes, 
                  NULL
                );

                if (INVALID_HANDLE_VALUE != NewFileHandle)
                {
                  // Copy the main data stream
                  //
                  LARGE_INTEGER TotalBytesTransferred;
                  TotalBytesTransferred.QuadPart = 0;
                  
                  // Copy Data only from files. Directories do not have a main data stream
                  //
                  if (!(dwCopyFlags & COPY_FILE_COPY_DIRECTORY))
                  {
                    if (ExistingFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
                      iResult = CopySparseStream(
                        ExistingFileHandle, 
                        NewFileHandle, 
                        lpProgressRoutine, 
                        lpData,
                        pbCancel,
                        TotalFileSize, 
                        TotalBytesTransferred,
                        0
                      );
                    else
                    {
                      if (!(ExistingFileAttributes & FILE_ATTRIBUTE_ENCRYPTED))
                        iResult = CopyFileStream(
                          ExistingFileHandle, 
                          NewFileHandle, 
                          lpProgressRoutine, 
                          lpData,
                          pbCancel,
                          TotalFileSize, 
                          TotalBytesTransferred,
                          0
                        );
                    }
                  }


                  if (ExistingFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
                  {
                    if (dwCopyFlags & COPY_FILE_COPY_SACL)
                    {
                      // Copying encrypted streams goes a totally different way than any other Backup
                      // So we have to close the handles, and afterwards open them again
                      CloseHandle(ExistingFileHandle);
                      CloseHandle(NewFileHandle);

                      iResult = CopyEncryptedStream(
                        lpExistingFileName, 
                        lpNewFileName, 
                        lpProgressRoutine, 
                        lpData,
                        pbCancel,
                        TotalFileSize, 
                        TotalBytesTransferred,
                        dwCopyFlags
                      );

                      // Reopen the Handles
                      ExistingFileHandle = CreateFile(
                        lpExistingFileName, 
                        ExistingFileDesiredAccess,
                        FILE_SHARE_READ, 
                        NULL, 
                        OPEN_EXISTING, 
                        ExistingFileFlagsAndAttributes, 
                        NULL
                      );

                      NewFileHandle = CreateFile(
                        lpNewFileName, 
                        NewFileDesiredAccess,
                        FILE_SHARE_READ, 
                        NULL, 
                        NewFileCreationDisposition,
                        NewFileFlagsAndAttributes, 
                        NULL
                      );
                    }
                    else
                    {
                      // Problem: Encrypted stream but not in backup mode
                      iResult = ERROR_ACCESS_DENIED;
                      CloseHandle(NewFileHandle);
                      NewFileHandle = INVALID_HANDLE_VALUE;
                      if (dwCopyFlags & COPY_FILE_COPY_DIRECTORY)
                        RemoveDirectory(lpNewFileName);
                      else
                        DeleteFile(lpNewFileName);
                    }
                  }

                  // Copy the alternative data streams
                  //
                  if (!(dwCopyFlags & COPY_FILE_COPY_SKIP_ADS))
                    if (ERROR_SUCCESS == iResult)
                      iResult = CopyAlternativeStreams(
                        lpExistingFileName, 
                        ExistingFileHandle, 
                        lpNewFileName, 
                        lpProgressRoutine, 
                        lpData,
                        pbCancel,
                        TotalFileSize, 
                        TotalBytesTransferred,
                        dwCopyFlags & COPY_FILE_COPY_DIRECTORY ? 0 : 1,
                        a_PathNameStatusList
                      );
                  
                  // Copy EA records 
                  //
                  if (!(dwCopyFlags & COPY_FILE_COPY_SKIP_EA))
                    if (ERROR_SUCCESS == iResult)
                    {
                      iResult = CopyEaRecords(ExistingFileHandle, NewFileHandle);
                      if (ERROR_SUCCESS != iResult)
                      {
                        PathNameStatus pns(PlusE, &lpNewFileName[PATH_PARSE_SWITCHOFF_SIZE], iResult);
                        a_PathNameStatusList->push_back(pns);
                      }
                    }

                  // Copy Security Attributes
                  //
                  if (dwCopyFlags & COPY_FILE_COPY_SACL)
                    if (ERROR_SUCCESS == iResult)
                      iResult = CopySecurityAttributes(
                        ExistingFileHandle, 
                        NewFileHandle, 
                        a_pSecDesc,
                        a_SecDescSize
                      );

                  // Copy Normal Attributes & timestamps
                  if (ERROR_SUCCESS == iResult)
                  {
                    // Do we really need to copy the attributes and timestamps?
                    if ( ExistingFileAttributes != NewFileAttributes || dwCopyFlags & (COPY_FILE_COPY_CREATION_TIME|COPY_FILE_COPY_ACCESS_TIME|COPY_FILE_COPY_WRITE_TIME) )
                    {
                      if (!(dwCopyFlags & COPY_FILE_COPY_CREATION_TIME))
                        ExistingCreationTime.ul64DateTime = 0;

                      if (!(dwCopyFlags & COPY_FILE_COPY_ACCESS_TIME))
                        ExistingLastAccessTime.ul64DateTime = 0;

                      if (!(dwCopyFlags & COPY_FILE_COPY_WRITE_TIME))
                        ExistingLastWriteTime.ul64DateTime = 0;

                      iResult = SetFileBasicInformation(NewFileHandle, 
                        ExistingFileAttributes, 
                        &ExistingCreationTime.FileTime, 
                        &ExistingLastAccessTime.FileTime, 
                        &ExistingLastWriteTime.FileTime
                      );
                    }
                  }
                
                  CloseHandle(NewFileHandle);
                  NewFileHandle = INVALID_HANDLE_VALUE;

                  iResult = CopyCompression(
                    lpExistingFileName, 
                    lpNewFileName, 
                    ExistingFileAttributes, 
                    NewFileAttributes
                  );
                }

              } // if (INVALID_HANDLE_VALUE != NewFileHandle)
            }
            else // if (ERROR_SUCCESS == AttrResult)
            {
              // We were not able to unlock the attributes, thus we give up with error
              iResult =  AttrResult; 
            }
          }
        } // if (ERROR_ACCESS_DENIED == iResult && (NewFileAttributes & (FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)) )

        if (INVALID_HANDLE_VALUE != NewFileHandle)
          CloseHandle(NewFileHandle);

      } // if (INVALID_HANDLE_VALUE != NewFileHandle)
      
      // If the NewFile was already there, and we can not access, even not for Attribute read,
      // return ERROR_FILE_EXISTS
      if (ERROR_ACCESS_DENIED == iResult)
        iResult = ERROR_FILE_EXISTS;
    }

    if (INVALID_HANDLE_VALUE != ExistingFileHandle)
      CloseHandle(ExistingFileHandle);

  } // if (INVALID_HANDLE_VALUE != ExistingFileHandle)
  else
  {
    // When we copy directories we don't want error messages from the source
    // because this is done earlier.
    if (dwCopyFlags & COPY_FILE_COPY_DIRECTORY)
      iResult = ERROR_SUCCESS;
    else
      iResult = GetLastError();
  }

  SetLastError(iResult);
  return iResult;
}


//--------------------------------------------------------------------
//
// CopyAlternativeStreams
//
// Copies alternative data streams
//
//--------------------------------------------------------------------
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
)
{
  int iResult = ERROR_SUCCESS;
  IO_STATUS_BLOCK iosb;
  NTSTATUS  Status;

  PVOID		pBuffer;
  PFILE_STREAM_INFORMATION pFileStream;
  int			BufferSize = 0;

  do
  {
    BufferSize += 0x400;
    pBuffer = (LPBYTE)malloc(BufferSize);

    Status = NtQueryInformationFile(
      a_ExistingFileHandle,
      &iosb,
      pBuffer,
      BufferSize,
      FileStreamInformation
      );

    if (Status == STATUS_INFO_LENGTH_MISMATCH) 
      // Grow the buffer, which means delete the already allocated buffer
      free(pBuffer);

  } while (Status == STATUS_INFO_LENGTH_MISMATCH);

  // Check if we received stream info
  pFileStream = (PFILE_STREAM_INFORMATION)pBuffer;
  if (NT_SUCCESS(Status) && iosb.Information != 0 && pFileStream->StreamNameLength != 0) 
  {
    // Run through the available filestreams
    //
    wchar_t ExistingFileName[HUGE_PATH];
    wcscpy_s(ExistingFileName, HUGE_PATH, a_ExistingFileName);
    size_t ExistingFileNameLen = wcslen(a_ExistingFileName);

    wchar_t NewFileName[HUGE_PATH];
    wcscpy_s(NewFileName, HUGE_PATH, a_NewFileName);
    size_t NewFileNameLen = wcslen(a_NewFileName);

    DWORD StreamNumber = 0;

    UINT Offset;
    do
    {
      if (StreamNumber >= a_StreamStart)
      {
        memcpy(&NewFileName[NewFileNameLen], pFileStream->StreamName, pFileStream->StreamNameLength );
        NewFileName[pFileStream->StreamNameLength / sizeof(wchar_t) + NewFileNameLen] = 0x00;

        memcpy(&ExistingFileName[ExistingFileNameLen], pFileStream->StreamName, pFileStream->StreamNameLength );
        ExistingFileName[pFileStream->StreamNameLength / sizeof(wchar_t) + ExistingFileNameLen] = 0x00;

        HANDLE  ExistingFileHandle = CreateFile(
          ExistingFileName, 
          FILE_READ_DATA | SYNCHRONIZE,
          FILE_SHARE_READ,
          NULL, 
          OPEN_EXISTING, 
          FILE_FLAG_BACKUP_SEMANTICS, 
          NULL
        );

        if (INVALID_HANDLE_VALUE != ExistingFileHandle)
        {
          HANDLE  NewFileHandle = CreateFile(
            NewFileName, 
            FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE,
            FILE_SHARE_READ, 
            NULL, 
            OPEN_ALWAYS, 
            FILE_FLAG_BACKUP_SEMANTICS, 
            NULL
          );

          if (INVALID_HANDLE_VALUE != NewFileHandle)
          {
            // Retrieve the attributes for alternative streams. Sounds weird, but there
            // can be sparse alternative streams
            DWORD ExistingFileAttributes{ 0 };
            FILETIME64 ExistingCreationTime, ExistingLastAccessTime, ExistingLastWriteTime;
            iResult = GetFileBasicInformation(
              ExistingFileHandle, 
              &ExistingFileAttributes, 
              &ExistingCreationTime.FileTime, 
              &ExistingLastAccessTime.FileTime, 
              &ExistingLastWriteTime.FileTime
            );

            if (ExistingFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)
              iResult = CopySparseStream(
                ExistingFileHandle, 
                NewFileHandle, 
                lpProgressRoutine, 
                lpData, 
                pbCancel,
                a_TotalFileSize, 
                a_TotalBytesTransferred,
                StreamNumber
              );
            else
              iResult = CopyFileStream(
                ExistingFileHandle, 
                NewFileHandle, 
                lpProgressRoutine, 
                lpData, 
                pbCancel,
                a_TotalFileSize, 
                a_TotalBytesTransferred,
                StreamNumber
              );

            CloseHandle(NewFileHandle);
          }
          else
            iResult = GetLastError();

          CloseHandle(ExistingFileHandle);
        }
        else
          iResult = GetLastError();
      }
      if (ERROR_SUCCESS != iResult)
      {
        wchar_t* pSuffix = wcsstr(&NewFileName[PATH_PARSE_SWITCHOFF_SIZE], L":$");
        if (pSuffix)
          *pSuffix = 0x00;

        PathNameStatus pns(PlusT, &NewFileName[PATH_PARSE_SWITCHOFF_SIZE], iResult);
        a_PathNameStatusList->push_back(pns);

        // Once we have reported via PathNameStatus we return ok
        iResult = ERROR_SUCCESS;
      }

      // Advance to next stream
      StreamNumber++;
      Offset = pFileStream->NextEntryOffset;
      pFileStream = (PFILE_STREAM_INFORMATION)((PBYTE)pFileStream + Offset);

    } while (Offset);
  }
  else
  {
    // Do nothing. Files on non NTFS drives do not have streams 
  }

  free(pBuffer);

  return iResult;
}

//--------------------------------------------------------------------
//
// CopyDirectory
//
// Creates directory and copies Alternative streams & EA records from 
// source to target directory
//
//--------------------------------------------------------------------
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
)
{
  BOOL bResult = CreateDirectory(a_NewFileName, NULL);
  int iResult = ERROR_SUCCESS;

  if (bResult)
  {
    BOOL                  bCancel = FALSE;

    iResult = CopyFileEx3(
      a_ExistingFileName, 
      a_NewFileName, 
      NULL, 
      NULL, 
      &bCancel, 
      dwCopyFlags | COPY_FILE_COPY_DIRECTORY, 
      a_pSecDesc, 
      a_SecDescSize,
      0,
      a_PathNameStatusList
    );
  }
  else
//    iResult = ERROR_INVALID_FUNCTION;
    iResult = GetLastError();

  return ERROR_SUCCESS == iResult;
}

//--------------------------------------------------------------------
// 
// CreateDirectoryHierarchy
// 
// Creates a directory hierarchy. Make sure this method
// is called with a non relative path, otherwise it might
// run below the begin of the path and crash
// 
//--------------------------------------------------------------------
int 
CreateDirectoryHierarchy(
	__in     LPWSTR	              aPath,
	__in     size_t		            len
)
{
  int rResult = ERROR_SUCCESS;
  
  DWORD r = GetFileAttributes(aPath);
  if (INVALID_FILE_ATTRIBUTES == r)
  {
    while (aPath[--len] != '\\' && len > 0);
    // Go back until we reach x: or arrive at \\?\Volume{97410ad7-54ec-11e3-97ab-005056c00008}\ 
    // to be honest this piece is ugly, but we exactly have to detect a volume-guid based path, 
    // and heavily rely on expression left to right evaluation shortcut, otherwise we would do lots
    // of wcscmp()
    if ( !(':' == aPath[len - 1] || (len == VOLUME_GUID_LENGTH - 1 && aPath[len - 1] == '}' && !wcsncmp(aPath, PATH_LONG_VOLUME_GUID, PATH_LONG_VOLUME_GUID_SIZE) )) )
    {
      aPath[len] = 0;
      rResult = CreateDirectoryHierarchy(aPath, len);
      aPath[len] = '\\';
    }

    if (ERROR_SUCCESS == rResult)
    {
      BOOL bResult = CreateDirectory(aPath, NULL);	
      if (FALSE == bResult)
        return GetLastError();
      else
        return ERROR_SUCCESS;
    }
  }
  return rResult;
}

//--------------------------------------------------------------------
//
// CopyDirectoryHierarchy
//
// Creates directory hierarchy and copies Alternative streams & 
// EA records from source to target directory
//
// unused
//--------------------------------------------------------------------
int 
CopyDirectoryHierarchy(
	__in     LPWSTR	              a_ExistingFileName,
	__in     LPWSTR	              a_NewFileName,
  __in     size_t		            a_NewFileNameLength,
  __in     DWORD                dwCopyFlags,
  __inout PSECURITY_DESCRIPTOR* a_pSecDesc,
  __inout int*                  a_SecDescSize
)
{
  BOOL bResult = CreateDirectoryHierarchy(a_NewFileName, a_NewFileNameLength);
  int iResult = ERROR_SUCCESS;

  if (bResult)
  {
    BOOL bCancel = FALSE;
    _PathNameStatusList        CopyFileError;
    
    iResult = CopyFileEx3(
      a_ExistingFileName, 
      a_NewFileName, 
      NULL, 
      NULL, 
      &bCancel, 
      dwCopyFlags | COPY_FILE_COPY_DIRECTORY, 
      a_pSecDesc,
      a_SecDescSize,
      0,
      &CopyFileError
    );
  }
  else
    iResult = ERROR_INVALID_FUNCTION;

  return iResult;
}

bool
ContainsRemote(
  __inout _ArgvList & a_PathList
)
{
  for (_ArgvListIterator iter = a_PathList.begin(); iter != a_PathList.end(); ++iter)
  {
    if (iter->DriveType == DRIVE_REMOTE)
      return true;
  }
  return false;
}



//--------------------------------------------------------------------
//
// EnumerateVolumes
//
// Lists all volumes into a_VolumeList
// EA records from source to target directory
//
//--------------------------------------------------------------------
int
EnumerateVolumes(
  __inout   _StringList& a_VolumeList
)
{
  WCHAR  VolumeName[MAX_PATH] = L"";

  //
  //  Enumerate all volumes in the system.
  HANDLE FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));
  if (FindHandle != INVALID_HANDLE_VALUE)
  {
    do 
    {
      a_VolumeList.push_back(VolumeName);
    }
    while (FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName)));
  }

  FindVolumeClose(FindHandle);
  return ERROR_SUCCESS;
}

int 
MemCpy(
	__in     PVOID	              a_Dst,
	__in     PVOID	              a_Src,
	__in     size_t	              a_Size
)
{
  int iResult = ERROR_SUCCESS;

  __try
  {
	  memcpy (a_Dst, a_Src, a_Size);
  }
  __except(GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
  {
    iResult = GetLastError();
  }
  return iResult;
}

DWORD
GetFileNotFoundCacheLifetime(
)
{
  DWORD   Timeout = INVALID_FILE_ATTRIBUTES;
  HKEY    HKLMRegKey;
	DWORD   aSize = sizeof(DWORD);

  DWORD RetVal = ::RegOpenKeyEx(
    HKEY_LOCAL_MACHINE,
    REGPATH_FILE_NOT_FOUND_CACHE_LIFETIME,
    0,
    KEY_READ,
    &HKLMRegKey
    );

  if (ERROR_SUCCESS == RetVal)
  {
    DWORD aType = REG_DWORD;
    RetVal = RegQueryValueEx(
      HKLMRegKey,
      REGKEY_FILE_NOT_FOUND_CACHE_LIFETIME,
      0,
      &aType,
      (LPBYTE)&Timeout,
      &aSize
      );

    RegCloseKey(HKLMRegKey);
  }
  else
    Timeout = 0;
  if (Timeout > 5)
    Timeout = 5;

  return Timeout;
}

__out
HANDLE
FindFirstFileRegExpW(
  __in  LPCWSTR             lpFileName,
  __out LPWIN32_FIND_DATAW  lpFindFileData,
  __in  regex_t*            lpRegExp
)
{
  HANDLE FindFileHandle;
  regmatch_t pmatch[1];
  FindFileHandle = FindFirstFile(lpFileName, lpFindFileData);
  int RegExpMatch = REG_NOMATCH;

  if ( INVALID_HANDLE_VALUE != FindFileHandle )
  {
    do 
    {
      int RegExpMatch = regwexec (lpRegExp, lpFindFileData->cFileName, 1, pmatch, 0);
      if (REG_OK == RegExpMatch)
        break;
    } 
    while (FindNextFile(FindFileHandle, lpFindFileData) > 0);
  }

  if (REG_OK == RegExpMatch)
    return FindFileHandle; 
  else
    return INVALID_HANDLE_VALUE;
}


__out
BOOL
FindNextFileRegExpW(
  __in  HANDLE hFindFile,
  __out LPWIN32_FIND_DATAW lpFindFileData,
  __in  regex_t*            lpRegExp
)
{
  return TRUE;
}

void
FindGlobalRootFromPath(
  wchar_t*    aFullPath
)
{
  // Search through the path until we arrive at the 6th \
  // e.g. \\?\GlOBALROOT\Device\HarddiskVolumeShadowCopy8\ 
  int nSlash = 6;
  size_t nLen = wcslen(aFullPath);
  size_t i;
  for (i = 0; (i < nLen) && (nSlash != 0); ++i)
  {
    if (aFullPath[i] == '\\')
      nSlash--;
  }

  if (nSlash)
    aFullPath[i++] = '\\';

  aFullPath[i] = 0;
}

bool GetShareNameFromUNC(
  const wchar_t* aPath,
  wchar_t* aShare
)
{
  bool b = false;

  // [XP On]
  // XP is not able to return proper Filesysteminfo when you pass
  // a UNC Path name. You have to pass the share, like \\server\share\
  // but not \\server\share\path. To overcome this we have to ask
  // XP to provide us with the share from a given fully qualified UNC 
  // name. Vista could do this
  DWORD dwBufferSize = sizeof(NETRESOURCE);
  LPBYTE lpBuffer;                  // buffer
  NETRESOURCE nr;
  LPTSTR pszSystem = NULL;          // variable-length strings

  //
  // Fill a block of memory with zeroes; then initialize
  // the NETRESOURCE structure. 
  //
  ZeroMemory(&nr, sizeof(nr));

  wchar_t	RemotePath[HUGE_PATH];
  wcscpy_s(RemotePath, HUGE_PATH, aPath);

  nr.dwScope       = RESOURCE_CONNECTED;
  nr.dwType        = RESOURCETYPE_DISK;
  nr.lpRemoteName  = RemotePath;

  //
  // First call the WNetGetResourceInformation function with 
  // memory allocated to hold only a NETRESOURCE structure. This 
  // method can succeed if all the NETRESOURCE pointers are NULL.
  // If the call fails because the buffer is too small, allocate
  // a larger buffer.
  //
  lpBuffer = (LPBYTE) malloc( dwBufferSize );
  if (lpBuffer == NULL) 
    return false;

  DWORD r;
  while ( (r = WNetGetResourceInformation(&nr, lpBuffer, &dwBufferSize, &pszSystem)) == ERROR_MORE_DATA)
  {
    dwBufferSize += sizeof(NETRESOURCE);
    lpBuffer = (LPBYTE) realloc(lpBuffer, dwBufferSize); //-V701
  }

  if (NO_ERROR == r)
  {
    NETRESOURCE *pnr = (NETRESOURCE*)lpBuffer;
    if (pnr->lpRemoteName)
      wcscpy(aShare, pnr->lpRemoteName);
    else
      aShare[0] = 0x00;
    PathAddBackslash(aShare);
    b = true;
  }
  free(lpBuffer);
  // [XP Off]

  return b;
}

BOOL
DeleteSiblingIntl(
  __in     const wchar_t*    a_SrcPath,
  __in     const DWORD       a_FileAttribute
)
{
  BOOL bDeleted = FALSE;

  // First get number of siblings
  BY_HANDLE_FILE_INFORMATION	FileInformation;
  ZeroMemory(&FileInformation, sizeof(BY_HANDLE_FILE_INFORMATION));
  HANDLE	SourceFileHandle = CreateFileW (
    a_SrcPath, 
    FILE_READ_ATTRIBUTES | FILE_READ_EA,
    FILE_SHARE_READ, 
    NULL, 
    OPEN_EXISTING, 
    FILE_ATTRIBUTE_NORMAL,
    NULL
  );
  BOOL	r = FALSE;
  if (INVALID_HANDLE_VALUE != SourceFileHandle)
  {
    r = GetFileInformationByHandle (SourceFileHandle, &FileInformation);
    CloseHandle(SourceFileHandle);
  }

  // Were there siblings at all?
  if (r && FileInformation.nNumberOfLinks > 1)
  {
    // Yes there were siblings
    wchar_t	LinkName[HUGE_PATH + 2];
    DWORD LinkNameLength = HUGE_PATH; 
    wcsncpy_s(LinkName, LinkNameLength, a_SrcPath, PATH_PARSE_SWITCHOFF_SIZE + 2);

    // Try to get at least one sibling
    HANDLE FindHardLinkHandle = FindFirstFileNameW(a_SrcPath, 0, &LinkNameLength, &LinkName[PATH_PARSE_SWITCHOFF_SIZE + 2]);
    if (INVALID_HANDLE_VALUE != FindHardLinkHandle)
    {
      do
      {
        // Search as long as we get a sibling and not the file itself.
        if (wcscmp(LinkName, a_SrcPath))
          break;
        LinkNameLength = HUGE_PATH; 
      } while (FindNextFileNameW(FindHardLinkHandle, &LinkNameLength, &LinkName[PATH_PARSE_SWITCHOFF_SIZE + 2]));
      FindClose(FindHardLinkHandle);

#if defined DEBUG_DS        
      fwprintf (gStdOutFile, L"DSI 01: %s:%08x\n", a_SrcPath, FileInformation.dwFileAttributes);
#endif
      // So delete the file but restore the attributes on a sibling
      SetFileAttributesW(
        a_SrcPath,
        FILE_ATTRIBUTE_NORMAL
      );
      bDeleted = DeleteFile(a_SrcPath);

      // Restore it on the sibling
      SetFileAttributesW(
        LinkName,
        FileInformation.dwFileAttributes
      );
    }
  }
  else
  {
    // This was easy. No siblings. No sideeffect of deleting hardlinks
#if defined DEBUG_DS        
    fwprintf (gStdOutFile, L"DSI 01: %s\n", a_SrcPath);
#endif
    SetFileAttributesW(
      a_SrcPath,
      FILE_ATTRIBUTE_NORMAL
    );
    bDeleted = DeleteFile(a_SrcPath);
  }
  return bDeleted;
}

// Delete a sibling of a hardlink group. This is easy if no attributes are set, then it
// ends in a DeleteFile(). 
// If there is set of deloreanbackups and the oldest gets deleted and changing attribute is necessary, 
// all attributes of a siblings set change, also the attribute of the newest. But this means the newest sibling 
// is now different to the source of the delorean operation, and a copy will be triggered with the next delorean 
// copy. :-/
BOOL
DeleteSibling(
  __in     const wchar_t*    a_SrcPath,
  __in     const DWORD       a_FileAttribute
)
{
  BOOL bDeleted = FALSE;

  // Only change attributes if neccesary, because changing attributes does this for all siblings. 
  if (a_FileAttribute & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
  {
    bDeleted = DeleteSiblingIntl(a_SrcPath, a_FileAttribute);
  }
  else // if (a_FileAttribute & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
  {
    // This is also easy. The attributes were not set
    bDeleted = DeleteFile(a_SrcPath);
#if defined DEBUG_DS        
    fwprintf (gStdOutFile, L"DS 01: %s:%08x\n", a_SrcPath, a_FileAttribute);
#endif

    // Well at a first glance it might look easy. But it turns out that attributes
    // due to Hardlink Attribute Teleportation (HAT) are not available properly.
    if (!bDeleted && ERROR_ACCESS_DENIED == GetLastError())
    {
      bDeleted = DeleteSiblingIntl(a_SrcPath, GetFileAttributes(a_SrcPath));
#if defined DEBUG_DS        
      fwprintf (gStdOutFile, L"DS 02: %s:%08x\n", a_SrcPath, a_FileAttribute);
#endif

      if (!bDeleted)
        wprintf(L"DEL04 '%s, %08x, %08x, %08x\n", a_SrcPath, GetLastError(), a_FileAttribute, GetFileAttributes(a_SrcPath));
    }
  }
  
  return bDeleted;
}
void
DbgOsPrint(
  wchar_t*    aTag
)
{
		wchar_t tmpfile[HUGE_PATH];
		GetTempPath(HUGE_PATH, tmpfile);
		wcscat_s(tmpfile, HUGE_PATH, L"\\OStrace.log");

//    FILE* f = _wfopen(tmpfile, L"wb");
    FILE* f;
    _wfopen_s(&f, tmpfile, L"wb");
    if (f)
    {
      fwprintf(f, L"Tag: %s\n", aTag);
      fwprintf(f, L"OSVersion: %d, %d, %d, %d\n", gVersionInfo.dwMajorVersion, gVersionInfo.dwMinorVersion, gVersionInfo.dwBuildNumber, gVersionInfo.dwPlatformId);

      // OSVersion    
      CModuleVersion ver;
      wchar_t Version[128];
      if (ver.GetFileVersionInfo(_T("HardlinkShellExt.dll")))
      {
        wsprintf(Version, L"LSEVersion %d.%d.%d.%d\n", 
          HIWORD(ver.dwFileVersionMS), 
          LOWORD(ver.dwFileVersionMS), 
          HIWORD(ver.dwFileVersionLS), 
          LOWORD(ver.dwFileVersionLS)
        );
        fwprintf(f, Version); //-V618
      }

      fclose(f);
    }
}


int 
ReadArgsFromFile(
  wchar_t*      aArgFileName,
  _StringList&  aArgumentList
)
{
  FILE* Args;
  _wfopen_s(&Args, aArgFileName, L"rt,ccs=UNICODE");
  wchar_t Argument[HUGE_PATH];
  if (Args)
  {
  	while (fgetws(Argument, HUGE_PATH, Args) != NULL)
    {
      size_t ArgLen = wcslen(Argument);
      // Chop off the last 0x0a
      if (0x0a == Argument[ArgLen - 1])
        Argument[ArgLen - 1] = 0x00;

      // Do not take empty lines from files
      StrTrim(Argument, L" \t");
      if (*Argument)
        aArgumentList.push_back(Argument);
    }
    fclose(Args);
  }
  
  return 1;
}

void
WildCard2RegExp(
  wstring&  aString
)
{
  stringreplace(aString, wstring(L"."), wstring(L"\\."));
  stringreplace(aString, wstring(L"*"), wstring(L".*"));
  stringreplace(aString, wstring(L"?"), wstring(L"."));

  stringreplace(aString, wstring(L"$"), wstring(L"\\$"));
  stringreplace(aString, wstring(L"["), wstring(L"\\["));
  stringreplace(aString, wstring(L"]"), wstring(L"\\]"));
  stringreplace(aString, wstring(L"^"), wstring(L"\\^"));
  stringreplace(aString, wstring(L"+"), wstring(L"\\+"));
  stringreplace(aString, wstring(L"-"), wstring(L"\\-"));
  stringreplace(aString, wstring(L"("), wstring(L"\\("));
  stringreplace(aString, wstring(L")"), wstring(L"\\)"));
  stringreplace(aString, wstring(L"{"), wstring(L"\\{"));
  stringreplace(aString, wstring(L"}"), wstring(L"\\}"));
  stringreplace(aString, wstring(L"~"), wstring(L"\\~"));
}

// How to determine if developer mode is on
// https://stackoverflow.com/questions/41231586/how-to-detect-if-developer-mode-is-active-on-windows-10?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
// 
// and how to enable
// https://www.howtogeek.com/292914/what-is-developer-mode-in-windows-10/
bool 
IsDeveloperModeEnabled() 
{
  HKEY hKey;
  LONG err = RegOpenKeyExW(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock"), 0, KEY_READ, &hKey);
  if (err != ERROR_SUCCESS)
     return false;
  DWORD value = 0;
  DWORD dwordSize = sizeof(DWORD);
  err = RegQueryValueExW(hKey, _T("AllowDevelopmentWithoutDevLicense"), 0, NULL, (LPBYTE)&value, &dwordSize);
  RegCloseKey(hKey);
  if (err != ERROR_SUCCESS)
    return false;
  return value != 0;
}

wchar_t*
ResolveUNCPath(
  wchar_t* aUNCPath,
  wchar_t* aResolvedUNCPath
)
{
  if (PathIsUNC(aUNCPath))
  {
    char ipAdress[MAX_PATH];
    size_t charsConverted;

    // Try to get the hostname portion of the UNC share
    wcstombs_s(&charsConverted, ipAdress, MAX_PATH, &aUNCPath[2], wcslen(&aUNCPath[2]));
    char* path = PathFindNextComponentA(ipAdress);
    if (ipAdress != path)
      *(path - 1) = 0x0;

    hostent* hostent = NULL;
    try
    {
      use_WSA x;

      struct in_addr addr = { 0 };
      addr.s_addr = inet_addr(ipAdress);

      if (addr.s_addr != INADDR_NONE)
        hostent = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
    }
    catch (std::exception const &exc)
    {
      exc;
    }

    // Check if name could be resolved
    if (hostent)
    {
      char serverName[MAX_PATH];
      strcpy_s(serverName, MAX_PATH, hostent->h_name);
      char* dot = strstr(serverName, ".");
      if (dot)
        *dot = 0x0;

      swprintf_s(aResolvedUNCPath, HUGE_PATH, L"\\\\%S\\%S", serverName, path);
    }
  }
  else
  {
    aResolvedUNCPath = nullptr;
  }
  return aResolvedUNCPath;

}


void
MakeAnsiString(
  const wchar_t*  unistring,
  char* ansistring
)
{
  int	s = 0;
  if (unistring)
  {
    while (unistring[s])
    {
      ansistring[s] = (char)unistring[s];
      s++;
    }
  }
  ansistring[s] = '\0';
}

void
stringreplace(wstring& aThis, wstring& src, wstring& dest)
{
  size_t slen = src.size();
  size_t  dlen = dest.size();

  size_t  pos = aThis.find(src, 0);
  while (pos != -1)
  {
    aThis.replace(pos, slen, dest);
    pos = aThis.find(src, pos + dlen);
  }
}

// MapNtStatusToWinError
DWORD
MapNtStatusToWinError(
  NTSTATUS        aNtStatus
)
{
  switch (aNtStatus)
  {
  case STATUS_ACCESS_DENIED:
    return ERROR_ACCESS_DENIED;

  case STATUS_OBJECT_NAME_INVALID:
    return ERROR_INVALID_NAME;
  }
  return aNtStatus;
}
//----------------------------------------------------------------------
//
// _ChangeTokenPrivilege
//
//----------------------------------------------------------------------
BOOL
_ChangeTokenPrivilege(
  __in LPCWSTR             PrivilegeName,
  PrivilegeModification_t  aMode
)
{
  TOKEN_PRIVILEGES tp;
  LUID luid;
  HANDLE	hToken;
  TOKEN_PRIVILEGES tpPrevious;
  DWORD cbPrevious = sizeof(TOKEN_PRIVILEGES);

  //
  // Check if we are allowed to change privileges
  //
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    return FALSE;

  if (!LookupPrivilegeValue(NULL, PrivilegeName, &luid))
  {
    CloseHandle(hToken);
    return FALSE;
  }

  //
  // first pass.  get current privilege setting
  //
  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = 0;

  AdjustTokenPrivileges(
    hToken,
    FALSE,
    &tp,
    sizeof(TOKEN_PRIVILEGES),
    &tpPrevious,
    &cbPrevious
  );

  if (GetLastError() != ERROR_SUCCESS)
  {
    CloseHandle(hToken);
    return FALSE;
  }

  //
  // second pass.  set privilege based on previous setting
  //
  tpPrevious.PrivilegeCount = 1;
  tpPrevious.Privileges[0].Luid = luid;
  switch (aMode)
  {
  case eSetPrivilege:
    tpPrevious.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(
      hToken,
      FALSE,
      &tpPrevious,
      cbPrevious,
      NULL,
      NULL
    );
    break;

  case eClearPrivilege:
    tpPrevious.Privileges[0].Attributes &= ~SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(
      hToken,
      FALSE,
      &tpPrevious,
      cbPrevious,
      NULL,
      NULL
    );
    break;

  case eProbePrivilege:
    if (tpPrevious.Privileges[0].Attributes & SE_PRIVILEGE_ENABLED)
      SetLastError(ERROR_SUCCESS);
    else
      SetLastError(ERROR_PRIVILEGE_NOT_HELD);
    break;

  }

  DWORD r = GetLastError();
  CloseHandle(hToken);
  return r == ERROR_SUCCESS;
}

//----------------------------------------------------------------------
//
// DisableTokenPrivilege
//
// Disables a named privilege
//
//----------------------------------------------------------------------
BOOL
DisableTokenPrivilege(
  __in LPCWSTR PrivilegeName
)
{
  return _ChangeTokenPrivilege(PrivilegeName, eClearPrivilege);
}

//----------------------------------------------------------------------
//
// EnableTokenPrivilege
//
// Enables a named privilege.
//
//----------------------------------------------------------------------
BOOL
EnableTokenPrivilege(
  __in LPCWSTR PrivilegeName
)
{
  return _ChangeTokenPrivilege(PrivilegeName, eSetPrivilege);
}

//----------------------------------------------------------------------
//
// ProbeTokenPrivilege
//
// Probes if a named privilege is set
//
//----------------------------------------------------------------------
BOOL
ProbeTokenPrivilege(
  __in LPCWSTR PrivilegeName
)
{
  return _ChangeTokenPrivilege(PrivilegeName, eProbePrivilege);
}

