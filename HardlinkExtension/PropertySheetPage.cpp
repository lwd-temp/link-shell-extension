/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */


#include "stdafx.h"

#include "PropertySheetPage.h"
#include "LinkShellMenu.h"


extern UINT       g_cRefThisDll;    // Reference count of this DLL.
extern HINSTANCE  g_hInstance;
extern LSESettings gLSESettings;

extern UINT       g_LockCount;

bool			 g_Themed;    


///////////////////////////////////////////////////////////////
// PropertySheetPageClassFactory

PropertySheetPageClassFactory::PropertySheetPageClassFactory() : m_cRef(0L)
{
    g_cRefThisDll++;
}

PropertySheetPageClassFactory::~PropertySheetPageClassFactory()
{
    g_cRefThisDll--;
}

STDMETHODIMP PropertySheetPageClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)
{
    *ppv = NULL;
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) PropertySheetPageClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) PropertySheetPageClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP
PropertySheetPageClassFactory::
CreateInstance(
	LPUNKNOWN pUnkOuter,
	REFIID riid,
	LPVOID *ppvObj
)
{
    *ppvObj = NULL;

	// it calls me with 00000000-0000-0000-C000-000000000046
    // Shell extensions typically don't support aggregation (inheritance)
    if (pUnkOuter)
    	return CLASS_E_NOAGGREGATION;

	PropertySheetPage* pPropertySheetPage = new PropertySheetPage();
	return pPropertySheetPage->QueryInterface(riid, ppvObj);
}

STDMETHODIMP PropertySheetPageClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

//
// PropertySheetPage Methods
//
PropertySheetPage::
PropertySheetPage() : m_cRef(0L)
{
	g_cRefThisDll++;

	g_Themed = false;
	if (IsThemeActive())
		g_Themed = true;

	m_File[0] = 0x00;
}

PropertySheetPage::
~PropertySheetPage()
{
	g_cRefThisDll--;
}

STDMETHODIMP 
PropertySheetPage::
QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	*ppv = NULL;

	HTRACE(L"LSE: PropertySheetPage IID %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n", 
			riid.Data1, 
			riid.Data2, 
			riid.Data3, 
			riid.Data4[0],
			riid.Data4[1],
			riid.Data4[2],
			riid.Data4[3],
			riid.Data4[4],
			riid.Data4[5],
			riid.Data4[6],
			riid.Data4[7]
	);

    if (IsEqualIID(riid, IID_IShellPropSheetExt) || IsEqualIID(riid, IID_IUnknown))
	{
		*ppv = (IShellPropSheetExt*)this;
	}
	else 
		if (IsEqualIID(riid, IID_IShellExtInit) )
		{
			*ppv = (IShellExtInit*)this;
		}

	if (*ppv)
	{
		AddRef();
		return NOERROR;
	}
		
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) 
PropertySheetPage::
AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) 
PropertySheetPage::
Release()
{
	if (--m_cRef)
		return m_cRef;

	delete this;
	return 0L;
}

// Called by the Shell when initializing an extension.
STDMETHODIMP
PropertySheetPage::
Initialize(
	LPCITEMIDLIST			pIDFolder,
	LPDATAOBJECT			pDataObj, 
	HKEY					hKeyID
)
{
  HTRACE(L"LSE: PropertySheetPage::Initialize called");

  HDROP     hdrop;
  FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM stg;

  // http://msdn2.microsoft.com/en-us/library/ms997646.aspx
  INITCOMMONCONTROLSEX iccex = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES };

  // Init the common controls.
  InitCommonControlsEx(&iccex);

  // Read the list of folders from the data object.  They're stored in HDROP
  // form, so just get the HDROP handle and then use the drag 'n' drop APIs
  // on it.
  if (FAILED(pDataObj->GetData(&etc, &stg)))
    return E_INVALIDARG;

  // Get an HDROP handle.
  hdrop = (HDROP)GlobalLock(stg.hGlobal);

  if (NULL == hdrop)
  {
    ReleaseStgMedium(&stg);
    return E_INVALIDARG;
  }

  // Determine how many files are involved in this operation.
  UINT uNumFiles = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);

  if (uNumFiles > 0)
  {
    // Get the filename.
    DragQueryFile(hdrop, 0, m_File, MAX_PATH);
  }

  // Release resources.
  GlobalUnlock(stg.hGlobal);
  ReleaseStgMedium(&stg);


  // Check if we show the Propertypage
  HRESULT rVal = S_OK;

  DWORD	FileSystemFlags;
  int DriveType;
  if (IsFileSystemNtfs(m_File, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType))
  {
    struct _stat stat;
    _wstat(m_File, &stat);
    if (stat.st_mode & _S_IFDIR)
    {
#if defined DEBUG_SHOW_TRUESIZE
      // Junctions, Mountpoints, SymbolicLinkDirs are all of type Directory. So no eed to check further
#else
      // Check if the selected dir is a junction, so that we
      // we can provide junction delete menue
      if (!ProbeJunction(m_File, NULL))
      {
        if (!ProbeMountPoint(m_File, NULL, 0, NULL))
        {
          // Check if we are under Vista
          if (!ProbeSymbolicLink(m_File, NULL))
            rVal = E_INVALIDARG;
        }
      }
#endif
    }
    else
    {
      if (!ProbeSymbolicLink(m_File, NULL))
      {
        // It could be a dangling Junction
        if (!ProbeJunction(m_File, NULL))
        {
          int RefCount = ProbeHardlink(m_File);
          if (RefCount < 2)
            rVal = E_INVALIDARG;
        }
      }
    }
  }
  else
  {
    rVal = E_INVALIDARG;
  }

  return rVal;
}

STDMETHODIMP 
PropertySheetPage::
AddPages (
  LPFNADDPROPSHEETPAGE lpfnAddPageProc,
  LPARAM lParam 
)
{
  // See also http://forums.codeguru.com/showthread.php?t=242567
  
  HTRACE(L"LSE: PropertySheetPage::AddPages called");

	PROPSHEETPAGE  psp;
	HPROPSHEETPAGE hPage;

  // Set up the PROPSHEETPAGE struct.
  ZeroMemory ( &psp, sizeof(PROPSHEETPAGE) );

  psp.dwSize      = sizeof(PROPSHEETPAGE);
  psp.dwFlags     = PSP_USEREFPARENT | PSP_USETITLE | PSP_DEFAULT | PSP_USECALLBACK; // | PSP_USEICONID;
  psp.hInstance   = g_hInstance;
  psp.pszTemplate = MAKEINTRESOURCE(IDD_LINKSHLEXT_PROPPAGE);
  //    psp.pszIcon     = MAKEINTRESOURCE(IDI_LINKSHLEXT_ICON_TAB);
  wchar_t ProPageCaption[MAX_PATH];
  LoadStringEx(g_hInstance, IDS_STRING_PropPageCaption, ProPageCaption, MAX_PATH, gLSESettings.GetLanguageID());
  psp.pszTitle    = ProPageCaption;
  psp.pfnDlgProc  = PropPageDlgProc;

  // Store the filename in this Property page's data area, for later use.
  ReparseProperties*  pReparseProperties = new ReparseProperties(m_File);
  psp.lParam      = (LPARAM) pReparseProperties;
  psp.pfnCallback = PropPageCallbackProc;
  //    psp.pcRefParent = (UINT*) &_Module.m_nLockCnt;
  psp.pcRefParent = &g_LockCount;

  // Create the page & get a handle.
  hPage = CreatePropertySheetPage ( &psp );

  if ( NULL != hPage )
  {
    // Call the shell's callback function, so it adds the page to
    // the property sheet.
    if ( !lpfnAddPageProc ( hPage, lParam ))
      DestroyPropertySheetPage ( hPage );
  }

  return S_OK;
}

INT_PTR CALLBACK PropPageDlgProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  BOOL bRet = FALSE;

  switch ( uMsg )
  {
    case WM_INITDIALOG:
      bRet = OnInitDialog ( hwnd, lParam );
    break;

    case WM_NOTIFY:
    {
      NMHDR* phdr = (NMHDR*) lParam;

      switch ( phdr->code )
      {
        case PSN_APPLY:
          bRet = OnApply ( hwnd, (PSHNOTIFY*) phdr );
        break;

       default:
          // HTRACE(L"WM_NOTIFY: phdr->idFrom: %08x, phdr->code %08x\n", phdr->idFrom, phdr->code);
        break;
      }
    }
    break;

    case WM_COMMAND:
    {
      long code = (long)wParam;		
      switch ( code )
      {
        case IDC_PROPPAGE_LINKSHLEXT_EXPLORE_TARGET_BUTTON:
          OnExploreTarget(hwnd, lParam);
        break;

#if defined DEBUG_SHOW_TRUESIZE
        case IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_BUTTON:
          OnTrueSize(hwnd, lParam);
        break;
#endif

        default:
          // HTRACE(L"WM_COMMAND: lparam %08x, wparam %08x\n", lParam, wParam);
        break;
      }

      // If the user changes any of the DTP controls, enable the Apply button.
      if (HIWORD(wParam) == EN_UPDATE)
        SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);
    
    }
    break;

    default:
      // HTRACE(L"umsg: %08x\n", uMsg);
    break;

  }

  return bRet;
}

UINT CALLBACK PropPageCallbackProc ( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp )
{
  if ( PSPCB_RELEASE == uMsg )
  {
    ReparseProperties* pReparseProperties = (ReparseProperties*)ppsp->lParam;
    delete pReparseProperties;
  }

  return 1;   // used for PSPCB_CREATE - let the page be created
}


void ShowTrueSizeData(HWND aHwnd, CopyStatistics& aStats)
{
  locale localeDeu("");
  ostringstream oss;
  oss.imbue(localeDeu);

  wchar_t buffer[MAX_PATH] = { 0 };

  oss << aStats.m_FilesTotal;
  wsprintf(buffer, L"%S", oss.str().c_str());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_FILE_ITEMS, buffer);
  oss.str("");
  oss.clear();

  oss << aStats.m_BytesTotal;
  wsprintf(buffer, L"%S", oss.str().c_str());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_FILE_BYTES, buffer);
  oss.str("");
  oss.clear();

  oss << aStats.m_FilesTotal - aStats.m_HardlinksTotal;
  wsprintf(buffer, L"%S", oss.str().c_str());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_HARDLINK_ITEMS, buffer);
  oss.str("");
  oss.clear();

  oss << aStats.m_BytesTotal - aStats.m_HardlinksTotalBytes;
  wsprintf(buffer, L"%S", oss.str().c_str());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_HARDLINK_BYTES, buffer);
  oss.str("");
  oss.clear();

  oss << aStats.m_BytesTotal - (aStats.m_BytesTotal - aStats.m_HardlinksTotalBytes);
  wsprintf(buffer, L"%S", oss.str().c_str());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_TOTAL_BYTES, buffer);
  oss.str("");
  oss.clear();

  oss << aStats.m_DirectoryTotal;
  wsprintf(buffer, L"%S", oss.str().c_str());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_FOLDER_ITEMS, buffer);
  oss.str("");
  oss.clear();

  oss << aStats.m_JunctionsTotal;
  wsprintf(buffer, L"%S", oss.str().c_str());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_JUNCTION_ITEMS, buffer);
  oss.str("");
  oss.clear();

  oss << aStats.m_SymlinksTotal;
  wsprintf(buffer, L"%S", oss.str().c_str());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_SYMLINK_ITEMS, buffer);
}

void ShowTrueSizeDlg(HWND aHwnd)
{
#if defined DEBUG_SHOW_TRUESIZE
  // Show TrueSize Menu
  for (int dlgItem = IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_FILE; dlgItem < IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_FREE; ++dlgItem)
  {
    HWND hStatic = GetDlgItem(aHwnd, dlgItem);
    ShowWindow(hStatic, SW_SHOWNORMAL);
  }
  UpdateWindow(aHwnd);
#endif
}

void OnTrueSize(HWND aHwnd, LPARAM lParam)
{
  // Get the filename 
  WCHAR	szTarget[HUGE_PATH];
  GetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, szTarget, HUGE_PATH);

  ShowTrueSizeDlg(aHwnd);

  _PathNameStatusList pathNameStatusList;
  CopyStatistics	stats;

  FileInfoContainer	FileList;
  GetLocalTime(&stats.m_StartTime);

  _ArgvPath TrueSizePath;
  _ArgvList TrueSizePathList;

  // Record all path for enumeration
  int DriveType = DRIVE_UNKNOWN;
  DWORD FileSystemFlags;
  IsFileSystemNtfs(szTarget, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);

  if (!PathIsUNC(szTarget))
    TrueSizePath.Argv = PATH_PARSE_SWITCHOFF;
  TrueSizePath.Argv += szTarget;

  TrueSizePath.DriveType = DriveType;
  TrueSizePathList.push_back(TrueSizePath);

  // Enumerate affected files
  AsyncContext    Context;
  int r = FileList.FindHardLink(TrueSizePathList, 0, &stats, &pathNameStatusList, &Context);
  while (!Context.Wait(250))
  {
    ShowTrueSizeData(aHwnd, stats);
  }

  // Calculate the true Size
  FileList.TrueSize(&stats, &pathNameStatusList, nullptr);
  ShowTrueSizeData(aHwnd, stats);


  DeletePathNameStatusList(pathNameStatusList);
  FileList.Dispose(nullptr, &stats);
}

void OnExploreTarget ( HWND aHwnd, LPARAM lParam )
{
  // Get the filename 
  WCHAR	szTarget[HUGE_PATH];
  GetDlgItemText (aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, szTarget, HUGE_PATH );

  ShellExecute (NULL, L"explore", szTarget, NULL, szTarget, SW_SHOWDEFAULT);
}

void ShowJunction(HWND aHwnd, wchar_t* aBuffer, wchar_t* aDest, ReparseProperties* apReparseProperties)
{
  // Show Junction
  //
  LoadStringEx(g_hInstance, IDS_STRING_PropPageLinkType, aBuffer, MAX_PATH, gLSESettings.GetLanguageID());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE, aBuffer);

  LoadStringEx(g_hInstance, IDS_STRING_Junction, aBuffer, MAX_PATH, gLSESettings.GetLanguageID());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE_VALUE, aBuffer);

  LoadStringEx(g_hInstance, IDS_STRING_Target, aBuffer, MAX_PATH, gLSESettings.GetLanguageID());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET, aBuffer);

  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE, aDest);
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, aDest);

  // Show Explore Target Button
  HWND hStatic = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_EXPLORE_TARGET_BUTTON);
  LoadStringEx(g_hInstance, IDS_STRING_ExploreTarget, aBuffer, MAX_PATH, gLSESettings.GetLanguageID());
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_EXPLORE_TARGET_BUTTON, aBuffer);
  ShowWindow(hStatic, SW_SHOWNORMAL);

  // Enable Edit
  HWND hEdit = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE);
  SendMessage(hEdit, EM_SETREADONLY, (WPARAM)0, (LPARAM)0);

  apReparseProperties->SetTarget(aDest, REPARSE_POINT_JUNCTION);
}

BOOL OnInitDialog ( HWND aHwnd, LPARAM lParam )
{
  PROPSHEETPAGE*  ppsp = (PROPSHEETPAGE*)lParam;
  ReparseProperties*		pReparseProperties = (ReparseProperties*)ppsp->lParam;

#if !defined DEBUG_SHOW_TRUESIZE
  // hide truesize button
  HWND hStatic = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_TRUESIZE_BUTTON);
  ShowWindow(hStatic, SW_HIDE);
  UpdateWindow(aHwnd);
#endif

  if (g_Themed)
  {
    EnableThemeDialogTexture(aHwnd, ETDT_USETABTEXTURE | ETDT_ENABLE);
    EnableTheming(true);
  }

  HTRACE(L"LSE: PropertySheetPage::OnInitDialog '%s'", pReparseProperties->Source);

  SetWindowLongPtr(aHwnd, GWLP_USERDATA, (LONG_PTR)pReparseProperties);

  // Display the full path in the top static control.
  SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_FILENAME, pReparseProperties->Source);

  struct _stat stat;
  _wstat(pReparseProperties->Source, &stat);

  wchar_t Buffer[MAX_PATH] = { 0 };
  if (stat.st_mode & _S_IFDIR)
  {
    // Check if the selected dir is a junction, so that we
    // we can provide junction delete menue
    WCHAR Dest[HUGE_PATH] = { 0 };
    if (ProbeJunction(pReparseProperties->Source, Dest))
    {
      ShowJunction(aHwnd, Buffer, Dest, pReparseProperties);
      ShowTrueSizeDlg(aHwnd);
    }
    else
    {
      WCHAR VolumeName[MAX_PATH] = { 0 };
      if (ProbeMountPoint(pReparseProperties->Source, Dest, HUGE_PATH, VolumeName))
      {
        // Show Mountpoint
        //
        LoadStringEx(g_hInstance, IDS_STRING_PropPageLinkType, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE, Buffer);

        LoadStringEx(g_hInstance, IDS_STRING_MountPoint, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE_VALUE, Buffer);

        LoadStringEx(g_hInstance, IDS_STRING_MountPointTarget, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET, Buffer);

        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE, Dest);
        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, Dest);

        // Display the Volumename 
        HWND hEdit = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_HARDLINKS);
        SendMessage(hEdit, EM_LIMITTEXT, (WPARAM)0, (LPARAM)0);
        ShowWindow(hEdit, SW_SHOWNORMAL);

        wcscat_s(VolumeName, MAX_PATH, L"\r\n");
        int ndx = GetWindowTextLength(hEdit);
        SendMessage(hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
        SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)VolumeName);

        // Show Explore Target Button
        HWND hStatic = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_EXPLORE_TARGET_BUTTON);
        LoadStringEx(g_hInstance, IDS_STRING_ExploreTarget, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_EXPLORE_TARGET_BUTTON, Buffer);
        ShowWindow(hStatic, SW_SHOWNORMAL);

        // Enable Edit
        hEdit = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE);
        SendMessage(hEdit, EM_SETREADONLY, (WPARAM)0, (LPARAM)0);

        pReparseProperties->SetTarget(Dest, REPARSE_POINT_MOUNTPOINT);

        ShowTrueSizeDlg(aHwnd);
      }
      else
      {
        if (ProbeSymbolicLink(pReparseProperties->Source, Dest))
        {
          // Show a Symbolic Link Directory
          //
          LoadStringEx(g_hInstance, IDS_STRING_PropPageLinkType, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE, Buffer);

          LoadStringEx(g_hInstance, IDS_STRING_Symlink, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE_VALUE, Buffer);

          LoadStringEx(g_hInstance, IDS_STRING_Target, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET, Buffer);

          int p = 0;
          if (IsVeryLongPath(Dest))
            p = PATH_PARSE_SWITCHOFF_SIZE;

          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE, &Dest[p]);

          // Since we can only resolve the symbolic link here, because pReparseProperties->Source
          // is a parameter of this function, the full path to the symbolic link has to be stored 
          // in the pReparseProperties struct
          WCHAR SymlinkTarget[HUGE_PATH] = { 0 };
          int r = ResolveSymboliclink(pReparseProperties->Source, Dest, SymlinkTarget);
          pReparseProperties->SetTarget(Dest, REPARSE_POINT_SYMBOLICLINK);

          if (S_OK == r)
          {
            WCHAR AbsoluteDest[HUGE_PATH];
            PathCanonicalize(AbsoluteDest, SymlinkTarget);
            SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, AbsoluteDest);
          }
          else
            SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, Dest);

          // Enable Edit
          HWND hEdit = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE);
          SendMessage(hEdit, EM_SETREADONLY, (WPARAM)0, (LPARAM)0);

          // Show Explore Target Button
          HWND hStatic = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_EXPLORE_TARGET_BUTTON);
          LoadStringEx(g_hInstance, IDS_STRING_ExploreTarget, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_EXPLORE_TARGET_BUTTON, Buffer);
          ShowWindow(hStatic, SW_SHOWNORMAL);

          ShowTrueSizeDlg(aHwnd);
        }
        else
        {
#if defined DEBUG_SHOW_TRUESIZE
          // directory
          int p = 0;
          if (IsVeryLongPath(pReparseProperties->Source))
            p = PATH_PARSE_SWITCHOFF_SIZE;

          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE, &pReparseProperties->Source[p]);

          LoadStringEx(g_hInstance, IDS_STRING_Target, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET, Buffer);

          LoadStringEx(g_hInstance, IDS_STRING_Directory, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE_VALUE, Buffer);

          pReparseProperties->SetTarget(pReparseProperties->Source, REPARSE_POINT_FAIL);

          // Enable Edit
          HWND hEdit = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE);
          SendMessage(hEdit, EM_SETREADONLY, (WPARAM)0, (LPARAM)0);
          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, pReparseProperties->Source);

          ShowTrueSizeDlg(aHwnd);
#endif
        }
      }
    }
  }
  else
  {
    wchar_t	Dest[MAX_PATH] = { 0 };
    // Make sure we check for the Symbolic Link first, because a symbolic link might also point to a hardlink, but
    // we don't want to see the properties what the symbolic link points to, but the Symbolic Link itself
    if (ProbeSymbolicLink(pReparseProperties->Source, Dest))
    {
      // Show Symbolic Link Files
      //
      LoadStringEx(g_hInstance, IDS_STRING_PropPageLinkType, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
      SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE, Buffer);

      LoadStringEx(g_hInstance, IDS_STRING_Symlink, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
      SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE_VALUE, Buffer);

      LoadStringEx(g_hInstance, IDS_STRING_Target, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
      SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET, Buffer);

      int p = 0;
      if (IsVeryLongPath(Dest))
        p = PATH_PARSE_SWITCHOFF_SIZE;

      SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE, &Dest[p]);

      // Enable Edit
      HWND hEdit = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE);
      SendMessage(hEdit, EM_SETREADONLY, (WPARAM)0, (LPARAM)0);

      pReparseProperties->SetTarget(Dest, REPARSE_POINT_SYMBOLICLINK);
    }
    else
    {
      // Hardlinks
      int RefCount = ProbeHardlink(pReparseProperties->Source);
      if (RefCount > 1)
      {
        // Show Hardlink
        //
        LoadStringEx(g_hInstance, IDS_STRING_PropPageLinkType, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE, Buffer);

        LoadStringEx(g_hInstance, IDS_STRING_Hardlink, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_LINKTYPE_VALUE, Buffer);

        wchar_t		RefCountStr[64];
#if defined DEBUG_RICHARD_SCHAEFER
        wsprintf(RefCountStr, L"#### %d ####", RefCount);
#else
        wsprintf(RefCountStr, L"%d", RefCount);
#endif
        LoadStringEx(g_hInstance, IDS_STRING_PropPageRefCount, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET, Buffer);

        SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE, RefCountStr);

        // Enumerate Hardlinks under Windows7
        wchar_t	LinkName[HUGE_PATH + 2];
        DWORD LinkNameLength = HUGE_PATH;

        // resolve a possible subst chain
        int LinkNameIdx = 0;
        if (!PathIsUNC(pReparseProperties->Source))
        {
          wchar_t FullName[HUGE_PATH];
          wcscpy_s(FullName, HUGE_PATH, PATH_PARSE_SWITCHOFF);

          GetFullPathName(pReparseProperties->Source, HUGE_PATH, &FullName[PATH_PARSE_SWITCHOFF_SIZE], NULL);

          if (IsVeryLongPath(FullName))
            LinkNameIdx = PATH_PARSE_SWITCHOFF_SIZE;

          // Follow a possible subst chain, until we end up with e.g \Device\HardDisk\Volume4
          do
          {
            wcscpy_s(LinkName, HUGE_PATH, &FullName[LinkNameIdx]);
            LinkName[2] = 0x00;
            QueryDosDevice(LinkName, FullName, HUGE_PATH);
          }
          // a subst on a subst can be recognized by \??\ as prefix to e.g. \??\f:\tmp
          while (FullName[1] == '?');
        }

        // enumerate the siblings
        HANDLE h = FindFirstFileNameW(pReparseProperties->Source, 0, &LinkNameLength, &LinkName[2]);
        if (INVALID_HANDLE_VALUE != h)
        {
          //
          // If some calls FindFirstFileName() with a file on a mapped network
          // drive FindFirstFileNameW returns ERROR_INVALID_LEVEL. But why?
          //

          // So we have to show the sibblins dialog only after we made sure
          // that we got the first sibling, since it seems it is not enough
          // to know that the reference count is > 1
          HWND hStatic = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_HARDLINKENUM);
          ShowWindow(hStatic, SW_SHOWNORMAL);
          LoadStringEx(g_hInstance, IDS_STRING_PropPageHardlinkEnum, Buffer, MAX_PATH, gLSESettings.GetLanguageID());
          SetDlgItemText(aHwnd, IDC_PROPPAGE_LINKSHLEXT_HARDLINKENUM, Buffer);

          HWND hEdit = GetDlgItem(aHwnd, IDC_PROPPAGE_LINKSHLEXT_HARDLINKS);
          SendMessage(hEdit, EM_LIMITTEXT, (WPARAM)0, (LPARAM)0);
          ShowWindow(hEdit, SW_SHOWNORMAL);

          do
          {
            wcscat_s(LinkName, HUGE_PATH + 2, L"\r\n");
            int ndx = GetWindowTextLength(hEdit);
            SendMessage(hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
            SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)LinkName);

            LinkNameLength = HUGE_PATH;
          } while (FindNextFileNameW(h, &LinkNameLength, &LinkName[2]));
          FindClose(h);
        }
        else
        {
#if defined DEBUG_RICHARD_SCHAEFER
          wchar_t	FullMsg[HUGE_PATH];
          wsprintf(FullMsg, L"LSE: FindFirstFileName failed: %08x,'%s'", GetLastError(), pReparseProperties->Source);
          int mr = MessageBox(NULL,
            FullMsg,
            L"LSE: PropertyPage",
            MB_ICONERROR);
#endif
        }
      }
      else
      {
        if (ProbeJunction(pReparseProperties->Source, Dest))
        {
          ShowJunction(aHwnd, Buffer, Dest, pReparseProperties);
        }
      }
    }
  }

  return FALSE;                       // Take the default focus handling.
}


BOOL OnApply ( HWND hwnd, PSHNOTIFY* phdr )
{
  ReparseProperties* pReparseProperties = (ReparseProperties*) GetWindowLongPtr ( hwnd, GWLP_USERDATA );

  if (REPARSE_POINT_UNDEFINED != pReparseProperties->Type)
  {
    wchar_t Destination[HUGE_PATH];
    GetDlgItemText(hwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_VALUE, Destination, HUGE_PATH);

    if (_wcsicmp(pReparseProperties->Target, Destination))
    {
      gLSESettings.ReadLSESettings(false);

      switch (pReparseProperties->Type)
      {
        case REPARSE_POINT_JUNCTION:
        {

          int rValue = ReplaceJunction(pReparseProperties->Source, Destination);
          if (ERROR_SUCCESS == rValue)
          {
            SetDlgItemText(hwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, Destination);
            pReparseProperties->SetTarget(Destination, REPARSE_POINT_JUNCTION);
          }
        }
        break;

        case REPARSE_POINT_SYMBOLICLINK:
        {
          ReplaceSymbolicLink(pReparseProperties->Source, Destination, true);
          
          // The absolute destination shall be stored so that Explore Target can take the value
          WCHAR SymlinkTarget[HUGE_PATH] = { 0 };
          int r = ResolveSymboliclink(pReparseProperties->Source, Destination, SymlinkTarget);

          if (S_OK == r)
            PathCanonicalize(Destination, SymlinkTarget);
          SetDlgItemText(hwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, Destination);
          pReparseProperties->SetTarget(Destination, REPARSE_POINT_SYMBOLICLINK);
        }
        break;

        case REPARSE_POINT_MOUNTPOINT:
        {
          int rValue = ReplaceMountPoint(pReparseProperties->Source, Destination);
          if (ERROR_SUCCESS == rValue)
          {
            SetDlgItemText(hwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, Destination);
            pReparseProperties->SetTarget(Destination, REPARSE_POINT_MOUNTPOINT);
          }
        }
        break;

        case REPARSE_POINT_FAIL:
        {
          // Directory. 
          BOOL  bMoved = MoveFile(pReparseProperties->Source, Destination);
          if (TRUE == bMoved)
          {
            SetDlgItemText(hwnd, IDC_PROPPAGE_LINKSHLEXT_REFTARGET_ABSOLUT_VALUE, Destination);
            pReparseProperties->SetTarget(Destination, REPARSE_POINT_MOUNTPOINT);
          }
        }
        break;
      }
    }
  }

  // Return PSNRET_NOERROR to allow the sheet to close if the user clicked OK.
  SetWindowLongPtr ( hwnd, DWLP_MSGRESULT, PSNRET_NOERROR );
  return TRUE;
}


STDMETHODIMP 
PropertySheetPage::
ReplacePage( 
    EXPPS uPageID,
    LPFNSVADDPROPSHEETPAGE pfnReplaceWith,
    LPARAM lParam
)
{
	return NOERROR;
}


