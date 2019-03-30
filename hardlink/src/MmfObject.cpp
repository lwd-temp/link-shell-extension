// MmfObject.cpp: implementation of the MmfObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "hardlink_types.h"

#include "MmfObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MmfObject::
MmfObject() :
  m_File (INVALID_HANDLE_VALUE),
  m_Filemap (INVALID_HANDLE_VALUE),
  m_pMemoryView (NULL)
{
  m_FileSize.ul64 = 0;
}

int 
MmfObject::
Open(
  PWCHAR	  	            aFilename,
  _PathNameStatusList*    a_PathNameStatusList,
  ULONG64		              dwOffset = 0,
  ULONG64		              dwCount = 0
)
{
	// Open the file for reading.
	m_File = CreateFileW(
					aFilename, 
					GENERIC_READ, 
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL
	);
	if (m_File == INVALID_HANDLE_VALUE) 
	{
    DWORD HardlinkError = GetLastError();
    
    // We failed on creating a hardlink
    PathNameStatus pns(BackSlashF, &aFilename[PATH_PARSE_SWITCHOFF_SIZE], HardlinkError);
    a_PathNameStatusList->push_back(pns);
		return errCanNotOpenFile;
	}

	m_FileSize.ul32l = GetFileSize(m_File, &m_FileSize.ul32h);
	m_Filemap = CreateFileMapping(
					m_File,
					NULL,
					PAGE_READONLY,
					m_FileSize.ul32h,
					m_FileSize.ul32l,
          NULL
	);
	if (m_Filemap == NULL) 
	{
    DWORD HardlinkError = GetLastError();
    
    // We failed opening the file
    PathNameStatus pns(BackSlashF, &aFilename[PATH_PARSE_SWITCHOFF_SIZE], HardlinkError);
    a_PathNameStatusList->push_back(pns);

    Close();
		return errCanNotMapFile;
	}

	if ((dwOffset + dwCount >  m_FileSize.ul64) && dwOffset)
		dwCount = m_FileSize.ul64 - dwOffset;

  UL64 Offset;
  Offset.ul64 = dwOffset;

  m_pMemoryView = MapViewOfFileEx(m_Filemap, 
		FILE_MAP_READ, 
    dwOffset ? Offset.ul32h : 0, 
    dwOffset ? Offset.ul32l : 0, 
		dwCount ? dwCount : 0,
    NULL
	);
	if (NULL == m_pMemoryView) 
	{
    DWORD HardlinkError = GetLastError();
    
    // We failed mapping the file
    PathNameStatus pns(MemoryMapF, &aFilename[PATH_PARSE_SWITCHOFF_SIZE], HardlinkError);
    a_PathNameStatusList->push_back(pns);

    Close();
		return errCanNotMapViewOfFile;
	}
	return errOk;
}

int 
MmfObject::
Close(
)
{
	BOOL r;
	if (NULL != m_pMemoryView)
	{
		r = UnmapViewOfFile(m_pMemoryView);
    if (!r)
      printf("### MMF: UnmapViewofFile failed\n");
		m_pMemoryView = NULL;
	}

	if (NULL != m_Filemap)
	{
		r = CloseHandle(m_Filemap);
    if (!r)
      printf("### MMF: DeleteFileMap failed\n");
		m_Filemap = NULL;
	}

  if (INVALID_HANDLE_VALUE != m_File)
	{
		r = CloseHandle(m_File);
    if (!r)
      printf("### MMF: CloseFile failed\n");
		m_File = INVALID_HANDLE_VALUE;
	}

	return errOk;
}

MmfObject::~MmfObject()
{
	Close();
}
