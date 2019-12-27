/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

DEFINE_GUID(CLSID_HardLinkCopyhook, 0xa479751, 0x2bc, 0x11d3, 0xa8, 0x55, 0x0, 0x4, 0xac, 0x25, 0x68, 0xbb);

// CopyHookClassFactory
class CopyHookClassFactory : public IClassFactory
{
public:
	CopyHookClassFactory();
	virtual ~CopyHookClassFactory();

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


// CopyHook
class CopyHook : public ICopyHook
{
public:
	CopyHook();
	virtual ~CopyHook();

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


    STDMETHODIMP_(UINT)
	CopyCallback ( HWND hwnd,
		UINT wFunc,
		UINT wFlags,
		LPCWSTR pszSrcFile,
		DWORD dwSrcAttribs,
		LPCWSTR pszDestFile,
		DWORD dwDestAttribs
	);

private:
	ULONG	m_cRef;
};

