
#include "stdafx.h"

#include "resource.h"
#include "..\HardlinkExtension\resource.h"

#include "..\..\shared\tre-0.8.0\lib\regex.h" 

#include "hardlink_types.h"
#include "..\hardlink\src\mmfobject.h"

#include "AsyncContext.h"
#include "hardlink.h"
#include "HardlinkUtils.h"

#include <ShlObj.h>

#include "multilang.h"
#include "ProgressBar.h"

#include "symlink.h"

int           gColourDepth = 8;
_LSESettings  gLSESettings;
HINSTANCE     gLSEInstance = NULL;

int
Usage()
{
	_tprintf (_T ("symlink. symlink helper for Windows Vista/W7 elevation\n"));
	return 42;
}

int
ElevatedHardlink(
#if defined(_DEBUG)
  FILE*             f,
#endif
  const wchar_t*   arg0,
  const wchar_t*   arg1
)
{
	int RetVal = CreateHardlink(arg0, arg1);
#if defined(_DEBUG)
	fwprintf(f, L"h arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif

  return RetVal;
}

int
SmartMirror(
  FILE*       ArgFile
)
{
  _PathNameStatusList MirrorPathNameStatusList;
  _PathNameStatusList CleanPathNameStatusList;
  int RetVal = ERROR_SUCCESS;

  { // Begin of Progressbar
    HWND hwnd = GetConsoleWindow();

    CoInitialize(NULL);

    FileInfoContainer MirrorList;
    MirrorList.Load(ArgFile);

    FileInfoContainer CleanList;
    CleanList.Load(ArgFile);

    // Load the position of the progress bar
    RECT ProgressbarPosition;
    fwscanf_s(ArgFile, L"%x,%x,%x,%x\n", &ProgressbarPosition.left, &ProgressbarPosition.top, &ProgressbarPosition.right, &ProgressbarPosition.bottom);

    // Walk through all the files
    CopyStatistics	MirrorStatistics;
    CopyStatistics	CleanStatistics;
    int progress = 0;
    const int MaxEndlessProgress = 50;
    FILE* LogFile;

    Progressbar aProgressbar(
      IDS_STRING_ProgressCreatingSmartMirror, 
      IDS_STRING_ProgressCanceling,
      gLSEInstance,
      gLSESettings.LanguageID,
      hwnd,
      MaxEndlessProgress
    );

    // If we are in Backup Mode, the number of files is 0 
    if (MirrorList.BackupMode())
    {
#if defined _DEBUG  
      aProgressbar.SetTitle(L"DEBUG: Symlink::SmartMirror FindHardlink");
#endif
      // Enumerate all files in the source
      AsyncContext    Context;
      _ArgvList MirrorPathList;
      MirrorList.GetAnchorPath(MirrorPathList);
      MirrorList.FindHardLink(
        MirrorPathList, 
        0,
        &MirrorStatistics, 
        &MirrorPathNameStatusList,
        &Context
      );

      AsyncContext    CleanContext;
      _ArgvList CleanPathList;
      CleanList.GetAnchorPath(CleanPathList);
      CleanList.FindHardLink(
        CleanPathList, 
        0,
        &CleanStatistics, 
        &CleanPathNameStatusList,
        &CleanContext
      );

      // In parallel wait for the searches to finish
      wchar_t         CurrentPath[HUGE_PATH];
      CurrentPath[0] = 0x00;
      while (!Context.Wait(125) || !CleanContext.Wait(125))
      {
        Context.GetStatus(CurrentPath);
        aProgressbar.SetProgress (progress++);
        aProgressbar.SetCurrentPath(CurrentPath);
        aProgressbar.Show();
        if (aProgressbar.HasUserCancelled())
        {
          Context.Cancel();
          CleanContext.Cancel();
          Context.Wait(INFINITE);
          CleanContext.Wait(INFINITE);
          progress = -1;
          break;
        }
        
        // This is an endless bar during searching for files
        if (progress >= MaxEndlessProgress)
        {
          aProgressbar.SetRange(MaxEndlessProgress);
          progress = 0;
        }
      } // while (!Context.Wait(125) && !CleanContext.Wait(125))

      LogFile = MirrorList.AppendLogging();
      MirrorList.HeadLogging(LogFile);
    }
    else
      LogFile = MirrorList.AppendLogging();


    // Only continue if nobody pressed cancel
    if ( progress >= 0 )
    {
      __int64 MaxProgressMirror = MirrorList.PrepareSmartCopy(FileInfoContainer::eSmartCopy, &MirrorStatistics);

      // During Clean the cost of a operation is always 1 so we go with eSmartClone
      __int64 MaxProgressClean = CleanList.PrepareSmartCopy(FileInfoContainer::eSmartClone, &CleanStatistics);
      aProgressbar.SetRange(MaxProgressMirror + MaxProgressClean);

      LogFile = MirrorList.AppendLogging();
      if (LogFile)
        CleanList.SetOutputFile(LogFile);

  #if defined _DEBUG
      aProgressbar.SetTitle(L"DEBUG: Symlink::SmartMirror");
  #endif
      AsyncContext  MirrorContext;
      AsyncContext  CleanContext;

      MirrorList.SetLookAsideContainer(&CleanList);
      MirrorList.SmartMirror (&MirrorStatistics, &MirrorPathNameStatusList, &MirrorContext);
      CleanList.SetLookAsideContainer(&MirrorList);
      CleanList.SmartClean (&CleanStatistics, &CleanPathNameStatusList, &CleanContext);


      wchar_t         CurrentPath[HUGE_PATH];
      CurrentPath[0] = 0x00;

      while (!MirrorContext.Wait(125) || !CleanContext.Wait(125))
      {
        MirrorContext.GetStatus(CurrentPath);
        aProgressbar.SetProgress (MirrorContext.Get() + CleanContext.Get());
        aProgressbar.SetCurrentPath(CurrentPath);

        if (aProgressbar.HasUserCancelled())
        {
          MirrorContext.Cancel();
          CleanContext.Cancel();
          MirrorContext.Wait(INFINITE);
          CleanContext.Wait(INFINITE);
          break;
        } // if
        
        // Check if LSE had a progressbar visible. If yes restore the old position
        // but do this only once. 
        if (ProgressbarPosition.left >= 0)
        {
          if (ProgressbarPosition.bottom > 0)
            aProgressbar.SetWindowPos(ProgressbarPosition);
          ProgressbarPosition.left = -1;
          aProgressbar.Show();
        }

      } // while
    }

    // Start releasing data async, because releasing data really takes time
    AsyncContext MirrorDisposeContext;
    AsyncContext CleanDisposeContext;
    
    const int nHandles = 2;
    HANDLE  WaitEvents[nHandles];
    
    MirrorList.Dispose(&MirrorDisposeContext);
    CleanList.Dispose(&CleanDisposeContext);

    WaitEvents[0] = MirrorDisposeContext.m_WaitEvent;
    WaitEvents[1] = CleanDisposeContext.m_WaitEvent;

    // Wait till releasing of resources has finished
    WaitForMultipleObjects(nHandles, WaitEvents, TRUE, INFINITE);

    GetLocalTime(&MirrorStatistics.m_EndTime);
    MirrorList.EndLogging(LogFile, MirrorPathNameStatusList, MirrorStatistics);

  } // Progressbar

  DeletePathNameStatusList(MirrorPathNameStatusList);
  DeletePathNameStatusList(CleanPathNameStatusList);

  CoUninitialize();
  return RetVal;
}

int
SmartXXX(
  FILE*                                     ArgFile,
  FileInfoContainer::CopyReparsePointFlags  aMode
)
{
  _PathNameStatusList PathNameStatusList;
  int RetVal = ERROR_SUCCESS;

  { // Begin of Progressbar
    HWND hwnd = GetConsoleWindow();

    CoInitialize(NULL);

    FileInfoContainer FileList;
    FileList.Load(ArgFile);

    // Load the position of the progress bar
    RECT ProgressbarPosition;
    fwscanf_s(ArgFile, L"%x,%x,%x,%x\n", &ProgressbarPosition.left, &ProgressbarPosition.top, &ProgressbarPosition.right, &ProgressbarPosition.bottom);

    // Walk through all the files
    CopyStatistics	aStats;
    int progress = 0;
    const int MaxEndlessProgress = 50;
    FILE* LogFile;

    Progressbar aProgressbar(
      IDS_STRING_ProgressCreatingSmartCopy, 
      IDS_STRING_ProgressCanceling,
      gLSEInstance,
      gLSESettings.LanguageID,
      hwnd,
      MaxEndlessProgress
    );

    GetLocalTime(&aStats.m_StartTime);

    // If we are in Backup Mode, the number of files is 0 
    if (FileList.BackupMode())
    {
      // Backup Mode. Also find the files elevated in symlink
      ULONG Count = 0;
      AsyncContext    Context;

#if defined _DEBUG  
      aProgressbar.SetTitle(L"DEBUG: Symlink::SmartXXX FindHardLink");
#endif
      _ArgvList TargetPathList;
      FileList.GetAnchorPath(TargetPathList);
      int r = FileList.FindHardLink (
        TargetPathList,
        0,
        &aStats, 
        &PathNameStatusList,
        &Context
      );

      wchar_t         CurrentPath[HUGE_PATH];
      CurrentPath[0] = 0x00;
      while (!Context.Wait(250))
      {
        Context.GetStatus(CurrentPath);
        aProgressbar.SetProgress (progress++);
        aProgressbar.SetCurrentPath(CurrentPath);
        aProgressbar.Show();
        if (aProgressbar.HasUserCancelled())
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          progress = -1;
          break;
        }
        
        // This is an endless bar during searching for files
        if (progress >= MaxEndlessProgress)
        {
          aProgressbar.SetRange(MaxEndlessProgress);
          progress = 0;
        }
      } // while (!Context.Wait(250))

      LogFile = FileList.AppendLogging();
      FileList.HeadLogging(LogFile);
    }
    else
      LogFile = FileList.AppendLogging();

    // Only continue if nobody pressed cancel
    if ( progress >= 0 )
    {
      __int64 MaxProgress = FileList.PrepareSmartCopy(aMode, &aStats);
      aProgressbar.SetRange(MaxProgress);
      HTRACE (L"Progress Start%08x\n", MaxProgress);

  #if defined _DEBUG 
      aProgressbar.SetTitle(L"DEBUG: Symlink::SmartXXX");
  #endif

      AsyncContext  Context;
      switch (aMode)
      {
        case FileInfoContainer::eSmartCopy:
          FileList.SmartCopy (&aStats, &PathNameStatusList, &Context);
        break;
      
        case FileInfoContainer::eSmartClone:
          FileList.SmartClone (&aStats, &PathNameStatusList, &Context);
        break;
      }

      wchar_t         CurrentPath[HUGE_PATH];
      CurrentPath[0] = 0x00;
      while (!Context.Wait(250))
      {
        Context.GetStatus(CurrentPath);
        aProgressbar.SetProgress (Context.Get());
        aProgressbar.SetCurrentPath(CurrentPath);
        
        // Check if LSE had a progressbar visible. If yes restore the old position
        // but do this only once. 
        if (ProgressbarPosition.left >= 0)
        {
          if (ProgressbarPosition.bottom > 0)
            aProgressbar.SetWindowPos(ProgressbarPosition);
          ProgressbarPosition.left = -1;
          aProgressbar.Show();
        }

        if (aProgressbar.HasUserCancelled())
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          break;
        } // if
      } // while
    
      GetLocalTime(&aStats.m_EndTime);
      FileList.EndLogging(LogFile, PathNameStatusList, aStats);
    }
  } // Progressbar

  DeletePathNameStatusList(PathNameStatusList);

  CoUninitialize();
  return RetVal;
}

int
SmartMove(FILE* ArgFile)
{
  _PathNameStatusList PathNameStatusList;
  int RetVal = ERROR_SUCCESS;

  { // Begin of Progressbar
    HWND hwnd = GetConsoleWindow();

    CoInitialize(NULL);

    FileInfoContainer FileList;
    FileList.Load(ArgFile);

    // Load the position of the progress bar
    RECT ProgressbarPosition;
    fwscanf_s(ArgFile, L"%x,%x,%x,%x\n", &ProgressbarPosition.left, &ProgressbarPosition.top, &ProgressbarPosition.right, &ProgressbarPosition.bottom);

    // Walk through all the files
    CopyStatistics	aStats;
    const int MaxEndlessProgress = 50;
    int progress = 0;

    Progressbar aProgressbar(
      IDS_STRING_ProgressSmartMove, 
      IDS_STRING_ProgressCanceling,
      gLSEInstance,
      gLSESettings.LanguageID,
      hwnd,
      MaxEndlessProgress
    );

    GetLocalTime(&aStats.m_StartTime);
    FILE* LogFile = FileList.StartLogging(gLSESettings, L"SmartMove");
    if (FileList.BackupMode())
    {
      // Backup Mode. Also find the files elevated in symlink
      AsyncContext    Context;
      wchar_t         CurrentPath[HUGE_PATH];

      int   RefCount;
      if (gVersionInfo.dwMajorVersion >= 6)
        // With Vista & W7 we also have to search files, because symbolic links are 
        // also reparsepoints
        RefCount = 0;
      else
        // With XP, we know there are no symbolic links, so we can speed up search by
        // only collecting directories/junctions
        RefCount = -1;
      
      _ArgvList MoveLocation;
      FileList.GetAnchorPath(MoveLocation);

#if defined _DEBUG 
      aProgressbar.SetTitle(L"DEBUG: Symlink::SmartMove FindHardlink");
#endif
      int r = FileList.FindHardLink (MoveLocation, RefCount, &aStats, &PathNameStatusList, &Context);
      FileList.HeadLogging(LogFile);
      
      if (!(gLSESettings.Flags & eForceAbsoluteSymbolicLinks))
        FileList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

      while (!Context.Wait(250))
      {
        Context.GetStatus(CurrentPath);
        aProgressbar.SetProgress (progress++);
        aProgressbar.SetCurrentPath(CurrentPath);
        aProgressbar.Show();
        if (aProgressbar.HasUserCancelled())
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          progress = -1;
          break;
        }
        
        // This is an endless bar during searching for files
        if (progress >= MaxEndlessProgress)
        {
          aProgressbar.SetRange(MaxEndlessProgress);
          progress = 0;
        }
      } // while (!Context.Wait(250))
    }

    // Only continue if nobody pressed cancel
    if ( progress >= 0 )
    {
      FileList.PrepareSmartCopy(FileInfoContainer::eSmartClone, &aStats);
      __int64 MaxProgress = FileList.PrepareSmartMove();
      aProgressbar.SetRange(MaxProgress);

  #if defined _DEBUG 
      aProgressbar.SetTitle(L"DEBUG: Symlink::SmartMove");
  #endif

      AsyncContext  Context;
      FileList.SmartMove (&aStats, 
        &PathNameStatusList, 
        &Context
        );

      wchar_t         CurrentPath[HUGE_PATH];
      CurrentPath[0] = 0x00;
      while (!Context.Wait(250))
      {
        Context.GetStatus(CurrentPath);
        aProgressbar.SetProgress (Context.Get());
        aProgressbar.SetCurrentPath(CurrentPath);

        // Check if LSE had a progressbar visible. If yes restore the old position
        // but do this only once. 
        if (ProgressbarPosition.left >= 0)
        {
          if (ProgressbarPosition.bottom > 0)
            aProgressbar.SetWindowPos(ProgressbarPosition);
          ProgressbarPosition.left = -1;
          aProgressbar.Show();
        }

        if (aProgressbar.HasUserCancelled())
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          break;
        } // if
      } // while
    } // if ( progress >= 0 )

    GetLocalTime(&aStats.m_EndTime);
    FileList.EndLogging(LogFile, PathNameStatusList, aStats);
  } // Progressbar

  DeletePathNameStatusList(PathNameStatusList);

  CoUninitialize();
  return RetVal;
}

void
ErrorCreating(
	const wchar_t*	aDirectory,
	int			aMessage,
	int			aCaption,
	int			aReason,
  HINSTANCE aLSEInstance
)
{
	// Assemble message string
	wchar_t *pMsgTxt = GetFormattedMlgMessage(aLSEInstance, gLSESettings.LanguageID, aMessage, aDirectory);

	wchar_t	MsgReason[MAX_PATH];
	LoadStringEx(aLSEInstance, aReason, MsgReason, MAX_PATH, gLSESettings.LanguageID);

	wchar_t	MsgCaption[MAX_PATH];
	LoadStringEx(aLSEInstance, aCaption, MsgCaption, MAX_PATH, gLSESettings.LanguageID);

	// Concat Message + Reason
	wchar_t msg[HUGE_PATH];
	wcscpy_s(msg, HUGE_PATH, pMsgTxt);
	wcscat_s(msg, HUGE_PATH, MsgReason);

	int mr = MessageBox ( NULL, 
		msg,
		MsgCaption,
		MB_ICONERROR );
	LocalFree(pMsgTxt);
}



int
DeloreanCopy(
  FILE*                                     ArgFile,
  FileInfoContainer::CopyReparsePointFlags  aMode
)
{
  _PathNameStatusList MirrorPathNameStatusList;
  _PathNameStatusList ClonePathNameStatusList;

  int RetVal = ERROR_SUCCESS;

  { // Begin of Progressbar
    HWND hwnd = GetConsoleWindow();

    CoInitialize(NULL);

    FileInfoContainer CloneList;
    CloneList.Load(ArgFile);

    // Load the position of the progress bar
    RECT ProgressbarPosition;
    fwscanf_s(ArgFile, L"%x,%x,%x,%x\n", &ProgressbarPosition.left, &ProgressbarPosition.top, &ProgressbarPosition.right, &ProgressbarPosition.bottom);

    FileInfoContainer MirrorList;
    MirrorList.Load(ArgFile);

    wchar_t Backup1[HUGE_PATH];
    fwscanf_s (ArgFile, L"%[^\n]\n", 
      Backup1,
      HUGE_PATH
    );

    wchar_t         CurrentPath[HUGE_PATH];

    __int64 CurrentProgress = 0;
    __int64 Progress = 0;
    __int64 LastProgress = 0;

    CopyStatistics	aStats;
    CopyStatistics	MirrorStats;
    const int MaxEndlessProgress = 50;
    FILE* LogFile;
    
    Progressbar aProgressbar(
      IDS_STRING_ProgressCreatingDeLoreanCopy, 
      IDS_STRING_ProgressCanceling,
      gLSEInstance,
      gLSESettings.LanguageID,
      hwnd,
      MaxEndlessProgress
      );

    // If we are in Backup Mode, the number of files is 0 
    if (CloneList.BackupMode())
    {
      ULONG Count = 0;
      AsyncContext    Context;
      AsyncContext    MirrorContext;

#if defined _DEBUG
      aProgressbar.Show();
      aProgressbar.SetTitle(L"DEBUG: Symlink::FindHardlink DeloreanCopy");
#endif

      // Find the files in Backup0
      _ArgvList Backup0PathList;
      CloneList.GetAnchorPath(Backup0PathList);
      CloneList.FindHardLink (
        Backup0PathList,
        0,
        &aStats,
        &ClonePathNameStatusList,
        &Context
      );

      if (Backup0PathList.size())
      {
        _ArgvList MirrorPathList;
        MirrorList.GetAnchorPath(MirrorPathList);
        MirrorList.FindHardLink (
          MirrorPathList, 
          0, 
          &MirrorStats, 
          &MirrorPathNameStatusList, 
          &MirrorContext
        );
      }

      // First we have to wait for the find on the Clone side
      // 
      while (!Context.Wait(250))
      {
        Context.GetStatus(CurrentPath);
        aProgressbar.SetProgress (Progress++);
        aProgressbar.SetCurrentPath(CurrentPath);
        aProgressbar.Show();
        if (aProgressbar.HasUserCancelled())
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          Progress = -1;
          break;
        }
        
        // This is an endless bar during searching for files
        if (Progress >= MaxEndlessProgress)
        {
          aProgressbar.SetRange(MaxEndlessProgress);
          Progress = 0;
        }
      } // while (!Context.Wait(250))

      LogFile = MirrorList.AppendLogging();
      MirrorList.HeadLogging(LogFile);

      // If we are going for the smartmirror, we also have to wait for 
      // the mirror directory scan
      if (Backup0PathList.size())
      {
        while (!MirrorContext.Wait(250))
        {
          // Check for cancel
          if (aProgressbar.HasUserCancelled())
          {
            MirrorContext.Cancel();
            MirrorContext.Wait(INFINITE);
            Progress = -1;
            break;
          } // if

          // This is an endless bar during searching for files
          if (Progress >= MaxEndlessProgress)
          {
            aProgressbar.SetRange(MaxEndlessProgress);
            Progress = 0;
          }

        } // while (!MirrorContext.Wait(250))
      } // if (Backup0PathList.size())
    }

    // Only continue if Cancel was not pressed
    if (Progress >= 0)
    {
      __int64 MaxProgress = CloneList.PrepareSmartCopy(FileInfoContainer::eSmartClone, &aStats);

      // 1 Clonen
      // 2 Clean 
      // 3 Mirror
      MaxProgress *= 3;

      LogFile = MirrorList.AppendLogging();
      if (LogFile)
        CloneList.SetOutputFile(LogFile);

      HTRACE (L"Progress Start%08x\n", MaxProgress);
      aProgressbar.SetRange(MaxProgress);

  #if defined _DEBUG
      aProgressbar.SetTitle(L"DEBUG: Symlink::DeloreanCopy SmartClone");
  #endif
      AsyncContext  Context;
      CloneList.SmartClone (&aStats, &ClonePathNameStatusList, &Context);

      LastProgress = 0;
      // Wait for the Clone to finish
      CurrentPath[0] = 0x00;
      while (!Context.Wait(250))
      {
        Context.GetStatus(CurrentPath);

        // Progress calculation over multiple operations
        CurrentProgress = Context.Get();
        Progress += CurrentProgress - LastProgress ;
        LastProgress = CurrentProgress;
        aProgressbar.SetProgress (Progress);

        aProgressbar.SetCurrentPath(CurrentPath);
        aProgressbar.Show();
        if (aProgressbar.HasUserCancelled())
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          Progress = -1;
          break;
        } // if
      } // while

#if defined _DEBUG
      aProgressbar.SetTitle(L"DEBUG: Symlink::DeloreanCopy Cleanup");
#endif

      // Only continue if Cancel was not pressed
      if (Progress >= 0)
      {
        // Create a temporary List of anchorpath for ChangePath()
        _ArgvList argvlist;
        _ArgvList& rAnchorPath = MirrorList.m_AnchorPathCache.m_AnchorPaths;
        for (_ArgvListIterator iter = rAnchorPath.begin(); iter != rAnchorPath.end(); ++iter)
        { 
          _ArgvPath argv;
          argv.Argv = iter->Argv;
          argv.ArgvDest = iter->Argv;
          argvlist.push_back(argv);
        }
        
        CloneList.ChangePath(Backup1, argvlist);
        CloneList.SetLookAsideContainer(&MirrorList);
        CloneList.PrepareSmartCopy(FileInfoContainer::eSmartClean, &aStats);
        AsyncContext CleanContext;
        CloneList.SmartClean (&MirrorStats, &MirrorPathNameStatusList, &CleanContext);

        // Wait for the SmartClean to finish
        LastProgress = 0;
        CurrentPath[0] = 0x00;
        while (!CleanContext.Wait(250))
        {
          CleanContext.GetStatus(CurrentPath);

          // Progress calculation over multiple operations
          CurrentProgress = CleanContext.Get();
          Progress += CurrentProgress - LastProgress;
          LastProgress = CurrentProgress;
          aProgressbar.SetProgress (Progress);

          aProgressbar.SetCurrentPath(CurrentPath);
          aProgressbar.Show();
          if (aProgressbar.HasUserCancelled())
          {
            CleanContext.Cancel();
            CleanContext.Wait(INFINITE);
            Progress = -1;
            break;
          } // if
        } // while
      }


      if (Progress >= 0)
      {
#if defined _DEBUG
        aProgressbar.SetTitle(L"DEBUG: Symlink::DeloreanCopy  Mirror");
#endif
        MirrorList.PrepareSmartCopy(FileInfoContainer::eSmartCopy, &MirrorStats);
        MirrorList.SetLookAsideContainer(&CloneList);

        AsyncContext MirrorContext;
        
        MirrorList.SmartMirror (&MirrorStats, &MirrorPathNameStatusList, &MirrorContext);
        LastProgress = 0;
        CurrentPath[0] = 0x00;
        while (!MirrorContext.Wait(250))
        {
          MirrorContext.GetStatus(CurrentPath);

          // Progress calculation over multiple operations
          CurrentProgress = MirrorContext.Get();
          Progress += CurrentProgress - LastProgress;
          LastProgress = CurrentProgress;
          aProgressbar.SetProgress (Progress);

          aProgressbar.SetCurrentPath(CurrentPath);
          aProgressbar.Show();
          if (aProgressbar.HasUserCancelled())
          {
            MirrorContext.Cancel();
            MirrorContext.Wait(INFINITE);
            break;
          } // if
        } // while
      } // if (Progress > 0) 
    } // if (Progress > 0)

    // Start releasing data async, because releasing data really takes time
    const int nHandles = 2;
    HANDLE  WaitEvents[nHandles];
    AsyncContext FileDisposeContext;
    AsyncContext MirrorDisposeContext;
    MirrorList.Dispose(&MirrorDisposeContext);
    CloneList.Dispose(&FileDisposeContext);

    WaitEvents[0] = MirrorDisposeContext.m_WaitEvent;
    WaitEvents[1] = FileDisposeContext.m_WaitEvent;

    // Wait till releasing of resources has finished
    WaitForMultipleObjects(nHandles, WaitEvents, TRUE, INFINITE);

    GetLocalTime(&MirrorStats.m_EndTime);
    MirrorList.EndLogging(LogFile, MirrorPathNameStatusList, MirrorStats);

  } // end of Progress Bar

  if (MirrorPathNameStatusList.size())
  {
    // Dump Errors
    _PathNameStatusList::iterator	iter;
    for (iter = MirrorPathNameStatusList.begin (); iter != MirrorPathNameStatusList.end (); ++iter)
    {
      PathNameStatus pns = *iter;
      if (pns.m_ErrorCode == ERROR_TOO_MANY_LINKS)
      {
        ErrorCreating(pns.m_PathName, 
          IDS_STRING_ErrExplorerCanNotCreateHardlink, 
          IDS_STRING_ErrCreatingHardlink,
          IDS_STRING_ErrHardlinkFailed,
          gLSEInstance
        );
        break;
      }
    }
  } // if (PathNameStatusList.size())

  DeletePathNameStatusList(MirrorPathNameStatusList);
  DeletePathNameStatusList(ClonePathNameStatusList);

  CoUninitialize();

  return RetVal;
}

int 
ReplaceJunction(
  wchar_t*  arg0,
  wchar_t*  arg1
)
{
  SECURITY_DESCRIPTOR SecDesc;
  PSECURITY_DESCRIPTOR pSecDesc = &SecDesc;
  int SecDescSize = sizeof(SECURITY_DESCRIPTOR);
  int iResult = ERROR_SUCCESS;
  wchar_t  JunctionTmpName[HUGE_PATH];

  if (gLSESettings.Flags & eBackupMode)
  {
    SYSTEMTIME  time;
    GetSystemTime(&time);
    
    PathRemoveBackslash(arg0);
    swprintf_s(JunctionTmpName, HUGE_PATH, L"%s%4d%02d%02d%02d%02d%02d%03d", 
      arg0,
      time.wYear,
      time.wMonth,
      time.wDay,
      time.wHour,
      time.wMinute,
      time.wSecond,
      time.wMilliseconds
    );

    if (FALSE == MoveFile(arg0, JunctionTmpName))
      iResult = GetLastError();
  }
  else
  {
    // No Backup Mode, just remove the old junction
    if (RemoveDirectory(arg0))
      iResult = GetLastError();
  }

  if (ERROR_SUCCESS == iResult)
  {
    iResult = CreateJunction(arg0, arg1);

    // In Backup Mode restore the permissions of the junction
    if (gLSESettings.Flags & eBackupMode)
      if (ERROR_SUCCESS == iResult)
      {
        _PathNameStatusList  pns;
        BOOL bCancel = FALSE;
        iResult = CopyFileEx3(JunctionTmpName,
          arg0,
          NULL, 
          NULL,
          &bCancel,
          COPY_FILE_COPY_DIRECTORY | COPY_FILE_COPY_SACL | COPY_FILE_COPY_REPARSE_POINT,
          &pSecDesc,
          &SecDescSize,
          0,
          &pns
        );
        if (RemoveDirectory(JunctionTmpName))
          iResult = GetLastError();
      }
      else
      {
        MoveFile(JunctionTmpName, arg0);
      }
  }

  return iResult;
}

int 
ReplaceSymbolicLink(
  wchar_t*  arg0,
  wchar_t*  arg1,
  DWORD     dwFlags
)
{
  SECURITY_DESCRIPTOR SecDesc;
  PSECURITY_DESCRIPTOR pSecDesc = &SecDesc;
  int SecDescSize = sizeof(SECURITY_DESCRIPTOR);
  int iResult = ERROR_SUCCESS;
  wchar_t  SymlinkTmpName[HUGE_PATH];

  if (gLSESettings.Flags & eBackupMode)
  {
    // temporarilly move the old symlink to some different name, so that 
    // we can copy its attributes to the newly created symlink
    SYSTEMTIME  time;
    GetSystemTime(&time);
    swprintf_s(SymlinkTmpName, HUGE_PATH, L"%s%4d%02d%02d%02d%02d%02d%03d", 
      arg0,
      time.wYear,
      time.wMonth,
      time.wDay,
      time.wHour,
      time.wMinute,
      time.wSecond,
      time.wMilliseconds
    );

    if (FALSE == MoveFile(arg0, SymlinkTmpName))
      iResult = GetLastError();
  }
  else
  {
    // But in normal mode just delete the original symlink
    if (dwFlags & SYMLINK_FLAG_DIRECTORY)
      RemoveDirectory(arg0);
    else
      DeleteFile(arg0);
  }

  WCHAR	SymLinkSource[HUGE_PATH];
  WCHAR	SymLinkTarget[HUGE_PATH];
  if (dwFlags & SYMLINK_FLAG_RELATIVE)
  {
    PWCHAR	FilePart;

    wcscpy_s(SymLinkSource, HUGE_PATH, PATH_PARSE_SWITCHOFF);
    GetFullPathName(arg0, HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &SymLinkSource[PATH_PARSE_SWITCHOFF_SIZE], &FilePart); 

    wcscpy_s(SymLinkTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);
    wcscpy_s(&SymLinkTarget[PATH_PARSE_SWITCHOFF_SIZE], HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, arg1);

    // It might happen, that there are relative symbolic links
    if ( PathIsRelative(&SymLinkTarget[PATH_PARSE_SWITCHOFF_SIZE]) )
    {
      // Resolve the symlinks
      WCHAR ReparseSrcTargetFullPath[HUGE_PATH];
      ResolveSymboliclink(arg0, &SymLinkTarget[PATH_PARSE_SWITCHOFF_SIZE], ReparseSrcTargetFullPath);
      
      // Unfortunately PathCanonicalize() cuts out \\?\ so we have to prepend it
      PathCanonicalize(&SymLinkTarget[PATH_PARSE_SWITCHOFF_SIZE], ReparseSrcTargetFullPath);
    }

    // Here we should end up in any case with SymLinkTarget beeing a long absolute path
  }
  else
  {
	  wcscpy_s(SymLinkSource, HUGE_PATH, arg0);
	  wcscpy_s(SymLinkTarget, HUGE_PATH, arg1);
  }

  // Create the symbolic link
  iResult = CreateSymboliclink (SymLinkSource, SymLinkTarget, dwFlags);  

  if (gLSESettings.Flags & eBackupMode)
  {
    if (ERROR_SUCCESS == iResult)
    {
      _PathNameStatusList  pns;
      BOOL bCancel = FALSE;
      iResult = CopyFileEx3(SymlinkTmpName,
        arg0,
        NULL, 
        NULL,
        &bCancel,
        COPY_FILE_COPY_DIRECTORY | COPY_FILE_COPY_SACL | COPY_FILE_COPY_REPARSE_POINT,
        &pSecDesc,
        &SecDescSize,
        0,
        &pns
      );
      
      if (dwFlags & SYMLINK_FLAG_DIRECTORY)
        RemoveDirectory(SymlinkTmpName);
      else
        DeleteFile(SymlinkTmpName);
    }
    else
    {
      // Rolback
      MoveFile(SymlinkTmpName, arg0);
    }
  }

  return iResult;
}

int 
ReplaceMountPoint(
  wchar_t*  arg1,
  wchar_t*  arg0
)
{
  SECURITY_DESCRIPTOR SecDesc;
  PSECURITY_DESCRIPTOR pSecDesc = &SecDesc;
  int SecDescSize = sizeof(SECURITY_DESCRIPTOR);
  int iResult = ERROR_SUCCESS;
  wchar_t  MountPointTmpName[HUGE_PATH];

  if (gLSESettings.Flags & eBackupMode)
  {
    // temporarilly move the old symlink to some different name, so that 
    // we can copy its attributes to the newly created symlink
    SYSTEMTIME  time;
    GetSystemTime(&time);
    swprintf_s(MountPointTmpName, HUGE_PATH, L"%s%4d%02d%02d%02d%02d%02d%03d", 
      arg0,
      time.wYear,
      time.wMonth,
      time.wDay,
      time.wHour,
      time.wMinute,
      time.wSecond,
      time.wMilliseconds
    );

    if (FALSE == MoveFile(arg0, MountPointTmpName))
      iResult = GetLastError();
  }
  else
  {
	  iResult = DeleteMountPoint (arg0);
    RemoveDirectory(arg0);
  }

  if (ERROR_SUCCESS == iResult)
  {
	  CreateDirectory(arg0, NULL);
    iResult = CreateMountPoint (arg1, arg0);
  }

  if (gLSESettings.Flags & eBackupMode)
  {
    if (ERROR_SUCCESS == iResult)
    {
      _PathNameStatusList  pns;
      BOOL bCancel = FALSE;
      iResult = CopyFileEx3(MountPointTmpName,
        arg0,
        NULL, 
        NULL,
        &bCancel,
        COPY_FILE_COPY_DIRECTORY | COPY_FILE_COPY_SACL | COPY_FILE_COPY_REPARSE_POINT,
        &pSecDesc,
        &SecDescSize,
        0,
        &pns
      );
      
      // DeleteMountPointName is only for XP
      DeleteMountPoint(MountPointTmpName);
      RemoveDirectory(MountPointTmpName);
    }
    else
    {
      // Rolback
      MoveFile(MountPointTmpName, arg0);
    }
  }
  return iResult;
}

#  ifdef __cplusplus
extern "C"
{
#  endif // __cplusplus
	int
	wmain(int argc,
				TCHAR* argv[])
	{
    // This needed because we use a non-msvcrt heap, which places the chunks so close
    // towards each other, that the crt-dbg would use its CRT secure fill pattern, and thus would
    // destroy memory during wcscpy_s() with _FILL_STRING
    _CrtSetDebugFillThreshold(0);

    // Get the Colordepth so that we load the proper progress bar animation
    HDC hDC = GetDC(0);
    gColourDepth = GetDeviceCaps(hDC, BITSPIXEL);
    ReleaseDC(0, hDC);

    GetLSESettings(gLSESettings);

    gLSEInstance = LoadLibraryEx( 
      L"HardlinkShellExt.dll\0",
      NULL, 
      DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE 
    );

    bool	recursive = false;
		ULONG	LinkCount = 0;
		int RetVal = ERROR_INVALID_FUNCTION;

#if defined(_DEBUG)
		char tmpfile[HUGE_PATH];
		GetTempPathA(HUGE_PATH, tmpfile);
		strcat_s(tmpfile, HUGE_PATH, "\\trace.log");
		FILE *f;
    fopen_s(&f, tmpfile, "w");
#endif

    if (gLSESettings.Flags & eBackupMode)
    {
      BOOL b = EnableTokenPrivilege(SE_BACKUP_NAME);
      BOOL r = EnableTokenPrivilege(SE_RESTORE_NAME);
			RetVal = GetLastError();
      if ( (FALSE == b) || (FALSE == r) )
      {
	      wchar_t	MsgReason[MAX_PATH];
	      LoadStringEx(gLSEInstance, IDS_STRING_BackupPrivilegesNotHeld, MsgReason, MAX_PATH, gLSESettings.LanguageID);

	      wchar_t	MsgCaption[MAX_PATH];
	      LoadStringEx(gLSEInstance, IDS_STRING_BackupFailed, MsgCaption, MAX_PATH, gLSESettings.LanguageID);

	      int mr = MessageBox ( NULL, 
		      MsgReason,
		      MsgCaption,
		      MB_ICONERROR 
        );
	      
      if (gLSEInstance)
        FreeLibrary(gLSEInstance);
			return -1;

#if defined(_DEBUG)
        fwprintf(f, L"BackupMode failed %08x\n", RetVal);
#endif
      }
#if defined(_DEBUG)
      else
      {
        fwprintf(f, L"backup Mode successfull %08x\n", RetVal);
      }
#endif
    }

		InitCreateHardlink();

		if (argc != 2)
		{
#if defined(_DEBUG)
			fwprintf(f, L"argc == %d\n", argc);
			fwprintf(f, L"argv[0] == '%s'\n", argv[0]);
			fclose(f);
#endif
      if (gLSEInstance)
        FreeLibrary(gLSEInstance);
			return -1;
		}
		FILE* SymlinkArgs;
    _wfopen_s (&SymlinkArgs, argv[1], L"rb");
		if (SymlinkArgs)
		{
			while (!feof(SymlinkArgs)) 
			{
				wchar_t line[HUGE_PATH];
				wchar_t* t = fgetws(line, HUGE_PATH, SymlinkArgs);
				if (!t)
					break;

				size_t LineLen = wcslen(line);

				int ii = 4;
				wchar_t* arg0 = &line[ii];
				while ( line[++ii] != '\"');
				line[ii] = 0x00;
				ii += 3;
				wchar_t* arg1 = &line[ii];
				while ( line[++ii] != '\"');
				line[ii] = 0x00;

				switch (line[1])
				{
					// Create symbolic link files relative
					case 'f':
					{
            WCHAR	SymLinkSource[HUGE_PATH];
            WCHAR	SymLinkTarget[HUGE_PATH];
	          PWCHAR	FilePart;

            wcscpy_s(SymLinkSource, HUGE_PATH, PATH_PARSE_SWITCHOFF);
	          GetFullPathName(arg0, HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &SymLinkSource[PATH_PARSE_SWITCHOFF_SIZE], &FilePart); 

            wcscpy_s(SymLinkTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);
	          GetFullPathName(arg1, HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &SymLinkTarget[PATH_PARSE_SWITCHOFF_SIZE], &FilePart); 

						RetVal = CreateSymboliclink (SymLinkSource, SymLinkTarget, SYMLINK_FLAG_RELATIVE | SYMLINK_FLAG_FILE);
#if defined(_DEBUG)
						fwprintf(f, L"f arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Create symbolic link files absolute
					case 'F':
					{
						RetVal = CreateSymboliclink (arg0, arg1, SYMLINK_FLAG_FILE);
#if defined(_DEBUG)
						fwprintf(f, L"F arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Create symbolic link directories relative
					case 'd':
					{
            WCHAR	SymLinkSource[HUGE_PATH];
            WCHAR	SymLinkTarget[HUGE_PATH];
	          PWCHAR	FilePart;

            wcscpy_s(SymLinkSource, HUGE_PATH, PATH_PARSE_SWITCHOFF);
	          GetFullPathName(arg0, HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &SymLinkSource[PATH_PARSE_SWITCHOFF_SIZE], &FilePart); 

            wcscpy_s(SymLinkTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);
	          GetFullPathName(arg1, HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &SymLinkTarget[PATH_PARSE_SWITCHOFF_SIZE], &FilePart); 

            RetVal = CreateSymboliclink (SymLinkSource, SymLinkTarget, SYMLINK_FLAG_RELATIVE | SYMLINK_FLAG_DIRECTORY);
#if defined(_DEBUG)
						fwprintf(f, L"d arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Symbolic Link directories absolute
					case 'D':
					{
						RetVal = CreateSymboliclink (arg0, arg1, SYMLINK_FLAG_DIRECTORY);
#if defined(_DEBUG)
						fwprintf(f, L"D arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Symbolic Link directories relative replacement
					case 'e':
					{
            RetVal = ReplaceSymbolicLink(arg0, arg1, SYMLINK_FLAG_RELATIVE | SYMLINK_FLAG_DIRECTORY);
#if defined(_DEBUG)
						fwprintf(f, L"e arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Symbolic Link directories absolute replacement
					case 'E':
					{
            RetVal = ReplaceSymbolicLink(arg0, arg1, SYMLINK_FLAG_DIRECTORY);
#if defined(_DEBUG)
						fwprintf(f, L"E arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Symbolic Link files relative replacement
					case 'g':
					{
            RetVal = ReplaceSymbolicLink(arg0, arg1, SYMLINK_FLAG_RELATIVE | SYMLINK_FLAG_FILE);
#if defined(_DEBUG)
						fwprintf(f, L"g arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Symbolic Link files absolute replacement
					case 'G':
					{
            RetVal = ReplaceSymbolicLink(arg0, arg1, SYMLINK_FLAG_FILE);
#if defined(_DEBUG)
						fwprintf(f, L"G arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Mountpoint Creation
					case 'm':
					{
						RetVal = CreateMountPoint (arg0, arg1);
#if defined(_DEBUG)
						fwprintf(f, L"m arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Mountpoint Deletion
					case 'n':
					{
						// Vista & W7 Mountpoints must be removed elevated, and in directories
						// like c:\Program Files also the directory must be removed elevated
						RetVal = DeleteMountPoint (arg0);
						BOOL b = RemoveDirectory(arg0);
						if (ERROR_SUCCESS == RetVal && !b)
							RetVal = GetLastError();

#if defined(_DEBUG)
						fwprintf(f, L"n arg0: '%s' RetVal: %d, GLE: %d\n", arg0, RetVal, GetLastError());
#endif
					}
					break;

					// Mountpoint Replacement
					case 'o':
					{
						// Vista & W7 Mountpoints must be removed elevated, and in directories
						// like c:\Program Files also the directory must be removed elevated
            RetVal = ReplaceMountPoint(arg0, arg1);
#if defined(_DEBUG)
						fwprintf(f, L"n arg0: '%s' RetVal: %d, GLE: %d\n", arg0, RetVal, GetLastError());
#endif
					}
					break;

					// Junction creation under e.g. c:\Program Files (x86)
					case 'j':
					{
						RetVal = CreateJunction(arg0, arg1);
#if defined(_DEBUG)
						fwprintf(f, L"j arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					}
					break;

					// Junction deletion under e.g. c:\Program Files (x86)
					case 'k':
					{
						BOOL b = RemoveDirectory(arg0);
						if (!b)
							RetVal = GetLastError();
						else
							RetVal = ERROR_SUCCESS;

#if defined(_DEBUG)
						fwprintf(f, L"k arg0: '%s' RetVal: %d, GLE: %d\n", arg0, RetVal, GetLastError());
#endif
					}
					break;

					// Junction Replacement under e.g. c:\Program Files (x86)
					case 'l':
            RetVal = ReplaceJunction(arg0, arg1);
#if defined(_DEBUG)
						fwprintf(f, L"l arg0: '%s' arg1'%s' RetVal: %d, GLE: %d\n", arg0, arg1, RetVal, GetLastError());
#endif
					break;

          // Smart Move
					case 'r': 
            SmartMove(SymlinkArgs);
					break;

          // Smart Copy
					case 's':
            SmartXXX(SymlinkArgs, FileInfoContainer::eSmartCopy);
					break;

          // Smart Clone
					case 't':
            SmartXXX(SymlinkArgs, FileInfoContainer::eSmartClone);
					break;

          // Smart Mirror
					case 'u':
            SmartXXX(SymlinkArgs, FileInfoContainer::eSmartClone);
					break;

          // DeloreanCopy
					case 'v':
            DeloreanCopy(SymlinkArgs, FileInfoContainer::eSmartClone);
					break;

          // SmartMirror
					case 'w':
            SmartMirror(SymlinkArgs);
          break;

          // Hardlink elevation in system directories
					case 'h':
#if defined(_DEBUG)
            RetVal = ElevatedHardlink(f, arg0, arg1);
#else
            RetVal = ElevatedHardlink(arg0, arg1);
#endif
          break;

          // Hardlink elevation in system directories
					case 'H':
            
          break;

        }
			} 
			fclose(SymlinkArgs);
		}
		else
		{
#if defined(_DEBUG)
			fwprintf(f, L"failed to open '%s'\n", argv[1]);
#endif
		}

#if defined(_DEBUG)
		fclose(f);
#endif
    if (gLSEInstance)
      FreeLibrary(gLSEInstance);

    ExitProcess(RetVal);
	}

#ifdef __cplusplus
}
#endif // __cplusplus

