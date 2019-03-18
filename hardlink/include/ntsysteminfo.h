/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

/*
 *  NtSystemInfo.h
 *
 *  Provide helper functions to access various
 *  NT System Information
 * 
 */

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_NO_MORE_FILES             ((NTSTATUS)0x80000006L)
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#define STATUS_NO_SUCH_FILE              ((NTSTATUS)0xC000000FL)
#define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005L)
#define STATUS_NO_EAS_ON_FILE            ((NTSTATUS)0xC0000052L)
#define STATUS_EAS_NOT_SUPPORTED         ((NTSTATUS)0xC000004FL)
#define STATUS_OBJECT_TYPE_MISMATCH      ((NTSTATUS)0xC0000024L)
#define STATUS_NAME_TOO_LONG             ((NTSTATUS)0xC0000106L)
#define STATUS_OBJECT_NAME_INVALID       ((NTSTATUS)0xC0000033L)

// See      http://nologs.com/ntstatus.html
// Mapping: http://support.microsoft.com/kb/113996

#define OBJ_CASE_INSENSITIVE    0x00000040L
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_DIRECTORY_FILE                     0x00000001

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }


typedef LONG	NTSTATUS;

typedef enum _SYSTEMINFOCLASS {
	SystemProcessInformation = 0x0005
} SYSTEMINFOCLASS;

typedef LONG KPRIORITY; 

typedef struct _UNICODE_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef struct _CLIENT_ID {
     PVOID UniqueProcess;
     PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _SYSTEM_PROCESS_INFORMATION {
  ULONG NextEntryOffset;
  ULONG NumberOfThreads;
  LARGE_INTEGER WorkingSetPrivateSize;
  ULONG HardFaultCount;
  ULONG NumberOfThreadsHighWatermark;
  ULONGLONG CycleTime;
  LARGE_INTEGER CreateTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER KernelTime;
  UNICODE_STRING ImageName;
  KPRIORITY BasePriority;
  HANDLE UniqueProcessId;
  HANDLE InheritedFromUniqueProcessId;
  ULONG HandleCount;
  ULONG SessionId;
  ULONG_PTR UniqueProcessKey;
  SIZE_T PeakVirtualSize;
  SIZE_T VirtualSize;
  ULONG PageFaultCount;
  SIZE_T PeakWorkingSetSize;
  SIZE_T WorkingSetSize;
  SIZE_T QuotaPeakPagedPoolUsage;
  SIZE_T QuotaPagedPoolUsage;
  SIZE_T QuotaPeakNonPagedPoolUsage;
  SIZE_T QuotaNonPagedPoolUsage;
  SIZE_T PagefileUsage;
  SIZE_T PeakPagefileUsage;
  SIZE_T PrivatePageCount;
  LARGE_INTEGER ReadOperationCount;
  LARGE_INTEGER WriteOperationCount;
  LARGE_INTEGER OtherOperationCount;
  LARGE_INTEGER ReadTransferCount;
  LARGE_INTEGER WriteTransferCount;
  LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;


typedef struct _SYSTEM_THREAD_INFORMATION {
  LARGE_INTEGER KernelTime;
  LARGE_INTEGER UserTime;
  LARGE_INTEGER CreateTime;
  ULONG WaitTime;
  PVOID StartAddress;
  CLIENT_ID ClientId;
  KPRIORITY Priority;
  LONG BasePriority;
  ULONG ContextSwitches;
  ULONG ThreadState;
  ULONG WaitReason;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;


//
// Object Attributes structure
//
typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

//
// Define the base asynchronous I/O argument types
//
typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef
VOID
(*PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );


typedef enum _FILE_INFORMATION_CLASS
{
   FileDirectoryInformation = 1,
   FileFullDirectoryInformation = 2,
   FileBothDirectoryInformation = 3,
   FileBasicInformation = 4,
   FileStandardInformation = 5,
   FileInternalInformation = 6,
   FileEaInformation = 7,
   FileAccessInformation = 8,
   FileNameInformation = 9,
   FileRenameInformation = 10,
   FileLinkInformation = 11,
   FileNamesInformation = 12,
   FileDispositionInformation = 13,
   FilePositionInformation = 14,
   FileFullEaInformation = 15,
   FileModeInformation = 16,
   FileAlignmentInformation = 17,
   FileAllInformation = 18,
   FileAllocationInformation = 19,
   FileEndOfFileInformation = 20,
   FileAlternateNameInformation = 21,
   FileStreamInformation = 22,
   FilePipeInformation = 23,
   FilePipeLocalInformation = 24,
   FilePipeRemoteInformation = 25,
   FileMailslotQueryInformation = 26,
   FileMailslotSetInformation = 27,
   FileCompressionInformation = 28,
   FileObjectIdInformation = 29,
   FileCompletionInformation = 30,
   FileMoveClusterInformation = 31,
   FileQuotaInformation = 32,
   FileReparsePointInformation = 33,
   FileNetworkOpenInformation = 34,
   FileAttributeTagInformation = 35,
   FileTrackingInformation = 36,
   FileIdBothDirectoryInformation = 37,
   FileIdFullDirectoryInformation = 38,
   FileValidDataLengthInformation = 39,
   FileShortNameInformation = 40,
   FileIoCompletionNotificationInformation = 41,
   FileIoStatusBlockRangeInformation = 42,
   FileIoPriorityHintInformation = 43,
   FileSfioReserveInformation = 44,
   FileSfioVolumeInformation = 45,
   FileHardLinkInformation = 46,
   FileProcessIdsUsingFileInformation = 47,
   FileNormalizedNameInformation = 48,
   FileNetworkPhysicalNameInformation = 49,
   FileMaximumInformation = 50
} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION {
  ULONG         NextEntryOffset;
  ULONG         FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG         FileAttributes;
  ULONG         FileNameLength;
  ULONG         EaSize;
  CCHAR         ShortNameLength;
  WCHAR         ShortName[12];
  LARGE_INTEGER FileId;
  WCHAR         FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef struct _RTL_CURDIR_REF
{
  DWORD       RefCount;
  HANDLE      Handle;
} RTL_CURDIR_REF, *PRTL_CURDIR_REF;


 typedef struct _RTL_RELATIVE_NAME {
  UNICODE_STRING      RelativeName;
  HANDLE              ContainingDirectory;
  PRTL_CURDIR_REF     CurDirRef;
} RTL_RELATIVE_NAME, *PRTL_RELATIVE_NAME;

 typedef struct _FILE_STREAM_INFORMATION { 
  ULONG NextEntryOffset;
  ULONG StreamNameLength;
  LARGE_INTEGER EndOfStream;
  LARGE_INTEGER AllocationSize;
  WCHAR StreamName[1];
} FILE_STREAM_INFORMATION, *PFILE_STREAM_INFORMATION;

 typedef struct _FILE_BASIC_INFORMATION {
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  ULONG         FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_EA_INFORMATION {
  ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_FULL_EA_INFORMATION {
  ULONG  NextEntryOffset;
  UCHAR  Flags;
  UCHAR  EaNameLength;
  USHORT EaValueLength;
  CHAR   EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;


extern "C" 
{

NTSTATUS
NTAPI
NtQueryDirectoryFile( 
  HANDLE			FileHandle,
  HANDLE            Event,
  PIO_APC_ROUTINE   ApcRoutine,
  PVOID             ApcContext,
  PIO_STATUS_BLOCK  IoStatusBlock,
  PVOID             FileInformation,
  ULONG             Length,
  FILE_INFORMATION_CLASS FileInformationClass,
  BOOLEAN ReturnSingleEntry,
  PUNICODE_STRING FileName,
  BOOLEAN RestartScan 
);

NTSTATUS
NTAPI
NtQuerySystemInformation(
	SYSTEMINFOCLASS		InformationClass,
	PVOID				Buffer,
	ULONG				BufferSize,
	PULONG				ReturnedLength
);

NTSTATUS 
NTAPI
NtOpenFile(
  PHANDLE             FileHandle,
  ACCESS_MASK         DesiredAccess,
  POBJECT_ATTRIBUTES  ObjectAttributes,
  PIO_STATUS_BLOCK    IoStatusBlock,
  ULONG               ShareAccess,
  ULONG               OpenOptions
);

NTSTATUS 
NTAPI
NtQuerySecurityObject(
  HANDLE ObjectHandle, 
  SECURITY_INFORMATION    SecurityInformationClass, 
  PSECURITY_DESCRIPTOR    DescriptorBuffer, 
  ULONG                   DescriptorBufferLength, 
  PULONG                  RequiredLength 
);

NTSTATUS 
NTAPI
NtSetSecurityObject(
  HANDLE ObjectHandle, 
  SECURITY_INFORMATION    SecurityInformationClass, 
  PSECURITY_DESCRIPTOR    DescriptorBuffer
);

NTSTATUS 
NTAPI
NtQueryInformationFile(
  HANDLE                  FileHandle, 
  PIO_STATUS_BLOCK        IoStatusBlock, 
  PVOID                   FileInformation, 
  ULONG                   Length, 
  FILE_INFORMATION_CLASS  FileInformationClass 
); 

NTSTATUS 
NTAPI
NtSetInformationFile(
  HANDLE                  FileHandle, 
  PIO_STATUS_BLOCK        IoStatusBlock, 
  PVOID                   FileInformation, 
  ULONG                   Length, 
  FILE_INFORMATION_CLASS  FileInformationClass 
); 

NTSTATUS
NTAPI
NtQueryEaFile(
  IN  HANDLE            FileHandle, 
  OUT PIO_STATUS_BLOCK  IoStatusBlock, 
  OUT PVOID             Buffer, 
  IN  ULONG             Length, 
  IN  BOOLEAN           ReturnSingleEntry, 
  IN  PVOID             EaList, 
  IN  ULONG             EaListLength, 
  IN  PULONG            EaIndex, 
  IN  BOOLEAN           RestartScan 
); 

NTSTATUS
NTAPI
NtSetEaFile(
  IN  HANDLE            FileHandle, 
  OUT PIO_STATUS_BLOCK  IoStatusBlock, 
  IN  PVOID             EaBuffer, 
  IN  ULONG             EaBufferSize
); 

NTSTATUS 
NTAPI
RtlDosPathNameToNtPathName_U(
  LPCWSTR            lpFileName,
  PUNICODE_STRING    PathName,
  PWSTR*             FileName,
  PRTL_RELATIVE_NAME RelativeName
);

NTSTATUS 
NTAPI
RtlNtStatusToDosError(
  NTSTATUS Status
);

}


