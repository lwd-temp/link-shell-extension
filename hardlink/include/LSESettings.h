/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */

#pragma once

int
GetCurrentSid(LPWSTR* aSid);

//--------------------------------------------------------------------
//
// CopySettings
//
// Copy the settings from HKLM to HKCU
//
//--------------------------------------------------------------------
int CopySettings(
);

int _CopySettings(
  DWORD     a_RegistryView = 0
);

//--------------------------------------------------------------------
//
// ChangegFlags
//
// Change the value of certain bits in gFlags either on or off
//
//--------------------------------------------------------------------
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


//--------------------------------------------------------------------
//
// Set/GetValue
//
// Sets or gets the given value on a REG_DWORD key from LSE Settings
//
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
//
// ChangeValue
//
// Sets or gets the given value on a REG_SZ key from LSE Settings
//
//--------------------------------------------------------------------
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

//--------------------------------------------------------------------
//
// DeleteValue
//
// Deletes the given value from LSE Settings
//
//--------------------------------------------------------------------
int DeleteValue(
  wchar_t*  aValue
);

int _DeleteValue(
  wchar_t*  aValue,
  DWORD     a_RegistryView = 0
);

class LSESettings
{
public:
  LSESettings() {};
  ~LSESettings() {};

  int ReadLSESettings(bool aReadAllSettings = true);
  int GetLanguageID() { return m_LanguageID; };
  int GetFlags() { return m_Flags; };
  int GetHardlinkOverlayPrio() { return m_HardlinkOverlayPrio; };
  int GetJunctionOverlayPrio() { return m_JunctionOverlayPrio; };
  int GetSymboliclinkOverlayPrio() { return m_SymboliclinkOverlayPrio; };

private:
  wchar_t m_LseRegistryLocation[MAX_PATH];
  HKEY m_Key;
  void AssembleLseRegLocation();

  int   m_LanguageID;
  int   m_Flags;
  int   m_HardlinkOverlayPrio;
  int   m_JunctionOverlayPrio;
  int   m_SymboliclinkOverlayPrio;

  void Init();

  const wchar_t*
    GetRegistryLocation() const { return m_LseRegistryLocation; };

public:
  const HKEY
    GetKey() const { return m_Key; }
};

