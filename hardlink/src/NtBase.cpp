/*
  Copyright (C) 1999 - 2019, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"

#include "hardlink_types.h"
#include "LSESettings.h"
#include "DbgHelpers.h"

#include "MmfObject.h"

#include "Multilang.h"
#include "AsyncContext.h"

#include "hardlinks.h"
#include "HardlinkUtils.h"

#include "NtBase.h"

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
    wcsncpy(LwrProcessName, aProcessName, MAX_PATH);
    wcslwr(LwrProcessName);

    PSYSTEM_PROCESS_INFORMATION	pCurrentProcess;
    PSYSTEM_PROCESS_INFORMATION	pProcess = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
    do
    {
      if (pProcess->ImageName.Length)
      {
        if (!wcsicmp(LwrProcessName, wcslwr(pProcess->ImageName.Buffer)))
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
            if (GetModuleFileNameExW(hProcess, hMods[i], szModName, sizeof(szModName)))
            {
              for (const auto& ii : aModuleNames)
              {
                //                if (wcseistr(szModName, (*ii).c_str()))
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


