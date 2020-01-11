/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#include "stdafx.h"

#include "hardlink_types.h"
#include "AsyncContext.h"

#include "Progressbar.h"


Progressbar::
Progressbar() : m_pIOperDlg{nullptr}, m_Rect{}, m_PreflightProgress{0}
{
  HRESULT hr = CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IOperationsProgressDialog, (void**)&m_pIOperDlg);
  hr = m_pIOperDlg->StartProgressDialog(NULL,
    PROGDLG_NORMAL | PROGDLG_MODAL | PROGDLG_AUTOTIME
  );

  hr = m_pIOperDlg->SetMode(PDM_DEFAULT);
  SetOperation(SPACTION_COPYING);

  m_Visible = false;
}


Progressbar::
~Progressbar(
)
{
  m_pIOperDlg->StopProgressDialog();
  m_pIOperDlg->Release();
  m_pIOperDlg = NULL;
}

HWND
Progressbar::
GetDlghWnd()
{
  // Show Window
  HWND hDlgWnd = NULL;
  IOleWindow *pOleWindow;
  HRESULT hr = m_pIOperDlg->QueryInterface(IID_IOleWindow, (LPVOID *)&pOleWindow);

  if (SUCCEEDED(hr))
  {
    hr = pOleWindow->GetWindow(&hDlgWnd);
    if (FAILED(hr))
      hDlgWnd = NULL;

    pOleWindow->Release();
  }
  return hDlgWnd;
}

bool 
Progressbar::
SetMode(DWORD aMode)
{
  m_pIOperDlg->SetMode(aMode);
  return true;
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

bool 
Progressbar::
Update(AsyncContext& aContext, DWORD aMode)
{
  wchar_t         SourcePath[HUGE_PATH], DestPath[HUGE_PATH];
  aContext.GetStatus(SourcePath, DestPath);
  SetMode(aMode);
  SetCurrentPath(SourcePath, DestPath);
  Show();
  return HasUserCancelled();
}

bool
Progressbar::
Update(AsyncContext& aContext, const Effort& aEffort)
{
  wchar_t         SourcePath[HUGE_PATH], DestPath[HUGE_PATH];
  aContext.GetStatus(SourcePath, DestPath);
  SetProgress(aEffort);
  SetCurrentPath(SourcePath, DestPath);
  Show();
  return HasUserCancelled();
}


bool 
Progressbar::
HasUserCancelled() 
{ 
  PDOPSTATUS OperationStatus;
  m_pIOperDlg->GetOperationStatus(&OperationStatus);
  return OperationStatus == PDOPS_CANCELLED;
}


bool 
Progressbar::
SetOperation(SPACTION aAction) 
{ 
  return ERROR_SUCCESS == m_pIOperDlg->SetOperation(aAction);
}

void 
Progressbar::
SetCurrentPath(
  wchar_t* aSourcePath,
  wchar_t* aDestPath
)
{ 
  IShellItem* FileItem;
  HRESULT hr = SHCreateItemFromParsingName(aSourcePath, NULL, IID_IShellItem, (void**)&FileItem);

  PathRemoveFileSpecW(aSourcePath);
  IShellItem* SourceItem;
  hr = SHCreateItemFromParsingName(aSourcePath, NULL, IID_IShellItem, (void**)&SourceItem);

  PathRemoveFileSpecW(aDestPath);
  IShellItem* TargetItem;
  hr = SHCreateItemFromParsingName(aDestPath, NULL, IID_IShellItem, (void**)&TargetItem);

  hr = m_pIOperDlg->UpdateLocations(SourceItem, TargetItem, FileItem);

  if (SourceItem)
    SourceItem->Release();
  if (TargetItem)
    TargetItem->Release();
  if (FileItem)
    FileItem->Release();
}

void
Progressbar::
SetRange(const Effort& aMaximum)
{
  m_Maximum = aMaximum;
}

void
Progressbar::
SetProgress(const Effort& aProgress)
{ 
  m_pIOperDlg->UpdateProgress(aProgress.m_Points, m_Maximum.m_Points,
    aProgress.m_Size, m_Maximum.m_Size,
    aProgress.m_Items, m_Maximum.m_Items
  );
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


bool 
Progressbar::
RestoreProgressbar(RECT&   aProgressbarPosition)
{
  if (aProgressbarPosition.left >= 0)
  {
    if (aProgressbarPosition.bottom > 0)
      SetWindowPos(aProgressbarPosition);
    aProgressbarPosition.left = -1;
    Show();
    return true;
  }
  return false;
}
