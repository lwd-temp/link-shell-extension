#include "stdafx.h"

#include "hardlink_types.h"
#include "AsyncContext.h"

AsyncContext::
AsyncContext()
{
  InitializeCriticalSection(&m_DataLock);
  m_WaitEvent = 0; 
  m_RetVal = 0;
  m_CurrentPath[0] = 0x00;
  m_Cancel = false;

  Reset();
}

int 
AsyncContext::
PutStatus(wchar_t*  aPath)
{
  int RetVal;
  EnterCriticalSection(&m_DataLock);
#if 0  // DEBUG_DEFINES 
  // Use this, if you want to slow down the operation, thus showing the progress bar 
  Sleep(250);
#endif
  if (!m_Cancel)
  {
    if (m_CurrentPath[0] == 0x00)
    {
      wcscpy_s(m_CurrentPath, HUGE_PATH, aPath);
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
GetStatus(wchar_t*  aPath)
{
  EnterCriticalSection(&m_DataLock);
  if (m_CurrentPath[0])
  {
    wcscpy_s(aPath, HUGE_PATH, m_CurrentPath);
    m_CurrentPath[0] = 0x00;
  }
  LeaveCriticalSection(&m_DataLock);
  
  return true;
}

int 
AsyncContext::
Add2SnapShot(
  __int64 aProgress
) 
{ 
  m_Value = m_SnapShotValue + aProgress; 
  if (m_Cancel)
    return ERROR_REQUEST_ABORTED;
  else
    return ERROR_SUCCESS;
}


AsyncContext::
~AsyncContext()
{
  DeleteCriticalSection(&m_DataLock);
  if (m_WaitEvent)
    CloseHandle(m_WaitEvent);
}

