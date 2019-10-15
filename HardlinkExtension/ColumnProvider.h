/*
	Copyright (C) 1999-2010, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#ifndef _COLUMPROVIDER_34E27C3E_AF5E_4043_BC5B_DC858AC4C9C6
#define _COLUMPROVIDER_34E27C3E_AF5E_4043_BC5B_DC858AC4C9C6


class ColumnProvider : public IColumnProvider
{
public:
	ColumnProvider();
	virtual ~ColumnProvider();

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

	//IColumnProvider methods
    STDMETHODIMP Initialize(
		LPCSHCOLUMNINIT psci
	)
	{ return S_OK; }

    STDMETHODIMP GetColumnInfo(
		DWORD dwIndex,
		SHCOLUMNINFO* psci
	);

    STDMETHODIMP GetItemData(
		LPCSHCOLUMNID pscid,
		LPCSHCOLUMNDATA pscd,
		VARIANT* pvarData
	);
private:
	ULONG	m_cRef;
};


#endif
