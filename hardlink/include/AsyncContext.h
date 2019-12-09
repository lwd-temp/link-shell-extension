/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

class AsyncContext
{
	public:
    AsyncContext();

    enum Status_t
    {
      eVoid = -1,
      eRunning = false,
      eStopped = true
    };

  protected:
    Effort            m_Progress;
    Effort            m_SnapShotValue;
    mutex             m_DataLock;
    wchar_t           m_SourcePath[HUGE_PATH];
    wchar_t           m_DestPath[HUGE_PATH];

  public:
    HANDLE            m_WaitEvent;
    int               m_RetVal;
    Status_t          m_Status;

  public:
    const Effort& GetProgress() { return m_Progress; };
    void Reset() { m_Progress.Reset(); m_SnapShotValue.Reset(); };
    void CreateSnapShot() { m_SnapShotValue = m_Progress; };
    int Add2SnapShot(const __int64 & aPoints, const __int64 & aSize, const __int64 & aItems);
    void AddProgress(const __int64 & aPoints, const __int64 & aSize, const __int64 & aItems);

    bool Wait(const DWORD aTimeout) { return WAIT_OBJECT_0 == WaitForSingleObject(m_WaitEvent, aTimeout) ? true : false; };
    void Cancel() { m_Status = eStopped; };
	  bool IsRunning() { return eRunning == m_Status; };
    Status_t  Status() { return m_Status; };

    int 
    PutStatus(wchar_t*  aPath, wchar_t* aDestination = nullptr);

    int 
    GetStatus(wchar_t*  aPath, wchar_t*  aDestpath);

    ~AsyncContext();
};

