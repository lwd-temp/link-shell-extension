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

