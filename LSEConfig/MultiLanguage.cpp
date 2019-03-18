/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#include "stdafx.h"

#include "MultiLanguage.h"

#include <shlwapi.h>			// because of StrRChr
#include <errno.h>


const int cstrClassNameLength = 512;

wchar_t gBuffer[MAX_PATH];

CMultiLanguage::CMultiLanguage() : m_LangCode(1033)
{
  m_Instance = GetModuleHandle(NULL);
}
//////////////////////////////////////////////////////////////////////
CMultiLanguage::~CMultiLanguage()
{
}


BOOL 
CALLBACK 
EnumProc(
  HWND hwnd, 
  LPARAM lParam
)
{
  //TRACE(_T("BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lParam)\r\n"));
  if(!IsWindow(hwnd))
  {
    return FALSE;
  }

  CString strClassName;
  LPTSTR pstrclName = strClassName.GetBuffer(cstrClassNameLength + 1);
  int nCh = ::GetClassName (hwnd,  pstrclName, cstrClassNameLength);
  strClassName.ReleaseBuffer();

  if (nCh > 0 && strClassName == _T("Edit"))
    return TRUE;
  if (nCh > 0 && strClassName == _T("ComboBox"))
    return TRUE;
  if (nCh > 0 && strClassName.Find(_T("RichEdit")) != -1)
    return TRUE;

  EnumWinData *pEWData = (EnumWinData *) lParam;
  TCHAR pstrWndKey[cstrClassNameLength + 1];

  ::GetWindowText(hwnd, pstrWndKey, cstrClassNameLength);

  if (*pstrWndKey == _T('\0'))
    return TRUE;
  if(_tcslen(pstrWndKey) >= cstrClassNameLength)
     return TRUE;

  ::SetWindowText(hwnd, pEWData->pLangObj->Replace(pstrWndKey));

  return TRUE;
}

// 
// ReplaceWindowTexts
// 
HRESULT 
CMultiLanguage::
ReplaceWindowTexts(
  HWND hWnd
)
{
  //TRACE(_T("CMultiLanguage::ReplaceWindowTexts(LPTSTR pLogicalName, HWND hWnd)\r\n"));
  //	Common
  HWND hWinWnd = (HWND) hWnd;
  if(!IsWindow(hWinWnd))
  {
	    //TRACE(_T("No valid windowhandle!\r\n"));
      return S_FALSE;
  }
  TCHAR pstrKey[512];
  ::GetWindowText(hWinWnd, pstrKey, 512);
  ::SetWindowText(hWinWnd, Replace(pstrKey));

  //	Common
  EnumWinData EWData;
  EWData.pLangObj = this;

  EnumChildWindows(hWinWnd, EnumProc, (LPARAM)&EWData);

  return ERROR_SUCCESS;
}

LPCWSTR 
CMultiLanguage::
Replace(
  wchar_t*  a_StringKey
)
{
  if (!_wcsnicmp(a_StringKey, MLG_PREFIX, MLG_PREFIX_SIZE ))
  {
    UINT  StringId;
    swscanf (&a_StringKey[MLG_PREFIX_SIZE], L"%d", &StringId);

    LoadStringEx( m_Instance, StringId, gBuffer, MAX_PATH, m_LangCode);
    return gBuffer;
  }

  return a_StringKey;
}

