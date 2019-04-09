#pragma once
#include "afxwin.h"

struct LanguageMapping
{
  wchar_t*  Name;
  int       Code;
};


// PropPageGeneral dialog

class PropPageGeneral : public CPropertyPage
{
	DECLARE_DYNAMIC(PropPageGeneral)

public:
	PropPageGeneral();
	virtual ~PropPageGeneral();

  virtual BOOL OnInitDialog();
  virtual void OnOK();
  virtual BOOL OnApply();

  // Dialog Data
	enum { IDD = IDD_GENERAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
  bool m_SmartMove;
  bool m_Logging;
  bool m_RemoteCapabilities;
  bool m_DisableSmartMirror;
  bool m_DisableDeloreanCopy;
  bool m_DisableJunction;
  bool m_SymlinkAbsRel;
  bool m_BackupMode;
  bool m_CropReparsePoints;
  bool m_UnrollReparsePoints;
  bool m_SpliceReparsePoints;

  int m_PreviousLangCode;

public:

public:
  afx_msg void OnBnClickedSmartMove();
  afx_msg void OnBnClickedLogging();
  afx_msg void OnBnClickedRemoteCapabilities();

  afx_msg void OnBnClickedRadioRelAbsSymbolicLinks();
  afx_msg void OnBnClickedBackupMode();

  CComboBox m_Language;

  afx_msg void OnBnClickedRadioOuterJunctionCrop();
  afx_msg void OnBnClickedRadioOuterJunctionUnroll();
  afx_msg void OnBnClickedRadioOuterJunctionSplice();
  afx_msg void OnCbnSelchangeComboLanguage();
};
