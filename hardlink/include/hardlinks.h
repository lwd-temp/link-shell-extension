/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once
#if !defined EXTERN
#define EXTERN extern
#endif

typedef vector<int>	_Of;

int
FindFreeNumber(
  _Of&		of
);

void
Test_FindFreeNumber(
);

// 
// Provide a function pointers to various calls from kernel32.dll
//
extern "C"
{
  typedef BOOL (__stdcall *CreateSymboliclinkW_t)(
		LPCTSTR lpSymlinkFileName,
		LPCTSTR lpTargetFileName,
		DWORD dwFlags
	);

	typedef BOOL (__stdcall *CreateHardlinkW_t)(
		LPCTSTR toFile,
		LPCTSTR fromFile, 
		LPSECURITY_ATTRIBUTES sa
	);

  typedef BOOL (__stdcall *IsUserAnAdmin_t)(
	);
}

EXTERN OSVERSIONINFO            gVersionInfo;
EXTERN LSESettings      gLSESettings;

bool
CheckFileSystemType(
	PTCHAR			aPath,
  int*        aDriveType
);

void
InitCreateHardlink();

int
CreateHardlink(
  __in    LPCWSTR	fromFile,
  __in    LPCWSTR	toFile
);

bool
IsVeryLongPath(
  __in    LPCWSTR lpPathName
);

int
ResolveSymboliclink(
  __in    LPCWSTR   lpSymlinkFileName,
  __in    LPCWSTR   lpTargetFileName,
  __inout LPWSTR    lpSymlinkTarget
);

int
FixVeryLongPath(
   wchar_t** aPathName
);

int
CreateSymboliclink(
  __in  LPWSTR    lpSymlinkFileName,
  __in  LPCWSTR   lpTargetFileName,
  __in  DWORD     dwFlags,
  __in  LPCWSTR   lpFakeSymlinkFileName = NULL
);

typedef DWORD (__stdcall *ThrFuncType)(void*);


BOOL RemoveDir (
  __in  LPCWSTR aPath,
  const BOOL aQuiet
);

struct	XDelStatistics
{
	ULONG64		Size;
	ULONG64		Count;
	ULONG64		Directories;
	ULONG64		Junctions;
	ULONG64		SymbolicLinks;

};

int XDel (
  wchar_t*          aSrcPath, 
  const size_t      aSrcSize,
  XDelStatistics&   rXDelStatistics,
  int               aFlags
);

int XDel_recursive (
  wchar_t*          aSrcPath, 
  const size_t      aSrcSize,
  XDelStatistics&   rXDelStatistics,
  int               aFlags
);


//--------------------------------------------------------------------
//
// ProbeHardlink
//
// Probes if a file is a hardlink, and if yes returns the
// RefCount of the hardlink
//--------------------------------------------------------------------
int 
ProbeHardlink(
	__in LPCTSTR		aFileName
);

//--------------------------------------------------------------------
//
// CreateJunction
//
// This routine creates a NTFS junction, using the undocumented
// FSCTL_SET_REPARSE_POINT structure Win2K uses for mount points
// and junctions.
//
//--------------------------------------------------------------------
int
CreateJunction( 
  __in  LPCWSTR LinkDirectory,
  __in  LPCWSTR LinkTarget,
  __in  DWORD   dwFlags = 0
);

extern "C"
{
//--------------------------------------------------------------------
//
// CreateSymboliclinkRaw
//
// This routine creates a NTFS SymbolicLink, using the undocumented
// FSCTL_SET_REPARSE_POINT structure Win2K uses for mount points
// and junctions. It allows to create dead symbolic links
//
//--------------------------------------------------------------------
int
__stdcall
CreateSymboliclinkRaw( 
  __in    LPCWSTR       lpTargetFileName,
  __in    LPCWSTR       lpSymlinkFileName,
  __in    const DWORD   dwFlags
);
}

//--------------------------------------------------------------------
//
// ProbeReparsePoint
//
// Determines if the file is a reparse point, and if so prints out
// some information about it.
// Return Value
//   REPARSE_POINT_FAIL
//   REPARSE_POINT_JUNCTION
//   REPARSE_POINT_MOUNTPOINT
//   REPARSE_POINT_SYMBOLICLINK
//
//--------------------------------------------------------------------
int 
ProbeReparsePoint( 
  __in      LPCWSTR   FileName,
  __inout   LPWSTR    aDestination
);

//--------------------------------------------------------------------
//
// ProbeJunction
//
// Determines if the file is a reparse point, and if so prints out
// some information about it.
// This coding is taken from www.sysinternals.com
//
//--------------------------------------------------------------------
BOOL 
ProbeJunction( 
	__in    LPCWSTR  aFileName,
  __inout PWCHAR   aDestination
);

//--------------------------------------------------------------------
//
// CreateTimeStampedFileName
//
// CreateFileName generates a Filename with the pattern of 
// Copy (x) of <filename>
//
//--------------------------------------------------------------------
class SringSorter
{
	public:
		bool
		operator()(const wstring& first, const wstring& second) const
		{
			return first <= second ? true : false;
		}
};

void
CreateTimeStampedFileName( 
  LPCWSTR       aSourcePath,
  PWCHAR        aBackup0Path,
  PWCHAR        aBackup1Path,
  LPCWSTR       aFilename,
  _SYSTEMTIME*  const a_pSysTime
);
//--------------------------------------------------------------------
//
// CreateFileName
//
// CreateFileName generates a Filename with the pattern of 
// Copy (x) of <filename>
//
//--------------------------------------------------------------------
class SizeSorter
{
	public:
		bool
		operator()(int first, int second) const
		{
			return first <= second ? true : false;
		}
};

int
CreateFileName( 
  __in    const HINSTANCE  aLSEInstance,
  __in    const int        aLanguageID,
  __in    LPCWSTR          aPrefix,
  __in    LPCWSTR          aOf,
  __in    LPCWSTR          aPath,
  __in    LPCWSTR          aFilename,
  __inout LPWSTR           aNewFilename,
  __in    const int        aOrderIdx
);

//--------------------------------------------------------------------
//
// IsFileSystemNtfs
//
// Checks if a given path is under NTFS
//
//--------------------------------------------------------------------
bool
IsFileSystemNtfs (
	const wchar_t*	aPath,
	DWORD*		      dwFileSystemFlags,
  const DWORD			aFlags,
  int*            aDriveType
);

int ReparseCanonicalize (
  __in    LPCWSTR		      aPath,
  __inout LPWSTR		      aDestination,
  __in    const size_t    aDestSize
);

typedef vector<int>	_Of;

#if defined _DEBUG
int
FindFreeNumber(
	_Of&		of
);

void
Test_FindFreeNumber(
);
#endif


//--------------------------------------------------------------------
//
// CreateMountPoint
//
// This routine creates a NTFS mount point
//
//--------------------------------------------------------------------
int
CreateMountPoint (
  __in    LPCWSTR aSourceDirectory,
  __in    LPCWSTR aDestinationDirectory
);

//--------------------------------------------------------------------
//
// TranslateMountPoint
//
// Translates an object path to a drive letter
//
//--------------------------------------------------------------------
BOOL
TranslateMountPoint (
  __inout LPWSTR        aDestination,
  __in    const size_t  aDestSize,
  __in    LPCWSTR       aVolumeName
);

//--------------------------------------------------------------------
//
// ProbeMountPoint
//
// This routine checks if a given location is a NTFS mount point, 
// using the undocumented FSCTL_SET_REPARSE_POINT structure.
//
//--------------------------------------------------------------------
BOOL
ProbeMountPoint (
  __in    LPCWSTR       aDirectory,
  __inout LPWSTR        aDestination,
  __in    const size_t  aDestSize,
  __in    LPWSTR        aVolumeName
);

//--------------------------------------------------------------------
//
// DeleteMountPoint
//
// This routine deletes a NTFS mount point.
//
//--------------------------------------------------------------------
int
DeleteMountPoint (
  __in    LPCWSTR  aDirectory
);

//--------------------------------------------------------------------
//
// ProbeSymbolicLink
//
// Determines if the file is a SymbolicLink.
//
//--------------------------------------------------------------------
BOOL 
ProbeSymbolicLink( 
  __in    LPCWSTR aFileName,
  __inout LPWSTR  aDestination
);



//--------------------------------------------------------------------
//
// CheckIfOnSameDrive
//
// Check if two path are on the same (network) (UNC) drive 
//
//--------------------------------------------------------------------
bool CheckIfOnSameDrive(
	const wchar_t* aPath1, 
	const wchar_t* aPath2
);



// This structure is a bit a hack, but it serves its purpose,
// well it hides MS unability to have the 64bit future in mind
//
// The point is to replace all split 2 * 32 bit members with 
// respective 64bit values, so that comparison can be done with
// one operation. If the aligning is 8, as it is by default, there
// would be a gap of 4 bytes between dwFileAttributes and ftCreationTime.
// To overcome this we set the alignment temporarily to 4 so that ftCreationTime
// without gap follows dwFileAttributes, and the size of this structure
// is the same as the original _WIN32_FILE_ATTRIBUTE_DATA
//
// The second problem with this structure is the order of the filesize
// DWORDS, which makes it hard to overlay two 32bit with one 64 bit
// value, because nFileSizeLow should be first and then nFileSizeHigh
// so that an overlayed 64bit can be interpreted properly.
//
// A much more robust defintion would be
//
// typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
//     FILETIME64 ftCreationTime;
//     FILETIME64 ftLastAccessTime;
//     FILETIME64 ftLastWriteTime;
//     union {
//      struct {
//         DWORD nFileSizeLow;
//         DWORD nFileSizeHigh;
//       };
//       ULONG64 nFileSize;
//     };
//     DWORD dwFileAttributes;
// } 
#pragma pack(push, 4)
typedef struct _WIN32_FILE_ATTRIBUTE_DATA64 {
    DWORD dwFileAttributes;
    FILETIME64 ftCreationTime;
    FILETIME64 ftLastAccessTime;
    FILETIME64 ftLastWriteTime;
    union {
      struct {
        DWORD nFileSizeHigh;
        DWORD nFileSizeLow;
      };
      ULONG64 nFileSize;
    };
} WIN32_FILE_ATTRIBUTE_DATA64, *LPWIN32_FILE_ATTRIBUTE_DATA64;
#pragma pack ( pop )


class DiskSerialCache;
class FileInfoContainer;

//
// FileInfo
//
class FileInfo
{
	// size 0x058
	public:
		FileInfo();
		// Since ln.exe, when used in copy mode and when it scans really large
		// file-trees, the whole tree is held in memory which blasts the memory
		// footprint. 
		// To lower the memory footprint the m_FileIndex is held for filenames
		// and the m_FileTime is held for directories mutually exclusive.
		union
		{
			// This is for directories, junctions, symbolic links, mountpoints
			struct {
        FILETIME64	      m_CreationTime;
        FILETIME64	      m_LastAccessTime;
        wchar_t*          m_ReparseSrcTargetHint;
			};
			// This is for files and DupeMerge
      struct {
			  UL64		m_FileIndex;
			  UL64		m_FileSize;
			  DWORD		m_RefCount;

        union {
          struct {
            ULONG   m_DiskIndex;
            int     m_HardlinkInSet;
          };

		      enum
		      {
			      cHashSize = 16
		      };
          struct {
            union {
              unsigned char	m_Hash[cHashSize];
              struct {
                ULONG64 m_Hash64_1;
                ULONG64 m_Hash64_2;
              };
            };
		        MmfObject*		m_pMmfObject;
            int       m_Flags;
          };
        };
      };

    };

    enum _AttributeChanged
    {
      AttributeSame = 0,
      CompressionChange = 1,
      SparseChange = 2
    };

		wchar_t*	                m_FileName;
    FILETIME64	              m_LastWriteTime;
    
    // This is an index into m_AnchorPathsCache 
    int                       m_DestPathIdx;
    DWORD                     m_SourcePathLen;
		DWORD		                  m_Type;

    void
    Init() 
    {
      m_RefCount = 0;
      m_HardlinkInSet = 0;
      m_FileName = NULL;
      m_Type = 0;
      m_Flags = 0;

      m_FileIndex.ul64 = 0;
      m_DestPathIdx = 0;
      m_SourcePathLen = 0;
      m_ReparseSrcTargetHint = NULL;
      m_LastWriteTime.ul64DateTime = 0;

	    m_FileSize.ul64 = 0;
	    m_Hash64_1 = 0;
	    m_Hash64_2 = 0;
	    m_pMmfObject = nullptr; 
    };


		bool
		IsDirectory()
		{
      return m_Type & FILE_ATTRIBUTE_DIRECTORY ? true : false;
		}

		bool
		IsReparsePoint()
		{
      return m_Type & FILE_ATTRIBUTE_REPARSE_POINT ? true : false;
		}

		bool
		IsDelayedReparsePoint()
		{
      return m_Type & FILE_ATTRIBUTE_DELAYED_REPARSE_POINT ? true : false;
		}

		bool
		IsNestedReparsePoint()
		{
      return m_Type & FILE_ATTRIBUTE_NESTED_REPARSE_POINT ? false : true;
		}

		bool
		IsPlainFile()
		{
      return FAT_SERIAL_NUMBER == m_DiskIndex ? false : true;
		}

		bool
		IsHardlink()
		{
      return m_RefCount > 1 ? true : false ;
		}

    bool
    IsOlder(
      FileInfo*	a_pFileInfo
    );

    bool
    IsDifferent(
      FileInfo*	a_pFileInfo,
      __int64   a_DateTimeTolerance
    );

    void
    ReplaceSourcePath(
      const wchar_t*	aNewPath,
      size_t    aNewPathLen
    );

  protected:
		enum
		{
			err_InvalidHash			= 0x01,
			inf_Single				= 0x02,
			inf_HardlinkRep			= 0x04,
			inf_HashFinished		= 0x08
		};

	public:
		void
		InvalidHash()
		{
			m_Flags |= (int) err_InvalidHash;
		}

		bool
		IsHashValid()
		{
			return !(bool) (m_Flags & (int) err_InvalidHash);
		}

		void
		Single()
		{
			m_Flags |= (int) inf_Single;
		}

		bool
		IsSingle()
		{
			return m_Flags & (int) inf_Single ? true : false;
		}

		void
		HardlinkRep()
		{
			m_Flags |= (int) inf_HardlinkRep;
		}

		void
		NoHardlinkRep()
		{
			m_Flags &= (int) ~inf_HardlinkRep;
		}

		bool
		IsHardlinkRep()
		{
			return m_Flags & (int) inf_HardlinkRep ? false : true;
		}

		bool
		IsUseful()
		{
			return m_Flags & (int) (inf_Single | err_InvalidHash) ? false : true;
		}

		bool
		IsADupe()
		{
			return IsUseful ();
		}

		void
		HashFinished()
		{
			m_Flags |= (int) inf_HashFinished;
		}

		bool
		IsHashFinished()
		{
			return m_Flags & (int) inf_HashFinished ? true : false;
		}

    friend FileInfoContainer;

};

typedef vector<FileInfo*>	_Pathes;

//
// DupeGroup
// 
class DupeGroup
{
	public:
    enum _DupeGroupState
    {
      eVoid = 0,
      eOld = 1,
      eNew = 2,
      eInvalid = 4
    } ;

    DupeGroup() :
      m_State(eVoid), m_cardinality(0) {};

		_Pathes::iterator	m_start;
		_Pathes::iterator	m_end;

		
    size_t				m_cardinality;
  
    _DupeGroupState     m_State;



		class CardinalitySorter
		{
			public:
				bool
				operator()(DupeGroup first,
									 DupeGroup second)
				{
					return first.m_cardinality <= second.m_cardinality ? false : true;
				}
		};

		class FileSizeSorter
		{
			public:
				bool
				operator()(DupeGroup first,
									 DupeGroup second)
				{
					return (*first.m_start)->m_FileSize.ul64 <=
						(*second.m_start)->m_FileSize.ul64 ?
						false :
						true;
				}
		};

	public:
		virtual
		~DupeGroup()
		{
		};
};

typedef vector<DupeGroup>	_DupeGroups;


//
// CopyStatistics
//
class StatisticsEvent
{
	public:
    StatisticsEvent() : m_state(0), m_time()
		{
		};

		int		m_state;
		SYSTEMTIME	m_time;

	public:
		virtual
		~StatisticsEvent()
		{
		};
};

typedef list<StatisticsEvent, allocator<StatisticsEvent> >	_StatisticsEvent;

class uDelta
{
public:
  uDelta() { Reset(); };
  ~uDelta() { };

#if defined DEBUG_STOPWATCH
private:
  LARGE_INTEGER   m_Start;
  LARGE_INTEGER   m_Value;

public:
  void Reset() { m_Value.QuadPart = 0; };
  void Start() { QueryPerformanceCounter(&m_Start); };
  __int64 Stop() { LARGE_INTEGER stop;  QueryPerformanceCounter(&stop); __int64 delta = stop.QuadPart - m_Start.QuadPart; m_Value.QuadPart += delta; return delta; };

  // Return stopwatch in uS
  __int64 Get() { LARGE_INTEGER frequency; QueryPerformanceFrequency(&frequency); return (double)m_Value.QuadPart / (double(frequency.QuadPart) / 1000000.0); };
#else
  void Reset() { };
  void Start() { };
  __int64 Stop() { return 0; };
  __int64 Get() { return 0; };
#endif
};


class	CopyStatistics
{
public:
  CopyStatistics();

  SYSTEMTIME	m_StartTime;
	SYSTEMTIME	m_CopyTime;
	SYSTEMTIME	m_EndTime;

  void Start() { GetLocalTime(&m_StartTime); };
  void End() { GetLocalTime(&m_EndTime); };


  ULONG64		m_DirectoryTotal; // no printout by design
  ULONG64		m_DirectoryCreated; //
  ULONG64		m_DirectoryCreateSkipped; //
  ULONG64		m_DirectoryCreateFailed; //
  ULONG64		m_DirectoriesCleaned; //
  ULONG64		m_DirectoriesCleanedFailed; //
  ULONG64		m_DirectoriesExcluded; //

  ULONG64		m_FilesTotal; // The number of files, which were found during the query
  ULONG64		m_FilesCopied; //
  ULONG64		m_FilesCopySkipped; //
  ULONG64		m_FilesCopyFailed; //ee
  ULONG64		m_FilesLinked; //
  ULONG64		m_FilesLinkSkipped; //
  ULONG64		m_FilesLinkFailed; //ee
  ULONG64		m_FilesCleaned; //
  ULONG64		m_FilesCleanedFailed; //
  ULONG64		m_FilesExcluded; //
  ULONG64		m_FilesCropped; //
  ULONG64   m_FilesSelected; // The number of files, which has been added to the container
  
  ULONG64		m_BytesTotal;
  ULONG64		m_BytesCopied;
  ULONG64		m_BytesCopySkippped;
  ULONG64   m_BytesCopyFailed;
  ULONG64		m_BytesLinked;
  ULONG64		m_BytesLinkFailed;
  ULONG64		m_BytesLinkSkipped;
  ULONG64		m_BytesCleaned;
  ULONG64	  m_BytesExcluded;

  ULONG64		m_SymlinksTotal; // The number of symlinks, which were found during the query
  ULONG64		m_SymlinksRestored; //
  ULONG64		m_SymlinksRestoreSkipped; //
  ULONG64		m_SymlinksRestoreFailed; //ee
  ULONG64		m_SymlinksCleaned; //
  ULONG64		m_SymlinksCleanedFailed; //
  ULONG64		m_SymlinksExcluded; //
  ULONG64		m_SymlinksCropped; //
  ULONG64		m_SymlinksDangling; //
  ULONG64   m_SymlinksSelected; // The number of symlinks, which has been added to the container


  ULONG64		m_JunctionsTotal; // no printout by design
  ULONG64		m_JunctionsRestored; //
  ULONG64		m_JunctionsRestoreSkipped; //
  ULONG64		m_JunctionsRestoreFailed; //ee
  ULONG64		m_JunctionsCleaned; //
  ULONG64		m_JunctionsCleanedFailed; //
  ULONG64		m_JunctionsExcluded; //
  ULONG64		m_JunctionsCropped; //
  ULONG64		m_JunctionsDangling; //

  ULONG64		m_MountpointsTotal; // no printout by design
  ULONG64		m_MountpointsRestored; //
  ULONG64		m_MountpointsRestoreSkipped; //
  ULONG64		m_MountpointsRestoreFailed; //ee
  ULONG64		m_MountpointsCleaned; //
  ULONG64		m_MountpointsCleanedFailed; //
  ULONG64		m_MountpointsExcluded; //
  ULONG64		m_MountpointsCropped; //
  ULONG64		m_MountpointsDangling; //

  ULONG64		m_ReparsePointTotal; // no printout by design
  ULONG64		m_ReparsePointRestored; //
  ULONG64		m_ReparsePointRestoreSkipped; //
  ULONG64		m_ReparsePointRestoreFailed; //ee
  ULONG64		m_ReparsePointCleaned; //
  ULONG64		m_ReparsePointCleanedFailed; //
  ULONG64		m_ReparsePointExcluded; //
  ULONG64		m_ReparsePointCropped; //
  ULONG64		m_ReparsePointDangling; //
  ULONG64		m_ReparsePointSelected; // The number of symlinks, which has been added to the container

  ULONG64		m_HardlinksTotal; // no printout by design
  ULONG64		m_HardlinksTotalBytes; // no printout by design

  uDelta    m_HeapAllocTime;
  uDelta    m_HeapDeletionTime;
  uDelta    m_RegExpMatchTime;


  // DupeMerge
  ULONG64		m_DupeGroupsTotal;
  ULONG64		m_DupeGroupsNew;
  ULONG64		m_DupeGroupsOld;
  ULONG64		m_DupeFilesHardlinked;   // The number of files which have been hardlinked
  ULONG64		m_DupeFilesHardlinkFailed;   // The number of failed hardlinked files
  ULONG64		m_DupeTotalBytesToHash;
  ULONG64		m_DupeCurrentBytesHashed;
  ULONG64		m_DupeBytesSaved;

	_StatisticsEvent	m_StatisticsEvents;

  enum
  {
    eVoid = 0x00,
    eScanning = 0x01,
    eScanned = 0x02,
    ePreparation = 0x03,
    ePreparated = 0x04,
    eDupeMerge = 0x05,
    eDupeMerged = 0x06,
    eFinished = 0x07
  };

  protected:
    mutex             m_EventGuard;
    mutex	            m_StatGuard;

  private:
    int					m_State;

	public:
		int
		Update(CopyStatistics& rCopyStatistics);

		int
		AddTlsStat(CopyStatistics* rCopyStatistics);

		int
		Proceed(int aNewState);

		int
		GetDupeEvent(StatisticsEvent& aStatisticsEvent);

    CopyStatistics& operator+=(const CopyStatistics& stats);

#if defined _DEBUG
		int
		Dump();
#endif

	public:
		virtual ~CopyStatistics();

};

class	EnumHardlinkSiblingsGlue
{
	public:
    EnumHardlinkSiblingsGlue() : m_RefCount{ 0 } {};
		virtual void Print(wchar_t* pSiblingFileName) = 0;

	protected:	
		int			m_RefCount;

	public:
    virtual ~EnumHardlinkSiblingsGlue() {};
};

struct map_comp
{
  bool operator()(const wchar_t* first, const wchar_t* second) const
  {
    return wcscmp(first, second) < 0; 
  }
};

struct map_comp_DWORD
{
  bool operator()(const DWORD first, const DWORD second) const
  {
    return first < second; 
  }
};

//
// AnchorPathCache
//
class AnchorPathCache
{
  public:
    AnchorPathCache() {};

    size_t 
    size(
    )
    {
      return m_AnchorPaths.size();
    };

    void 
    Add(
      _ArgvList&  a_AnchorPath
    );

    void 
    Get(
      _ArgvList&  a_AnchorPath
    );

    size_t
    Add(
      _ArgvPath& a_AnchorPath
    )
    {
      m_AnchorPaths.push_back(a_AnchorPath);
      return m_AnchorPaths.size() - 1;
    }

    // Searches the whole AnchorPathCache and returns the Index under which a Destination
    // of an anchor was found
    int
    GetDestPathIndex (
#if defined EXACT_PATH_INDEX
      _ArgvListIterator& a_AnchorPath
#else
      const wchar_t*  a_Path
#endif
    );

#if defined EXACT_PATH_INDEX
    void 
    MarkCreated(
      wchar_t*    a_DestPath
    );

    int 
    GetFlags(
      int    a_DestPathIdx
    ) 
    { return m_AnchorPaths[a_DestPathIdx].Flags; };


#endif

    wchar_t* 
    ContainsSource(
      const wchar_t*  a_Path,
      wchar_t*        a_DestPath = NULL,
      int*            a_AlternativeDestPathIdx = NULL
    );

    int
    ContainsExactSource(
      const wchar_t*  a_Path
    );

    size_t
    GetDestPath(
      wchar_t*      a_DestPath,
//      const size_t  a_DestSize,
      int           a_DestPathIdx
    );

    void
    ResolveRemote(
    );

    size_t
    Save(
      FILE*         aFile
    );

    size_t
    Load(
      FILE*         aFile
    );

    size_t
    Logon(
      wchar_t*   a_Username,
      wchar_t*   a_Passwort
    );

    _ArgvList          m_AnchorPaths;

  public:
    virtual ~AnchorPathCache() { m_AnchorPaths.clear(); };
};

typedef pair<const wchar_t*, DWORD> _DiskSerialMap_Pair;
typedef map<const wchar_t*, DWORD, map_comp> _DiskSerialMap;

//
// DiskSerialCache
//
class DiskSerialCache
{
  public:
    DiskSerialCache() {};

    typedef pair<DWORD, const wchar_t*> _DiskSerialInverseMap_Pair;
    typedef map<DWORD, const wchar_t*, map_comp_DWORD> _DiskSerialInverseMap;

    int 
    Lookup(
      wchar_t*  a_pRootPathName, 
      DWORD&    a_pVolumeSerialNumber
    );

    int 
    Load(
      FILE*     a_File
    );

    int 
    Save(
      FILE*     a_File
    );

  protected:
    _DiskSerialMap          m_DiskSerialMap;
    _DiskSerialInverseMap   m_DiskSerialInverseMap;

  public:
    virtual ~DiskSerialCache();
};

// FileInfoContainer
//
#if defined REGEXP_STL
typedef vector<wregex*>	_RegExpList;
#else
typedef vector<regex_t*>	_RegExpList;
#endif
class FileInfoContainer
{
	public:
    FileInfoContainer ();


    typedef pair<const wchar_t*, FileInfo*> _PathMap_Pair;
#if defined USE_VECTOR
    typedef vector<_PathMap_Pair>	_PathVector;

    struct vector_comp
    {
      bool operator()(const _PathMap_Pair& first, const _PathMap_Pair& second)
      {
        bool b = wcscmp(first.first, second.first) < 0; 
        return b;
      }
    };

#else
    typedef map<const wchar_t*, FileInfo*, map_comp > _PathMap;
#endif

    int AddDirectoryFast(
      wchar_t*	            aName,
      _Pathes&              a_Filenames,
      CopyStatistics*	      pStats,
      PWCHAR	              aReparseSrcTargetHint,
      DWORD                 aOriginalFileAttribute,
      _PathNameStatusList*  a_pPathNameStatus,
      PFILE_ID_BOTH_DIR_INFORMATION   pFullDirInfo
		);

    int AddDirectory(
      wchar_t*	            aName,
      _Pathes&              a_Filenames,
      DWORD		              FileAttribute,
      CopyStatistics*	      pStats,
      PWCHAR	              aReparseSrcTargetHint,
      DWORD                 aOriginalFileAttribute,
      _PathNameStatusList*  a_pPathNameStatus
		);


    int
    AddFileFast(
      wchar_t*	                      aName,
      DWORD	                          nType,
      CopyStatistics*	                aStats,
      PWCHAR	                        aReparseSrcTargetHint,
      bool                            aUseReparseFileIndex,
      PFILE_ID_BOTH_DIR_INFORMATION   pFullDirInfo,
      int                             a_ReparsePointType
    );

		int AddFile(
      wchar_t*	        aName,
      DWORD	            nType,
      int               aRefCount,
      CopyStatistics*	  pStats,
      PWCHAR	          aReparseSrcTargetHint,
      WIN32_FIND_DATA*  pwfind,
      int               a_ReparsePointType,
      bool              aUseReparseFileIndex = false
		);

    bool BackupMode() { return m_Flags & eBackupMode ? true : false;};
    void SetJsonWriterState(bool* a_pSetJsonWriterState) { m_pJsonWriterState = a_pSetJsonWriterState; };

#if defined _DEBUG
    int
    AddFile(
      wchar_t*	        aName,
      DWORD	            nType,
      ULONG             aFileIndexLow,
      ULONG             aFileIndexHigh,
      ULONG             aDiskIndex,
      int               aDestPathIdx
    );
#endif

    enum CopyReparsePointFlags
    {
      eSmartMove = 0x01,
      eRelativeSymboliclinks = SYMLINK_FLAG_RELATIVE,
      eSmartClone = 0x04,
      eSymboliclinkClone = 0x08, // clash C! used the same value! intentional
      eDebugDupeMerge = 0x08,    // clash C! used the same value! intentional
      eQuiet = 0x10,
      eSmartMirror = 0x20,
      eSmartClean = 0x40,
      eSmartCopy = 0x80,
      eAlwaysUnrollDir = 0x100,
      eAlwaysSpliceDir = 0x200,
      eQuiet2 = 0x0400,
      eSkipFiles = 0x0800,    // clash A! used the same value! intentional
      eListOnly	= 0x0800,     // clash A! used the same value! intentional
      eTraditional = 0x1000,
      eJson = 0x2000,
      eSortBySize	= 0x4000,           // clash B! used the same value! intentional
      eKeepSymlinkRelation = 0x4000,  // clash B! used the same value! intentional
      eBackupMode = 0x8000
    };

    enum CopyReparsePointFlags2
    {
      eCloneMirror = 0x01,
      eDupemerge = 0x02,
      eNoAds = 0x04,
      eNoEa = 0x08,
      eNoSparseFile = 0x10,
      eLogFileOutput = 0x20,
      eDeloreanMerge = 0x40,

      // With Windows10 > 14972 SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE came up as flag
      // In order to switch the whole container into creating Symlinks unprivileged we need this
      // flag. See also SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE in this source.
      eUnprivSymlinkCreate = 0x80,
      eDelOnlyReparsePoints = 0x100
    };

    enum LogLevels
    {
      eLogVerbose = 0x00,    // Show all: Normal, changed and error messages == 3
      eLogChanged = eQuiet,  // Only show changed and error messages == 2
      eLogError = eQuiet2,   // Only show Error messages == 1
      eLogQuiet = eQuiet | eQuiet2, // show nothing == 0

      eLogMask = eQuiet | eQuiet2,
    };
    
	protected:
    // EnumHardlinkSiblings Related Methods
    struct EnumHardlinkSiblingsParams
    {
      EnumHardlinkSiblingsParams() : 
        m_SrcPath {nullptr}, m_ExcludePath{nullptr},
        m_pEnumHardlinkSiblingsGlue{ nullptr }, m_PathNameStatusList{ nullptr }, 
        m_pContext{ nullptr }, m_Quiet{ false }, m_This{ nullptr } {};

			PWCHAR						          m_SrcPath;
			PWCHAR						          m_ExcludePath;
			EnumHardlinkSiblingsGlue*	  m_pEnumHardlinkSiblingsGlue;
      _PathNameStatusList*        m_PathNameStatusList;
      AsyncContext*               m_pContext;
			bool						            m_Quiet;
      FileInfoContainer*          m_This;
    };

		int
    _EnumHardlinkSiblings(
      PWCHAR						          aSrcPath,
      PWCHAR						          aExcludePath,
      EnumHardlinkSiblingsGlue*	  pEnumHardlinkSiblingsGlue,
      bool						            aQuiet,
      _PathNameStatusList*        aPathNameStatusList,
      AsyncContext*               apContext
    );

    int
    _EnumHardlinkSiblingsUp(
      PWCHAR						        aSrcPath,
      const size_t              aSrcSize,
      PWCHAR						        aExcludePath,
      EnumHardlinkSiblingsGlue*	pEnumHardlinkSiblingsGlue,
      bool						          aQuiet,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext
    );

		int
    _EnumHardlinkSiblingsDown(
      PWCHAR						        aSrcPath,
      const size_t              aSrcSize,
      PWCHAR						        aExcludePath,
      EnumHardlinkSiblingsGlue*	pEnumHardlinkSiblingsGlue,
      bool						          aQuiet,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext
    );

    static
    DWORD
    __stdcall
    RunEnumHardlinkSiblings(EnumHardlinkSiblingsParams* p);

		int
		Dump(
			_Pathes::iterator&	last,
			_Pathes::iterator&	iter
		);

    // SmartCopy Related Methods
    struct SmartCopyParams
    {
      SmartCopyParams() : m_Stats{ nullptr }, m_PathNameStatusList{ nullptr }, m_pContext{ nullptr }, m_This{ nullptr } {};
    
      CopyStatistics*	      m_Stats;
      _PathNameStatusList*  m_PathNameStatusList;
      AsyncContext*         m_pContext;
      FileInfoContainer*    m_This;
    };

    int
    _SmartCopy(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    static
    DWORD
    __stdcall
    RunSmartCopy(
      SmartCopyParams* pParams
    );

    // TrueSize Related Methods
    int
    _TrueSize(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    // SmartClone Related Methods
    int
    _SmartClone(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    static
    DWORD
    __stdcall
    RunSmartClone(
      SmartCopyParams* pParams
    );

    // SmartMove Related Methods
    int
    _SmartMove(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    static
    DWORD
    __stdcall
    RunSmartMove(
      SmartCopyParams* pParams
    );

    // SmartClean Related Methods
    int
    _SmartClean(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    static
    DWORD
    __stdcall
    RunSmartClean(
      SmartCopyParams* pParams
    );

    // SmartMirror Related Methods
    int
    _SmartMirror(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    static
    DWORD
    __stdcall
    RunSmartMirror(
      SmartCopyParams* pParams
    );

    // Dispose Related Methods
    struct DisposeParams
    {
      DisposeParams() : m_pContext { nullptr }, m_This{ nullptr }, m_pStats{ nullptr } {};

      AsyncContext*         m_pContext;
      FileInfoContainer*    m_This;
      CopyStatistics*	      m_pStats;
    };

    static
    DWORD
    __stdcall
    RunDispose(
      DisposeParams* pParams
    );

    int
    _Dispose(
      AsyncContext*         apContext,
      CopyStatistics*	      aStats
    );

    int
    CopyDirectoriesSilent(
      _Pathes::iterator&	      aBegin,
      _Pathes::iterator&	      aEnd,
      AsyncContext*             apContext,
      CopyReparsePointFlags     aFlags
    );

		int
		CopyDirectories(
			_Pathes::iterator&	      aBegin,
			_Pathes::iterator&	      aEnd,
			CopyStatistics*		        pStats,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext,
      CopyReparsePointFlags     aFlags
		);

    bool
    DeleteItem(
      wchar_t*                  a_FileName,
      FileInfo*                 a_pFileInfo,
      _PathNameStatusList*      a_PathNameStatusList,
      CopyStatistics*		        a_pStats
    );

    int
    CopyReparsePoints_Junction_SmartCopy(
      wchar_t*                  a_pDestpath,
      wchar_t*                  a_pJunctionDestTarget,
      wchar_t**                 a_pSourceFilename,
      CopyStatistics*		        a_pStats,
      _PathNameStatusList*      a_pPathNameStatusList
    );

    void
    CopyReparsePoints_Junction_Statistics(
      int&                      a_result,
      const wchar_t*            a_pSourceFilename,
      CopyStatistics*           a_pStats,
      _PathNameStatusList*      a_pPathNameStatusList
    );

    int
    CopyReparsePoints_Mountpoint_SmartCopy(
      wchar_t*                  a_pDestpath,
      wchar_t*                  a_pJunctionDestTarget,
      wchar_t**                 a_pSourceFilename,
      CopyStatistics*		        a_pStats,
      _PathNameStatusList*      a_pPathNameStatusList
    );

    void
    CopyReparsePoints_Mountpoint_Statistics(
      int&                      a_result,
      wchar_t*                  a_pSourceFilename,
      CopyStatistics*		        a_pStats,
      _PathNameStatusList*      a_pPathNameStatusList
    );

    void
    CopyReparsePoints_Symlink_Statistics(
      int&                   a_result,
      wchar_t*                  a_pSourceFilename,
      CopyStatistics*		        a_pStats,
      _PathNameStatusList*      a_pPathNameStatusList
    );

    int
		CopyReparsePoints(
			const _Pathes::iterator&	aBegin,
			const _Pathes::iterator&	aEnd,
			CopyStatistics*		        pStats,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext,
      CopyReparsePointFlags     aFlags
		);

    int
    CopyHardlinks(
	    _Pathes::iterator&	      aBegin,
	    _Pathes::iterator&	      aEnd,
	    CopyStatistics*		        pStats,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext,
      CopyReparsePointFlags     aFlags
    );

		int
		CopyHardlink(
			PWCHAR				            aDestPath,
			size_t&				            aDestPathLen,
      int&                      aDestPathIdx,
			_Pathes::iterator&	      aBegin,
			_Pathes::iterator&	      aEnd,
			CopyStatistics*		        pStats,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext,
      CopyReparsePointFlags     aFlags
		);

    bool
    DeleteDirectoryAndReport(
      wchar_t*                  a_FileName,
      _PathNameStatusList*      a_pPathNameStatusList
    );

    bool
    DeleteFileAndReport(
      wchar_t*                  aDestPath,
      _PathNameStatusList*      a_pPathNameStatusList
    );

    int
    RestoreTimeStamps(
      _Pathes::iterator&	      aBegin,
      _Pathes::iterator&	      aEnd,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext,
      CopyReparsePointFlags     aFlags
    );

    int
    CopyDirectoryAttributes(
      _Pathes::iterator&	      aBegin,
      _Pathes::iterator&	      aEnd,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext,
      CopyReparsePointFlags     aFlags
    );

    int
    CloneFiles(
	    _Pathes::iterator&	      aBegin,
	    _Pathes::iterator&	      aEnd,
	    CopyStatistics*		        pStats,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext
    );

    int
    CleanItems(
	    _Pathes::iterator&	      aBegin,
	    _Pathes::iterator&	      aEnd,
	    CopyStatistics*		        pStats,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext,
      DWORD                     aItemType
    );

    int
    CalcSize(
      _Pathes::iterator&	      aBegin,
      _Pathes::iterator&	      aEnd,
      CopyStatistics*		        pStats,
      _PathNameStatusList*      aPathNameStatusList,
      AsyncContext*             apContext
    );

    // FindHardlink related methods
    struct FindHardLinkParams
    {
      FindHardLinkParams() : m_RefCount{ 0 }, m_Stats { nullptr }, m_PathNameStatusList{ nullptr }, m_pContext{ nullptr }, m_This{ nullptr }{};

      _ArgvList             m_SrcPathList;
      int		                m_RefCount;
	    CopyStatistics*	      m_Stats;
      _PathNameStatusList*  m_PathNameStatusList;
      AsyncContext*         m_pContext;
      FileInfoContainer*    m_This;
    };

		int
    _FindHardLink(
      _ArgvList&              aSrcPath, 
      int		                  aRefCount,
      CopyStatistics*	        aStats,
      _PathNameStatusList*    aPathNameStatusList,
      AsyncContext*           apContext
		);

		int
    _FindHardLinkRecursive(
      LPWSTR	                aSrcPath,
      const size_t            aSrcSize,
      PWCHAR*	                aReparsePointReferencePath,
      int&                    aCurrentBoundaryCross,
      int		                  aRefCount,
      __int64&                aFileAddedOneLevelBelow,
      CopyStatistics*	        aStats,
      _PathNameStatusList*    aPathNameStatusList,
      AsyncContext*           apContext
		);

		int
    _FindHardLinkTraditionalRecursive(
      LPWSTR	                aSrcPath,
      const size_t            aSrcSize,
      PWCHAR*	                aReparsePointReferencePath,
      int&                    aCurrentBoundaryCross,
      int		                  aRefCount,
      __int64&                aFileAddedOneLevelBelow,
      CopyStatistics*	        aStats,
      _PathNameStatusList*    aPathNameStatusList,
      AsyncContext*           apContext
		);

    bool IsOuterReparsePoint(
      PWCHAR                  aSrcPath,
      PWCHAR*                 aCurrentReparsePointReferencePath,
      PWCHAR                  aReparsePointReferencePath,
      PWCHAR*                 aPreviousReparsePointReferencePath,
      DWORD&                  aFileAttributes,
      bool&                   aReparsePointBoundaryCrossed,
      int&                    aCurrentBoundaryCross,
      int&                    aPreviousBoundaryCross,
      PWCHAR                  aReparseSrcTargetHint,
      _PathNameStatusList*    aPathNameStatusList,
      bool&                   aFatalReparseError,
      CopyStatistics*	        aStats

    );

    //--------------------------------------------------------------------
    // 
    // Walks through all delayed ReparsePoints and searched the destination
    // among files. There are two results
    // o) If it finds them, then the symbolic link is a new inner symbolic
    //    link. The ReparseDstTarget is created, and the type is changed
    //    to FILE_ATTRIBUTE_REPARSE_POINT
    // o) It does not find it, then a copy will be created.
    //    The file attribute is changed to FILE_ATTRIBUTE_NORMAL
    // 
    //--------------------------------------------------------------------
    _Pathes::iterator
    ResolveDelayedReparsePoints(
       _Pathes::iterator      aDelayedReparsePointBegin, 
       _Pathes::iterator      aFilenameBegin, 
       _Pathes::iterator      aDirectoryBegin,
      CopyStatistics*	        pStats
    );


		static 
    DWORD
    __stdcall
    RunFindHardLink(
      FindHardLinkParams* 
      pParams
    );

    void
    CopyRegExpList(
      _StringList* a_pStringList, 
      _StringList& a_StringList
    );

    void
    CompileRegExpList(
      _StringList*  a_pStringList, 
      _RegExpList&  a_RegExpList
    );

    void
    DisposeRegExpList(
      _RegExpList&  a_RegExpList
    );

    bool
    MatchRegExpList(
      wchar_t*      a_String,
      _RegExpList&  a_RegExpList
    );

	protected:
    _Pathes	                  m_Filenames;
    _Pathes	                  m_FilenamesSave;
    _Pathes                   m_DateTimeRestore;

    _RegExpList               m_RegExpInclFileList;
    _RegExpList               m_RegExpInclDirList;
    _RegExpList               m_RegExpExclFileList;
    _RegExpList               m_RegExpExclDirList;
    _RegExpList               m_RegExpUnrollDirList;
    _RegExpList               m_RegExpSpliceDirList;

    _StringList               m_SpliceDirList;

    size_t                    m_StackUsageFindHardlink;
    size_t                    m_LastStackUsageFindHardlink;

    __int64                   m_DateTimeTolerance;
    int                       m_HardlinkLimit;

    enum  
    {
        eRoundOffset = 3
    };
    DWORD                     m_SystemAllocGranularity;
    ULONG64                   m_MaxAdressSpace;
    int                       m_MaxRound;

#if defined USE_VECTOR
    _PathVector               m_PathVector;
#else
    _PathMap                  m_PathMap;
#endif

    FileInfoContainer*        m_pLookAsideFileInfoContainer;
    int                       m_CurDestPathIdx;
    size_t                    m_CurSourcePathLen;
    bool                      m_Prepared;
    int                       m_Flags;
    int                       m_Flags2;

    _Pathes::iterator         m_HardlinkBegin;
    _Pathes::iterator         m_FATFilenameBegin;
    _Pathes::iterator         m_DirectoryBegin;

    DiskSerialCache           m_DiskSerialCache;
    ULONG                     m_CurrentSerialNumber;

    FILE*                     m_OutputFile;

    _DupeGroups		            m_DupeGroups;

    LONG64	                  m_MaxSize;
    LONG64	                  m_MinSize;

	  SYSTEM_INFO	              m_SystemInfo;

    bool*                     m_pJsonWriterState;
    bool                      m_JsonWriterState;

  public:
    AnchorPathCache           m_AnchorPathCache;

    PSECURITY_DESCRIPTOR      m_pSecDesc;
    int                       m_SecDescSize;

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
    wchar_t                   m_ContainerName[20];
#endif

    int
    FillWithRefCount(
      _Pathes::iterator&	      aBegin,
      _Pathes::iterator&	      aEnd
    );

    int
    PrepareRefcounts(
    );

		int
		FindDupes(
			int		saturated,
			int		aPrintSizePos
		);

    enum ItemWeight
    {
      eFileWeight = 105,
      eHardlinkWeight = eFileWeight,
      eDirectoryWeight = 105,
      eTimeStampWeight = 50,
      eReparseWeight = 1
    };

    int 
    Prepare(
      FileInfoContainer::CopyReparsePointFlags  aMode,
      CopyStatistics*	                          pStats,
      Effort *                                  aEffort = NULL
    );

    // Estimate effort for hardlink groups
    void
    EstimateHardlinkGroupEffort(
      _Pathes::iterator&	a_Begin,
      _Pathes::iterator&	a_End,
      Effort* a_Effort
    );

    int
    FileInfoContainer::
    EstimateEffort(
      FileInfoContainer::CopyReparsePointFlags  aMode,
      Effort *                                  aEffort = NULL
    );

    BOOL
    CheckSymbolicLinks();

    int 
    Lookup(
      wchar_t*  a_pRootPathName
    ) 
    {
      return m_DiskSerialCache.Lookup(a_pRootPathName, m_CurrentSerialNumber);
    }

    void 
    Log(
      const wchar_t*  a_pTag,
      const wchar_t*  a_pPathName,
      int             a_Flags,
      int             a_LogLevel
    );

#if defined USE_VECTOR
    void
    SetLookAsideContainer(FileInfoContainer* a_pFileInfoContainer) { m_pLookAsideFileInfoContainer = a_pFileInfoContainer; m_pLookAsideFileInfoContainer->Sort(); };
#else
    void
    SetLookAsideContainer(FileInfoContainer* a_pFileInfoContainer) { m_pLookAsideFileInfoContainer = a_pFileInfoContainer;  };
#endif

    FileInfo* Find(const wchar_t* a_pFilename);
    
#if defined USE_VECTOR
    void Sort() { sort( m_PathVector.begin(), m_PathVector.end(), vector_comp()); };
#endif
    void
    SetIncludeList(_StringList* a_pStringList);

    void
    SetIncludeDirList(_StringList* a_pStringList);

    void
    SetExcludeList(_StringList* a_pStringList);

    void
    SetExcludeDirList(_StringList* a_pStringList);

    void
    SetUnrollDirList(_StringList* a_pStringList);

    void
    SetSpliceDirList(_StringList* a_pStringList);

#if defined _DEBUG
    void DumpPathMap();
#endif

    void
    SetFlags(int aFlags) { m_Flags |= aFlags; }

    void
    SetFlags2(int aFlags) { m_Flags2 |= aFlags; }

    void
    ClearFlags(int aFlags) { m_Flags &= ~aFlags; }

    void SetCurSourcePathlen(size_t aLen) { m_CurSourcePathLen = aLen; }

    void SetOutputFile(FILE* aOutputFile) { m_OutputFile = aOutputFile; }

    void SetDateTimeTolerance(int aDateTimeTolerance) { m_DateTimeTolerance = aDateTimeTolerance; }

    void SetHardlinkLimit(int aHardlinkLimit) { m_HardlinkLimit = aHardlinkLimit; }

    void SetAnchorPath(_ArgvList& a_ArgvList) { m_AnchorPathCache.Add(a_ArgvList);}

    void GetAnchorPath(_ArgvList& a_ArgvList) { m_AnchorPathCache.Get(a_ArgvList);}

    int
    SmartCopy(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    int
    TrueSize(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    int
    SmartClone(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    int
    SmartMove(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    int
    SmartClean(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    int
    SmartMirror(
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    int
    FindHardLink(
      _ArgvList&            aSrcPath,
      int		                aRefCount,
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

    int
    FindSingleUnsaturated(
      int		aPrintSizePos
    );

    int
    EnumHardlinkSiblings(
      PWCHAR						          aSrcPath,
      PWCHAR						          aExcludePath,
      EnumHardlinkSiblingsGlue*	  pEnumHardlinkSiblingsGlue,
      bool						            aQuiet,
      _PathNameStatusList*        aPathNameStatusList,
      AsyncContext*               apContext
    );

    int
 		Dispose(
      AsyncContext*         apContext,
      CopyStatistics*	      apStats
		);

    void
    SetDestPathIdx (int aDestPathIdx) { m_CurDestPathIdx = aDestPathIdx; };

    size_t
    AddAnchorPath(
      _ArgvPath& aAnchorPath
    )
    {
      return m_AnchorPathCache.Add(aAnchorPath);
    }

    void
    ChangePath(
      const wchar_t*  aNewSourcePath,
      _ArgvList&      aNewDestPath 
    );

    enum PersistanceMode
    {
      eVoid = 0,
      eNormalPersistance,
      eDupeMergePersistance
    };

    size_t
    Load(
      FILE*   aSourceFile,
      int     aMode = 0
    );

    size_t
    Save(
      FILE*    aDestFile,
      int      aMode = 0
    );

    size_t
    LoadStringList(
      FILE*         aFile,
      _StringList&  a_StringList
    );

    size_t
    SaveStringList(
      FILE*         aFile,
      _StringList&  a_StringList
    );

    FILE*
    StartLogging(
      LSESettings&   r_LSESettings,
      wchar_t*        a_Tag
    );

    FILE*
    AppendLogging(
    );

    void
    HeadLogging(
      FILE*                 a_File
    );

    void
    EndLogging(
      FILE*                 a_File,
      _PathNameStatusList&  r_PathNameStatusList,
      CopyStatistics&       r_CopyStatistics,
      FileInfoContainer::CopyReparsePointFlags  aMode = FileInfoContainer::eSmartCopy
    );

    void
    StopLogging(
      FILE*                 a_File
    );

    size_t
    Size(
    ) { return m_Filenames.size(); };

    virtual
		~FileInfoContainer();

		class FileIndexCompare
		{
			public:
				bool
				operator()(FileInfo* const first,
						   FileInfo* const second) const
				{
          if ( first->m_DiskIndex < second->m_DiskIndex) 
            return true;
          else
          {
            if (first->m_DiskIndex == second->m_DiskIndex)
            {
              // m_FileIndex comparison
              if ( first->m_FileIndex.ul64 < second->m_FileIndex.ul64) 
                return true;
              else
                return false;
            }
            else
              return false;
          }
				}
		};

		class PathIdxFileIndexSorter
		{
			public:
				bool
				operator()(FileInfo* const first,
						   FileInfo* const second) const
        {
          // First sort by DiskIndex ascending, then by FileIndex ascending, then by m_DestPathIdx ascending
          if ( first->m_DiskIndex < second->m_DiskIndex) 
          {
            return true;
          }
          else
          {
            if (first->m_DiskIndex == second->m_DiskIndex)
            {
              // m_FileIndex comparison
              if ( first->m_FileIndex.ul64 < second->m_FileIndex.ul64) 
              {
                return true;
              }
              else
              {
                if (first->m_FileIndex.ul64 == second->m_FileIndex.ul64)
                {
                  // m_DestPathIdx comparison
                  if (first->m_DestPathIdx < second->m_DestPathIdx)
                    return true;
                  else
                    return false;
                }
                else
                {
                  return false;
                }
              }
            }
            else
            {
              return false;
            }
          }
				}
    };

		class PathIdxFilenameSorterDescending
		{
			public:
				bool
				operator()(FileInfo* const first, FileInfo* const second) const
				{
          // First sort by m_DestPathIdx ascending, then by m_FileName in descending order
          // e.g. 0 d:\1\yyy\z
          //      0 d:\1\yyy
          //      1 d:\1\test\z
          //      1 d:\1\test\y
          //      1 d:\1\test
          if ( first->m_DestPathIdx < second->m_DestPathIdx ) 
            return true;
          else 
          {
            if (first->m_DestPathIdx == second->m_DestPathIdx)
            {
              if (wcscmp (first->m_FileName, second->m_FileName) <= 0)
                return false;
              else
                return true;
            }
            else
              return false;
          }
				}
		};

		class PathIdxFilenameSorterAscending
		{
			public:
				bool
				operator()(FileInfo* const first, FileInfo* const second) const
				{
          // First sort by m_DestPathIdx ascending, then by m_FileName in ascending order
          // e.g. 00 D:\1\DirTEst\yyy 
          //      00 D:\1\DirTEst\yyy\x 
          //      00 D:\1\DirTEst\yyy\z 
          //      01 D:\1\DirTEst\test 
          //      01 D:\1\DirTEst\test\e 
          //      01 D:\1\DirTEst\test\zzz
          if ( first->m_DestPathIdx < second->m_DestPathIdx ) 
            return true;
          else 
          {
            if (first->m_DestPathIdx == second->m_DestPathIdx)
            {
              if (wcscmp (first->m_FileName, second->m_FileName) < 0)
                return true;
              else
                return false;
            }
            else
              return false;
          }
				}
		};

		class FilenameSorterAscending
		{
			public:
				bool
				operator()(FileInfo* const first, FileInfo* const second) const
				{
          return wcscmp(first->m_FileName, second->m_FileName) < 0; 
				}
		};

		class IsDirectory
		{
			public:
				bool
				operator()(FileInfo* const item) const
				{
					return !item->IsDirectory();
				}
		};

		class IsReparsePoint
		{
			public:
				bool
				operator()(FileInfo* const item) const
				{
					return item->IsReparsePoint();
				}
		};

    class IsDelayedReparsePoint
		{
			public:
				bool
				operator()(FileInfo* const item) const
				{
					return item->IsDelayedReparsePoint();
				}
		};
    
    class IsNestedReparsePoint
		{
			public:
				bool
				operator()(FileInfo* const item) const
				{
					return item->IsNestedReparsePoint();
				}
		};
    
    class IsPlainFile
		{
			public:
				bool
				operator()(FileInfo* const item) const
				{
					return item->IsPlainFile();
				}
		};

    class IsHardlink
		{
			public:
				bool
				operator()(FileInfo* const item) const
				{
					return item->IsHardlink();
				}
		};


/*
 *
 *
 *
 * DupeMerge related
 *
 *
 *
 */
	public:
		typedef DWORD (__stdcall *ThrFuncType)(void*);

    enum  DupeMergeDebugFlags
    {
      eNoDupeDebug      = 0x0000,
      eSlurpAfterFind   = 0x0001,
      eDumpAfterFind    = 0x0002,
      eSlurpAfterCalc   = 0x0004,
      eDumpAfterCalc    = 0x0008
    };

		int
		DupeMerge(
      _ArgvList&            aSrcPathList, 
      _StringList*          aIncludeFileList,
      _StringList*          aIncludeDirList,
      _StringList*          aExcludeFileList,
      _StringList*          aExcludeDirList,
      int                   aDebugFlags,
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

		int
		DmDump(
      _Pathes::iterator	  aBegin,
	    _Pathes::iterator	  aEnd
    );

		int
		OutputDupes(
      int     aMode
    );

		int
		DmFindDupes(
      _Pathes::iterator&      begin,
      _Pathes::iterator&      end,
      _PathNameStatusList*    a_PathNameStatusList,
      CopyStatistics*	        pStats
    );

		void
		SetMaxSize(LONG64 aMaxSize)
		{
			m_MaxSize = aMaxSize;
		};

		void
		SetMinSize(LONG64 aMinSize)
		{
			m_MinSize = aMinSize;
		};

	protected:

		ULONG
		NumberOfHardLinkGroups(
								_Pathes::iterator	start,
								_Pathes::iterator	end
    );

    int
    SetFileIndex(
      _Pathes::iterator&	start,
      _Pathes::iterator&	end,
      UL64&               a_FileIndex
    );

    int
    CalcFileIndex(
      _Pathes::iterator&	      start,
      _Pathes::iterator&	      end
    );

    int
    SetRefCount(
      _Pathes::iterator	start,
      _Pathes::iterator	end
    );

    int
    FileInfoContainer::
    CalcRefCount(
      _Pathes::iterator	start,
	    _Pathes::iterator	end
    );

		int
		CheckIfModified(
      FileInfo* a_pFileInfoReference
    );

		int
		FindHashDupes(
      _Pathes::iterator	start,
			_Pathes::iterator	end    
    );

		int
		CopyHashes(
      _Pathes::iterator	start,
			_Pathes::iterator	end
    );

		int
		UpdateHlReps(
      _Pathes::iterator	start,
			_Pathes::iterator	end
    );

		int
		MergeDupes(
      _Pathes::iterator	      last,
      _Pathes::iterator	      iter,
      _PathNameStatusList*    a_PathNameStatusList,
      CopyStatistics*	        pStats
    );

		int
		HardlinkDupes(
      _Pathes::iterator	      last,
			_Pathes::iterator	      iter,
      _PathNameStatusList*    a_PathNameStatusList,
      CopyStatistics*	        pStats
    );

		int
		FindDupeGroups(
      _Pathes::iterator	      start,
      _Pathes::iterator	      end,
      _PathNameStatusList*    a_PathNameStatusList,
      CopyStatistics*	        pStats
    );

    struct CheckDupesParams
    {
      CheckDupesParams() : m_PathNameStatusList{ nullptr }, m_pStats{ nullptr }, m_pContext{ nullptr }, m_This{ nullptr }, m_pTlsStats{ nullptr } {};

      _Pathes::iterator	    m_Start;
      _Pathes::iterator	    m_End;
      _PathNameStatusList*  m_PathNameStatusList;
      CopyStatistics*	      m_pStats;
      AsyncContext*         m_pContext;
      FileInfoContainer*    m_This;
      CopyStatistics*	      m_pTlsStats;
    };

		int
		CheckDupes(
      _Pathes::iterator	      a_Start,
      _Pathes::iterator	      a_End,
      _PathNameStatusList*    a_PathNameStatusList,
      CopyStatistics*	        a_pStats,
      AsyncContext*           a_pContext
    );

    static 
		DWORD
		__stdcall
		RunCheckDupes(
      CheckDupesParams* pParams
    );

		int
		_CheckDupes(
      _Pathes::iterator	      start,
      _Pathes::iterator	      end,
      _PathNameStatusList*    a_PathNameStatusList,
      CopyStatistics*	        pStats,
      AsyncContext*           apContext
    );

    int
    InvalidateFileIndex(
      _Pathes::iterator	      start,
      _Pathes::iterator	      end
    );

    int
    SanitySizeCheck(
      _Pathes::iterator&      start,
	    _Pathes::iterator&      end,
      UL64                    filesize
    );

    int
    SanityCheck(
      _Pathes::iterator	      start,
	    _Pathes::iterator	      end
    );

    ULONG64 
    CalcDataSize(
      int       a_Round,
      ULONG64&  a_CheckSize
    );

		bool
		CreateHashes(
      _Pathes::iterator	      start,
      _Pathes::iterator	      end,
	    int					            round,
      _PathNameStatusList*    a_PathNameStatusList,
      CopyStatistics*	        pStats
    );

		int
		MarkRep(
      _Pathes::iterator	start,
			_Pathes::iterator	end
    );

		int
		MarkSingleHash(
      _Pathes::iterator	start,
			_Pathes::iterator	end
    );

		int
		MarkSingleSize(
      _Pathes::iterator	start,
			_Pathes::iterator	end
    );

		int
		Dispose(
      _Pathes::iterator	    aBegin,
	    _Pathes::iterator	    aEnd,
      CopyStatistics*	      apStats
    );

    int
    DisposeMmf(
      _Pathes::iterator	aBegin,
	    _Pathes::iterator	aEnd
    );

    struct DupeMergeParams
    {
      DupeMergeParams() : m_DebugFlags{ 0 }, m_Stats{ nullptr }, m_PathNameStatusList{ nullptr }, m_pContext{ nullptr }, m_This{ nullptr } {};

      _ArgvList             m_SrcPathList;
      int                   m_DebugFlags;
      CopyStatistics*	      m_Stats;
      _PathNameStatusList*  m_PathNameStatusList;
      AsyncContext*         m_pContext;
      FileInfoContainer*    m_This;
    };

    
    static 
		DWORD
		__stdcall
		RunDupeMerge(
      DupeMergeParams* pParams
    );

		int
		_DupeMerge(
      _ArgvList&            aSrcPathList, 
      int                   aDebugFlags,
      CopyStatistics*	      pStats,
      _PathNameStatusList*  aPathNameStatusList,
      AsyncContext*         apContext
    );

		
		// Create functors for sort 
		class PathesSorter
		{
			public:
				bool
				operator()(
          FileInfo* first,
					FileInfo* second
        )
				{
					return	_wcsicmp (first->m_FileName, second->m_FileName) >=
						0 ?
						false :
						true;
				}
		};

		// Create functors for unique
		class PathCompare
		{
			public:
				bool
				operator()(
          FileInfo* first,
					FileInfo* second
        )
				{
					return	_wcsicmp (first->m_FileName, second->m_FileName) !=
						0 ?
						false :
						true;
				}
		};

		class IsUseful
		{
			public:
				bool
				operator()(FileInfo* item)
				{
					return item->IsUseful ();
				}
		};

		class HashSorter
		{
			public:
				bool
				operator()(
          FileInfo* first,
					FileInfo* second
        )
				{
					return	memcmp (first->m_Hash, second->m_Hash, 16) >= 0 ? false : true;
				}
		};

		class DmSizeSorter
		{
			public:
				bool
				operator()(
          FileInfo* first,
					FileInfo* second
        )
				{
					return first->m_FileSize.ul64 >=
						second->m_FileSize.ul64 ?
						false :
						true;
				}
		};

		class FileIndexSorter
		{
			public:
				bool
				operator()(
          FileInfo* first,
					FileInfo* second
        )
				{
					return first->m_FileIndex.ul64 >= second->m_FileIndex.ul64 ?
						false :
						true;
				}
		};

		class FileIndexHlSorter
		{
			public:
				bool
				operator()(
          FileInfo* first,
					FileInfo* second
        )
				{
					if (first->m_FileIndex.ul64 < second->m_FileIndex.ul64)
					{
						return true;
					}
					else
					{
						if (first->m_FileIndex.ul64 == second->m_FileIndex.ul64)
						{
							if (first->IsHardlinkRep () < second->IsHardlinkRep ())
								return true;
							else
								return false;
						}
						else
							return false;
					}
				}
		};

		class IsHardLink
		{
			public:
				bool
				operator()(
          FileInfo* item
        )
				{
					return 0 != item->m_FileIndex.ul64;
				}
		};

		class IsHardlinkRep
		{
			public:
				IsHardlinkRep()
				{
					m_Flag = true;
				};

        IsHardlinkRep(bool flag)
				{
					m_Flag = flag;
				};

        bool
				operator()(
          FileInfo* item
        )
				{
					return m_Flag ? item->IsHardlinkRep () : !item->IsHardlinkRep ();
				}

			protected:
				bool	m_Flag;
		};

		class IsADupe
		{
			public:
				bool
				operator()(
          FileInfo* item
        )
				{
					return 0 != item->IsADupe ();
				}
		};


		class RefCountSorter
		{
			public:
				bool
				operator()(
          FileInfo* first,
					FileInfo* second
        )
				{
					return first->m_RefCount >= second->m_RefCount ? false : true;
				}
		};

};


wchar_t* GetRelativeFilename(
  wchar_t *relativeFilename, 
  wchar_t *currentDirectory, 
  wchar_t *absoluteFilename
);
