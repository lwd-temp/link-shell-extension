/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

bool
NtQueryProcessId(
  PCWCH		  aProcessName,
  PUINT_PTR*		lpProcessIdBuff,
  PULONG		dwProcessIdBuff
);

bool
NtQueryProcessByModule(
  _StringList&  aModuleNames,
  _StringSet&  aProcessNames
);

wchar_t * __cdecl wcseistr(
  const wchar_t * wcs1,
  const wchar_t * wcs2
);

wchar_t * __cdecl wcsesc_s(
  wchar_t * dst,
  const wchar_t * src,
  size_t _SIZE,
  const wchar_t search
);

