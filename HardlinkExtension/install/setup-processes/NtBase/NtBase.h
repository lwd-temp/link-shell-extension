
#pragma once

//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)

#include <vector>
#include <map>
#include <string>

using namespace std;

typedef pair<wstring, DWORD> _StringMapPair;
typedef map<wstring, DWORD> _StringMap;
typedef vector<wstring> _StringList;
typedef vector<DWORD> _LongList;




wchar_t * __cdecl 
wcseistr (
  const wchar_t * wcs1,
  const wchar_t * wcs2
);

bool
NtQueryProcessId(
	PWCHAR		aProcessName,
	PULONG*		lpProcessIdBuff,
	PULONG		dwProcessIdBuff
);

bool
NtQueryProcessByModule(
  _StringList&  aModuleNames,
  _StringMap&  aProcessNames
);

typedef BOOL (WINAPI *K32EnumProcessModulesEx_t)(
  IN    HANDLE hProcess,
  OUT   HMODULE *lphModule,
  IN    DWORD cb,
  OUT   LPDWORD lpcbNeeded,
  IN    DWORD dwFilterFlag
);

#define LIST_MODULES_32BIT 0x01
#define LIST_MODULES_64BIT 0x02
