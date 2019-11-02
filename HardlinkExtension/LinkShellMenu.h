/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once


#define CFSTR_HARDLINK        TEXT("HardLink")

struct Target
{
  Target() { m_Path[0] = 0x0; m_Flags = 0; };
  WCHAR		m_Path[HUGE_PATH];
	ULONG		m_Flags;
};

void
ErrorFromSystem(
	DWORD	aErrorCode
);

class Progressbar;

// this is the actual OLE Shell context menu handler
class HardLinkExt : public IContextMenu, IShellExtInit
{
public:
	HardLinkExt();
	virtual ~HardLinkExt();

	//IUnknown members
	STDMETHODIMP
	QueryInterface(
		REFIID,
		LPVOID FAR *
	);

	STDMETHODIMP_(ULONG)
	AddRef(
	);

	STDMETHODIMP_(ULONG)
	Release(
	);

	//IShell members
	STDMETHODIMP
	QueryContextMenu(
		HMENU					hMenu,
		UINT					indexMenu,
		UINT					idCmdFirst,
		UINT					idCmdLast,
		UINT					uFlags
	);

	STDMETHODIMP
	InvokeCommand(
		LPCMINVOKECOMMANDINFO lpcmi
	);

	STDMETHODIMP
	GetCommandString(
		UINT_PTR				idCmd,
		UINT					uFlags,
		UINT FAR*				reserved,
		LPSTR					pszName,
		UINT					cchMax
	);

	//IShellExtInit methods
	STDMETHODIMP
	Initialize(
		LPCITEMIDLIST			pIDFolder,
		LPDATAOBJECT		pDataObj,
		HKEY				hKeyID
	);

private:
	enum			FileType
	{
		eFile		= 0x01,
		eDir		= 0x02,
		eJunction	= 0x04,
		eVolume	    = 0x08,
		eMountPoint = 0x10,
		eSymbolicLink = 0x20,
		eNTFS		= 0x40,
    eSymbolicLinkFile = 0x80,
    eExtendedVerbs = 0x100
	};

	CommandType	m_Command[eCommandType__Free__];
	ULONG		m_cRef;
	UINT		m_ClipFormat;
	ULONG		m_nTargets;
	ULONG		m_bTargetsFlag;
	Target*		m_pTargets;
	size_t		m_ClipboardSize;
	Target		m_DropTarget;
	bool		m_DragDrop;
	bool		m_ForceContext;

private:
	HRESULT
	FreeShellSelection (
	);

	HRESULT
	GetShellSelection (
	LPDATAOBJECT			pDataObj
	);

	HRESULT
	SelectionToClipboard(
	);

	HRESULT
	ClipboardToSelection(
		bool aProbe
	);

	void
	GetFileAttr(
		Target&			aTarget,
		ULONG&			aTargetsFlag,
    const bool  aIsNtfs
  );

  void
  InsertCommand(
    HMENU&        a_Submenu,
    UINT&         a_SubmenuIdx,
    UINT&         a_idCmd,
    int           a_MenuIdx,
    CommandType   a_CommandType,
    UINT&         a_CommandIdx,
    INT&          a_nEntries
  );

  void
  CreateContextMenu(
    HMENU&				a_hSubmenu,
    UINT&					a_idCmd,
    UINT&					a_CommandIdx,
    UINT					a_MenuOffset,
    INT&          a_nEntries,
    bool          a_SrcDstOnSameDrive
  );

  void
  CreateContextMenu(
    HMENU&				hMenu,
    UINT&					indexMenu,
    UINT&					idCmd,
    UINT&					aCommandIdx,
    UINT					MenuOffset
  );

	HRESULT
	CancelPickLink(
	);

	HRESULT
	DropHardLink(
		Target&		DestPath
	);

  HRESULT
  DropSymbolicLink(
    Target&		DestPath,
    bool      aCopy = false
	);

	HRESULT
	DropJunction(
		Target&		DestPath,
    bool      aCopy = false
	);

	HRESULT
	DropReplaceJunction(
		Target&		DestPath
	);

	HRESULT
	DropHardLinkClone(
		Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi
	);

	HRESULT
	DropSymbolicLinkClone(
		Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi
	);

	HRESULT 
	DropMountPoint(
		Target&		aTarget,
    bool      aCopy = false
	);

	HRESULT 
	DeleteMountPoint(
	);

  HRESULT
  SmartMirror(
	  Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi,
    int                     aIDS_ProgressBarHeading,
    int                     aIDS_AutoRename,
    int                     aIDS_Failed1,
    int                     aIDS_Failed2,
    FileInfoContainer::CopyReparsePointFlags  aMode
  );

  HRESULT
  SmartXXX(
	  Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi,
    int                     aIDS_ProgressBarHeading,
    int                     aIDS_AutoRename,
    int                     aIDS_Failed1,
    int                     aIDS_Failed2,
    FileInfoContainer::CopyReparsePointFlags  aFlags,
    bool                    aHardVsSymbolic
  );

	HRESULT 
	DropSmartCopy(
	  Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi
	);

  HRESULT 
  DropReplaceSymbolicLink(
	  Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi
  );

  HRESULT
  DropReplaceMountPoint(
	  Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi
  );

  HRESULT
  DropSmartMirror(
	  Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi
  );

  HRESULT
  DropDeloreanCopy(
	  Target&		              aTarget,
    LPCMINVOKECOMMANDINFO   lpcmi
  );



	void
	ErrorCreating(
		const wchar_t*	aDirectory,
		int			aType,
		int			aErrString,
		int			aReason
	);

	wchar_t*
	DrivePrefix(
		const wchar_t*	aPath,
		wchar_t*	aDrivePrefix
	);
};

int
ReplaceJunction(
  wchar_t*  aSource,
  wchar_t*  aTarget
);

int
ReplaceSymbolicLink(
  wchar_t*  aSource,
  wchar_t*  aTarget,
  bool      aKeepAbsRel
);

int
ReplaceMountPoint(
  wchar_t*  aTarget,
  wchar_t*  aSource
);

