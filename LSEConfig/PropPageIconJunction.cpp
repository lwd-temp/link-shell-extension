// PropPageIconJunction.cpp : implementation file
//

#include "stdafx.h"
#include "LSEConfig.h"
#include "PropPageIconJunction.h"
#include "PropPageGeneral.h"

extern int TheRebootExplorer;
extern CMultiLanguage gMlg;


// PropPageIconJunction dialog

IMPLEMENT_DYNAMIC(PropPageIconJunction, CPropertyPage)

PropPageIconJunction::PropPageIconJunction()
	: CPropertyPage(PropPageIconJunction::IDD)
  , m_Overlay(false)
  , m_OverlayPathCustom(false)
{

}

PropPageIconJunction::~PropPageIconJunction()
{
}

void PropPageIconJunction::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EDIT_Junction_Overlay_Priority, m_OverlayPriority);
  DDX_Control(pDX, IDC_BUTTON_Junction_Icon, m_ButtonIcon);
  DDX_Control(pDX, IDC_EDIT_Junction_Overlay_Path, m_OverlayPath);
}

BOOL PropPageIconJunction::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

  gMlg.ReplaceWindowTexts(GetSafeHwnd());

  // Get the overall Overlay status from registry to checkbox
  ChangegFlags(eJunctionOverlay, &m_Overlay, true);
  CheckDlgButton(IDC_CHECK_Junction_Overlay_Icon, !m_Overlay);
  EnableDisableOverlayControl(!m_Overlay);

  // Update the value for the overlay Prio
  int Prio = 0;
  GetValue(LSE_REGISTRY_JUNCTION_OVERLAY_PRIO, &Prio);
  
  wchar_t buf[16];
  if(EOF != swprintf_s(buf, L"%d", Prio))
    m_OverlayPriority.SetWindowText(buf);

  // See if a custom icon is specified and set dialog elements
  wchar_t OverlayPath[MAX_PATH];
  OverlayPath[0] = 0x00;
  ChangeValue(LSE_REGISTRY_JUNCTION_ICON, OverlayPath, MAX_PATH);
  if (OverlayPath[0])
    m_OverlayPathCustom = true;

  m_OverlayPath.SetWindowText(OverlayPath);
  CheckDlgButton(IDC_CHECK_Junction_Overlay_Path, m_OverlayPathCustom);

  EnableDisableFileDialog(m_OverlayPathCustom & !m_Overlay);

  LoadIconPreview(OverlayPath);

  return TRUE;
}

BEGIN_MESSAGE_MAP(PropPageIconJunction, CPropertyPage)
  ON_BN_CLICKED(IDC_CHECK_Junction_Overlay_Icon, &PropPageIconJunction::OnBnClickedOverlayIcon)
  ON_EN_KILLFOCUS(IDC_EDIT_Junction_Overlay_Priority, &PropPageIconJunction::OnEnKillfocusEditOverlayPriority)
  ON_EN_KILLFOCUS(IDC_EDIT_Junction_Overlay_Path, &PropPageIconJunction::OnEnKillfocusEditOverlayPath)
  ON_BN_CLICKED(IDC_BUTTON_Junction_Overlay_Path, &PropPageIconJunction::OnBnClickedButtonOverlayPath)
  ON_BN_CLICKED(IDC_CHECK_Junction_Overlay_Path, &PropPageIconJunction::OnBnClickedCheckOverlayPath)
END_MESSAGE_MAP()


void PropPageIconJunction::LoadIconPreview(const wchar_t* aOverlayPath)
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
                                   MAKEINTRESOURCE(IDI_ICON_JunctionOverlay),
                                   IMAGE_ICON, 256, 256, LR_DEFAULTSIZE);
  }

  m_ButtonIcon.Resize();
  m_ButtonIcon.SetIcon(hGlobeIcon, 128);
}

void PropPageIconJunction::EnableDisableOverlayControl(int aSwitch)
{
  CButton* pButton = (CButton*)GetDlgItem(IDC_STATIC_Junction_Overlay_Priority01);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_STATIC_Junction_Overlay_Priority02);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_EDIT_Junction_Overlay_Priority);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_STATIC_Junction_Overlay_Priority03);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_BUTTON_Junction_Icon);
  pButton->EnableWindow(aSwitch);

  pButton = (CButton*)GetDlgItem(IDC_CHECK_Junction_Overlay_Path);
  pButton->EnableWindow(aSwitch);
}

void PropPageIconJunction::EnableDisableFileDialog(int aSwitch)
{
  CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_Junction_Overlay_Path);
  pButton->EnableWindow(aSwitch);
  pButton = (CButton*)GetDlgItem(IDC_EDIT_Junction_Overlay_Path);
  pButton->EnableWindow(aSwitch);
}

void PropPageIconJunction::OnBnClickedOverlayIcon()
{
  m_Overlay = !m_Overlay;

  EnableDisableOverlayControl(!m_Overlay);
  EnableDisableFileDialog(m_OverlayPathCustom & !m_Overlay);

  SetModified();
  TheRebootExplorer = 1;
}

int PropPageIconJunction::FetchOverlayPrio()
{
  wchar_t buf[16];
  m_OverlayPriority.GetWindowText(buf, 16);
  
  int Prio;
  if(EOF != swscanf_s(buf, L"%d", &Prio))
    SetValue(LSE_REGISTRY_JUNCTION_OVERLAY_PRIO, Prio);

  return 42;
}

int PropPageIconJunction::FetchOverlayPath(bool aCommit)
{
  wchar_t buf[MAX_PATH];
  m_OverlayPath.GetWindowText(buf, MAX_PATH);
  
  if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(buf))
  {
    if (aCommit)
      ChangeValue(LSE_REGISTRY_JUNCTION_ICON, buf, MAX_PATH);
    LoadIconPreview(buf);
  }

  return 42;
}

void PropPageIconJunction::OnEnKillfocusEditOverlayPriority()
{
  SetModified();
  TheRebootExplorer = 1;
}

void PropPageIconJunction::OnEnKillfocusEditOverlayPath()
{
  FetchOverlayPath(false);
  SetModified();
  TheRebootExplorer = 1;
}

void PropPageIconJunction::OnBnClickedButtonOverlayPath()
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

void PropPageIconJunction::OnBnClickedCheckOverlayPath()
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

void PropPageIconJunction::OnOK()
{
  ChangegFlags(eJunctionOverlay, NULL, m_Overlay);

  FetchOverlayPrio();
  if (m_OverlayPathCustom)
    FetchOverlayPath(true);
  else
    DeleteValue(LSE_REGISTRY_JUNCTION_ICON);
}

BOOL PropPageIconJunction::OnApply()
{
  SetModified(FALSE);
  return CPropertyPage::OnApply();
}
