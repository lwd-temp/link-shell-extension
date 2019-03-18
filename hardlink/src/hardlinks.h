/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

#ifdef UNICODE
#define CREATEHARDLINK "CreateHardLinkW"
#else
#define CREATEHARDLINK "CreateHardLinkA"
#endif

#ifdef UNICODE
#define CREATESYMBOLICLINK "CreateSymbolicLinkW"
#else
#define CREATESYMBOLICLINK "CreateSymbolicLinkA"
#endif

#define ISUSERANADMIN "IsUserAnAdmin"


#define EXTERN

enum PrivilegeModification_t
{
  eClearPrivilege,
  eSetPrivilege,
  eProbePrivilege
} ;



DWORD
CreateHardLinkNt4(const TCHAR* fromFile,
				  const TCHAR* toFile);


#if ! defined _DEBUG
typedef vector<int>	_Of;

int
FindFreeNumber(
	_Of&		of
);

void
Test_FindFreeNumber(
);
#endif
