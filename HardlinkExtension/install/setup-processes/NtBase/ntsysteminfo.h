/*
 *  NtSystemInfo.h
 *
 *  Provide helper functions to access various
 *  NT System Information
 * 
 */

typedef UINT 		NTSTATUS;
typedef void*		PVOID;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#define STATUS_NAME_TOO_LONG             ((NTSTATUS)0xC0000106L)
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)

typedef enum _SYSTEMINFOCLASS {
	SystemProcessListInformation = 0x0005,
  SystemHandleInformation = 16
} SYSTEMINFOCLASS;

typedef struct _THREAD_INFO {
	LARGE_INTEGER	KernelTime;
	LARGE_INTEGER	UserTime;
	LARGE_INTEGER	CreateTime;
	ULONG			dwThreadID;
	ULONG			WaitTime;
	ULONG			StartAddress;
	LARGE_INTEGER	Cid;
	ULONG			dwThreadState;
	ULONG			Priority;
	ULONG			BasePriority;
	ULONG			ContextSwitches;
	ULONG			State;
	ULONG			WaitReason;
	ULONG			Unised;
} THREAD_INFO, *PTHREAD_INFO;

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


#if 0
typedef struct _PROCESS_INFO {
	ULONG			Offset;
	ULONG			ThreadCount;
	ULONG			Unused1;
	ULONG			Unused2;
	ULONG			Unused3;
	ULONG			Unused4;
	ULONG			Unused5;
	ULONG			Unused6;
	FILETIME		CreationTime;
	LARGE_INTEGER	UserTime;
	LARGE_INTEGER	KernelTime;
	ULONG			Unused7;
	WCHAR*			pszProcessName;
	ULONG			BasePriority;
	ULONG			UniqueProcessID;
	ULONG			InheritedFromUniqueProcessID;
	ULONG			HandleCount;
	ULONG			Unused8;
	ULONG			Unused9;
	ULONG			PeakVirtualSize;
	ULONG			VirtualSize;
	ULONG			PageFaults;
	ULONG			WorkingSetPeak;
	ULONG			WorkingSet;
	ULONG			QuotaPeakPoolUsagePaged;
	ULONG			QuotaPoolUsagePaged;
	ULONG			QuotaPeakPoolUsageNonPaged;
	ULONG			QuotaPoolUsageNonPaged;
	ULONG			PagefileUsage;
	ULONG			PeakPagefileUsage;
	ULONG			CommitCharge;
	ULONG			Unused10;
	ULONG			Unused11;
	ULONG			Unused12;
	ULONG			Unused13;
	THREAD_INFO		Threads;
} PROCESS_INFO, *PPROCESS_INFO;
#endif

#if defined _NTQUERYHANDLE

typedef enum _OBJECT_INFORMATION_CLASS {
  ObjectBasicInformation = 0,
  ObjectNameInformation = 1,
  ObjectTypeInformation = 2
} OBJECT_INFORMATION_CLASS;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef enum _POOL_TYPE
{
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS
} POOL_TYPE, *PPOOL_TYPE;

typedef struct _OBJECT_TYPE_INFORMATION
{
    UNICODE_STRING Name;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG TotalPagedPoolUsage;
    ULONG TotalNonPagedPoolUsage;
    ULONG TotalNamePoolUsage;
    ULONG TotalHandleTableUsage;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG HighWaterPagedPoolUsage;
    ULONG HighWaterNonPagedPoolUsage;
    ULONG HighWaterNamePoolUsage;
    ULONG HighWaterHandleTableUsage;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccess;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    USHORT MaintainTypeList;
    POOL_TYPE PoolType;
    ULONG PagedPoolUsage;
    ULONG NonPagedPoolUsage;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO { // Information Class 17
    ULONG ProcessId;
    BYTE ObjectTypeNumber;
    BYTE Flags;
    USHORT Handle;
    PVOID Object;
    ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION { // Information Class 16
    ULONG HandleCount;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

#endif

extern "C" 
{

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
NtQueryObject(
  HANDLE ObjectHandle,
  ULONG ObjectInformationClass,
  PVOID ObjectInformation,
  ULONG ObjectInformationLength,
  PULONG ReturnLength
);


}


