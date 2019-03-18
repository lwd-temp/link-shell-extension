// PropPageIconHardlink.cpp : implementation file
//

#include "stdafx.h"
#include "LSEConfig.h"
#include "PropPageIconHardlink.h"
#include "PropPageGeneral.h"

extern int TheRebootExplorer;
extern CMultiLanguage gMlg;

// PropPageIconHardlink dialog

IMPLEMENT_DYNAMIC(PropPageIconHardlink, CPropertyPage)

PropPageIconHardlink::PropPageIconHardlink()
	: CPropertyPage(PropPageIconHardlink::IDD)
  , m_Overlay(false)
  , m_OverlayPathCustom(false)
{

}

PropPageIconHardlink::~PropPageIconHardlink()
{
}

void PropPageIconHardlink::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EDIT_Hardlink_Overlay_Priority, m_OverlayPriority);
  DDX_Control(pDX, IDC_BUTTON_Hardlink_Icon, m_ButtonIcon);
  DDX_Control(pDX, IDC_EDIT_Hardlink_Overlay_Path, m_OverlayPath);
}

BOOL PropPageIconHardlink::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  gMlg.ReplaceWindowTexts(GetSafeHwnd());

  // Get the overall Overlay status from registry to checkbox
  ChangegFlags(eHardlinkOverlay, &m_Overlay, true);
  CheckDlgButton(IDC_CHECK_Hardlink_Overlay_Icon, !m_Overlay);
  EnableDisableOverlayControl(!m_Overlay);

  // Update the value for the overlay Prio
  int Prio = 0;
  GetValue(LSE_REGISTRY_HARDLINK_OVERLAY_PRIO, &Prio);
  
  wchar_t buf[16];
  if(EOF != swprintf_s(buf, L"%d", Prio))
    m_OverlayPriority.SetWindowText(buf);

  // See if a custom icon is specified and set dialog elements
  wchar_t OverlayPath[MAX_PATH];
  OverlayPath[0] = 0x00;
  ChangeValue(LSE_REGISTRY_HARDLINK_ICON, OverlayPath, MAX_PATH);
  if (OverlayPath[0])
    m_OverlayPathCustom = true;

  m_OverlayPath.SetWindowText(OverlayPath);
  CheckDlgButton(IDC_CHECK_Hardlink_Overlay_Path, m_OverlayPathCustom);

  EnableDisableFileDialog(m_OverlayPathCustom & !m_Overlay);

  LoadIconPreview(OverlayPath);

  return TRUE;
}

BEGIN_MESSAGE_MAP(PropPageIconHardlink, CPropertyPage)
  ON_BN_CLICKED(IDC_CHECK_Hardlink_Overlay_Icon, &PropPageIconHardlink::OnBnClickedOverlayIcon)
  ON_EN_KILLFOCUS(IDC_EDIT_Hardlink_Overlay_Priority, &PropPageIconHardlink::OnEnKillfocusEditOverlayPriority)
  ON_EN_KILLFOCUS(IDC_EDIT_Hardlink_Overlay_Path, &PropPageIconHardlink::OnEnKillfocusEditOverlayPath)
  ON_BN_CLICKED(IDC_BUTTON_Hardlink_Overlay_Path, &PropPageIconHardlink::OnBnClickedButtonOverlayPath)
  ON_BN_CLICKED(IDC_CHECK_Hardlink_Overlay_Path, &PropPageIconHardlink::OnBnClickedCheckOverlayPath)
END_MESSAGE_MAP()


void PropPageIconHardlink::LoadIconPreview(const wchar_t* aOverlayPath)
{
  // Load the icon onto the screen
  HICON hGlobeIcon;
  if (aOverlayPath[0] && m_OverlayPathCustom && !m_Overlay)
  {
    hGlobeIcon = (HICON)::LoadImage(AfxGetInstanceHandle(), 
                                   aOverlayPath,
                                   IMAGE_ICON, 256, 256, LR_LOADFROMFILE);
  }
  else
  {
    hGlobeIcon = (HICON)::LoadImage(AfxGetInstanceHandle(), 
                                   MAKEINTRESOURCE(IDI_ICON_HardlinkOverlay),
                                   IMAGE_ICON, 256, 256, LR_DEFAULTSIZE);
  }

  m_ButtonIcon.Resize();
  m_ButtonIcon.SetIcon(hGlobeIcon, 128);
}

void PropPageIconHardlink::EnableDisableOverlayControl(int aSwitch)
{
  CButton* pButton = (CButton*)GetDlgItem(IDC_STATIC_Hardlink_Overlay_Priority01);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_STATIC_Hardlink_Overlay_Priority02);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_EDIT_Hardlink_Overlay_Priority);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_STATIC_Hardlink_Overlay_Priority03);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_BUTTON_Hardlink_Icon);
  pButton->EnableWindow(aSwitch);

  pButton = (CButton*)GetDlgItem(IDC_CHECK_Hardlink_Overlay_Path);
  pButton->EnableWindow(aSwitch);
}

void PropPageIconHardlink::EnableDisableFileDialog(int aSwitch)
{
  CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_Hardlink_Overlay_Path);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_EDIT_Hardlink_Overlay_Path);
  pButton->EnableWindow(aSwitch);
}

void PropPageIconHardlink::OnBnClickedOverlayIcon()
{
  m_Overlay = !m_Overlay;

  EnableDisableOverlayControl(!m_Overlay);
  EnableDisableFileDialog(m_OverlayPathCustom & !m_Overlay);

  SetModified();
  TheRebootExplorer = 1;
}

int PropPageIconHardlink::FetchOverlayPrio()
{
  wchar_t buf[16];
  m_OverlayPriority.GetWindowText(buf, 16);
  
  int Prio;
  if(EOF != swscanf_s(buf, L"%d", &Prio))
    SetValue(LSE_REGISTRY_HARDLINK_OVERLAY_PRIO, Prio);

  return 42;
}

int PropPageIconHardlink::FetchOverlayPath(bool aCommit)
{
  wchar_t buf[MAX_PATH];
  m_OverlayPath.GetWindowText(buf, MAX_PATH);
  
  if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(buf))
  {
    if (aCommit)
      ChangeValue(LSE_REGISTRY_HARDLINK_ICON, buf, MAX_PATH);
    LoadIconPreview(buf);
  }

  return 42;
}

void PropPageIconHardlink::OnEnKillfocusEditOverlayPriority()
{
  SetModified();
  TheRebootExplorer = 1;
}

void PropPageIconHardlink::OnEnKillfocusEditOverlayPath()
{
  FetchOverlayPath(false);
  SetModified();
  TheRebootExplorer = 1;
}

void PropPageIconHardlink::OnBnClickedButtonOverlayPath()
{
  CFileDialog IconSelectionDlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT,L"Icon Files (*.ico)|*.ico|");
  INT_PTR iRet = IconSelectionDlg.DoModal();

  if(iRet == IDOK)
  {
    CString PathName = IconSelectionDlg.GetPathName();
    m_OverlayPath.SetWindowText(PathName);
    FetchOverlayPath(false);

    SetModified();
    TheRebootExplorer = 1;
  }
 }

void PropPageIconHardlink::OnBnClickedCheckOverlayPath()
{
  m_OverlayPathCustom = !m_OverlayPathCustom;
  EnableDisableFileDialog(m_OverlayPathCustom);

  if (m_OverlayPathCustom)
    FetchOverlayPath(false);
  else
    LoadIconPreview(L"");

  SetModified();
  TheRebootExplorer = 1;
}

void PropPageIconHardlink::OnOK()
{
  ChangegFlags(eHardlinkOverlay, NULL, m_Overlay);

  FetchOverlayPrio();
  if (m_OverlayPathCustom)
    FetchOverlayPath(true);
  else
    DeleteValue(LSE_REGISTRY_HARDLINK_ICON);
}

BOOL PropPageIconHardlink::OnApply()
{
  SetModified(FALSE);
  return CPropertyPage::OnApply();
}