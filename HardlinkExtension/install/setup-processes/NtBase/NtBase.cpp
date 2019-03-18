#include "stdafx.h"
#include "NtBase.h"
#include "string.h"
#include "NtSystemInfo.h"



#include <psapi.h>
#pragma comment(lib, "psapi.lib")


_locale_t         g_locale_t;


/***
*wchar_t *wcseistr(string1, string2) - search for string2 in string1 ignoring the case
*       (wide strings)
*
*Purpose:
*       finds the first occurrence of string2 in string1 (wide strings)
*
*Entry:
*       wchar_t *string1 - string to search in
*       wchar_t *string2 - string to search for
*
*Exit:
*       returns a pointer to the first occurrence of string2 in
*       string1, or NULL if string2 does not occur in string1
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/

wchar_t * __cdecl wcseistr (
        const wchar_t * wcs1,
        const wchar_t * wcs2
        )
{
  wchar_t *cp = (wchar_t *) wcs1;
  wchar_t *s1, *s2;

  if ( !*wcs2)
    return (wchar_t *)wcs1;

  while (*cp)
  {
    s1 = cp;
    s2 = (wchar_t *) wcs2;

    while ( *s1 && *s2 && !(_towlower_l((unsigned short)(*s1), g_locale_t) - _towlower_l((unsigned short)(*s2), g_locale_t)) )
      s1++, s2++;

    if (!*s2)
      return(s1);

    cp++;
  }

  return(NULL);
}


bool
NtQueryProcessId(
	PWCHAR		aProcessName,
	PULONG*		lpProcessIdBuff,
	PULONG		dwProcessIdBuff
)
{
	int			found = false;
	NTSTATUS	status;
	int			BufferSize = 0x10000;
	PVOID		pBuffer;
	ULONG		ulSize;

	/* Get process information
	 * We don't know how much data there is so just keep
	 * increasing the buffer size until the call succeeds
	 */
	do
	{
		BufferSize += 0x10000;
		pBuffer = (LPBYTE)malloc(BufferSize);

		status = NtQuerySystemInformation(SystemProcessListInformation, pBuffer, BufferSize, &ulSize);

		if (status == 0xC0000004 /*STATUS_INFO_LENGTH_MISMATCH*/) {
			free(pBuffer);
		}

	} while (status == 0xC0000004 /*STATUS_INFO_LENGTH_MISMATCH*/);

	// Allocate some buffer to hold the return strings
	//
	const int PROCESS_NAMES_BUFF_SIZE = 2048;
	*lpProcessIdBuff = (PULONG)GlobalAlloc(
		GPTR ,
		PROCESS_NAMES_BUFF_SIZE
	);

	*dwProcessIdBuff = 0;
	PSYSTEM_PROCESS_INFORMATION	pProcess = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
	do
	{
    if (pProcess->ImageName.Length)
		{
      if ( !_wcsicmp(aProcessName, pProcess->ImageName.Buffer) )
			{
				(*lpProcessIdBuff)[*dwProcessIdBuff] = (DWORD)pProcess->UniqueProcessId;
				(*dwProcessIdBuff)++;
			}
		}

		UINT	Offset = pProcess->NextEntryOffset;
		pProcess = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)pProcess + Offset);
	}
	while (pProcess->NextEntryOffset != 0 && *dwProcessIdBuff < PROCESS_NAMES_BUFF_SIZE / sizeof(ULONG));

	free (pBuffer);

	return *dwProcessIdBuff ? true : false;
}

bool
NtQueryProcessByModule(
  _StringList&  aModuleNames,
  _StringMap &  aProcessNames
)
{
	int			found = false;
	NTSTATUS	status;
	int			BufferSize = 0x10000;
	PVOID		pBuffer;
	ULONG		ulSize;

	/* Get process information
	 * We don't know how much data there is so just keep
	 * increasing the buffer size until the call succeeds
	 */
	do
	{
		BufferSize += 0x10000;
		pBuffer = (LPBYTE)malloc(BufferSize);

		status = NtQuerySystemInformation(SystemProcessListInformation, pBuffer, BufferSize, &ulSize);

		if (status == 0xC0000004 /*STATUS_INFO_LENGTH_MISMATCH*/) {
			free(pBuffer);
		}

	} while (status == 0xC0000004 /*STATUS_INFO_LENGTH_MISMATCH*/);


  // Use K32EnumProcessModulesEx from kernel32.lib
  //
  K32EnumProcessModulesEx_t pfnK32EnumProcessModulesEx = NULL;
  HMODULE	hKernel32 = LoadLibrary (L"kernel32.dll");
  if (hKernel32 > (HMODULE) 32)
    pfnK32EnumProcessModulesEx = (K32EnumProcessModulesEx_t)GetProcAddress (hKernel32, "K32EnumProcessModulesEx");

	// Run through all processes
	//
	PSYSTEM_PROCESS_INFORMATION	pProcess = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
	do
	{
    if (pProcess->ImageName.Length)
		{
      HMODULE hMods[1024];
      DWORD cbNeeded;
      unsigned int i;


//      printf( "\nProcess ID: %u: %S\n", pProcess->UniqueProcessID,pProcess->pszProcessName );
      DWORD err;
      HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)pProcess->UniqueProcessId);
      DWORD errrr = GetLastError();
      if (hProcess != NULL)
      {
        // With Windows7 try to use the functions from kernel32
        bool bEnum;
        if (pfnK32EnumProcessModulesEx)
          bEnum = pfnK32EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL);
        else
          // if not available use the psapi functions, but they do not always work to the full extent
          // e.g. they deliver less hMods
          bEnum = EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded);

        DWORD LastError = GetLastError();
        if ( bEnum )
        {
          for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
          {
            wchar_t szModName[MAX_PATH];
            if ( GetModuleFileNameExW(hProcess, hMods[i], szModName, sizeof(szModName)))
            {
              for (_StringList::iterator ii = aModuleNames.begin(); ii != aModuleNames.end(); ++ii)
              {
                if (wcseistr(szModName, (*ii).c_str() ) )
                {
                  if (aProcessNames.find(pProcess->ImageName.Buffer) == aProcessNames.end())
                    std::pair< _StringMap::iterator, bool > pr = aProcessNames.insert(_StringMapPair(pProcess->ImageName.Buffer, 42));
                  // printf("\t%S\t%S \n", pProcess->pszProcessName, szModName);
                }
                  // printf("\t%S\t%S\t%S \n", pProcess->pszProcessName, szModName, (*ii).c_str());
              }
            }
          }
        }
        else
        {
          err = GetLastError();
        }
        CloseHandle( hProcess );
      }
    }

		UINT	Offset = pProcess->NextEntryOffset;
		pProcess = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)pProcess + Offset);
	}
	while (pProcess->NextEntryOffset != 0 );

  if (hKernel32 > (HMODULE) 32)
    FreeLibrary(hKernel32);

	return true;
}



#if defined _NTQUERYHANDLE
//--------------------------------------------------------------------
//
// NtQueryHandle
//
//
//--------------------------------------------------------------------
int
NtQueryHandle(
  ULONG	        aProcessID,
  _StringList& aObjectNameList,
  _LongList&   aHandles
)
{
  NTSTATUS	                  status;
  int			                    BufferSize = 0x10000;
  PSYSTEM_HANDLE_INFORMATION  pBuffer;
  ULONG		                    ulSize;

  // Check if we are allowed to open the process
  HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, aProcessID);
  if (INVALID_HANDLE_VALUE == hProcess)
    return 1;

  /* Get process information
  * We don't know how much data there is so just keep
  * increasing the buffer size until the call succeeds
  */
  do
  {
    BufferSize += 0x10000;
    pBuffer = (PSYSTEM_HANDLE_INFORMATION)malloc(BufferSize);
    status = NtQuerySystemInformation(SystemHandleInformation, pBuffer, BufferSize, &ulSize);

    if (status == STATUS_INFO_LENGTH_MISMATCH)
      free(pBuffer);

  } while (status == STATUS_INFO_LENGTH_MISMATCH);

  HANDLE dupHandle;
  ULONG objectNameInfoSize = 0x10000;
  POBJECT_TYPE_INFORMATION pObjectNameInfo = (POBJECT_TYPE_INFORMATION)malloc(objectNameInfoSize);

  // Process all handles
  for (ULONG i = 0; i < pBuffer->HandleCount; i++)
  {
    SYSTEM_HANDLE_TABLE_ENTRY_INFO handle = pBuffer->Handles[i];

    // Does this handle belong to the given process?
    if (handle.ProcessId != aProcessID)
      continue;

    // There is a bug in XP, which causes calls to NtQueryObject to hang. Be careful.
    if (handle.GrantedAccess != 0x0012019f)
    {
      // Duplicate the handle so that we can query the handle
      status = DuplicateHandle(hProcess, (HANDLE)handle.Handle, GetCurrentProcess(), &dupHandle, 0, 0, 0);
      if (NT_SUCCESS(status))
      {
        // Query the handle on further information
        status = NtQueryObject(dupHandle, ObjectNameInformation, pObjectNameInfo, objectNameInfoSize, NULL);
        if (NT_SUCCESS(status))
        {
          // Check the returned object if it maps the objectnames 
          for (_StringList::iterator ii = aObjectNameList.begin(); ii != aObjectNameList.end(); ii++)
          {
//            if (StrStrIW(pObjectNameInfo->Name.Buffer, (*ii).c_str()))
//            if ((pObjectNameInfo->Name.Buffer, (*ii).c_str()))
              ;
              // aHandles.push_back((DWORD)handle.Handle);
          }
        }
        CloseHandle(dupHandle);  
      }
    }
  }

  free(pObjectNameInfo);
  CloseHandle(hProcess);
  free(pBuffer);

  return 42;
}

#endif // _NTQUERYHANDLE



