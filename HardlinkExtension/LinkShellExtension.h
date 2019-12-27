/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */


#pragma once


void
LoadMlgTexts(
  int aLanguage
);

void
FreeMlgTexts(
);

int 
GetLanguage(
);

// this class factory object creates context menu handlers for Windows 95 shell
class HardLinkExtClassFactory : public IClassFactory
{
public:
	HardLinkExtClassFactory();
	virtual ~HardLinkExtClassFactory();

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

