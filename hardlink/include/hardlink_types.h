

#ifndef hardlink_types_h_7988191F_6E30_4dcd_9381_83299250C6A2
#define hardlink_types_h_7988191F_6E30_4dcd_9381_83299250C6A2

#define HUGE_PATH 8192


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
    PathNameStatus() : m_ErrorCode(ERROR_SUCCESS), m_PathName(NULL), m_Operation(0) {};
    PathNameStatus(int a_Operation, const wchar_t* a_pPathName, int aErrorCode);
  
    wchar_t*      m_PathName;
    int           m_ErrorCode;
    int           m_Operation;

public:
    virtual ~PathNameStatus();
};

inline PathNameStatus::PathNameStatus(int a_Operation, const wchar_t* a_pPathName, int aErrorCode)
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

typedef std::vector<PathNameStatus>	_PathNameStatusList;

void DeletePathNameStatusList(_PathNameStatusList &p);


#endif