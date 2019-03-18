/*
	Copyright (C) 1999 - 2009, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"
#include "multilang.h"
#include "Progressbar.h"

#if _MSC_VER < 1300 // 1200 == VC++ 6.0
// Necessary for getting gpfCreateHardlink defined
#  include "AsyncContext.h"
#  include "hardlink.h"
#endif

extern int  gColourDepth;

Progressbar::
Progressbar(
  int         aTitleId,
  int         aCancelMsgId,
  HINSTANCE   aLSEInstance,
  int         aLanguageID,
  HWND        ahWnd,
  __int64     aMaximum
  ) : m_pIDlg(NULL), m_Rect()
{
#if _MSC_VER < 1300 // 1200 == VC++ 6.0
  // For Nt4 we need a CoInitialize, otherwise it crashes
  if (!gpfCreateHardlink) 
    CoInitialize(NULL);
#endif

  HRESULT hr = CoCreateInstance ( CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER,
                          IID_IProgressDialog, (void**) &m_pIDlg );

  m_LSEInstance = aLSEInstance;
  
	wchar_t	MlgMessage[MAX_PATH];
	LoadStringEx(m_LSEInstance, aTitleId, MlgMessage, MAX_PATH, aLanguageID);
  m_pIDlg->SetTitle(MlgMessage);

  m_Shell32Instance = LoadLibraryEx( 
	  L"shell32.dll\0",
	  NULL, 
	  DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE 
  );

  // Depending on resolution load resource id 161 for high color depth or 168 for low color depth
  if (m_Shell32Instance)
    m_pIDlg->SetAnimation ( m_Shell32Instance, gColourDepth == 8 ? 168 : 161);

	LoadStringEx(m_LSEInstance, aCancelMsgId, MlgMessage, MAX_PATH, aLanguageID);
  m_pIDlg->SetCancelMsg (MlgMessage, NULL );

  hr = m_pIDlg->StartProgressDialog ( ahWnd, 
    NULL,
    PROGDLG_NORMAL | /* PROGDLG_MODAL | */ PROGDLG_AUTOTIME ,
    NULL 
  );

  SetRange(aMaximum);
  m_Visible = false;
}


Progressbar::
~Progressbar(
)
{
  if (m_pIDlg)
  {
    Hide();
    m_pIDlg->StopProgressDialog();
    m_pIDlg->Release();
    m_pIDlg = NULL;
  }

  if (m_Shell32Instance)
    FreeLibrary(m_Shell32Instance);

#if _MSC_VER < 1300 
  if (!gpfCreateHardlink)
    CoUninitialize();
#endif
}

HWND
Progressbar::
GetDlghWnd()
{
  // Show Window
  HWND hDlgWnd = NULL;
  IOleWindow *pOleWindow;
  HRESULT hr = m_pIDlg->QueryInterface(IID_IOleWindow,(LPVOID *)&pOleWindow);
  if(SUCCEEDED(hr))
  {
    hr = pOleWindow->GetWindow(&hDlgWnd);
    if(FAILED(hr))
      hDlgWnd = NULL;

    pOleWindow->Release();
  }
  return hDlgWnd;
}

void
Progressbar::
Show()
{
  if (!m_Visible)
  {
    HWND h = GetDlghWnd(); 
    if (h)
      ShowWindow(h, SW_NORMAL);
  }
}

void
Progressbar::
Hide()
{
  if (m_Visible)
  {
    HWND h = GetDlghWnd(); 
    if (h)
      ShowWindow(h, SW_HIDE);
  }
}

void
Progressbar::
SetTitle(
  wchar_t* aTitle
) 
{ 
  HWND h = GetDlghWnd(); 
  SetWindowText(h, aTitle);
}

void 
Progressbar::
SetCurrentPath(
  wchar_t* aCurrentPath
) 
{ 
  wchar_t* pFileName = PathFindFileName(aCurrentPath);
  if (pFileName != aCurrentPath)
  {
    if (*(pFileName - 2) == L':')
    {
      // x:\file.txt
      wchar_t Save = *pFileName; 
      *pFileName = 0x00;
      m_pIDlg->SetLine ( 2, aCurrentPath, true, NULL ); 
      *pFileName = Save;
    }
    else
    {
      // x:\dir\file.txt
      wchar_t Save = *(pFileName - 1); 
      *(pFileName - 1) = 0x00;
      m_pIDlg->SetLine ( 2, aCurrentPath, true, NULL ); 
      *(pFileName - 1) = Save;
    }
    m_pIDlg->SetLine ( 1, pFileName, true, NULL ); 
  }
  else
  {
    // x:\ 
    m_pIDlg->SetLine ( 1, L" ", true, NULL ); 
    m_pIDlg->SetLine ( 2, pFileName, true, NULL ); 
  }
};

void
Progressbar::
SetRange(__int64 aMaximum)
{
  m_pIDlg->Timer (PDTIMER_RESET, NULL);
  m_Maximum = aMaximum;
  m_pIDlg->SetProgress64 (0, m_Maximum);
}

int
Progressbar::
GetWindowPos(
  RECT&   aRect
)
{
  int RetVal = 0;
  HWND h = GetDlghWnd(); 
  if (h)
  {
    GetWindowRect(h, &aRect);
    RetVal = 1;
  }
  return RetVal;
}

int
Progressbar::
SetWindowPos(
  RECT&   aRect
)
{
  int RetVal = 0;
  HWND h = GetDlghWnd(); 
  if (h)
  {
    RetVal = MoveWindow(h, aRect.left, aRect.top, aRect.right - aRect.left, aRect.bottom - aRect.top, TRUE);
  }
  return RetVal;
}
