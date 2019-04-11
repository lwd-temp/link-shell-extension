/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

void
_SmartMirror(
  FileInfoContainer&    CloneList,
  FileInfoContainer&    MirrorList,
  CopyStatistics&       MirrorStats,
  _PathNameStatusList&  MirrorPathNameStatusList,
  AsyncContext*         a_pMirrorContext,
  Effort&               Progress,
  Progressbar*          pProgressbar
);

void
DisposeAsync(
  FileInfoContainer&    List1,
  FileInfoContainer&    List2,
  CopyStatistics&       Stats1,
  CopyStatistics&       Stats2
);

#define UACHELPERARGS L"uachelper.args"

FILE*
OpenFileForExeHelper(
  wchar_t* curdir,
  wchar_t* sla_quoted
);

int
ForkExeHelper(
  wchar_t*	curdir,
  wchar_t*	sla_quoted
);

void WriteUACHelperArgs(
  FILE*           aArgsFile,
  const wchar_t   aFunction,
  const wchar_t*  aArgument1,
  const wchar_t*  aArgument2
);
