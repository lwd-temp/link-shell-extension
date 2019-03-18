// PropPageGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "LSEConfig.h"
#include "PropPageGeneral.h"

extern int TheRebootExplorer;
extern CMultiLanguage gMlg;

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
  , m_SmartMirror(false)
  , m_DeloreanCopy(false)
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

  ChangegFlags(eDisableSmartmove, &m_SmartMove, true);
  CheckDlgButton(IDC_CHECK_SmartMove, !m_SmartMove);

  ChangegFlags(eLogOutput, &m_Logging, true);
  CheckDlgButton(IDC_CHECK_Logging, m_Logging);

  ChangegFlags(eEnableRemote, &m_RemoteCapabilities, true);
  CheckDlgButton(IDC_CHECK_RemoteCapabilities, !m_RemoteCapabilities);

  if (!gpfCreateSymbolicLink)
  {
    // With WindowsXP gray out the Backup Option if the user does not hold the backup privs
    BOOL b = EnableTokenPrivilege(SE_BACKUP_NAME);
    BOOL r = EnableTokenPrivilege(SE_RESTORE_NAME);
    EnableTokenPrivilege(SE_SECURITY_NAME);
    if ( ((FALSE == b) || (FALSE == r)) )
    {
      // And disable the backup
      ChangegFlags(eBackupMode, NULL, m_BackupMode);

      CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_BackupMode);
      pButton->EnableWindow(FALSE);
    }
    else
      ChangegFlags(eBackupMode, &m_BackupMode, true);
  }
  else
    ChangegFlags(eBackupMode, &m_BackupMode, true);

  CheckDlgButton(IDC_CHECK_BackupMode, m_BackupMode);


  // Outer ReparsePoint Handling
  ChangegFlags(eCropReparsePoints, &m_CropReparsePoints, true);
  ChangegFlags(eUnrollReparsePoints, &m_UnrollReparsePoints, true);
  ChangegFlags(eSpliceReparsePoints, &m_SpliceReparsePoints, true);
  CheckDlgButton(IDC_RADIO_OuterJunctionCrop, m_CropReparsePoints);
  CheckDlgButton(IDC_RADIO_OuterJunctionUnroll, m_UnrollReparsePoints);
  CheckDlgButton(IDC_RADIO_OuterJunctionSplice, m_SpliceReparsePoints);


  // If we are under Vsita/Windows7 show all dialogs for symlinks
  if (gpfCreateSymbolicLink)
  {
    CButton* pButton = (CButton*)GetDlgItem(IDC_RADIO_Relative_Symbolic_Links);
    pButton->ShowWindow(SW_SHOW);
    pButton = (CButton*)GetDlgItem(IDC_RADIO_Absolute_Symbolic_Links);
    pButton->ShowWindow(SW_SHOW);
    pButton = (CButton*)GetDlgItem(IDC_STATIC_Symlink_Caption);
    pButton->ShowWindow(SW_SHOW);

    ChangegFlags(eForceAbsoluteSymbolicLinks, &m_SymlinkAbsRel, true);
    CheckDlgButton(IDC_RADIO_Relative_Symbolic_Links, !m_SymlinkAbsRel);
    CheckDlgButton(IDC_RADIO_Absolute_Symbolic_Links, m_SymlinkAbsRel);
  }
  // Fill the Combo Box
  int i = 0;
  do {
    m_Language.AddString(TheLanguageMapping[i++].Name);
  } while (TheLanguageMapping[i].Code != 0 );


  // Read out the current language setting
  GetValue(L"Language", &m_PreviousLangCode);
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
  SetValue(L"Language", TheLanguageMapping[nIndex].Code);

  ChangegFlags(eDisableSmartmove, NULL, m_SmartMove);
  ChangegFlags(eLogOutput, NULL, m_Logging);
  ChangegFlags(eEnableRemote, NULL, m_RemoteCapabilities);

  ChangegFlags(eEnableSmartMirror, NULL, m_SmartMirror);
  ChangegFlags(eDeloreanCopy, NULL, m_DeloreanCopy);

  ChangegFlags(eBackupMode, NULL, m_BackupMode);
  ChangegFlags(eForceAbsoluteSymbolicLinks, NULL, m_SymlinkAbsRel);

  ChangegFlags(eCropReparsePoints, NULL, m_CropReparsePoints);
  ChangegFlags(eUnrollReparsePoints, NULL, m_UnrollReparsePoints);
  ChangegFlags(eSpliceReparsePoints, NULL, m_SpliceReparsePoints);
  
  return CPropertyPage::OnOK();
}

BOOL PropPageGeneral::OnApply()
{
  SetModified(FALSE);
  return CPropertyPage::OnApply();
}

