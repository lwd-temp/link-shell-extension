/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

#define USE_VECTOR

// define if we don't want to fork LSEUacHelper.exe to perform operations
// but instead do it in hardlinkshellext.dll
// #define UAC_FORCE
// #define UAC_OUTPROC 0

// define if we want to fork LSEUacHelper.exe to perform all operations
// #define UAC_FORCE
// #define UAC_OUTPROC 1

// #define FIND_HARDLINK_DEBUG // DEBUG_DEFINES

// Used in ln.exe to do all operations in sequence. This eases debugging a lot
// #define SEPERATED_CLONE_MIRROR // DEBUG_DEFINES

// Enable the HTRACE Macro even in Release builds
// #define _HTRACE_DEBUG

// HTRACE can output to either console or via OutputDebugString
// #define _HTRACE_OUTPUT_DEBUG_STRING

#define EXACT_PATH_INDEX

// #define LSE_DEBUG //DEBUG_DEFINES

// #define DMDUMP

// #define PRINT_DISPOSE_DURATION

// If you want not to delete the symlinks.args in release builds. Debug keep them anyhow
// #define DEBUG_DO_NOT_DELETE_SYMLINKS_ARGS

// #define DEBUG_REGEXP_SEAN_MALONEY // DEBUG_DEFINES

// #define DEBUG_ARCHIVE_COMPLIANT

// Disable the Stopwatch functionality
// #define DEBUG_STOPWATCH

// Use STL regular Expression system. This is just an experiment, an dit turned out, that regex is 13t imes slower on average 
// for a simple regex which has been transformed from a wildcard. Using one expression on 401000 items takes 1500 mSec with tre
// The same with regex takes 20000 mSec. So for now we will not move to c++ 11 regex, because it is too slow. 
// #define REGEXP_STL


// Test the progress calculation with real world datam but without copying large amounts of files
// Used to record files with effort triples. 
// #define DEBUG_PREDICTION_RECORD

// Used to replay files with effort triples via --adsdev
// #define DEBUG_PREDICTION_REPLAY



// Test a safe delete via DeleteSiblings(), so that Hardlink Attribute Teleportation does
// not interfere the delorean process
// #define DEBUG_DS

#define NTQUERYDIRECTORY_BUFSIZE 100000

#define FAT_SERIAL_NUMBER 0xffffffff

#define STACKSIZE 45000000

#define ERROR_END_OF_RECURSION					30000

#define LSE_REGISTRY_LOCATION L"Software\\LinkShellExtension"
#define REGPATH_FILE_NOT_FOUND_CACHE_LIFETIME L"System\\CurrentControlSet\\Services\\Lanmanworkstation\\Parameters"
#define REGKEY_FILE_NOT_FOUND_CACHE_LIFETIME L"FileNotFoundCacheLifetime"

#define PATH_PARSE_SWITCHOFF L"\\\\?\\" 
#define PATH_PARSE_SWITCHOFF_SIZE (sizeof(PATH_PARSE_SWITCHOFF) - 1) / sizeof(wchar_t)

// e.g \\?\Volume{97410ad7-54ec-11e3-97ab-005056c00008}\ 
#define PATH_VOLUME_GUID L"Volume{" 
#define PATH_VOLUME_GUID_SIZE (sizeof(PATH_VOLUME_GUID) - 1) / sizeof(wchar_t)

#define PATH_LONG_VOLUME_GUID L"\\\\?\\Volume{" 
#define PATH_LONG_VOLUME_GUID_SIZE (sizeof(PATH_LONG_VOLUME_GUID) - 1) / sizeof(wchar_t)
#define VOLUME_GUID_LENGTH 49

#define PATH_GLOBALROOT L"GLOBALROOT\\" 
#define PATH_GLOBALROOT_SIZE (sizeof(PATH_GLOBALROOT) - 1) / sizeof(wchar_t)

#define PATH_LONG_GLOBALROOT L"\\\\?\\GLOBALROOT\\" 
#define PATH_LONG_GLOBALROOT_SIZE (sizeof(PATH_LONG_GLOBALROOT) - 1) / sizeof(wchar_t)

#define PATH_NAMESPACE_ROOT L"\\??\\" 
#define PATH_NAMESPACE_ROOT_SIZE (sizeof(PATH_NAMESPACE_ROOT) - 1) / sizeof(wchar_t)

#define NULL_STRING L"nul"

#define EXPLORER L"explorer.exe"


#define HARDLINK_TRACELEVEL_BRIEF     0x00
#define HARDLINK_TRACELEVEL_CHANGED   0x10
#define HARDLINK_TRACELEVEL_ALL       0x40

const int cMaxHardlinkLimit = 1023;

#define HUGE_PATH 8192

enum LSE_Flags
{
  eEnableRemote = 1,
  eEnableExtended = 2,
  eDisableSmartmove = 4,
  eForceAbsoluteSymbolicLinks = 8,
  eSymboliclinkOverlay = 0x10,
  eJunctionOverlay = 0x20,
  eHardlinkOverlay = 0x40,
  eDisableSmartMirror = 0x80,
  eDisableDeloreanCopy = 0x100,
  eCropReparsePoints = 0x200,
  eUnrollReparsePoints = 0x400,
  eSpliceReparsePoints = 0x800,
  eLogOutput = 0x1000,
  eBackupMode = 0x2000,
  eDisableJunction = 0x4000
};

enum Operations
{
  Nop = 0,
  PlusF,
  StarH,
  PlusD,
  PlusJ,
  PlusS,
  PlusH,
  PlusR,
  MinusJ,
  MinusM,
  MinusS,
  MinusD,
  MinusF,
  EqualF,     // Used in Dupemerge when moving didn't work
  EqualD,
  EqualJ,
  EqualS,
  EqualH,
  QuestionD,
  QuestionJ,
  QuestionM,
  QuestionS,
  QuestionF,
  QuestionR,
  DangleJ,
  DangleM,
  BackSlashF,
  MemoryMapF,
  CompressionF,
  CompressionD,
  PlusT,      // Alternative data streams could not be copied
  PlusE,      // EA Record could not be processed
  PlusP,       // Sparse Streams could not be processed
  MergeF
};

//
// There are different types of reparsepoints
//
#define REPARSE_POINT_UNDEFINED    -1
#define REPARSE_POINT_FAIL          0
#define REPARSE_POINT_JUNCTION      1
#define REPARSE_POINT_MOUNTPOINT    2
#define REPARSE_POINT_SYMBOLICLINK  3

//
// Define CopyFileEx3 option flags
//
#define COPY_FILE_COPY_SACL                   0x00000010
#define COPY_FILE_COPY_DIRECTORY              0x00000020
#define COPY_FILE_COPY_ACCESS_TIME            0x00000040
#define COPY_FILE_COPY_WRITE_TIME             0x00000080
#define COPY_FILE_COPY_CREATION_TIME          0x00000100
#define COPY_FILE_COPY_SKIP_ADS               0x00000200
#define COPY_FILE_COPY_SKIP_EA                0x00000400
#define COPY_FILE_COPY_REPARSE_POINT          0x00000800


//
// Define the names of settings in registry
//
#define LSE_REGISTRY_LANGUAGE                   L"Language"
#define LSE_REGISTRY_GFLAGS                     L"gFlags"
#define LSE_REGISTRY_INSTALLED_VERSION          L"InstalledVersion"
#define LSE_REGISTRY_HARDLINK_OVERLAY_PRIO      L"HardlinkOverlayPrio"
#define LSE_REGISTRY_JUNCTION_OVERLAY_PRIO      L"JunctionOverlayPrio"
#define LSE_REGISTRY_SYMBOLICLINK_OVERLAY_PRIO  L"SymboliclinkOverlayPrio"
#define LSE_REGISTRY_HARDLINK_ICON              L"Hardlink Icon"
#define LSE_REGISTRY_JUNCTION_ICON              L"Junction Icon"
#define LSE_REGISTRY_SYMBOLICLINK_ICON          L"SymbolicLink Icon"
#define LSE_REGISTRY_SUPPORTED_FILESYSTEMS      L"ThirdPartyFileSystems"

#define SE_INCREASE_WORKINGSET_PRIVILEGE             TEXT("SeIncreaseWorkingSetPrivilege")
#define SE_CREATE_SYMBOLICLINK_PRIVILEGE             TEXT("SeCreateSymbolicLinkPrivilege")


#define SYMLINK_FLAG_FILE 	                    0x00
#define SYMLINK_FLAG_DIRECTORY 	                0x01
#define SYMLINK_FLAG_RELATIVE 	                0x02

// With Windows10 > 14972 SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE came up as flag and was defined to be 0x02
// Unfortunatley all my framework used 0x02 for 10 years to flag that a relative symlink should be created. Grmpf.
// So we have to define our own _ALLOW_UNPRIVILEGED_CREATE value as 0x04, route it through the layers and the very
// bottom translate it to the offical SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE, which is then 0x02
#define SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 	0x04

// And due to still running with VS2005 we have to define that new values too. sic
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE 0x02

#define FILE_ATTRIBUTE_DELAYED_REPARSE_POINT  0x00008000
#define FILE_ATTRIBUTE_NESTED_REPARSE_POINT  0x00010000  
#define FILE_ATTRIBUTE_DEAD_REPARSE_POINT  0x00020000  

#define DRIVE_REMOTE_SHARED      10

#define WINDOWS_VERSION_WIN10   0x0A


 // defines for commandline wrappers
 //
#define ERR_ERROR 		                  1
#define ERR_SUCCESS 		                  0
#define ERR_SOURCE_DIR_DOES_NOT_EXIST 		-1
#define ERR_ARG_IS_NOT_A_DIRECTORY				-2
#define ERR_FILE_ALREADY_EXISTS 				-3
#define ERR_FAILED_TO_CREATE_DIR				-4
#define ERR_FILE_DOES_NOT_EXIST 				-5
#define ERR_HARDLINKS_UNSUPPORTED 				-6
#define ERR_CREATE_HARDLINK_FAILED				-7
#define ERR_LESS_CMD_ARGUMENTS					-8
#define ERR_FAILED_TO_ENUMERATE_FILES			-9
#define ERR_TOO_MANY_LINKS					    -10  // Not yet in html docu
#define ERR_NOT_ON_SAME_VOLUME					-11  // Not yet in html docu
#define ERR_SMARTCOPY_FAILED					-12
#define ERR_NO_HARDLINKGROUPS					-13
#define ERR_SMARTCLONE_FAILED         -14
#define ERR_SMARTMIRROR_FAILED					-15
#define ERR_DELOREANCOPY_FAILED					-16
#define ERR_ACCESS_DENIED					      -17 // Not yet in html docu
#define ERR_OPERATION_NOT_SUPPORTED     -18 // Not yet in html docu and not yet used
#define ERR_SMARTDELETE_FAILED          -19
#define ERR_TARGET_DIR_DOES_NOT_EXIST 		-20 // Not yet in html docu

class _ArgvPath
{
public:
  wstring  Argv;
  wstring  ArgvOrg;
  wstring  ArgvDest;
  int           DriveType;
  int           FileAttribute;
  int           Flags;

  enum {
    Created = 1,
    Anchor = 2,
  };

  _ArgvPath() : DriveType{ 0 }, FileAttribute{ 0 }, Flags{ 0 } {};
};


typedef vector<_ArgvPath>	_ArgvList;
typedef vector<_ArgvPath>::iterator	_ArgvListIterator;

typedef vector<wstring>	_StringList;
typedef vector<wstring>::iterator	_StringListIterator;

typedef set<wstring> _StringSet;



// ------------------
typedef union
{
		struct
		{
				ULONG		ul32l;
				ULONG		ul32h;
		};
		ULONG64	ul64;
		LONG64	l64;
		BYTE	ulchar[sizeof(ULONG64)];
} UL64;

// We have our own filetime structure
typedef union
{
		struct 
    {
				DWORD		dwLowDateTime;
				DWORD		dwHighDateTime;
		};
		ULONG64	ul64DateTime;
    LONG64 l64DateTime;
    FILETIME FileTime;
} FILETIME64;


//
// PathNameStatus
//
class PathNameStatus
{
	public:
    PathNameStatus() : m_ErrorCode{ ERROR_SUCCESS }, m_PathName{ nullptr }, m_Operation{ 0 } {};
    PathNameStatus(const int a_Operation, const wchar_t* a_pPathName, const int aErrorCode);
  
    wchar_t*      m_PathName;
    int           m_ErrorCode;
    int           m_Operation;

public:
    virtual ~PathNameStatus();
};

inline PathNameStatus::PathNameStatus(const int a_Operation, const wchar_t* a_pPathName, const int aErrorCode)
{ 
  size_t len = wcslen(a_pPathName) + 1;
  m_PathName = new wchar_t[len];
  wcscpy_s(m_PathName, len, a_pPathName);
  m_ErrorCode = aErrorCode; 
  m_Operation = a_Operation;
}

inline PathNameStatus::~PathNameStatus()
{
}

// TODO PathNameStatisLIst shall be multithread safe
typedef vector<PathNameStatus>	_PathNameStatusList;

void DeletePathNameStatusList(_PathNameStatusList &p);

// Effort. Needed to store the output of effort estimation. Later used as source data for progressbar
//
class Effort
{
public:
  Effort() : m_Points{0}, m_Size{0}, m_Items{0} { };
  Effort(__int64 aPoints, __int64 aSize, __int64 aItems) { m_Points = aPoints; m_Size = aSize; m_Items = aItems; }

  Effort (const Effort& aEffort) {
    m_Points = aEffort.m_Points.load(); m_Size = aEffort.m_Size.load(); m_Items = aEffort.m_Items.load();
  };

  const Effort& operator= (const Effort& aEffort) {
    if (&aEffort == this) return *this;  m_Points = aEffort.m_Points.load(); m_Size = aEffort.m_Size.load(); m_Items = aEffort.m_Items.load(); return *this;
  };

  Effort& operator+= (const Effort& aEffort) {
    m_Points += aEffort.m_Points.load(); m_Size += aEffort.m_Size.load(); m_Items += aEffort.m_Items.load(); return *this;
  };

  void Reset() { m_Points = 0;  m_Size = 0; m_Items = 0;};

  atomic<int_fast64_t>     m_Points;
  atomic<int_fast64_t>     m_Size;
  atomic<int_fast64_t>     m_Items;
};

inline Effort operator+ (const Effort& aEffort1, const Effort& aEffort2) {
  return Effort(aEffort1.m_Points.load() + aEffort2.m_Points.load(), aEffort1.m_Size.load() + aEffort2.m_Size.load(), aEffort1.m_Items.load() + aEffort2.m_Items.load()); 
};

inline Effort operator- (const Effort& aEffort1, const Effort& aEffort2) {
  return Effort(aEffort1.m_Points.load() - aEffort2.m_Points.load(), aEffort1.m_Size.load() - aEffort2.m_Size.load(), aEffort1.m_Items.load() - aEffort2.m_Items.load());
};


