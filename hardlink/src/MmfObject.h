// MmfObject.h: interface for the MmfObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MMFOBJECT_H__999C97DC_79EE_4E1E_B1E9_24884586A846__INCLUDED_)
#define AFX_MMFOBJECT_H__999C97DC_79EE_4E1E_B1E9_24884586A846__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#endif // !defined(AFX_MMFOBJECT_H__999C97DC_79EE_4E1E_B1E9_24884586A846__INCLUDED_)
