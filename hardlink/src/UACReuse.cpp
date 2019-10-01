#include "stdafx.h"

#include "hardlink_types.h"
#include "DbgHelpers.h"

// TODO include Murks
#include "LSESettings.h"

#include "MmfObject.h"
#include "AsyncContext.h"
#include "hardlinks.h"
#include "Progressbar.h"

#include "UACReuse.h"

void
_SmartMirror(
  FileInfoContainer&    CloneList,
  FileInfoContainer&    MirrorList,
  CopyStatistics&       MirrorStats,
  _PathNameStatusList&  MirrorPathNameStatusList,
  AsyncContext*         a_pMirrorContext,
  Effort&               Progress,
  Progressbar*          pProgressbar
)
{
  // The concurrency during delorean works like this
  //           Find Src
  // o----------------------------------->
  //      Find Dst                       |
  //  o------------>                     |
  //               |  Clone Bkp          |
  //               o------------->       |   Clean Bkp
  //               +----Gain-----+       o-------------->
  //                                                    |    Mirror Src
  //                                                    o------------------>
  //
  // This has the advantage, that a search on the source might take longer for whatever reason
  // and the extra time can be used by a Find on the Dst and Clone to Bkp. So we have a +gain+
  // On the other hand, if things go not that well, nothing is lost:
  //           Find Src
  // o------------------->               |
  //      Find Dst                       |
  //  o------------>                     |
  //               |  Clone Bkp          |
  //               o-------------------->|   Clean Bkp
  //                                     o-------------->
  //                                                    |    Mirror Src
  //                                                    o------------------>
  //

  Effort LastProgress;
  Effort CurrentProgress;

  CloneList.SetLookAsideContainer(&MirrorList);
  AsyncContext CleanContext;
  CloneList.SmartClean(&MirrorStats, &MirrorPathNameStatusList, &CleanContext);

  // Wait for the SmartClean to finish
  while (!CleanContext.Wait(250))
  {
    // Progress calculation over multiple operations
    CurrentProgress = CleanContext.GetProgress();
    Progress += CurrentProgress - LastProgress;
    LastProgress = CurrentProgress;

    if (pProgressbar->Update(CleanContext, Progress))
    {
      CleanContext.Cancel();
      CleanContext.Wait(INFINITE);
      break;
    }
  }

  // Sum up the rest of progress, which happened during wait
  Progress += CleanContext.GetProgress() - LastProgress;
  pProgressbar->SetProgress(Progress);

  // Check if Cancel was pressed during cleaning
  if (CleanContext.IsRunning())
  {
    // The Find operation on the source is still running so check if it's done, but only if we are called
    // from delorean. Mirror does not have this shrink wrapped concurrency model.
    if (a_pMirrorContext)
    {
      // Only take this path if we are in delorean mode. Mirror does not need this
      while (!a_pMirrorContext->Wait(250))
      {
        if (pProgressbar->HasUserCancelled())
        {
          // Finding Files on the source is not part of the progress because it is running in parallel to the clone
          a_pMirrorContext->Cancel();
          a_pMirrorContext->Wait(INFINITE);
          break;
        } // if
      }
      // Check if someone pressed cancel
      if (!a_pMirrorContext->IsRunning())
      {
        // Leave operation 

        // Sum up the rest of progress, which happened during wait
        Progress += a_pMirrorContext->GetProgress() - LastProgress;
        pProgressbar->SetProgress(Progress);

        a_pMirrorContext->Cancel();
        a_pMirrorContext->Wait(INFINITE);

        return;
      }
    }

    // Mirror Src to Bkp
    AsyncContext MirrorContext;
    if (!a_pMirrorContext)
      a_pMirrorContext = &MirrorContext;

    MirrorList.SetLookAsideContainer(&CloneList);
    a_pMirrorContext->Reset();
    MirrorList.SmartMirror(&MirrorStats, &MirrorPathNameStatusList, a_pMirrorContext);

    LastProgress.Reset();
    while (!a_pMirrorContext->Wait(250))
    {
      // Progress calculation 
      CurrentProgress = a_pMirrorContext->GetProgress();
      Progress += CurrentProgress - LastProgress;
      LastProgress = CurrentProgress;

      if (pProgressbar->Update(MirrorContext, Progress))
      {
        a_pMirrorContext->Cancel();
        a_pMirrorContext->Wait(INFINITE);
        break;
      }
    }

    // Sum up the rest of progress, which happened during wait
    Progress += a_pMirrorContext->GetProgress() - LastProgress;
    pProgressbar->SetProgress(Progress);

  }
  else
  {
    // Cancel was pressed during clean, so wait for the mirror file scan to finish
    if (a_pMirrorContext)
    {
      a_pMirrorContext->Cancel();
      a_pMirrorContext->Wait(INFINITE);
    }
  }
}

// TODO Use DisposeAsync from many places

void
DisposeAsync(
  FileInfoContainer&    List1,
  FileInfoContainer&    List2,
  CopyStatistics&       Stats1,
  CopyStatistics&       Stats2
)
{
  // Release data async, because releasing data really takes time
  AsyncContext DisposeContext1;
  AsyncContext DisposeContext2;
  const int nHandles = 2;
  HANDLE  WaitEvents[nHandles];

  List1.Dispose(&DisposeContext1, &Stats1);
  List2.Dispose(&DisposeContext2, &Stats2);

  WaitEvents[0] = DisposeContext1.m_WaitEvent;
  WaitEvents[1] = DisposeContext2.m_WaitEvent;

  // Wait till releasing of resources has finished
  WaitForMultipleObjects(nHandles, WaitEvents, TRUE, INFINITE);
}

extern HINSTANCE g_hInstance;

void 
UACHelper::Open(
)
{
  if (!m_UacArgs)
  {
    // Get the current path of extension installation
    m_SlaQuoted = '\"';

    wchar_t currentDir[MAX_PATH];
    GetTempPath(MAX_PATH, currentDir);
    m_CurrentDir = currentDir;

    m_SlaQuoted += m_CurrentDir + UACHELPERARGS;

    // Write the file for the args
#if defined _DEBUG
    DeleteFile(&m_SlaQuoted.c_str()[1]);
#endif

    _wfopen_s(&m_UacArgs, &m_SlaQuoted.c_str()[1], L"wb");
  }
}

int 
UACHelper::Close()
{ 
    int retVal = -1;
    if (m_UacArgs)
    {
      retVal = fclose(m_UacArgs);
      m_UacArgs = nullptr; 
    }
    return retVal;
};

int
UACHelper::Fork(
)
{
  Close();
  
  // TODO: Das Übergabefile sollte nicht nur einen Namen haben, weil auch mehrere Instanzen der LSE laufen können
  wchar_t uachelper[MAX_PATH];
  GetModuleFileNameW(g_hInstance, uachelper, MAX_PATH);
  PathRemoveFileSpec(uachelper);

  wcscat_s(uachelper, MAX_PATH, L"\\LSEUacHelper.exe");
  m_SlaQuoted += L"\"";

  // Start the process
  SHELLEXECUTEINFO se;
  ZeroMemory(&se, sizeof(se));

  se.cbSize = sizeof(SHELLEXECUTEINFO);
  se.fMask = SEE_MASK_NOCLOSEPROCESS;
  se.lpVerb = NULL;
  se.lpFile = uachelper;
  se.lpParameters = m_SlaQuoted.c_str();
  se.lpDirectory = m_CurrentDir.c_str();
  se.nShow = SW_HIDE;
  ShellExecuteEx(&se);

  // Wait for the process to finish
  WaitForSingleObject(se.hProcess, INFINITE);
  DWORD	ExitCode;
  GetExitCodeProcess(se.hProcess, &ExitCode);
  CloseHandle(se.hProcess);

#if !defined(_DEBUG)
  // Delete the tmp files afterwards

#if !defined DEBUG_DO_NOT_DELETE_SYMLINKS_ARGS
  m_SlaQuoted.erase(m_SlaQuoted.size() - 1);
  DeleteFile(&m_SlaQuoted.c_str()[1]);
#endif
#endif
  HTRACE(L"%s\n", &m_SlaQuoted.c_str()[1]);

  return ExitCode;
}

void UACHelper::WriteArgs(
  const wchar_t   aFunction,
  const wchar_t*  aArgument1,
  const wchar_t*  aArgument2
)
{
  Open();
  fwprintf(m_UacArgs, L"-%c \"%s\" \"%s\"\n", aFunction, aArgument1, aArgument2);

  // Save the SID under which we are running. 
  // Since we raise UAC via LSEUacHelper.exe, it runs as admin user, which has a different SID, 
  // so we need the current SID in symlink, because there we need to read the settings from current user
  wchar_t*  currentSid;
  bool bSidValid = GetCurrentSid(&currentSid);
  fwprintf(m_UacArgs, L"%s\n", currentSid);
  LocalFree(currentSid);
}

void UACHelper::SaveProgressbarPosition(
  RECT& a_ProgressbarPosition
)
{
  fwprintf(m_UacArgs, L"%x,%x,%x,%x\n", a_ProgressbarPosition.left, a_ProgressbarPosition.top, a_ProgressbarPosition.right, a_ProgressbarPosition.bottom);
}



