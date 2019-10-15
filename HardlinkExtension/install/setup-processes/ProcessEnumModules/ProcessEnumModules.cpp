// ProcessEnumModules.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "hardlink_types.h"

#include "NtBase.h"


int _tmain(int argc, _TCHAR* argv[])
{
  _StringList Modules;
  _StringSet  Processes;

  Modules.push_back(L"HardlinkShellExt.dll");
  NtQueryProcessByModule(Modules, Processes);

  wchar_t TempPath[MAX_PATH];
  GetTempPath(MAX_PATH, TempPath);
  wcscat_s(TempPath, MAX_PATH, L"LseInstallerCollidingProcesses.txt");

  FILE* processes = _wfopen (TempPath, L"wb");
  if (processes)
  {
    for (const auto& iter : Processes)
      fwprintf (processes, L"%s\n", iter.c_str());

    fclose(processes);
  }

  return 0;
}

