/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
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


	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO:: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("LSEConfig"));

	PropPageGeneral aPropPageGeneral;
	PropPageIconHardlink  aPropPageIconHardlink;
  PropPageIconJunction  aPropPageIconJunction;
  PropPageIconSymlink   aPropPageIconSymlink;
  PropPageCustomize     aPropPageCustomize;
  
  int LangCode;
  GetValue(L"Language", &LangCode);
  gMlg.SetLangCode(LangCode);
  TreePropSheet::CTreePropSheetEx sheet( gMlg.Replace(L"#MLG#10100") );

  // 
  // To initiate a CopySettings() if necessary
  //
  _LSESettings  LSESettings;
  GetLSESettings(LSESettings, true);


  sheet.SetTreeViewMode( TRUE, TRUE, FALSE);
  sheet.SetEmptyPageText( gMlg.Replace(L"#MLG#10101") );
  sheet.SetTreeDefaultImages( IDB_EMPTY_IMAGE_LIST, 16, RGB( 255, 255, 255 ) );

  sheet.SetIsResizable(false);

  sheet.AddPage( &aPropPageGeneral);
  aPropPageGeneral.m_psp.dwFlags &= ~PSP_HASHELP;
  sheet.AddPage( &aPropPageIconHardlink );
  aPropPageIconHardlink .m_psp.dwFlags &= ~PSP_HASHELP;
  sheet.AddPage( &aPropPageIconJunction );
  aPropPageIconJunction.m_psp.dwFlags &= ~PSP_HASHELP;

  if (gpfCreateSymbolicLink)
  {
    sheet.AddPage( &aPropPageIconSymlink );
    aPropPageIconSymlink.m_psp.dwFlags &= ~PSP_HASHELP;
  }

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
