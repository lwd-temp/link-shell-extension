#include "stdafx.h"

#include "hardlink_types.h"
#include "AsyncContext.h"

AsyncContext::
AsyncContext() : m_WaitEvent{ 0 }, m_RetVal{ 0 }, m_Status{ eRunning }, m_SourcePath{ 0 }, m_DestPath{0}
{
  InitializeCriticalSection(&m_DataLock);
  Reset();
}

int 
AsyncContext::
PutStatus(wchar_t*  aSource, wchar_t* aDestination)
{
  int RetVal;
  EnterCriticalSection(&m_DataLock);
#if 0
  // Use this, if you want to slow down the operation, thus showing the progress bar 
  Sleep(1);
#endif
  if (eRunning == m_Status)
  {
    if (m_SourcePath[0] == 0x00)
    {
      wcscpy_s(m_SourcePath, HUGE_PATH, aSource);

      // In some cases we don't have a destination, e.g. in FindHardlink
      if (aDestination)
        wcscpy_s(m_DestPath, HUGE_PATH, aDestination);

      RetVal = ERROR_SUCCESS;
    }
    else
      RetVal = ERROR_REQUEST_REFUSED;
  }
  else
    RetVal = ERROR_REQUEST_ABORTED;

  LeaveCriticalSection(&m_DataLock);

  return RetVal;
}

int
AsyncContext::
GetStatus(wchar_t*  aSourcePath, wchar_t*  aDestpath)
{
  // TODO: statt der CriticalSection einen atomic whcar verwenden, dann ist das alles viel übersichtlicher
  EnterCriticalSection(&m_DataLock);
  if (m_SourcePath[0])
  {
    wcscpy_s(aSourcePath, HUGE_PATH, m_SourcePath);
    m_SourcePath[0] = 0x00;
  }

  if (m_DestPath[0])
  {
    wcscpy_s(aDestpath, HUGE_PATH, m_DestPath);
    m_DestPath[0] = 0x00;
  }
  LeaveCriticalSection(&m_DataLock);
  
  return true;
}

int 
AsyncContext::
Add2SnapShot(
  const __int64 & aPoints, 
  const __int64 & aSize, 
  const __int64 & aItems
) 
{ 
  m_Progress = m_SnapShotValue + Effort(aPoints, aSize, aItems);
  if (eStopped == m_Status)
    return ERROR_REQUEST_ABORTED;
  else
    return ERROR_SUCCESS;
}

void
AsyncContext::
AddProgress(
  const __int64 & aPoints,
  const __int64 & aSize,
  const __int64 & aItems
)
{
  m_Progress += Effort(aPoints, aSize, aItems);
}

AsyncContext::
~AsyncContext()
{
  DeleteCriticalSection(&m_DataLock);
  if (m_WaitEvent)
    CloseHandle(m_WaitEvent);
}

