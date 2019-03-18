/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

class Progressbar
{
protected:
  IOperationsProgressDialog*  m_pIOperDlg;
  bool              m_Visible;

  Effort            m_Progress;
  Effort            m_Maximum;

  RECT              m_Rect;

  int               m_PreflightProgress;

  const int MaxPreflight = 50;

public:
  Progressbar();

  void SetProgress(const Effort& aProgress);
  bool HasUserCancelled();
  bool SetMode(DWORD aMode);
  bool SetOperation(SPACTION aAction);
  bool Update(AsyncContext& apContext, DWORD aMode);
  bool Update(AsyncContext& aContext, const Effort& aEffort);

  void Show();

  void Hide();
  void SetCurrentPath(wchar_t* aSourcePath, wchar_t* aDestPath);
  void SetRange(const Effort& aMaximum);
  int GetWindowPos(RECT&   aRect);
  bool RestoreProgressbar(RECT&   aRect);

protected:
  HWND GetDlghWnd();
  int SetWindowPos(RECT&   aRect);

public:
  virtual
    ~Progressbar();
};