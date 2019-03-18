#ifndef hardlinks_h_FFE41427_6A0A_46e5_AFA0_54547CF97526
#define hardlinks_h_FFE41427_6A0A_46e5_AFA0_54547CF97526

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

typedef BOOL (__stdcall *GetVolumePathNamesForVolumeName_t)(
	LPCWSTR lpszVolumeName,
	LPWCH lpszVolumePathNames,
	DWORD cchBufferLength,
	PDWORD lpcchReturnLength
);
GetVolumePathNamesForVolumeName_t pfnGetVolumePathNamesForVolumeName;

typedef BOOL (__stdcall *SetVolumeMountPoint_t)(
  LPCTSTR lpszVolumeMountPoint,
  LPCTSTR lpszVolumeName
);
SetVolumeMountPoint_t pfnSetVolumeMountPoint;

typedef BOOL (__stdcall *DeleteVolumeMountPoint_t)(
  LPCTSTR lpszVolumeMountPoint
);
DeleteVolumeMountPoint_t pfnDeleteVolumeMountPoint;



#if ! defined _DEBUG
typedef std::vector<int>	_Of;

int
FindFreeNumber(
	_Of&		of
);

void
Test_FindFreeNumber(
);
#endif


#endif

