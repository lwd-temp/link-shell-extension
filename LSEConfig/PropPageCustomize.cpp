// PropPageCustomize.cpp : implementation file
//

#include "stdafx.h"
#include "LSEConfig.h"
#include "PropPageCustomize.h"

extern int TheRebootExplorer;
extern CMultiLanguage gMlg;

// PropPageCustomize dialog

IMPLEMENT_DYNAMIC(PropPageCustomize, CPropertyPage)

PropPageCustomize::PropPageCustomize()
	: CPropertyPage(PropPageCustomize::IDD)
  , m_SmartMirror(false)
  , m_DeloreanCopy(false)
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
  ON_BN_CLICKED(IDC_CHECK_ExtendedMenue, &PropPageCustomize::OnBnClickedExtendedMenue)
  ON_BN_CLICKED(IDC_CHECK_BackupMode, &PropPageCustomize::OnBnClickedBackupMode)
END_MESSAGE_MAP()


BOOL PropPageCustomize::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  gMlg.ReplaceWindowTexts(GetSafeHwnd());

  ChangegFlags(eEnableSmartMirror, &m_SmartMirror, true);
  CheckDlgButton(IDC_CHECK_SmartMirror, !m_SmartMirror);

  ChangegFlags(eDeloreanCopy, &m_DeloreanCopy, true);
  CheckDlgButton(IDC_CHECK_DeloreanCopy, !m_DeloreanCopy);

  ChangegFlags(eEnableExtended, &m_ExtendedMenue, true);
  CheckDlgButton(IDC_CHECK_ExtendedMenue, m_ExtendedMenue);

  ChangegFlags(eBackupMode, &m_BackupMode, true);
  CheckDlgButton(IDC_CHECK_BackupMode, m_BackupMode);

  return TRUE;
}

// PropPageCustomize message handlers

void PropPageCustomize::OnBnClickedCheckSmartMirror()
{
  m_SmartMirror = !m_SmartMirror;
  SetModified();
}

void PropPageCustomize::OnBnClickedCheckDeloreanCopy()
{
  m_DeloreanCopy = !m_DeloreanCopy;
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
  ChangegFlags(eEnableSmartMirror, NULL, m_SmartMirror);
  ChangegFlags(eDeloreanCopy, NULL, m_DeloreanCopy);
  ChangegFlags(eEnableExtended, NULL, m_ExtendedMenue);
  ChangegFlags(eBackupMode, NULL, m_BackupMode);
  
  return CPropertyPage::OnOK();
}

BOOL PropPageCustomize::OnApply()
{
  SetModified(FALSE);
  return CPropertyPage::OnApply();
}

