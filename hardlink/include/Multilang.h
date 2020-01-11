/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once


LPVOID LoadResourceEx( HINSTANCE hInstance, LPCTSTR lpType, LPCTSTR lpName, WORD wLanguage, HRSRC *phrsrc );
int LoadStringEx( HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax, WORD wLanguage );

PWSTR 
GetFormattedMlgMessage(
  HINSTANCE hInstance,
  int       aLanguageID,
  UINT      uID,
	...
);


/*
HACCEL LoadAcceleratorsEx( HINSTANCE hInstance, LPCTSTR lpTableName, WORD wLanguage );
HMENU LoadMenuEx( HINSTANCE hInstance, LPCTSTR lpMenuName, WORD wLanguage );
INT_PTR DialogBoxParamEx( HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam, WORD wLanguage );
HWND CreateDialogParamEx( HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam, WORD wLanguage );
INT_PTR DialogBoxEx( HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, WORD wLanguage );
HWND CreateDialogEx( HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, WORD wLanguage );
*/



