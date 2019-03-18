/*
	Copyright (C) 1999_2008, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#pragma once

class Progressbar 
{
public:  
  Progressbar(  
    int         aTitleId,
    int         aCancelMsgId,
    HINSTANCE   aLSEInstance,
    int         aLanguageID,
    HWND        ahWnd,
    __int64     aMaximum
  );

  
  void
  SetProgress(__int64 aProgress) { m_pIDlg->SetProgress64 (aProgress, m_Maximum); };

  BOOL 
  HasUserCancelled() { return m_pIDlg->HasUserCancelled(); };

  void
  SetTitle(wchar_t* aTitle);


  void
  Show();

  void
  Hide();

  void SetCurrentPath(wchar_t* aCurrentPath);

  void
  SetRange(__int64 aMaximum);

  int
  GetWindowPos(RECT&   aRect);

  int
  SetWindowPos(RECT&   aRect);

protected:
  IProgressDialog*  m_pIDlg;
  __int64           m_Maximum;
  bool              m_Visible;
  HINSTANCE         m_Shell32Instance;
  HINSTANCE         m_LSEInstance;

  RECT              m_Rect;



  HWND
  GetDlghWnd();

public:
  virtual 
    ~Progressbar();
};