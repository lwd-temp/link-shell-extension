/*
	Copyright (C) 1999 - 2020, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"

#include "hardlink_types.h"

#include "Multilang.h"


/////////////////////////////////////////////////////////////////////////
LPVOID LoadResourceEx( HINSTANCE hInstance, LPCTSTR lpType, LPCTSTR lpName, WORD wLanguage, HRSRC *phrsrc=NULL )
{
  HRSRC hrsrc = FindResourceEx(hInstance,lpType,lpName,wLanguage);
  if (!hrsrc) 
	  hrsrc = FindResource(hInstance,lpName,lpType);
  if (!hrsrc) 
	  return NULL;
  if (phrsrc) 
	  *phrsrc=hrsrc;
  HGLOBAL hglb = LoadResource(hInstance,hrsrc);
  if (!hglb) 
	  return NULL;
  return LockResource(hglb);
}

///////////////////////////////////////////////////////////////////////////////

int LoadStringEx( HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax, WORD wLanguage )
{
  uID+=16;
  if (uID&~0xFFFFF) {
    SetLastError(ERROR_RESOURCE_NAME_NOT_FOUND);
    return 0;
  }
  
  WORD *data=(WORD*)LoadResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(uID>>4), wLanguage);

  if (!data) 
	  return 0;
  int n=uID&15;
  for (int i=0;i<n;++i)
    data+=*data+1;
  int len=*data;
  if (len==0) return 0;
  data++;
#ifdef _UNICODE
  if (len>nBufferMax-1) 
	  len=nBufferMax-1;
  memcpy(lpBuffer,data,len*2);
  lpBuffer[len]=0;
  return len;
#else
  int len2=WideCharToMultiByte(CP_ACP,0,data,len,NULL,NULL,0,NULL);
  if (len2>nBufferMax-1) 
	  len2=nBufferMax-1;
  WideCharToMultiByte(CP_ACP,0,data,len,lpBuffer,len2,0,NULL);
  lpBuffer[len2]=0;
  return len2;
#endif
}

PWSTR 
GetFormattedMlgMessage(
  HINSTANCE hInstance,
	int       aLanguageID,
  UINT      uID,
	...
)
{
	wchar_t	 MsgTxt[HUGE_PATH];
	LoadStringEx(hInstance, uID, MsgTxt, HUGE_PATH, aLanguageID);

  LPWSTR pBuffer = NULL;

  va_list args;
  va_start(args, uID);

	FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                  MsgTxt, 
                  0,  // ignored
                  0,  // ignored
                  (LPWSTR)&pBuffer, 
                  0, 
                  &args);

	va_end(args);

    return pBuffer;
}
///////////////////////////////////////////////////////////////////////////////

/* Unused, but could be useful
HACCEL LoadAcceleratorsEx( HINSTANCE hInstance, LPCTSTR lpTableName, WORD wLanguage )
{
  HRSRC hrsrc;
  LPVOID lpsz=LoadResourceEx(hInstance,RT_ACCELERATOR,lpTableName,wLanguage,&hrsrc);
  if (!lpsz) return NULL;

  struct ACCELTABLEENTRY {
    WORD fFlags;
    WORD wAnsi;
    WORD wId;
    WORD padding;
  };

  ACCELTABLEENTRY *src=(ACCELTABLEENTRY *)lpsz;
  int n=SizeofResource(hInstance,hrsrc)/sizeof(ACCELTABLEENTRY);
  ACCEL *dst=(ACCEL*)malloc(n*sizeof(ACCEL));
  if (!dst) {
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return NULL;
  }
  for (int i=0;i<n;++i) {
    dst[i].fVirt=src[i].fFlags&(FALT|FCONTROL|FNOINVERT|FSHIFT|FVIRTKEY);
    dst[i].key=src[i].wAnsi;
    dst[i].cmd=src[i].wId;
  }
  HACCEL hacc=CreateAcceleratorTable(dst,n);
  free(dst);
  return hacc;

}

///////////////////////////////////////////////////////////////////////////////

HMENU LoadMenuEx( HINSTANCE hInstance, LPCTSTR lpMenuName, WORD wLanguage )
{
  LPVOID lpsz=LoadResourceEx(hInstance,RT_MENU,lpMenuName,wLanguage);
  if (!lpsz) return NULL;
  return LoadMenuIndirect(lpsz);

}

///////////////////////////////////////////////////////////////////////////////

INT_PTR DialogBoxParamEx( HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam, WORD wLanguage )
{
  LPVOID lpsz=LoadResourceEx(hInstance,RT_DIALOG,lpTemplateName,wLanguage);
  if (!lpsz) return -1;
  return DialogBoxIndirectParam(hInstance,(DLGTEMPLATE*)lpsz,hWndParent,lpDialogFunc,dwInitParam);

}

///////////////////////////////////////////////////////////////////////////////

HWND CreateDialogParamEx( HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam, WORD wLanguage )
{
  LPVOID lpsz=LoadResourceEx(hInstance,RT_DIALOG,lpTemplateName,wLanguage);
  if (!lpsz) return NULL;
  return CreateDialogIndirectParam(hInstance,(DLGTEMPLATE*)lpsz,hWndParent,lpDialogFunc,dwInitParam);

}

///////////////////////////////////////////////////////////////////////////////

INT_PTR DialogBoxEx( HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, WORD wLanguage )
{
  return DialogBoxParamEx(hInstance,lpTemplateName,hWndParent,lpDialogFunc,0,wLanguage);

}

///////////////////////////////////////////////////////////////////////////////

HWND CreateDialogEx( HINSTANCE hInstance, LPCTSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, WORD wLanguage )
{
  return CreateDialogParamEx(hInstance,lpTemplateName,hWndParent,lpDialogFunc,0,wLanguage);

}

*/
///////////////////////////////////////////////////////////////////////////////

