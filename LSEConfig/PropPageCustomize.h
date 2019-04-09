#pragma once
#include "afxwin.h"


// PropPageCustomize dialog

class PropPageCustomize : public CPropertyPage
{
	DECLARE_DYNAMIC(PropPageCustomize)

public:
	PropPageCustomize();
	virtual ~PropPageCustomize();

  virtual BOOL OnInitDialog();
  virtual void OnOK();
  virtual BOOL OnApply();

  // Dialog Data
	enum { IDD = IDD_CUSTOMIZE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
  bool m_DisableSmartMirror;
  bool m_DisableDeloreanCopy;
  bool m_DisableJunction;
  bool m_ExtendedMenue;
  bool m_BackupMode;


public:

public:
  afx_msg void OnBnClickedCheckSmartMirror();
  afx_msg void OnBnClickedCheckDeloreanCopy();
  afx_msg void OnBnClickedCheckJunction();
  afx_msg void OnBnClickedExtendedMenue();
  afx_msg void OnBnClickedBackupMode();
};
