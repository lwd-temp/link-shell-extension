/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once


//
// Maximum reparse buffer info size. The max user defined reparse
// data is 16KB, plus there's a header.
//
#define MAX_REPARSE_SIZE	17000


//
// Undocumented FSCTL_SET_REPARSE_POINT structure definition
//
#define IO_REPARSE_TAG_SYMBOLIC_LINK		0xa000000c
#define REPARSE_MOUNTPOINT_HEADER_SIZE   8
typedef struct {
/* 00 - 03 */    DWORD	ReparseTag;
/* 04 - 07 */    DWORD	ReparseDataLength;
/* 08 - 09 */    WORD	Reserved;
/* 0A - 0B */    WORD	ReparseTargetLength;
/* 0C - 0D */    WORD	ReparseTargetMaximumLength;
/* 0E - 0F */    WORD	Reserved1;
/* 10 - 11 */    WCHAR	ReparseTarget[1];
} REPARSE_MOUNTPOINT_DATA_BUFFER, *PREPARSE_MOUNTPOINT_DATA_BUFFER;


typedef struct _REPARSE_DATA_BUFFER {
/* 00 - 03 */    ULONG  ReparseTag;
/* 04 - 05 */    USHORT ReparseDataLength;
/* 06 - 07 */    USHORT Reserved;
    union {
        struct {
/* 08 - 09 */    USHORT SubstituteNameOffset;
/* 0A - 0B */    USHORT SubstituteNameLength;
/* 0C - 0D */    USHORT PrintNameOffset;
/* 0E - 0F */    USHORT PrintNameLength;
/* 10 - 13 */    ULONG  Reserved;
/* 14 - 15 */    WCHAR PathBuffer[1];
        } SymbolicLinkReparseBuffer;

		struct {
/* 08 - 09 */    USHORT SubstituteNameOffset;
/* 0A - 0B */    USHORT SubstituteNameLength;
/* 0C - 0D */    USHORT PrintNameOffset;
/* 0E - 0F */    USHORT PrintNameLength;
/* 10 - 11 */    WCHAR PathBuffer[1];
        } MountPointReparseBuffer;

		struct {
/* 08 - 09 */    USHORT SubstituteNameOffset;
/* 0A - 0B */    USHORT SubstituteNameLength;
/* 0C - 0D */    USHORT PrintNameOffset;
/* 0E - 0F */    USHORT PrintNameLength;
/* 10 - 11 */    WCHAR PathBuffer[1];
        } JunctionReparseBuffer;

		struct {
/* 08 - 09 */    UCHAR  DataBuffer[1];
        } GenericReparseBuffer;
    };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

