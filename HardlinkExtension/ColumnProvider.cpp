/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */


#include "stdafx.h"

#include "localisation.h"
#include "ColumnProvider.h"

extern UINT      g_cRefThisDll;    // Reference count of this DLL.
extern PWCHAR  TopMenuEntries[eTopMenu__Free__];

ColumnProvider::ColumnProvider() : m_cRef(0L)
{
	g_cRefThisDll++;
}

ColumnProvider::~ColumnProvider()
{
	g_cRefThisDll--;
}

STDMETHODIMP ColumnProvider::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	*ppv = NULL;

    if (IsEqualIID(riid, IID_IColumnProvider) || IsEqualIID(riid, IID_IUnknown))
	{
		*ppv = (IColumnProvider*)this;
		AddRef();
		return NOERROR;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ColumnProvider::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) ColumnProvider::Release()
{
	if (--m_cRef)
		return m_cRef;

	delete this;
	return 0L;
}

//
// ColumProvider Methods
//
STDMETHODIMP 
ColumnProvider::
GetColumnInfo(
	DWORD dwIndex, 
	SHCOLUMNINFO* psci
)
{
  if ( 0 == dwIndex )
  {
    psci->scid.fmtid = CLSID_HardLinkExtension; 
    psci->scid.pid   = 0;
    psci->vt         = VT_INT;
    psci->fmt        = LVCFMT_RIGHT;
    psci->csFlags    = SHCOLSTATE_TYPE_INT | SHCOLSTATE_SLOW | SHCOLSTATE_ONBYDEFAULT;  
    psci->cChars     = 12;                  // Default col width in chars

    lstrcpynW ( psci->wszTitle, TopMenuEntries[eTopMenuReference], MAX_COLUMN_NAME_LEN ); 
    lstrcpynW ( psci->wszDescription, TopMenuEntries[eTopMenuReferenceHelpText], MAX_COLUMN_DESC_LEN );
    return S_OK;
  }
  else
  {
    return S_FALSE;
  }
}

STDMETHODIMP 
ColumnProvider::
GetItemData(
	LPCSHCOLUMNID pscid, 
	LPCSHCOLUMNDATA pscd, 
	VARIANT* pvarData
)
{
  if ( pscid->fmtid == CLSID_HardLinkExtension )
  {
    // return on offline files
    if ( pscd->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE )
      return S_FALSE;

    // Add Destination for directories
    if ( pscd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {
      WCHAR Dest[HUGE_PATH] = { 0 };
      if (ProbeJunction((PWCHAR)pscd->wszFile, Dest))
      {
        pvarData->vt = VT_BSTR;

        if (wcslen(Dest) > MAX_COLUMN_NAME_LEN)
        {
          wchar_t ShrinkedDest[HUGE_PATH];
          PathCompactPathEx(ShrinkedDest, Dest, MAX_COLUMN_NAME_LEN, 0);
          pvarData->bstrVal = SysAllocString(ShrinkedDest);
        }
        else
          pvarData->bstrVal = SysAllocString(Dest);
        return S_OK;
      }
      else
      {
        // Add Destination for Volume mountpoints
        WCHAR	VolumeName[HUGE_PATH] = { 0 };
        if (ProbeMountPoint((PWCHAR)pscd->wszFile, Dest, HUGE_PATH, VolumeName))
        {
          pvarData->vt = VT_BSTR;
          pvarData->bstrVal = SysAllocString(Dest);
          return S_OK;
        }
        else
          return S_FALSE;
      }
    }

    int RefCount = ProbeHardlink(pscd->wszFile);
    if ( RefCount < 0 ) 
      return S_FALSE;
    //		TRACE (L"ColumnProvider::GetItemData, '%s' refCount = %d\n", pscd->wszFile, RefCount);

    pvarData->vt = VT_I4;
    pvarData->intVal = RefCount;

    return S_OK;
  }
  return S_FALSE;
}


