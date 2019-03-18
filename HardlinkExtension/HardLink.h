/*
	Copyright (C) 1999_2006, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/


#ifndef _HARDLINK_39ABD724_8121_4bd1_BD66_3A0BADAC9767
#define _HARDLINK_39ABD724_8121_4bd1_BD66_3A0BADAC9767

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


#endif

/////////////////////////////////////////////////////////////////////////////
