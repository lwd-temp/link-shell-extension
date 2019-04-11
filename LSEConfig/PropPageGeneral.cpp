// PropPageGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "LSEConfig.h"
#include "PropPageGeneral.h"

extern int TheRebootExplorer;
extern CMultiLanguage gMlg;
extern LSESettings gLSESettings;

// PropPageGeneral dialog

// http://www.science.co.il/language/locale-codes.asp
//
LanguageMapping TheLanguageMapping[17] = { 
  { L"English", 1033 },
  { L"Chinese", 2052 },
  { L"Czech", 1029 },
  { L"French", 1036 },
  { L"German", 1031 },
  { L"Greek", 1032 },
  { L"Italian", 1040 },
  { L"Japanese", 1041 },
  { L"Korean", 1042 },
  { L"Polish", 1045 },
  { L"Portugese Brazil", 1046 },
  { L"Russian", 1049 },
  { L"Slovak", 1051 },
  { L"Spanish", 3082 },
  { L"Swedish", 1053 },
  { L"Turkish", 1055 },
  { NULL, 0}
};

IMPLEMENT_DYNAMIC(PropPageGeneral, CPropertyPage)

PropPageGeneral::PropPageGeneral()
	: CPropertyPage(PropPageGeneral::IDD)
  , m_SmartMove(false)
  , m_Logging(false)
  , m_RemoteCapabilities(false)
  , m_SymlinkAbsRel(false)
  , m_BackupMode(false)
  , m_DisableSmartMirror(false)
  , m_DisableDeloreanCopy(false)
  , m_DisableJunction(false)
  , m_CropReparsePoints(false)
  , m_UnrollReparsePoints(false)
  , m_SpliceReparsePoints(false)
  , m_PreviousLangCode(0)
{
}

PropPageGeneral::~PropPageGeneral()
{
}

void PropPageGeneral::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_COMBO_Language, m_Language);
}


BEGIN_MESSAGE_MAP(PropPageGeneral, CPropertyPage)
  ON_BN_CLICKED(IDC_CHECK_RemoteCapabilities, &PropPageGeneral::OnBnClickedRemoteCapabilities)
  ON_BN_CLICKED(IDC_CHECK_SmartMove, &PropPageGeneral::OnBnClickedSmartMove)
  ON_BN_CLICKED(IDC_RADIO_Relative_Symbolic_Links, &PropPageGeneral::OnBnClickedRadioRelAbsSymbolicLinks)
  ON_BN_CLICKED(IDC_RADIO_Absolute_Symbolic_Links, &PropPageGeneral::OnBnClickedRadioRelAbsSymbolicLinks)
  ON_BN_CLICKED(IDC_RADIO_OuterJunctionUnroll, &PropPageGeneral::OnBnClickedRadioOuterJunctionUnroll)
  ON_BN_CLICKED(IDC_RADIO_OuterJunctionSplice, &PropPageGeneral::OnBnClickedRadioOuterJunctionSplice)
  ON_BN_CLICKED(IDC_RADIO_OuterJunctionCrop, &PropPageGeneral::OnBnClickedRadioOuterJunctionCrop)
  ON_CBN_SELCHANGE(IDC_COMBO_Language, &PropPageGeneral::OnCbnSelchangeComboLanguage)
  ON_BN_CLICKED(IDC_CHECK_Logging, &PropPageGeneral::OnBnClickedLogging)
  ON_BN_CLICKED(IDC_CHECK_BackupMode, &PropPageGeneral::OnBnClickedBackupMode)
END_MESSAGE_MAP()


BOOL PropPageGeneral::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  gMlg.ReplaceWindowTexts(GetSafeHwnd());

  gLSESettings.ChangegFlags(eDisableSmartmove, &m_SmartMove, true);
  CheckDlgButton(IDC_CHECK_SmartMove, !m_SmartMove);

  gLSESettings.ChangegFlags(eLogOutput, &m_Logging, true);
  CheckDlgButton(IDC_CHECK_Logging, m_Logging);

  gLSESettings.ChangegFlags(eEnableRemote, &m_RemoteCapabilities, true);
  CheckDlgButton(IDC_CHECK_RemoteCapabilities, !m_RemoteCapabilities);

  gLSESettings.ChangegFlags(eBackupMode, &m_BackupMode, true);

  CheckDlgButton(IDC_CHECK_BackupMode, m_BackupMode);


  // Outer ReparsePoint Handling
  gLSESettings.ChangegFlags(eCropReparsePoints, &m_CropReparsePoints, true);
  gLSESettings.ChangegFlags(eUnrollReparsePoints, &m_UnrollReparsePoints, true);
  gLSESettings.ChangegFlags(eSpliceReparsePoints, &m_SpliceReparsePoints, true);
  CheckDlgButton(IDC_RADIO_OuterJunctionCrop, m_CropReparsePoints);
  CheckDlgButton(IDC_RADIO_OuterJunctionUnroll, m_UnrollReparsePoints);
  CheckDlgButton(IDC_RADIO_OuterJunctionSplice, m_SpliceReparsePoints);


  CButton* pButton = (CButton*)GetDlgItem(IDC_RADIO_Relative_Symbolic_Links);
  pButton->ShowWindow(SW_SHOW);
  pButton = (CButton*)GetDlgItem(IDC_RADIO_Absolute_Symbolic_Links);
  pButton->ShowWindow(SW_SHOW);
  pButton = (CButton*)GetDlgItem(IDC_STATIC_Symlink_Caption);
  pButton->ShowWindow(SW_SHOW);

  gLSESettings.ChangegFlags(eForceAbsoluteSymbolicLinks, &m_SymlinkAbsRel, true);
  CheckDlgButton(IDC_RADIO_Relative_Symbolic_Links, !m_SymlinkAbsRel);
  CheckDlgButton(IDC_RADIO_Absolute_Symbolic_Links, m_SymlinkAbsRel);

  // Fill the Combo Box
  int i = 0;
  do {
    m_Language.AddString(TheLanguageMapping[i++].Name);
  } while (TheLanguageMapping[i].Code != 0 );


  // Read out the current language setting
  gLSESettings.GetValue(L"Language", &m_PreviousLangCode);
  i = 0;
  do 
  {
     if (TheLanguageMapping[i].Code == m_PreviousLangCode)
       break;
  } while (TheLanguageMapping[++i].Code != 0 );

  if (TheLanguageMapping[i].Code != 0)
    m_Language.SetCurSel(i);


  return TRUE;
}

// PropPageGeneral message handlers

void PropPageGeneral::OnBnClickedSmartMove()
{
  m_SmartMove = !m_SmartMove;
  SetModified();
}

void PropPageGeneral::OnBnClickedBackupMode()
{
  m_BackupMode = !m_BackupMode;
  SetModified();
}

void PropPageGeneral::OnBnClickedLogging()
{
  m_Logging = !m_Logging;
  SetModified();
}

void PropPageGeneral::OnBnClickedRemoteCapabilities()
{
  m_RemoteCapabilities = !m_RemoteCapabilities;
  SetModified();
}

void PropPageGeneral::OnBnClickedRadioRelAbsSymbolicLinks()
{
  m_SymlinkAbsRel = !m_SymlinkAbsRel;
  SetModified();
}

void PropPageGeneral::OnBnClickedRadioOuterJunctionCrop()
{
  m_CropReparsePoints = true;
  m_UnrollReparsePoints = false;
  m_SpliceReparsePoints = false;
  SetModified();
}

void PropPageGeneral::OnBnClickedRadioOuterJunctionUnroll()
{
  m_CropReparsePoints = false;
  m_UnrollReparsePoints = true;
  m_SpliceReparsePoints = false;
  SetModified();
}

void PropPageGeneral::OnBnClickedRadioOuterJunctionSplice()
{
  m_CropReparsePoints = false;
  m_UnrollReparsePoints = false;
  m_SpliceReparsePoints = true;
  SetModified();
}
void PropPageGeneral::OnCbnSelchangeComboLanguage()
{
  SetModified();
  TheRebootExplorer = 1;
}

void PropPageGeneral::OnOK()
{
  int nIndex = m_Language.GetCurSel();
  gLSESettings.SetValue(L"Language", TheLanguageMapping[nIndex].Code);

  gLSESettings.ChangegFlags(eDisableSmartmove, NULL, m_SmartMove);
  gLSESettings.ChangegFlags(eLogOutput, NULL, m_Logging);
  gLSESettings.ChangegFlags(eEnableRemote, NULL, m_RemoteCapabilities);

  gLSESettings.ChangegFlags(eDisableSmartMirror, NULL, m_DisableSmartMirror);
  gLSESettings.ChangegFlags(eDisableDeloreanCopy, NULL, m_DisableDeloreanCopy);
  gLSESettings.ChangegFlags(eDisableJunction, NULL, m_DisableJunction);

  gLSESettings.ChangegFlags(eBackupMode, NULL, m_BackupMode);
  gLSESettings.ChangegFlags(eForceAbsoluteSymbolicLinks, NULL, m_SymlinkAbsRel);

  gLSESettings.ChangegFlags(eCropReparsePoints, NULL, m_CropReparsePoints);
  gLSESettings.ChangegFlags(eUnrollReparsePoints, NULL, m_UnrollReparsePoints);
  gLSESettings.ChangegFlags(eSpliceReparsePoints, NULL, m_SpliceReparsePoints);
  
  return CPropertyPage::OnOK();
}

BOOL PropPageGeneral::OnApply()
{
  SetModified(FALSE);
  return CPropertyPage::OnApply();
}

