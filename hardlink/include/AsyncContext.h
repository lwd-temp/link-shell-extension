
#pragma once

class AsyncContext
{
	public:
    AsyncContext();

  protected:
    __int64           m_Value;
    __int64           m_SnapShotValue;
    CRITICAL_SECTION  m_DataLock;
    wchar_t           m_CurrentPath[HUGE_PATH];

  public:
    HANDLE            m_WaitEvent;
    int               m_RetVal;
    bool              m_Cancel;

  public:
    __int64 Get() { return m_Value; };
    void Inc() { m_Value++; };
    void Reset() { m_Value = 0; m_SnapShotValue = 0; };
    void CreateSnapShot() { m_SnapShotValue = m_Value; };
    void AddProgress(__int64 aProgress) { m_Value += aProgress; };
    int Add2SnapShot(__int64 aProgress);

    bool Wait(DWORD aTimeout) { return WAIT_OBJECT_0 == WaitForSingleObject(m_WaitEvent, aTimeout) ? true : false; };
    void Cancel() { m_Cancel = true; };

    int 
    PutStatus(wchar_t*  aPath);

    int 
    GetStatus(wchar_t*  aPath);

    ~AsyncContext();
};

