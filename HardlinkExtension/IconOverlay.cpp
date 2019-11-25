/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#include "stdafx.h"

#include "IconOverlay.h"

extern LSESettings gLSESettings;


///////////////////////////////////////////////////////////////
// IconOverlayHardlinkClassFactory

IconOverlayHardlinkClassFactory::IconOverlayHardlinkClassFactory() : m_cRef(0L)
{
    g_cRefThisDll++;
}

IconOverlayHardlinkClassFactory::~IconOverlayHardlinkClassFactory()
{
    g_cRefThisDll--;
}

STDMETHODIMP IconOverlayHardlinkClassFactory::QueryInterface(REFIID riid,
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

STDMETHODIMP_(ULONG) IconOverlayHardlinkClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) IconOverlayHardlinkClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP
IconOverlayHardlinkClassFactory::
CreateInstance(
	LPUNKNOWN pUnkOuter,
	REFIID riid,
	LPVOID *ppvObj
)
{
    *ppvObj = NULL;

    // Shell extensions typically don't support aggregation (inheritance)
    if (pUnkOuter)
    	return CLASS_E_NOAGGREGATION;

	IconOverlayHardlink* pIconOverlayHardlink = new IconOverlayHardlink();
	return pIconOverlayHardlink->QueryInterface(riid, ppvObj);
}

STDMETHODIMP IconOverlayHardlinkClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}


//
// IconOverlayHardlink Methods
//
IconOverlayHardlink::IconOverlayHardlink() : m_cRef(0L)
{
	g_cRefThisDll++;
}

IconOverlayHardlink::~IconOverlayHardlink()
{
	g_cRefThisDll--;
}

STDMETHODIMP IconOverlayHardlink::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	*ppv = NULL;

  if (!(gLSESettings.GetFlags() & eHardlinkOverlay))
    if (IsEqualIID(riid, IID_IShellIconOverlayIdentifier) || IsEqualIID(riid, IID_IUnknown))
    {
      *ppv = (IShellIconOverlayIdentifier*)this;
      AddRef();
      return NOERROR;
    }

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) IconOverlayHardlink::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) IconOverlayHardlink::Release()
{
	if (--m_cRef)
		return m_cRef;

	delete this;
	return 0L;
}

int 
GetIconFromRegistry(
	HKEY		aHive,
	wchar_t*	aIconKey,
	int			aDefaultIconIndex,
	int*		pIconIndex,
	LPWSTR		pwszIconFile,
	int			aIconFileLength
)
{
	int	rr = S_FALSE;
	
	HKEY RegKey;
	DWORD RetVal = ::RegOpenKeyEx(
            aHive,
            LSE_REGISTRY_LOCATION,
            0,
            KEY_READ,
            &RegKey
    );

	if (ERROR_SUCCESS == RetVal)
	{
		DWORD aSize = MAX_PATH;
		DWORD aType = REG_SZ;
		RetVal = RegQueryValueEx(
				RegKey,
				aIconKey,
				0,
				&aType,
				(LPBYTE)pwszIconFile,
				&aSize
		);

		if (ERROR_SUCCESS == RetVal)
		{
			if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pwszIconFile))
			{
				GetModuleFileNameW(g_hInstance, pwszIconFile, aIconFileLength);
				HTRACE (L"LSE::IconOverlayHardlink::GetOverlayInfo file '%s' not found:\n", pwszIconFile, errno);
				*pIconIndex = aDefaultIconIndex;
			}
			else
			{
        HTRACE (L"LSE::IconOverlayHardlink::GetOverlayInfo Success '%s': %d\n", pwszIconFile, RetVal);
        rr = ERROR_SUCCESS;
				*pIconIndex = 0;
			}
		}
		else
		{
			HTRACE (L"LSE::IconOverlayHardlink::GetOverlayInfo RegQueryValue faild %d\n", RetVal);
			GetModuleFileNameW(g_hInstance, pwszIconFile, aIconFileLength);
			*pIconIndex = aDefaultIconIndex;
		}
		::RegCloseKey(RegKey);		
	}
	else
	{
		HTRACE (L"LSE::IconOverlayHardlink::GetOverlayInfo 2 %d\n", RetVal);
		GetModuleFileNameW(g_hInstance, pwszIconFile, aIconFileLength);
		*pIconIndex = aDefaultIconIndex;
	}

	return rr;
}
STDMETHODIMP 
IconOverlayHardlink::
GetOverlayInfo(
	 LPWSTR pwszIconFile,
	 int cchMax,
	 int* pIndex,
	 DWORD* pdwFlags
)
{
	HRESULT r = GetIconFromRegistry(HKEY_CURRENT_USER, LSE_REGISTRY_HARDLINK_ICON, 1, pIndex, pwszIconFile, cchMax);
	if (S_FALSE == r)
		GetIconFromRegistry(HKEY_LOCAL_MACHINE, LSE_REGISTRY_HARDLINK_ICON, 1, pIndex, pwszIconFile, cchMax);

  *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

	return S_OK;
}

STDMETHODIMP 
IconOverlayHardlink::
GetPriority(
	int* pPriority
)
{
  // highest priority
  *pPriority = gLSESettings.GetHardlinkOverlayPrio();
  return S_OK;
}

STDMETHODIMP 
IconOverlayHardlink::
IsMemberOf(
	LPCWSTR pwszPath, 
	DWORD dwAttrib
)
{
	HRESULT RetVal = S_FALSE; 
	DWORD	FileSystemFlags;
  int DriveType;
	if (IsFileSystemNtfs(pwszPath, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType))
	{
		// Before we check for hardlinks, see if it is a Symbolic Link
		// pointing to a hardlink. If this is true do not probe for
		// the reference count
		BOOL b = ProbeSymbolicLink(pwszPath, NULL);
		if (!b)
		{
			// ok it was a real hardlink, and not a symbolic link pointing
			// to a hardlink
			int r = ProbeHardlink(pwszPath);
			if (r > 1)
				RetVal = S_OK;
			else
				RetVal = S_FALSE;
		}
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////
// IconOverlaySymbolicLinkClassFactory

IconOverlaySymbolicLinkClassFactory::IconOverlaySymbolicLinkClassFactory() : m_cRef(0L)
{
    g_cRefThisDll++;
}

IconOverlaySymbolicLinkClassFactory::~IconOverlaySymbolicLinkClassFactory()
{
    g_cRefThisDll--;
}

STDMETHODIMP IconOverlaySymbolicLinkClassFactory::QueryInterface(REFIID riid,
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

STDMETHODIMP_(ULONG) IconOverlaySymbolicLinkClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) IconOverlaySymbolicLinkClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP
IconOverlaySymbolicLinkClassFactory::
CreateInstance(
	LPUNKNOWN pUnkOuter,
	REFIID riid,
	LPVOID *ppvObj
)
{
    *ppvObj = NULL;

    // Shell extensions typically don't support aggregation (inheritance)
    if (pUnkOuter)
    	return CLASS_E_NOAGGREGATION;

	IconOverlaySymbolicLink* pIconOverlaySymbolicLink = new IconOverlaySymbolicLink();
	return pIconOverlaySymbolicLink->QueryInterface(riid, ppvObj);
}

STDMETHODIMP IconOverlaySymbolicLinkClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}


//
// IconOverlaySymbolicLink Methods
//
IconOverlaySymbolicLink::IconOverlaySymbolicLink() : m_cRef(0L)
{
	g_cRefThisDll++;
}

IconOverlaySymbolicLink::~IconOverlaySymbolicLink()
{
	g_cRefThisDll--;
}

STDMETHODIMP IconOverlaySymbolicLink::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	*ppv = NULL;

  if (!(gLSESettings.GetFlags() & eSymboliclinkOverlay))
    if (IsEqualIID(riid, IID_IShellIconOverlayIdentifier) || IsEqualIID(riid, IID_IUnknown))
    {
      *ppv = (IShellIconOverlayIdentifier*)this;
      AddRef();
      return NOERROR;
    }

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) IconOverlaySymbolicLink::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) IconOverlaySymbolicLink::Release()
{
	if (--m_cRef)
		return m_cRef;

	delete this;
	return 0L;
}

STDMETHODIMP 
IconOverlaySymbolicLink::
GetOverlayInfo(
	 LPWSTR pwszIconFile,
	 int cchMax,
	 int* pIndex,
	 DWORD* pdwFlags
)
{
  HRESULT r = GetIconFromRegistry(HKEY_CURRENT_USER, LSE_REGISTRY_SYMBOLICLINK_ICON, 0, pIndex, pwszIconFile, cchMax);
	if (S_FALSE == r)
		GetIconFromRegistry(HKEY_LOCAL_MACHINE, LSE_REGISTRY_SYMBOLICLINK_ICON, 0, pIndex, pwszIconFile, cchMax);

	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

	return S_OK;
}

STDMETHODIMP 
IconOverlaySymbolicLink::
GetPriority(
	int* pPriority
)
{
  // highest priority
  *pPriority = gLSESettings.GetSymboliclinkOverlayPrio();
  return S_OK;
}

STDMETHODIMP 
IconOverlaySymbolicLink::
IsMemberOf(
	LPCWSTR pwszPath, 
	DWORD dwAttrib
)
{
	HRESULT RetVal = S_FALSE;

	// Check the filesystemtype
	DWORD	FileSystemFlags;
  int DriveType;
	if (IsFileSystemNtfs(pwszPath, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType))
	{
		if (FileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS)
		{
			BOOL r = ProbeSymbolicLink(pwszPath, NULL);
			if (r)
				// We go with a SymbolicLink
				RetVal = S_OK;
		}
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////
// IconOverlayJunctionClassFactory

IconOverlayJunctionClassFactory::IconOverlayJunctionClassFactory() : m_cRef(0L)
{
    g_cRefThisDll++;
}

IconOverlayJunctionClassFactory::~IconOverlayJunctionClassFactory()
{
    g_cRefThisDll--;
}

STDMETHODIMP IconOverlayJunctionClassFactory::QueryInterface(REFIID riid,
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

STDMETHODIMP_(ULONG) IconOverlayJunctionClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) IconOverlayJunctionClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP
IconOverlayJunctionClassFactory::
CreateInstance(
	LPUNKNOWN pUnkOuter,
	REFIID riid,
	LPVOID *ppvObj
)
{
  *ppvObj = NULL;

  // Shell extensions typically don't support aggregation (inheritance)
  if (pUnkOuter)
    return CLASS_E_NOAGGREGATION;

  IconOverlayJunction* pIconOverlayJunction = new IconOverlayJunction();
  return pIconOverlayJunction->QueryInterface(riid, ppvObj);
}

STDMETHODIMP IconOverlayJunctionClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

//
// IconOverlayJunction Methods
//
IconOverlayJunction::IconOverlayJunction() : m_cRef(0L)
{
	g_cRefThisDll++;
}

IconOverlayJunction::~IconOverlayJunction()
{
	g_cRefThisDll--;
}

STDMETHODIMP IconOverlayJunction::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	*ppv = NULL;

  if (!(gLSESettings.GetFlags() & eJunctionOverlay))
    if (IsEqualIID(riid, IID_IShellIconOverlayIdentifier) || IsEqualIID(riid, IID_IUnknown))
    {
      *ppv = (IShellIconOverlayIdentifier*)this;
      AddRef();
      return NOERROR;
    }

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) IconOverlayJunction::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) IconOverlayJunction::Release()
{
	if (--m_cRef)
		return m_cRef;

	delete this;
	return 0L;
}

STDMETHODIMP 
IconOverlayJunction::
GetOverlayInfo(
	 LPWSTR pwszIconFile,
	 int cchMax,
	 int* pIndex,
	 DWORD* pdwFlags
)
{
	HRESULT r = GetIconFromRegistry(HKEY_CURRENT_USER, LSE_REGISTRY_JUNCTION_ICON, 0, pIndex, pwszIconFile, cchMax);
	if (S_FALSE == r)
		GetIconFromRegistry(HKEY_LOCAL_MACHINE, LSE_REGISTRY_JUNCTION_ICON, 0, pIndex, pwszIconFile, cchMax);
	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;

	return S_OK;
}

STDMETHODIMP 
IconOverlayJunction::
GetPriority(
	int* pPriority
)
{
  // highest priority
  *pPriority = gLSESettings.GetJunctionOverlayPrio();
  return S_OK;
}

STDMETHODIMP 
IconOverlayJunction::
IsMemberOf(
	LPCWSTR pwszPath, 
	DWORD dwAttrib
)
{
	HRESULT RetVal = S_FALSE;

	// Check the filesystemtype
	DWORD	FileSystemFlags;
  int DriveType;
  bool IsNtfs = IsFileSystemNtfs(pwszPath, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);
#if 0
  wchar_t msg[HUGE_PATH];
  wsprintf(msg, L"##### Path:%s:%08x:%08x:%d:%08x", pwszPath, FileSystemFlags, DriveType, IsNtfs, gLSESettings.GetFlags());
  OutputDebugStringW(msg);
#endif
	if (IsNtfs)
	{
		if (FileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS)
		{
			BOOL r = ProbeJunction(pwszPath, NULL);
			if (r)
				// We go with a Junction
				RetVal = S_OK;
		}
	}
	return RetVal;
}

