/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */


#include "stdafx.h"
#include "LSEConfig.h"


#include "TreePropSheetEx.h"

#include "PropPageIconHardlink.h"
#include "PropPageIconJunction.h"
#include "PropPageIconSymlink.h"
#include "PropPageGeneral.h"
#include "PropPageCustomize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CMultiLanguage gMlg;
extern LSESettings gLSESettings;
HINSTANCE     g_hInstance = NULL;


// CLSEConfigApp

BEGIN_MESSAGE_MAP(CLSEConfigApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CLSEConfigApp construction

CLSEConfigApp::CLSEConfigApp()
{
	// TODO:: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CLSEConfigApp object

CLSEConfigApp theApp;


// CLSEConfigApp initialization

BOOL CLSEConfigApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

  InitCreateHardlink ();

  SetRegistryKey(_T("LSEConfig"));

	PropPageGeneral aPropPageGeneral;
	PropPageIconHardlink  aPropPageIconHardlink;
  PropPageIconJunction  aPropPageIconJunction;
  PropPageIconSymlink   aPropPageIconSymlink;
  PropPageCustomize     aPropPageCustomize;
  
  gLSESettings.Init();

  int LangCode;
  gLSESettings.GetValue(L"Language", &LangCode);
  gMlg.SetLangCode(LangCode);
  TreePropSheet::CTreePropSheetEx sheet( gMlg.Replace(L"#MLG#10100") );

  // 
  // To initiate a CopySettings() if necessary
  //


  sheet.SetTreeViewMode( TRUE, TRUE, FALSE);
  sheet.SetEmptyPageText( gMlg.Replace(L"#MLG#10101") );
  sheet.SetTreeDefaultImages( IDB_EMPTY_IMAGE_LIST, 16, RGB( 255, 255, 255 ) );

  sheet.SetIsResizable(false);

  sheet.AddPage( &aPropPageGeneral);
  aPropPageGeneral.m_psp.dwFlags &= ~PSP_HASHELP;
  sheet.AddPage( &aPropPageIconHardlink );
  aPropPageIconHardlink .m_psp.dwFlags &= ~PSP_HASHELP;

  if (!(gLSESettings.GetFlags() & eDisableJunction))
  { 
    sheet.AddPage(&aPropPageIconJunction);
    aPropPageIconJunction.m_psp.dwFlags &= ~PSP_HASHELP;
  }

  sheet.AddPage( &aPropPageIconSymlink );
  aPropPageIconSymlink.m_psp.dwFlags &= ~PSP_HASHELP;

#if defined _DEBUG
  sheet.AddPage( &aPropPageCustomize);
  aPropPageCustomize.m_psp.dwFlags &= ~PSP_HASHELP;
#endif

  sheet.SetAutoExpandTree(true);
  // Use this to disable the Apply Butoon
  // sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;
  sheet.m_psh.dwFlags &= ~(PSH_HASHELP); 

  sheet.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
