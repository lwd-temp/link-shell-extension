// PropPageCustomize.cpp : implementation file
//

#include "stdafx.h"
#include "LSEConfig.h"
#include "PropPageCustomize.h"

extern int TheRebootExplorer;
extern CMultiLanguage gMlg;
extern LSESettings gLSESettings;


// PropPageCustomize dialog

IMPLEMENT_DYNAMIC(PropPageCustomize, CPropertyPage)

PropPageCustomize::PropPageCustomize()
	: CPropertyPage(PropPageCustomize::IDD)
  , m_DisableSmartMirror(false)
  , m_DisableDeloreanCopy(false)
  , m_DisableJunction(false)
  , m_ExtendedMenue(false)
  , m_BackupMode(false)
{
}

PropPageCustomize::~PropPageCustomize()
{
}

void PropPageCustomize::DoDataExchange(CDataExchange* pDX)
{
}


BEGIN_MESSAGE_MAP(PropPageCustomize, CPropertyPage)
  ON_BN_CLICKED(IDC_CHECK_SmartMirror, &PropPageCustomize::OnBnClickedCheckSmartMirror)
  ON_BN_CLICKED(IDC_CHECK_DeloreanCopy, &PropPageCustomize::OnBnClickedCheckDeloreanCopy)
  ON_BN_CLICKED(IDC_CHECK_Junction, &PropPageCustomize::OnBnClickedCheckJunction)
  ON_BN_CLICKED(IDC_CHECK_ExtendedMenue, &PropPageCustomize::OnBnClickedExtendedMenue)
  ON_BN_CLICKED(IDC_CHECK_BackupMode, &PropPageCustomize::OnBnClickedBackupMode)
END_MESSAGE_MAP()


BOOL PropPageCustomize::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  gMlg.ReplaceWindowTexts(GetSafeHwnd());

  gLSESettings.ChangegFlags(eDisableSmartMirror, &m_DisableSmartMirror, true);
  CheckDlgButton(IDC_CHECK_SmartMirror, !m_DisableSmartMirror);

  gLSESettings.ChangegFlags(eDisableDeloreanCopy, &m_DisableDeloreanCopy, true);
  CheckDlgButton(IDC_CHECK_DeloreanCopy, !m_DisableDeloreanCopy);

  gLSESettings.ChangegFlags(eDisableJunction, &m_DisableJunction, true);
  CheckDlgButton(IDC_CHECK_Junction, !m_DisableJunction);

  gLSESettings.ChangegFlags(eEnableExtended, &m_ExtendedMenue, true);
  CheckDlgButton(IDC_CHECK_ExtendedMenue, m_ExtendedMenue);

  gLSESettings.ChangegFlags(eBackupMode, &m_BackupMode, true);
  CheckDlgButton(IDC_CHECK_BackupMode, m_BackupMode);

  return TRUE;
}

// PropPageCustomize message handlers

void PropPageCustomize::OnBnClickedCheckSmartMirror()
{
  m_DisableSmartMirror = !m_DisableSmartMirror;
  SetModified();
}

void PropPageCustomize::OnBnClickedCheckDeloreanCopy()
{
  m_DisableDeloreanCopy = !m_DisableDeloreanCopy;
  SetModified();
}

void PropPageCustomize::OnBnClickedCheckJunction()
{
  m_DisableJunction = !m_DisableJunction;
  SetModified();
}

void PropPageCustomize::OnBnClickedExtendedMenue()
{
  m_ExtendedMenue = !m_ExtendedMenue;
  SetModified();
}

void PropPageCustomize::OnBnClickedBackupMode()
{
  m_BackupMode = !m_BackupMode;
  SetModified();
}

void PropPageCustomize::OnOK()
{
  gLSESettings.ChangegFlags(eDisableSmartMirror, NULL, m_DisableSmartMirror);
  gLSESettings.ChangegFlags(eDisableDeloreanCopy, NULL, m_DisableDeloreanCopy);
  gLSESettings.ChangegFlags(eDisableJunction, NULL, m_DisableJunction);
  gLSESettings.ChangegFlags(eEnableExtended, NULL, m_ExtendedMenue);
  gLSESettings.ChangegFlags(eBackupMode, NULL, m_BackupMode);
  
  return CPropertyPage::OnOK();
}

BOOL PropPageCustomize::OnApply()
{
  SetModified(FALSE);
  return CPropertyPage::OnApply();
}

