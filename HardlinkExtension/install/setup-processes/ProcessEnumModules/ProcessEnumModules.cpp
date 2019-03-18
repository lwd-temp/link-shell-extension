// ProcessEnumModules.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\NtBase\NtBase.h"


int _tmain(int argc, _TCHAR* argv[])
{
  _StringList Modules;
  _StringMap  Processes;

  Modules.push_back(L"HardlinkShellExt.dll");
  NtQueryProcessByModule(Modules, Processes);

  wchar_t TempPath[MAX_PATH];
  GetTempPath(MAX_PATH, TempPath);
  wcscat_s(TempPath, MAX_PATH, L"LseInstallerCollidingProcesses.txt");

  FILE* processes = _wfopen (TempPath, L"wb");
  if (processes)
  {
    for (_StringMap::iterator iter = Processes.begin(); iter != Processes.end(); ++iter)
      fwprintf (processes, L"%s\n", iter->first.c_str());

    fclose(processes);
  }

  return 0;
}

