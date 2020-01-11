/*
	Copyright (C) 1999 - 2020, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"

#include "hardlink_types.h"
#include "DbgHelpers.h"

#include "LSESettings.h"


//--------------------------------------------------------------------
//
// CopySettings
//
// Copy the settings from HKLM to HKCU
//
//--------------------------------------------------------------------
int LSESettings::CopySettings(
)
{
  DWORD RetVal = _CopySettings();
#if defined _M_X64
  if (ERROR_SUCCESS == RetVal)
    RetVal = _CopySettings(KEY_WOW64_32KEY);
#endif
  return RetVal;
}

int LSESettings::_CopySettings(
  DWORD     a_RegistryView
)
{
  HKEY RegKey;
  DWORD aLanguage = 0;
  DWORD agFlags = 0;
  DWORD HardlinkPrio = 0;
  DWORD JunctionPrio = 0;
  DWORD SymboliclinkPrio = 0;
  DWORD aSize = sizeof(DWORD);
  int r = 0;
  DWORD aType;

  DWORD RetVal = ::RegOpenKeyEx(
    HKEY_LOCAL_MACHINE,
    LSE_REGISTRY_LOCATION,
    0,
    KEY_READ | a_RegistryView,
    &RegKey
  );

  if (ERROR_SUCCESS == RetVal)
  {
    aType = REG_DWORD;
    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_LANGUAGE,
      0,
      &aType,
      (LPBYTE)&aLanguage,
      &aSize
    );

    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_GFLAGS,
      0,
      &aType,
      (LPBYTE)&agFlags,
      &aSize
    );

    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_HARDLINK_OVERLAY_PRIO,
      0,
      &aType,
      (LPBYTE)&HardlinkPrio,
      &aSize
    );

    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_JUNCTION_OVERLAY_PRIO,
      0,
      &aType,
      (LPBYTE)&JunctionPrio,
      &aSize
    );

    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_SYMBOLICLINK_OVERLAY_PRIO,
      0,
      &aType,
      (LPBYTE)&SymboliclinkPrio,
      &aSize
    );

    aType = REG_SZ;
    wchar_t JunctionIcon[MAX_PATH] = { 0 };
    DWORD JunctionLen{ 0 };
    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_JUNCTION_ICON,
      0,
      &aType,
      (LPBYTE)JunctionIcon,
      &JunctionLen
    );

    wchar_t HardlinkIcon[MAX_PATH] = { 0 };
    DWORD HardlinkLen{ 0 };
    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_HARDLINK_ICON,
      0,
      &aType,
      (LPBYTE)HardlinkIcon,
      &HardlinkLen
    );

    wchar_t SymbolicLinkIcon[MAX_PATH];
    SymbolicLinkIcon[0] = 0x00;
    DWORD SymbolicLinkLen = 0;
    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_SYMBOLICLINK_ICON,
      0,
      &aType,
      (LPBYTE)SymbolicLinkIcon,
      &SymbolicLinkLen
    );

    ::RegCloseKey(RegKey);

    // Create Keys under HKCU
    RetVal = RegCreateKeyEx(
      GetUserHive(),
      m_LseRegistryLocation,
      0,
      NULL,
      REG_OPTION_NON_VOLATILE,
      KEY_WRITE | a_RegistryView,
      NULL,
      &RegKey,
      &aSize
    );

    if (ERROR_SUCCESS == RetVal)
    {
      RetVal = RegSetValueEx(
        RegKey,
        LSE_REGISTRY_LANGUAGE,
        0,
        REG_DWORD,
        (LPBYTE)&aLanguage,
        (DWORD) sizeof(aLanguage)
      );

      RetVal = RegSetValueEx(
        RegKey,
        LSE_REGISTRY_GFLAGS,
        0,
        REG_DWORD,
        (LPBYTE)&agFlags,
        (DWORD) sizeof(agFlags)
      );

      RetVal = RegSetValueEx(
        RegKey,
        LSE_REGISTRY_HARDLINK_OVERLAY_PRIO,
        0,
        REG_DWORD,
        (LPBYTE)&HardlinkPrio,
        (DWORD) sizeof(HardlinkPrio)
      );

      RetVal = RegSetValueEx(
        RegKey,
        LSE_REGISTRY_JUNCTION_OVERLAY_PRIO,
        0,
        REG_DWORD,
        (LPBYTE)&JunctionPrio,
        (DWORD) sizeof(JunctionPrio)
      );

      RetVal = RegSetValueEx(
        RegKey,
        LSE_REGISTRY_SYMBOLICLINK_OVERLAY_PRIO,
        0,
        REG_DWORD,
        (LPBYTE)&SymboliclinkPrio,
        (DWORD) sizeof(SymboliclinkPrio)
      );

      if (*JunctionIcon)
        RetVal = RegSetValueEx(
          RegKey,
          LSE_REGISTRY_JUNCTION_ICON,
          0,
          REG_SZ,
          (LPBYTE)JunctionIcon,
          JunctionLen + 1
        );

      if (*HardlinkIcon)
        RetVal = RegSetValueEx(
          RegKey,
          LSE_REGISTRY_HARDLINK_ICON,
          0,
          REG_SZ,
          (LPBYTE)HardlinkIcon,
          HardlinkLen + 1
        );

      if (*SymbolicLinkIcon)
        RetVal = RegSetValueEx(
          RegKey,
          LSE_REGISTRY_SYMBOLICLINK_ICON,
          0,
          REG_SZ,
          (LPBYTE)SymbolicLinkIcon,
          SymbolicLinkLen + 1
        );

      ::RegCloseKey(RegKey);
    }
  }
  return RetVal;
}

//
// ChangeFlags
//
int LSESettings::ChangegFlags(
  LSE_Flags aBit,
  bool*     aValue,
  bool      aOnOff
)
{
  DWORD RetVal = _ChangegFlags(aBit, aValue, aOnOff);
#if defined _M_X64
  if (ERROR_SUCCESS == RetVal)
    RetVal = _ChangegFlags(aBit, aValue, aOnOff, KEY_WOW64_32KEY);
#endif
  return RetVal;
}

int LSESettings::_ChangegFlags(
  LSE_Flags aBit,
  bool*     aValue,
  bool      aOnOff,
  DWORD     a_RegistryView
)
{
  HKEY RegKey;
  DWORD agFlags = 0;
  DWORD aSize = sizeof(DWORD);
  DWORD aType = REG_DWORD;

  DWORD RetVal = ::RegOpenKeyEx(
    GetUserHive(),
    m_LseRegistryLocation,
    0,
    KEY_READ | KEY_WRITE | a_RegistryView,
    &RegKey
  );

  if (ERROR_SUCCESS == RetVal)
  {
    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_GFLAGS,
      0,
      &aType,
      (LPBYTE)&agFlags,
      &aSize
    );

    if (!aValue)
    {
      // Set a a value 
      if (aOnOff)
        agFlags |= aBit;
      else
        agFlags &= ~aBit;

      RetVal = RegSetValueEx(
        RegKey,
        LSE_REGISTRY_GFLAGS,
        0,
        REG_DWORD,
        (LPBYTE)&agFlags,
        (DWORD) sizeof(agFlags)
      );
    }
    else
    {
      // Get a a value 
      *aValue = agFlags & aBit ? true : false;
    }

    ::RegCloseKey(RegKey);
  }
  return RetVal;
}

int LSESettings::SetValue(
  wchar_t*  aValue,
  int       aData
)
{
  DWORD RetVal = _SetValue(aValue, aData);
#if defined _M_X64
  if (ERROR_SUCCESS == RetVal)
    RetVal = _SetValue(aValue, aData, KEY_WOW64_32KEY) != ERROR_SUCCESS;
#endif

  return RetVal;
}

int LSESettings::_SetValue(
  wchar_t*  aValue,
  int       aData,
  DWORD     a_RegistryView
)
{
  HKEY RegKey;
  DWORD aSize = sizeof(DWORD);
  DWORD aType = REG_DWORD;

  DWORD RetVal = ::RegOpenKeyEx(
    GetUserHive(),
    m_LseRegistryLocation,
    0,
    KEY_READ | KEY_WRITE | a_RegistryView,
    &RegKey
  );

  if (ERROR_SUCCESS == RetVal)
  {
    RetVal = RegSetValueEx(
      RegKey,
      aValue,
      0,
      REG_DWORD,
      (LPBYTE)&aData,
      aSize
    );
    ::RegCloseKey(RegKey);
  }
  return RetVal;
}

int LSESettings::GetValue(wchar_t* aValue, int* aData)
{
  HKEY RegKey;
  DWORD aSize = sizeof(DWORD);
  DWORD aType = REG_DWORD;

  DWORD RetVal = ::RegOpenKeyEx(
    GetUserHive(),
    m_LseRegistryLocation,
    0,
    KEY_READ,
    &RegKey
  );

  if (ERROR_SUCCESS == RetVal)
  {
    RetVal = RegQueryValueEx(
      RegKey,
      aValue,
      0,
      &aType,
      (LPBYTE)aData,
      &aSize
    );
    ::RegCloseKey(RegKey);
  }
  return RetVal;
}

//
// ChangeValue
//
int LSESettings::ChangeValue(
  wchar_t*  aValue,
  wchar_t*  aData,
  int       aDataLen
)
{
  DWORD RetVal = _ChangeValue(aValue, aData, aDataLen);
#if defined _M_X64
  if (ERROR_SUCCESS == RetVal)
    RetVal = _ChangeValue(aValue, aData, aDataLen, KEY_WOW64_32KEY) != ERROR_SUCCESS;
#endif

  return RetVal;
}

int LSESettings::_ChangeValue(
  wchar_t*  aValue,
  wchar_t*  aData,
  int       aDataLen,
  DWORD     a_RegistryView
)
{
  HKEY RegKey;
  DWORD aType = REG_SZ;

  DWORD RetVal = ::RegOpenKeyEx(
    HKEY_CURRENT_USER,
    LSE_REGISTRY_LOCATION,
    0,
    KEY_READ | KEY_WRITE,
    &RegKey
  );

  if (ERROR_SUCCESS == RetVal)
  {
    if (!*aData)
    {
      RegQueryValueEx(
        RegKey,
        aValue,
        0,
        &aType,
        (LPBYTE)aData,
        (LPDWORD)&aDataLen
      );
    }
    else
    {
      RetVal = RegSetValueEx(
        RegKey,
        aValue,
        0,
        REG_SZ,
        (LPBYTE)aData,
        aDataLen
      );
    }

    ::RegCloseKey(RegKey);
  }
  return RetVal;
}

//
// DeleteValue
//
int LSESettings::DeleteValue(
  wchar_t* aValue
)
{
  DWORD RetVal = _DeleteValue(aValue);
#if defined _M_X64
  if (ERROR_SUCCESS == RetVal)
    RetVal = _DeleteValue(aValue, KEY_WOW64_32KEY) != ERROR_SUCCESS;
#endif

  return RetVal;
}

int LSESettings::_DeleteValue(
  wchar_t*  aValue,
  DWORD     a_RegistryView
)
{
  HKEY RegKey;
  DWORD aType = REG_SZ;

  DWORD RetVal = ::RegOpenKeyEx(
    GetUserHive(),
    m_LseRegistryLocation,
    0,
    KEY_READ | KEY_WRITE | a_RegistryView,
    &RegKey
  );

  if (ERROR_SUCCESS == RetVal)
  {
    RegDeleteValue(
      RegKey,
      aValue
    );
    ::RegCloseKey(RegKey);
  }
  return RetVal;
}

bool GetCurrentSid(LPWSTR* aSid)
{
  int RetVal = false;

  HANDLE hTok = INVALID_HANDLE_VALUE;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTok))
  {
    // get user info size
    LPBYTE buf = nullptr;
    DWORD  dwSize = 0;
    GetTokenInformation(hTok, TokenUser, NULL, 0, &dwSize);
    if (dwSize)
      buf = (LPBYTE)LocalAlloc(LPTR, dwSize);

    // get user info
    if (GetTokenInformation(hTok, TokenUser, buf, dwSize, &dwSize))
    {
      // here's the SID we've looked for
      if (buf)
      {
        PSID pSid = ((PTOKEN_USER)buf)->User.Sid;

        // check user name for SID
        const int bufSize = 128;
        DWORD cbUser = bufSize, cbDomain = bufSize;
        TCHAR* userName = new TCHAR[bufSize];
        TCHAR* domainName = new TCHAR[bufSize];
        SID_NAME_USE nu;
        BOOL bLookup = LookupAccountSid(NULL, pSid, userName, &cbUser, domainName, &cbDomain, &nu);
        if (TRUE == bLookup)
        {
          ConvertSidToStringSid(pSid, aSid);
          RetVal = true;
        }
        else
        {
          // The buffer was too small, so try again with returned buffer size
          if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
          {
            delete[] domainName;
            delete[] userName;
            userName = new TCHAR[cbUser];
            domainName = new TCHAR[cbDomain];
            bLookup = LookupAccountSid(NULL, pSid, userName, &cbUser, domainName, &cbDomain, &nu);
            if (TRUE == bLookup)
            {
              ConvertSidToStringSid(pSid, aSid);
              RetVal = true;
            }
          }
        }
        delete[] domainName;
        delete[] userName;
      }
    }
    LocalFree(buf);
    CloseHandle(hTok);
  }

  return RetVal;
}


void
LSESettings::Init(wchar_t* aSid)
{
  if (aSid)
    wcscpy_s(m_Sid, MAX_PATH, aSid);

  AssembleLseRegLocation();
  ReadLSESettings(true);
}

void
LSESettings::AssembleLseRegLocation()
{
  if (m_Sid[0])
  {
    swprintf_s(m_LseRegistryLocation, MAX_PATH, L"%s\\%s", m_Sid, LSE_REGISTRY_LOCATION);
    m_Key = HKEY_USERS;
  }
  else
  {
    wcscpy_s(m_LseRegistryLocation, MAX_PATH, LSE_REGISTRY_LOCATION);
    m_Key = HKEY_CURRENT_USER;
  }
}

//--------------------------------------------------------------------
//
// GetLSESettings
//
//--------------------------------------------------------------------
int
LSESettings::ReadLSESettings(
  bool          aReadAllSettings
)
{
  HKEY RegKey;
  DWORD aSize = sizeof(DWORD);
  DWORD aType = REG_DWORD;
  int r = 0;

  HTRACE(L"%S: %08x, %s", __FUNCTION__, m_Key, m_LseRegistryLocation);

  DWORD RetVal = ::RegOpenKeyEx(
    GetUserHive(),
    m_LseRegistryLocation,
    0,
    KEY_READ,
    &RegKey
  );

  // Probe if settings are already there
  if (ERROR_SUCCESS != RetVal)
  {
    ::RegCloseKey(RegKey);

    // No get it from the main source under HKLM
    RetVal = CopySettings();

    RetVal = ::RegOpenKeyEx(
      GetUserHive(),
      m_LseRegistryLocation,
      0,
      KEY_READ,
      &RegKey
    );
  }

    // And read the language value
  if (ERROR_SUCCESS == RetVal)
  {
    bool CopygFlags = false;

    // Check if the settings are up to date
    DWORD HKCUInstalledVersion;
    LONG lResult = RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_INSTALLED_VERSION,
      0,
      &aType,
      (LPBYTE)&HKCUInstalledVersion,
      &aSize
    );

    if (lResult != ERROR_SUCCESS)
      CopygFlags = true;

    if (HKCUInstalledVersion != LSE_REG_SCHEMA_VERSION)
      CopygFlags = true;

    // This only happens when you for the first time run a new version
    if (CopygFlags)
    {
      HKEY HKLMRegKey;
      DWORD RetVal = ::RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        LSE_REGISTRY_LOCATION,
        0,
        KEY_READ,
        &HKLMRegKey
      );

      if (ERROR_SUCCESS == RetVal)
      {
        HKEY HKCURegKey;
        DWORD RetVal = ::RegOpenKeyEx(
          GetUserHive(),
          m_LseRegistryLocation,
          0,
          KEY_READ | KEY_WRITE,
          &HKCURegKey
        );

        DWORD NewgFlags;

        aType = REG_DWORD;
        RegQueryValueEx(
          HKLMRegKey,
          LSE_REGISTRY_GFLAGS,
          0,
          &aType,
          (LPBYTE)&NewgFlags,
          &aSize
        );

        DWORD r1 = RegSetValueEx(
          HKCURegKey,
          LSE_REGISTRY_GFLAGS,
          0,
          REG_DWORD,
          (LPBYTE)&NewgFlags,
          (DWORD) sizeof(NewgFlags)
        );

        DWORD LSECurrentVersion = LSE_REG_SCHEMA_VERSION;
        RetVal = RegSetValueEx(
          HKCURegKey,
          LSE_REGISTRY_INSTALLED_VERSION,
          0,
          REG_DWORD,
          (LPBYTE)&LSECurrentVersion,
          (DWORD) sizeof(LSECurrentVersion)
        );

        ::RegCloseKey(HKCURegKey);
        ::RegCloseKey(HKLMRegKey);
      }
    }


    // Now go on and read the values
    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_GFLAGS,
      0,
      &aType,
      (LPBYTE)&m_Flags,
      &aSize
    );

    if (aReadAllSettings)
    {
      RegQueryValueEx(
        RegKey,
        LSE_REGISTRY_LANGUAGE,
        0,
        &aType,
        (LPBYTE)&m_LanguageID,
        &aSize
      );

      RegQueryValueEx(
        RegKey,
        LSE_REGISTRY_HARDLINK_OVERLAY_PRIO,
        0,
        &aType,
        (LPBYTE)&m_HardlinkOverlayPrio,
        &aSize
      );

      RegQueryValueEx(
        RegKey,
        LSE_REGISTRY_JUNCTION_OVERLAY_PRIO,
        0,
        &aType,
        (LPBYTE)&m_JunctionOverlayPrio,
        &aSize
      );

      RegQueryValueEx(
        RegKey,
        LSE_REGISTRY_SYMBOLICLINK_OVERLAY_PRIO,
        0,
        &aType,
        (LPBYTE)&m_SymboliclinkOverlayPrio,
        &aSize
      );
    }

    ::RegCloseKey(RegKey);
  }

  return ERROR_SUCCESS;
}

// SupportedFileSystems
//
const wchar_t* BuiltInFileSystems[] = { L"Ntfs", L"ReFs", NULL };

bool
LSESettings::
IsSupportedFileSystem(
  const wchar_t*	aFileSystemName
)
{
  // Check for hardcoded filesystems
  for (int i = 0; NULL != BuiltInFileSystems[i]; ++i)
    if (!_wcsicmp(aFileSystemName, BuiltInFileSystems[i]))
      return true;

  // Check for filesystems from registry
  for (_StringListIterator iter = m_ThirdPartyFileSystems.begin(); iter != m_ThirdPartyFileSystems.end(); ++iter)
    if (!_wcsicmp(iter->c_str(), aFileSystemName))
      return true;

  return false;
}

bool
LSESettings::
ReadFileSystems()
{
  
  // Known filesystems are maintained in HKLM, since this is a very sensitive settings and 
  // should not be made by any user
  HKEY HKLMRegKey;
  DWORD RetVal = ::RegOpenKeyEx(
    HKEY_LOCAL_MACHINE,
    LSE_REGISTRY_LOCATION,
    0,
    KEY_READ,
    &HKLMRegKey
  );

  if (ERROR_SUCCESS == RetVal)
  {
    DWORD aType = REG_SZ;
    DWORD aSize = MAX_PATH;
    wchar_t SuppFs[MAX_PATH];
    RetVal = RegQueryValueEx(
      HKLMRegKey,
      LSE_REGISTRY_SUPPORTED_FILESYSTEMS,
      0,
      &aType,
      (LPBYTE)SuppFs,
      &aSize
    );

    if (ERROR_SUCCESS == RetVal)
    {
      // Parse Filesystems from the registry
      TCHAR		seps[] = _T(";");
      PTCHAR  NextToken{ nullptr };
      PTCHAR	token = wcstok_s(SuppFs, seps, &NextToken);
      if (token)
        m_ThirdPartyFileSystems.push_back(token);
      while (PTCHAR ppp = wcstok_s(NULL, seps, &NextToken))
        m_ThirdPartyFileSystems.push_back(ppp);
    }

    ::RegCloseKey(HKLMRegKey);
  }
  return true;
}

