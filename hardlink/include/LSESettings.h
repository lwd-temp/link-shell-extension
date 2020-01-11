/*
 * Copyright (C) 1999 - 2020, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

bool
GetCurrentSid(LPWSTR* aSid);


class LSESettings
{
public:
  LSESettings() :
    m_Sid{ 0 },
    m_LseRegistryLocation { 0 }
  { };
  ~LSESettings() { };

  // Copy the settings from HKLM to HKCU
  int CopySettings(
  );

  int _CopySettings(
    DWORD     a_RegistryView = 0
  );

  // Change the value of certain bits in gFlags either on or off
  int ChangegFlags(
    LSE_Flags aBit,
    bool*     aValue,
    bool      aOnOff
  );

  int _ChangegFlags(
    LSE_Flags aBit,
    bool*     aValue,
    bool      aOnOff,
    DWORD     a_RegistryView = 0
  );

  // Sets or gets the given value on a REG_DWORD key from LSE Settings
  int SetValue(
    wchar_t*  aValue,
    int       aData
  );

  int _SetValue(
    wchar_t*  aValue,
    int       aData,
    DWORD     a_RegistryView = 0
  );

  int GetValue(wchar_t* aValue, int* aData);

  // Sets or gets the given value on a REG_SZ key from LSE Settings
  int ChangeValue(
    wchar_t*  aValue,
    wchar_t*  aData,
    int       aDataLen
  );

  int _ChangeValue(
    wchar_t*  aValue,
    wchar_t*  aData,
    int       aDataLen,
    DWORD     a_RegistryView = 0
  );

  // Deletes the given value from LSE Settings
  int DeleteValue(
    wchar_t*  aValue
  );

  int _DeleteValue(
    wchar_t*  aValue,
    DWORD     a_RegistryView = 0
  );

  int ReadLSESettings(bool aReadAllSettings = true);
  const int GetLanguageID() const { return m_LanguageID; };
  const int GetFlags() const { return m_Flags; };
  const int GetHardlinkOverlayPrio() const { return m_HardlinkOverlayPrio; };
  const int GetJunctionOverlayPrio()const { return m_JunctionOverlayPrio; };
  const int GetSymboliclinkOverlayPrio() const { return m_SymboliclinkOverlayPrio; };
  const HKEY GetUserHive() const { return m_Key; }
  void Init(wchar_t* aSid = nullptr);
  void SetSid(wchar_t* aSid) { wcscpy_s(m_Sid, MAX_PATH, aSid); };

  bool IsSupportedFileSystem(const wchar_t*	aFileSystemName);
  bool ReadFileSystems();
  void Add(const wchar_t*	aFileSystemName) { m_ThirdPartyFileSystems.push_back(aFileSystemName); };

private:
  void AssembleLseRegLocation();

  wchar_t m_LseRegistryLocation[MAX_PATH];
  wchar_t m_Sid[MAX_PATH];
  HKEY m_Key;

  int   m_LanguageID;
  int   m_Flags;
  int   m_HardlinkOverlayPrio;
  int   m_JunctionOverlayPrio;
  int   m_SymboliclinkOverlayPrio;

  _StringList m_ThirdPartyFileSystems;

  const wchar_t*
    GetRegistryLocation() const { return m_LseRegistryLocation; };
};

