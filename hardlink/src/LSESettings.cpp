/*
	Copyright (C) 1999 - 2019, Hermann Schinagl, Hermann.Schinagl@gmx.net
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
    wchar_t JunctionIcon[MAX_PATH];
    JunctionIcon[0] = 0x00;
    DWORD JunctionLen;
    RegQueryValueEx(
      RegKey,
      LSE_REGISTRY_JUNCTION_ICON,
      0,
      &aType,
      (LPBYTE)JunctionIcon,
      &JunctionLen
    );

    wchar_t HardlinkIcon[MAX_PATH];
    HardlinkIcon[0] = 0x00;
    DWORD HardlinkLen;
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
    DWORD SymbolicLinkLen;
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

  HANDLE hTok = NULL;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTok))
  {
    // get user info size
    LPBYTE buf = NULL;
    DWORD  dwSize = 0;
    GetTokenInformation(hTok, TokenUser, NULL, 0, &dwSize);
    if (dwSize)
      buf = (LPBYTE)LocalAlloc(LPTR, dwSize);

    // get user info
    if (GetTokenInformation(hTok, TokenUser, buf, dwSize, &dwSize))
    {
      // here's the SID we've looked for
      PSID pSid = ((PTOKEN_USER)buf)->User.Sid;

      // check user name for SID
      DWORD cbUser = 20, cbDomain = 20;
      TCHAR user[20], domain[20];
      SID_NAME_USE nu;
      BOOL bLookup = LookupAccountSid(NULL, pSid, user, &cbUser, domain, &cbDomain, &nu);
      if (bLookup)
      {
        ConvertSidToStringSid(pSid, aSid);
        RetVal = true;
      }
    }
    LocalFree(buf);
    CloseHandle(hTok);
  }

  return RetVal;
}


void
LSESettings::Init()
{
  AssembleLseRegLocation();
  ReadLSESettings(true);
}

void
LSESettings::AssembleLseRegLocation()
{
  wchar_t*  currentSid;
  bool bSidValid = GetCurrentSid(&currentSid);

  if (bSidValid)
  {
    swprintf_s(m_LseRegistryLocation, MAX_PATH, L"%s\\%s", currentSid, LSE_REGISTRY_LOCATION);
    m_Key = HKEY_USERS;
  }
  else
  {
    wcscpy_s(m_LseRegistryLocation, MAX_PATH, LSE_REGISTRY_LOCATION);
    m_Key = HKEY_CURRENT_USER;
  }
  LocalFree(currentSid);
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

    if (HKCUInstalledVersion != LSE_CURRENT_VERSION)
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

        DWORD LSECurrentVersion = LSE_CURRENT_VERSION;
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

