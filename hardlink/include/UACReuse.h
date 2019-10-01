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

class UACHelper
{
public: 
  UACHelper() : m_UacArgs{ nullptr }  {};
  ~UACHelper() { Close(); };

  int Fork();
  void WriteArgs(
    const wchar_t   aFunction,
    const wchar_t*  aArgument1,
    const wchar_t*  aArgument2
  );
  FILE* File() { return m_UacArgs; };
  void SaveProgressbarPosition(RECT& a_ProgressbarPosition);


protected:
  void Open();
  int Close();

  FILE*   m_UacArgs;
  wstring  m_CurrentDir;
  wstring  m_SlaQuoted;
};
