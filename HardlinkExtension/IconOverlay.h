/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once


DEFINE_GUID(CLSID_IconOverlayHardLink, 0xa479751, 0x2bc, 0x11d3, 0xa8, 0x55, 0x0, 0x4, 0xac, 0x25, 0x68, 0xdd);

extern HINSTANCE g_hInstance;
extern UINT      g_cRefThisDll;

// IconOverlayHardlinkClassFactory
class IconOverlayHardlinkClassFactory : public IClassFactory
{
public:
	IconOverlayHardlinkClassFactory();
	virtual ~IconOverlayHardlinkClassFactory();

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

	//IClassFactory members
	STDMETHODIMP
	CreateInstance(
		LPUNKNOWN,
		REFIID,
		LPVOID FAR *
	);

	STDMETHODIMP
	LockServer(
		BOOL
	);
private:
	ULONG	m_cRef;
};

// IconOverlayHardlink
class IconOverlayHardlink : public IShellIconOverlayIdentifier
{
public:
	IconOverlayHardlink();
	virtual ~IconOverlayHardlink();

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

	//IIconOverlay methods
	STDMETHODIMP GetOverlayInfo(
				 LPWSTR pwszIconFile,
				 int cchMax,
				 int* pIndex,
				 DWORD* pdwFlags
	);

	STDMETHODIMP GetPriority(int* pPriority
	);

	STDMETHODIMP IsMemberOf(
		LPCWSTR pwszPath,
		DWORD dwAttrib
	);

private:
	ULONG	m_cRef;
};


DEFINE_GUID(CLSID_IconOverlaySymbolicLink, 0xa479751, 0x2bc, 0x11d3, 0xa8, 0x55, 0x0, 0x4, 0xac, 0x25, 0x68, 0xee);

// IconOverlayHardlinkClassFactory
class IconOverlaySymbolicLinkClassFactory : public IClassFactory
{
public:
	IconOverlaySymbolicLinkClassFactory();
	virtual ~IconOverlaySymbolicLinkClassFactory();

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

	//IClassFactory members
	STDMETHODIMP
	CreateInstance(
		LPUNKNOWN,
		REFIID,
		LPVOID FAR *
	);

	STDMETHODIMP
	LockServer(
		BOOL
	);
private:
	ULONG	m_cRef;
};

// IconOverlaySymbolicLink
class IconOverlaySymbolicLink : public IShellIconOverlayIdentifier
{
public:
	IconOverlaySymbolicLink();
	virtual ~IconOverlaySymbolicLink();

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

	//IIconOverlay methods
	STDMETHODIMP GetOverlayInfo(
				 LPWSTR pwszIconFile,
				 int cchMax,
				 int* pIndex,
				 DWORD* pdwFlags
	);

	STDMETHODIMP GetPriority(int* pPriority
	);

	STDMETHODIMP IsMemberOf(
		LPCWSTR pwszPath,
		DWORD dwAttrib
	);


private:
	ULONG	m_cRef;
};


DEFINE_GUID(CLSID_IconOverlayJunction, 0xa479751, 0x2bc, 0x11d3, 0xa8, 0x55, 0x0, 0x4, 0xac, 0x25, 0x68, 0xff);

// IconOverlayJunctionClassFactory
class IconOverlayJunctionClassFactory : public IClassFactory
{
public:
	IconOverlayJunctionClassFactory();
	virtual ~IconOverlayJunctionClassFactory();

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

	//IClassFactory members
	STDMETHODIMP
	CreateInstance(
		LPUNKNOWN,
		REFIID,
		LPVOID FAR *
	);

	STDMETHODIMP
	LockServer(
		BOOL
	);
private:
	ULONG	m_cRef;
};



// IconOverlayJunction
class IconOverlayJunction : public IShellIconOverlayIdentifier
{
public:
	IconOverlayJunction();
	virtual ~IconOverlayJunction();

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

	//IIconOverlay methods
	STDMETHODIMP GetOverlayInfo(
				 LPWSTR pwszIconFile,
				 int cchMax,
				 int* pIndex,
				 DWORD* pdwFlags
	);

	STDMETHODIMP GetPriority(int* pPriority
	);

	STDMETHODIMP IsMemberOf(
		LPCWSTR pwszPath,
		DWORD dwAttrib
	);


private:
	ULONG	m_cRef;
};
