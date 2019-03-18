/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

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


