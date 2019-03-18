#include "stdafx.h"

#include "hardlink_types.h"

#include "MmfObject.h"
#include "AsyncContext.h"
#include "hardlink.h"
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

