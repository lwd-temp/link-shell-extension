#pragma once
#include "afxwin.h"

#include "XIcon.h"

// PropPageIconJunction dialog

class PropPageIconJunction : public CPropertyPage
{
	DECLARE_DYNAMIC(PropPageIconJunction)

public:
	PropPageIconJunction();
	virtual ~PropPageIconJunction();

  virtual BOOL OnInitDialog();
  virtual void OnOK();
  virtual BOOL OnApply();

  // Dialog Data
	enum { IDD = IDD_ICONJUNCTION };
  CXIcon m_ButtonIcon;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  int FetchOverlayPrio();
  int FetchOverlayPath(bool aCommit);
  void EnableDisableOverlayControl(int aSwitch);
  void EnableDisableFileDialog(int aSwitch);
  void LoadIconPreview(const wchar_t* aOverlayPath);


	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedOverlayIcon();
  afx_msg void OnEnKillfocusEditOverlayPriority();
  afx_msg void OnEnKillfocusEditOverlayPath();
  afx_msg void OnBnClickedButtonOverlayPath();
  afx_msg void OnBnClickedCheckOverlayPath();

protected:
  bool m_Overlay;
  CEdit m_OverlayPriority;
  CEdit m_OverlayPath;
  bool m_OverlayPathCustom;
};
