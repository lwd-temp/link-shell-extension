/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#if !defined EXTERN
#define EXTERN extern
#endif


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

