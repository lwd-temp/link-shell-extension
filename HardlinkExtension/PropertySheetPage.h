/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once



DEFINE_GUID(CLSID_HardLinkPropertySheetPage, 0xa479751, 0x2bc, 0x11d3, 0xa8, 0x55, 0x0, 0x4, 0xac, 0x25, 0x68, 0xcc);

INT_PTR CALLBACK PropPageDlgProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
UINT CALLBACK PropPageCallbackProc ( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
BOOL OnInitDialog ( HWND hwnd, LPARAM lParam );
void OnExploreTarget ( HWND hwnd, LPARAM lParam );
BOOL OnApply ( HWND hwnd, PSHNOTIFY* phdr );


// PropertySheetPageClassFactory
class PropertySheetPageClassFactory : public IClassFactory
{
public:
	PropertySheetPageClassFactory();
	virtual ~PropertySheetPageClassFactory();

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

// PropertySheetPage
class PropertySheetPage : public IShellPropSheetExt, IShellExtInit
{
public:
	PropertySheetPage();
	virtual ~PropertySheetPage();

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

	//IShellExtInit methods
	STDMETHODIMP
	Initialize(
		LPCITEMIDLIST			pIDFolder,
		LPDATAOBJECT			pDataObj,
		HKEY					hKeyID
	);

	STDMETHODIMP 
	AddPages (
	  LPFNADDPROPSHEETPAGE lpfnAddPageProc,
	  LPARAM lParam 
	);

	STDMETHODIMP 
	ReplacePage( 
        EXPPS uPageID,
        LPFNSVADDPROPSHEETPAGE pfnReplaceWith,
        LPARAM lParam
	);

private:
	wchar_t						m_File[MAX_PATH];
	ULONG						m_cRef;


};

struct ReparseProperties
{
  ReparseProperties() 
  {
    Init();
  };
  
  ReparseProperties(const wchar_t*  pSource)
  {
    Init();

    size_t  Len = wcslen(pSource) + 1;
    Source = new wchar_t[Len];
    wcscpy_s(Source, Len, pSource);
  };
  
  void
  Init() 
  {
    Source = nullptr;
    Target = nullptr;
    Type = REPARSE_POINT_UNDEFINED;
  };
  
  wchar_t* Source;
  wchar_t* Target;
  int Type;

  void
  SetTarget(
    wchar_t*  pTarget,
    int       aType
  )
  {
    size_t  Len = wcslen(pTarget) + 1;
    Target = new wchar_t[Len];
    wcscpy_s(Target, Len, pTarget);

    Type = aType;
  };


  ~ReparseProperties() 
  {
    delete [] Source;
    delete [] Target;
  };

};

