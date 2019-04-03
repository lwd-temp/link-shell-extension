/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

enum DmErrorCodes
{
	errOk												= 0,
	errCanNotOpenFile						= -1,
	errCanNotMapFile						= -2,
	errCanNotMapViewOfFile			= -3,
	errFileNotFound							= -4,
	errPathNotFound							= -5,
	errEmptySet									= -6,
	errFileHasChanged						= -7,
	errInvalidRegExp						= -8,
	errBrokenNTFS						    = -9
};

class MmfObject
{
	public:
		MmfObject();

		int 
		Open(
			 PWCHAR	  	            aFilename,
       _PathNameStatusList*   a_PathNameStatusList,
			 ULONG64		            dwOffset,
			 ULONG64		            dwCount
		);

		int 
		Close(
		);

		LPVOID 
		GetMemoryView(
		) 
		{ return m_pMemoryView;};

		ULONG64
		GetSize(
		) 
		{ return m_FileSize.ul64;}

	protected:
		HANDLE		m_File;
		HANDLE		m_Filemap;
		LPVOID		m_pMemoryView;
		UL64		  m_FileSize;

	public:
		virtual ~MmfObject();

};
