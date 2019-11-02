/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once


enum			TopMenuEntriesIdx
{
	eTopMenuPickLinkSource		= 0,
	eTopMenuCancelLinkCreation,
	eTopMenuHardlink,
	eTopMenuJunction,	
	eTopMenuSymbolicLink,
	eTopMenuTo,
	eTopMenuOf,
	eTopMenuReference,
	eTopMenuHardlinkClone,  
	eTopMenuSymbolicLinkClone, 
	eTopMenuReferenceHelpText,
	eTopMenuDeleteMountPoint,
	eTopMenuMountPoint,
	eTopMenuSmartCopy,
	eTopMenu__Free__
};


enum			MenuEntriesIdx
{
	eMenuHardlink		= 0,
	eMenuSymbolicLink,
	eMenuJunction,
	eMenuHardLinkClone,
	eMenuSymbolicLinkClone,
	eMenuReplaceJunction,
	eMenuSmartCopy,
	eMenuReplaceSymbolicLink,
	eMenuReplaceMountPoint,
	eMenuSmartMirror,
	eMenuDeloreanCopy,
	eMenuCreateMountPoint,
  eMenuCopySymbolicLink,
  eMenuCopyJunction,
  eMenuCopyMountpoint,

  // The 'Drop' entries are needed if there is just one entry
  // so that the entry is e.g 'Drop Hardlink here'
  eMenuDropeAs,
	eMenuDropHardlink,
	eMenuDropHardLinkClone,
	eMenuDropJunction,
	eMenuDropReplaceJunction,
	eMenuDropSmartCopy,  // unused, but there if we ever need the strings
	eMenuDropSymbolicLink,
	eMenuDropReplaceSymbolicLink,   // unused, but there if we ever need the strings
	eMenuDropReplaceMountPoint,   // unused, but there if we ever need the strings
	eMenuDropSmartMirror,  // unused, but there if we ever need the strings
	eMenuDropDeloreanCopy,  // unused, but there if we ever need the strings
	eMenue__Free__
};


enum			CommandType
{
	ePickLinkSource = 0,
	eDropHardLink,
	eDropSymbolicLink,
	eDropJunction,
	eDropHardLinkClone,
	eDropSymbolicLinkClone,
  eDropAs,
	eCancelPickLink,
	eDropCreateMountPoint,
	eDropDeleteMountPoint,
	eDropReplaceJunction,
	eDropSmartCopy,
	eDropReplaceSymbolicLink,
	eDropReplaceMountPoint,
	eDropSmartMirror,
	eDropDeloreanCopy,
  eDropCopySymbolicLink,
  eDropCopyJunction,
  eDropCopyMountpoint,
  eCommandType__Free__
};

const PCHAR  VerbsA[eCommandType__Free__] = {
							"PickLinkSource",
							"DropHardlink",
							"DropSymbolicLink",
							"DropJunction",
							"DropHardLinkClone",
							"DropSymbolicLinkClone",
							"DropAs",
							"CancelPickLink",
							"DropCreateMountPoint",
							"DropDeleteMountPoint",
							"DropReplaceJunction",
							"DropSmartCopy",
							"DropReplaceSymbolicLink",
              "DropReplaceMountPoint",
              "DropSmartMirror",
              "DropDeloreanCopy",
              "DropCopySymbolicLink"
              "DropCopyJunction"
              "DropCopyMountpoint"
};

const PWCHAR  VerbsW[eCommandType__Free__] = {
							L"PickLinkSource",
							L"DropHardlink",
							L"DropSymbolicLink",
							L"DropJunction",
							L"DropHardLinkClone",
							L"DropSymbolicLinkClone",
							L"DropAs",
							L"CancelPickLink",
							L"DropCreateMountPoint",
							L"DropDeleteMountPoint",
							L"DropReplaceJunction",
							L"DropSmartCopy",
							L"DropReplaceSymbolicLink",
              L"DropReplaceMountPoint",
              L"DropSmartMirror",
              L"DropDeloreanCopy",
              L"DropCopySymbolicLink"
              L"DropCopyJunction"
              L"DropCopyMountpoint"
};

