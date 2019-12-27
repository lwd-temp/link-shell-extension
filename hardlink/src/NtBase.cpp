/*
  Copyright (C) 1999 - 2020, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"

#include "hardlink_types.h"

#include "NtBase.h"

extern "C" { int __locale_changed; }
_locale_t g_locale_t;


bool
NtQueryProcessId(
  PCWCH		    aProcessName,
  PUINT_PTR*	lpProcessIdBuff,
  PULONG		  dwProcessIdBuff
)
{
  NTSTATUS	status = STATUS_INVALID_PARAMETER;
  ULONG		BufferSize = 0x80000;
  PVOID		pBuffer;
  ULONG		ulSize = 0;

  // Get process information. We don't know how much data there is so just keep
  // increasing the buffer size until the call succeeds
  do
  {
    pBuffer = (LPBYTE)malloc(BufferSize);
    if (!pBuffer)
      break;

    status = NtQuerySystemInformation(SystemProcessListInformation, pBuffer, BufferSize, &ulSize);

    if (status == STATUS_INFO_LENGTH_MISMATCH)
    {
      BufferSize += 0x20000;
      free(pBuffer);
    }

  } while (status == STATUS_INFO_LENGTH_MISMATCH);

  if (NT_SUCCESS(status))
  {
    // Allocate some buffer to hold the return strings
    //
    const int PROCESS_NAMES_BUFF_SIZE = 2048;
    *lpProcessIdBuff = (PUINT_PTR)GlobalAlloc(
      GPTR,
      PROCESS_NAMES_BUFF_SIZE
    );

    *dwProcessIdBuff = 0;
    WCHAR LwrProcessName[MAX_PATH];
    wcscpy_s(LwrProcessName, MAX_PATH, aProcessName);
    _wcslwr_s(LwrProcessName, MAX_PATH);

    PSYSTEM_PROCESS_INFORMATION	pCurrentProcess;
    PSYSTEM_PROCESS_INFORMATION	pProcess = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
    do
    {
      if (pProcess->ImageName.Length)
      {
        _wcslwr_s(pProcess->ImageName.Buffer, pProcess->ImageName.Length);
        if (!_wcsicmp(LwrProcessName, pProcess->ImageName.Buffer))
        {
          (*lpProcessIdBuff)[*dwProcessIdBuff] = pProcess->UniqueProcessId;
          (*dwProcessIdBuff)++;
        }
      }

      pCurrentProcess = pProcess;
      UINT	Offset = pProcess->NextEntryOffset;
      pProcess = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)pProcess + Offset);
    } while (pCurrentProcess->NextEntryOffset != 0 && *dwProcessIdBuff < PROCESS_NAMES_BUFF_SIZE / sizeof(ULONG));
  }

  if (pBuffer)
    free(pBuffer);

  return *dwProcessIdBuff ? true : false;
}

bool
NtQueryProcessByModule(
  _StringList&  aModuleNames,
  _StringSet &  aProcessNames
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

    if (status == STATUS_INFO_LENGTH_MISMATCH) {
      free(pBuffer);
    }

  } while (status == STATUS_INFO_LENGTH_MISMATCH);


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
        BOOL bEnum = K32EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_ALL);
        DWORD LastError = GetLastError();
        if (bEnum)
        {
          for (i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i)
          {
            wchar_t szModName[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, hMods[i], szModName, _countof(szModName)))
            {
              for (const auto& ii : aModuleNames)
              {
                if (wcseistr(szModName, ii.c_str()))
                {
                  if (aProcessNames.find(pProcess->ImageName.Buffer) == aProcessNames.end())
                  {
                    aProcessNames.insert(pProcess->ImageName.Buffer);
                    // printf("\t%S\t%S \n", pProcess->pszProcessName, szModName);
                  }
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
        CloseHandle(hProcess);
      }
    }

    UINT	Offset = pProcess->NextEntryOffset;
    pProcess = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)pProcess + Offset);
  } while (pProcess->NextEntryOffset != 0);

  return true;
}

/***
*wchar_t *wcseistr(string1, string2) - search for string2 in string1
*       (wide strings)
*
*Purpose:
*       finds the first occurrence of string2 in string1 (wide strings)
*       derived from wcsstr()
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
#define __ascii_towlower(c)     ( (((c) >= L'A') && ((c) <= L'Z')) ? ((c) - L'A' + L'a') : (c) )

wchar_t * __cdecl wcseistr(
  const wchar_t * wcs1,
  const wchar_t * wcs2
)
{
  if (__locale_changed == 0)
  {
    wchar_t *cp = (wchar_t *)wcs1;
    wchar_t *s1, *s2;

    if (!*wcs2)
      return (wchar_t *)wcs1;

    while (*cp)
    {
      s1 = cp;
      s2 = (wchar_t *)wcs2;

      while (*s1 && *s2 && !(__ascii_towlower(*s1) - __ascii_towlower(*s2)))
        s1++, s2++;

      if (!*s2)
        return(s1);

      cp++;
    }
  }
  else
  {
    wchar_t *cp = (wchar_t *)wcs1;
    wchar_t *s1, *s2;

    if (!*wcs2)
      return (wchar_t *)wcs1;

    while (*cp)
    {
      s1 = cp;
      s2 = (wchar_t *)wcs2;

      while (*s1 && *s2 && !(_towlower_l((unsigned short)(*s1), g_locale_t) - _towlower_l((unsigned short)(*s2), g_locale_t)))
        s1++, s2++;

      if (!*s2)
        return(s1);

      cp++;
    }
  }

  return(NULL);
}

/***
*wchar_t *wcsesc_s(string1, string2, size, esc) - escapes character esc
*       (wide strings)
*
*Purpose:
*       find the charater esc and doubles it
*
*Entry:
*       wchar_t *string1 - string to search in
*       wchar_t *string2 - string to copy to
*       wchar_t  esc     - character to escape
*
*Exit:
*       The address of "dst"
*
*Exceptions:
*
*******************************************************************************/
wchar_t * __cdecl wcsesc_s(
  wchar_t * dst,
  const wchar_t * src,
  size_t _SIZE,
  const wchar_t search
)
{
  wchar_t * cp = dst;
  size_t size = 0;

  while ((size < _SIZE) && (*cp = *src))
  {
    if (*src == search)
    {
      *(++cp) = search;
      size++;
    }
    src++;
    cp++;
    size++;
  }

  return (dst);
}



