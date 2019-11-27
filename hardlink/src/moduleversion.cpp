////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CModuleVersion provides an easy way to get version info
// for a module.(DLL or EXE).
//

#include "StdAfx.h"
#include "ModuleVersion.h"

CModuleVersion::CModuleVersion() : m_pVersionInfo{ nullptr }
{
}

//////////////////
// Destroy: delete version info
//
CModuleVersion::~CModuleVersion()
{
  delete [] m_pVersionInfo;
}

BOOL CModuleVersion::GetFileVersionInfoWithoutLoadDll(LPCTSTR modulename)
{
	m_translation.charset = 1252;		// default = ANSI code page
	memset((VS_FIXEDFILEINFO*)this, 0, sizeof(VS_FIXEDFILEINFO));

	// get module handle
	TCHAR filename[_MAX_PATH];
  wcscpy_s(filename, _MAX_PATH, modulename);
	// read file version info
	DWORD dwDummyHandle; // will always be set to zero
	DWORD len = GetFileVersionInfoSize(filename, &dwDummyHandle);
	if (len <= 0)
		return FALSE;

	m_pVersionInfo = new BYTE[len]; // allocate version info
	if (!::GetFileVersionInfo(filename, 0, len, m_pVersionInfo))
		return FALSE;

	LPVOID lpvi;
	UINT iLen;
	if (!VerQueryValue(m_pVersionInfo, _T("\\"), &lpvi, &iLen))
		return FALSE;

	// copy fixed info to myself, which am derived from VS_FIXEDFILEINFO
	*(VS_FIXEDFILEINFO*)this = *(VS_FIXEDFILEINFO*)lpvi;

	// Get translation info
	if (VerQueryValue(m_pVersionInfo,
		_T("\\VarFileInfo\\Translation"), &lpvi, &iLen) && iLen >= 4) {
		m_translation = *(TRANSLATION*)lpvi;
	}
   // FreeLibrary(hModule);
	return dwSignature == VS_FFI_SIGNATURE;
}
//////////////////
// Get file version info for a given module
// Allocates storage for all info, fills "this" with
// VS_FIXEDFILEINFO, and sets codepage.
//
BOOL 
CModuleVersion::
GetFileVersionInfo(
	LPCTSTR		modulename
)
{
	if (NULL == modulename)
    return FALSE;

  m_translation.charset = 1252;		// default = ANSI code page
  memset((VS_FIXEDFILEINFO*)this, 0, sizeof(VS_FIXEDFILEINFO));

  // get module handle
  TCHAR filename[_MAX_PATH];
  wcscpy_s(filename, _MAX_PATH, modulename);
  HMODULE		hModule = GetModuleHandle(modulename);
  bool		Loaded = FALSE;
  if (hModule == NULL && modulename != NULL)
  {
    hModule = LoadLibraryEx(modulename, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if(!hModule)
      return FALSE;
    Loaded = TRUE;
  }

  // get module file name
  DWORD		len = GetModuleFileName(hModule, filename, sizeof(filename)/sizeof(filename[0]));
  if (len <= 0)
  {
    if (Loaded)
      FreeLibrary(hModule);
    return FALSE;
  }

	// read file version info
	DWORD		dwDummyHandle; // will always be set to zero
	len = GetFileVersionInfoSize(filename, &dwDummyHandle);
	if (len <= 0)
	{
		if (Loaded)
			FreeLibrary(hModule);
		return FALSE;
	}

	m_pVersionInfo = new BYTE[len]; // allocate version info
	if (!::GetFileVersionInfo(filename, 0, len, m_pVersionInfo))
	{
		delete [] m_pVersionInfo;
		m_pVersionInfo = NULL;
		if (Loaded)
			FreeLibrary(hModule);
		return FALSE;
	}

	LPVOID		lpvi;
	UINT		iLen;
	if (!VerQueryValue(m_pVersionInfo, _T("\\"), &lpvi, &iLen))
	{
		delete [] m_pVersionInfo;
		m_pVersionInfo = NULL;
		if (Loaded)
			FreeLibrary(hModule);
		return FALSE;
	}

	// copy fixed info to myself, which am derived from VS_FIXEDFILEINFO
	*(VS_FIXEDFILEINFO*)this = *(VS_FIXEDFILEINFO*)lpvi;

	// Get translation info
	if (VerQueryValue(m_pVersionInfo, _T("\\VarFileInfo\\Translation"), &lpvi, &iLen) && iLen >= 4) 
	{
		m_translation = *(TRANSLATION*)lpvi;
	}
	
	// Check if we got the handle to the module via GetModuleHandle
	// or via LoadLibrary. Only if we got the handle via LoadLibrary
	// we are allowed to free the library. Otherwise .dlls of the 
	// running process would be unmapped, and lead to a crash.
	if (Loaded)
		FreeLibrary(hModule);
	
	return dwSignature == VS_FFI_SIGNATURE;
}

//////////////////
// Get string file info.
// Key name is something like "CompanyName".
//
void	
CModuleVersion::
GetValue(
	LPCTSTR		lpKeyName, 
	LPTSTR		lpValue,	
	DWORD		nSize
)
{
	if (m_pVersionInfo)
	{
		TCHAR query[_MAX_PATH];
		_stprintf_s(query, _MAX_PATH, _T("\\StringFileInfo\\%04x%04x\\%s"),
		m_translation.langID,
		m_translation.charset,
		lpKeyName);

		LPCTSTR pVal;
		UINT iLenVal;
		if (VerQueryValue(m_pVersionInfo, query, (LPVOID*)&pVal, &iLenVal)) 
		{
			int len = iLenVal < nSize ? iLenVal : nSize;
			wcsncpy(lpValue, pVal, len);
			lpValue[len] = 0;
		}
	}
	else
		nSize = (DWORD)-1;
}

