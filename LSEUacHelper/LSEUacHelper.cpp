/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */


#include "stdafx.h"

// Cmmon version number
#include "..\HardlinkExtension\resource.h"

#include "ProgressBar.h"
#include "UacReuse.h"

#include "LSEUacHelper.h"

HINSTANCE     g_hInstance = NULL;

int
Usage()
{
	_tprintf (_T ("symlink. symlink helper for Windows 7 elevation\n"));
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
    FILE* LogFile;

    Progressbar aProgressbar;

    // If we are in Backup Mode, the number of files is 0 
    AsyncContext    MirrorContext, CleanContext;
    if (MirrorList.BackupMode())
    {
      // Enumerate all files in the source
      _ArgvList MirrorPathList;
      MirrorList.GetAnchorPath(MirrorPathList);
      MirrorList.FindHardLink(
        MirrorPathList, 
        0,
        &MirrorStatistics, 
        &MirrorPathNameStatusList,
        &MirrorContext
      );

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
      while (!MirrorContext.Wait(125) || !CleanContext.Wait(125))
      {
        if (aProgressbar.Update(MirrorContext, PDM_PREFLIGHT))
        {
          MirrorContext.Cancel();
          CleanContext.Cancel();
          MirrorContext.Wait(INFINITE);
          CleanContext.Wait(INFINITE);
          break;
        }
      } // while (!MirrorContext.Wait(125) && !CleanContext.Wait(125))
      aProgressbar.SetMode(PDM_DEFAULT);

      LogFile = MirrorList.AppendLogging();
      MirrorList.HeadLogging(LogFile);
    }
    else
      LogFile = MirrorList.AppendLogging();


    // Only continue if nobody pressed cancel during FindHardlink
    if ( CleanContext.IsRunning() )
    {
      Effort MirrorEffort;
      MirrorList.Prepare(FileInfoContainer::eSmartMirror, &MirrorStatistics, &MirrorEffort);

      Effort CleanEffort;
      CleanList.Prepare(FileInfoContainer::eSmartClean, &CleanStatistics, &CleanEffort);
      aProgressbar.SetRange(MirrorEffort + CleanEffort);

      LogFile = MirrorList.AppendLogging();
      if (LogFile)
        CleanList.SetOutputFile(LogFile);

      MirrorContext.Reset();
      CleanContext.Reset();

      Effort        Progress;
      _SmartMirror(CleanList, MirrorList, MirrorStatistics, MirrorPathNameStatusList, NULL, Progress, &aProgressbar);
    }

    aProgressbar.RestoreProgressbar(ProgressbarPosition);

    DisposeAsync(MirrorList, CleanList, MirrorStatistics, CleanStatistics);

    GetLocalTime(&MirrorStatistics.m_EndTime);
    MirrorList.EndLogging(LogFile, MirrorPathNameStatusList, MirrorStatistics, FileInfoContainer::eSmartMirror);

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
    FILE* LogFile;

    Progressbar aProgressbar;

    GetLocalTime(&aStats.m_StartTime);

    // If we are in Backup Mode, the number of files is 0 
    if (FileList.BackupMode())
    {
      // Backup Mode. Also find the files elevated in symlink
      ULONG Count = 0;
      AsyncContext    Context;

      _ArgvList TargetPathList;
      FileList.GetAnchorPath(TargetPathList);
      int r = FileList.FindHardLink (
        TargetPathList,
        0,
        &aStats, 
        &PathNameStatusList,
        &Context
      );

      while (!Context.Wait(250))
      {
        if (aProgressbar.Update(Context, PDM_PREFLIGHT))
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          progress = -1;
          break;
        }
      } // while (!Context.Wait(250))
      aProgressbar.SetMode(PDM_DEFAULT);

      LogFile = FileList.AppendLogging();
      FileList.HeadLogging(LogFile);
    }
    else
      LogFile = FileList.AppendLogging();

    // Only continue if nobody pressed cancel
    if ( progress >= 0 )
    {
      Effort MaxProgress;
      FileList.Prepare(aMode, &aStats, &MaxProgress);
      aProgressbar.SetRange(MaxProgress);

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

      while (!Context.Wait(250))
      {
        if (aProgressbar.Update(Context, Context.GetProgress()))
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          break;
        } // if

        // Check if LSE had a progressbar visible. If yes restore the old position
        aProgressbar.RestoreProgressbar(ProgressbarPosition);

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
    int progress = 0;

    Progressbar aProgressbar;

    GetLocalTime(&aStats.m_StartTime);
    FILE* LogFile = FileList.StartLogging(gLSESettings, L"SmartMove");
	  AsyncContext    Context;
	  if (FileList.BackupMode())
    {
      // Backup Mode. Also find the files elevated in symlink
      _ArgvList MoveLocation;
      FileList.GetAnchorPath(MoveLocation);

      int r = FileList.FindHardLink (MoveLocation, 0, &aStats, &PathNameStatusList, &Context);
      FileList.HeadLogging(LogFile);
      
      if (!(gLSESettings.GetFlags() & eForceAbsoluteSymbolicLinks))
        FileList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

      while (!Context.Wait(250))
      {
        if (aProgressbar.Update(Context, PDM_PREFLIGHT))
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          break;
        }
        
      } // while (!Context.Wait(250))
      aProgressbar.SetMode(PDM_DEFAULT);
    }

    // Only continue if nobody pressed cancel
    if ( Context.IsRunning() )
    {
      Effort MaxProgress;
      FileList.Prepare(FileInfoContainer::eSmartMove, &aStats, &MaxProgress);
      aProgressbar.SetRange(MaxProgress);

      AsyncContext  MoveContext;
      FileList.SmartMove (&aStats, 
        &PathNameStatusList, 
        &MoveContext
        );

      while (!MoveContext.Wait(250))
      {
        if (aProgressbar.Update(MoveContext, MoveContext.GetProgress()))
        {
          MoveContext.Cancel();
          MoveContext.Wait(INFINITE);
          break;
        } // if

        // Check if LSE had a progressbar visible. If yes restore the old position
        aProgressbar.RestoreProgressbar(ProgressbarPosition);

      } // while
    } // if (Context.IsRunning())

    GetLocalTime(&aStats.m_EndTime);
    FileList.EndLogging(LogFile, PathNameStatusList, aStats);
  } // Progressbar

  DeletePathNameStatusList(PathNameStatusList);

  // TODO: Warum wird beim SmartMove der Speicher nicht freigegeben? Notwendig ist es nicht, weil der
  // Prozess eh terminiert, aber unsauber ist es

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
	wchar_t *pMsgTxt = GetFormattedMlgMessage(aLSEInstance, gLSESettings.GetLanguageID(), aMessage, aDirectory);

	wchar_t	MsgReason[MAX_PATH];
	LoadStringEx(aLSEInstance, aReason, MsgReason, MAX_PATH, gLSESettings.GetLanguageID());

	wchar_t	MsgCaption[MAX_PATH];
	LoadStringEx(aLSEInstance, aCaption, MsgCaption, MAX_PATH, gLSESettings.GetLanguageID());

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

    Effort CurrentProgress;
    Effort Progress;
    Effort LastProgress;

    CopyStatistics	aStats;
    CopyStatistics	MirrorStats;
    FILE* LogFile = { nullptr };
    
    Progressbar aProgressbar;

    // If we are in Backup Mode, the number of files is 0 
    AsyncContext    Context, MirrorContext;
    if (CloneList.BackupMode())
    {
#pragma region BackupMode 
      ULONG Count = 0;

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

      // Wait for the FindHardLink with DST
      // 
      while (!Context.Wait(50))
      {
        if (aProgressbar.Update(Context, PDM_PREFLIGHT))
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          break;
        }
        
      } // while (!Context.Wait(250))

      LogFile = MirrorList.AppendLogging();
      MirrorList.HeadLogging(LogFile);

      // If we are going for the smartmirror, we also have to wait for the mirror directory scan
      if (Backup0PathList.size())
      {
        while (!MirrorContext.Wait(50))
        {
          // Check for cancel
          if (aProgressbar.HasUserCancelled())
          {
            MirrorContext.Cancel();
            MirrorContext.Wait(INFINITE);
            break;
          } // if

        } // while (!MirrorContext.Wait(250))
      } // if (Backup0PathList.size())

      aProgressbar.SetMode(PDM_DEFAULT);
#pragma endregion
    } 

    // Only continue if Cancel was not pressed
    if (Context.IsRunning())
    {
      Effort CloneEffort;
      CloneList.Prepare(FileInfoContainer::eSmartClone, &aStats, &CloneEffort);
      Effort CleanEffort;
      CloneList.EstimateEffort(FileInfoContainer::eSmartClean, &CleanEffort);
      Effort MirrorEffort;
      MirrorList.Prepare(FileInfoContainer::eSmartMirror, &MirrorStats, &MirrorEffort);

      LogFile = MirrorList.AppendLogging();
      if (LogFile)
        CloneList.SetOutputFile(LogFile);

      HTRACE(L"Progress Start%08x\n", CloneEffort.m_Points + CleanEffort.m_Points + MirrorEffort.m_Points);
      aProgressbar.SetRange(CloneEffort + CleanEffort + MirrorEffort);

      Context.Reset();
      CloneList.SmartClone(&aStats, &ClonePathNameStatusList, &Context);

      // Wait for the Clone to finish
      while (!Context.Wait(250))
      {
        // Progress calculation over multiple operations
        CurrentProgress = Context.GetProgress();
        Progress += CurrentProgress - LastProgress;
        LastProgress = CurrentProgress;

        if (aProgressbar.Update(Context, Progress))
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          break;
        } // if
      } // while

      // Sum up the rest of progress, which happened during wait
      Progress += Context.GetProgress() - LastProgress;
      aProgressbar.SetProgress(Progress);

      // Only continue if Cancel was not pressed
      AsyncContext CleanContext;
      if (Context.IsRunning())
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

        _SmartMirror(CloneList, MirrorList, MirrorStats, MirrorPathNameStatusList, nullptr, Progress, &aProgressbar);
      }
    } // if (Context.IsRunning())

    DisposeAsync(MirrorList, CloneList, MirrorStats, aStats);

    GetLocalTime(&MirrorStats.m_EndTime);
    MirrorList.EndLogging(LogFile, MirrorPathNameStatusList, MirrorStats, FileInfoContainer::eSmartMirror);

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
          g_hInstance
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
  PSECURITY_DESCRIPTOR pSecDesc = nullptr;
  int SecDescSize = 0;

  int iResult = ERROR_SUCCESS;
  wchar_t  JunctionTmpName[HUGE_PATH] = { 0 };
  
  DWORD JunctionAttributes = GetFileAttributes(arg0);

  if (gLSESettings.GetFlags() & eBackupMode)
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

    SecDescSize = sizeof(SECURITY_DESCRIPTOR);
    pSecDesc = (PSECURITY_DESCRIPTOR)malloc(SecDescSize);
  }
  else
  {
    // No Backup Mode, just remove the old junction
    SetFileAttributes(arg0, JunctionAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
    if (RemoveDirectory(arg0))
      iResult = GetLastError();
  }

  if (ERROR_SUCCESS == iResult)
  {
    iResult = CreateJunction(arg0, arg1);

    // In Backup Mode restore the permissions of the junction
    if (gLSESettings.GetFlags() & eBackupMode)
    {
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
        SetFileAttributes(JunctionTmpName, JunctionAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
        if (RemoveDirectory(JunctionTmpName))
          iResult = GetLastError();
      }
      else
      {
        // Rollback
        MoveFile(JunctionTmpName, arg0);
      }
      if (pSecDesc)
        free(pSecDesc);
    }

    // Restore the original Junction attributes
    SetFileAttributes(arg0, JunctionAttributes);
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
  PSECURITY_DESCRIPTOR pSecDesc = nullptr;
  int SecDescSize = 0;

  int iResult = ERROR_SUCCESS;
  wchar_t  SymlinkTmpName[HUGE_PATH] = { 0 };

  DWORD SymLinkAttribute = GetFileAttributes(arg0);

  if (gLSESettings.GetFlags() & eBackupMode)
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

    SecDescSize = sizeof(SECURITY_DESCRIPTOR);
    pSecDesc = (PSECURITY_DESCRIPTOR)malloc(SecDescSize);
  }
  else
  {
    // But in normal mode just delete the original symlink
    SetFileAttributes(arg0, SymLinkAttribute & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
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
      WCHAR ReparseSrcTargetFullPath[HUGE_PATH] = { 0 };
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

  if (gLSESettings.GetFlags() & eBackupMode)
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
    if (pSecDesc)
      free(pSecDesc);
  }

  SetFileAttributes(arg0, SymLinkAttribute);

  return iResult;
}

int 
ReplaceMountPoint(
  wchar_t*  arg1,
  wchar_t*  arg0
)
{
  PSECURITY_DESCRIPTOR pSecDesc = nullptr;
  int SecDescSize = 0;

  int iResult = ERROR_SUCCESS;
  wchar_t  MountPointTmpName[HUGE_PATH] = { 0 };
  DWORD MountpointAttributes = GetFileAttributes(arg0);

  if (gLSESettings.GetFlags() & eBackupMode)
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

    SecDescSize = sizeof(SECURITY_DESCRIPTOR);
    pSecDesc = (PSECURITY_DESCRIPTOR)malloc(SecDescSize);
  }
  else
  {
    SetFileAttributes(arg0, MountpointAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
    iResult = DeleteMountPoint (arg0);
    RemoveDirectory(arg0);
  }

  if (ERROR_SUCCESS == iResult)
  {
	  CreateDirectory(arg0, NULL);
    iResult = CreateMountPoint (arg1, arg0);
  }

  if (gLSESettings.GetFlags() & eBackupMode)
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
    if (pSecDesc)
      free(pSecDesc);
  }
  SetFileAttributes(arg0, MountpointAttributes);

  return iResult;
}

void
RebootExplorer()
{
  int MsgRet = MessageBox(NULL,
    L"To apply your changes Explorer.exe must be restarted\r\n\r\nPress Yes to restart or No to quit",
    L"Caption",
    MB_YESNO | MB_ICONEXCLAMATION);

  if (IDYES == MsgRet)
  {
    wchar_t		szProcessNameUni[32] = EXPLORER;

    ULONG		dPIDSize = 0;
    PUINT_PTR  dPID;
    bool b = NtQueryProcessId(szProcessNameUni, &dPID, &dPIDSize);

    if (b)
    {
      // Kill all proccesses with the given name
      for (ULONG i = 0; i < dPIDSize; i++)
      {
        HANDLE pHandle = OpenProcess(PROCESS_TERMINATE, FALSE, dPID[i]);
        if (INVALID_HANDLE_VALUE != pHandle)
        {
          BOOL b = TerminateProcess(pHandle, 42);
          if (FALSE == b)
            HTRACE(L"RebootExplorer TerminateProcess failed: %d, %08x", dPID[i], GetLastError());
          CloseHandle(pHandle);
        }
        else
        {
          HTRACE(L"RebootExplorer OpenProcess failed: %d, %08x", dPID[i], GetLastError());
        }
      }

      GlobalFree(dPID);
    }
  }
}

#  ifdef __cplusplus
extern "C"
{
#  endif // __cplusplus
	int
	wmain(int argc,
				TCHAR* argv[])
	{
    g_hInstance = LoadLibraryEx( 
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

    gLSESettings.Init();
    if (gLSESettings.GetFlags() & eBackupMode)
    {
      BOOL b = EnableTokenPrivilege(SE_BACKUP_NAME);
      BOOL r = EnableTokenPrivilege(SE_RESTORE_NAME);
      BOOL s = EnableTokenPrivilege(SE_SECURITY_NAME);
      RetVal = GetLastError();
      if ( (FALSE == b) || (FALSE == r) )
      {
	      wchar_t	MsgReason[MAX_PATH];
	      LoadStringEx(g_hInstance, IDS_STRING_BackupPrivilegesNotHeld, MsgReason, MAX_PATH, gLSESettings.GetLanguageID());

	      wchar_t	MsgCaption[MAX_PATH];
	      LoadStringEx(g_hInstance, IDS_STRING_BackupFailed, MsgCaption, MAX_PATH, gLSESettings.GetLanguageID());

	      int mr = MessageBox ( NULL, 
		      MsgReason,
		      MsgCaption,
		      MB_ICONERROR 
        );
	      
      if (g_hInstance)
        FreeLibrary(g_hInstance);
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
    gLSESettings.ReadFileSystems();


		if (argc != 2)
		{
#if defined(_DEBUG)
			fwprintf(f, L"argc == %d\n", argc);
			fwprintf(f, L"argv[0] == '%s'\n", argv[0]);
			fclose(f);
#endif
      if (g_hInstance)
        FreeLibrary(g_hInstance);
			return -1;
		}
		FILE* LseUacHelperArgs;
    _wfopen_s (&LseUacHelperArgs, argv[1], L"rb");
		if (LseUacHelperArgs)
		{
			while (!feof(LseUacHelperArgs)) 
			{
				wchar_t line[HUGE_PATH];
        wchar_t sid[MAX_PATH];
        wchar_t* t = fgetws(line, HUGE_PATH, LseUacHelperArgs);
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

        // read the SID, We need it to read settings from the non UAC user
        fwscanf_s(LseUacHelperArgs, L"%s", sid, _countof(sid));
        gLSESettings.Init(sid);
        // eat rest of line
        fgetws(sid, MAX_PATH, LseUacHelperArgs);
        int bb = gLSESettings.GetFlags();

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

					
          // TODO via Drag and drop kann man nicht Symbolic link replacement machen. Geht einfach nicht
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
            SmartMove(LseUacHelperArgs);
					break;

          // Smart Copy
					case 's':
            SmartXXX(LseUacHelperArgs, FileInfoContainer::eSmartCopy);
					break;

          // Smart Clone
					case 't':
            SmartXXX(LseUacHelperArgs, FileInfoContainer::eSmartClone);
					break;

          // DeloreanCopy
					case 'v':
            DeloreanCopy(LseUacHelperArgs, FileInfoContainer::eSmartClone);
					break;

          // SmartMirror
					case 'w':
            SmartMirror(LseUacHelperArgs);
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

          // Reboot explorer: Used from LSEConfig
          case 'z':
            RebootExplorer();
          break;

        }
			} 
			fclose(LseUacHelperArgs);
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
    if (g_hInstance)
      FreeLibrary(g_hInstance);

    ExitProcess(RetVal);
	}

#ifdef __cplusplus
}
#endif // __cplusplus

