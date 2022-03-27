/*
  Copyright (C) 1999 - 2020, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"

#define EXTERN

#include "hardlink_types.h"
#include "LSESettings.h"
#include "DbgHelpers.h"

#include "MmfObject.h"

#include "Multilang.h"
#include "AsyncContext.h"

#include "hardlinks.h"
#include "HardlinkUtils.h"

#include "Reparse.h"

#include "NtBase.h"

#include "moduleversion.h"




#pragma hdrstop
#pragma comment( lib, "advapi32.lib" )


void
InitCreateHardlink()
{
  // Get the Windows version
  gVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&gVersionInfo);

  // One might not be able to get the privileges for CreateSymbolicLink with Windows10, especially
  // if you are not running ln.exe from an administrative command prompt.
  BOOL b = EnableTokenPrivilege(SE_CREATE_SYMBOLICLINK_PRIVILEGE);
}

int
HardlinkExists(
  LPCWSTR	toFile,
  FileInfo* pFileInfo
)
{
  int	RetVal = ERROR_SUCCESS;

  // Check if the file to be created is already there, and if 
  // toFile is already tied together as hardlink
  if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(toFile))
  {
    BY_HANDLE_FILE_INFORMATION toFileHandleInfo;
    
    HANDLE	toFileHandle = CreateFileW (toFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE != toFileHandle)
    {
      GetFileInformationByHandle (toFileHandle, &toFileHandleInfo);

      if (pFileInfo->m_FileIndex.ul32h == toFileHandleInfo.nFileIndexHigh && pFileInfo->m_FileIndex.ul32l == toFileHandleInfo.nFileIndexLow)
      {
        SetLastError(ERROR_ALREADY_EXISTS);
        RetVal = ERROR_ALREADY_EXISTS; 
      }
      else
      {
        RetVal = GetLastError();
      }
      CloseHandle(toFileHandle);
    }
    else
    {
      RetVal = GetLastError();
    }
  }

  return RetVal;
}


int
HardlinkExists(
  LPCWSTR	fromFile,
  LPCWSTR	toFile
)
{
  int	RetVal = ERROR_SUCCESS;

  // Check if the file to be created is already there, and if fromFile
  // and toFile are already tied together as hardlink
  if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(toFile))
  {
    BY_HANDLE_FILE_INFORMATION toFileHandleInfo, fromFileHandleInfo;
    
    HANDLE	toFileHandle = CreateFileW (toFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE != toFileHandle)
    {
      GetFileInformationByHandle (toFileHandle, &toFileHandleInfo);

      HANDLE	fromFileHandle = CreateFileW (fromFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
      if (INVALID_HANDLE_VALUE != fromFileHandle)
      {
        GetFileInformationByHandle (fromFileHandle, &fromFileHandleInfo);

        if (fromFileHandleInfo.nFileIndexHigh == toFileHandleInfo.nFileIndexHigh && fromFileHandleInfo.nFileIndexLow == toFileHandleInfo.nFileIndexLow)
        {
          SetLastError(ERROR_ALREADY_EXISTS);
          RetVal = ERROR_ALREADY_EXISTS; 
        }
        CloseHandle(fromFileHandle);
      }
      else
      {
        RetVal = GetLastError();
      }
      CloseHandle(toFileHandle);
    }
    else
    {
      RetVal = GetLastError();
    }
  }

  return RetVal;
}


int
CreateHardlink(
  LPCWSTR	fromFile,
  LPCWSTR	toFile,
  FileInfo* pFileInfo
)
{
  int	RetVal = HardlinkExists(toFile, pFileInfo);
  if (ERROR_SUCCESS == RetVal)
    RetVal = TRUE == CreateHardLink(toFile, fromFile, NULL) ? ERROR_SUCCESS : GetLastError();

  return RetVal;
}

int
CreateHardlink(
  __in  LPCWSTR	fromFile,
  __in  LPCWSTR	toFile
)
{

  int	RetVal = HardlinkExists(fromFile, toFile);
  if (ERROR_SUCCESS == RetVal)
    RetVal = TRUE == CreateHardLink(toFile, fromFile, NULL) ? ERROR_SUCCESS : GetLastError();

  return RetVal;
}


/// <summary>Checks if a given path is a very long path</summary>
/// <remarks>
/// </remarks>
/// <param name="lpPathName">
///   PathName
/// </param>
/// <returns>
///   true if it is a very long path, otherwise returns false
/// </returns>
bool
IsVeryLongPath(
   __in LPCWSTR lpPathName
)
{
  bool r = false;

  // See if the string is at least PATH_PARSE_SWITCHOFF_SIZE characters long
  LPWSTR pos = (LPWSTR)lpPathName;
  int count = 0;
  while (*pos && count < PATH_PARSE_SWITCHOFF_SIZE)
  {
    pos++;
    count++;
  }

  if (count > PATH_PARSE_SWITCHOFF_SIZE - 1)
    if (*(__int64*)PATH_PARSE_SWITCHOFF == *(__int64*)lpPathName)
      r = true;

  return r;
}

/// <summary>Resolves a symbolic and returns the absolute target path</summary>
/// <remarks>
/// </remarks>
/// <param name="lpSymlinkFileName">
///   Location of the Symboliclink. 
/// </param>
/// <param name="lpTargetFileName">
///   The target of the symboliclink. This could be either an absolute path
///   or a relative path
/// </param>
/// <param name="lpSymlinkTarget">
///   The resolved target of the symboliclink. 
/// </param>
/// <returns>
///   returns S_OK if lpTargetFileName was a relative path and the path had 
///   to be combined. The result can be found under lpSymlinkTarget
///   returns S_FALSE if lpTargetFileName was a absolute path lpTargetFileName 
///   already is the target
/// </returns>
int
ResolveSymboliclink(
  __in    LPCWSTR   lpSymlinkFileName,
  __in    LPCWSTR   lpTargetFileName,
  __inout LPWSTR    lpSymlinkTarget
)
{
  int SymbolicLinkResolved = ERROR_INVALID_FUNCTION;

#if defined _DEBUG
  DWORD FileAttributes;
#endif

  size_t TargetLength = wcslen(lpTargetFileName);
  if (TargetLength > MAX_PATH)
  {
    if (!IsVeryLongPath(lpTargetFileName))
    {
      // x:\very\long\path

      // We have to prepend \\? because the path is > 256 and has no \\?
      wcscpy_s(lpSymlinkTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);
      wcscat_s(lpSymlinkTarget, HUGE_PATH, lpTargetFileName);
#if defined _DEBUG
      FileAttributes = GetFileAttributes(lpSymlinkTarget);
#endif
    }
    else
    {
      // \\?\x:\very\long\path
#if defined _DEBUG
      FileAttributes = GetFileAttributes(lpTargetFileName);
#endif
    }
  }
  else
  {
    if (IsVeryLongPath(lpTargetFileName))
    {
      // Check if it is a volume-guid path 
      int PathParseSwitchOffSize = 0;
      if (wcsncmp(&lpTargetFileName[PATH_PARSE_SWITCHOFF_SIZE], PATH_VOLUME_GUID, PATH_VOLUME_GUID_SIZE))
      {
        // Normal very long path \\?\x:\bla\foo
        PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
      }
      else
      {
        // It is \\?\Volume{GUID}\ 
      }

      if (PathIsRelative(&lpTargetFileName[PathParseSwitchOffSize]))
      {
        // \\?\..\..\relative\path

        LPWSTR lpSymlinkFileNameParent = PathFindFileName (lpSymlinkFileName);
        if (lpSymlinkFileNameParent != lpSymlinkFileName)
        {
          wchar_t SaveChar = *lpSymlinkFileNameParent;
          *lpSymlinkFileNameParent = 0x00;

          wchar_t SymlinkTarget[HUGE_PATH];
          PathCombine(SymlinkTarget, lpSymlinkFileName, &lpTargetFileName[PATH_PARSE_SWITCHOFF_SIZE]);
          wcscpy_s(lpSymlinkTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);
          PathCanonicalize(&lpSymlinkTarget[PATH_PARSE_SWITCHOFF_SIZE], SymlinkTarget);
#if defined _DEBUG
          FileAttributes = GetFileAttributes(SymlinkTarget);
#endif

          *lpSymlinkFileNameParent = SaveChar;

          SymbolicLinkResolved = ERROR_SUCCESS;
        }
      }
      else
      {
        // \\?\X:\absolute\path
#if defined _DEBUG
        FileAttributes = GetFileAttributes(lpTargetFileName);
#endif
      }
    }
    else
    {
      if (!wcsncmp(lpTargetFileName, PATH_VOLUME_GUID, PATH_VOLUME_GUID_SIZE))
      {
#if defined _DEBUG
        FileAttributes = GetFileAttributes(lpTargetFileName);
#endif
      }
      else
      {
        if (PathIsRelative(lpTargetFileName))
        {
          // ..\..\relative\path

          LPWSTR lpSymlinkFileNameParent = PathFindFileName (lpSymlinkFileName);
          if (lpSymlinkFileNameParent != lpSymlinkFileName)
          {
            wchar_t SaveChar = *lpSymlinkFileNameParent;
            *lpSymlinkFileNameParent = 0x00;

            wchar_t SymlinkTarget[HUGE_PATH];
            PathCombine(SymlinkTarget, lpSymlinkFileName, lpTargetFileName);

            // This is for WindowsXP only, because PathCanonicalize behaves differently on Windows7 and XP
            // Windows7 merges two \\? prefixed pathes into one prefix \\? but XP generates \\?\\?\ 
            int CanonPos = 0;
            if (!IsVeryLongPath(SymlinkTarget))
            {
              wcscpy_s(lpSymlinkTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);
              if (!PathIsUNC(SymlinkTarget))
                CanonPos = PATH_PARSE_SWITCHOFF_SIZE;
            }
            PathCanonicalize(&lpSymlinkTarget[CanonPos], SymlinkTarget);

#if defined _DEBUG
            FileAttributes = GetFileAttributes(SymlinkTarget);
#endif

            *lpSymlinkFileNameParent = SaveChar;

            SymbolicLinkResolved = ERROR_SUCCESS;
          }
        }
        else
        {
          // X:\absolute\path
#if defined _DEBUG
          FileAttributes = GetFileAttributes(lpTargetFileName);
#endif
        }
      }
    }
  }

  return SymbolicLinkResolved;
}

int
FixVeryLongPath(
   wchar_t** aPathName
)
{
  if (IsVeryLongPath(*aPathName))
  {
    if (wcslen(*aPathName) > MAX_PATH)
    {
      // This was really a very long path prepended with \\?
      // Everything fine, do nothing
    }
    else
    {
      // This was a path prepended with \\?\, but it did not exceed MAX_PATH

      if (wcsncmp(&(*aPathName)[PATH_PARSE_SWITCHOFF_SIZE], PATH_VOLUME_GUID, PATH_VOLUME_GUID_SIZE))
        // too short and not a \\?\Volume{, so strip off \\?
        *aPathName = &(*aPathName)[PATH_PARSE_SWITCHOFF_SIZE];
      else
      {
        // < MAX_PATH, but \\?\Volume\ => Don't strip off
      }
    }
  }
  else
  {
    // Path not prepended with \\?
    size_t PathNameLength = wcslen(*aPathName);
    if (PathNameLength > MAX_PATH)
    {
      // Path longer than MAX_PATH, so prepend with \\?
      MoveMemory(&(*aPathName)[PATH_PARSE_SWITCHOFF_SIZE], *aPathName, PathNameLength * sizeof(wchar_t));
      CopyMemory(*aPathName, &PATH_PARSE_SWITCHOFF, PATH_PARSE_SWITCHOFF_SIZE * sizeof(wchar_t));
    }
    else
    {
      // This was a normal path and smaller than MAX_PATH. Do nothing
    }
  }
  return 42;
}

///--------------------------------------------------------------------
///
/// CreateSymboliclinkMS
///
/// This is just a wrapper to the orginal MS function, which translates 
//  all the flags in the proper way. There should be only one way to do the
//  ugly translation. Thus it is also inline
///
//--------------------------------------------------------------------

inline BOOL CreateSymboliclinkMS(
   LPCWSTR   lpSymlinkFileName,
   LPCWSTR   lpTargetFileName,
   DWORD     dwFlags
)
{
  // The original CreateSymbolicLink(), does not understand SYMLINK_FLAG_RELATIVE, so we have to remove it
  DWORD dwSymLinkFlag = dwFlags & ~SYMLINK_FLAG_RELATIVE;

  // Furthermore the SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE has to translated into SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
  if (dwSymLinkFlag & SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)
  {
    dwSymLinkFlag &= ~SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    dwSymLinkFlag |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
  }
  return CreateSymbolicLink (lpSymlinkFileName, lpTargetFileName, dwSymLinkFlag);
}

///--------------------------------------------------------------------
///
/// CreateSymbolicLink
///
/// This routine creates a NTFS SymbolicLink, using the undocumented
/// FSCTL_SET_REPARSE_POINT structure Win2K uses for mount points
/// and junctions. It allows to create dead symbolic links
///
/// <param name="lpSymlinkFileName">
///   Location of the Symboliclink. 
/// </param>
/// <param name="lpTargetFileName">
///   The target of the symboliclink. This could be either a absolute path
///   or a relative path
/// </param>
/// <param name="dwFlags">
///   Specifies whether a file or directory symbolic link is created
///   switches via FILE_FLAG_BACKUP_SEMANTICS into Symlink creation during backup
/// </param>
/// <param name="lpFakeSymlinkFileName">
///   When this argument is not NULL, this will be the location where the
///   symlink is created. By specifying this argument some can create 
///   dead relative symbolic links. The relative relation is calculated
///   between lpSymlinkFileName and lpTargetFileName as usual, but the link is 
///   created under lpFakeSymlinkFileName, where it creates a dead relative 
///   symbolic link. 
/// </param>
/// <returns>
///   returns S_OK if lpTargetFileName was a relative path and the path had 
///   to be combined. The result can be found under lpSymlinkTarget
///   returns S_FALSE if lpTargetFileName was a absolute path lpTargetFileName 
///   already is the target
/// </returns>
//--------------------------------------------------------------------
int
CreateSymboliclink(
  __in  LPWSTR    lpSymlinkFileName,
  __in  LPCWSTR   lpTargetFileName,
  __in  DWORD     dwFlags,
  __in  LPCWSTR   lpFakeSymlinkFileName
)
{
  int RetVal = ERROR_INVALID_FUNCTION;

  wchar_t SymlinkFileNameTarget[HUGE_PATH];
  wchar_t* pSymlinkFileNameTarget = SymlinkFileNameTarget;
  wcscpy_s(SymlinkFileNameTarget, HUGE_PATH, lpTargetFileName);

  bool bRelativeSymlinkCreated = false;
  BOOL bSymLinkCreation = FALSE;
  if (dwFlags & SYMLINK_FLAG_RELATIVE)
  {
    // Create relative symbolic links
    wchar_t SymbolicPath[HUGE_PATH];
    wchar_t* pSymbolicPath = SymbolicPath;
    LPWSTR lpSymlinkFileParent = NULL;
    wchar_t SaveChar = 0x00;

    bool VeryLongSymlink = IsVeryLongPath(lpSymlinkFileName);

    // If it is a directory, we have to hand over the parent of the 
    // SymbolicLink Directory
    if (dwFlags & SYMLINK_FLAG_DIRECTORY)
    {
      lpSymlinkFileParent = PathFindFileName (lpSymlinkFileName);
      if (lpSymlinkFileParent != lpSymlinkFileName)
      {
        SaveChar = *lpSymlinkFileParent;
        *lpSymlinkFileParent = 0x00;
          
        // Check if we reached the root of a drive e.g. d:\tmp
        if (!PathIsRoot(lpSymlinkFileName))
        {
          *lpSymlinkFileParent = SaveChar;
          *--lpSymlinkFileParent = 0x00;
          SaveChar = 0x00;
        }
      }
    }

    // Check if the location, where symbolic link has to be created, is also a symbolic link
    // If yes *and* the target is on another drive, then resolve it.
    // Do this only for one level
    wchar_t ReparseTarget[HUGE_PATH] = { 0 };
    if (ProbeReparsePoint(lpSymlinkFileName, ReparseTarget))
    {
      bool VeryLongSymlinkFileNameTarget = IsVeryLongPath(ReparseTarget);
      if (&ReparseTarget[VeryLongSymlinkFileNameTarget ? PATH_PARSE_SWITCHOFF_SIZE : 0] !=
        &lpSymlinkFileName[VeryLongSymlink ? PATH_PARSE_SWITCHOFF_SIZE : 0])
      {
        wcscpy_s(SymlinkFileNameTarget, HUGE_PATH, ReparseTarget);
        // TODO return resolved symlink in a fifth parameter
        // TODO resolve more than one level
      }
    }
      

    // PathRelativePathTo() has a flaw when two very long path get compared
    // It assumes that e.g \\?\t:\ and \\?\e:\bla\foo have a common ancestor
    // But this applies only if the \\?\ prefixed path are < MAX_PATH. Unfortunatley
    // it gets worse if the \\?\ path are longer than 256 chars. Then this method
    // can't be used
    //
    // TODO: This is why we have *not* to run this with very long path. ;-((
    BOOL bHasCommonAncestor;
#if 0 // DEBUG_DEFINES
    bHasCommonAncestor = (bool)GetRelativeFilename(SymbolicPath, &lpSymlinkFileName[PATH_PARSE_SWITCHOFF_SIZE],&SymlinkFileNameTarget[PATH_PARSE_SWITCHOFF_SIZE]);
#else
    bool VeryLongTarget = IsVeryLongPath(SymlinkFileNameTarget);
    bHasCommonAncestor = PathRelativePathTo(SymbolicPath,
      &lpSymlinkFileName[VeryLongSymlink ? PATH_PARSE_SWITCHOFF_SIZE : 0],
      dwFlags & SYMLINK_FLAG_DIRECTORY ? FILE_ATTRIBUTE_DIRECTORY : 0,
      &SymlinkFileNameTarget[VeryLongTarget ? PATH_PARSE_SWITCHOFF_SIZE : 0],
      dwFlags & SYMLINK_FLAG_DIRECTORY ? FILE_ATTRIBUTE_DIRECTORY : 0
    );
#endif
    if (dwFlags & SYMLINK_FLAG_DIRECTORY)
    {
      if (SaveChar)
        *lpSymlinkFileParent = SaveChar;
      else
        *lpSymlinkFileParent = '\\';
    }

    // Create relative Symbolic link targets
    if (bHasCommonAncestor)
    {
      // Create a relative symbolic link between to normal path ( < MAX_PATH )
      //
      // If there is a relative path between two very long path, with a normal
      // CreateSymbolicLink this can not acchieved.
      if (wcslen(&lpSymlinkFileName[PATH_PARSE_SWITCHOFF_SIZE]) < MAX_PATH && wcslen(&SymlinkFileNameTarget[PATH_PARSE_SWITCHOFF_SIZE]) < MAX_PATH)
      {
        // PathRelativePathTo() returns '.\bla' for pathes in the same location
        // which also is just 'bla', so chop them of in front
        if (SymbolicPath[0] == '.' && SymbolicPath[1] == '\\' )
          pSymbolicPath += 2;
          
        if (lpFakeSymlinkFileName)
          // Create a dead symbolic link. Only CreateSymboliclinkRaw() can do this
          bSymLinkCreation = CreateSymboliclinkRaw (lpFakeSymlinkFileName, pSymbolicPath, dwFlags) == ERROR_SUCCESS ? TRUE : FALSE;
        else
        {
          // CreateSymbolicLink does not work when used with SE_BACKUPNAME, but CreateSymbolcLinkRaw() does
          if (dwFlags & FILE_FLAG_BACKUP_SEMANTICS)
            bSymLinkCreation = CreateSymboliclinkRaw (lpSymlinkFileName, pSymbolicPath, dwFlags) == ERROR_SUCCESS ? TRUE : FALSE;
          else
          {
            // The original CreateSymbolicLink(), does not understand the content of dwFlags, thus translate it
            bSymLinkCreation = CreateSymboliclinkMS(lpSymlinkFileName, pSymbolicPath, dwFlags);
          }
        }

        bRelativeSymlinkCreated = true;
      }
      else
      {
        // Create a relative symbolic link between to very long path ( > MAX_PATH )
        // Currently we can not use this because PathRelativePathTo() is broken for path > MAX_PATH
        // There is a method PathRelativ under test in this lib. But to create relative symlinks
        // between very long path is more an academic problem, so this is the reason the code is commented out
        //
        // bSymLinkCreation = CreateSymboliclinkRaw (lpSymlinkFileName, pSymbolicPath, dwFlags);
        // bRelativeSymlinkCreated = true;

        // So we create it absolute
        dwFlags &= ~SYMLINK_FLAG_RELATIVE;
      }
    } // if (bHasCommonAncestor)
    else
      dwFlags &= ~SYMLINK_FLAG_RELATIVE;
  } // if (dwFlags & SYMLINK_FLAG_RELATIVE)


  // Create absolute Symbolic link targets. This is because
  // it was either requested, or relative creation didn't work
  // because the target was on a different drive
  if (!bRelativeSymlinkCreated)
  {
    FixVeryLongPath(&lpSymlinkFileName);
    FixVeryLongPath(&pSymlinkFileNameTarget);

    // If we go to a volumeguid, we always have to be absolute. Windows7 would be that robust to also accept relative
    // symlinks to \\?\VolumeGUID, but the XP driver is not
    if (!wcsncmp(pSymlinkFileNameTarget, PATH_LONG_VOLUME_GUID, PATH_LONG_VOLUME_GUID_SIZE))
      dwFlags &= ~SYMLINK_FLAG_RELATIVE;

    if (lpFakeSymlinkFileName)
      bSymLinkCreation = CreateSymboliclinkRaw (lpFakeSymlinkFileName, pSymlinkFileNameTarget, dwFlags & ~SYMLINK_FLAG_RELATIVE) == ERROR_SUCCESS ? TRUE : FALSE;
    else
    {
      if (dwFlags & FILE_FLAG_BACKUP_SEMANTICS)
        bSymLinkCreation = CreateSymboliclinkRaw (lpSymlinkFileName, pSymlinkFileNameTarget, dwFlags & ~SYMLINK_FLAG_RELATIVE) == ERROR_SUCCESS ? TRUE : FALSE;
      else
      {
        // The original CreateSymbolicLink(), does not understand the content of dwFlags, thus translate it
        bSymLinkCreation = CreateSymboliclinkMS (lpSymlinkFileName, pSymlinkFileNameTarget, dwFlags);

        // MS changed the return type of CreateSymbolicLink for Windows10 from BOOLEAN to int. Idiots!
        // But success is still BOOL true, and everything above shall be treated as an error
        if ((gVersionInfo.dwMajorVersion >= WINDOWS_VERSION_WIN10) && (int)bSymLinkCreation > 1)
        {
          SetLastError(bSymLinkCreation);
          bSymLinkCreation = FALSE;
        }
      }
    }
  }

  if (bSymLinkCreation)
    RetVal = ERROR_SUCCESS;
  else 
    RetVal = GetLastError();

  return RetVal;
}

void DeletePathNameStatusList(_PathNameStatusList &p)
{
  for (_PathNameStatusList::iterator i = p.begin(); i != p.end(); ++i)
    delete [] (*i).m_PathName;
}

BOOL RemoveDir (
  __in  LPCWSTR aPath,
  const BOOL aQuiet
)
{
	SetFileAttributesW(
		aPath,
		FILE_ATTRIBUTE_NORMAL
	);

	BOOL bRemoved = RemoveDirectory(aPath);
  if (aQuiet)
    return bRemoved;

  int PrefixPos = 0;
  if (IsVeryLongPath(aPath))
    PrefixPos = PATH_PARSE_SWITCHOFF_SIZE;

  if( bRemoved )
		wprintf( L"Removing '%s'\n", &aPath[PrefixPos]);
	else
		wprintf( L"Could not remove '%s'\n", &aPath[PrefixPos]);

  return bRemoved;
}


int XDel (
  wchar_t*          aSrcPath, 
  const size_t      aSrcSize,
  XDelStatistics&   rXDelStatistics,
  int               aFlags
)
{
  int ReparseType = ProbeReparsePoint(aSrcPath, NULL);
  switch (ReparseType)
  {
    case REPARSE_POINT_SYMBOLICLINK:
    {
      // Unlink SymbolicLink
      DWORD FileAttributes = GetFileAttributes(aSrcPath);
      if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        RemoveDirectoryW(aSrcPath);
      else
        DeleteFileW(aSrcPath);

      rXDelStatistics.SymbolicLinks++;
    }
    break;

    case REPARSE_POINT_JUNCTION:
    {
      // Unlink a Junction
      RemoveDirectoryW(aSrcPath);
      rXDelStatistics.Junctions++;
    }
    break;

    case REPARSE_POINT_FAIL:
      XDel_recursive (aSrcPath, aSrcSize, rXDelStatistics, aFlags);
    break;
  }
	return 42;
}


int XDel_recursive (
  wchar_t*          aSrcPath, 
  const size_t      aSrcSize,
  XDelStatistics&   rXDelStatistics,
  int               aFlags
)
{
  WIN32_FIND_DATAW		wfind;

  size_t sSrcLen = wcslen(aSrcPath);
  wcscat_s(aSrcPath, aSrcSize, L"\\*.*");
  HANDLE sh = FindFirstFileW (aSrcPath, &wfind);
  aSrcPath[sSrcLen] = 0x00;
  if (INVALID_HANDLE_VALUE != sh)
    do
    {
      if (wfind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (wcscmp(wfind.cFileName, L"."))
        {
          if (wcscmp(wfind.cFileName, L".."))
          {
            sSrcLen = wcslen(aSrcPath);
            wcscat_s(aSrcPath, aSrcSize, L"\\");
            wcscat_s(aSrcPath, aSrcSize, wfind.cFileName);

            int ReparseType = ProbeReparsePoint(aSrcPath, NULL);
            switch (ReparseType)
            {
              case REPARSE_POINT_SYMBOLICLINK:
              {
                // Unlink SymbolicLink
                DWORD FileAttributes = GetFileAttributes(aSrcPath);
                if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                  BOOL b = RemoveDirectoryW(aSrcPath);
                  if (!b)
                    wprintf( L"Del Failed '%s': %08x\n", aSrcPath, GetLastError() );   
                }
                else
                {
                  BOOL b = DeleteFileW(aSrcPath);
//                  if (!b)
//                    wprintf( L"Del Failed '%s': %08x\n", aSrcPath, GetLastError() );   
                }

                rXDelStatistics.SymbolicLinks++;
              }
              break;

              case REPARSE_POINT_JUNCTION:
              {
                // Unlink a reparse point
                RemoveDirectoryW(aSrcPath);
                rXDelStatistics.Junctions++;
              }
              break;

              case REPARSE_POINT_FAIL:
              {
                // recursivley delete files
                XDel_recursive (aSrcPath, aSrcSize, rXDelStatistics, aFlags);
                if (0 == (aFlags & FileInfoContainer::eDelOnlyReparsePoints))
                {
                  // Delete Dir and file only if selected
                  rXDelStatistics.Directories++;
                  BOOL b = RemoveDir(aSrcPath, aFlags & FileInfoContainer::eQuiet ? TRUE : FALSE);
                  if (!b)
                    wprintf( L"Remove Dir Failed '%s': %08x\n", aSrcPath, GetLastError() );   
                }
              }
              break;
            }
            aSrcPath[sSrcLen] = 0x00;
          }
        }
      }
      else
      {
        // Delete Dir and file only if selected
        if (0 == (aFlags & FileInfoContainer::eDelOnlyReparsePoints))
        {
          sSrcLen = wcslen(aSrcPath);
          wcscat_s(aSrcPath, aSrcSize, L"\\");
          wcscat_s(aSrcPath, aSrcSize, wfind.cFileName);
          BOOL b = DeleteSibling(aSrcPath, wfind.dwFileAttributes);
          if( !b && !(aFlags & FileInfoContainer::eQuiet) )
            wprintf( L"Could not delete '%s'\n", aSrcPath );

          rXDelStatistics.Count++;
          rXDelStatistics.Size += wfind.nFileSizeLow;

          aSrcPath[sSrcLen] = 0x00;
        }
      }
    } while (FindNextFileW (sh, &wfind) > 0);
    FindClose(sh);

    return 42;
}

//--------------------------------------------------------------------
//
// ProbeHardlink
//
//--------------------------------------------------------------------
int 
ProbeHardlink(
  __in LPCTSTR		aFileName
)
{
  BY_HANDLE_FILE_INFORMATION 	FileInformation;
  int							RetVal = -1;

  HANDLE fh = CreateFileW( 
    aFileName,
    0, //GENERIC_READ, 
    0,
    NULL,
    OPEN_EXISTING,
    0,
    NULL 
    );

  if (INVALID_HANDLE_VALUE != fh)
  {
    BOOL r = GetFileInformationByHandle(
      fh,
      &FileInformation
      );

    // Check the item is a file. This is because of Samba
    // reporting . and .. as hardlink to the parent directory
    // causing Samba to report directories with subdirectories
    // as hardlinks
    if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      RetVal = -1;
    }
    else
    {
      if (r)
        RetVal = FileInformation.nNumberOfLinks;
    }

    CloseHandle(fh);
  }

  return RetVal;
}

//--------------------------------------------------------------------
//
// CreateJunction
//
// This routine creates a NTFS junction, using the undocumented
// FSCTL_SET_REPARSE_POINT structure Win2K uses for mount points
// and junctions.
//
// Returns either ERROR_SUCCESS or GetLastError()
//--------------------------------------------------------------------
int
CreateJunction( 
  __in        LPCWSTR LinkDirectory,
  __in        LPCWSTR LinkTarget,
  __in        DWORD   dwFlags
)
{
  DWORD		RetVal = ERROR_SUCCESS;
  // Size assumption: We have to copy 2 path with each HUGE_PATH long onto the structure. So we take 3 times HUGE_PATH
  char		reparseBuffer[HUGE_PATH * 3];
  WCHAR		directoryFileName[HUGE_PATH];
  WCHAR		targetFileName[HUGE_PATH];
  PWCHAR		filePart;
  HANDLE		hFile;
  DWORD		returnedLength;

  //
  // Get the full path referenced by the target
  //
  if( !GetFullPathName( LinkTarget, 
    HUGE_PATH, targetFileName,
    &filePart )) {

      return GetLastError();
  }
  //
  // Get the full path referenced by the directory
  //
  if( !GetFullPathName( LinkDirectory, 
    HUGE_PATH, directoryFileName,
    &filePart )) {

      return GetLastError();
  }
  //
  // Create the link - ignore errors since it might already exist
  //
  BOOL bb = CreateDirectory( LinkDirectory, NULL );
  if (!bb)
  {
    int LastError = GetLastError();
    if (ERROR_ALREADY_EXISTS != LastError)
      return LastError;
    else
    {
      // If a Junction already exists, we have to check if it
      // Points to the same location, and if yes then return 
      // ERROR_ALREADY_EXISTS
      wchar_t Destination[HUGE_PATH] = { 0 };
      BOOL AlreadyExists = ProbeJunction(LinkDirectory, Destination);
      if (AlreadyExists)
      {
        if (!_wcsicmp(Destination, LinkTarget))        
        {
          SetLastError(ERROR_ALREADY_EXISTS);
          return ERROR_ALREADY_EXISTS;
        }
      }
    }
  }

  hFile = CreateFile( 
    LinkDirectory, 
    GENERIC_WRITE, 
    0,
    NULL, 
    OPEN_EXISTING, 
    FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, 
    NULL 
  );
  
  if( INVALID_HANDLE_VALUE == hFile)
    return GetLastError();

  //
  // Make the native target name
  //
  WCHAR       SubstituteName[HUGE_PATH];

  // The target might be
  if (IsVeryLongPath(targetFileName))
  {
    // a very long path: \\?\x:\path\target
    swprintf_s( SubstituteName, HUGE_PATH, L"\\??\\%s", &targetFileName[PATH_PARSE_SWITCHOFF_SIZE] );
  }
  else
  {
    if (targetFileName[0] == L'\\' && targetFileName[1] == L'\\' )
      // a UNC name: \\myShare\path\target
      swprintf_s( SubstituteName, HUGE_PATH, L"\\??\\UNC\\%s", &targetFileName[2] );
    else
      // a normal full path: x:\path\target
      swprintf_s( SubstituteName, HUGE_PATH, _T("\\??\\%s"), targetFileName );
  }

  // Delete the trailing slashes for non root path x:\path\foo\ -> x:\path\foo, but keep x:\
  // Furthermore keep \\?\Volume{GUID}\ for 'root' volume-names
  size_t lenSub = wcslen(SubstituteName);
  if ( (SubstituteName[lenSub - 1] == L'\\') && (SubstituteName[lenSub - 2] != L':') && (SubstituteName[lenSub - 2] != L'}'))
    SubstituteName[lenSub - 1] = 0;

  PREPARSE_DATA_BUFFER reparseJunctionInfo = (PREPARSE_DATA_BUFFER) reparseBuffer;
  memset( reparseJunctionInfo, 0, sizeof(REPARSE_DATA_BUFFER));
  reparseJunctionInfo->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;

  reparseJunctionInfo->JunctionReparseBuffer.SubstituteNameOffset = 0x00;
  reparseJunctionInfo->JunctionReparseBuffer.SubstituteNameLength = (USHORT)(wcslen(SubstituteName) * sizeof(wchar_t));
  wcscpy_s(reparseJunctionInfo->JunctionReparseBuffer.PathBuffer, HUGE_PATH, SubstituteName);

  reparseJunctionInfo->JunctionReparseBuffer.PrintNameOffset = reparseJunctionInfo->JunctionReparseBuffer.SubstituteNameLength + sizeof(wchar_t);
  reparseJunctionInfo->JunctionReparseBuffer.PrintNameLength = (USHORT)(wcslen(targetFileName) * sizeof(wchar_t));
  wcscpy_s(reparseJunctionInfo->JunctionReparseBuffer.PathBuffer + wcslen(SubstituteName) + 1, HUGE_PATH, targetFileName);


  reparseJunctionInfo->ReparseDataLength = 
    reparseJunctionInfo->JunctionReparseBuffer.SubstituteNameLength + 
    reparseJunctionInfo->JunctionReparseBuffer.PrintNameLength +
    FIELD_OFFSET(REPARSE_DATA_BUFFER, JunctionReparseBuffer.PathBuffer[2]) - FIELD_OFFSET(REPARSE_DATA_BUFFER, JunctionReparseBuffer);

  //
  // Set the link
  //
  if( !DeviceIoControl( hFile, FSCTL_SET_REPARSE_POINT,
     reparseJunctionInfo,
     reparseJunctionInfo->ReparseDataLength + REPARSE_MOUNTPOINT_HEADER_SIZE,
    NULL, 0, &returnedLength, NULL )) 
  {	
    RetVal = GetLastError();
    CloseHandle( hFile );
    BOOL bb = RemoveDirectory( LinkDirectory );
    return RetVal;
  }

  CloseHandle( hFile );
  return ERROR_SUCCESS;
}

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
)
{
  DWORD		RetVal = ERROR_SUCCESS;
  char		reparseBuffer[HUGE_PATH * 3];
  WCHAR		SymlinkFileName[HUGE_PATH];
  WCHAR		targetFileName[HUGE_PATH];
  WCHAR   targetNativeFileName[HUGE_PATH];
  PTCHAR	filePart;
  HANDLE	hFile;
  DWORD		returnedLength;
  PREPARSE_DATA_BUFFER reparseInfo = (PREPARSE_DATA_BUFFER) reparseBuffer;

  //
  // Get the full path referenced by the target
  //
  if( !GetFullPathName( lpTargetFileName, HUGE_PATH, targetFileName, &filePart ))
    return GetLastError();

	//
	// Get the full path referenced by the directory if we create absolute symbolic links
	//
  if (dwFlags & SYMLINK_FLAG_RELATIVE)
  {
    wcscpy_s(SymlinkFileName, HUGE_PATH, lpSymlinkFileName);
  }
  else
  {
    if( !GetFullPathName( lpSymlinkFileName, HUGE_PATH, SymlinkFileName, &filePart ))
		  return GetLastError();
  }

	//
	// Make the native target name
	//
  // The target might be
  if (IsVeryLongPath(SymlinkFileName))
  {
    // a very long path: \\?\x:\path\target
    swprintf_s( targetNativeFileName, HUGE_PATH, L"\\??\\%s", &SymlinkFileName[PATH_PARSE_SWITCHOFF_SIZE] );
  }
  else
  {
    if (SymlinkFileName[0] == L'\\' && SymlinkFileName[1] == L'\\' )
      // a UNC name: \\myShare\path\target
      swprintf_s( targetNativeFileName, HUGE_PATH, L"\\??\\UNC\\%s", &SymlinkFileName[2] );
    else
    {
      if (dwFlags & SYMLINK_FLAG_RELATIVE) 
      {
        // a relative path: relativepath
        wcscpy_s(targetNativeFileName, HUGE_PATH, SymlinkFileName);
      }
      else
      {
        // a normal full path: x:\path\target
        swprintf_s( targetNativeFileName, HUGE_PATH, L"\\??\\%s", SymlinkFileName );
      }
    }
  }

  // Only with full path remove the trailing \ for x:\ which results in x:
  if (!(dwFlags & SYMLINK_FLAG_RELATIVE)) 
  {
    size_t lenSub = wcslen(targetNativeFileName);
    if ( (targetNativeFileName[lenSub - 1] == _T('\\')) && (targetNativeFileName[lenSub - 2] != _T(':'))) 
	    targetNativeFileName[lenSub - 1] = 0;
  }

  //
  // Create the link - ignore errors since it might already exist
  //
  if (dwFlags & SYMLINK_FLAG_DIRECTORY)
  {
    if (!CreateDirectory(lpTargetFileName, NULL))
    {
      int r = GetLastError();
      return r;
    }
    hFile = CreateFile( lpTargetFileName, 
      GENERIC_WRITE|GENERIC_READ,
      FILE_SHARE_READ,
      NULL, 
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS,
      NULL );
  }
  else
  {
    // Create file symbolic links
    DWORD dwFlagsAndAttributes = FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
    hFile = CreateFile( lpTargetFileName, 
      GENERIC_WRITE|GENERIC_READ, 
      FILE_SHARE_READ,
      NULL, 
      OPEN_ALWAYS,
      dwFlagsAndAttributes,
      NULL );
  }
  DWORD LastError = GetLastError();
  if( hFile == INVALID_HANDLE_VALUE) 
    return LastError;

  if (ERROR_ALREADY_EXISTS == LastError)
  {
    CloseHandle(hFile);
    return LastError;
  }

  //
  // Build the reparse info
  //
  memset( reparseInfo, 0, sizeof( *reparseInfo ));
  reparseInfo->ReparseTag = IO_REPARSE_TAG_SYMBOLIC_LINK;

  // All offsets are relative to the beginning of PathBuffer

  // Prepare the Printname
  reparseInfo->SymbolicLinkReparseBuffer.PrintNameOffset = 0; 
  reparseInfo->SymbolicLinkReparseBuffer.PrintNameLength = (USHORT)(wcslen(SymlinkFileName) * sizeof(wchar_t));
  wcscpy_s(reparseInfo->SymbolicLinkReparseBuffer.PathBuffer, HUGE_PATH, SymlinkFileName);

  // Prepare the Substitutename
  reparseInfo->SymbolicLinkReparseBuffer.SubstituteNameOffset = reparseInfo->SymbolicLinkReparseBuffer.PrintNameLength;
  reparseInfo->SymbolicLinkReparseBuffer.SubstituteNameLength = (USHORT)(wcslen(targetNativeFileName)  * sizeof(wchar_t));
  wcscpy_s(reparseInfo->SymbolicLinkReparseBuffer.PathBuffer + wcslen(SymlinkFileName), HUGE_PATH, targetNativeFileName);

  reparseInfo->ReparseDataLength = reparseInfo->SymbolicLinkReparseBuffer.PrintNameLength + reparseInfo->SymbolicLinkReparseBuffer.SubstituteNameLength + 12;

  // Translate to Relative == 1, Absolute == 0
  reparseInfo->SymbolicLinkReparseBuffer.Reserved = dwFlags & SYMLINK_FLAG_RELATIVE ? 1 : 0;

  // Translate the values of the hardlink framework to MS values. 
  reparseInfo->SymbolicLinkReparseBuffer.Reserved |= dwFlags & SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE ? SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE : 0;

  //
  // Set the link
  //
  if( !DeviceIoControl( hFile, FSCTL_SET_REPARSE_POINT,
    reparseInfo, 
    reparseInfo->ReparseDataLength + REPARSE_MOUNTPOINT_HEADER_SIZE,
    NULL, 0, &returnedLength, NULL )) 
  {	
    RetVal = GetLastError();
    CloseHandle( hFile );
    if (dwFlags & SYMLINK_FLAG_DIRECTORY)
      RemoveDirectory(lpSymlinkFileName);
    else
      DeleteFile(lpSymlinkFileName);
    return RetVal;
  }
  CloseHandle( hFile );
  return ERROR_SUCCESS;
}



void 
CreateVeryLongPath( 
  __inout   LPWSTR      aDestination,
  __in      LPCWSTR     aPath
)
{
  // Always return targets without the very long path prefix
  if (IsVeryLongPath(aPath))
  {
    wcscpy_s(aDestination, HUGE_PATH, &aPath[PATH_PARSE_SWITCHOFF_SIZE]);
  }
  else
  {
    wcscpy_s(aDestination, HUGE_PATH, aPath);
  }
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
)
{
  int	 RetVal = REPARSE_POINT_FAIL;
  HANDLE   fileHandle;
  BYTE	 reparseBuffer[MAX_REPARSE_SIZE];
  PBYTE	 reparseData;
  PREPARSE_GUID_DATA_BUFFER reparseInfo = (PREPARSE_GUID_DATA_BUFFER) reparseBuffer;
  PREPARSE_DATA_BUFFER msReparseInfo = (PREPARSE_DATA_BUFFER) reparseBuffer;
  DWORD	returnedLength;
  TCHAR	name[HUGE_PATH];
  TCHAR	name1[HUGE_PATH];

  DWORD FileAttributes = GetFileAttributes(FileName);
  if(  FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT && FileAttributes != INVALID_FILE_ATTRIBUTES) 
  {
    //
    // Open the directory
    //
    fileHandle = CreateFile( FileName, 0,
      FILE_SHARE_READ, 
      NULL, 
      OPEN_EXISTING, 
      FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT , 0 );

    if (INVALID_HANDLE_VALUE != fileHandle)
    {
      if( DeviceIoControl( fileHandle, FSCTL_GET_REPARSE_POINT,
        NULL, 0, reparseInfo, sizeof( reparseBuffer ),
        &returnedLength, NULL )) 
      {
        if( IsReparseTagMicrosoft( reparseInfo->ReparseTag )) 
        {
          switch( reparseInfo->ReparseTag ) 
          {
            case IO_REPARSE_TAG_MOUNT_POINT:
            {
              //
              // Mount points and junctions
              //
              reparseData = (PBYTE) &msReparseInfo->MountPointReparseBuffer.PathBuffer;
              wcsncpy_s( name, 
                HUGE_PATH,
                (PWCHAR) (reparseData + msReparseInfo->MountPointReparseBuffer.PrintNameOffset),
                msReparseInfo->MountPointReparseBuffer.PrintNameLength );
              name[msReparseInfo->MountPointReparseBuffer.PrintNameLength] = 0;
              wcsncpy_s( name1, 
                HUGE_PATH,
                (PWCHAR) (reparseData + msReparseInfo->MountPointReparseBuffer.SubstituteNameOffset),
                msReparseInfo->MountPointReparseBuffer.SubstituteNameLength );
              name1[msReparseInfo->MountPointReparseBuffer.SubstituteNameLength] = 0;

              //
              // I use a simple heuristic to differentiate mount points and junctions: a junction
              // specifies a drive-letter based target, so I look for the drive letter colon
              // in the target, which should be in the form \??\D:\target
              //
              PTCHAR p = wcsstr( name1, _T(":") ); 
              if (p) 
              {
                // This is a junction
                if (aDestination) 
                  CreateVeryLongPath(aDestination, --p);
                RetVal = REPARSE_POINT_JUNCTION;
              }
              else 
              {
                // This is a mount point: e.g. \\?\Volume{f6c8b244-1ce4-11e0-a607-005056c00008}
                if (aDestination)
                {
                  wcscpy(aDestination, name1);
                  
                  // There are two ways a mountpoint can be reported: Via \??\\ or \\Device\bla
                  if (!wcsncmp(aDestination, PATH_NAMESPACE_ROOT, PATH_NAMESPACE_ROOT_SIZE))
                    aDestination[1] = '\\';
                }
                RetVal = REPARSE_POINT_MOUNTPOINT;
              }
            }
            break;

            //
            // Symbolic Links
            //
            case IO_REPARSE_TAG_SYMBOLIC_LINK:
            {
              reparseData = (PBYTE) &msReparseInfo->SymbolicLinkReparseBuffer.PathBuffer;
              // Probe for the printname
              wcsncpy_s( name, 
                HUGE_PATH,
                (PWCHAR) (reparseData + msReparseInfo->SymbolicLinkReparseBuffer.PrintNameOffset),
                msReparseInfo->SymbolicLinkReparseBuffer.PrintNameLength );
              name[msReparseInfo->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(wchar_t)] = 0;

              // Probe for the subsitutename
              wcsncpy_s( name1, 
                HUGE_PATH,
                (PWCHAR) (reparseData + msReparseInfo->SymbolicLinkReparseBuffer.SubstituteNameOffset),
                msReparseInfo->SymbolicLinkReparseBuffer.SubstituteNameLength );
              name1[msReparseInfo->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(wchar_t) ] = 0;

              // Are we interested in the target of the symlink?
              if (aDestination)
                CreateVeryLongPath(aDestination, name);
              RetVal = REPARSE_POINT_SYMBOLICLINK;
            }
            break;

#if _WIN32_WINNT == 0x601
            case IO_REPARSE_TAG_DEDUP:
            {
            }
            break;
#endif
          }
        }
      }

      CloseHandle( fileHandle );
    }
  }
  return RetVal;
}

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
)
{
  int r = ProbeReparsePoint(aFileName, aDestination);
  return REPARSE_POINT_JUNCTION == r ? TRUE:FALSE;
}

bool
CheckFileSystemType(
  PTCHAR	aPath,
  int*    aDriveType
)
{
  // Check the filesystemtype
  TCHAR	FileSystemName[HUGE_PATH];
  TCHAR	Path[HUGE_PATH];
  TCHAR	FullPath[HUGE_PATH];

  FileSystemName[0] = 0x00;
  if (PathIsRelative (aPath))
  {
    GetCurrentDirectory (HUGE_PATH, Path);
    PathCombine (FullPath, Path, aPath);
  }
  else
  {
    PTCHAR fname;
    GetFullPathName(aPath, HUGE_PATH, FullPath, &fname);
  }


  DWORD FileSystemFlags;
  return IsFileSystemNtfs(FullPath, &FileSystemFlags, 0, aDriveType);
}


bool
IsFileSystemNtfs (
  const wchar_t*	aPath,
  DWORD*			    dwFileSystemFlags,
  const DWORD			aFlags,
  int*            aDriveType
)
{
  // Check the filesystemtype
  WCHAR		FileSystemName[MAX_PATH];
  FileSystemName[0] = 0x00;
  WCHAR		Drive[HUGE_PATH] = { 0 };
  UINT		DriveType = DRIVE_UNKNOWN;

  int SwitchPos = 0;
  bool VoumeGuid = false;
  bool GlobalRoot = false;
  if (IsVeryLongPath(aPath))
  {
    // Check for \\?\Volume{ 
    if ( !wcsncmp(&aPath[PATH_PARSE_SWITCHOFF_SIZE], PATH_VOLUME_GUID, PATH_VOLUME_GUID_SIZE) )
      // This is \\?\Volume
      VoumeGuid = true; 

    // Check for \\?\GLOBALROOT\ 
    if ( !wcsncmp(&aPath[PATH_PARSE_SWITCHOFF_SIZE], PATH_GLOBALROOT, PATH_GLOBALROOT_SIZE) )
      // This is \\?\GLOBALROOT
      GlobalRoot = true; 
    
    if (!GlobalRoot || !VoumeGuid) 
      // This is \\?\e:\ 
      SwitchPos = PATH_PARSE_SWITCHOFF_SIZE;
  }

  // We have to ask here for extra VolumeGuid too, because XP thinks \\?\ is a UNC Path 
  // and Windows7 knows that is not a UNC Path ;-/
  if (!PathIsUNC (&aPath[SwitchPos]) || VoumeGuid || GlobalRoot)
  {
    // This is the easy one: A normal path like c:\path\foo.txt
    _wsplitpath(&aPath[SwitchPos], Drive, NULL, NULL, NULL);

    // Splitpath will not resolve on \\?\Volume{ or \\?\GLOBALROOT\ 
    if (Drive[0])
    {
      // x:\ 
      DriveType = GetDriveType(Drive);
      wcscat_s (Drive, MAX_PATH, L"\\");
    }
    else
    {
      // \\?\Volume{GUID}\Path
      // \\?\GLOBALROOT\Path
      
      if (VoumeGuid)
        // GetDriveType() will only work for \\?\Volume{GUID}\, but not for \\?\Volume{GUID}\Path
        // so we have to copy the portion of the path which contains no path. This is done by assuming
        // that VOLUME_GUID_LENGTH contains the exact number of characters of a volume GUID path
        wcsncpy_s(Drive, HUGE_PATH, aPath, VOLUME_GUID_LENGTH);

      if (GlobalRoot)
      {
        // GetDriveType() will only work for \\?\GlOBALROOT\Device\HarddiskVolumeShadowCopy8, but not 
        // for \\?\GlOBALROOT\Device\HarddiskVolumeShadowCopy8\path\foo\dir. So we have to copy the 
        // portion of the path which contains no path. 
        wcscpy_s(Drive, HUGE_PATH, aPath);
        FindGlobalRootFromPath(Drive);
      }

      DriveType = GetDriveType(Drive);
    }
  }
  else
  {
    // UNC Path like \\server\share\path
    if (aFlags & eEnableRemote)
    {
      // There is no point in getting the Share info if remote capabilities are disabled  
      DriveType = DRIVE_REMOTE;
    }
    else
    {
      wchar_t	Share[HUGE_PATH];
      bool b = GetShareNameFromUNC(&aPath[SwitchPos], Share);
      if (b)
      {
        DriveType = DRIVE_REMOTE;
        wcscpy_s(Drive, MAX_PATH, Share);
      }
      else
      {
        *aDriveType = DRIVE_UNKNOWN;
        return false;    
      }
    }
  }

  switch (DriveType)
  {
    // This is a workaround, since getVolumeInformation takes
    // ages to complete for removeable drives, when the media
    // is not inserted.
    // Assume that removeable drives not beeing a: or b: always
    // have a media in, e.g they are mem-sticks ...
    case DRIVE_REMOVABLE:
      if (!(!_wcsicmp(Drive, L"a:\\") || !_wcsicmp(Drive, L"b:\\")) )
      {
        BOOL bResult = GetVolumeInformation(
          Drive, 
          NULL, 0,
          NULL,
          NULL,
          dwFileSystemFlags,
          FileSystemName,
          MAX_PATH
          );
      }
    break;

    case DRIVE_REMOTE:
      // Disable remote capabilities if necessary
      if (aFlags)
        break;

    // Volume Snapshots disguise as RAM_DISK
    case DRIVE_RAMDISK:
    case DRIVE_FIXED:
    {
      GetVolumeInformation(
        Drive, 
        NULL, 0,
        NULL,
        NULL,
        dwFileSystemFlags,
        FileSystemName,
        MAX_PATH
      );
    }
    break;

    default:
      break;
  }


  *aDriveType = DriveType;

  return gLSESettings.IsSupportedFileSystem(FileSystemName);
}

int 
ReparseCanonicalize (
  __in    LPCWSTR		      aPath,
  __inout LPWSTR		      aDestination,
  __in    const size_t    aDestSize
)
{
  if (!aPath)
    return ERROR_PATH_NOT_FOUND;

  wchar_t	TempPath[HUGE_PATH];
  wcscpy_s(TempPath, HUGE_PATH, aPath);
  size_t len = 0;
  size_t OldLen = 0;
  int done = 0;

  while(1) {
    while (TempPath[len] != '\\' && TempPath[len] != 0x00) 
      len++;

    int save = TempPath[len];
    TempPath[len] = 0x00;
    int r = ProbeReparsePoint(TempPath, aDestination);
    TempPath[len] = save;
    if (REPARSE_POINT_FAIL != r)
    {
      if (REPARSE_POINT_MOUNTPOINT == r)
        TranslateMountPoint(aDestination, aDestSize, aDestination);
      
      if (PathIsRelative(aDestination))
      {
        wchar_t	RemainingPath[HUGE_PATH];
        wcscpy_s(RemainingPath, HUGE_PATH, &TempPath[len]);
        TempPath[OldLen] = 0x00;
        wcscat_s(TempPath, HUGE_PATH, aDestination);
        len = wcslen(TempPath);
        wcscat_s(TempPath, HUGE_PATH, RemainingPath);
      }
      else
      {
        // Assume the result is absolute, and take the abolute path
        size_t olen = wcslen(aDestination);
        wcscat_s(aDestination, aDestSize, &TempPath[len]);
        len = olen;
        wcscpy_s(TempPath, HUGE_PATH, aDestination);
      }
      done = 1;
    }
    if (TempPath[len] == 0x00)
      break;

    len++;
    OldLen = len;
  }

  // If the path didn't contain a junction at all
  // copy the source to the destination
  if (!done)
    wcscpy_s(aDestination, aDestSize, aPath);
  else
    wcscpy_s(aDestination, aDestSize, TempPath);

  return ERROR_SUCCESS;
}



//--------------------------------------------------------------------
//
// CreateTimeStampedFileName
//
// generates a Filename with the pattern of 
// 
//
//--------------------------------------------------------------------
void
CreateTimeStampedFileName( 
  LPCWSTR       aDestPath,
  PWCHAR        aBackup0Path,
  PWCHAR        aBackup1Path,
  LPCWSTR       aFilename,
  _SYSTEMTIME*  const a_pSysTime
)
{
  wchar_t FileSearchPattern[HUGE_PATH];
  swprintf_s(FileSearchPattern, HUGE_PATH, L"%s\\%s - \?\?\?\?-\?\?-\?\? \?\?-\?\?-\?\?", aDestPath, aFilename);

  _StringList		FilenameList;

  // Read the names of all already existing backup sets
  WIN32_FIND_DATA	wfind;
  HANDLE	sh = FindFirstFile (FileSearchPattern, &wfind);
  if (INVALID_HANDLE_VALUE != sh)
  {
    do
      FilenameList.push_back(wfind.cFileName);
    while (FindNextFile (sh, &wfind) > 0);
    FindClose (sh);
  }

  if (FilenameList.size())
  {
    sort(FilenameList.begin(), FilenameList.end(), SringSorter());
    wcscpy(aBackup0Path, aDestPath);
    PathAddBackslash(aBackup0Path);
    wcscat(aBackup0Path, FilenameList.back().c_str());
  }
  else
    aBackup0Path[0] = 0x00;
  
  wsprintf(aBackup1Path, L"%s\\%s - %04d-%02d-%02d %02d-%02d-%02d", 
    aDestPath,
    aFilename,
    a_pSysTime->wYear,
    a_pSysTime->wMonth,
    a_pSysTime->wDay,
    a_pSysTime->wHour,
    a_pSysTime->wMinute,
    a_pSysTime->wSecond
  );
}
//--------------------------------------------------------------------
//
// CreateFileName
//
// CreateFileName generates a Filename with the pattern of 
// Copy (x) of <filename>
//
//--------------------------------------------------------------------
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
)
{
  // Check if the file exists, trying to get its attributes
  wcscpy_s(aNewFilename, HUGE_PATH, aPath);
  wcscat_s(aNewFilename, HUGE_PATH, aFilename);
  DWORD FileAttribute = GetFileAttributes(aNewFilename);
  if (INVALID_FILE_ATTRIBUTES == FileAttribute)
    // ok the file didn't exist, so we got it. This is the easiest way
    return 1;

  // Windows7/8/10 compliant name generation
  wcscpy_s(aNewFilename, HUGE_PATH, aFilename);
  wchar_t* pExtension = PathFindExtension(aNewFilename);
  if (*pExtension)
  {
    // There was an extension so cut it off
    *pExtension = 0x00;
    pExtension++;
  }

    
  wchar_t* pMsg;
  // W7 behaves differently for directories and for files
  if (FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
  {
    if (*pExtension)
      // Now check for '<aPath>\<aFilename>.<extension> - <aPrefix>'
      // %1!s!%2!s!.%4!s! - %3!s!
	    pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 6, 
        aPath,
        aNewFilename,
        pExtension,
        aPrefix
      );
    else
      // Now check for '<aPath>\<aFilename> - <aPrefix>'
      // %1!s!%2!s! - %3!s!
	    pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 7, 
        aPath,
        aNewFilename,
        aPrefix
      );
  }
  else
  {
    // Now check for '<aPath>\<aFilename> - <aPrefix>.<extension>'
    // %1!s!%2!s! - %3!s!.%4!s!
	  pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 3, 
      aPath,
      aNewFilename,
      aPrefix,
      pExtension
    );
    
    if (!*pExtension)
      PathRemoveExtension(pMsg);
  }

  if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(pMsg))
  {
    // Found a free filename
    wcscpy_s(aNewFilename, HUGE_PATH, pMsg);
    LocalFree(pMsg);
    return 2;
  }

  // Go on with wildcard search
  if (FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
  {
    if (*pExtension)
      // Now check for '<aPath>\<aFilename>.<extension> - <aPrefix> (n)'
      // %1!s!%2!s!.%4!s! - %3!s! (*)
	    pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 8, 
        aPath,
        aNewFilename,
        aPrefix,
        pExtension
      );
    else
      // Now check for '<aPath>\<aFilename> - <aPrefix> (n)'
      // %1!s!%2!s! - %3!s! (*)
	    pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 9, 
        aPath,
        aNewFilename,
        aPrefix
      );
      
  }
  else
  {
    // Now check for '<aPath>\<aFilename> - <aPrefix> (n).<extension>'
    // %1!s!%2!s! - %3!s! (*).%4!s!
    pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 4, 
      aPath,
      aNewFilename,
      aPrefix,
      pExtension
    );

    if (!*pExtension)
      PathRemoveExtension(pMsg);
  }

  WIN32_FIND_DATA	wfind;
  HANDLE	sh = FindFirstFile (pMsg, &wfind);

  // There is the need to start searching from an offset, otherwise the wrong opening bracket will be found.
  // The files below would cause such a problem
  //
  // bw05image1 - Hardlink (2) - Hardlink (2).jpeg
  // bw05image1 - Hardlink (2) - Hardlink.jpeg
  // bw05image1 - Hardlink (2).jpeg
  size_t SearchOffset = wcslen(aNewFilename);

  // Search for the highest available number
  int MaxOf = 2;
  if (INVALID_HANDLE_VALUE != sh)
  {
    _Of		of;

    do 
    {
      PWCHAR closepos = NULL;
      PWCHAR openpos = wcsstr(&wfind.cFileName[SearchOffset], L" (");
      if (openpos)
        closepos = wcsrchr(openpos, L')');

      if (openpos && closepos)
      {
        openpos += sizeof(wchar_t);
        *closepos = 0x00;

        // Check if all parts of the number are digits. This can happen if there are files like
        // foo - Hardlink (2).x
        // foo - Hardlink (2) - Hardlink(2).x
        // in a directory. This would push the number 2 two times on the _Of list, and later crash the sort
        size_t idx;
        size_t len = wcslen(openpos);
        for (idx = 0; idx < len; ++idx)
          if (openpos[idx] < '0' || openpos[idx] > '9')
            break;
        if (len == idx)
        {
          int num = _wtoi(openpos);
          if (num > 1)
            of.push_back(num);
        }
      }
    }
    while (FindNextFile (sh, &wfind) > 0);

    sort(of.begin(), of.end(), SizeSorter());
    MaxOf = FindFreeNumber(of);
  }
  FindClose (sh);
  LocalFree(pMsg);

  if (FileAttribute & FILE_ATTRIBUTE_DIRECTORY)
  {
    if (*pExtension)
      // generate the new name
      // %1!s!%2!s!.%5!s! - %3!s! (%4!d!)
	    pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 10,
        aPath,
        aNewFilename,
        aPrefix,
        MaxOf, 
        pExtension
      );
    else
      // generate the new name
      // %1!s!%2!s! - %3!s! (%4!d!)
	    pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 11,
        aPath,
        aNewFilename,
        aPrefix,
        MaxOf
      );
  }
  else
  {
    // generate the new name
    // %1!s!%2!s! - %3!s! (%4!d!).%5!s!
	  pMsg = GetFormattedMlgMessage(aLSEInstance, aLanguageID, aOrderIdx + 5, 
      aPath,
      aNewFilename,
      aPrefix,
      MaxOf, 
      pExtension
    );

    if (!*pExtension)
      PathRemoveExtension(pMsg);
  }

  wcscpy_s(aNewFilename, HUGE_PATH, pMsg);
  LocalFree(pMsg);

  return 3;
}

#if defined _DEBUG

int LoadOfArray(
                BYTE*	aData,
                int		aLen
                )
{
  _Of	of;
  for (int i = 0; i < aLen; ++i)
    of.push_back(aData[i]);

  return FindFreeNumber(of);
}

void
Test_FindFreeNumber(
                    )
{
  int rrr;

  BYTE d0[] = { 2, 3, 4 };
  rrr = LoadOfArray ((BYTE*)&d0, 3); // rrr = 5

  BYTE d1[] = { 2, 6, 7 };
  rrr = LoadOfArray ((BYTE*)&d1, 3); // rrr = 3

  BYTE d2[] = { 2, 6, 7,8 };
  rrr = LoadOfArray ((BYTE*)&d2, 4); // rrr = 3

  BYTE d5[] = { 8 };
  rrr = LoadOfArray ((BYTE*)&d5, 1); // rrr = 2

  BYTE d6[] = { 5, 6, 7 };
  rrr = LoadOfArray ((BYTE*)&d6, 3); // rrr = 2

  BYTE d7[] = { 2, 3, 5 };
  rrr = LoadOfArray ((BYTE*)&d7, 3); // rrr = 4

  BYTE d8[] = { 2, 3 };
  rrr = LoadOfArray ((BYTE*)&d8, 2); // rrr = 4

  BYTE d9[] = { 2, 6, 7, 8 };
  rrr = LoadOfArray ((BYTE*)&d9, 4); // rrr = 3

  BYTE da[] = { 2 };
  rrr = LoadOfArray ((BYTE*)&da, 1); // rrr = 3

  BYTE db[] = { 2, 3, 4, 8 };
  rrr = LoadOfArray ((BYTE*)&db, 4); // rrr = 5

  BYTE dc[] = { 6, 7, 8, 9 };
  rrr = LoadOfArray ((BYTE*)&dc, 4); // rrr = 2

}

#endif

// This procedure expects a vector with increasing numbers staring with a minimum value of 2
int
FindFreeNumber(
  _Of&		of
)
{
  if (of.empty())
    return 2;

  int rVal = 2;

  size_t size = of.size() ;
  if (size > 1)
  {
    int i = 1;
    do
    {
      // Check if the array is proportional to index
      if (rVal == of[i-1])
      {
        // Check if it is monotonically increasing
        if (of[i-1]+1 != of[i])
        {
          // not monotonic, return the next free value
          break;
        }
      }
      else
      {
        // array is not proportional to index
        rVal--;
        break;
      }

      // Increase all counters
      i++;
      rVal++;

    } while (i < size);
  }
  else
  {
    // rVal is 2 here
    if (rVal != of[0])
      // This path is for (n != 2)
      return 2;
    // This path is for (2). We will return 3. See below
  }

  return ++rVal;
}

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
)
{
  // GetVolumePathNamesForVolumeName
  WCHAR	VolumeName[MAX_PATH];
  BOOL r = GetVolumeNameForVolumeMountPoint(aSourceDirectory, VolumeName, MAX_PATH);
  if (r)
  {
    if ('\\' != aDestinationDirectory[wcslen(aDestinationDirectory) - 1])
    {
      WCHAR	Dst[MAX_PATH];
      wcscpy_s(Dst, MAX_PATH, aDestinationDirectory);
      wcscat_s(Dst, MAX_PATH, L"\\");
      r = SetVolumeMountPoint(Dst, VolumeName);
      if (!r)
        return GetLastError();
    }
    else
    {
      r = SetVolumeMountPoint(aDestinationDirectory, VolumeName);
      if (!r)
        return GetLastError();
    }
  }
  else
    return GetLastError();

  return ERROR_SUCCESS;
}

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
)
{
  if (aVolumeName)
  {
    DWORD  Error;
    PWCHAR Names;

    DWORD  Chars = MAX_PATH + 1;
    for (;;)
    {
      Error = S_FALSE;

      // Allocate a buffer to hold the path names
      Names = (PWCHAR) new BYTE[Chars * sizeof(WCHAR)];

      // Obtain all of the path names for this volume
      BOOL Status = GetVolumePathNamesForVolumeName(aVolumeName, Names, Chars, &Chars);
      if (Status)
      {
        Error = ERROR_SUCCESS;
        break;
      }

      Error = GetLastError();
      if (Error != ERROR_MORE_DATA)
        break;

      // Try again with the new suggested size
      delete[] Names;
    }

    if (ERROR_SUCCESS == Error)
      wcscpy_s(aDestination, aDestSize, Names);
    else
      wcscpy_s(aDestination, aDestSize, aVolumeName);

    delete[] Names;
  }

  return TRUE;
}

//--------------------------------------------------------------------
//
// ProbeMountPoint
//
// This routine checks if a given location is a NTFS mount point, 
//
//--------------------------------------------------------------------
BOOL
ProbeMountPoint (
  __in    LPCWSTR       aDirectory,
  __inout LPWSTR        aDestination,
  __in    const size_t  aDestSize,
  __in    LPWSTR        aVolumeName
)
{
  int r = ProbeReparsePoint(aDirectory, aVolumeName);
  if (REPARSE_POINT_MOUNTPOINT == r)
    return TranslateMountPoint(aDestination, aDestSize, aVolumeName);
  else
    return FALSE;
}

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
)
{
  int RetVal = ERROR_SUCCESS;
  BOOL b;

  if ('\\' != aDirectory[wcslen(aDirectory) - 1])
  {
    WCHAR	Dst[MAX_PATH];
    wcscpy_s(Dst, MAX_PATH, aDirectory);
    wcscat_s(Dst, MAX_PATH, L"\\");
    b = DeleteVolumeMountPoint(Dst);
  }
  else
    b = DeleteVolumeMountPoint(aDirectory);

  if (!b)
    RetVal = GetLastError();

  return RetVal;
}

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
)
{
    int r = ProbeReparsePoint(aFileName, aDestination);
    return REPARSE_POINT_SYMBOLICLINK == r ? TRUE:FALSE;
}


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
)
{
  if (PathIsUNC(aPath1))
  {
    // We have to go in detail
    if (PathIsUNC(aPath2))
    {
      // \\server\share\path1\file == \\server\share\path2\file 
      wchar_t	share1[HUGE_PATH];
      wchar_t	share2[HUGE_PATH];

      GetShareNameFromUNC(aPath1, share1); 
      GetShareNameFromUNC(aPath2, share2);

      if (!_wcsicmp(share1, share2))
        return true;
      else
        return false;
    }
    else
    {
      // \\server\share\path1 == x:\path2\file
      wchar_t	share1[HUGE_PATH];

      DWORD cbBuff = HUGE_PATH;
      WCHAR szBuff[HUGE_PATH];
      REMOTE_NAME_INFO  * prni = (REMOTE_NAME_INFO *)   &szBuff;

      DWORD r = WNetGetUniversalName(aPath2,
        REMOTE_NAME_INFO_LEVEL,
        prni,
        &cbBuff);

      if (NO_ERROR == r)
      {
        PathAddBackslash(prni->lpConnectionName);
        GetShareNameFromUNC(aPath1, share1);

        if (!_wcsicmp(share1, prni->lpConnectionName))
          return true;
        else
          return false;
      }
      else
        return false;
    }
  }
  else
  {
    if (PathIsUNC(aPath2))
    {
      // x:\path1\file == \\server\share\path2
      wchar_t	share2[HUGE_PATH];

      DWORD cbBuff = HUGE_PATH;
      WCHAR szBuff[HUGE_PATH];
      REMOTE_NAME_INFO  * prni = (REMOTE_NAME_INFO *)   &szBuff;

      DWORD r = WNetGetUniversalName(aPath1,
        REMOTE_NAME_INFO_LEVEL,
        prni,
        &cbBuff);

      if (NO_ERROR == r)
      {
        GetShareNameFromUNC(aPath2, share2);
        PathAddBackslash(prni->lpConnectionName);

        if (!_wcsicmp(prni->lpConnectionName, share2))
          return true;
        else
          return false;
      }
      else
        return false;
    }
    else
    {
      wchar_t DevicePath1[HUGE_PATH + PATH_NAMESPACE_ROOT_SIZE];
      wchar_t DevicePath2[HUGE_PATH + PATH_NAMESPACE_ROOT_SIZE + 1];
      wchar_t aFullPath1[HUGE_PATH];
      wchar_t aFullPath2[HUGE_PATH];

      wchar_t*	FileName;

      // Expand the path to a full, path because it could also be a relative path
      GetFullPathName(aPath1, HUGE_PATH, &DevicePath1[PATH_NAMESPACE_ROOT_SIZE], &FileName);
      GetFullPathName(aPath2, HUGE_PATH, &DevicePath2[PATH_NAMESPACE_ROOT_SIZE], &FileName);

      // A drive letter could also have been created via subst. So we need to
      // drill down a possible subst chain, until we end up with e.g \Device\HardDisk\Volume4
      do
      {
        wcscpy_s(aFullPath1, HUGE_PATH, &DevicePath1[PATH_NAMESPACE_ROOT_SIZE]);
        aFullPath1[2] = 0x00;
        QueryDosDevice(aFullPath1, DevicePath1, HUGE_PATH);
      }
      // a subst on a subst can be recognized by \??\ as prefix to e.g.
      // \??\f:\tmp
      while(DevicePath1[1] == '?');
      
      do
      {
        wcscpy_s(aFullPath2, HUGE_PATH, &DevicePath2[PATH_NAMESPACE_ROOT_SIZE]);
        aFullPath2[2] = 0x00;
        QueryDosDevice(aFullPath2, DevicePath2, HUGE_PATH);
      }
      while(DevicePath2[1] == '?');

      if (!wcscmp(DevicePath1, DevicePath2))
        return true;
      else
        return false;
    }
  }

}


//--------------------------------------------------------------------
// 
// CopyStatistics
// 
//--------------------------------------------------------------------

CopyStatistics::
CopyStatistics():
  m_DirectoryTotal{ 0 },
  m_DirectoryCreated{ 0 },
  m_DirectoryCreateSkipped{ 0 },
  m_DirectoryCreateFailed{ 0 },
  m_DirectoriesCleaned{ 0 },
  m_DirectoriesCleanedFailed{ 0 },
  m_DirectoriesExcluded{ 0 },

  m_FilesTotal{ 0 },
  m_FilesCopied{ 0 },
  m_FilesCopySkipped{ 0 },
  m_FilesCopyFailed{ 0 },
  m_FilesLinked{ 0 },
  m_FilesLinkSkipped{ 0 },
  m_FilesLinkFailed{ 0 },
  m_FilesCleaned{ 0 },
  m_FilesCleanedFailed{ 0 },
  m_FilesExcluded{ 0 },
  m_FilesCropped{ 0 },
  m_FilesSelected{ 0 },

  m_BytesTotal{ 0 },
  m_BytesCopied{ 0 },
  m_BytesCopySkippped{ 0 },
  m_BytesCopyFailed{ 0 },
  m_BytesLinked{ 0 },
  m_BytesLinkFailed{ 0 },
  m_BytesLinkSkipped{ 0 },
  m_BytesCleaned{ 0 },
  m_BytesExcluded{ 0 },


  m_SymlinksTotal{ 0 },
  m_SymlinksRestored{ 0 },
  m_SymlinksRestoreSkipped{ 0 },
  m_SymlinksRestoreFailed{ 0 },
  m_SymlinksCleaned{ 0 },
  m_SymlinksCleanedFailed{ 0 },
  m_SymlinksExcluded{ 0 },
  m_SymlinksCropped{ 0 },
  m_SymlinksDangling{ 0 },
  m_SymlinksSelected{ 0 },

  m_JunctionsTotal{ 0 },
  m_JunctionsRestored{ 0 },
  m_JunctionsRestoreSkipped{ 0 },
  m_JunctionsRestoreFailed{ 0 },
  m_JunctionsCleaned{ 0 },
  m_JunctionsCleanedFailed{ 0 },
  m_JunctionsExcluded{ 0 },
  m_JunctionsCropped{ 0 },
  m_JunctionsDangling{ 0 },

  m_MountpointsTotal{ 0 },
  m_MountpointsRestored{ 0 },
  m_MountpointsRestoreSkipped{ 0 },
  m_MountpointsRestoreFailed{ 0 },
  m_MountpointsCleaned{ 0 },
  m_MountpointsCleanedFailed{ 0 },
  m_MountpointsExcluded{ 0 },
  m_MountpointsCropped{ 0 },
  m_MountpointsDangling{ 0 },

  m_ReparsePointTotal{ 0 },
  m_ReparsePointRestored{ 0 },
  m_ReparsePointRestoreSkipped{ 0 },
  m_ReparsePointRestoreFailed{ 0 },
  m_ReparsePointCleaned{ 0 },
  m_ReparsePointCleanedFailed{ 0 },
  m_ReparsePointExcluded{ 0 },
  m_ReparsePointCropped{ 0 },
  m_ReparsePointDangling{ 0 },

  m_HardlinksTotal{ 0 },
  m_HardlinksTotalBytes{ 0 },

  m_DupeGroupsTotal{ 0 },
  m_DupeGroupsNew{ 0 },
  m_DupeGroupsOld{ 0 },
  m_DupeFilesHardlinked{ 0 },
  m_DupeFilesHardlinkFailed{ 0 },
  m_DupeTotalBytesToHash{ 0 },
  m_DupeCurrentBytesHashed{ 0 },
  m_DupeBytesSaved{ 0 },
  m_State{ eVoid },
  m_StartTime{ 0 },
  m_CopyTime{ 0 },
  m_EndTime{ 0 }
{
}

CopyStatistics::~CopyStatistics()
{
}

int
CopyStatistics::
AddTlsStat(CopyStatistics* rCopyStatistics)
{
  {
    scoped_lock  lock{m_StatGuard};
  
//  rCopyStatistics->m_DupeGroupsTotal += m_DupeGroupsTotal;
//  rCopyStatistics->m_DupeGroupsNew += m_DupeGroupsNew;
//  rCopyStatistics->m_DupeGroupsOld += m_DupeGroupsOld;
//  rCopyStatistics->m_DupeFilesHardlinked += m_DupeFilesHardlinked;   
//  rCopyStatistics->m_DupeFilesHardlinkFailed += m_DupeGroupsTotal;
//  rCopyStatistics->m_DupeTotalBytesToHash += m_DupeTotalBytesToHash;
  m_DupeCurrentBytesHashed += rCopyStatistics->m_DupeCurrentBytesHashed;
//  rCopyStatistics->m_DupeBytesSaved += m_DupeBytesSaved;

  }

  return ERROR_SUCCESS;
}

int
CopyStatistics::
Update(CopyStatistics& rCopyStatistics)
{
  rCopyStatistics.m_FilesTotal = m_FilesTotal;

  rCopyStatistics.m_FilesSelected = m_FilesSelected;
	rCopyStatistics.m_DupeFilesHardlinked = m_DupeFilesHardlinked;
	rCopyStatistics.m_DupeFilesHardlinked = m_DupeFilesHardlinkFailed;
	
	rCopyStatistics.m_DupeTotalBytesToHash = m_DupeTotalBytesToHash;
	rCopyStatistics.m_DupeBytesSaved = m_DupeBytesSaved;

	rCopyStatistics.m_DupeGroupsTotal = m_DupeGroupsTotal;
	rCopyStatistics.m_DupeGroupsNew = m_DupeGroupsNew;
	rCopyStatistics.m_DupeGroupsOld = m_DupeGroupsOld;

	rCopyStatistics.m_DupeCurrentBytesHashed = m_DupeCurrentBytesHashed;

  rCopyStatistics.m_StartTime = m_StartTime;
	rCopyStatistics.m_EndTime = m_EndTime;

	rCopyStatistics.m_State = m_State;

	return errOk;
}

int
CopyStatistics::
Proceed(int aNewState)
{
	StatisticsEvent	se;
	se.m_state = aNewState;
	GetSystemTime(&se.m_time);

  {
    scoped_lock  lock{m_EventGuard};
	m_StatisticsEvents.push_back (se);
  }

	m_State = aNewState;
	return errOk;
}

int
CopyStatistics::
GetDupeEvent(StatisticsEvent& aStatisticsEvent)
{
  {
    scoped_lock  lock{m_EventGuard};
	if (m_StatisticsEvents.begin () != m_StatisticsEvents.end ())
	{
		StatisticsEvent& rStatisticsEvents = m_StatisticsEvents.front ();
		aStatisticsEvent.m_state = rStatisticsEvents.m_state;
		aStatisticsEvent.m_time = rStatisticsEvents.m_time;

		m_StatisticsEvents.pop_front ();
	}
	else
	{
		aStatisticsEvent.m_state = m_State;
		GetSystemTime(&aStatisticsEvent.m_time);
	}
  }

	return errOk;
}


CopyStatistics&
CopyStatistics::
operator+=(const CopyStatistics& stats)
{
  m_DirectoryTotal += stats.m_DirectoryTotal;
  m_DirectoryCreated += stats.m_DirectoryCreated; 
  m_DirectoryCreateSkipped += stats.m_DirectoryCreateSkipped; 
  m_DirectoryCreateFailed += stats.m_DirectoryCreateFailed; 
  m_DirectoriesCleaned += stats.m_DirectoriesCleaned; 
  m_DirectoriesCleanedFailed += stats.m_DirectoriesCleanedFailed; 
  m_DirectoriesExcluded += stats.m_DirectoriesExcluded; 
  m_FilesTotal += stats.m_FilesTotal; 
  m_FilesCopied += stats.m_FilesCopied; 
  m_FilesCopySkipped += stats.m_FilesCopySkipped; 
  m_FilesCopyFailed += stats.m_FilesCopyFailed; 
  m_FilesLinked += stats.m_FilesLinked; 
  m_FilesLinkSkipped += stats.m_FilesLinkSkipped; 
  m_FilesLinkFailed += stats.m_FilesLinkFailed; 
  m_FilesCleaned += stats.m_FilesCleaned; 
  m_FilesCleanedFailed += stats.m_FilesCleanedFailed; 
  m_FilesExcluded += stats.m_FilesExcluded; 
  m_FilesCropped += stats.m_FilesCropped; 
  m_FilesSelected += stats.m_FilesSelected; 
  m_BytesTotal += stats.m_BytesTotal;
  m_BytesCopied += stats.m_BytesCopied;
  m_BytesCopySkippped += stats.m_BytesCopySkippped;
  m_BytesCopyFailed += stats.m_BytesCopyFailed;
  m_BytesLinked += stats.m_BytesLinked;
  m_BytesLinkFailed += stats.m_BytesLinkFailed;
  m_BytesLinkSkipped += stats.m_BytesLinkSkipped;
  m_BytesCleaned += stats.m_BytesCleaned;
  m_BytesExcluded += stats.m_BytesExcluded;
  m_SymlinksTotal += stats.m_SymlinksTotal; 
  m_SymlinksRestored += stats.m_SymlinksRestored; 
  m_SymlinksRestoreSkipped += stats.m_SymlinksRestoreSkipped; 
  m_SymlinksRestoreFailed += stats.m_SymlinksRestoreFailed; 
  m_SymlinksCleaned += stats.m_SymlinksCleaned; 
  m_SymlinksCleanedFailed += stats.m_SymlinksCleanedFailed; 
  m_SymlinksExcluded += stats.m_SymlinksExcluded; 
  m_SymlinksCropped += stats.m_SymlinksCropped; 
  m_SymlinksDangling += stats.m_SymlinksDangling; 
  m_SymlinksSelected += stats.m_SymlinksSelected; 
  m_JunctionsTotal += stats.m_JunctionsTotal; 
  m_JunctionsRestored += stats.m_JunctionsRestored; 
  m_JunctionsRestoreSkipped += stats.m_JunctionsRestoreSkipped; 
  m_JunctionsRestoreFailed += stats.m_JunctionsRestoreFailed; 
  m_JunctionsCleaned += stats.m_JunctionsCleaned; 
  m_JunctionsCleanedFailed += stats.m_JunctionsCleanedFailed; 
  m_JunctionsExcluded += stats.m_JunctionsExcluded; 
  m_JunctionsCropped += stats.m_JunctionsCropped; 
  m_JunctionsDangling += stats.m_JunctionsDangling; 
  m_MountpointsTotal += stats.m_MountpointsTotal; 
  m_MountpointsRestored += stats.m_MountpointsRestored; 
  m_MountpointsRestoreSkipped += stats.m_MountpointsRestoreSkipped; 
  m_MountpointsRestoreFailed += stats.m_MountpointsRestoreFailed; 
  m_MountpointsCleaned += stats.m_MountpointsCleaned; 
  m_MountpointsCleanedFailed += stats.m_MountpointsCleanedFailed; 
  m_MountpointsExcluded += stats.m_MountpointsExcluded; 
  m_MountpointsCropped += stats.m_MountpointsCropped; 
  m_MountpointsDangling += stats.m_MountpointsDangling; 
  m_ReparsePointTotal += stats.m_ReparsePointTotal; 
  m_ReparsePointRestored += stats.m_ReparsePointRestored; 
  m_ReparsePointRestoreSkipped += stats.m_ReparsePointRestoreSkipped; 
  m_ReparsePointRestoreFailed += stats.m_ReparsePointRestoreFailed; 
  m_ReparsePointCleaned += stats.m_ReparsePointCleaned; 
  m_ReparsePointCleanedFailed += stats.m_ReparsePointCleanedFailed; 
  m_ReparsePointExcluded += stats.m_ReparsePointExcluded; 
  m_ReparsePointCropped += stats.m_ReparsePointCropped; 
  m_ReparsePointDangling += stats.m_ReparsePointDangling; 
  m_ReparsePointSelected += stats.m_ReparsePointSelected; 
  m_HardlinksTotal += stats.m_HardlinksTotal; 
  m_HardlinksTotalBytes += stats.m_HardlinksTotalBytes; 
  m_DupeGroupsTotal += stats.m_DupeGroupsTotal;
  m_DupeGroupsNew += stats.m_DupeGroupsNew;
  m_DupeGroupsOld += stats.m_DupeGroupsOld;
  m_DupeFilesHardlinked += stats.m_DupeFilesHardlinked;   
  m_DupeFilesHardlinkFailed += stats.m_DupeFilesHardlinkFailed;   
  m_DupeTotalBytesToHash += stats.m_DupeTotalBytesToHash;
  m_DupeCurrentBytesHashed += stats.m_DupeCurrentBytesHashed;
  m_DupeBytesSaved += stats.m_DupeBytesSaved;

  return *this;
}




FileInfoContainer::
FileInfoContainer() :
  m_StackUsageFindHardlink{ 0 },
  m_LastStackUsageFindHardlink{ 0 },

  m_DateTimeTolerance{ 0 },
  m_HardlinkLimit{ cMaxHardlinkLimit },

  m_MaxRound{ 0 },

  m_pLookAsideFileInfoContainer{ nullptr },
  m_CurDestPathIdx{ -1 },
  m_CurSourcePathLen{ 0 },
  m_Prepared{ false },

  m_Flags{ eSmartCopy },
  m_Flags2{ 0 },


  m_CurrentSerialNumber{ 0 },
  m_OutputFile{ nullptr },

  m_MinSize{ 0 },
  m_MaxSize{ 0x7fffffffffffffff },

  m_pSecDesc{ nullptr },
  m_JsonWriterState{false},
  m_pJsonWriterState{ &m_JsonWriterState }
{
	GetSystemInfo (&m_SystemInfo);
	m_SystemAllocGranularity = m_SystemInfo.dwAllocationGranularity;
  
  // The maximum available mapping space, which is a quarter of the available adressspace
  m_MaxAdressSpace = (ULONG64)m_SystemInfo.lpMaximumApplicationAddress + (ULONG64)m_SystemInfo.lpMinimumApplicationAddress;
  m_MaxAdressSpace >>= 1;
  m_MaxAdressSpace++;

  // Will this work on any x86 any CPU compiled with VS2005SP1 ( e.g AMD, Core, Core2... )?
  // 
  // int nPages = 0x4000;
  // int Power2 = 0;
  // while (nPages >>= 1)
  // Power2++;
  // 
  // No. On some CPUs this will end in an endless loop!!! nPages becomes not 0. But
  // 
  // unsigned nPages = 0x4000;
  // int Power2 = 0;
  // while (nPages >>= 1)
  // Power2++;
  // 
  // works. Spot the difference? No? This time nPages is unsignend instead of int.
  // The message is *never shift signed data types*
  unsigned nPages = m_MaxAdressSpace / m_SystemAllocGranularity;
  int Power2 = 0;
  while (nPages >>= 1)
    Power2++;
  
  m_MaxRound = Power2 + eRoundOffset;

  m_SecDescSize = sizeof(SECURITY_DESCRIPTOR);
  m_pSecDesc = (PSECURITY_DESCRIPTOR)malloc(sizeof(SECURITY_DESCRIPTOR));
}

//--------------------------------------------------------------------
// 
// Hardlink Sibling enumeration the manual way. 
// 
//--------------------------------------------------------------------

int
FileInfoContainer::
EnumHardlinkSiblings(
  PWCHAR						        aSrcPath, 
  PWCHAR						        aExcludePath, 
  EnumHardlinkSiblingsGlue*	pEnumHardlinkSiblingsGlue,
  bool						          aQuiet,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext
)
{
  if (apContext)
  {
    EnumHardlinkSiblingsParams* pEnumHardlinkSiblingsParams = new EnumHardlinkSiblingsParams;
    pEnumHardlinkSiblingsParams->m_SrcPath = aSrcPath;
    pEnumHardlinkSiblingsParams->m_ExcludePath = aExcludePath;
    pEnumHardlinkSiblingsParams->m_pEnumHardlinkSiblingsGlue = pEnumHardlinkSiblingsGlue;
    pEnumHardlinkSiblingsParams->m_Quiet = aQuiet;
    pEnumHardlinkSiblingsParams->m_PathNameStatusList = aPathNameStatusList;
    pEnumHardlinkSiblingsParams->m_pContext = apContext;

    pEnumHardlinkSiblingsParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunEnumHardlinkSiblings), 
      pEnumHardlinkSiblingsParams,
      0, 
      &ThreadId);

    return 0;
  }
  else
    return _EnumHardlinkSiblings(aSrcPath, aExcludePath, pEnumHardlinkSiblingsGlue, aQuiet, aPathNameStatusList, apContext);
}

DWORD
__stdcall
FileInfoContainer::
RunEnumHardlinkSiblings(EnumHardlinkSiblingsParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_EnumHardlinkSiblings(p->m_SrcPath, p->m_ExcludePath, p->m_pEnumHardlinkSiblingsGlue, p->m_Quiet, p->m_PathNameStatusList, pCtx);
  delete p;

  return 1;
}

int
FileInfoContainer::
_EnumHardlinkSiblings(
  PWCHAR						          aSrcPath, 
  PWCHAR						          aExcludePath, 
  EnumHardlinkSiblingsGlue*	  pEnumHardlinkSiblingsGlue,
  bool						            aQuiet,
  _PathNameStatusList*        aPathNameStatusList,
  AsyncContext*               apContext
)
{
  WCHAR	SrcPath[HUGE_PATH];
  wcscpy_s(SrcPath, HUGE_PATH, aSrcPath);

  // CopyStatistics is just a fake here to satisfy AddFile()
  CopyStatistics	aStats;
  WCHAR FakeReparseTarget[1];
  FakeReparseTarget[0] = 0x00;
  int rrr = AddFile(SrcPath, FILE_ATTRIBUTE_NORMAL, 1, &aStats, FakeReparseTarget, NULL, REPARSE_POINT_FAIL);
  if (ERROR_SUCCESS == rrr)
  {
    PathRemoveFileSpec(&SrcPath[PATH_PARSE_SWITCHOFF_SIZE]);
    return _EnumHardlinkSiblingsUp(SrcPath, HUGE_PATH, aExcludePath, pEnumHardlinkSiblingsGlue, aQuiet, aPathNameStatusList, apContext);
  }
  else
    return GetLastError();
}

int
FileInfoContainer::
_EnumHardlinkSiblingsUp(
  PWCHAR						        aSrcPath, 
  const size_t              aSrcSize,
  PWCHAR						        aExcludePath,
  EnumHardlinkSiblingsGlue*	pEnumHardlinkSiblingsGlue,
  bool						          aQuiet,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext
  )
{
  int RetVal = _EnumHardlinkSiblingsDown(
    aSrcPath, 
    aSrcSize,
    aExcludePath, 
    pEnumHardlinkSiblingsGlue, 
    aQuiet,
    aPathNameStatusList,
    apContext
  );

  // Check if we have been canceled from outside
  if (apContext)
  {
    apContext->AddProgress(1, 1, 1);

    // Set the status, and check if Smartcopy has been cancelled from outside
    int r = apContext->PutStatus(&aSrcPath[PATH_PARSE_SWITCHOFF_SIZE]);
    if ( ERROR_REQUEST_ABORTED == r)
      RetVal = ERROR_REQUEST_ABORTED;
  }

  if (ERROR_SUCCESS == RetVal)
  {
    // Go up one level if possible
    size_t sSrcLen = wcslen(aSrcPath);
    WCHAR	NewSrcPath[HUGE_PATH];

    // One up aka 'cd ..'
    wcscpy_s(NewSrcPath, HUGE_PATH, aSrcPath);
    PathRemoveFileSpec (&NewSrcPath[PATH_PARSE_SWITCHOFF_SIZE]);

    aSrcPath[sSrcLen] = 0x00;

    // PathRemoveBackslash does not remove traling slashes
    // for root pathes, e.g. c:\ . So we need our own code
    size_t NewLen = wcslen(NewSrcPath);
    if (NewSrcPath[NewLen - 1] == '\\')
      NewSrcPath[NewLen - 1] = 0x00;

    if (wcscmp(NewSrcPath, aSrcPath))
    {
      //				wprintf(L"^ %s\n",NewSrcPath); 
      RetVal = _EnumHardlinkSiblingsUp(NewSrcPath, 
        HUGE_PATH,
        aSrcPath,
        pEnumHardlinkSiblingsGlue, 
        aQuiet,
        aPathNameStatusList,
        apContext);
    }
  }
  return RetVal;
}

int
FileInfoContainer::
_EnumHardlinkSiblingsDown(
  PWCHAR						        aSrcPath, 
  const size_t              aSrcSize,
  PWCHAR						        aExcludePath, 
  EnumHardlinkSiblingsGlue*	pEnumHardlinkSiblingsGlue,
  bool						          aQuiet,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext
)
{
  WIN32_FIND_DATA	wfind;
  int							RetVal = ERROR_SUCCESS;

  size_t			sSrcLen = wcslen(aSrcPath);

  // Search the files under a given path
  wcscat_s(aSrcPath, aSrcSize, L"\\*.*");
  HANDLE	sh = FindFirstFile (aSrcPath, &wfind);
  aSrcPath[sSrcLen] = 0x00;
  if (INVALID_HANDLE_VALUE != sh)
  {
    _Pathes::iterator head = m_Filenames.begin();
    FileInfo*	pFileInfo = *head;

    do
    {
      // Only follow Directories, but don't follow ReparsePoints
      if (!(wfind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
        // Check if it is a hardlink
        sSrcLen = wcslen(aSrcPath);
        if (aSrcPath[sSrcLen - 1] != '\\')
          wcscat_s(aSrcPath, aSrcSize, L"\\");
        wcscat_s(aSrcPath, aSrcSize, wfind.cFileName);

        if (apContext)
        {
          apContext->AddProgress(1, 1, 1);

          // Set the status, and check if EnumHardlinkSiblings has been cancelled from outside
          int r = apContext->PutStatus(&aSrcPath[PATH_PARSE_SWITCHOFF_SIZE]);
          if ( ERROR_REQUEST_ABORTED == r)
          {
            RetVal = ERROR_REQUEST_ABORTED;
            aSrcPath[sSrcLen] = 0x00;
            break;
          }
        }

        HANDLE	fh = CreateFileW (
          aSrcPath, 
          GENERIC_READ, 
          FILE_SHARE_READ,
          NULL, 
          OPEN_EXISTING, 
          FILE_FLAG_OPEN_REPARSE_POINT, 
          NULL);

        if (INVALID_HANDLE_VALUE != fh)
        {
          BY_HANDLE_FILE_INFORMATION	FileInformation;
          BOOL	r = GetFileInformationByHandle (fh, &FileInformation);
          if (r)
          {
            if (FileInformation.nNumberOfLinks > 1)
            {
              // Lookup file in in Container
              if (pFileInfo->m_FileIndex.ul32l == FileInformation.nFileIndexLow && pFileInfo->m_FileIndex.ul32h == FileInformation.nFileIndexHigh)
              {
                pEnumHardlinkSiblingsGlue->Print(aSrcPath);
                if (--pFileInfo->m_RefCount == 0)
                {
                  // end of search, found all siblings
                  RetVal = ERROR_END_OF_RECURSION;
                  CloseHandle (fh);
                  break;
                }
              }
            }
          }
          CloseHandle (fh);
        }

        aSrcPath[sSrcLen] = 0x00;
      }
    }
    while (FindNextFile (sh, &wfind) > 0);
    FindClose (sh);
  }

  if (ERROR_REQUEST_ABORTED == RetVal)
    return ERROR_REQUEST_ABORTED;

  // Search the directories below the current path
  wcscat_s(aSrcPath, aSrcSize, L"\\*.*");
  sh = FindFirstFile (aSrcPath, &wfind);
  aSrcPath[sSrcLen] = 0x00;
  if (INVALID_HANDLE_VALUE != sh && RetVal == ERROR_SUCCESS)
  {
    do
    {
      if (wfind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (wcscmp (wfind.cFileName, L".") )
          if ( wcscmp (wfind.cFileName, L"..") )
          {
            sSrcLen = wcslen(aSrcPath);
            wcscat_s(aSrcPath, aSrcSize, L"\\");
            wcscat_s(aSrcPath, aSrcSize, wfind.cFileName);

            // Check if we have been canceled from outside
            if (apContext)
            {
              apContext->AddProgress(1, 1, 1);

              // Set the status, and check if EnumHardlinkSiblings has been cancelled from outside
              int r = apContext->PutStatus(&aSrcPath[PATH_PARSE_SWITCHOFF_SIZE]);
              if ( ERROR_REQUEST_ABORTED == r)
              {
                RetVal = ERROR_REQUEST_ABORTED;
                aSrcPath[sSrcLen] = 0x00;
                break;
              }
            }

            if (wcscmp(aSrcPath, aExcludePath))
            {

              // By default we do not follow Junctions or Symbolic Links, or other reparse points
              if (wfind.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
              {
                aSrcPath[sSrcLen] = 0x00;
              }
              else
              {
                int	rr = _EnumHardlinkSiblingsDown (aSrcPath, aSrcSize, aExcludePath, pEnumHardlinkSiblingsGlue, aQuiet, aPathNameStatusList, apContext);
                aSrcPath[sSrcLen] = 0x00;
                if (rr != ERROR_SUCCESS)
                {
                  // if we encountered an error escape the recursion
                  RetVal = rr;
                  break;
                }
              }
            }
            else
            {
              aSrcPath[sSrcLen] = 0x00;
            }
          }
      }
    }
    while (FindNextFile (sh, &wfind) > 0);
    FindClose (sh);
  }

  return RetVal;
}

int
FileInfoContainer::
FindHardLink(
  _ArgvList&            aSrcPathList, 
  int		                aRefCount,
  CopyStatistics*	      aStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
  if (apContext)
  {
    FindHardLinkParams* pFindHardLinkParams  = new FindHardLinkParams;
    pFindHardLinkParams->m_SrcPathList = aSrcPathList;
    pFindHardLinkParams->m_RefCount = aRefCount;
    pFindHardLinkParams->m_Stats = aStats;
    pFindHardLinkParams->m_PathNameStatusList = aPathNameStatusList;
    pFindHardLinkParams->m_pContext = apContext;
    pFindHardLinkParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunFindHardLink), 
      pFindHardLinkParams,
      0, 
      &ThreadId);

    return 1;
  }
  else
    return _FindHardLink(aSrcPathList, aRefCount, aStats, aPathNameStatusList, apContext);
}

DWORD
__stdcall
FileInfoContainer::
RunFindHardLink(FindHardLinkParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_FindHardLink(p->m_SrcPathList, p->m_RefCount, p->m_Stats, p->m_PathNameStatusList, pCtx);
  delete p;

  return 1;
}


int
FileInfoContainer::
_FindHardLink(
  _ArgvList&              aSrcPathList, 
  int		                  aRefCount,
  CopyStatistics*	        aStats,
  _PathNameStatusList*    aPathNameStatusList,
  AsyncContext*           apContext
)
{
#if defined _M_IX86
  PVOID OldValue = NULL;
  Wow64DisableWow64FsRedirection(&OldValue);
#endif
  WCHAR	SrcPath[HUGE_PATH];

  // Add all source path to the m_AnchorPathCache, because during FindHardlink
  // all source path are needed so that for multiple source the inner and outer junctions
  // can be detected in Findhardlink
  m_AnchorPathCache.Add(aSrcPathList);
  m_AnchorPathCache.ResolveRemote();

  int RetVal = ERROR_SUCCESS;

  // No go through the source path and enumerate
  for (_ArgvListIterator iter = aSrcPathList.begin(); iter != aSrcPathList.end(); ++iter)
  {
    // Only iterate over directories but not files. Files are in the list, because if 
    // Findhardlink is spawned during Backup in LSEUacHelper.exe the arguments are transferred
    // to LSEUacHelper.exe via _ArgvList.
    if (iter->FileAttribute == FILE_ATTRIBUTE_NORMAL)
      continue;
    
    // Do not search below paths which are only anchors. Anchors are only used
    // during resolving of junction
    if (iter->Flags & _ArgvPath::Anchor)
      continue;
    
    // See if path is still UNC
    if (!PathIsUNC (iter->Argv.c_str()))
    {
      if ( !IsVeryLongPath(iter->Argv.c_str()) )
      {
        PWCHAR	FilePart;
        wcscpy_s(SrcPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
        GetFullPathName(iter->Argv.c_str(), HUGE_PATH - PATH_PARSE_SWITCHOFF_SIZE, &SrcPath[PATH_PARSE_SWITCHOFF_SIZE], &FilePart);
      }
      else
      {
        wcscpy_s(SrcPath, HUGE_PATH, iter->Argv.c_str());
      }
    }
    else
    {
      // The path might still be UNC if it was not a local UNC path, which could not be resolved
      wcscpy_s(SrcPath, HUGE_PATH, iter->Argv.c_str());
    }

    // For each path origin set the enumeration mode accordingly.
    if (iter->DriveType == DRIVE_REMOTE)
      SetFlags(eTraditional);
    
    // For each source find the Destination Path Index
#if !defined EXACT_PATH_INDEX
    m_CurDestPathIdx = m_AnchorPathCache.GetDestPathIndex(iter->ArgvDest.c_str());
#else
    m_CurDestPathIdx = m_AnchorPathCache.GetDestPathIndex(iter);
#endif
    size_t SourcePathLen = wcslen(SrcPath);

    // All Reparsepoints are either inner or outer reparsepoints with respect 
    // to a given path. The recursion starts with ReparsePointReferencePath
    // equal to SrcPath, but on the occurence of outer reparsepoints 
    // ReparsePointReferencePath is changed to the destination of a 
    // outer reparsepoint.
    WCHAR	CurrentReparsePointReferencePath[HUGE_PATH];
    PWCHAR pCurrentReparsePointReferencePath = CurrentReparsePointReferencePath;
    WCHAR	ReparsePointReferencePath[HUGE_PATH];
    wcscpy_s(CurrentReparsePointReferencePath, HUGE_PATH, SrcPath);

    // Get the File Attributes early, because it might be a root path, e.g t:\
    // which means the \ gets cut off resulting in t: which would not result
    // in a proper file attribute
    
    DWORD FileAttributes;
    wchar_t VolumeName[HUGE_PATH];

    // Make sure GetVolumeNameForVolumeMountPoint() is available
    //
    BOOL IsVolume = GetVolumeNameForVolumeMountPoint(SrcPath, VolumeName, HUGE_PATH);
    if (ERROR_SUCCESS != GetLastError())
    {
      // Another hack. Root-Path based on \\?\GLOBALROOT need a trailing slash so that 
      // GetVolumeNameForVolumeMountPoint() works.
      size_t Len = wcslen(SrcPath);
      wcscat_s(SrcPath, HUGE_PATH, L"\\");
      IsVolume = GetVolumeNameForVolumeMountPoint(SrcPath, VolumeName, HUGE_PATH);

      // But it does not work when \\?\GLOBALROOT is the prefix for path created by volumeshadows
      if ((!wcsncmp(SrcPath, PATH_LONG_GLOBALROOT, PATH_LONG_GLOBALROOT_SIZE)) && !IsVolume)
      {
        FileAttributes = GetFileAttributes(SrcPath);
        if (INVALID_FILE_ATTRIBUTES != FileAttributes)
          IsVolume = true;
      }
      else
      {
        // Still unable to get the fileAttributes from \\?\GLOBALROOT as prefix for path created by volumeshadows
      }

      SrcPath[Len] = 0x0;
    }

    //
    // Retrieving the attribute seems so easy, but when it comes to a root directory or mountpoint
    // we get 0x16, which is directory hidden and system but a nonsense, if you at the very end
    // restore exactly these attributes.
    //
    // We we need to find out if it is a volume mount point, and if yes fix the attribute to 
    // FILE_ATTRIBUTE_DIRECTORY
    //
    if (!IsVolume)
      FileAttributes = GetFileAttributes(SrcPath);
    else
      FileAttributes = FILE_ATTRIBUTE_DIRECTORY;

    DWORD aOriginalFileAttribute = FileAttributes;

    // Get the DiskSerialNumber
    int r = m_DiskSerialCache.Lookup(SrcPath, m_CurrentSerialNumber);


    // PathIsRoot does not work with XP with long path name (\\?\)
    // but works in Windows7
    if (PathIsRoot(&SrcPath[PATH_PARSE_SWITCHOFF_SIZE]))
    {
      SrcPath[SourcePathLen - 1] = 0x00;
      SourcePathLen--;
    }

    // If a pure Volume{GUID} was specified, then it must have a trailing slash like Volume{GUID}\ 
    // But in FindHardlinkRecursive we only append slashes if there is not already one to avoid haing 
    // two slashes in the path. So we need the length of Volume{GUID}\ smaler by 1, so that during
    // concatenation for the destination the \ is copied. A tricky (but not nice) spread feature.
    if (!wcsncmp(SrcPath, PATH_LONG_VOLUME_GUID, PATH_LONG_VOLUME_GUID_SIZE) && '\\' == SrcPath[SourcePathLen - 1]) 
      SourcePathLen--;

    SetCurSourcePathlen(SourcePathLen);



    // If CurrentBoundaryCross == 0 means we haven't crossed to a outer ReparsePoint
    // so all ReparsePoint reconstruct mechanisms go the standard way
    int CurrentBoundaryCross = 0; 
    int PreviousBoundaryCross;

    bool ReparsePointBoundaryCrossed;
    PWCHAR PreviousReparsePointReferencePath;
    wchar_t ReparseSrcTargetHint[HUGE_PATH];
    ReparseSrcTargetHint[0] = 0x00;


    // For the first level we follow a reparse point, lets say we do a 'undercover' unroll 
    // for the first layer. So we temporarily have to switch on unroll, and after this 
    // first level switch it back
    int SaveFlag = m_Flags;
    m_Flags |= eAlwaysUnrollDir;
    bool FatalReparseError = false;
    bool DoRecursion = IsOuterReparsePoint(
      SrcPath, 
      &pCurrentReparsePointReferencePath, 
      ReparsePointReferencePath, 
      &PreviousReparsePointReferencePath,
      FileAttributes, 
      ReparsePointBoundaryCrossed,
      CurrentBoundaryCross,
      PreviousBoundaryCross,
      ReparseSrcTargetHint,
      aPathNameStatusList,
      FatalReparseError,
      aStats
    );
    m_Flags = SaveFlag;

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
  wprintf(L"%s _FindHardlink::RefCount %d\n", m_ContainerName, aRefCount);
#endif

    
    int ResultAddDir = ERROR_SUCCESS;
    // Assume we start with a directory and add itself also, because
    // we need to restore the date of the very root too
    if (0 == aRefCount)
    {
      // If we start with a directory, we end up here with \\?\q:
      // This is necessary, that the recursion works, but when er
      // call AddDirectory \\?\q: is not legal. We temporarily need
      // \\?\q:\

      if (SrcPath[SourcePathLen - 1] == L':') 
        wcscat_s(SrcPath, HUGE_PATH, L"\\");
      else
      {
        if (!wcsncmp(SrcPath, PATH_LONG_GLOBALROOT, PATH_LONG_GLOBALROOT_SIZE) && SrcPath[SourcePathLen - 1] != L'\\')
          wcscat_s(SrcPath, HUGE_PATH, L"\\");
      }

      if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(SrcPath))
        ResultAddDir = ERROR_PATH_NOT_FOUND;
      else
        // We can't go the fast way with the first directory
        ResultAddDir = AddDirectory(
          SrcPath, 
          m_DateTimeRestore, 
          FileAttributes, 
          aStats, 
          ReparseSrcTargetHint, 
          aOriginalFileAttribute, 
          aPathNameStatusList
        );

      if (SrcPath[SourcePathLen - 1] == L':')
        SrcPath[SourcePathLen] = 0x00;
      else
      {
        if (!wcsncmp(SrcPath, PATH_LONG_GLOBALROOT, PATH_LONG_GLOBALROOT_SIZE) && SrcPath[SourcePathLen - 1] != L'\\')
          SrcPath[SourcePathLen] = 0x00;
      }
    }

    // Go through the rest of files below SrcPath
    if (ResultAddDir == ERROR_SUCCESS)
    {
      if (m_Flags & eTraditional)
      {
#if defined LSE_DEBUG
        Log(L"'Traditional enumeration %s\n", L" ", m_Flags, eLogVerbose);
#endif
        __int64 FileAddedOneLevelBelow = 0;
        RetVal = _FindHardLinkTraditionalRecursive(
          SrcPath, 
          HUGE_PATH,
          &pCurrentReparsePointReferencePath,
          CurrentBoundaryCross, 
          aRefCount, 
          FileAddedOneLevelBelow, 
          aStats, 
          aPathNameStatusList, 
          apContext
        );
      }
      else
      {
#if defined LSE_DEBUG
        Log(L"'fast enumeration %s\n", L" ", m_Flags, eLogVerbose);
#endif
        __int64 FileAddedOneLevelBelow = 0;
        RetVal = _FindHardLinkRecursive(
          SrcPath, 
          HUGE_PATH,
          &pCurrentReparsePointReferencePath, 
          CurrentBoundaryCross, 
          aRefCount, 
          FileAddedOneLevelBelow, 
          aStats, 
          aPathNameStatusList, 
          apContext
        );
      }
    }
    else
      RetVal = ResultAddDir;


    if (RetVal != ERROR_SUCCESS)
      break;
  }
#if defined _M_IX86
  Wow64RevertWow64FsRedirection(OldValue);
#endif
  return RetVal;
}
//
// Recursivley runs through a given aSrcPath, and record all path
// which have a Reference Count greater than aRefCount. 
// If aRefCount == 0 even directories are added to the container
// 
// Return Value: ERROR_SUCCESS  == 0
//               ERROR_xxxxxxxx  > 0
int
FileInfoContainer::
_FindHardLinkTraditionalRecursive(
  LPWSTR	                aSrcPath,
  const size_t            aSrcSize,
  PWCHAR*	                aCurrentReparsePointReferencePath,
  int&                    aCurrentBoundaryCross,
  int		                  aRefCount,
  __int64&                aFileAddedOneLevelBelow,
  CopyStatistics*	        aStats,
  _PathNameStatusList*    aPathNameStatusList,
  AsyncContext*           apContext
)
{
  //
  // Since the depth of the recursion is the depth of the source tree
  // we need to make sure this depth is not exceeded, otherwise we get cryptic
  // error messages from crashes and crash dumps, which do not give any clue
  //
  // Taken from
  // http://stackoverflow.com/questions/1740888/determining-stack-space-with-visual-studio
  //

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
    wprintf(L"%s " __FUNCTION__ L" 01 %s\n", m_ContainerName, aSrcPath);
#endif

  size_t sp_value;

#if defined _M_IX86
  _asm { mov [sp_value], esp }
#elif defined _M_X64
  CONTEXT context;
  RtlCaptureContext(&context);
  sp_value = context.Rsp;
#endif

  PNT_TIB lTeb = (PNT_TIB)NtCurrentTeb();
  size_t stackAvailable = (size_t)lTeb->StackBase - (size_t)lTeb->StackLimit;

  size_t used_stack_size = (size_t)lTeb->StackBase - sp_value;
  if (0 == m_StackUsageFindHardlink && m_LastStackUsageFindHardlink)
     m_StackUsageFindHardlink = used_stack_size - m_LastStackUsageFindHardlink;

  m_LastStackUsageFindHardlink = used_stack_size;

  // Make sure this number is the same as given under the linker section
  // via /stack:number . It seems that 45Mb is enough for a path with 128
  // levels aka 256 character for MAX_PATH
  if (used_stack_size + m_StackUsageFindHardlink > STACKSIZE)
    return ERROR_STACK_BUFFER_OVERRUN;


  //
  // ReparsePointReferencePath is kept for each level of recursion
  // and is referenced from deeper levels of recursions
  //
  PWCHAR PreviousReparsePointReferencePath;
  WCHAR	ReparsePointReferencePath[HUGE_PATH];
  int PreviousBoundaryCross;


  WIN32_FIND_DATA	wfind;
  int							RetVal = 0;

  size_t			sSrcLen = wcslen(aSrcPath);
  wcscat_s(aSrcPath, aSrcSize, L"\\*.*");
  HANDLE	sh = FindFirstFile (aSrcPath, &wfind);
  aSrcPath[sSrcLen] = 0x00;

  if (INVALID_HANDLE_VALUE != sh)
  {
    do
    {
      int PathParseSwitchOffSize = 0;

      DWORD OriginalFileAttribute = wfind.dwFileAttributes;
      if (wfind.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (wcscmp (wfind.cFileName, L"."))
        {
          if (wcscmp (wfind.cFileName, L".."))
          {
            sSrcLen = wcslen(aSrcPath);

            if ('\\' != aSrcPath[sSrcLen - 1])
              wcscat_s(aSrcPath, aSrcSize, L"\\");
            else
              sSrcLen--;

            wcscat_s(aSrcPath, aSrcSize, wfind.cFileName);
            if (IsVeryLongPath(aSrcPath))
              PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
            wprintf(L"%s " __FUNCTION__ L" 04 %s\n", m_ContainerName, aSrcPath);
#endif
            if (apContext)
            {
              apContext->AddProgress(eDirectoryWeight, 1, 1);

              // Set the status, and check if FindHardLink has been cancelled from outside
              int r = apContext->PutStatus(&aSrcPath[PathParseSwitchOffSize]);
              if ( ERROR_REQUEST_ABORTED == r)
              {
                RetVal = ERROR_REQUEST_ABORTED;
                break;
              }
            }

            // Check if there are reg exps for exclusion of directories
            bool RegExpMatch;

            if (!m_RegExpInclDirList.size())
              // An empty Incl List means 'take everything'
              RegExpMatch = true;
            else
              // But if there are items on the incl-list we would like just have them
              RegExpMatch = MatchRegExpList(&aSrcPath[PathParseSwitchOffSize], m_RegExpInclDirList);

            // But afterwards exclude
            RegExpMatch &= !MatchRegExpList(&aSrcPath[PathParseSwitchOffSize], m_RegExpExclDirList);

            if (RegExpMatch)
            {
              // By default we do not follow reparse points
              bool ReparsePointBoundaryCrossed = false;

              wchar_t ReparseSrcTargetHint[HUGE_PATH];
              ReparseSrcTargetHint[0] = 0x00;
              
              // IsOuterReparseTarget() might throw fatal error, which prevent further
              // adding of that directory to the pool
              bool  FatalReparseError = false;
              bool DoRecursion = IsOuterReparsePoint(
                aSrcPath, 
                aCurrentReparsePointReferencePath, 
                ReparsePointReferencePath, 
                &PreviousReparsePointReferencePath,
                wfind.dwFileAttributes, 
                ReparsePointBoundaryCrossed,
                aCurrentBoundaryCross,
                PreviousBoundaryCross,
                ReparseSrcTargetHint,
                aPathNameStatusList,
                FatalReparseError,
                aStats
              );

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
              wprintf(L"%s " __FUNCTION__ L" 05 %s, %d, %d, %d\n", m_ContainerName, aSrcPath, DoRecursion, aRefCount, FatalReparseError);
#endif
              // Check if we are in copy mode, then we also need the directories
              if (aRefCount <= 0 && !FatalReparseError)
              {
                // A directory might also be a Junction or Symbolic Link
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
                  wprintf(L"%s " __FUNCTION__ L" 06 %s\n", m_ContainerName, aSrcPath);
#endif
                // A directory might also be a Junction or Symbolic Link
                int ResultAddDir = AddDirectory(
                  aSrcPath, 
                  m_Filenames, 
                  wfind.dwFileAttributes, 
                  aStats, 
                  ReparseSrcTargetHint, 
                  OriginalFileAttribute, 
                  aPathNameStatusList
                );
                
                // We are not going to step down directories, which could not be accessed for various
                // reasons, like .e.g. During test we had a left over Symbolic Link on a truecrypt container
                // under XP. But XP can't handle Symbolic Links
                if (ResultAddDir != ERROR_SUCCESS)
                  DoRecursion = false;
              }

                __int64 FileAddedOneLevelBelow = 0;
              if (DoRecursion)
              {
                int	rr = _FindHardLinkTraditionalRecursive (aSrcPath, 
                  aSrcSize,
                  aCurrentReparsePointReferencePath, 
                  aCurrentBoundaryCross, 
                  aRefCount, 
                  FileAddedOneLevelBelow, 
                  aStats, 
                  aPathNameStatusList, 
                  apContext
                );

                // On return from the recursion restore the SrcPath
                aSrcPath[sSrcLen] = 0x00;

                // On return from a outer reparse point the ReparsePointReferencePath
                // and the boundary cross index must be restored 
                if (ReparsePointBoundaryCrossed)
                {
                  *aCurrentReparsePointReferencePath = PreviousReparsePointReferencePath;
                  aCurrentBoundaryCross = PreviousBoundaryCross;

                  // We need the previous diskid but the lookup when stepping back will always work
                  m_DiskSerialCache.Lookup(*aCurrentReparsePointReferencePath, m_CurrentSerialNumber);
                }

                // In case of catastrophic error, the recursion can be broken
                if (rr != 0)
                {
                  // TBD: if we encounter an error escape the recursion, and use PathNameStatuslist 
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
                    wprintf(L"%s " __FUNCTION__ L" 07 %08x\n", m_ContainerName, rr);
#endif
                  RetVal = rr;
                  break;
                }
              }

                  
                // On our way back we have to check if there were really files added. If not and we are
                // in include Mode, we have to get rid of these falsely added directories
                //
                if (FileAddedOneLevelBelow)
                {
                  // If one level below files have been added, we have to keep it
                  aFileAddedOneLevelBelow += FileAddedOneLevelBelow;
                }
                else
                {
                  // But do this only when we are searching for directories, because with DupeMerge we only 
                  // add files. So removing non added directories would result in loss of files.
                  if (m_RegExpInclFileList.size() && aRefCount <= 0)
                  {
#if defined USE_VECTOR
                    // Remove Last added directory
                    m_PathVector.pop_back();
                    m_Filenames.pop_back();
#else    

#endif
                    // If we went down the recursion, a directory has been added
                    if (DoRecursion)
                    {
                      aStats->m_DirectoryTotal--;
                    }
                    else
                    {
                      // But when an reparsepoint was encountered, the respective statistics has to be fixed
                      int TypeOfReparsePoint = REPARSE_POINT_UNDEFINED;
                      TypeOfReparsePoint = ProbeReparsePoint(aSrcPath, NULL);
                      switch (TypeOfReparsePoint)
                      {
                        case REPARSE_POINT_JUNCTION:
                          aStats->m_JunctionsTotal--;
                        break;

                        case REPARSE_POINT_MOUNTPOINT:
                          aStats->m_MountpointsTotal--;
                        break;

                        case REPARSE_POINT_SYMBOLICLINK:
                          aStats->m_SymlinksTotal--;
                        break;
                      }
                    }
                  }
                }

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
                wprintf(L"%s " __FUNCTION__ L" 08\n", m_ContainerName);
#endif
            } // if (RegExpMatch)

            if (!m_RegExpInclDirList.size())
            {
              if (!RegExpMatch)
              {
                // We need to keep track of excluded items in the statistic
                if (OriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
                {
                  int TypeOfReparsePoint = ProbeReparsePoint(aSrcPath, NULL);
                  switch (TypeOfReparsePoint)
                  {
                    case REPARSE_POINT_JUNCTION:
                      aStats->m_JunctionsTotal++;
                      aStats->m_JunctionsExcluded++;
                      Log(L"~j %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                    break;

                    case REPARSE_POINT_SYMBOLICLINK:
                      aStats->m_SymlinksTotal++;
                      aStats->m_SymlinksExcluded++;
                      Log(L"~s %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                    break;

                    case REPARSE_POINT_MOUNTPOINT:
                      aStats->m_MountpointsTotal++;
                      aStats->m_MountpointsExcluded++;
                      Log(L"~m %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                    break;

                    default:
                      // unknown reparse point excluded
                      aStats->m_ReparsePointExcluded++;
                      Log(L"~r %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                    break;
                  }
                }
                else
                {
                  aStats->m_DirectoryTotal++;
                  aStats->m_DirectoriesExcluded++;
                    Log(L"~d %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                }
              } // if (RegExpMatch)
            } // if (!m_RegExpInclDirList.size())

            aSrcPath[sSrcLen] = 0x00;
          }
        }
      }
      else
      // The else clause only deals with files
      {
        // Check if it is a hardlink
        sSrcLen = wcslen(aSrcPath);
        if ('\\' != aSrcPath[sSrcLen - 1])
          wcscat_s(aSrcPath, aSrcSize, L"\\");
        else
          sSrcLen--;

        wcscat_s(aSrcPath, aSrcSize, wfind.cFileName);

        if (IsVeryLongPath(aSrcPath))
          PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;

        if (apContext)
        {
          apContext->AddProgress(1, 1, 1);

          // Set the status, and check if Smartcopy has been cancelled from outside
          int r = apContext->PutStatus(&aSrcPath[PathParseSwitchOffSize]);
          if ( ERROR_REQUEST_ABORTED == r)
          {
            RetVal = ERROR_REQUEST_ABORTED;
            break;
          }
        }
        
        // When aRefCount >= 0 files and directories should be collected, but with 
        // aRefCount < 0 only directories should be collected
        if (aRefCount >= 0)
        {
          bool RegExpMatch;

          if (!m_RegExpInclFileList.size())
            // An empty Incl List means 'take everything'
            RegExpMatch = true;
          else
            // But if there are items on the incl-list we would like just have them
            RegExpMatch = MatchRegExpList(aSrcPath, m_RegExpInclFileList);

          // But afterwards exclude
          RegExpMatch &= !MatchRegExpList(aSrcPath, m_RegExpExclFileList);

          if (RegExpMatch) 
          {
            // See how to handle symbolic links, which are reparse points
            DWORD FileAttribute;
            bool UseReparseFileIndex = false;
            int ReparsePointType = REPARSE_POINT_UNDEFINED;


            wchar_t ReparseSrcTargetHint[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
            ReparseSrcTargetHint[0] = 0x00;
            if (wfind.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
              // This is similar to IsOuterReparsePoint()
              bool ReparsePointBoundaryCrossed = false;

              // Check if it is a Inner or Outer ReparsePoint ...
              wchar_t ReparseSrcTarget[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
              wcscpy_s(ReparseSrcTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);

              ReparsePointType = ProbeReparsePoint(aSrcPath, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);
              switch (ReparsePointType)
              {
                case REPARSE_POINT_MOUNTPOINT:
                  // With Mounpoint we receive \\?\Volume... as target, but we already have prepended \\?\ so 
                  // we have to get rid of one \\?\ 
                  wcscpy_s(ReparseSrcTarget, HUGE_PATH, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);
                break;

                case REPARSE_POINT_SYMBOLICLINK:
                {
                  // It might happen, that there are relative symbolic links
                  if ( PathIsRelative(&ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]) )
                  {
                    // Resolve the symlinks
                    WCHAR ReparseSrcTargetFullPath[HUGE_PATH] = { 0 };

                    WCHAR ReparseFullPath[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
                    wcscpy_s(ReparseFullPath, HUGE_PATH, &(*aCurrentReparsePointReferencePath)[PATH_PARSE_SWITCHOFF_SIZE]);
                    wcscat_s(ReparseFullPath, HUGE_PATH, &aSrcPath[aCurrentBoundaryCross ? aCurrentBoundaryCross : m_CurSourcePathLen]);

                    int iResult = ResolveSymboliclink(ReparseFullPath, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE], ReparseSrcTargetFullPath);
                    if (ERROR_SUCCESS == iResult)
                    {
                      // This is for WindowsXP only, because PathCanonicalize behaves differently on Windows7 and XP
                      // Windows7 cuts out a \\? prefixed pathe but XP generates does not which would result here in \\?\\?\ 
                      int CanonPos = 0;
                      if (IsVeryLongPath(ReparseSrcTargetFullPath))
                        CanonPos = PATH_PARSE_SWITCHOFF_SIZE;
                      // Unfortunately PathCanonicalize() cuts out \\?\ so we have to prepend it
                      int PathParseSwitchOff_ReparseSrcTarget = 0;
                      if (!PathIsUNC(&ReparseSrcTargetFullPath[CanonPos]))
                        PathParseSwitchOff_ReparseSrcTarget = PATH_PARSE_SWITCHOFF_SIZE;
                      PathCanonicalize(&ReparseSrcTarget[PathParseSwitchOff_ReparseSrcTarget], &ReparseSrcTargetFullPath[CanonPos]);
                    }
                  }
                  // Here we should end up in any case with ReparseSrcTarget beeing a long path
                }
                break;

                default:
                break;
              }

              // Check if it is a inner or outer symbolic link
              wchar_t* InnerReparsePoint = wcseistr(ReparseSrcTarget, *aCurrentReparsePointReferencePath);
              if (!InnerReparsePoint)
              {
                  // It was an outer symbolic link, we only change the attribute to
                // FILE_ATTRIBUTE_NORMAL no need to keep track of boundary crosses
                ReparsePointBoundaryCrossed = true;
              }
              else
              {
                // ReparseSrcTargetHint is a helper which is necessary for inner-outer ReparsePoints
                // and should only be used in that case. We don't want to use this helper
                // as long as we didn't cross out to an outer ReparsePoint
                if (aCurrentBoundaryCross)
                {
                  // Assemble destination for the perhaps new-inner symbolic link
                  aSrcPath[aCurrentBoundaryCross] = 0x00;
                  wcscpy_s(ReparseSrcTargetHint, HUGE_PATH, &aSrcPath[m_CurSourcePathLen]);
                  aSrcPath[aCurrentBoundaryCross] = '\\';
                  wcscat_s(ReparseSrcTargetHint, HUGE_PATH, InnerReparsePoint);
                }
              }

              FileAttribute = FILE_ATTRIBUTE_REPARSE_POINT;
              if (ReparsePointBoundaryCrossed)
              {
                if (m_Flags & eAlwaysUnrollDir)  
                {
                  UseReparseFileIndex = true;
                  FileAttribute = FILE_ATTRIBUTE_NORMAL;
                }
                else
                {
                  // Apply regular expression to know which symbolic link should be unrolled
                  bool RegExpUnrollMatch = MatchRegExpList(wfind.cFileName, m_RegExpUnrollDirList);
                  if (RegExpUnrollMatch)
                  {
                    UseReparseFileIndex = true;
                    FileAttribute = FILE_ATTRIBUTE_NORMAL;
                  }
                }
              } // if (ReparsePointBoundaryCrossed)

              // If a symlink points to a different drive we have to switch for 
              // just this very smlink to the disk index of the outer disk
              if (UseReparseFileIndex)
              {
                  int result = m_DiskSerialCache.Lookup(ReparseSrcTarget, m_CurrentSerialNumber);
                // TBD something could go wrong
                // if (ERROR_SUCCESS == result)
              }

            } // if (wfind.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            else
              FileAttribute = FILE_ATTRIBUTE_NORMAL;

            // File attribute is ready so add it to the container
            int ResultAddFile = AddFile(aSrcPath, 
              FileAttribute, 
              aRefCount,
              aStats,
              ReparseSrcTargetHint,
              &wfind,
              ReparsePointType,
              UseReparseFileIndex
            );
            aFileAddedOneLevelBelow++;

            // If we crossed the boundary we have to switch back to the 
            // original disk id
            if (UseReparseFileIndex)
              m_DiskSerialCache.Lookup(aSrcPath, m_CurrentSerialNumber);

            if (ERROR_SUCCESS != ResultAddFile)
            {
              // We failed on adding a file, so record it
              PathNameStatus pns(QuestionF, &aSrcPath[PathParseSwitchOffSize], ResultAddFile);
              aPathNameStatusList->push_back(pns);
            }
          }

          // The branch below must not work if we have the include option selected, because we don't 
          // want to see all the files as excluded, which are not selected via via wildcard and include
          if (!m_RegExpInclFileList.size())
          {
            if (!RegExpMatch)
            {
              // We need to keep track of excluded items in the statistic
              if (OriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
              {
                int TypeOfReparsePoint = ProbeReparsePoint(aSrcPath, NULL);
                switch (TypeOfReparsePoint)
                {
                  case REPARSE_POINT_JUNCTION:
                  // Empty: There are no 'Junction Files'
                  break;

                  case REPARSE_POINT_MOUNTPOINT:
                  // Empty: There are no 'Mountpoint Files'
                  break;

                    // Symbolic link files
                  case REPARSE_POINT_SYMBOLICLINK:
                    aStats->m_SymlinksTotal++;
                    aStats->m_SymlinksExcluded++;
                    Log(L"~s %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                  break;

                  default:
                  break;
                }
              }
              else // if (OriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
              {
                // Even if the files are excluded we have to add them so that the
                // statistics is correct
                aStats->m_FilesTotal++;
                aStats->m_FilesExcluded++;

                WIN32_FILE_ATTRIBUTE_DATA FileInfo;
                if (GetFileAttributesEx(aSrcPath, GetFileExInfoStandard, &FileInfo))
                {
                  UL64 size;
                  size.ul32l = FileInfo.nFileSizeLow;
                  size.ul32h = FileInfo.nFileSizeHigh;
                  aStats->m_BytesExcluded += size.ul64;

                  aStats->m_BytesTotal += size.ul64;

                  Log(L"~f %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                }
              }
            } // if (!RegExpMatch) 
          } // if (!m_RegExpInclFileList.size())
        }
        else
        {
          // Only directories will be collected if aRefCount < 0
        }

        aSrcPath[sSrcLen] = 0x00;
      } // else : deal with files
    }
    while (FindNextFile (sh, &wfind) > 0);

    FindClose (sh);
  }
  else // if (INVALID_HANDLE_VALUE != sh)
  {
    int PathParseSwitchOffSize = 0;
    if (IsVeryLongPath(aSrcPath))
      PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;

    // We end up rarley, but when a symbolic link under WindowsXP should be resolved this happens. 
    // This seems to be a problem of the third party Symbolic Link Driver for WindowsXP.
    DWORD dwLastError = GetLastError();
    int TypeOfReparsePoint = ProbeReparsePoint(aSrcPath, NULL);
    switch (TypeOfReparsePoint)
    {
      case REPARSE_POINT_JUNCTION:
      {
        // We failed on Searching a junction, so record it
        PathNameStatus pns(QuestionJ, &aSrcPath[PathParseSwitchOffSize], dwLastError);
        aPathNameStatusList->push_back(pns);
      }
      break;

      case REPARSE_POINT_MOUNTPOINT:
      {
        // We failed on Searching a junction, so record it
        PathNameStatus pns(QuestionM, &aSrcPath[PathParseSwitchOffSize], dwLastError);
        aPathNameStatusList->push_back(pns);
      }
      break;

      case REPARSE_POINT_SYMBOLICLINK:
      {
        // We failed on Searching a symlink, so record it
        PathNameStatus pns(QuestionS, &aSrcPath[PathParseSwitchOffSize], dwLastError);
        aPathNameStatusList->push_back(pns);
      }
      break;

      case REPARSE_POINT_FAIL:
      {
        PathNameStatus pns(QuestionD, &aSrcPath[PathParseSwitchOffSize], dwLastError);
        aPathNameStatusList->push_back(pns);
      }
      break;
    }
  }

  return RetVal;
}
int
FileInfoContainer::
_FindHardLinkRecursive(
  LPWSTR	                aSrcPath,
  const size_t            aSrcSize,
  PWCHAR*	                aCurrentReparsePointReferencePath,
  int&                    aCurrentBoundaryCross,
  int		                  aRefCount,
  __int64&                aFileAddedOneLevelBelow,
  CopyStatistics*	        aStats,
  _PathNameStatusList*    aPathNameStatusList,
  AsyncContext*           apContext
)
{
  //
  // Since the depth of the recursion is the depth of the source tree
  // we need to make sure this depth is not exceeded, otherwise we get cryptic
  // error messages from crashes and crash dump, which do not give any clue
  //
  // Taken from
  // http://stackoverflow.com/questions/1740888/determining-stack-space-with-visual-studio
  //

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
    wprintf(L"%s " __FUNCTION__ L" 01 %s\n", m_ContainerName, aSrcPath);
#endif

  size_t sp_value;

#if defined _M_IX86
  _asm { mov [sp_value], esp }
#elif defined _M_IA64
  CONTEXT context;
  RtlCaptureContext(&context);
  sp_value = context.IntSp;
#elif defined _M_X64
  CONTEXT context;
  RtlCaptureContext(&context);
  sp_value = context.Rsp;
#endif

  PNT_TIB lTeb = (PNT_TIB)NtCurrentTeb();
  size_t stackAvailable = (size_t)lTeb->StackBase - (size_t)lTeb->StackLimit;

  size_t used_stack_size = (size_t)lTeb->StackBase - sp_value;
  if (0 == m_StackUsageFindHardlink && m_LastStackUsageFindHardlink)
     m_StackUsageFindHardlink = used_stack_size - m_LastStackUsageFindHardlink;

  m_LastStackUsageFindHardlink = used_stack_size;

  // Make sure this number is the same as given under the linker section
  // via /stack:number . It seems that 45Mb is enough for a path with 128
  // levels aka 256 character for MAX_PATH
  if (used_stack_size + m_StackUsageFindHardlink > STACKSIZE)
    return ERROR_STACK_BUFFER_OVERRUN;

  //
  // ReparsePointReferencePath is kept for each level of recursion
  // and is referenced from deeper levels of recursions
  //
  PWCHAR                        PreviousReparsePointReferencePath;
  WCHAR	                        ReparsePointReferencePath[HUGE_PATH];
  int                           PreviousBoundaryCross;

	NTSTATUS                      ntStatus = 0x00;
	PFILE_ID_BOTH_DIR_INFORMATION BothDirInfo, CurrentBothDirInfo;
	BOOLEAN                       FirstFullDirQuery = TRUE;
	IO_STATUS_BLOCK               ioStatus;
  HANDLE                        DirHandle;
  OBJECT_ATTRIBUTES             Obja;
  UNICODE_STRING					      PathName;
  UNICODE_STRING					      FileName;
  RTL_RELATIVE_NAME             RelativeName;

  int							              RetVal = 0;
  size_t			                  sSrcLen = wcslen(aSrcPath);

  wcscat_s(aSrcPath, aSrcSize, L"\\*");
  NTSTATUS TranslationStatus = RtlDosPathNameToNtPathName_U(
    aSrcPath,
    &PathName,
    &FileName.Buffer,
    &RelativeName
  );
  aSrcPath[sSrcLen] = 0x00;


  if (FileName.Buffer) 
  {
    FileName.Length = PathName.Length - (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)PathName.Buffer);
  } 
  else 
  {
    FileName.Length = 0;
  }

  FileName.MaximumLength = FileName.Length;

  if (FileName.Buffer) 
  {
    PathName.Length = (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)PathName.Buffer);
    PathName.MaximumLength = PathName.Length;
  }

  if ( (PathName.Length>>1) >= 2 &&
    PathName.Buffer[(PathName.Length>>1)-2] != L':' &&
    PathName.Buffer[(PathName.Length>>1)-1] != L'\\'   
    ) 
  {
    PathName.Length -= sizeof(UNICODE_NULL);
  } 

  InitializeObjectAttributes(
    &Obja,
    &PathName,
    OBJ_CASE_INSENSITIVE,
    NULL,
    NULL
  );

  ntStatus = NtOpenFile(
    &DirHandle,
    FILE_LIST_DIRECTORY | SYNCHRONIZE,
    &Obja,
    &ioStatus,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
  );

  if( NT_SUCCESS( ntStatus ))
  {
    aStats->m_HeapAllocTime.Start();
    BothDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION) malloc( NTQUERYDIRECTORY_BUFSIZE + sizeof(long) );
    aStats->m_HeapAllocTime.Stop();
    while(1)
    {
      ntStatus = NtQueryDirectoryFile( 
        DirHandle, 
        NULL, 
        NULL, 
        0, 
        &ioStatus,
		    BothDirInfo, 
        NTQUERYDIRECTORY_BUFSIZE, 
        FileIdBothDirectoryInformation,
        FALSE, 
        &FileName, 
        FirstFullDirQuery 
      );
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
      wprintf(L"%s " __FUNCTION__ L" 03 %s, %08x\n", m_ContainerName, aSrcPath, ntStatus);
#endif
      if( !NT_SUCCESS( ntStatus ))
        break;

      FirstFullDirQuery = FALSE;
      CurrentBothDirInfo = BothDirInfo;

      while(1)
      {
        int PathParseSwitchOffSize = 0;
        
        DWORD OriginalFileAttribute = CurrentBothDirInfo->FileAttributes;
        if (CurrentBothDirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          if (wcsncmp (CurrentBothDirInfo->FileName, L".", CurrentBothDirInfo->FileNameLength / sizeof(wchar_t)))
          {
            if (wcsncmp (CurrentBothDirInfo->FileName, L"..", CurrentBothDirInfo->FileNameLength / sizeof(wchar_t)))
            {
              sSrcLen = wcslen(aSrcPath);
              if ('\\' != aSrcPath[sSrcLen - 1])
                wcscat_s(aSrcPath, aSrcSize, L"\\");
              else
                sSrcLen--;

              wcsncat_s(aSrcPath, aSrcSize, CurrentBothDirInfo->FileName, CurrentBothDirInfo->FileNameLength / sizeof(wchar_t));
              aSrcPath[sSrcLen + CurrentBothDirInfo->FileNameLength / sizeof(wchar_t) + 1] = 0x00;

              if (IsVeryLongPath(aSrcPath))
                PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;


#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
              wprintf(L"%s " __FUNCTION__ L" 04 %s\n", m_ContainerName, aSrcPath);
#endif
              if (apContext)
              {
                apContext->AddProgress(eDirectoryWeight, 1, 1);

                // Set the status, and check if FindHardLink has been cancelled from outside
                int r = apContext->PutStatus(&aSrcPath[PathParseSwitchOffSize]);
                if ( ERROR_REQUEST_ABORTED == r)
                {
                  RetVal = ERROR_REQUEST_ABORTED;
                  break;
                }
              }

              // Check if there are reg exps for exclusion of directories
              bool RegExpIncludeMatch; 
              if (!m_RegExpInclDirList.size())
                // An empty include list means 'take everything'
                RegExpIncludeMatch = true;
              else
                // But if there are items on the include list we would like only to have them
                RegExpIncludeMatch = MatchRegExpList(&aSrcPath[PathParseSwitchOffSize], m_RegExpInclDirList);

              // But afterwards exclude
              bool RegExpExcludeMatch = MatchRegExpList(&aSrcPath[PathParseSwitchOffSize], m_RegExpExclDirList);

              bool RegExpMatch = RegExpIncludeMatch && !RegExpExcludeMatch;
#if defined DEBUG_REGEXP_SEAN_MALONEY
              // Try out for seanmaloney@hotmail.com: Ln.exe --includedir bug?. Siehe Inbox
              // The problem is that we want to match patterns of a path e.g. *p1\\p2*, but crawling down does not happen
              // because when we come here, only *p1 would match. So to match path patterns, which go over many levels, we have
              // to delay this to the full path, but this can only be done, once we have gone down the recursion.
              // But at the end of the recursion it might turn out, that no match was there, but then on the way down the recursion many directories
              // have been added, which didn't match, but had to be added just to crawl down. sic!
              if (RegExpMatch || ( m_RegExpInclDirList.size() && !RegExpIncludeMatch ) )
#else
              if (RegExpMatch)
#endif
              {
                // By default we do not follow reparse points
                bool ReparsePointBoundaryCrossed = false;

                wchar_t ReparseSrcTargetHint[HUGE_PATH];
                ReparseSrcTargetHint[0] = 0x00;
                
                // IsOuterReparseTarget() might throw fatal error, which prevent further adding of that directory to the pool
                bool  FatalReparseError = false;
                bool DoRecursion = IsOuterReparsePoint(
                  aSrcPath, 
                  aCurrentReparsePointReferencePath, 
                  ReparsePointReferencePath, 
                  &PreviousReparsePointReferencePath,
                  CurrentBothDirInfo->FileAttributes, 
                  ReparsePointBoundaryCrossed,
                  aCurrentBoundaryCross,
                  PreviousBoundaryCross,
                  ReparseSrcTargetHint,
                  aPathNameStatusList,
                  FatalReparseError,
                  aStats
                );

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
                wprintf(L"%s " __FUNCTION__ L" 05 %s, %d, %d, %d\n", m_ContainerName, aSrcPath, DoRecursion, aRefCount, FatalReparseError);
#endif
                // Check if we are in copy mode, then we also need the directories
                if (aRefCount <= 0 && !FatalReparseError)
                {
                  // A directory might also be a Junction or Symbolic Link
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
                  wprintf(L"%s " __FUNCTION__ L" 06 %s\n", m_ContainerName, aSrcPath);
#endif
                  int ResultAddDir = 
                    AddDirectoryFast(
                    aSrcPath, 
                    m_Filenames, 
                    aStats, 
                    ReparseSrcTargetHint, 
                    OriginalFileAttribute, 
                    aPathNameStatusList, 
                    CurrentBothDirInfo
                  );
                  
                  // We are not going to step down directories, which could not be accessed for various
                  // reasons, like .e.g. During test we had a left over Symbolic Link on a truecrypt container
                  // under XP. But XP can't handle Symbolic Links
                  if (ResultAddDir != ERROR_SUCCESS)
                    DoRecursion = false;
                }

                __int64 FileAddedOneLevelBelow = 0;
                if (DoRecursion)
                {
                  int	RecursionResult = _FindHardLinkRecursive (aSrcPath, 
                    aSrcSize,
                    aCurrentReparsePointReferencePath,
                    aCurrentBoundaryCross, 
                    aRefCount, 
                    FileAddedOneLevelBelow, 
                    aStats, 
                    aPathNameStatusList, 
                    apContext
                  );

                  // On return from the recursion restore the SrcPath
                  aSrcPath[sSrcLen] = 0x00;

                  // On return from a outer reparse point the ReparsePointReferencePath
                  // and the boundary cross index must be restored 
                  if (ReparsePointBoundaryCrossed)
                  {
                    *aCurrentReparsePointReferencePath = PreviousReparsePointReferencePath;
                    aCurrentBoundaryCross = PreviousBoundaryCross;

                    // We need the previous diskid but the lookup when stepping back will always work
                    m_DiskSerialCache.Lookup(*aCurrentReparsePointReferencePath, m_CurrentSerialNumber);
                  }

                  // In case of catastrophic error, the recursion can be broken
                  if (RecursionResult != 0)
                  {
                    // TBD: if we encounter an error escape the recursion, and use PathNameStatuslist 
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
                    wprintf(L"%s " __FUNCTION__ L" 07 %08x\n", m_ContainerName, RecursionResult);
#endif
                    RetVal = RecursionResult;
                    break;
                  }
                }

                  
                // On our way back we have to check if there were really files added. If not and we are
                // in include Mode, we have to get rid of these falsely added directories
                //
                if (FileAddedOneLevelBelow)
                {
                  // If one level below files have been added, we have to keep it
                  aFileAddedOneLevelBelow += FileAddedOneLevelBelow;
                }
                else
                {
                  // But do this only when we are searching for directories, because with DupeMerge we only 
                  // add files. So removing non added directories would result in loss of files.
                  if (m_RegExpInclFileList.size() && aRefCount <= 0)
                  {
#if defined USE_VECTOR
                    // Remove last added directory
                    m_PathVector.pop_back();
                    m_Filenames.pop_back();
#else    

#endif
                    // If we went down the recursion, a directory has been added
                    if (DoRecursion)
                    {
                      aStats->m_DirectoryTotal--;
                    }
                    else
                    {
                      // But when an reparsepoint was encountered, the respective statistics has to be fixed
                      int TypeOfReparsePoint = REPARSE_POINT_UNDEFINED;
                      TypeOfReparsePoint = ProbeReparsePoint(aSrcPath, NULL);
                      switch (TypeOfReparsePoint)
                      {
                        case REPARSE_POINT_JUNCTION:
                          aStats->m_JunctionsTotal--;
                        break;

                        case REPARSE_POINT_MOUNTPOINT:
                          aStats->m_MountpointsTotal--;
                        break;

                        case REPARSE_POINT_SYMBOLICLINK:
                          aStats->m_SymlinksTotal--;
                        break;
                      }
                    }
                  }
                }

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
                wprintf(L"%s " __FUNCTION__ L" 08\n", m_ContainerName);
#endif
              } // if (RegExpMatch)

              if (!m_RegExpInclDirList.size())
              {
                if (!RegExpMatch)
                {
                  // We need to keep track of excluded items in the statistic
                  if (OriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
                  {
                    int TypeOfReparsePoint = ProbeReparsePoint(aSrcPath, NULL);
                    switch (TypeOfReparsePoint)
                    {
                      case REPARSE_POINT_JUNCTION:
                        aStats->m_JunctionsTotal++;
                        aStats->m_JunctionsExcluded++;
                        Log(L"~j %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                      break;

                      case REPARSE_POINT_SYMBOLICLINK:
                        aStats->m_SymlinksTotal++;
                        aStats->m_SymlinksExcluded++;
                        Log(L"~s %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                      break;

                      case REPARSE_POINT_MOUNTPOINT:
                        aStats->m_MountpointsTotal++;
                        aStats->m_MountpointsExcluded++;
                        Log(L"~m %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                      break;

                      default:
                      // unknown reparse point excluded
                      aStats->m_ReparsePointExcluded++;
                      Log(L"~r %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                      break;
                    }
                  }
                  else
                  {
                    aStats->m_DirectoryTotal++;
                    aStats->m_DirectoriesExcluded++;
                    Log(L"~d %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                  }
                } // if (RegExpMatch)
              } // if (!m_RegExpInclDirList.size())

              aSrcPath[sSrcLen] = 0x00;
            }
          }
        }
        else
        // The else clause only deals with files
        {
          // Check if it is a hardlink
          sSrcLen = wcslen(aSrcPath);

          // Volume{GUID} path end with a slash, but we don't want to have two slashes
          if ('\\' != aSrcPath[sSrcLen - 1])
            wcscat_s(aSrcPath, aSrcSize, L"\\");
          else
            sSrcLen--;

          wcsncat_s(aSrcPath, aSrcSize, CurrentBothDirInfo->FileName, CurrentBothDirInfo->FileNameLength / sizeof(wchar_t));
          aSrcPath[sSrcLen + CurrentBothDirInfo->FileNameLength / sizeof(wchar_t) + 1] = 0x00;

          if (IsVeryLongPath(aSrcPath))
            PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;

          if (apContext)
          {
            apContext->AddProgress(1, 1, 1);

            // Set the status, and check if Smartcopy has been cancelled from outside
            int r = apContext->PutStatus(&aSrcPath[PathParseSwitchOffSize]);
            if ( ERROR_REQUEST_ABORTED == r)
            {
              RetVal = ERROR_REQUEST_ABORTED;
              break;
            }
          }
          
          // When aRefCount >= 0 files and directories should be collected, but with 
          // aRefCount < 0 only directories should be collected
          if (aRefCount >= 0)
          {
            // we have to unfortunately send a zero terminated string
            wchar_t SaveChar = CurrentBothDirInfo->FileName[CurrentBothDirInfo->FileNameLength / sizeof(wchar_t)];
            CurrentBothDirInfo->FileName[CurrentBothDirInfo->FileNameLength / sizeof(wchar_t)] = 0x00;

            bool RegExpMatch;

            if (!m_RegExpInclFileList.size())
            {
              // An empty include List means 'take everything'
              RegExpMatch = true;
            }
            else
            {
              // But if there are items on the include list we would like only to have them
              aStats->m_RegExpMatchTime.Start();
              RegExpMatch = MatchRegExpList(aSrcPath, m_RegExpInclFileList);
              aStats->m_RegExpMatchTime.Stop();
            }

            // But afterwards exclude
            aStats->m_RegExpMatchTime.Start();
            RegExpMatch &= !MatchRegExpList(aSrcPath, m_RegExpExclFileList);
            aStats->m_RegExpMatchTime.Stop();

            CurrentBothDirInfo->FileName[CurrentBothDirInfo->FileNameLength / sizeof(wchar_t)] = SaveChar;

            if (RegExpMatch) 
            {
              // See how to handle symbolic links, which are reparse points
              DWORD FileAttribute;
              bool UseReparseFileIndex = false;
              int ReparsePointType = REPARSE_POINT_UNDEFINED;

              wchar_t ReparseSrcTargetHint[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
              ReparseSrcTargetHint[0] = 0x00;
              if (CurrentBothDirInfo->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
              {
                // This is similar to IsOuterReparsePoint()
                bool ReparsePointBoundaryCrossed = false;

                // Check if it is a Inner or Outer ReparsePoint ...
                wchar_t ReparseSrcTarget[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
                wcscpy_s(ReparseSrcTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);

                ReparsePointType = ProbeReparsePoint(aSrcPath, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);
                switch (ReparsePointType)
                {
                  case REPARSE_POINT_MOUNTPOINT:
                  // With Mounpoint we receive \\?\Volume... as target, but we already have prepended \\?\ so 
                  // we have to get rid of one \\?\ 
                    wcscpy_s(ReparseSrcTarget, HUGE_PATH, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);
                  break;

                  case REPARSE_POINT_SYMBOLICLINK:
                  {
                    // It might happen, that there are relative symbolic links
                    if ( PathIsRelative(&ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]) )
                    {
                      // Resolve the symlinks
                      WCHAR ReparseSrcTargetFullPath[HUGE_PATH];
                      
                      WCHAR ReparseFullPath[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
                      wcscpy_s(ReparseFullPath, HUGE_PATH, &(*aCurrentReparsePointReferencePath)[PATH_PARSE_SWITCHOFF_SIZE]);
                      wcscat_s(ReparseFullPath, HUGE_PATH, &aSrcPath[aCurrentBoundaryCross ? aCurrentBoundaryCross : m_CurSourcePathLen]);
                      
                      int iResult = ResolveSymboliclink(ReparseFullPath, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE], ReparseSrcTargetFullPath);
                      if (ERROR_SUCCESS == iResult)
                      {
                        // This is for WindowsXP only, because PathCanonicalize behaves differently on Windows7 and XP
                        // Windows7 merges two \\? prefixed pathes into one prefix \\? but XP generates \\?\\?\ 
                        int CanonPos = 0;
                        if (IsVeryLongPath(ReparseSrcTargetFullPath))
                          CanonPos = PATH_PARSE_SWITCHOFF_SIZE;
                        // Unfortunately PathCanonicalize() cuts out \\?\ so we have to prepend it
                        int PathParseSwitchOff_ReparseSrcTarget = 0;
                        if (!PathIsUNC(&ReparseSrcTargetFullPath[CanonPos]))
                          PathParseSwitchOff_ReparseSrcTarget = PATH_PARSE_SWITCHOFF_SIZE;

                        PathCanonicalize(&ReparseSrcTarget[PathParseSwitchOff_ReparseSrcTarget], &ReparseSrcTargetFullPath[CanonPos]);
                      }
                    }
                    // Here we should end up in any case with ReparseSrcTarget beeing a long path
                  }
                  break;

                  default:
                  break;
                }

                // Check if it is a inner or outer symbolic link
                //
                // This is the secret of unknown reparse points beeing copied when used with --unroll:
                wchar_t* InnerReparsePoint = wcseistr(ReparseSrcTarget, *aCurrentReparsePointReferencePath);
                if (!InnerReparsePoint)
                {
                  // It was an outer symbolic link, we only change the attribute to
                  // FILE_ATTRIBUTE_NORMAL no need to keep track of boundary crosses
                  ReparsePointBoundaryCrossed = true;
                }
                else
                {
                  // ReparseSrcTargetHint is a helper which is necessary for inner-outer ReparsePoints
                  // and should only be used in that case. We don't want to use this helper
                  // as long as we didn't cross out to an outer ReparsePoint
                  if (aCurrentBoundaryCross)
                  {
                    // Assemble destination for the perhaps new-inner symbolic link
                    aSrcPath[aCurrentBoundaryCross] = 0x00;
                    wcscpy_s(ReparseSrcTargetHint, HUGE_PATH, &aSrcPath[m_CurSourcePathLen]);
                    aSrcPath[aCurrentBoundaryCross] = '\\';
                    wcscat_s(ReparseSrcTargetHint, HUGE_PATH, InnerReparsePoint);
                  }
                }

                FileAttribute = FILE_ATTRIBUTE_REPARSE_POINT;
                if (ReparsePointBoundaryCrossed)
                {
                  if (m_Flags & eAlwaysUnrollDir)  
                  {
                    UseReparseFileIndex = true;
                    FileAttribute = FILE_ATTRIBUTE_NORMAL;
                  }
                  else
                  {
                    // We have to unfortunately send a zero terminated string, so save characters
                    wchar_t SaveChar = CurrentBothDirInfo->FileName[CurrentBothDirInfo->FileNameLength / sizeof(wchar_t)];
                    CurrentBothDirInfo->FileName[CurrentBothDirInfo->FileNameLength / sizeof(wchar_t)] = 0x00;
                    
                    // Apply regular expression to know which symbolic link should be unrolled
                    bool RegExpUnrollMatch = MatchRegExpList(CurrentBothDirInfo->FileName, m_RegExpUnrollDirList);
                    
                    CurrentBothDirInfo->FileName[CurrentBothDirInfo->FileNameLength / sizeof(wchar_t)] = SaveChar;

                    if (RegExpUnrollMatch)
                    {
                      UseReparseFileIndex = true;
                      FileAttribute = FILE_ATTRIBUTE_NORMAL;
                    }
                  }
                } // if (ReparsePointBoundaryCrossed)

                // If a symlink points to a different drive we have to switch for 
                // just this very smlink to the disk index of the outer disk
                if (UseReparseFileIndex)
                {
                  int result = m_DiskSerialCache.Lookup(ReparseSrcTarget, m_CurrentSerialNumber);
                  // TBD something could go wrong
                  // if (ERROR_SUCCESS == result)
                }

              } // if (wfind.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
              else
                FileAttribute = FILE_ATTRIBUTE_NORMAL;

              // File attribute is ready so add it to the container
              int ResultAddFile = AddFileFast(aSrcPath, 
                FileAttribute, 
                aStats,
                ReparseSrcTargetHint,
                UseReparseFileIndex,
                CurrentBothDirInfo,
                ReparsePointType
              );
              aFileAddedOneLevelBelow++;

              // If we crossed the boundary we have to switch back to the 
              // original disk id
              if (UseReparseFileIndex)
                m_DiskSerialCache.Lookup(aSrcPath, m_CurrentSerialNumber);

              if (ERROR_SUCCESS != ResultAddFile)
              {
                // We failed on adding a file, so record it
                PathNameStatus pns(QuestionF, &aSrcPath[PathParseSwitchOffSize], ResultAddFile);
                aPathNameStatusList->push_back(pns);
              }
            }
            
            // The branch below must not work if we have the include optione selected, because we don't 
            // want to see all the files as excluded, which are not selected via via wildcard and include
            if (!m_RegExpInclFileList.size())
            {
              if (!RegExpMatch)
              {
                // We need to keep track of excluded items in the statistic
                if (OriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
                {
                  int TypeOfReparsePoint = ProbeReparsePoint(aSrcPath, NULL);
                  switch (TypeOfReparsePoint)
                  {
                    case REPARSE_POINT_JUNCTION:
                    // Empty: There are no 'Junction Files'
                    break;

                    case REPARSE_POINT_MOUNTPOINT:
                    // Empty: There are no 'Mountpoint Files'
                    break;

                    // Symbolic link files
                    case REPARSE_POINT_SYMBOLICLINK:
                      aStats->m_SymlinksTotal++;
                      aStats->m_SymlinksExcluded++;
                      Log(L"~s %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                    break;

                    default:
                    break;
                  }
                }
                else // if (OriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
                {
                  // Even if the files are excluded we have to add them so that the
                  // statistics is correct
                  aStats->m_FilesTotal++;
                  aStats->m_FilesExcluded++;

                  WIN32_FILE_ATTRIBUTE_DATA FileInfo;
                  if (GetFileAttributesEx(aSrcPath, GetFileExInfoStandard, &FileInfo))
                  {
                    UL64 size;
                    size.ul32l = FileInfo.nFileSizeLow;
                    size.ul32h = FileInfo.nFileSizeHigh;
                    aStats->m_BytesExcluded += size.ul64;

                    aStats->m_BytesTotal += size.ul64;

                    Log(L"~f %s\n", &aSrcPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                  }
                }
              } // if (!RegExpMatch) 
            } // if (!m_RegExpInclFileList.size())
          }
          else
          {
            // Only directories will be collected if aRefCount < 0
          }

          aSrcPath[sSrcLen] = 0x00;
        } // else : deal with files

        if( CurrentBothDirInfo->NextEntryOffset == 0 ) 
          break;

        CurrentBothDirInfo = (PFILE_ID_BOTH_DIR_INFORMATION) ((PCHAR) CurrentBothDirInfo + CurrentBothDirInfo->NextEntryOffset );

      } // while
    } // while

    CloseHandle (DirHandle);

    aStats->m_HeapDeletionTime.Start();
    free(BothDirInfo);
    aStats->m_HeapDeletionTime.Stop();
  }
  else // if( NT_SUCCESS( ntStatus ))
  {
    int PathParseSwitchOffSize = 0;
    if (IsVeryLongPath(aSrcPath))
      PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;

    DWORD dwLastError = GetLastError();
//    DWORD dwLastError = RtlNtStatusToDosError(ntStatus);
    int TypeOfReparsePoint = ProbeReparsePoint(aSrcPath, NULL);
    switch (TypeOfReparsePoint)
    {
      case REPARSE_POINT_JUNCTION:
      {
        // We failed on Searching a junction, so record it
        PathNameStatus pns(QuestionJ, &aSrcPath[PathParseSwitchOffSize], dwLastError);
        aPathNameStatusList->push_back(pns);
      }
      break;

      case REPARSE_POINT_MOUNTPOINT:
      {
        // We failed on Searching a mountpoint, so record it
        PathNameStatus pns(QuestionM, &aSrcPath[PathParseSwitchOffSize], dwLastError);
        aPathNameStatusList->push_back(pns);
      }
      break;

      case REPARSE_POINT_SYMBOLICLINK:
      {
        // We failed on Searching a symlink, so record it
        PathNameStatus pns(QuestionS, &aSrcPath[PathParseSwitchOffSize], dwLastError);
        aPathNameStatusList->push_back(pns);
      }
      break;

      case REPARSE_POINT_FAIL:
      {
//        PathNameStatus pns(QuestionD, &aSrcPath[PATH_PARSE_SWITCHOFF_SIZE], RtlNtStatusToDosError(ntStatus));
        PathNameStatus pns(QuestionD, &aSrcPath[PathParseSwitchOffSize], MapNtStatusToWinError(ntStatus));
        aPathNameStatusList->push_back(pns);
      }
      break;
    }
  }

  return RetVal;
}

bool 
FileInfoContainer::
IsOuterReparsePoint(
  PWCHAR                  aSrcPath,
  PWCHAR*                 aCurrentReparsePointReferencePath,
  PWCHAR                  aReparsePointReferencePath,
  PWCHAR*                 aPreviousReparsePointReferencePath,
  DWORD&                  aFileAttributes,
  bool&                   aReparsePointBoundaryCrossed,
  int&                    aCurrentBoundaryCross,
  int&                    aPreviousBoundaryCross,
  
  //
  // In certain situation like unrolling during this method the final structure 
  // as it will be in the destination is known, but only here. Later on e.g. during
  // CopyReparsePoints() it can not be found out where the real and final target of the 
  // ReparsePoint is. So at this place we have to keep this final path as hint, and 
  // save it for CopyReparsePoints().
  //
  PWCHAR                  aReparseSrcTargetHint,
  _PathNameStatusList*    aPathNameStatusList,
  bool&                   aFatalReparseError,
  CopyStatistics*	        aStats
)
{
  bool DoRecursion = false;

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
  wprintf(L"%s " __FUNCTION__ L" 01 %s, %08x\n", m_ContainerName, aSrcPath, aFileAttributes);
#endif
  if (aFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
  {
    // Fetch the target of the ReparsePoint
    wchar_t ReparseSrcTarget[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
    wcscpy_s(ReparseSrcTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);

    int ReparsePointType = ProbeReparsePoint(aSrcPath, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);

    switch (ReparsePointType)
    {
      case REPARSE_POINT_MOUNTPOINT:
        // With Mounpoint we receive \\?\Volume... as target, but we already have prepended \\?\ so 
        // we have to get rid of one \\?\ 
        wcscpy_s(ReparseSrcTarget, HUGE_PATH, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);
      break;

      case REPARSE_POINT_SYMBOLICLINK:
      {
        // Only symbolic links might be relative. Junctions are always absolute
        // Since PathIsRelative() is expensive, we do not give Junctions the chance
        // to be tested for relatity, by ahead checking the reparsepoint type

        // It might happen, that there are relative symbolic links
        if ( PathIsRelative(&ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]) )
        {
          // Resolve the symlinks
          WCHAR ReparseSrcTargetFullPath[HUGE_PATH];
          
          // ProbeReparsePoint returns normal path, so we prepend \\?\ 
          WCHAR ReparseFullPath[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
          wcscpy_s(ReparseFullPath, HUGE_PATH, &(*aCurrentReparsePointReferencePath)[PATH_PARSE_SWITCHOFF_SIZE]);
          wcscat_s(ReparseFullPath, HUGE_PATH, &aSrcPath[aCurrentBoundaryCross ? aCurrentBoundaryCross : m_CurSourcePathLen]);
          
          int iResult = ResolveSymboliclink(ReparseFullPath, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE], ReparseSrcTargetFullPath);
          if (ERROR_SUCCESS == iResult)
          {
            // This is for WindowsXP only, because PathCanonicalize behaves differently on Windows7 and XP
            // Windows7 cuts out a \\? prefixed pathe but XP generates does not which would result here in \\?\\?\ 
            int CanonPos = 0;
            if (IsVeryLongPath(ReparseSrcTargetFullPath))
              CanonPos = PATH_PARSE_SWITCHOFF_SIZE;

            // Unfortunately PathCanonicalize() cuts out \\?\ so we have to prepend it
            int PathParseSwitchOff_ReparseSrcTarget = 0;
            if (!PathIsUNC(&ReparseSrcTargetFullPath[CanonPos]))
              PathParseSwitchOff_ReparseSrcTarget = PATH_PARSE_SWITCHOFF_SIZE;

            PathCanonicalize(&ReparseSrcTarget[PathParseSwitchOff_ReparseSrcTarget], &ReparseSrcTargetFullPath[CanonPos]);
          }
        }
      }
      break;
    }

    // Here we should end up in any case with ReparseSrcTarget beeing a long path

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
  wprintf(L"%s " __FUNCTION__ L" 02 '%s', %08x, '%s'\n", m_ContainerName, aSrcPath, ReparsePointType, ReparseSrcTarget);
#endif

    //
    // Circularity Check: See if we are running into a circularity, which would cause us
    // to run till the stack is over when we unroll
    //
    // But we have to add a \ to the ReparseSrcTarget because otherwise a 
    // ReparseSrcTarget == x:\path\3 and a aSrcPath == x:\path\333 will result
    // in a recursive junction, which it is not. So if we temporarily add a 
    // \ to x:\path\3 ReparseSrcTarget == x:\path\3\ and the wcseistr() will 
    // correctly report that it is not a circular dependency
    //
    size_t ReparseSrcTargetLen = wcslen(ReparseSrcTarget);
    wchar_t SaveChar1;
    wchar_t SaveChar2;
    if (L'\\' != ReparseSrcTarget[ReparseSrcTargetLen - 1])
    {
      SaveChar1 = ReparseSrcTarget[ReparseSrcTargetLen];
      SaveChar2 = ReparseSrcTarget[ReparseSrcTargetLen + 1];
      ReparseSrcTarget[ReparseSrcTargetLen] = L'\\';
      ReparseSrcTarget[ReparseSrcTargetLen + 1] = 0x00;
    }
    else
      ReparseSrcTargetLen = 0;

    wchar_t* Circularity = wcseistr(aSrcPath, ReparseSrcTarget);
    
    if (ReparseSrcTargetLen)
    {
      ReparseSrcTarget[ReparseSrcTargetLen] = SaveChar1;
      ReparseSrcTarget[ReparseSrcTargetLen + 1] = SaveChar2;
    }

    //
    // Decide whether it is an Inner or Outer ReparsePoint
    //

    //
    // We have to add a \ to the aCurrentReparsePointReferencePath because otherwise a 
    // aCurrentReparsePointReferencePath == x:\path\3 and a ReparseSrcTarget == x:\path\333 will result
    // in a inner reparse point, which it is not. So if we temporarily add a 
    // \ to x:\path\3 aCurrentReparsePointReferencePath == x:\path\3\ and the wcseistr() will 
    // correctly report that it is not a inner reparse point
    //
    size_t CurrentReparsePointReferencePathLen = wcslen(*aCurrentReparsePointReferencePath);
    if (L'\\' != (*aCurrentReparsePointReferencePath)[CurrentReparsePointReferencePathLen - 1])
    {
      SaveChar1 = (*aCurrentReparsePointReferencePath)[CurrentReparsePointReferencePathLen];
      SaveChar2 = (*aCurrentReparsePointReferencePath)[CurrentReparsePointReferencePathLen + 1];
      (*aCurrentReparsePointReferencePath)[CurrentReparsePointReferencePathLen] = L'\\';
      (*aCurrentReparsePointReferencePath)[CurrentReparsePointReferencePathLen + 1] = 0x00;
    }
    else
      CurrentReparsePointReferencePathLen = 0;

    // Check if it is an inner or outer ReparsePoint by checking if we find the aCurrentReparsePointReferencePath
    // in the ReparseSrcTarget ...
    wchar_t* InnerReparsePoint = wcseistr(ReparseSrcTarget, *aCurrentReparsePointReferencePath);
    
    // ... and see if one of the stored path in the AnchorPathCache is part of the ReparseSrcTarget
    // which means that it is a inner junction with respect to the given source path
    //
    // [REMOTE_JUNCTION_RESOLVE]
    // One more note: Junctions on network drives might point to places which look 'outer', but they
    // are not: e.g. 
    // remote: n:\tmp\foo -> n:\tmp\bla
    // remote: n:\tmp shared as \\server\share
    // local: \\server\share mounted as t:\ 
    // local: t:\foo returns a target to n:\tmp\bla, which looks outer, but it is not.
    // The m_AnchorPathCache contains also an additional entry generated in ResolveRemote() under 
    // which a drive was shared, here n:\tmp, so that ReparseSrcTarget is found anyway.
    // This prevents unrolling for junctions on mapped drives. In CopyReparsePoints we have to deal
    // that the junctions get created correctly.
    // [/REMOTE_JUNCTION_RESOLVE]
    //
    wchar_t* MultiSourcePath = m_AnchorPathCache.ContainsSource(ReparseSrcTarget);

    //
    // If a ReparsePoint directly points to one of the source directories, the above
    // search in m_AnchorPathCache would be successfull, and make this a inner 
    // ReparsePoint. But this must not happen, because this generates endless ReparsePoint
    // loops. To avoid this we check for an exact match, and force the algorithm to treat
    // such a situation as a outer ReparsePoint
    //
    if (MultiSourcePath)
    {
      //
      // Detection of such an exact match is easy. wcseistr() points to the terminating \0 character
      // of ReparseSrcTarget
      if (!*MultiSourcePath)
      {
#if 0
        // TBD
        if (m_AnchorPathCache.ContainsExactSource(aSrcPath) && (aFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
        {
          // There is one more special condition:
          // If the source itself was on the m_AnchorPathCache, then this should be treated
          // as inner reparse point, because this means a reparse point was selected on level 0.
          // See Multisource01.bat: MS01_2
        }
        else
        {
          // Invalidate, because it would generate endless reparse point loops.
          MultiSourcePath = NULL;
        }
#else
        MultiSourcePath = NULL;

#endif

      }
    }


    // Do we need to repair a few modified characters?
    if (CurrentReparsePointReferencePathLen)
    {
      (*aCurrentReparsePointReferencePath)[CurrentReparsePointReferencePathLen] = SaveChar1;
      (*aCurrentReparsePointReferencePath)[CurrentReparsePointReferencePathLen + 1] = SaveChar2;
    }

    //
    // Check what the reason for 'Innerness' was. If it was just the fact, that it was due to
    // multiple source, then we don't want to unroll. This is flagged by setting 
    // aReparsePointBoundaryCrossed to false
    // 
    // If both MultiSourcePath and InnerReparsePoint are != 0, which happens if we traverse
    // the same locations twice, once copied due to --source and once due to be unrolled, 
    // because a Reparse Point directly pointed to a junction, we must be carefull to choose
    // the InnerReparsePoint way, because otherwise the Reparsepoints from unrolled portions
    // of the tree points to locations which are there due to --source
    // 
    // The UnrollPrepare01.bat sample shows this for the unrolled portion below F3_J0, which
    // point to F2/F2_F2, and the F2/F2_F2 portion, which is there due to --source F2
    // 
    if (MultiSourcePath && !InnerReparsePoint)
    {
      //
      // With only relative ReparsePoints it is completley impossible to find back to the
      // ReparseSrcTarget during CopyReparsePoints. We can pass a hint to CopyReparsePoints()
      // by storing the ReparseSrcTarget in aReparseSrcTargetHint
      //
      if (REPARSE_POINT_SYMBOLICLINK == ReparsePointType)
        wcscpy_s(aReparseSrcTargetHint, HUGE_PATH, MultiSourcePath);  

      aReparsePointBoundaryCrossed = false;
    }
    else
    {
      // 
      // InnerReparsePoint != 0 means that the ReparseSrcTarget is a inner junction
      // 
      if (InnerReparsePoint)
      {
  #if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
        wprintf(L"%s " __FUNCTION__ L" 04 %s, %08x, %s\n", m_ContainerName, aSrcPath, aCurrentBoundaryCross, aCurrentReparsePointReferencePath);
  #endif
        // Inner ReparsePoint with respect to aCurrentReparsePointReferencePath
        if (aCurrentBoundaryCross)
        {
          // Assemble destination for the new inner-outer ReparsePoint, but
          // only if it is a new inner-outer ReparsePoint
          aSrcPath[aCurrentBoundaryCross] = 0x00;
          wcscpy_s(aReparseSrcTargetHint, HUGE_PATH, &aSrcPath[m_CurSourcePathLen]);
          aSrcPath[aCurrentBoundaryCross] = '\\';
          if (*InnerReparsePoint != '\\')
            wcscat_s(aReparseSrcTargetHint, HUGE_PATH, L"\\");
          wcscat_s(aReparseSrcTargetHint, HUGE_PATH, InnerReparsePoint);
  #if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
          wprintf(L"%s " __FUNCTION__ L" 05 %s, %s\n", m_ContainerName, aSrcPath, aReparseSrcTargetHint);
  #endif
        }

        aReparsePointBoundaryCrossed = false;
      }
      else
      {
        // Outer ReparsePoint

  #if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
        wprintf(L"%s " __FUNCTION__ L" 06 %s\n", m_ContainerName, aSrcPath);
  #endif
        // Save the current DiskSerialNumber, and only after successfull traverse make it the new SerialNumber
        DWORD DiskSerialNumber = m_CurrentSerialNumber;
        
        // See if we can cross the border. There are reasons not to cross
        int result = m_DiskSerialCache.Lookup(ReparseSrcTarget, DiskSerialNumber);
#undef STRICT_WITH_MULTIPLE_DISK_IDS
#if defined STRICT_WITH_MULTIPLE_DISK_IDS
        if (ERROR_SUCCESS == result)
#endif
        {
          // Only mark traversion if destination exists
          if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(ReparseSrcTarget))
          {
            // We need to keep track, that we crossed a outer reparsepoint boundary
            // because on return from the recursion, we have to switch back to the
            // previous ReparsePointReferencePath
            aReparsePointBoundaryCrossed = true;
            m_CurrentSerialNumber = DiskSerialNumber;
          }
          else
          {
            // The error will be reported later when it comes to link, but for now all is ok
            // we are recording a junction, which is broken, ok.
          }
        }
#if defined STRICT_WITH_MULTIPLE_DISK_IDS
        else
        {
          // Save an error, because we couldn't traverse into that outer symboliclink/junction, because 
          // we had two disks with the same id, which is crucial. Only enter here due to double disk-id problem
          //
          // Having two disks with same id is crucial, but traversing many times into a disk with the same
          // id via junction/symlink is not crucial. That's why the coding is disabled here
          switch(ReparsePointType)
          {
            case REPARSE_POINT_SYMBOLICLINK:
            {
              PathNameStatus pns(QuestionS, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE], result);
              aPathNameStatusList->push_back(pns);
            }
            break;

            case REPARSE_POINT_JUNCTION:
            {
              PathNameStatus pns(QuestionJ, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE], result);
              aPathNameStatusList->push_back(pns);
            }
            break;

            case REPARSE_POINT_MOUNTPOINT:
            {
              PathNameStatus pns(QuestionM, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE], result);
              aPathNameStatusList->push_back(pns);
            }
            break;

            default:
            {
              // unknown Reparse Point
              PathNameStatus pns(QuestionR, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE], result);
              aPathNameStatusList->push_back(pns);
            }
            break;
          }
        }
#endif
      }
    } 
    //
    // Inner or Outer ReparsePoint: End
    //




    //
    // No go for Unroll. Only unroll outer junctions, we must not follow inner junctions
    // because this would result in files found twice
    //
    if (aReparsePointBoundaryCrossed)
    {
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
      wprintf(L"%s " __FUNCTION__ L" 07 %s\n", m_ContainerName, aSrcPath);
#endif
      
      // Unrolling on circularities would drive us into an endless loop thus we have to break it
      // Create the last known good junctions and stop recursion
      if (!Circularity)
      {
        // Check if all junctions/symlink dirs should be unrolled. 
        // This switch is here because if we can avoid time consuming 
        // regular expression matches the performance will be better
        if (m_Flags & eAlwaysUnrollDir)
          DoRecursion = true;
        else
        {
          // Apply regular expression to know which junctions/symlink dirs should be unrolled
          if (MatchRegExpList(&aSrcPath[PATH_PARSE_SWITCHOFF_SIZE], m_RegExpUnrollDirList))
            DoRecursion = true;
        }

        // Only unroll reparse points if either the directory matches or all reparse
        // points should be unrolled
        if (DoRecursion)
        {
          // We are crossing a outer reparse point boundary, so save the previous 
          // ReparsePointReferencePath
          *aPreviousReparsePointReferencePath = *aCurrentReparsePointReferencePath;
          wcscpy_s(aReparsePointReferencePath, HUGE_PATH, ReparseSrcTarget);

          // Pass on the new ReparseSrcTarget as ReparsePointReferencePath to the 
          // recursion, which is operating in the outer reparse point.
          *aCurrentReparsePointReferencePath = aReparsePointReferencePath;

          // The position in SrcPath when it crosses to a outer junction is 
          // important because with it the wrapped destination to new-inner
          // junctions can be assembled.
          aPreviousBoundaryCross = aCurrentBoundaryCross;
          aCurrentBoundaryCross = wcslen(aSrcPath);

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
          wprintf(L"%s " __FUNCTION__ L" 09 %s %s\n", m_ContainerName, aSrcPath, aReparsePointReferencePath);
#endif
          aFileAttributes &= ~FILE_ATTRIBUTE_REPARSE_POINT;
        }
      }
    }
  }
  else
  {
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
    wprintf(L"%s " __FUNCTION__ L" 08 %s\n", m_ContainerName, aSrcPath);
#endif

    // Follow directories anyway
    DoRecursion = true;
  }

  return DoRecursion;
}

// This version of add is intended to store DirectoryNames only
int
FileInfoContainer::
AddDirectoryFast(
  wchar_t*	                      aName,
  _Pathes&                        a_Filenames,
  CopyStatistics*	                aStats,
  PWCHAR                          aReparseSrcTargetHint,
  DWORD                           aOriginalFileAttribute,
  _PathNameStatusList*            a_pPathNameStatusList,
  PFILE_ID_BOTH_DIR_INFORMATION   pFullDirInfo
)
{
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
  wprintf(L"%s " __FUNCTION__ L"\n", m_ContainerName);
#endif

  int RetVal = ERROR_SUCCESS;

  size_t FilenameLength = wcslen (aName) + 1;
  aStats->m_HeapAllocTime.Start();
  FileInfo*	pFileInfo = new FileInfo;
  pFileInfo->Init();
  pFileInfo->m_FileName = new wchar_t[FilenameLength];
  aStats->m_HeapAllocTime.Stop();

  wcscpy_s (pFileInfo->m_FileName, FilenameLength, aName);

  pFileInfo->m_CreationTime.ul64DateTime = pFullDirInfo->CreationTime.QuadPart;
  pFileInfo->m_LastAccessTime.ul64DateTime = pFullDirInfo->LastAccessTime.QuadPart;
  pFileInfo->m_LastWriteTime.ul64DateTime = pFullDirInfo->LastWriteTime.QuadPart;

  pFileInfo->m_Type = pFullDirInfo->FileAttributes;

  pFileInfo->m_DestPathIdx = m_CurDestPathIdx;
  pFileInfo->m_SourcePathLen = (DWORD)m_CurSourcePathLen;

  if (aReparseSrcTargetHint[0])
  {
    size_t ReparseSrcTargetHintLength = wcslen(aReparseSrcTargetHint) + 1;

    aStats->m_HeapAllocTime.Start();
    pFileInfo->m_ReparseSrcTargetHint = new wchar_t[ReparseSrcTargetHintLength];
    aStats->m_HeapAllocTime.Stop();

    wcscpy_s (pFileInfo->m_ReparseSrcTargetHint, ReparseSrcTargetHintLength, aReparseSrcTargetHint);
  }
  else
    pFileInfo->m_ReparseSrcTargetHint = NULL;

  // Save a call to ProbeReparsePoint by setting the content of TypeOfReparsePoint
  wchar_t ReparseSrcTarget[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
  wcscpy_s(ReparseSrcTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);

  int ReparsePointType = REPARSE_POINT_UNDEFINED;
  if (pFullDirInfo->FileAttributes & aOriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
  {
    ReparsePointType = ProbeReparsePoint(aName, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);

    switch (ReparsePointType)
    {
      case REPARSE_POINT_JUNCTION:
        aStats->m_JunctionsTotal++;
      break;

      case REPARSE_POINT_MOUNTPOINT:
        // With Mounpoint we receive \\?\Volume... as target, but we already have prepended \\?\ so 
        // we have to get rid of one \\?\ 
        wcscpy_s(ReparseSrcTarget, HUGE_PATH, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);

        aStats->m_MountpointsTotal++;
      break;

      case REPARSE_POINT_SYMBOLICLINK:
        aStats->m_SymlinksTotal++;
      break;

      default:
        // Unknown Reparse Points
        aStats->m_ReparsePointTotal++;
      break;
    }
  }
  else
  {
    aStats->m_DirectoryTotal++;
    if (aOriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
    {
      if (REPARSE_POINT_UNDEFINED == ReparsePointType)
      {
        ReparsePointType = ProbeReparsePoint(aName, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);

        // With Mounpoint we receive \\?\Volume... as target, but we already have prepended \\?\ so 
        // we have to get rid of one \\?\ 
        if (REPARSE_POINT_MOUNTPOINT == ReparsePointType)
          wcscpy_s(ReparseSrcTarget, HUGE_PATH, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);
      }

      switch (ReparsePointType)
      {
        case REPARSE_POINT_JUNCTION:
        {
          if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(ReparseSrcTarget))
          {
            aStats->m_JunctionsRestoreFailed++;
            aStats->m_JunctionsTotal++;
            aStats->m_DirectoryTotal--;

            // We failed on searching a junction, so record it
            PathNameStatus pns(QuestionJ, &aName[PATH_PARSE_SWITCHOFF_SIZE], GetLastError());
            a_pPathNameStatusList->push_back(pns);

            RetVal = ERROR_PATH_NOT_FOUND;
          }
        }
        break;

        case REPARSE_POINT_MOUNTPOINT:
        {
          if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(ReparseSrcTarget))
          {
            aStats->m_MountpointsRestoreFailed++;
            aStats->m_MountpointsTotal++;
            aStats->m_DirectoryTotal--;

            // We failed on searching a mountpoint, so record it
            PathNameStatus pns(QuestionJ, &aName[PATH_PARSE_SWITCHOFF_SIZE], GetLastError());
            a_pPathNameStatusList->push_back(pns);

            RetVal = ERROR_PATH_NOT_FOUND;
          }
        }
        break;

        case REPARSE_POINT_SYMBOLICLINK:
        {
// If this is commented in the regression test goes badly! Should investigate TBD
#if 0 
          // It might happen, that there are relative symbolic links
          if ( PathIsRelative(ReparseSrcTarget) )
          {
            // Resolve the symlinks
            WCHAR ReparseSrcTargetFullPath[HUGE_PATH];

            int iResult = ResolveSymboliclink(&aName[PATH_PARSE_SWITCHOFF_SIZE], ReparseSrcTarget, ReparseSrcTargetFullPath);
            if (ERROR_SUCCESS == iResult)
              PathCanonicalize(ReparseSrcTarget, ReparseSrcTargetFullPath);
          }
          // Here we should end up in any case with ReparseSrcTarget beeing a long path

          if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(ReparseSrcTarget))
          {
            aStats->m_SymlinksRestoreFailed++;
            aStats->m_SymlinksTotal++;
            aStats->m_DirectoryTotal--;

            // We failed on searching a symlink, so record it
            PathNameStatus pns(QuestionS, &aName[PATH_PARSE_SWITCHOFF_SIZE], RetVal);
            a_pPathNameStatusList->push_back(pns);

            RetVal = ERROR_PATH_NOT_FOUND;
          }

#endif
        }
        break;

        default:
          // Unknown Reparse Points
          aStats->m_ReparsePointTotal++;
        break;

      }
    }
  }

  if (ERROR_SUCCESS == RetVal)
  {
    a_Filenames.push_back (pFileInfo);
#if defined USE_VECTOR
    m_PathVector.push_back(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#else    
    /* pair< _PathMap::iterator, bool > pr = */ m_PathMap.insert(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#endif

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
    wprintf(L"%s push_back fast dir %s, %08x\n", m_ContainerName, pFileInfo->m_FileName, RetVal);
#endif
  }


  return RetVal;
}

// This version of add is intended to store DirectoryNames only
int
FileInfoContainer::
AddDirectory(
  wchar_t*	            aName,
  _Pathes&              a_Filenames,
  DWORD		              FileAttribute,
  CopyStatistics*	      aStats,
  PWCHAR                aReparseSrcTargetHint,
  DWORD                 aOriginalFileAttribute,
  _PathNameStatusList*  a_pPathNameStatusList
)
{
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
  wprintf(L"%s " __FUNCTION__ L"\n", m_ContainerName);
#endif

  int RetVal = ERROR_SUCCESS;

  HANDLE	fh = CreateFileW (aName, 
    FILE_READ_ATTRIBUTES, 
    FILE_SHARE_READ,
    NULL, 
    OPEN_EXISTING, 
    FileAttribute & FILE_ATTRIBUTE_REPARSE_POINT ? FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS : FILE_FLAG_BACKUP_SEMANTICS, 
    NULL);

  if (INVALID_HANDLE_VALUE != fh)
  {
    BY_HANDLE_FILE_INFORMATION	FileInformation;
    BOOL	r = GetFileInformationByHandle (fh, &FileInformation);
    if (r)
    {
      size_t FilenameLength = wcslen (aName) + 1;

      aStats->m_HeapAllocTime.Start();
      FileInfo*	pFileInfo = new FileInfo;
      pFileInfo->m_FileName = new wchar_t[FilenameLength];
      aStats->m_HeapAllocTime.Stop();

      wcscpy_s (pFileInfo->m_FileName, FilenameLength, aName);

      pFileInfo->m_CreationTime.dwLowDateTime = FileInformation.ftCreationTime.dwLowDateTime;
      pFileInfo->m_CreationTime.dwHighDateTime = FileInformation.ftCreationTime.dwHighDateTime;

      pFileInfo->m_LastWriteTime.dwLowDateTime = FileInformation.ftLastWriteTime.dwLowDateTime;
      pFileInfo->m_LastWriteTime.dwHighDateTime = FileInformation.ftLastWriteTime.dwHighDateTime;

      pFileInfo->m_LastAccessTime.dwLowDateTime = FileInformation.ftLastAccessTime.dwLowDateTime;
      pFileInfo->m_LastAccessTime.dwHighDateTime = FileInformation.ftLastAccessTime.dwHighDateTime;

      pFileInfo->m_Type = FileAttribute;

      pFileInfo->m_DestPathIdx = m_CurDestPathIdx;
      pFileInfo->m_SourcePathLen = (DWORD)m_CurSourcePathLen;

      if (aReparseSrcTargetHint[0])
      {
        size_t ReparseSrcTargetHintLength = wcslen(aReparseSrcTargetHint) + 1;

        aStats->m_HeapAllocTime.Start();
        pFileInfo->m_ReparseSrcTargetHint = new wchar_t[ReparseSrcTargetHintLength];
        aStats->m_HeapAllocTime.Stop();

        wcscpy_s (pFileInfo->m_ReparseSrcTargetHint, ReparseSrcTargetHintLength, aReparseSrcTargetHint);
      }
      else
        pFileInfo->m_ReparseSrcTargetHint = NULL;

      a_Filenames.push_back (pFileInfo);
#if defined USE_VECTOR
      m_PathVector.push_back(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#else    
      /* pair< _PathMap::iterator, bool > pr = */ m_PathMap.insert(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#endif

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
      wprintf(L"%s " __FUNCTION__ L"::push_back dir %s\n", m_ContainerName, pFileInfo->m_FileName);
#endif

      if (FileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
      {
        int TypeOfReparsePoint = ProbeReparsePoint(aName, NULL);
        switch(TypeOfReparsePoint)
        {
          case REPARSE_POINT_JUNCTION:
            aStats->m_JunctionsTotal++;
          break;

          case REPARSE_POINT_MOUNTPOINT:
            aStats->m_MountpointsTotal++;
          break;

          case REPARSE_POINT_SYMBOLICLINK:
            aStats->m_SymlinksTotal++;
          break;

          default:
            aStats->m_ReparsePointTotal++;
          break;
        }
      }
      else
        aStats->m_DirectoryTotal++;
    } 
    else // if (GetFileInformationByHandle (fh, &FileInformation))
    {
      // Only in rare cases this path will be executed, because if CreateFile
      // succeeds, GetFileInformationByHandle will also succeed. Anyway to be on the
      // safe side the coding is duplicated from below
      //
      // Items failed during enumeration will get counted but at the same time 
      // are counted as creation failed. This path is entered for dead junctions
      // mostly with --unroll, because a dead junction can not be opened without
      // FILE_ATTRIBUTE_REPARSE_POINT because the destination is not here.
      //
      // During --Unroll Junctions/Symbolic Links are converted to directories
      // and so the REPARSE bit is removed from the FileAttribute. But for the
      // statistics and the error handling we need to count the original items
      RetVal = GetLastError();

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
      wprintf(L"%s " __FUNCTION__ L"::GetFileInformationByHandle failed %s(%08d)\n", m_ContainerName, aName, RetVal);
#endif

      if (aOriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
      {
        int TypeOfReparsePoint = ProbeReparsePoint(aName, NULL);
        switch (TypeOfReparsePoint)
        {
          case REPARSE_POINT_JUNCTION:
          {
            aStats->m_JunctionsTotal++;
            aStats->m_JunctionsRestoreFailed++;

            // We failed on Searching a junction, so record it
            PathNameStatus pns(QuestionJ, &aName[PATH_PARSE_SWITCHOFF_SIZE], GetLastError());
            a_pPathNameStatusList->push_back(pns);
          }
          break;

          case REPARSE_POINT_MOUNTPOINT:
          {
            aStats->m_MountpointsTotal++;
            aStats->m_MountpointsRestoreFailed++;

            // We failed on Searching a junction, so record it
            PathNameStatus pns(QuestionM, &aName[PATH_PARSE_SWITCHOFF_SIZE], GetLastError());
            a_pPathNameStatusList->push_back(pns);
          }
          break;

          case REPARSE_POINT_SYMBOLICLINK:
          {
            aStats->m_SymlinksTotal++;
            aStats->m_SymlinksRestoreFailed++;

            // We failed on Searching a symlink, so record it
            PathNameStatus pns(QuestionS, &aName[PATH_PARSE_SWITCHOFF_SIZE], RetVal);
            a_pPathNameStatusList->push_back(pns);
          }
          break;

          default:
          break;
        }
      }
      else
      {
        aStats->m_DirectoryTotal++;
        aStats->m_DirectoryCreateFailed++;

        // We failed on Searching a directory, so record it
        PathNameStatus pns(QuestionD, &aName[PATH_PARSE_SWITCHOFF_SIZE], RetVal);
        a_pPathNameStatusList->push_back(pns);
      }

    }
    CloseHandle (fh);
  } 
  else // if (INVALID_HANDLE_VALUE != fh)
  {
#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
    wprintf(L"%s " __FUNCTION__ L"::CreateFileW failed %s(%08d)\n", m_ContainerName, aName, RetVal);
#endif
    RetVal = GetLastError();

    // Items failed during enumeration will get counted but at the same time 
    // are counted as creation failed. This path is entered for dead junctions
    // mostly with --unroll, because a dead junction can not be opened without
    // FILE_ATTRIBUTE_REPARSE_POINT because the destination is not here.
    //
    // During --Unroll Junctions/Symbolic Links are converted to directories
    // and so the REPARSE bit is removed from the FileAttribute. But for the
    // statistics and the error handling we need to count the original items
    if (aOriginalFileAttribute & FILE_ATTRIBUTE_REPARSE_POINT)
    {
      int TypeOfReparsePoint = ProbeReparsePoint(aName, NULL);
      switch (TypeOfReparsePoint)
      {
        case REPARSE_POINT_JUNCTION:
        {
          aStats->m_JunctionsTotal++;
          aStats->m_JunctionsRestoreFailed++;

          // We failed on Searching a junction, so record it
          PathNameStatus pns(QuestionJ, &aName[PATH_PARSE_SWITCHOFF_SIZE], RetVal);
          a_pPathNameStatusList->push_back(pns);
        }
        break;

        case REPARSE_POINT_MOUNTPOINT:
        {
          aStats->m_MountpointsTotal++;
          aStats->m_MountpointsRestoreFailed++;

          // We failed on Searching a junction, so record it
          PathNameStatus pns(QuestionM, &aName[PATH_PARSE_SWITCHOFF_SIZE], RetVal);
          a_pPathNameStatusList->push_back(pns);
        }
        break;

        case REPARSE_POINT_SYMBOLICLINK:
        {
          aStats->m_SymlinksTotal++;
          aStats->m_SymlinksRestoreFailed++;

          // We failed on Searching a symlink, so record it
          PathNameStatus pns(QuestionS, &aName[PATH_PARSE_SWITCHOFF_SIZE], RetVal);
          a_pPathNameStatusList->push_back(pns);
        }
        break;

        default:
        break;
      }
    }
    else
    {
      aStats->m_DirectoryTotal++;
      aStats->m_DirectoryCreateFailed++;

      // We failed on Searching a directory, so record it
      PathNameStatus pns(QuestionD, &aName[PATH_PARSE_SWITCHOFF_SIZE], RetVal);
      a_pPathNameStatusList->push_back(pns);
    }
  }

  return RetVal;
}

int
FileInfoContainer::
AddFileFast(
  wchar_t*	                    aName,
  DWORD	                        nType,
  CopyStatistics*	              aStats,
  PWCHAR	                      aReparseSrcTargetHint,
  bool                          aUseReparseFileIndex,
  PFILE_ID_BOTH_DIR_INFORMATION pFullDirInfo,
  int                           a_ReparsePointType
)
{
  int RetVal = ERROR_SUCCESS;

  // This is currently for dupemerge, which can limit the size of processed files
  if (pFullDirInfo->EndOfFile.QuadPart < m_MinSize)
    return RetVal;

  if (pFullDirInfo->EndOfFile.QuadPart > m_MaxSize)
    return RetVal;

  size_t FilenameLength = wcslen (aName) + 1;

  aStats->m_HeapAllocTime.Start();
  FileInfo*	pFileInfo = new FileInfo;
  pFileInfo->m_FileName = new wchar_t[FilenameLength];
  aStats->m_HeapAllocTime.Stop();

  wcscpy_s (pFileInfo->m_FileName, FilenameLength, aName);

  // Take the specified attribute, but merge with the compression status
#if defined DEBUG_ARCHIVE_COMPLIANT
  pFileInfo->m_Type = nType | (pFullDirInfo->FileAttributes & ( FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE ));
#else
  pFileInfo->m_Type = nType | (pFullDirInfo->FileAttributes & ( FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ));
#endif
  pFileInfo->m_DestPathIdx = m_CurDestPathIdx;
  pFileInfo->m_SourcePathLen = (DWORD)m_CurSourcePathLen;

  // With Vista & W7 we also can have symbolic file links which are 
  // also reparse points, so we have to save the time for those items too
  if (nType & FILE_ATTRIBUTE_REPARSE_POINT) 
  {
    // Symbolic links directory or inner Symbolic Link Files
    pFileInfo->m_CreationTime.ul64DateTime = pFullDirInfo->CreationTime.QuadPart;
    pFileInfo->m_LastAccessTime.ul64DateTime = pFullDirInfo->LastAccessTime.QuadPart;

    // Save the ReparseTarget so that we can later on assemble the destination
    if (aReparseSrcTargetHint[0])
    {
      size_t ReparseSrcTargetHintLength = wcslen(aReparseSrcTargetHint) + 1;

      aStats->m_HeapAllocTime.Start();
      pFileInfo->m_ReparseSrcTargetHint = new wchar_t[ReparseSrcTargetHintLength];
      aStats->m_HeapAllocTime.Stop();

      wcscpy_s (pFileInfo->m_ReparseSrcTargetHint, ReparseSrcTargetHintLength, aReparseSrcTargetHint);
    }
    else
      pFileInfo->m_ReparseSrcTargetHint = NULL;

    m_Filenames.push_back (pFileInfo);

    // But since DeloreanCopy & SmartMirror at least LastWriteTime 
    // must be also saved for files
    pFileInfo->m_LastWriteTime.ul64DateTime = pFullDirInfo->LastWriteTime.QuadPart;

#if defined USE_VECTOR
    m_PathVector.push_back(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#else    
    /* pair< _PathMap::iterator, bool > pr = */ m_PathMap.insert(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#endif

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
    wprintf(L"%s push_back reparse %s\n", m_ContainerName, pFileInfo->m_FileName);
#endif

    switch (a_ReparsePointType)
    {
      case REPARSE_POINT_SYMBOLICLINK:
        aStats->m_SymlinksTotal++;
        aStats->m_SymlinksSelected++;
      break;

      case REPARSE_POINT_FAIL:
        aStats->m_ReparsePointTotal++;
        aStats->m_ReparsePointSelected++;
      break;
    }
  } 
  else // if (nType & FILE_ATTRIBUTE_REPARSE_POINT)
  {
    pFileInfo->m_DiskIndex = m_CurrentSerialNumber;

    if (aUseReparseFileIndex)
    {
      // Maybe outer Symbolic link Files, which can not be decided yet
      pFileInfo->m_Type = FILE_ATTRIBUTE_DELAYED_REPARSE_POINT;

      // With the fast query, we always get the Fileinfo from the reparsepoint
      // but in that situation, we need the index of the file, so we have to open 
      // the file 
      HANDLE	fh = CreateFileW (aName, 
        m_Flags & eBackupMode ? GENERIC_READ | READ_CONTROL | ACCESS_SYSTEM_SECURITY : FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        m_Flags & eBackupMode ? FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL, 
        NULL
      );
      if (INVALID_HANDLE_VALUE != fh)
      {
        BY_HANDLE_FILE_INFORMATION	FileInformation;
        BOOL	r = GetFileInformationByHandle (fh, &FileInformation);
        if (r)
        {
          // Symbolic links directory or inner Symbolic Link Files
          pFileInfo->m_LastWriteTime.dwLowDateTime = FileInformation.ftLastWriteTime.dwLowDateTime;
          pFileInfo->m_LastWriteTime.dwHighDateTime = FileInformation.ftLastWriteTime.dwHighDateTime;

          pFileInfo->m_FileIndex.ul32l = FileInformation.nFileIndexLow;
          pFileInfo->m_FileIndex.ul32h = FileInformation.nFileIndexHigh;

          pFileInfo->m_FileSize.ul32h = FileInformation.nFileSizeHigh;
          pFileInfo->m_FileSize.ul32l = FileInformation.nFileSizeLow;
        }
        else
        {
          // Files failed during enumeration will get counted but at the same 
          // time are counted as copy failed
          aStats->m_FilesTotal++;
          aStats->m_FilesCopyFailed++;

          aStats->m_BytesCopyFailed += pFullDirInfo->EndOfFile.QuadPart;
          aStats->m_BytesTotal += pFullDirInfo->EndOfFile.QuadPart;

          // PathNameStatus in added outside this method upon return value
          RetVal = GetLastError();
        }

        CloseHandle(fh);
      }
      else
      {
        // Files failed during enumeration will get counted but at the same 
        // time are counted as copy failed
        aStats->m_FilesTotal++;
        aStats->m_FilesCopyFailed++;

        aStats->m_BytesCopyFailed += pFullDirInfo->EndOfFile.QuadPart;
        aStats->m_BytesTotal += pFullDirInfo->EndOfFile.QuadPart;

        // PathNameStatus in added outside this method upon return value
        RetVal = GetLastError();
      }
    }
    else // if (aUseReparseFileIndex)
    {
      // files or hardlinks
      pFileInfo->m_FileIndex.ul64 = pFullDirInfo->FileId.QuadPart;
      pFileInfo->m_FileSize.ul64 = pFullDirInfo->EndOfFile.QuadPart;

      // But with DeloreanCopy & SmartMirror at least LastWriteTime 
      // must be also saved for files
      pFileInfo->m_LastWriteTime.ul64DateTime = pFullDirInfo->LastWriteTime.QuadPart;

      aStats->m_FilesTotal++;
      aStats->m_FilesSelected++;
      aStats->m_BytesTotal += pFileInfo->m_FileSize.ul64;
    } // if (aUseReparseFileIndex)

    m_Filenames.push_back (pFileInfo);
#if defined USE_VECTOR
    m_PathVector.push_back(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#else    
    /*/ pair< _PathMap::iterator, bool > pr = /*/ m_PathMap.insert(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#endif

// #if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
#if 0 // DEBUG_DEFINES
    fwprintf(m_OutputFile, L"%s push_back file (%08x%08x)%s %d \n", m_ContainerName, pFileInfo->m_FileIndex.ul32h, pFileInfo->m_FileIndex.ul32l,  pFileInfo->m_FileName,  pFileInfo->m_DestPathIdx);
#endif
  } // if (nType & FILE_ATTRIBUTE_REPARSE_POINT)

  return RetVal;
}

// This version of add is intended to store FileNames only
int
FileInfoContainer::
AddFile(
  wchar_t*	        aName,
  DWORD	            nType,
  int               aRefCount,
  CopyStatistics*   aStats,
  PWCHAR	          aReparseSrcTargetHint,
  WIN32_FIND_DATA*  pwfind,
  int               a_ReparsePointType,
  bool              aUseReparseFileIndex
)
{
  int RetVal = ERROR_SUCCESS;

  DWORD FlagsAndAttributes;
  if (FILE_ATTRIBUTE_REPARSE_POINT == nType)
    FlagsAndAttributes = FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
  else
    FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  
  if (m_Flags & eBackupMode)
    FlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;

  HANDLE	fh = CreateFileW (
    aName, 
    m_Flags & eBackupMode ? GENERIC_READ | READ_CONTROL | ACCESS_SYSTEM_SECURITY | FILE_READ_EA : FILE_READ_ATTRIBUTES | FILE_READ_EA,
    FILE_SHARE_READ, 
    NULL, 
    OPEN_EXISTING, 
    FlagsAndAttributes,
    NULL
  );

  if (INVALID_HANDLE_VALUE != fh)
  {
    size_t FilenameLength = wcslen (aName) + 1;

    aStats->m_HeapAllocTime.Start();
    FileInfo*	pFileInfo = new FileInfo;
    pFileInfo->m_FileName = new wchar_t[FilenameLength];
    aStats->m_HeapAllocTime.Stop();

    wcscpy_s (pFileInfo->m_FileName, FilenameLength, aName);

    pFileInfo->m_Type = nType;
    pFileInfo->m_DestPathIdx = m_CurDestPathIdx;
    pFileInfo->m_SourcePathLen = (DWORD)m_CurSourcePathLen;

    // Get the reference count
    BY_HANDLE_FILE_INFORMATION	FileInformation;
    BOOL	r = GetFileInformationByHandle (fh, &FileInformation);
    if (r)
    {
      // This is currently for dupemerge, which can limit the size of processed files
      // 
      UL64 FileSize;
      FileSize.ul32l = FileInformation.nFileSizeLow;
      FileSize.ul32h = FileInformation.nFileSizeHigh;
      bool bAddItem = true;
      if (FileSize.l64 < m_MinSize)
        bAddItem = false;

      if (FileSize.l64 > m_MaxSize)
        bAddItem = false;

      if (!bAddItem)
      {
        aStats->m_HeapDeletionTime.Start();
        delete[] pFileInfo->m_FileName;
        delete pFileInfo;
        aStats->m_HeapDeletionTime.Stop();

        CloseHandle(fh);
        return RetVal;
      }
      // Dupemerge off

      // Merge the compression status into the attributes
#if defined DEBUG_ARCHIVE_COMPLIANT
      pFileInfo->m_Type |= FileInformation.dwFileAttributes & ( FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE ) ;
#else
      pFileInfo->m_Type |= FileInformation.dwFileAttributes & ( FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM) ;
#endif

      // With Vista & W7 we also can have symbolic file links which are 
      // also reparse points, so we have to save the time for those items too
      if (nType & FILE_ATTRIBUTE_REPARSE_POINT)
      {
        // Symbolic links directory or inner Symbolic Link Files
        pFileInfo->m_CreationTime.dwLowDateTime = FileInformation.ftCreationTime.dwLowDateTime;
        pFileInfo->m_CreationTime.dwHighDateTime = FileInformation.ftCreationTime.dwHighDateTime;

        pFileInfo->m_LastAccessTime.dwLowDateTime = FileInformation.ftLastAccessTime.dwLowDateTime;
        pFileInfo->m_LastAccessTime.dwHighDateTime = FileInformation.ftLastAccessTime.dwHighDateTime;

        // Save the ReparseTarget so that we can later on assemble the destination
        if (aReparseSrcTargetHint[0])
        {
          size_t ReparseSrcTargetHintLength = wcslen(aReparseSrcTargetHint) + 1;

          aStats->m_HeapAllocTime.Start();
          pFileInfo->m_ReparseSrcTargetHint = new wchar_t[ReparseSrcTargetHintLength];
          aStats->m_HeapAllocTime.Stop();

          wcscpy_s(pFileInfo->m_ReparseSrcTargetHint, ReparseSrcTargetHintLength, aReparseSrcTargetHint);
        }
        else
          pFileInfo->m_ReparseSrcTargetHint = NULL;

        m_Filenames.push_back (pFileInfo);

#if defined USE_VECTOR
        m_PathVector.push_back(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#else    
        /* pair< _PathMap::iterator, bool > pr = */ m_PathMap.insert(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#endif

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
        wprintf(L"%s push_back reparse %s\n", m_ContainerName, pFileInfo->m_FileName);
#endif

        switch (a_ReparsePointType)
        {
          case REPARSE_POINT_SYMBOLICLINK:
            aStats->m_SymlinksTotal++;
            aStats->m_SymlinksSelected++;
          break;

          case REPARSE_POINT_FAIL:
            aStats->m_ReparsePointTotal++;
            aStats->m_ReparsePointSelected++;
          break;
        }
      } 
      else // if (nType & FILE_ATTRIBUTE_REPARSE_POINT)
      {
        // These are either files, hardlinks or delayed outer symbolic links
        // Record all files, which have a RefCount greater than specified.
        // This means some could also search for all files, which have
        // a refcount greater than e.g 4. Basically this just makes this code
        // more generic
        if (FileInformation.nNumberOfLinks >= aRefCount)
        {
          pFileInfo->m_FileIndex.ul32l = FileInformation.nFileIndexLow;
          pFileInfo->m_FileIndex.ul32h = FileInformation.nFileIndexHigh;
          pFileInfo->m_DiskIndex = m_CurrentSerialNumber;

          pFileInfo->m_RefCount = FileInformation.nNumberOfLinks;

          pFileInfo->m_FileSize.ul32h = FileInformation.nFileSizeHigh;
          pFileInfo->m_FileSize.ul32l = FileInformation.nFileSizeLow;

          if (aUseReparseFileIndex)
          {
            // Maybe outer Symbolic link Files, which can not be decided yet
            pFileInfo->m_Type = FILE_ATTRIBUTE_DELAYED_REPARSE_POINT;
          }
          else
          {
            // files or hardlinks
            aStats->m_FilesTotal++;
            aStats->m_FilesSelected++;
            aStats->m_BytesTotal += pFileInfo->m_FileSize.ul64;
          }

          m_Filenames.push_back (pFileInfo);
#if defined USE_VECTOR
          m_PathVector.push_back(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#else    
          /*/ pair< _PathMap::iterator, bool > pr = /*/ m_PathMap.insert(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#endif

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
           wprintf(L"%s push_back file (%08x%08x)%s \n", m_ContainerName, pFileInfo->m_FileIndex.ul32h, pFileInfo->m_FileIndex.ul32l,  pFileInfo->m_FileName);
#endif
        }
      }

      // But since DeloreanCopy & SmartMirror at least LastWriteTime 
      // must be also saved for files
      pFileInfo->m_LastWriteTime.dwLowDateTime = FileInformation.ftLastWriteTime.dwLowDateTime;
      pFileInfo->m_LastWriteTime.dwHighDateTime = FileInformation.ftLastWriteTime.dwHighDateTime;
    }
    else
    {
      // Files failed during enumeration will get counted but at the same 
      // time are counted as copy failed
      aStats->m_FilesTotal++;
      aStats->m_FilesCopyFailed++;

      if (pwfind)
      {
        UL64 FileSize;
        FileSize.ul32h = pwfind->nFileSizeHigh;
        FileSize.ul32l = pwfind->nFileSizeLow;

        aStats->m_BytesCopyFailed += FileSize.ul64;
        aStats->m_BytesTotal += FileSize.ul64;
      }

      // PathNameStatus in added outside this method upon return value
      RetVal = GetLastError();

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
      wprintf(L"%s open_file handleinfo failed %s(%08d)\n", m_ContainerName, pFileInfo->m_FileName, RetVal);
#endif
    }

    CloseHandle (fh);
  }
  else // if (INVALID_HANDLE_VALUE != fh)
  {
    // Files failed during enumeration will get counted but at the same 
    // time are counted as copy failed
    aStats->m_FilesTotal++;
    aStats->m_FilesCopyFailed++;

    if (pwfind)
    {
      UL64 FileSize;
      FileSize.ul32h = pwfind->nFileSizeHigh;
      FileSize.ul32l = pwfind->nFileSizeLow;

      aStats->m_BytesCopyFailed += FileSize.ul64;
      aStats->m_BytesTotal += FileSize.ul64;
    }

    // PathNameStatus in added outside this method upon return value
    RetVal = GetLastError();

#if defined FIND_HARDLINK_DEBUG // DEBUG_DEFINES
    wprintf(L"%s open_file failed %s(%08d)\n", m_ContainerName, aName, RetVal);
#endif
  }

  return RetVal;
}
#if defined _DEBUG
int
FileInfoContainer::
AddFile(
  wchar_t*	        aName,
  DWORD	            nType,
  ULONG             aFileIndexHigh,
  ULONG             aFileIndexLow,
  ULONG             aDiskIndex,
  int               aDestPathIdx
)
{
  FileInfo*	pFileInfo = new FileInfo;

  size_t FilenameLength = wcslen (aName) + 1;
  pFileInfo->m_FileName = new wchar_t[FilenameLength];
  wcscpy_s (pFileInfo->m_FileName, FilenameLength, aName);

  pFileInfo->m_Type = nType;
  pFileInfo->m_DestPathIdx = m_CurDestPathIdx;
  pFileInfo->m_SourcePathLen = (DWORD)m_CurSourcePathLen;

  pFileInfo->m_FileIndex.ul32l = aFileIndexLow;
  pFileInfo->m_FileIndex.ul32h = aFileIndexHigh;
  pFileInfo->m_DiskIndex = aDiskIndex;

  pFileInfo->m_RefCount = 2;

  pFileInfo->m_DestPathIdx = aDestPathIdx;

  pFileInfo->m_FileSize.ul32h = 42;
  pFileInfo->m_FileSize.ul32l = 42;

  m_Filenames.push_back (pFileInfo);

  return 0;
}
#endif

size_t
FileInfoContainer::
Load(
  FILE*    aSourceFile,
  int      aMode
)
{
  wchar_t Filename[HUGE_PATH];

  // Read the files & directories
  int FileInfoSize = 0;
  fwscanf_s(aSourceFile, L"%x\n", &FileInfoSize);
  while (!feof(aSourceFile) && FileInfoSize-- > 0)
  {
    FileInfo*	pFileInfo = new FileInfo;
    int r = fwscanf_s (aSourceFile, L"%[^\"]\"%x,%x,%x,%x,%x\n", 
      Filename,
      HUGE_PATH,
      &pFileInfo->m_Type,
      &pFileInfo->m_SourcePathLen,
      &pFileInfo->m_DestPathIdx,
      &pFileInfo->m_LastWriteTime.dwHighDateTime,
      &pFileInfo->m_LastWriteTime.dwLowDateTime
    );

    size_t FileNameLength = wcslen (Filename) + 1;
    pFileInfo->m_FileName = new wchar_t[FileNameLength];
    wcscpy_s (pFileInfo->m_FileName, FileNameLength, Filename);

    if (pFileInfo->IsDirectory() || pFileInfo->IsReparsePoint())
    {
      wchar_t ReparseSrcTargetHint[HUGE_PATH];
      r = fwscanf_s (aSourceFile, L"%[^\"]\"%x,%x,%x,%x\n", 
        ReparseSrcTargetHint,
        HUGE_PATH,
        &pFileInfo->m_CreationTime.dwHighDateTime,
        &pFileInfo->m_CreationTime.dwLowDateTime,
        &pFileInfo->m_LastAccessTime.dwHighDateTime,
        &pFileInfo->m_LastAccessTime.dwLowDateTime
      );

      size_t ReparseSrcTargetHintLength = wcslen(ReparseSrcTargetHint) + 1;
      if (wcscmp(ReparseSrcTargetHint, NULL_STRING))
      {
        pFileInfo->m_ReparseSrcTargetHint = new wchar_t[ReparseSrcTargetHintLength];
        wcscpy_s (pFileInfo->m_ReparseSrcTargetHint, ReparseSrcTargetHintLength, ReparseSrcTargetHint);
      }
      else
        pFileInfo->m_ReparseSrcTargetHint = NULL;
    }
    else
    {
      if (eDupeMergePersistance == aMode)
      {
        r = fwscanf_s(aSourceFile, L"%I64x,%I64x,%x,%x,%I64x,%I64x\n", 
          &pFileInfo->m_FileIndex.ul64,
          &pFileInfo->m_FileSize.ul64,
          &pFileInfo->m_DiskIndex,
          &pFileInfo->m_RefCount,
          &pFileInfo->m_Hash64_1,
          &pFileInfo->m_Hash64_2
        );
      }
      else
      {
        r = fwscanf_s(aSourceFile, L"%I64x,%I64x,%x,%x\n",
          &pFileInfo->m_FileIndex.ul64,
          &pFileInfo->m_FileSize.ul64,
          &pFileInfo->m_DiskIndex,
          &pFileInfo->m_RefCount
        );
      }
    }

    m_Filenames.push_back (pFileInfo);
#if defined USE_VECTOR
    m_PathVector.push_back(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#else
    m_PathMap.insert(_PathMap_Pair(pFileInfo->m_FileName, pFileInfo));
#endif

  }

  m_AnchorPathCache.Load(aSourceFile);

  // Load SpliceDirList
  LoadStringList(aSourceFile, m_SpliceDirList);
  CompileRegExpList(&m_SpliceDirList, m_RegExpSpliceDirList);

  // Save DiskSerialCache needed due to BackupMode
  fwscanf_s(aSourceFile, L"%x\n", &m_CurrentSerialNumber);
  m_DiskSerialCache.Load(aSourceFile);

  // Load misc data
  fwscanf_s(aSourceFile, L"%x,%x\n", &m_Flags, &m_Flags2);

  return 1;
}

size_t
FileInfoContainer::
LoadStringList(
  FILE*         aFile,
  _StringList&  a_StringList
)
{
  // Load Destpath
  int DestPathSize = 0;
  wchar_t Filename[HUGE_PATH];

  fwscanf_s(aFile, L"%x\n", &DestPathSize);
  while (!feof(aFile) && DestPathSize-- > 0)
  {
    int r = fwscanf_s (aFile, L"%[^\"]\"\n", Filename, HUGE_PATH);
    a_StringList.push_back(Filename);
  }

  return ERROR_SUCCESS;
}

size_t
FileInfoContainer::
Save(
  FILE*    aDestFile,
  int      aMode
)
{
  // Save all filenames
  fwprintf(aDestFile, L"%zx\n", m_Filenames.size()); 

  for (auto PathIter : m_Filenames)
  {
    FileInfo*	pFileInfo = PathIter;
    
    // Save properties common to all type of items
    fwprintf(aDestFile, L"%s\"%x,%x,%x,%x,%x\n", 
      pFileInfo->m_FileName,
      pFileInfo->m_Type,
      pFileInfo->m_SourcePathLen,
      pFileInfo->m_DestPathIdx,
      pFileInfo->m_LastWriteTime.dwHighDateTime,
      pFileInfo->m_LastWriteTime.dwLowDateTime
    );

    if (pFileInfo->IsDirectory() || pFileInfo->IsReparsePoint() ) 
    {
      // Save directory properties
      fwprintf (aDestFile, L"%s\"%x,%x,%x,%x\n", 
        pFileInfo->m_ReparseSrcTargetHint ? pFileInfo->m_ReparseSrcTargetHint : NULL_STRING,
        pFileInfo->m_CreationTime.dwHighDateTime,
        pFileInfo->m_CreationTime.dwLowDateTime,
        pFileInfo->m_LastAccessTime.dwHighDateTime,
        pFileInfo->m_LastAccessTime.dwLowDateTime
      );
    }
    else 
    {
      // Save File properties
      fwprintf (aDestFile, L"%I64x,%I64x,%x,%x", 
        pFileInfo->m_FileIndex.ul64,
        pFileInfo->m_FileSize.ul64,
        pFileInfo->m_DiskIndex,
        pFileInfo->m_RefCount
      );

      if (eDupeMergePersistance == aMode)
      {
        fwprintf (aDestFile, L",%I64x,%I64x\n", 
          pFileInfo->m_Hash64_1,
          pFileInfo->m_Hash64_2
        );
      }
      else
      {
        fwprintf (aDestFile, L"\n");
      }
    }
  }

  m_AnchorPathCache.Save(aDestFile);

  // Save all SpliceDir RegExps
  SaveStringList(aDestFile, m_SpliceDirList);

  // Save DiskSerialCache needed due to BackupMode
  fwprintf(aDestFile, L"%x\n", m_CurrentSerialNumber); 
  m_DiskSerialCache.Save(aDestFile);

  // Save misc data
  fwprintf(aDestFile, L"%x,%x\n", m_Flags, m_Flags2); 

  return 1;
}

size_t
FileInfoContainer::
SaveStringList(
  FILE*         aFile,
  _StringList&  a_StringList
)
{
  fwprintf(aFile, L"%zx\n", a_StringList.size()); 
  for (_StringListIterator iter = a_StringList.begin(); iter != a_StringList.end(); ++iter)
    fwprintf (aFile, L"%s\"\n", iter->c_str());

  return ERROR_SUCCESS;
}

FileInfo::
FileInfo()
{
  Init();
}

//
// returns true if a_pFileInfo.date is older or the a_pFileInfo.size size is different than this
// used e.g for /update option, which only copies if newer or changed size
bool
FileInfo::
IsOlder(
  FileInfo*	a_pFileInfo
)
{
  bool bResult = false;
  switch(m_Type & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NORMAL) )
  {
    // For normal files
    case FILE_ATTRIBUTE_NORMAL:
    {
      // Smaller than == older than
      //
      // if a_pFileInfo is older than this
      if (a_pFileInfo->m_LastWriteTime.ul64DateTime < m_LastWriteTime.ul64DateTime) 
      {
        bResult = true;
      }
      else
      {
        if (a_pFileInfo->m_FileSize.ul64 != m_FileSize.ul64)
          // Different size, forget it, they can't be the same
          bResult = true;
      }
    }
    break;

    // For Junctions
    case FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT:

    // For Directories
    case FILE_ATTRIBUTE_DIRECTORY:

    // For Symbolic Link Files
    case FILE_ATTRIBUTE_REPARSE_POINT:
    {
      if (a_pFileInfo->m_LastWriteTime.ul64DateTime < m_LastWriteTime.ul64DateTime) 
        bResult = true;
    }
    break;
  }

  return bResult;
}

//
// returns true if a_pFileInfo.date is different or the a_pFileInfo.size size is different than this
// used e.g for /mirror option, which only copies if date is different or changed size
//
bool
FileInfo::
IsDifferent(
  FileInfo*	a_pFileInfo,
  __int64   a_DateTimeTolerance
)
{
  bool bResult = false;

  // Are the types different? Did one e.g change a file into a directory
  if (a_pFileInfo->m_Type != m_Type)
    return true;

  switch(m_Type & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NORMAL) )
  {
    // For normal files
    case FILE_ATTRIBUTE_NORMAL:
    {
      // Smaller than == older than
      //
      // if a_pFileInfo is older than this
      if (_abs64(a_pFileInfo->m_LastWriteTime.l64DateTime - m_LastWriteTime.l64DateTime) > a_DateTimeTolerance) 
      {
        bResult = true;
      }
      else
      {
        if (a_pFileInfo->m_FileSize.ul64 != m_FileSize.ul64)
          // Different size, forget it, they can't be the same
          bResult = true;
      }
    }
    break;

    // For Junctions
    case FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT:

    // For Directories
    case FILE_ATTRIBUTE_DIRECTORY:

    // For Symbolic Link Files
    case FILE_ATTRIBUTE_REPARSE_POINT:
    {
      if (_abs64(a_pFileInfo->m_LastWriteTime.l64DateTime - m_LastWriteTime.l64DateTime) > a_DateTimeTolerance) 
        bResult = true;
    }
    break;
  }

  return bResult;
}


void
FileInfo::
ReplaceSourcePath(
  const wchar_t*	aNewPath,
  size_t          aNewPathLen
)
{
  size_t OldLength = wcslen(m_FileName);
  if (aNewPathLen > m_SourcePathLen)
  {
    // We have to realloc
    size_t FilenameLength = OldLength - m_SourcePathLen + aNewPathLen + 1;
    wchar_t* pNewFilename = new wchar_t[FilenameLength];
    wcscpy_s(pNewFilename, FilenameLength, aNewPath);
    CopyMemory(&pNewFilename[aNewPathLen], &m_FileName[m_SourcePathLen], (OldLength - m_SourcePathLen + 1) * sizeof(wchar_t));

    delete [] m_FileName;

    m_FileName = pNewFilename;
  }
  else
  {
    // The new string fits into the already allocated space
    MoveMemory(&m_FileName[aNewPathLen], &m_FileName[m_SourcePathLen], (OldLength - m_SourcePathLen + 1) * sizeof(wchar_t));
    CopyMemory(m_FileName, aNewPath, aNewPathLen * sizeof(wchar_t));
  }

  m_SourcePathLen = (DWORD)aNewPathLen;
}


int
FileInfoContainer::
Dump(
     _Pathes::iterator&	aBegin,
     _Pathes::iterator&	aEnd
)
{
  _Pathes::iterator	iter;
  for (iter = aBegin; iter != aEnd; ++iter)
  {
    FileInfo*	pF = *iter;

#if !defined LSE_DEBUG
    if (!pF->IsDirectory())
      fwprintf(m_OutputFile, L"%s: %ld \n", &pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], pF->m_RefCount);
//    wprintf(L"%s\"%ld\"%ld\"%0I64d\n", &pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], pF->m_RefCount, pF->m_DestPathIdx, pF->m_FileIndex);
#else
#if 0
    fwprintf (m_OutputFile, L"%02x, %02x, %08x,% 08x, %s\n", 
      pF->m_DestPathIdx,
      pF->m_DiskIndex,
      pF->m_FileIndex.ul32h, pF->m_FileIndex.ul32l,
      &pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE]
    );

#else
#if 0
    HTRACE(L"%02d %s: %ld \n", 
      pF->m_DestPathIdx,
      &pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], 
      pF->m_RefCount
    );
#endif

    HTRACE(L"%08x %08x %02d:%s %I64d \n", 
      pF->m_FileIndex.ul32h, pF->m_FileIndex.ul32l,
      pF->m_RefCount,
      &pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], 
      pF->m_FileSize
    );
#endif
#endif
  }

#if !defined LSE_DEBUG
  fwprintf(m_OutputFile, L"\n");
#else
  HTRACE(L"\n");
#endif
  return 1;
}

int
FileInfoContainer::
FillWithRefCount(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd
)
{
  BY_HANDLE_FILE_INFORMATION	FileInformation;
  HANDLE	fh = CreateFileW ((*aBegin)->m_FileName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
    FILE_ATTRIBUTE_NORMAL, 
    NULL);

  if (INVALID_HANDLE_VALUE != fh)
  {
    BOOL	r = GetFileInformationByHandle (fh, &FileInformation);
    if (r)
    {
      for (_Pathes::iterator filliter = aBegin; filliter != aEnd; ++filliter)
        (*filliter)->m_RefCount = FileInformation.nNumberOfLinks;
    }
    CloseHandle(fh);
  }

  return 1;
}

int
FileInfoContainer::
PrepareRefcounts(
)
{
  CopyStatistics	aStats;
  Prepare(FileInfoContainer::eSmartClone, &aStats);

  if (m_HardlinkBegin == m_FATFilenameBegin)
    return 0;

  _Pathes::iterator	iter = m_HardlinkBegin;
  _Pathes::iterator	end = m_FATFilenameBegin;

  _Pathes::iterator	last = iter;
  while (++iter != end)
  {
    FileInfo* pF = *iter;
    if (pF->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64 || pF->m_DiskIndex != (*last)->m_DiskIndex )
    {
      FillWithRefCount(last, iter);
      last = iter;
    }
  }

  FillWithRefCount(last, iter);

  m_FATFilenameBegin = partition (m_HardlinkBegin, m_DirectoryBegin, IsHardlink());

  return 1;
}

int
FileInfoContainer::
FindDupes(
  int		saturated,
  int		aPrintSizePos
)
{
  sort (m_HardlinkBegin, m_FATFilenameBegin, FileIndexCompare());

  // Go through the files which have one twin
  _Pathes::iterator	iter = m_HardlinkBegin;
  _Pathes::iterator	end = m_FATFilenameBegin;

  bool							equal = false;
  int								dupegroups = 0;
  if (iter == end)
    return 0;
  _Pathes::iterator	last = iter;
  while (++iter != end)
  {
    if ((*iter)->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64 || (*iter)->m_DiskIndex != (*last)->m_DiskIndex )
    {
      if (equal)
      {
        if ((distance (last, iter) == (*last)->m_RefCount) ^ saturated)
        {
          sort(last, iter, FilenameSorterAscending());
          Dump(last, iter);
          dupegroups++;
        }
      }

      last = iter;
      equal = false;
    }
    else
    {
      equal = true;
    }
  }

  if (equal)
  {
    if ((distance (last, iter) == (*last)->m_RefCount) ^ saturated)
    {
      // TBD remove Dump() from release build
      sort(last, iter, FilenameSorterAscending());
      Dump (last, iter);
      dupegroups++;
    }
  }

  return dupegroups;
}

int
FileInfoContainer::
FindSingleUnsaturated(
  int		aPrintSizePos
)
{
  int								SingleUnsaturated = 0;

  _Pathes::iterator	iter = m_HardlinkBegin;
  _Pathes::iterator	end = m_FATFilenameBegin;

  if (iter == end)
    return 0;

  int	same = 1;
  _Pathes::iterator	last = iter;
  while (++iter != end)
  {
    if (((*iter)->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64) || 
      (*iter)->m_DiskIndex != (*last)->m_DiskIndex &&
      ((*last)->m_RefCount > 1))
    {
      if (1 == same)
      {
        Dump (last, iter);
        SingleUnsaturated++;
      }

      same = 1;
    }
    else if (same)
      same++;

    last = iter;
  } 
  if (1 == same)
  {
    Dump (last, iter);
    SingleUnsaturated++;
  }

  return SingleUnsaturated;
}

DWORD CALLBACK CopyProgressRoutine(
   LARGE_INTEGER TotalFileSize,
   LARGE_INTEGER TotalBytesTransferred,
   LARGE_INTEGER StreamSize,
   LARGE_INTEGER StreamBytesTransferred,
   DWORD dwStreamNumber,
   DWORD dwCallbackReason,
   HANDLE hSourceFile,
   HANDLE hDestinationFile,
   LPVOID lpData
)
{
  AsyncContext*   apContext = (AsyncContext*)lpData;
  if (apContext)
  {
    int r = apContext->Add2SnapShot(TotalBytesTransferred.QuadPart, TotalBytesTransferred.QuadPart, 0);
    if (ERROR_REQUEST_ABORTED == r)
      return PROGRESS_CANCEL;
    else
      return PROGRESS_CONTINUE;
  }
  else
    return PROGRESS_CONTINUE;
}

int
FileInfoContainer::
CopyHardlinks(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  CopyStatistics*		        pStats,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext,
  CopyReparsePointFlags     aFlags
)
{
  // check for empty set
  if (aBegin == aEnd)
    return ERROR_SUCCESS;

  wchar_t DestPath[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
  DestPath[0] = L'\0';
  size_t  DestPathLen = 0;

  int RetVal = ERROR_SUCCESS;
  int r;

  // start with an invalid value
  int DestPathIdx = -1;

  // Go through all files
  _Pathes::iterator	iter = aBegin;
  _Pathes::iterator	last = aBegin;
  while (++iter != aEnd)
  {
    if ((*iter)->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64 || (*iter)->m_DiskIndex != (*last)->m_DiskIndex )
    {
      // border of set
      r = CopyHardlink (DestPath,
        DestPathLen,
        DestPathIdx,
        last, 
        iter, 
        pStats, 
        aPathNameStatusList,
        apContext,
        aFlags
      );
      last = iter;
      if (ERROR_REQUEST_ABORTED == r)
      {
        RetVal = ERROR_REQUEST_ABORTED;
        break;
      }
    }
  }

  // Do the last file too
  if (ERROR_REQUEST_ABORTED != RetVal)
  {
    r = CopyHardlink (DestPath,
      DestPathLen,
      DestPathIdx,
      last, 
      iter, 
      pStats, 
      aPathNameStatusList,
      apContext,
      aFlags
    );
    if (ERROR_REQUEST_ABORTED == r)
      RetVal = ERROR_REQUEST_ABORTED;
  }

  return RetVal;
}

int
FileInfoContainer::
CopyHardlink(
  PWCHAR				            aDestPath,
  size_t&			              aDestPathLen,
  int&                      aDestPathIdx,
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  CopyStatistics*		        pStats,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext,
  CopyReparsePointFlags     aFlags
)
{
  // CopyHardlink is longpath safe

  int RetVal = ERROR_SUCCESS;

#if defined _DEBUG
  size_t effort = distance(aBegin, aEnd);
  int count = 0;
#endif

  _Pathes::iterator copyiter = aBegin;

  BOOL CopyOk = false;
  bool DoCopy = true;
  bool IsFirstHardlink = true;
  wchar_t HardlinkLeaderPath[HUGE_PATH];
  int CurDestPathIdx = -1;
  int PathParseSwitchOffSize;
  // Check if the destination is a very long path. It could also be a UNC Path
  if (IsVeryLongPath(aDestPath))
    PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
  else
    // UNC Path
    PathParseSwitchOffSize = 0;

    int CopyFlags = COPY_FILE_COPY_WRITE_TIME | COPY_FILE_FAIL_IF_EXISTS;
    if (m_Flags & eBackupMode)
      CopyFlags |= COPY_FILE_COPY_SACL | COPY_FILE_COPY_CREATION_TIME | COPY_FILE_COPY_ACCESS_TIME;

    if (m_Flags2 & eNoAds)
      CopyFlags |= COPY_FILE_COPY_SKIP_ADS;

    if (m_Flags2 & eNoEa)
      CopyFlags |= COPY_FILE_COPY_SKIP_EA;

    if (aFlags & eSmartMirror)
      CopyFlags |= COPY_FILE_COPY_CREATION_TIME | COPY_FILE_COPY_ACCESS_TIME;

  // We have to take into account, that copying the first file might not
  // work. So we have to try out file by file of the hardlink tupel if
  // at least one file can be copied, and all others can be linked to 
  // that file. The first file of the hardlink tupel, which can be copied
  // is called the 'leader' of the hardlink tupel
  do 
  {
    // Prepare first entry, which will be copied
    FileInfo*	pF = *copyiter;
    int SourcePathLen = pF->m_SourcePathLen;

    // See if the DestPath has changed since the last time
    CurDestPathIdx = pF->m_DestPathIdx;
    if (CurDestPathIdx != aDestPathIdx)
    {
      aDestPathLen = m_AnchorPathCache.GetDestPath(aDestPath, CurDestPathIdx);
      aDestPathIdx = CurDestPathIdx;

      // Check if the destination is a very long path. It could also be a UNC Path
      if (IsVeryLongPath(aDestPath))
        PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
      else
        // UNC Path
        PathParseSwitchOffSize = 0;

#if defined EXACT_PATH_INDEX
      if (! (m_AnchorPathCache.GetFlags(CurDestPathIdx) & _ArgvPath::Created) )
      {
#endif
        // SmartMirror or SmartCopy?
        if (!(aFlags & eSmartMirror) )
        {
          // Not SmartMirror 
          //
          // Check if destination directory already exists 
          if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(aDestPath))
          {
            int rResult = CreateDirectoryHierarchy(aDestPath, aDestPathLen);
            if (ERROR_SUCCESS == rResult)
            {
              pStats->m_DirectoryCreated++;
              Log(L"+d %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
#if defined EXACT_PATH_INDEX
              m_AnchorPathCache.MarkCreated(aDestPath);
#endif
            }
            else
            {
              pStats->m_DirectoryCreateFailed++;
              PathNameStatus pns(PlusD, &aDestPath[PathParseSwitchOffSize], rResult);
            }
          }
        }
        else
        {
          // SmartMirror
        }
#if defined EXACT_PATH_INDEX
      }
#endif
    }


    aDestPath[aDestPathLen] = 0x00;
    wcscat_s(aDestPath, HUGE_PATH, &pF->m_FileName[SourcePathLen]);
    
    // Check if the source is a very long path. It could also be a UNC Path
    size_t PathParseSwitchOffSize_Source = 0;
    if (IsVeryLongPath(pF->m_FileName))
      PathParseSwitchOffSize_Source = PATH_PARSE_SWITCHOFF_SIZE;

    // We have to save the destination of the leader of the hardlink tupel
    // because later on an additional hardlink can be tied to the leader,
    // but the leader has not been copied
    wcscpy_s(HardlinkLeaderPath, HUGE_PATH, aDestPath);

    // it might happen that only the compression status changed
    bool CompressionChanged = false;
    FileInfo* pLookAside = NULL;
    switch (aFlags & (eSmartMirror|eSmartCopy))
    {
      case eSmartMirror:
      {
        pLookAside = m_pLookAsideFileInfoContainer->Find(aDestPath);
        if (pLookAside)
          DoCopy = pF->IsDifferent(pLookAside, m_DateTimeTolerance);

        if (false == DoCopy)
        {
          // DoCopy == false
          // The files were the same

          if (apContext)
          {
            // Anyway if the files are the same we have to move on with the progressbar
            apContext->AddProgress(pF->m_FileSize.ul64, pF->m_FileSize.ul64, 1);
#if defined _DEBUG
            count++;
#endif

          }

          if (m_Flags2 & eDeloreanMerge)
          {
            // With --merge having the same files means: hardlink the files, aka merge the sets
            //

            // The files could be already hardlinked so lets check with via pLookAside
            if (pLookAside->m_FileIndex.ul64 == pF->m_FileIndex.ul64)
            {
              // uuups the files are already hardlinked
              Log(L"°=f %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
            }
            else
            {
              // No the files to be merged are already hardlinked, so lets go and try to merge them
              wchar_t	LinkName[HUGE_PATH + 2];
              DWORD LinkNameLength = HUGE_PATH; 
              HRESULT HardlinkResult;

              // We have to check if we exceed the number of 1023 hardlink limit
                
              // If we are in --traditional we know the refcount from enumeration. With the fast mode 
              // we don't know it, so we have to query it once, even if it is time-consuming
              if (!(m_Flags & eTraditional))
              {
                pF->m_RefCount = -1;

                // Open the file so that we can read out the refcount
                HANDLE	SourceFileHandle = CreateFileW (
                  pF->m_FileName, 
                  m_Flags & eBackupMode ? GENERIC_READ | READ_CONTROL | ACCESS_SYSTEM_SECURITY | FILE_READ_EA : FILE_READ_ATTRIBUTES | FILE_READ_EA,
                  FILE_SHARE_READ, 
                  NULL, 
                  OPEN_EXISTING, 
                  m_Flags & eBackupMode ? FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL,
                  NULL
                );
                if (INVALID_HANDLE_VALUE != SourceFileHandle)
                {
                  BY_HANDLE_FILE_INFORMATION	FileInformation;
                  BOOL	r = GetFileInformationByHandle (SourceFileHandle, &FileInformation);
                  if (r)
                    pF->m_RefCount = FileInformation.nNumberOfLinks;

                  CloseHandle(SourceFileHandle);
                }
              }
              // By the end we have a valid pF->m_RefCount


              // Iterate through all siblings, add it to a temporary list and count how many siblings there are
              int DestNumberSiblings = 0;
              wcsncpy_s(LinkName, LinkNameLength, aDestPath, PATH_PARSE_SWITCHOFF_SIZE + 2);
              HANDLE FindHardLinkHandle = FindFirstFileNameW(aDestPath, 0, &LinkNameLength, &LinkName[PATH_PARSE_SWITCHOFF_SIZE + 2]);
              if (INVALID_HANDLE_VALUE != FindHardLinkHandle)
              {
	              // Add to list, because one can not delete aDestPath itself while iterating over the siblings
                _StringList Siblings;
                do
                {
                  Siblings.push_back(LinkName);
                  LinkNameLength = HUGE_PATH; 
                  DestNumberSiblings++;
                } while (FindNextFileNameW(FindHardLinkHandle, &LinkNameLength, &LinkName[PATH_PARSE_SWITCHOFF_SIZE + 2]));
                FindClose(FindHardLinkHandle);

                // Check if we do not exceed the hardlink limit of 1023
                if (DestNumberSiblings + pF->m_RefCount < m_HardlinkLimit)
                {
                  // If there was at least one sibling, and the refcount will not be exceeded
                  if (Siblings.size())
                  {
                    bool MergeSuccess = true;
                      
                    // Hardlink the siblings
                    for (_StringListIterator iter = Siblings.begin(); iter != Siblings.end(); ++iter)
                    {
                      BOOL bDeleted = DeleteSibling(iter->c_str(), GetFileAttributes(iter->c_str())); 
                      HardlinkResult = TRUE == CreateHardLink(iter->c_str(), pF->m_FileName, NULL) ? ERROR_SUCCESS : GetLastError();

                      if (ERROR_SUCCESS == HardlinkResult)
                      {
                        pStats->m_FilesLinked++;
                        pStats->m_BytesLinked += pF->m_FileSize.ul64;
  //                      Log(L"°f %s\n", &iter->c_str()[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                      }
                      else
                      {
                        pStats->m_FilesLinkFailed++;
                        pStats->m_BytesLinkFailed += pF->m_FileSize.ul64;

                        PathNameStatus pns(MergeF, &iter->c_str()[PathParseSwitchOffSize], ERROR_ALREADY_EXISTS == HardlinkResult ? ERROR_FILE_EXISTS : HardlinkResult);
                        aPathNameStatusList->push_back(pns);

                        MergeSuccess = false;
                      }
                    }
                      
                    if (MergeSuccess)
                      Log(L"°f %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);

                    Siblings.clear();
                  }
                } // if (DestNumberSiblings + pF->m_RefCount < m_HardlinkLimit)
                else
                {
                  /// The hardlink limit was exceed for this file, can't link
                  Log(L"°~f %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                }
              }
            }
          }
          else
          {
            // With --mirror having the same files means: do nothing
            pStats->m_FilesCopySkipped++;
            pStats->m_BytesCopySkippped += pF->m_FileSize.ul64;

            Log(L"=f %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
          }
		  
		      // Skipping is ok to leave the loop
          CopyOk = true;

          // we are done here with SmartMirror
          break;
        }
        else // if (false == DoCopy)
        {
          // DoCopy == true;
          
          // The file in the source is different than in the destination so delete it, and use the 
          // SmartCopy below to replace this file. But if we are only copying over we don't want to report 
          // that we first deleted it and then copied it over, because this info is useless. We
          // are just reporting that we copied it over later

          // But there are situations if the permission to a file in the destination are not enough that
          // it was enumerated and found in the LookAside buffer, but the file is there and a deletion
          // is necessary. Then use pF to at least try to delete the type of file, if it is in the source.
          // If it is a different type in the destination, and the permissions are not enough, then we 
          // are lost
          FileInfo* pFileInfo = pLookAside ? pLookAside : pF;

          bool bDeleted = false;
          if (pF->m_Type != pFileInfo->m_Type)
          {
            // There are changes of m_Type for which it is neccesary to delete the file, e.g if a file 
            // changed into a symlink, but if just the compression bit changed, there is no need to delete
            if (pF->m_Type & FILE_ATTRIBUTE_COMPRESSED ^ pFileInfo->m_Type & FILE_ATTRIBUTE_COMPRESSED)
            {
              // Fake a deletion, let it copy over, and also copy the attribute
              bDeleted = true;
              CompressionChanged = true;
            }
            else
            {
              // If the type of item in the source and destination is different, we would like to report this
              bDeleted = DeleteItem(aDestPath, pFileInfo, aPathNameStatusList, pStats);
            }
          }
          else // if (pF->m_Type != pFileInfo->m_Type)
          {
            // If it is the same just try to delete it
            bDeleted = DeleteFileAndReport(aDestPath, aPathNameStatusList);
          }

          // Remove the item, and if it was found in the LookAside Cache, use it.
          if (!bDeleted)
          {
            // Couldn't delete file
            pStats->m_FilesCopyFailed++;
            pStats->m_BytesCopyFailed += pF->m_FileSize.ul64;

            // Thus we would like to leave the loop for this item
            CopyOk = true;
          
            // We also have to move ahead with the progress, which would normally be done during copy
            if (apContext)
            {
              apContext->AddProgress(pF->m_FileSize.ul64, 1, 1);
#if defined _DEBUG
              count++;
#endif
            }

            // Breaking here will not bring us to SmartCopy
            break;
          }
        } // if (false == DoCopy)
      }
      // break intentionally omitted. 
      /// We deleted successfully before and want to use SmartCopy to copy over the leader

      case eSmartCopy:
      {
        // Check whether we have to go async
        if (apContext)
        {
          // Set the status, and check if CopyHardlink has been cancelled from outside
          int r = apContext->PutStatus(&pF->m_FileName[PathParseSwitchOffSize_Source], &aDestPath[PathParseSwitchOffSize]);
          if ( ERROR_REQUEST_ABORTED == r)
            RetVal = ERROR_REQUEST_ABORTED;
        }

        // 
        if (DoCopy)
        {
          DWORD                 LastError;
          _PathNameStatusList   CopyFileError;
          int                   CopyOkInt = ERROR_SUCCESS;

          if (apContext)
          {
            // Since CopyFileEx only returns absolute progress values, we have to create a snapshot
            apContext->CreateSnapShot();

            CopyOkInt = CopyFileEx3(
              pF->m_FileName, 
//              L"\\\\?\\f:\\tmp\\hhh.txt",
              aDestPath, 
              (LPPROGRESS_ROUTINE)CopyProgressRoutine, 
              apContext, 
              (LPBOOL)&apContext->m_Status, 
              CopyFlags, 
              &m_pSecDesc, 
              &m_SecDescSize,
              m_DateTimeTolerance,
              &CopyFileError
            );
            CopyOk = ERROR_SUCCESS == CopyOkInt;
            LastError = GetLastError();

            // Anyway at the end we have to add the size to the progress
            apContext->Add2SnapShot(pF->m_FileSize.ul64, pF->m_FileSize.ul64, 1);
          }
          else 
          {
            BOOL                  bCancel = FALSE;
 
            // Normal way
            CopyOkInt = CopyFileEx3(
              pF->m_FileName, 
              aDestPath, 
              NULL, 
              NULL, 
              &bCancel, 
              CopyFlags, 
              &m_pSecDesc, 
              &m_SecDescSize,
              m_DateTimeTolerance,
              &CopyFileError
            );
            CopyOk = ERROR_SUCCESS == CopyOkInt;
            LastError = GetLastError();
          }

          switch (CopyOkInt)
          {
            case ERROR_ALREADY_EXISTS:
            {
              // File in destination was there, has same size, but is older or of same age than source. Don't copy
              DoCopy = false;

              pStats->m_FilesCopySkipped++;
              pStats->m_BytesCopySkippped += pF->m_FileSize.ul64;

              Log(L"=f %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);

              // But having a leader of the hardlink tupel already there is also ok
              // so that we can tie all other files to it. We need to set CopyOk to 
              // true so that the loop to copy at the leader of the hardlink tupel
              // ends
              CopyOk = true;
            }
            break;

            case ERROR_SUCCESS:
            {
              if (CompressionChanged)
              {
                // if only the compression changed we assume this as beeing already there
                pStats->m_FilesCopySkipped++;
                pStats->m_BytesCopySkippped += pF->m_FileSize.ul64;

                Log(L"=f %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
              }
              else
              {
                pStats->m_FilesCopied++;
                pStats->m_BytesCopied += pF->m_FileSize.ul64;

                Log(L"+f %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
              }
            }
            break;

            case ERROR_FILE_EXISTS:
            {
              // To be more specific about which file could not be opened, we abuse the ERROR_FILE_EXISTS
              // for indication of a problem in the *target file*, and ERROR_ACCESS_DENIED as usual as
              // indication for problems on the *source file*
              pStats->m_FilesCopyFailed++;
              pStats->m_BytesCopyFailed += pF->m_FileSize.ul64;

              // If the CopyfileEx3 function provides itself a more detailed error description, then use it
              if (CopyFileError.size())
              {
                for (_PathNameStatusList::iterator iter = CopyFileError.begin(); iter != CopyFileError.end(); ++iter)
                  aPathNameStatusList->push_back(*iter);
              }
              else
              {
                PathNameStatus pns(PlusF, &aDestPath[PathParseSwitchOffSize], ERROR_ACCESS_DENIED);
                aPathNameStatusList->push_back(pns);
              }

              // This item can not be copied, so leave the loop
              CopyOk = true;
            }
            break;

            case ERROR_ACCESS_DENIED:
            default: 
            {
              // Copy Operation failed
              pStats->m_FilesCopyFailed++;
              pStats->m_BytesCopyFailed += pF->m_FileSize.ul64;

              // If the CopyfileEx3 function provides itself a more detailed error description, then use it
              if (CopyFileError.size())
              {
                for (_PathNameStatusList::iterator iter = CopyFileError.begin(); iter != CopyFileError.end(); ++iter)
                  aPathNameStatusList->push_back(*iter);
              }
              else
              {
                PathNameStatus pns(PlusF, &(pF->m_FileName[PathParseSwitchOffSize_Source]), LastError);
                aPathNameStatusList->push_back(pns);
              }

              // This item can not be copied, so leave the loop
              CopyOk = true;
            }
            break;
          }
        } // if (DoCopy)
      }
      break;
    }

    ++copyiter;
    // Go on until one file was copied successfully, or we reach the end of the tupel
  } while (CopyOk == false && copyiter != aEnd  );


  // Hardlink all other files of a tupel to the first file, aka the 'leader' of the hardlink tupel.
  //
  wchar_t		LinkFile[HUGE_PATH];
  wcscpy_s(LinkFile, HUGE_PATH, aDestPath);

  // Only link to the leader during copy, but not during --merge, because the 
  // merge has already happened while operating on the leader. See above
  if (!(m_Flags2 & eDeloreanMerge))
  {
    for (_Pathes::iterator iter = copyiter; iter != aEnd; ++iter)
    {
      FileInfo*	pF = *iter;

      // See if the DestPath has changed since last time
      if (pF->m_DestPathIdx != aDestPathIdx)
      {
        aDestPathLen = m_AnchorPathCache.GetDestPath(LinkFile, pF->m_DestPathIdx);
        aDestPathIdx = pF->m_DestPathIdx;

        // Check if the destination is a very long path. It could also be a UNC Path
        if (IsVeryLongPath(LinkFile))
          PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
        else
          // UNC Path
          PathParseSwitchOffSize = 0;

        // Check if destination directory already exists if UpdateCopy is set
        // Otherwise in create the directory hierarchy
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(LinkFile))
        {
          int rResult = CreateDirectoryHierarchy(LinkFile, aDestPathLen);
          if (ERROR_SUCCESS == rResult)
          {
            pStats->m_DirectoryCreated++;
            Log(L"+d %s\n", &aDestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
          }
          else
          {
            pStats->m_DirectoryCreateFailed++;
            PathNameStatus pns(PlusD, &aDestPath[PathParseSwitchOffSize], rResult);
          }
        }
      }
      else
      {
        LinkFile[aDestPathLen] = 0x00;
      }

      wcscat_s(LinkFile, HUGE_PATH, &pF->m_FileName[pF->m_SourcePathLen]);

      if (apContext)
      {
        // Check if the source is a very long path. It could also be a UNC Path
        size_t PathParseSwitchOffSize_Source = 0;
        if (IsVeryLongPath(pF->m_FileName))
          PathParseSwitchOffSize_Source = PATH_PARSE_SWITCHOFF_SIZE;

        // Increment the progress by 'weight' of hardlink creation
        apContext->AddProgress(eHardlinkWeight, 1, 1);
#if defined _DEBUG
        count++;
#endif

        // Set the status, and check if Smartcopy has been cancelled from outside
        int r = apContext->PutStatus(&pF->m_FileName[PathParseSwitchOffSize_Source], &aDestPath[PathParseSwitchOffSize]);
        if ( ERROR_REQUEST_ABORTED == r)
        {
          RetVal = ERROR_REQUEST_ABORTED;
          break;
        }
      }

      // aDestPath and LinkFile are both long path safe
      bool DoHardlink = true;
      
      switch (aFlags & (eSmartMirror|eSmartCopy))
      {
        case eSmartMirror:
        {
          // In Mirror mode things get linked only if the file is newer
          FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(LinkFile);
          if (pLookAside)
            DoHardlink = pF->IsDifferent(pLookAside, m_DateTimeTolerance);

          // DoCopy is here because it is set during copying the file under HardlinkLeaderPath. If it was copied, 
          // we also have to newly tie all the other dupes to the just newly copied HardlinkLeader
          if (true == DoHardlink || true == DoCopy)
          {
            FileInfo* pFileInfo = pLookAside ? pLookAside : pF;

            // Only if the type changed we would like to have it with with report...
            if (pF->m_Type != pFileInfo->m_Type)
              DeleteItem(LinkFile, pFileInfo, aPathNameStatusList, pStats);
            else
              // ... otherwise delete it silently
              DeleteFile(LinkFile);

            // Code will be continued with case: SmartCopy
          }
          else
          {
            pStats->m_FilesLinkSkipped++;
            pStats->m_BytesLinkSkipped += pF->m_FileSize.ul64;

            Log(L"=h %s\n", &LinkFile[PathParseSwitchOffSize], m_Flags, eLogVerbose);

            break;
          }
        }
        // break intentionally omitted

        case eSmartCopy:
        {
          // If the a file has been copied we have to force that all other files get 
          // newly hardlinked, because it could happen, that a file x.ext gets a new
          // hardlink 'hardlink of x.ext', but the uniqe id of 'hardlink of x.ext'
          // is lower than the uniqe id of x.ext. Then 'hardlink of x.ext' would
          // get copied as a new file, because it is the 'leader' of the hardlink 
          // n-tupel, but x.ext is alreay there in the destination, but not
          // hardlinked to 'hardlink of x.ext'. So this would split up a hardlink 
          // n-tupel. ==> Once a file is copied hardlink all its siblings newly
          if (DoCopy)
            BOOL DeleteOk = DeleteFile(LinkFile);

          int result = CreateHardlink(aDestPath, LinkFile);
          if (ERROR_SUCCESS == result)
          {
            // Successfully created hardlink
            if (!DoCopy && IsFirstHardlink)
            {
              // There is a situation, where the first file (leader) of the hardlink tupel 
              // didn't get copied, but new hardlinks have been linked to that file. In this case
              // we have to print out the name of this 'leader', which has not not been copied
              // so that we know, that the following files are linked to that leader.
              // 
              // If the leader would have been copied, it would be easy, because it would
              // in any case be printed as copied file, but here we need to pull the
              // name of the 'hardlink leader' and show it. 
              //
              Log(L".f %s\n", &HardlinkLeaderPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
              IsFirstHardlink = false;
            }

            pStats->m_FilesLinked++;
            pStats->m_BytesLinked += pF->m_FileSize.ul64;

            Log(L"*h %s\n", &LinkFile[PathParseSwitchOffSize], m_Flags, eLogChanged);
          }
          else
          {
            // Failed in hardlink creation
            if (ERROR_ALREADY_EXISTS == result)
            {
              pStats->m_FilesLinkSkipped++;
              pStats->m_BytesLinkSkipped += pF->m_FileSize.ul64;

              Log(L"=h %s\n", &LinkFile[PathParseSwitchOffSize], m_Flags, eLogVerbose);
            }
            else
            {
              pStats->m_FilesLinkFailed++;
              pStats->m_BytesLinkFailed += pF->m_FileSize.ul64;

              PathNameStatus pns(StarH, &LinkFile[PathParseSwitchOffSize], result);
              aPathNameStatusList->push_back(pns);
            } // if (ERROR_ALREADY_EXISTS == result)
          }
          break;
        } // case SmartCopy
      } // switch
    } // for (iter = aBegin; iter != aEnd; ++iter)
  }
  else
  {
    // DeloreanMerge
    if (apContext)
    {
      size_t nSiblings = distance(copyiter, aEnd);
      apContext->AddProgress(nSiblings * eHardlinkWeight, nSiblings, nSiblings);
#if defined _DEBUG
      count += nSiblings;
#endif
    }
  }

  // aDestPathIdx must never get of sync with aDestPath so at the end copy it
  // back to aDestPath. Before chop it back
  LinkFile[aDestPathLen] = 0x00;
  wcscpy_s(aDestPath, HUGE_PATH, LinkFile);

  return RetVal;
}

bool
FileInfoContainer::
DeleteDirectoryAndReport(
  wchar_t*                  a_FileName,
  _PathNameStatusList*      a_pPathNameStatusList
)
{
  return RemoveDirectory(a_FileName) ? true : false;;

#if 0
  BOOL DeleteOk = DeleteFile(a_FileName);
  
  // It might happen that DeleteFile fails, because e.g the readonly attributes are set
  // in the destination, or one has no permission to delete the file in the destination 
  // at all. If it is just the readonly atrribute or something simple similar, we are
  // going to unlock the file in the destination and try again
  if (!DeleteOk && GetLastError() != ERROR_FILE_NOT_FOUND)
  {
    BOOL report = SetFileAttributes(a_FileName, FILE_ATTRIBUTE_NORMAL);
    if (report)
    {
      // ok it was a simple case, and we could unlock it
      report = DeleteFile(a_FileName);

      // But maybe DeleteFile also fails after changing the attributes...
    }

    if (!report)
    {
      // Could not get access to the file in the destination so report it
      PathNameStatus pns(MinusF, &(a_FileName[PATH_PARSE_SWITCHOFF_SIZE]), GetLastError());
      a_pPathNameStatusList->push_back(pns);

      return false;
    }
  }

  // Successfully deleted the file
  return true;
#endif
}

bool
FileInfoContainer::
DeleteFileAndReport(
  wchar_t*                  a_FileName,
  _PathNameStatusList*      a_pPathNameStatusList
)
{
  DWORD Attrib = GetFileAttributes(a_FileName);
  BOOL DeleteOk = DeleteSibling(a_FileName, Attrib);
#if defined DEBUG_DS
  fwprintf (m_OutputFile, L"DFAR 01: %s:%08x\n", a_FileName, Attrib);
#endif
  
  // It might happen that DeleteFile fails, because e.g the readonly attributes are set
  // in the destination, or one has no permission to delete the file in the destination 
  // at all. If it is just the readonly attribute or something simple similar, we are
  // going to unlock the file in the destination and try again
  if (!DeleteOk && (GetLastError() != ERROR_FILE_NOT_FOUND))
  {
    BOOL report = SetFileAttributes(a_FileName, FILE_ATTRIBUTE_NORMAL);
    if (TRUE == report)
    {
      // ok it was a simple case, and we could unlock it
      report = DeleteFile(a_FileName);

      // But maybe DeleteFile also fails after changing the attributes...
    }

    if (FALSE == report)
    {
      int PathParseSwitchOffSize;

      // Check if the destination is a very long path. It could also be a UNC Path
      if (IsVeryLongPath(a_FileName))
        PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
      else
        // UNC Path
        PathParseSwitchOffSize = 0;

      // Could not get access to the file in the destination so report it
      PathNameStatus pns(MinusF, &a_FileName[PathParseSwitchOffSize], GetLastError());
      a_pPathNameStatusList->push_back(pns);

      return false;
    }
  }

  // Successfully deleted the file
  return true;
}

int
FileInfoContainer::
CopyDirectoriesSilent(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  AsyncContext*             apContext,
  CopyReparsePointFlags     aFlags
)
{
  // The copy behaviour of the first directory is a little bit broken, because we have to add the
  // first directory into a separate container, so that we can restore the timestamps. But the 
  // first directory should also be part of m_Filenames and not of a seperate container. This 
  // causes problems, when you want to create symlinks/junctions to first directories
  //
  // Or maybe the first directories are already there in m_Filenames, but somehow are counted 
  // wrong in the stats. The CopyDirectories seems not to create empty directories. Why? Maybe
  // this is a cheap solution. It is a mess, and one day this should be solved. On the otherhand it
  // has no much impact if it is working like this. It only annoys LSE users.
  //
  // So CopyDirectorySeilent is a workaround anyway, and should be removed.
  //
  // It might happen, that there were empty root directories, which have not been created
  // so create it, so that we can restore the timestamps
  _PathNameStatusList SilentPathNameStatusList;
  CopyStatistics SilentStats;
  int SaveFlags = m_Flags;
  m_Flags |= eLogQuiet;
  int iResult = CopyDirectories(
    aBegin, 
    aEnd, 
    &SilentStats, 
    &SilentPathNameStatusList,
    apContext,
    aFlags
  );
  m_Flags = SaveFlags;

  return iResult;
}


int
FileInfoContainer::
CopyDirectories(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  CopyStatistics*		        pStats,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext,
  CopyReparsePointFlags     aFlags
)
{
  int RetVal = ERROR_SUCCESS;

  if (aBegin == aEnd)
    return RetVal;

  WCHAR	DestPath[HUGE_PATH] = { 0 };
  size_t  DestPathLen = 0;

  // Create all Directories. Sorting brings directories in such an order
  // so that we do not have to create directory hierarchies
  sort (aBegin, aEnd, PathIdxFilenameSorterAscending());

  // start with an invalid value
  int DestPathIdx = -1;
  int PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;

  // There is no point to copy the timestamps of directories right now, 
  // because they get changed as soon as the first file is copied to it.
  // So we have to do it later.
  int CreateDirFlags = 0;
  if (m_Flags & eBackupMode)
    CreateDirFlags |= COPY_FILE_COPY_SACL;

  if (m_Flags2 & eNoAds)
    CreateDirFlags |= COPY_FILE_COPY_SKIP_ADS;

  if (m_Flags2 & eNoEa)
    CreateDirFlags |= COPY_FILE_COPY_SKIP_EA;

  _Pathes::iterator iter;
  for (iter = aBegin; iter != aEnd; ++iter)
  {
    FileInfo*	pF = *iter;

    // See if the DestPath has changed since the last time
    if (pF->m_DestPathIdx != DestPathIdx)
    {
      DestPathLen = m_AnchorPathCache.GetDestPath(DestPath, pF->m_DestPathIdx);
      DestPathIdx = pF->m_DestPathIdx;

      // Check if the destination is a very long path. It could also be a UNC Path
      if (IsVeryLongPath(DestPath))
        PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
      else
        // UNC Path
        PathParseSwitchOffSize = 0;

#if defined EXACT_PATH_INDEX
      if ( !(m_AnchorPathCache.GetFlags(DestPathIdx) & _ArgvPath::Created) )
      {
#endif

        // SmartMirror or SmartCopy
        if (!(aFlags & eSmartMirror))
        {
          // Not SmartMirror

          // Check if destination directory already exists
          DWORD DirectoryAttributes = GetFileAttributes(DestPath);
          if (INVALID_FILE_ATTRIBUTES == DirectoryAttributes)
          {
            // Remember that we later on have to create the whole hierarchy
            int rResult = CreateDirectoryHierarchy(DestPath, DestPathLen);
            if (ERROR_SUCCESS == rResult)
            {
              pStats->m_DirectoryCreated++;
              Log(L"+d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);

#if defined EXACT_PATH_INDEX
              m_AnchorPathCache.MarkCreated(DestPath);
#endif
            }
            else
            {
              pStats->m_DirectoryCreateFailed++;
              PathNameStatus pns(PlusD, &DestPath[PathParseSwitchOffSize], rResult);
            }
          }
          else
          {
            // SmartMirror
            pStats->m_DirectoryCreateSkipped++;
            Log(L"=d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
          }
        } // if (!(aFlags & eSmartMirror))
      } // if (pF->m_DestPathIdx != DestPathIdx)

#if defined EXACT_PATH_INDEX
    }
#endif

    // If the Destpath has not changed, we have to append a piece of the source
    wcscat_s(DestPath, HUGE_PATH, &(pF->m_FileName[pF->m_SourcePathLen]));

    if (apContext)
    {
      // Increment the progress
      apContext->AddProgress(eDirectoryWeight, 1, 1 );

      // Set the status, and check if CopyHardlink has been cancelled from outside
      int r = apContext->PutStatus(&pF->m_FileName[PathParseSwitchOffSize], &DestPath[PathParseSwitchOffSize]);
      if ( ERROR_REQUEST_ABORTED == r)
      {
        RetVal = ERROR_REQUEST_ABORTED;
        break;
      }
    }

    bool CompressionChanged = false;
    switch (aFlags & (eSmartMirror|eSmartCopy))
    {
      case eSmartMirror:
      {
        // Tricky but dangerous :)
        FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath); 
        if (pLookAside)
        {
          if (false == pF->IsDifferent(pLookAside, m_DateTimeTolerance))
          {
            pStats->m_DirectoryCreateSkipped++;
            Log(L"=d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);

            // We are done here, so leave the case: of the switch via break
            break;
          }
          else
          {
            // The item in the destination is anything but a directory. Or it could have changed its timestamp
            
            if (pF->m_Type != pLookAside->m_Type)
            {
              bool bDeleted = false;
              if (pF->m_Type & FILE_ATTRIBUTE_COMPRESSED ^ pLookAside->m_Type & FILE_ATTRIBUTE_COMPRESSED)
              {
                // Fake a deletion
                bDeleted = true;
                CompressionChanged = true;
              }
              else
              {
                // It changed its type and thus we are going to delete it in the destination
                bDeleted = DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
              }
              
              if (!bDeleted)
              {
                // TBD Check the result
                ;
              }
              
              // But the break here is intentionally omitted, because we have to create a directory
            }
            else
            {
              // If the timestamp is different, we can't do anything about it for directories
              // We report it to be the same, even if we should report it as the same but with changed timestamp
              pStats->m_DirectoryCreateSkipped++;
              Log(L"=d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);

              break;
            } // if (pF->m_Type != pLookAside->m_Type)
          } // if (!pF->IsDifferent....
        }
        else
        {
          // It was not found in the LookAsideContainer so go on 
          // with eSmartCopy where the directories are created
        }
      }
      // break intentionally ommitted


      case eSmartCopy:
      {
        _PathNameStatusList CopyDirectoryError;

        BOOL result = CopyDirectory(
          pF->m_FileName, 
          DestPath, 
          NULL, 
          NULL, 
          CreateDirFlags, 
          &m_pSecDesc,
          &m_SecDescSize,
          &CopyDirectoryError
        );
        if ((ERROR_SUCCESS == result) && !CompressionChanged) 
        {
          DWORD LastError = GetLastError();
          if (ERROR_ALREADY_EXISTS == LastError) 
          {
            pStats->m_DirectoryCreateSkipped++;
            Log(L"=d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
          }
          else
          {
            // The directory creation failed
            pStats->m_DirectoryCreateFailed++;

            // If the CopyDirectory function provides itself a more detailed error description, then use it
            if (CopyDirectoryError.size())
            {
              for (_PathNameStatusList::iterator iter = CopyDirectoryError.begin(); iter != CopyDirectoryError.end(); ++iter)
                aPathNameStatusList->push_back(*iter);
            }
            else
            {
              PathNameStatus pns(PlusD, &DestPath[PathParseSwitchOffSize], LastError);
              aPathNameStatusList->push_back(pns);
            }
          }
        }
        else
        {
          // Also copy the compression bit
          CopyCompression(pF->m_FileName, DestPath, pF->m_Type, GetFileAttributes(DestPath));

          if (CompressionChanged)
          {
            pStats->m_DirectoryCreateSkipped++;
            // Log(L"%%d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
            Log(L"=d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
          }
          else
          {
            pStats->m_DirectoryCreated++;
            Log(L"+d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
          }

//          CopySecurityAttributesByName(pF->m_FileName, DestPath, &m_pSecDesc, 0); 
        }
      }
      break;
    }

    // restore Destination FullPath for the next string concatenate
    DestPath[DestPathLen] = 0x00;
  }

  return RetVal;
}

//
// Deletes an item
//
bool
FileInfoContainer::
DeleteItem(
  wchar_t*                  a_FileName,
  FileInfo*                 a_pFileInfo,
  _PathNameStatusList*      a_PathNameStatusList,
  CopyStatistics*		        a_pStats
)
{
  bool bResult = true;
  int PathParseSwitchOffSize;

  // Check if the destination is a very long path. It could also be a UNC Path
  
  if (IsVeryLongPath(a_FileName))
    PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
  else
    // UNC Path
    PathParseSwitchOffSize = 0;

  switch(a_pFileInfo->m_Type & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NORMAL) )
  {
    // For normal files
    case FILE_ATTRIBUTE_NORMAL:
    {
      bResult = DeleteFileAndReport(a_FileName, a_PathNameStatusList);
      if (bResult)
      {
        a_pStats->m_FilesCleaned++;
        Log(L"-f %s\n", &a_FileName[PathParseSwitchOffSize], m_Flags, eLogChanged);
      }
      else
        a_pStats->m_FilesCleanedFailed++;
    }
    break;

    // For Junctions or Symbolic Link Directories
    case FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT:
    {
      int ReparsePointType = ProbeReparsePoint(a_FileName, NULL);
      bResult = DeleteDirectoryAndReport(a_FileName, a_PathNameStatusList);
      if (bResult)
      {
        // For a proper logging we need to check the type of the ReparsePoint
        switch(ReparsePointType)
        {
          case REPARSE_POINT_SYMBOLICLINK:
            a_pStats->m_SymlinksCleaned++;
            Log(L"-s %s\n", &a_FileName[PathParseSwitchOffSize], m_Flags, eLogChanged);
          break;

          case REPARSE_POINT_JUNCTION:
            a_pStats->m_JunctionsCleaned++;
            Log(L"-j %s\n", &a_FileName[PathParseSwitchOffSize], m_Flags, eLogChanged);
          break;

          case REPARSE_POINT_MOUNTPOINT:
            a_pStats->m_MountpointsCleaned++;
            Log(L"-m %s\n", &a_FileName[PathParseSwitchOffSize], m_Flags, eLogChanged);
          break;

          default:
            // unknown reparse point deleted
            a_pStats->m_ReparsePointCleaned++;
            Log(L"-r %s\n", &a_FileName[PathParseSwitchOffSize], m_Flags, eLogChanged);
          break;
        }
      }
    }
    break;

    // For Directories
    case FILE_ATTRIBUTE_DIRECTORY:
    {
      a_pStats->m_DirectoriesCleaned++;
      Log(L"-d %s\n", &a_FileName[PathParseSwitchOffSize], m_Flags, eLogChanged);
      bResult = RemoveDirectory(a_FileName) ? true : false;
    }
    break;

    // For Symbolic Link Files
    case FILE_ATTRIBUTE_REPARSE_POINT:
    {
      a_pStats->m_SymlinksCleaned++;
      Log(L"-s %s\n", &a_FileName[PathParseSwitchOffSize], m_Flags, eLogChanged);
      bResult = DeleteFile(a_FileName) ? true : false;
    }
    break;

    default:
      bResult = false;
    break;

  }
  return bResult;
}


int
FileInfoContainer::
CopyReparsePoints_Mountpoint_SmartCopy(
  wchar_t*                  a_pDestpath,
  wchar_t*                  a_pJunctionDestTarget,
  wchar_t**                 a_pSourceFilename,
  CopyStatistics*		        a_pStats,
  _PathNameStatusList*      a_pPathNameStatusList
)
{
  int result = ERROR_SUCCESS;

  *a_pSourceFilename = a_pDestpath;

  // Check if the junction target exists, because we do not want 
  // to create dead junctions during smart copy
  if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(a_pJunctionDestTarget))
  {
    // CreateJunction implicetdly is long path safe so we use [PATH_PARSE_SWITCHOFF_SIZE]
    result = CreateJunction(a_pDestpath, a_pJunctionDestTarget);
  }
  else
  {
    // Dead junctions are failed junctions
    result = ERROR_PATH_NOT_FOUND;
  }

  return result;
}

void
FileInfoContainer::
CopyReparsePoints_Mountpoint_Statistics(
  int&                      a_result,
  wchar_t*                  a_pSourceFilename,
  CopyStatistics*		        a_pStats,
  _PathNameStatusList*      a_pPathNameStatusList
)
{
  // Track statistics and output
  if (a_pSourceFilename)
  {
    int PathParseSwitchOffSize;
    if (IsVeryLongPath(a_pSourceFilename))
      PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
    else
      // UNC Path
      PathParseSwitchOffSize = 0;

    switch (a_result)
    {
      case ERROR_ALREADY_EXISTS:
        a_pStats->m_MountpointsRestoreSkipped++;
        Log(L"=m %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogVerbose);
      break;

      case ERROR_PATH_NOT_FOUND:
        a_pStats->m_MountpointsDangling++;
        Log(L"&m %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogChanged);
      break;

      case ERROR_BAD_PATHNAME:
      {
        // This error happens under XP if a dangling mountpoint should be restored
        a_pStats->m_MountpointsRestoreFailed++;
        PathNameStatus pns(DangleM, &a_pSourceFilename[PathParseSwitchOffSize], a_result);
        a_pPathNameStatusList->push_back(pns);
      }
      break;

      case ERROR_SUCCESS:
        a_pStats->m_MountpointsRestored++;
        Log(L"+m %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogChanged);
      break;



      default:
      {
        a_pStats->m_MountpointsRestoreFailed++;
        PathNameStatus pns(PlusJ, &a_pSourceFilename[PathParseSwitchOffSize], a_result);
        a_pPathNameStatusList->push_back(pns);
      }
      break;
    }
  } // if (SourceFilename)
}

int
FileInfoContainer::
CopyReparsePoints_Junction_SmartCopy(
  wchar_t*                  a_pDestpath,
  wchar_t*                  a_pJunctionDestTarget,
  wchar_t**                 a_pSourceFilename,
  CopyStatistics*		        a_pStats,
  _PathNameStatusList*      a_pPathNameStatusList
)
{
  int result = ERROR_SUCCESS;

  *a_pSourceFilename = a_pDestpath;

  // Check if the junction target exists, because we do not want 
  // to create dead junctions during smart copy
  if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(a_pJunctionDestTarget))
  {
    // CreateJunction implicitly is long path safe so we use [PATH_PARSE_SWITCHOFF_SIZE]
    result = CreateJunction(a_pDestpath, &a_pJunctionDestTarget[PATH_PARSE_SWITCHOFF_SIZE]);
  }
  else
  {
    // Dead junctions are failed junctions
    result = ERROR_PATH_NOT_FOUND;
  }

  return result;
}

void
FileInfoContainer::
CopyReparsePoints_Junction_Statistics(
  int&                      a_result,
  const wchar_t*            a_pSourceFilename,
  CopyStatistics*           a_pStats,
  _PathNameStatusList*      a_pPathNameStatusList
)
{
  // Track statistics and output
  if (a_pSourceFilename)
  {
    int PathParseSwitchOffSize;
    if (IsVeryLongPath(a_pSourceFilename))
      PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
    else
      // UNC Path
      PathParseSwitchOffSize = 0;

    switch (a_result)
    {
      case ERROR_ALREADY_EXISTS:
        a_pStats->m_JunctionsRestoreSkipped++;
        Log(L"=j %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogVerbose);
      break;

      case ERROR_PATH_NOT_FOUND:
        a_pStats->m_JunctionsDangling++;
        Log(L"&j %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogChanged);
      break;

      case ERROR_BAD_PATHNAME:
      {
        // This error happens under XP if a dangling junction should be restored
        a_pStats->m_JunctionsRestoreFailed++;
        PathNameStatus pns(DangleJ, &a_pSourceFilename[PathParseSwitchOffSize], a_result);
        a_pPathNameStatusList->push_back(pns);
      }
      break;

      case ERROR_SUCCESS:
        a_pStats->m_JunctionsRestored++;
        Log(L"+j %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogChanged);
      break;



      default:
      {
        a_pStats->m_JunctionsRestoreFailed++;
        PathNameStatus pns(PlusJ, &a_pSourceFilename[PathParseSwitchOffSize], a_result);
        a_pPathNameStatusList->push_back(pns);
      }
      break;
    }
  } // if (SourceFilename)
}

void
FileInfoContainer::
CopyReparsePoints_Symlink_Statistics(
  int&                   a_result,
  wchar_t*                  a_pSourceFilename,
  CopyStatistics*		        a_pStats,
  _PathNameStatusList*      a_pPathNameStatusList
)
{
  // Track statistics and output for symbolic links. 
  // If SourceFilename is set it is assumed, that something has been done
  // above. If not (see SmartMirror) do not run the statistics
  if (a_pSourceFilename)
  {
    int PathParseSwitchOffSize;
    if (IsVeryLongPath(a_pSourceFilename))
      PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
    else
      // UNC Path
      PathParseSwitchOffSize = 0;

    switch (a_result)
    {
      case ERROR_ALREADY_EXISTS:
        a_pStats->m_SymlinksRestoreSkipped++;
        Log(L"=s %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogVerbose);
      break;

      case ERROR_PATH_NOT_FOUND:
        a_pStats->m_SymlinksDangling++;
        Log(L"&s %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogChanged);
      break;

      case ERROR_SUCCESS:
        a_pStats->m_SymlinksRestored++;
        Log(L"+s %s\n", &a_pSourceFilename[PathParseSwitchOffSize], m_Flags, eLogChanged);
      break;

      default:
        a_pStats->m_SymlinksRestoreFailed++;

        PathNameStatus pns(PlusS, &a_pSourceFilename[PathParseSwitchOffSize], a_result);
        a_pPathNameStatusList->push_back(pns);
      break;
    }
  } // if (a_pSourceFilename)
}

int
FileInfoContainer::
CopyReparsePoints(
  const _Pathes::iterator&	aBegin,
  const _Pathes::iterator&	aEnd,
  CopyStatistics*		        pStats,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext,
  CopyReparsePointFlags     aFlags
)
{
  _Pathes::iterator ReparseBegin = aBegin;
  _Pathes::iterator ReparseEnd = aEnd;

  int RetVal = ERROR_SUCCESS;

#if defined _DEBUG
  size_t effort = distance(ReparseBegin, ReparseEnd);
  int count = 0;
#endif

  // This loop covers the dead reparse points. We stay in here until all dead reparse 
  // points are processed and added to the statistics
  while (ReparseBegin != ReparseEnd) 
  {
    // This loop processes nested reparse points, and we can only leave this loop
    // if the number of nested reparse points does not shrink anymore.
    // 
    // The idea is to try to create all reparse points, and if the creation of one
    // fails we change the type into a NESTED_REPARSE_POINT and go through this loop 
    // once more, hopefully this time beeing able to create this nested reparse point, 
    // and if successfull set the type back to REPARSE_POINT.
    size_t LastReparseSize;
    size_t CurrentReparseSize = distance (ReparseBegin, ReparseEnd);

    do 
    {
      LastReparseSize = CurrentReparseSize;
      
      WCHAR	DestPath[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE] = { 0 };
      size_t  DestPathLen = 0;

      // Create all junctions. Sorting brings junctions in such an order
      // so that we do not have to create directory hierarchies
      sort (ReparseBegin, ReparseEnd, PathIdxFilenameSorterAscending());

      // start with an invalid value
      int DestPathIdx = -1;
      int PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;

      // Recreate the Reparse Points
      _Pathes::iterator iter;
      for (iter = ReparseBegin; iter != ReparseEnd; ++iter)
      {
        // [EXAMPLE] pF->m_FileName == "\\?\n:\MultiSourceUnroll\source\F1\F1_F0\F1_F0_J0\F0_F1_J2"
        FileInfo*	pF = *iter;

        // in backupmode (and proably also in other modes) we have to rebuild reparsepoints in 
        // the target with the same relativity/absoluty as they were in the source.
        // By default we assume a reparsepoint is absolute.
        bool  ReparsePointRelative = false;

        // ProbeReparsePoint returns normal path, so we prepare \\?\ 
        WCHAR	ReparseSrcTarget[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE];
        wcscpy_s(ReparseSrcTarget, HUGE_PATH, PATH_PARSE_SWITCHOFF);

        // Retrieve the location where the junction points to
        // [EXAMPLE] ReparseSrcTarget == "\\?\N:\MultiSourceUnroll\source\F0\F0_F1\F0_F1_F1"
        int ReparsePointType = ProbeReparsePoint(pF->m_FileName, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);
        
        // With Mounpoint we receive \\?\Volume... as target, but we already have prepended \\?\ so 
        // we have to get rid of one \\?\ 
        if (REPARSE_POINT_MOUNTPOINT == ReparsePointType)
          wcscpy_s(ReparseSrcTarget, HUGE_PATH, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]);

        // It might happen, that there are relative symbolic links
        if ( PathIsRelative(&ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE]) )
        {
          // Resolve the symlinks
          WCHAR ReparseSrcTargetFullPath[HUGE_PATH];
          ReparseSrcTargetFullPath[0] = 0x00;
          
          int iResult = ResolveSymboliclink(pF->m_FileName, &ReparseSrcTarget[PATH_PARSE_SWITCHOFF_SIZE], ReparseSrcTargetFullPath);
          if (ERROR_SUCCESS == iResult)
          {
            // This is for WindowsXP only, because PathCanonicalize behaves differently on Windows7 and XP
            // Windows7 cuts out a \\? prefixed path, but XP does not which would result here in \\?\\?\ 
            int CanonPos = 0;
            if (IsVeryLongPath(ReparseSrcTargetFullPath))
              CanonPos = PATH_PARSE_SWITCHOFF_SIZE;

            // Unfortunately PathCanonicalize() cuts out \\?\ so we have to prepend it, but only if it no UNC Path
            int PathParseSwitchOff_ReparseSrcTarget = 0;
            if (!PathIsUNC(&ReparseSrcTargetFullPath[CanonPos]))
              PathParseSwitchOff_ReparseSrcTarget = PATH_PARSE_SWITCHOFF_SIZE;

            PathCanonicalize(&ReparseSrcTarget[PathParseSwitchOff_ReparseSrcTarget], &ReparseSrcTargetFullPath[CanonPos]);

            ReparsePointRelative = true;
          }
        }
        // Here we should end up in any case with ReparseSrcTarget beeing a long absolute path

        if (REPARSE_POINT_FAIL != ReparsePointType)
        {
          // See if the DestPath has changed since last time
          if (pF->m_DestPathIdx != DestPathIdx)
          {
            DestPathLen = m_AnchorPathCache.GetDestPath(DestPath, pF->m_DestPathIdx);
            DestPathIdx = pF->m_DestPathIdx;

            // Check if the destination is a very long path. It could also be a UNC Path
            if (IsVeryLongPath(DestPath))
              PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
            else
              // UNC Path
              PathParseSwitchOffSize = 0;

            // Check if destination directory already exists
            if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(DestPath))
            {
              int rResult = CreateDirectoryHierarchy(DestPath, DestPathLen);
              if (ERROR_SUCCESS == rResult)
              {
                pStats->m_DirectoryCreated++;
                Log(L"+d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
              }
              else
              {
                pStats->m_DirectoryCreateFailed++;
                PathNameStatus pns(PlusD, &DestPath[PathParseSwitchOffSize], rResult);
              }
            }
          }

          // Check if it is a inner or outer reparse point, by searching the source path in the ReparseSrcTarget
          // 
          // [EXAMPLE] 
          // if 
          //   "\\?\n:\MultiSourceUnroll\source\F1" == pF->m_FileName
          // found in 
          //   "\\?\N:\MultiSourceUnroll\source\F0\F0_F1\F0_F1_F1" == ReparseSrcTarget
          // then
          //   inner-junction
          // 
          // This example ends in an outer reparse point
          //
          // [EXAMPLE/] 
          //

          wchar_t AlternativeDestPath[HUGE_PATH];
          int AlternativeDestPathIdx = -1;
          wchar_t* MultiSourcePath = m_AnchorPathCache.ContainsSource(
            ReparseSrcTarget, 
            AlternativeDestPath, 
            &AlternativeDestPathIdx
          );

          int SourcePathLen = pF->m_SourcePathLen;

          // We have to save the character here, because it might be that
          // SourcePathLen already points to the end of the string, and then
          // later on we don't want to replace the terminating 0x00 with '\'
          wchar_t SourcePathSaveChar = pF->m_FileName[SourcePathLen];

          // Copy out the 'pure' sourcepath from the original source path, so that
          // we can do the wcseistr() to find out if it is a inner or outer junction
          // [EXAMPLE] pF->m_FileName == "\\?\n:\MultiSourceUnroll\source\F1"
          wchar_t PureSource[HUGE_PATH];
          wcsncpy_s(PureSource, HUGE_PATH, pF->m_FileName, SourcePathLen);
          PureSource[SourcePathLen] = 0x00;

          if (MultiSourcePath)
          {
            // Inner Reparsepoints

            WCHAR	ReparseDestTarget[HUGE_PATH];
            wcscpy_s(ReparseDestTarget, HUGE_PATH, DestPath);
            wchar_t* pReparseSrcTarget = &ReparseSrcTarget[SourcePathLen];


            // The path, where the reparse points to, contains the source ==> inner reparse point,
            // but we need to know the reason for the 'innerness', because it could just
            // be an inner reparse point due beeing on the MultiSourceCache, but it could be also an
            // 'pure' inner reparse point because PureSource was found in ReparseSrcTarget

            wchar_t* InnerReparsePoint = wcseistr(ReparseSrcTarget, PureSource);
            if (InnerReparsePoint)
            {
              // This is a pure inner reparse point, and all the coding below will work
            }
            else
            {
              // This is a inner reparse point, which is also fine, but if multiple destinations
              // are given, we end up here, because it is an inner reparse point due to MultiSourcePath
              // but the DestPath are different. So it can happen, that we have a reparse point, which 
              // points across DestPath. 
              // e.g.
              //   PureSource	==        "\\?\N:\MultiDest\source\F0\F0_J0" points to ReparseSrcTarget
              //   ReparseSrcTarget	==  "\\?\N:\MultiDest\source\F1\F1_F0" and has
              //   DestPath	==          "\\?\N:\MultiDest\dest\F0" as its DestPath
              //
              //   ReparseDestTarget == "\\?\N:\MultiDest\dest\F0\F1_F0"
              //
              // By cutting out \F1_F0 from ReparseSrcTarget, and appending to ReparseDestTarget
              // this ends up in a wrong reparse point: "\\?\N:\MultiDest\dest\F0\F1_F0"
              // which should be "\\?\N:\MultiDest\dest\F***1***\F1_F0"
              //
              // This problem only occurs with LSE, because one can specify multiple source, and due 
              // to LSE behaviour also multiple destinations. Well the --destination in ln.exe is only
              // here to simulate this, and thus be able to have an automated testcase
              
              // So now for the solution of this problem:
              // Multiple SourcePath and multiple DestPath are held in AnchorPathCache. For each SourcePath
              // there is a DestPath in AnchorPathCache. If we search if a reparse point is a inner or outer 
              // reparse point we do this by checking if any of the source pathes from AnchorPathCache 
              // are part of the reparse point location via ContainsSource().
              // If this matches, we know if it is a inner reparse point.
              // If it is a pure inner reparse point, we are fine, but if it is just a inner reparse
              // point, we have to switch the Destpath as below.
              //
              // So in this case, when a reparse point 'crosses' and is on a different DestPath, and
              // there are many DestPath, we have to switch to the Destpath which is related to the 
              // SourcePath that was searched for in ContainsSource()
              // 
              // We will also fall in this branch if there is only one destination and multiple sources
              // but the AlternativeDestPath will alwys be the same. So it is fine, and does no harm.
              //
              // Why can DestPath be different? Think of LSE when it generates a different name because 
              // a folder that already exists, like 'Junction of foo/', then the source is 'foo/' and
              // the destination is 'Junction of foo/'
              //
              if (AlternativeDestPathIdx != DestPathIdx)
              {
                wcscpy_s(ReparseDestTarget, HUGE_PATH, AlternativeDestPath);
                pReparseSrcTarget = MultiSourcePath;
              }
            }

            // Assemble the junction destination 
            if (pF->m_ReparseSrcTargetHint)
              pReparseSrcTarget = pF->m_ReparseSrcTargetHint;

            wcscat_s(ReparseDestTarget, HUGE_PATH, pReparseSrcTarget);

            if (apContext)
            {
              // Set the status and check if CopyReparsePoints has been canceled from outside
              int r = apContext->PutStatus(&pF->m_FileName[PathParseSwitchOffSize], &DestPath[PathParseSwitchOffSize]);
              if (ERROR_REQUEST_ABORTED == r)
              {
                RetVal = ERROR_REQUEST_ABORTED;
                break;
              }
            }

            // TBD Muessen wir da auch den PFad erstellen, wenn ein Kopiervorgang
            // nur aus Junctions besteht.???? Wie bei CopyDirectories

            // Append the source to DestPath 
            wcscat_s(DestPath, HUGE_PATH, &(pF->m_FileName[SourcePathLen]));

            // Do the Junction Target
            // ok destination is there, lets create the ReparsePoint
            int result = ERROR_SUCCESS;
            size_t SecurityResult = ERROR_SUCCESS; 

            switch (ReparsePointType)
            {
              case REPARSE_POINT_JUNCTION:
              {
                wchar_t*  SourceFilename = NULL;
                switch (aFlags & (eSmartMove | eSmartMirror | eSmartCopy))
                {
                  case eSmartMove:
                  {
                    // With rename we also do operate on dead junctions
                    RemoveDirectory(pF->m_FileName);

                    SourceFilename = pF->m_FileName;
                    int rResult = ERROR_SUCCESS;
                    // TODO Why can't we delete this with Windows7. IF we do regression fails in t_SmartMove02.bat horribly
                    
                    // WindowsXP can not create dangling junctions. One would get 0x1128 as error code
                    // for illegal reparse data. So only for XP, we have to create the directory where the
                    // junction points to, create the junction and later on remove the whole tree
                    if (gVersionInfo.dwMajorVersion < 6)
                      rResult = CreateDirectoryHierarchy(ReparseDestTarget, wcslen(ReparseDestTarget));

                    if (ERROR_SUCCESS == rResult)
                      // CreateJunction implictly is long path safe so we use [PATH_PARSE_SWITCHOFF_SIZE]
                      result = CreateJunction(pF->m_FileName, &ReparseDestTarget[PATH_PARSE_SWITCHOFF_SIZE]);
                  }
                  break;

                  case eSmartMirror:
                  {
                    // Proceed with the next file and do nothing if DestPath is newer
                    FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
                    if (pLookAside)
                    {
                      // found
                      if (false == pF->IsDifferent(pLookAside, m_DateTimeTolerance))
                      {
                        // Do nothing, the already existing Junction is newer
                        SourceFilename = DestPath;
                        result = ERROR_ALREADY_EXISTS;
                        break;
                      }
                      else
                      {
                        // Check if the type has changed
                        if (pF->m_Type != pLookAside->m_Type)
                        {
                          DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                        }
                        else
                        {
                          // There is a rare situation, but it is there: The m_Types can be both 0x410
                          // but it can't be decided if it is a Junction or Symbolic Link. If it is a symbolic
                          // link we don't want to print a message, but if it is a Junction or maybe in the future
                          // a mountpoint, we would like to notify the user on the change of type

                          int ReparsePointType = ProbeReparsePoint(DestPath, NULL);
                          switch(ReparsePointType)
                          {
                            case REPARSE_POINT_SYMBOLICLINK:
                              DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                            break;

                            case REPARSE_POINT_JUNCTION:
                              RemoveDirectory(DestPath);
                              // TBD We might not have the permissions
                            break;

                            case REPARSE_POINT_MOUNTPOINT:
                              DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                              // TBD We might not have the permissions
                            break;

                            default:
                              // unknown reparse point deleted
                              DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                            break;
                          }
                        }
                      }
                    }
                  }
                  // break intentionally ommitted

                  case eSmartCopy:
                  {
                    result = CopyReparsePoints_Junction_SmartCopy(
                      DestPath,
                      ReparseDestTarget,
                      &SourceFilename,
                      pStats,
                      aPathNameStatusList
                    );
                    size_t SecurityResult = ERROR_SUCCESS;
                    if (ERROR_SUCCESS == result && m_Flags & eBackupMode)
                      SecurityResult = CopySecurityAttributesByName(
                        pF->m_FileName, 
                        DestPath, 
                        &m_pSecDesc,
                        &m_SecDescSize,
                        FILE_FLAG_OPEN_REPARSE_POINT
                      );

                    //
                    // It could be a junction pointing to a junction, which might fail, because
                    // the junction could not be there because it will be created in the future,
                    // because it is e.g after the current junction in alphabectical order:
                    // e.g A_Junction pointing to Z_Junction will most propably fail
                    //
                    if (ERROR_PATH_NOT_FOUND == result)
                    {
                      int JunctionResult = ERROR_SUCCESS;

                      // If this finally was a dead reparse point, then add it to the statistics
                      if (FILE_ATTRIBUTE_DEAD_REPARSE_POINT & pF->m_Type)
                      {
                        // Create dead junction
                        JunctionResult = CreateJunction(
                          DestPath, 
                          &ReparseDestTarget[PATH_PARSE_SWITCHOFF_SIZE]
                        );

                        // If creation of dangling junctions would fail, we would get ERROR_INVALID_REPARSE_DATA == 4392L (0x1128)
                        if (ERROR_INVALID_REPARSE_DATA == JunctionResult)
                          result = ERROR_BAD_PATHNAME;

                        // Set back the dead attribute
                        pF->m_Type &= ~FILE_ATTRIBUTE_DEAD_REPARSE_POINT;
                        break;
                      }
                      
                      // If we can not create the reparse point we have 4 choices
                      // a) the reparse point is dead
                      // b) it is a nested reparse points
                      // c) it is a hidden reparse points
                      // d) circular reparse points
                      // 
                      // b) and c) can be resolved, but we want to know also a) and d)
                      // The solution is to postpone such reparse point, and try it again.
                      // We do this until the number of postponed reparse points shrinks for
                      // each iteration. If it stays the same we break, and decide that the
                      // remaining reparse points are a) and d)
                      pF->m_Type |= FILE_ATTRIBUTE_NESTED_REPARSE_POINT;

                      // Since the creation of the nested reparse point might work in the next pass
                      // we don't want to have statistics on this now. CopyReparsePoints_Junction_Statistics()
                      // does nothing if SourceFilename == NULL
                      SourceFilename = NULL;
                    }
                    else
                    {
                      // If it was a nested reparse point, then set it back, so that this loop comes to an end
                      pF->m_Type &= ~FILE_ATTRIBUTE_NESTED_REPARSE_POINT;
                    }
                  }
                  break;
                } // switch (aFlags & (eSmartMove | eSmartMirror | eSmartCopy))

                CopyReparsePoints_Junction_Statistics(
                  result,
                  SourceFilename,
                  pStats,
                  aPathNameStatusList
                );
              } // case REPARSE_POINT_JUNCTION:
              break;

              case REPARSE_POINT_MOUNTPOINT:
                // There are no inner mountpoints. That's technically impossible. Is it?
              break;

              case REPARSE_POINT_SYMBOLICLINK:
              {
                // Prepare the flags for CreateSymboliclink
                DWORD SourceAttr = GetFileAttributes(pF->m_FileName);

                int aSymlinkFlags = 0;
                if (m_Flags & eKeepSymlinkRelation)
                  aSymlinkFlags = ReparsePointRelative ? SYMLINK_FLAG_RELATIVE : 0;
                else
                  aSymlinkFlags = m_Flags & eRelativeSymboliclinks;

                if (SourceAttr & FILE_ATTRIBUTE_DIRECTORY)
                  aSymlinkFlags |= SYMLINK_FLAG_DIRECTORY;
                else
                  aSymlinkFlags |= SYMLINK_FLAG_FILE;
                
                if (m_Flags2 & eUnprivSymlinkCreate)
                  aSymlinkFlags |= SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

                // See which operation should be performed
                wchar_t*  SourceFilename;
                switch (aFlags & (eSmartMove | eSmartMirror | eSmartCopy))
                {
                  case eSmartMove:
                  {
                    // It is SmartMove
                    SourceFilename = pF->m_FileName;
                    if (m_Flags & eBackupMode)
                      GetSecurityAttributesByName(
                        pF->m_FileName, 
                        &m_pSecDesc,
                        &m_SecDescSize,
                        FILE_FLAG_OPEN_REPARSE_POINT
                      );

                    if (SourceAttr & FILE_ATTRIBUTE_DIRECTORY)
                      RemoveDirectory(pF->m_FileName);
                    else
                      DeleteFile(pF->m_FileName);

                    // Create dead symbolic links, which will become valid after the move
                    result = CreateSymboliclink(DestPath, 
                      ReparseDestTarget,
                      m_Flags & eBackupMode ? aSymlinkFlags | FILE_FLAG_BACKUP_SEMANTICS : aSymlinkFlags,
                      pF->m_FileName
                    );

                    if (m_Flags & eBackupMode)
                      SetSecurityAttributesByName(pF->m_FileName, m_pSecDesc, FILE_FLAG_OPEN_REPARSE_POINT);
                  }
                  break;

                  case eSmartMirror:
                  {
                    SourceFilename = DestPath;
                    // Proceed with the next file and do nothing if DestPath is newer
                    FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
                    if (pLookAside)
                    {
                      if (false == pF->IsDifferent(pLookAside, m_DateTimeTolerance))
                      {
                        SourceFilename = DestPath;
                        result = ERROR_ALREADY_EXISTS;
                        break;
                      }
                      else
                      {
                        // If the symbolic link is already there but DestPath is different or the type
                        // has changed, it has to be deleted, because it could point to a e.g 
                        // different location
                        if (pF->m_Type != pLookAside->m_Type)
                        {
                          // The type was completley different, so we would like to report this
                          DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);

                          // break intentionally ommitted
                        }
                        else
                        {
                          // We only want to do this for Junctions and Symbolic Link Directories
                          if (pLookAside->m_Type & FILE_ATTRIBUTE_DIRECTORY)
                          {
                            // There is a rare situation, but it is there: The m_Types can be both 0x410
                            // but it can't be decided if it is a Junction or Symbolic Link. If it is a junction or 
                            // link we don't want to print a message, but if it is a Junction or maybe in the future
                            // a mountpoint, we would like to notify the user on the change of type
                            int ReparsePointType = ProbeReparsePoint(DestPath, NULL);
                            switch(ReparsePointType)
                            {
                              case REPARSE_POINT_SYMBOLICLINK:
                                // Do not report if a symlink was replaced by a symlink
                                RemoveDirectory(DestPath);
                                // TBD We might not have the permissions
                              break;

                              case REPARSE_POINT_JUNCTION:
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                                // TBD We might not have the permissions
                              break;

                              case REPARSE_POINT_MOUNTPOINT:
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                                // TBD We might not have the permissions
                              break;

                              default:
                                // unknown reparse point deleted
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                              break;
                            }
                          }
                          else
                          {
                            // For Symbolic Link Files we simply want to delete it
                            DeleteFile(DestPath);
                          }
                          
                          // break intentionally ommitted
                        } // if (pF->m_Type != pLookAside->m_Type)

                      }
                    }
                  }
                  // break intentionally ommitted

                  case eSmartCopy:
                  {
                    // It is SmartCopy/SmartClone
                    SourceFilename = DestPath;
                    
                    if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(ReparseDestTarget))
                    {
                      result = CreateSymboliclink(
                        DestPath, 
                        ReparseDestTarget,
                        m_Flags & eBackupMode ? aSymlinkFlags | FILE_FLAG_BACKUP_SEMANTICS : aSymlinkFlags
                      );
                      if (ERROR_SUCCESS == result && m_Flags & eBackupMode) 
                        SecurityResult = CopySecurityAttributesByName(
                          pF->m_FileName, 
                          DestPath, 
                          &m_pSecDesc,
                          &m_SecDescSize,
                          FILE_FLAG_OPEN_REPARSE_POINT
                        );
                    }
                    else
                      result = ERROR_PATH_NOT_FOUND;
                  
                    //
                    // It could be a junction pointing to a junction, which might fail, because
                    // the junction could not be there because it will be created in the future,
                    // because it is e.g after the current junction in alphabectical order:
                    // e.g A_Junction pointing to Z_Junction will most propably fail
                    //
                    if (ERROR_PATH_NOT_FOUND == result)
                    {
                      // If this finally was a dead reparse point, then add it to the statistics
                      if (FILE_ATTRIBUTE_DEAD_REPARSE_POINT & pF->m_Type)
                      {
                        // Create dead symlink
                        CreateSymboliclink(
                          DestPath, 
                          ReparseDestTarget,
                          m_Flags & eBackupMode ? aSymlinkFlags | FILE_FLAG_BACKUP_SEMANTICS : aSymlinkFlags,
                          DestPath
                        );

                        // Set back the dead attribute
                        pF->m_Type &= ~FILE_ATTRIBUTE_DEAD_REPARSE_POINT;
                        break;
                      }
                      
                      // If we can not create the reparse point we have 4 choices
                      // a) the reparse point is dead
                      // b) it is a nested reparse points
                      // c) it is a hidden reparse points
                      // d) circular reparse points
                      // 
                      // b) and c) can be resolved, but we want to know also a) and d)
                      // The solution is to postpone such reparse point, and try it again.
                      // We do this as long as the number of postponed reparse points shrinks for
                      // each iteration. If it stays the same we break, and decide that the
                      // remaining reparse points are a) and d)
                      pF->m_Type |= FILE_ATTRIBUTE_NESTED_REPARSE_POINT;

                      // Since the creation of the nested reparse point might work in the next pass
                      // we don't want to have statistics on this now. CopyReparsePoints_Symlink_Statistics()
                      // does nothing if SourceFilename == NULL
                      SourceFilename = NULL;
                    }
                    else
                    {
                      // If it was a nested reparse point, then set it back, so that this loop comes to an end
                      pF->m_Type &= ~FILE_ATTRIBUTE_NESTED_REPARSE_POINT;
                    }
                  }
                  break;
                }
                
                CopyReparsePoints_Symlink_Statistics(
                  result,
                  SourceFilename,
                  pStats,
                  aPathNameStatusList
                );
              }
              break;

              default:
              break;
            }
          } 
          else // if (MultiSourcePath) 
          {
            // Outer Reparsepoints

            // For outer ReparsePoints we have two choices here:
            // o) They are spliced
            // o) They are unrolled

            // Check if outer junctions symlinks should be spliced
            bool DoSplice = false;
            if (m_Flags & eAlwaysSpliceDir)
              DoSplice = true; 
            else
              if (MatchRegExpList(&pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], m_RegExpSpliceDirList))
                DoSplice = true;

            // When splicing back some can start with a junction as source directory
            // which would create a junction from 'dest' -> 'source'. But 'dest' has
            // already been created as directory, thus it can not be created as junction
            // anymore. That's why we have to abort splicing back for this case
            if (!SourcePathSaveChar)
              DoSplice = false;

            // If we have to splice then DoSplice is our inidcator, but for unroll the 
            // m_ReparseSrcTargetHint was set during enumeration
            if (DoSplice || pF->m_ReparseSrcTargetHint)
            {
              // Decide where to put the reparse targests to
              WCHAR ReparseDestTarget[HUGE_PATH];
              if (pF->m_ReparseSrcTargetHint)
              {
                // it was a outer ReparsePoint and we are here it beause it was unrolled
                wcscpy_s(ReparseDestTarget, HUGE_PATH, DestPath);
                wcscat_s(ReparseDestTarget, HUGE_PATH, pF->m_ReparseSrcTargetHint);
              }
              else
              {
                // The ReparePoints should be spliced
                wcscpy_s(ReparseDestTarget, HUGE_PATH, ReparseSrcTarget);
              }

              // Append the source to DestPath 
              wcscat_s(DestPath, HUGE_PATH, &pF->m_FileName[SourcePathLen]);  

              int result = ERROR_SUCCESS;
              size_t SecurityResult = ERROR_SUCCESS;

              // The coding below contains the case statements for REPARSE_POINT_MOUNTPOINT, REPARSE_POINT_JUNCTION and
              // REPARSE_POINT_SYMBOLICLINK is almost the same, but differs in some small details.
              // I didn't want to create one coding which has obfuscated if's in it, so I decided to 
              // copy and paste and adapt for each type. It is more for maintenance and it is easier to 
              // understand and if neccessary can be changed for one type without creating sideeffect
              // for the other types. So this is intentional.
              switch (ReparsePointType)
              {
                case REPARSE_POINT_MOUNTPOINT:
                {
                  wchar_t*  SourceFilename = NULL;

                  // With SmartMove outer Mountpoints must not be touched
                  switch (aFlags & (eSmartMirror | eSmartCopy))
                  {
                    case eSmartMirror:
                    {
                      // Proceed with the next file and do nothing if DestPath is newer
                      FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
                      if (pLookAside)
                      {
                        // found
                        if (false == pF->IsDifferent(pLookAside, m_DateTimeTolerance))
                        {
                          // Do nothing, the already existing Junction is newer
                          SourceFilename = DestPath;
                          result = ERROR_ALREADY_EXISTS;
                          break;
                        }
                        else
                        {
                          // Check if the type has changed
                          if (pF->m_Type != pLookAside->m_Type)
                          {
                            // The type was completley different, so we would like to report this
                            DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);

                            // break intentionally ommitted
                          }
                          else // if (pF->m_Type != pLookAside->m_Type)
                          {
                            // There is a rare situation, but it is there: The m_Types can be both 0x410
                            // but it can't be decided if it is a Junction or Symbolic Link. If it is a symbolic
                            // link we don't want to print a message, but if it is a Junction or maybe in the future
                            // a mountpoint, we would like to notify the user on the change of type

                            int ReparsePointType = ProbeReparsePoint(DestPath, NULL);
                            switch(ReparsePointType)
                            {
                              case REPARSE_POINT_SYMBOLICLINK:
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                                // TBD We might not have the permissions
                              break;

                              case REPARSE_POINT_JUNCTION:
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                                // TBD We might not have the permissions
                              break;

                              case REPARSE_POINT_MOUNTPOINT:
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                                // TBD We might not have the permissions
                              break;

                              default:
                                // unknown reparse point deleted
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                              break;
                            }
                          }
                        }
                      }
                    }
                    // break intentionally ommitted

                    case eSmartCopy:
                    {
                      result = CopyReparsePoints_Mountpoint_SmartCopy(
                        DestPath, 
                        ReparseDestTarget,
                        &SourceFilename,
                        pStats,
                        aPathNameStatusList
                      );
                      size_t SecurityResult = ERROR_SUCCESS;
                      if (ERROR_SUCCESS == result && m_Flags & eBackupMode)
                        SecurityResult = CopySecurityAttributesByName(
                          pF->m_FileName, 
                          DestPath, 
                          &m_pSecDesc,
                          &m_SecDescSize,
                          FILE_FLAG_OPEN_REPARSE_POINT
                        );

                      //
                      // It could be a junction pointing to a junction, which might fail, because
                      // the junction could not be there because it will be created in the future,
                      // because it is e.g after the current junction in alphabectical order:
                      // e.g A_Junction pointing to Z_Junction will fail
                      //
                      if (ERROR_PATH_NOT_FOUND == result)
                      {
                        int JunctionResult = ERROR_SUCCESS;

                        // If this finally was a dead reparse point, then add it to the statistics
                        if (FILE_ATTRIBUTE_DEAD_REPARSE_POINT & pF->m_Type)
                        {
                          // Create dead junction
                          JunctionResult = CreateJunction(
                            DestPath, 
                            &ReparseDestTarget[PATH_PARSE_SWITCHOFF_SIZE]
                          );

                          // Set back the dead attribute
                          pF->m_Type &= ~FILE_ATTRIBUTE_DEAD_REPARSE_POINT;
                          break;
                        }
                        

                        // If creation of dangling junctions would fail, we would get ERROR_INVALID_REPARSE_DATA == 4392L (0x1128)
                        if (ERROR_INVALID_REPARSE_DATA == JunctionResult)
                          result = ERROR_BAD_PATHNAME;

                        // If we can not create the reparse point we have 5 choices
                        // a) the reparse point is dead in the destination and in the source evers since
                        // b) [not yet handled] the reparse point is dead in the destinationit is alive in the source
                        // c) it is a nested reparse points
                        // d) it is a hidden reparse points
                        // e) circular reparse points
                        // 
                        // c) and d) can be resolved, but we want to know also a) and e)
                        // The solution is to postpone such reparse point, and try it again.
                        // We do this until the number of postponed reparse points shrinks for
                        // each iteration. If it stays the same we break, and decide that the
                        // remaining reparse points are a) b) and e)
                        //
                        // b) [not yet handled] can happen if one used --exclude on directories in 
                        // the source and wants to have them in via --unroll. Kind of overrule --exclude 
                        // via --unroll. Don't know how to solve this, but this would mean that a inner 
                        // reparse point converts to a outer reparse point and afterwards gets unrolled. 
                        // Tricky.
                        //
                        pF->m_Type |= FILE_ATTRIBUTE_NESTED_REPARSE_POINT;

                        // Since the creation of the nested reparse point might work in the next pass
                        // we don't want to have statistics on this now. CopyReparsePoints_Junction_Statistics()
                        // does nothing if SourceFilename == NULL
                        SourceFilename = NULL;
                      }
                      else
                      {
                        // If it was a nested reparse point, then set it back, so that this loop comes to an end
                        pF->m_Type &= ~FILE_ATTRIBUTE_NESTED_REPARSE_POINT;
                      }
                    }
                    break;
                  }

                  CopyReparsePoints_Mountpoint_Statistics(
                    result,
                    SourceFilename,
                    pStats,
                    aPathNameStatusList
                  );
                } // case REPARSE_MOUNTPOINT
                break;

                case REPARSE_POINT_JUNCTION:
                {
                  wchar_t*  SourceFilename = NULL;

                  // With SmartMove outer Symbolic Links must not be touched
                  switch (aFlags & (eSmartMirror | eSmartCopy))
                  {
                    case eSmartMirror:
                    {
                      // Proceed with the next file and do nothing if DestPath is newer
                      FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
                      if (pLookAside)
                      {
                        // found
                        if (false == pF->IsDifferent(pLookAside, m_DateTimeTolerance))
                        {
                          // Do nothing, the already existing Junction is newer
                          SourceFilename = DestPath;
                          result = ERROR_ALREADY_EXISTS;
                          break;
                        }
                        else
                        {
                          // Check if the type has changed
                          if (pF->m_Type != pLookAside->m_Type)
                          {
                            // The type was completley different, so we would like to report this
                            DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);

                            // break intentionally ommitted
                          }
                          else // if (pF->m_Type != pLookAside->m_Type)
                          {
                            // There is a rare situation, but it is there: The m_Types can be both 0x410
                            // but it can't be decided if it is a Junction or Symbolic Link. If it is a symbolic
                            // link we don't want to print a message, but if it is a Junction or maybe in the future
                            // a mountpoint, we would like to notify the user on the change of type

                            int ReparsePointType = ProbeReparsePoint(DestPath, NULL);
                            switch(ReparsePointType)
                            {
                              case REPARSE_POINT_SYMBOLICLINK:
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                                // TBD We might not have the permissions
                              break;

                              case REPARSE_POINT_JUNCTION:
                                RemoveDirectory(DestPath);
                                // TBD We might not have the permissions
                              break;

                              case REPARSE_POINT_MOUNTPOINT:
                                // TBD
                              break;

                              default:
                                // unknown reparse point deleted
                                DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                              break;
                            }
                          }
                        }
                      }
                    }
                    // break intentionally ommitted

                    case eSmartCopy:
                    {
                      result = CopyReparsePoints_Junction_SmartCopy(
                        DestPath, 
                        ReparseDestTarget,
                        &SourceFilename,
                        pStats,
                        aPathNameStatusList
                      );
                      size_t SecurityResult = ERROR_SUCCESS;
                      if (ERROR_SUCCESS == result && m_Flags & eBackupMode)
                        SecurityResult = CopySecurityAttributesByName(
                          pF->m_FileName, 
                          DestPath, 
                          &m_pSecDesc,
                          &m_SecDescSize,
                          FILE_FLAG_OPEN_REPARSE_POINT
                        );

                      //
                      // It could be a junction pointing to a junction, which might fail, because
                      // the junction could not be there because it will be created in the future,
                      // because it is e.g after the current junction in alphabectical order:
                      // e.g A_Junction pointing to Z_Junction will fail
                      //
                      if (ERROR_PATH_NOT_FOUND == result)
                      {
                        int JunctionResult = ERROR_SUCCESS;

                        // If this finally was a dead reparse point, then add it to the statistics
                        if (FILE_ATTRIBUTE_DEAD_REPARSE_POINT & pF->m_Type)
                        {
                          // Create dead junction
                          JunctionResult = CreateJunction(
                            DestPath, 
                            &ReparseDestTarget[PATH_PARSE_SWITCHOFF_SIZE]
                          );

                          // Set back the dead attribute
                          pF->m_Type &= ~FILE_ATTRIBUTE_DEAD_REPARSE_POINT;
                          break;
                        }
                        

                        // If creation of dangling junctions would fail, we would get ERROR_INVALID_REPARSE_DATA == 4392L (0x1128)
                        if (ERROR_INVALID_REPARSE_DATA == JunctionResult)
                          result = ERROR_BAD_PATHNAME;

                        // If we can not create the reparse point we have 5 choices
                        // a) the reparse point is dead in the destination and in the source evers since
                        // b) [not yet handled] the reparse point is dead in the destinationit is alive in the source
                        // c) it is a nested reparse points
                        // d) it is a hidden reparse points
                        // e) circular reparse points
                        // 
                        // c) and d) can be resolved, but we want to know also a) and e)
                        // The solution is to postpone such reparse point, and try it again.
                        // We do this until the number of postponed reparse points shrinks for
                        // each iteration. If it stays the same we break, and decide that the
                        // remaining reparse points are a) b) and e)
                        //
                        // b) [not yet handled] can happen if one used --exclude on directories in 
                        // the source and wants to have them in via --unroll. Kind of overrule --exclude 
                        // via --unroll. Don't know how to solve this, but this would mean that a inner 
                        // reparse point converts to a outer reparse point and afterwards gets unrolled. 
                        // Tricky.
                        //
                        pF->m_Type |= FILE_ATTRIBUTE_NESTED_REPARSE_POINT;

                        // Since the creation of the nested reparse point might work in the next pass
                        // we don't want to have statistics on this now. CopyReparsePoints_Junction_Statistics()
                        // does nothing if SourceFilename == NULL
                        SourceFilename = NULL;
                      }
                      else
                      {
                        // If it was a nested reparse point, then set it back, so that this loop comes to an end
                        pF->m_Type &= ~FILE_ATTRIBUTE_NESTED_REPARSE_POINT;
                      }
                    }
                    break;
                  }

                  CopyReparsePoints_Junction_Statistics(
                    result,
                    SourceFilename,
                    pStats,
                    aPathNameStatusList
                  );
                } // case REPARSE_POINT_JUNCTION:
                break;

                case REPARSE_POINT_SYMBOLICLINK:
                {
                  // Prepare the flags for CreateSymboliclink
                  DWORD SourceAttr = GetFileAttributes(pF->m_FileName);

                  int aSymlinkFlags = 0;
                  if (m_Flags & eKeepSymlinkRelation)
                    aSymlinkFlags = ReparsePointRelative ? SYMLINK_FLAG_RELATIVE : 0;
                  else
                    aSymlinkFlags = m_Flags & eRelativeSymboliclinks;

                  if (SourceAttr & FILE_ATTRIBUTE_DIRECTORY)
                    aSymlinkFlags |= SYMLINK_FLAG_DIRECTORY;
                  else
                    aSymlinkFlags |= SYMLINK_FLAG_FILE;
                  
                  if (m_Flags2 & eUnprivSymlinkCreate)
                    aSymlinkFlags |= SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

                  // See which operation should be performed
                  wchar_t*  SourceFilename;

                  // With SmartMove outer Symbolic Links must not be touched
                  switch (aFlags & (eSmartMirror | eSmartCopy))
                  {
                    case eSmartMirror:
                    {
                      SourceFilename = DestPath;
                      // Proceed with the next file and do nothing if DestPath is newer
                      FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
                      if (pLookAside)
                      {
                        // found
                        if (false == pF->IsDifferent(pLookAside, m_DateTimeTolerance))
                        {
                          // Do nothing, the already existing Junction is newer
                          SourceFilename = DestPath;
                          result = ERROR_ALREADY_EXISTS;
                          break;
                        }
                        else
                        {

                          // Check if the type has changed
                          if (pF->m_Type != pLookAside->m_Type)
                          {
                            // The type was completley different, so we would like to report this
                            DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);

                            // break intentionally ommitted
                          }
                          else // if (pF->m_Type != pLookAside->m_Type)
                          {
                            // We only want to do this for Junctions and Symbolic Link Directories
                            if (pLookAside->m_Type & FILE_ATTRIBUTE_DIRECTORY)
                            {
                              // There is a rare situation, but it is there: The m_Types can be both 0x410
                              // but it can't be decided if it is a Junction or Symbolic Link. If it is a junction or 
                              // link we don't want to print a message, but if it is a Junction or maybe in the future
                              // a mountpoint, we would like to notify the user on the change of type

                              int ReparsePointType = ProbeReparsePoint(DestPath, NULL);
                              switch(ReparsePointType)
                              {
                                case REPARSE_POINT_SYMBOLICLINK:
                                  // Do not report if a symlink was replaced by a symlink
                                  RemoveDirectory(DestPath);
                                  // TBD We might not have the permissions
                                break;

                                case REPARSE_POINT_JUNCTION:
                                  DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                                  // TBD We might not have the permissions
                                break;

                                case REPARSE_POINT_MOUNTPOINT:
                                  // TBD
                                break;

                                default:
                                  // unknown reparse point deleted
                                  DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                                break;
                              }
                            }
                            else // if (pF->m_Type != pLookAside->m_Type)
                            {
                              // For Symbolic Link Files we simply want to delete it
                              DeleteFile(DestPath);
                            }
                            
                            // break intentionally ommitted
                          }

                        }
                      } // if (pLookAside)
                    }
                    // break intentionally ommitted

                    case eSmartCopy:
                    {
                      // It is SmartCopy/SmartClone
                      SourceFilename = DestPath;
                      if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(ReparseDestTarget))
                      {
                        result = CreateSymboliclink(
                          DestPath, 
                          ReparseDestTarget,
                          m_Flags & eBackupMode ? aSymlinkFlags | FILE_FLAG_BACKUP_SEMANTICS : aSymlinkFlags
                        );
                        if (ERROR_SUCCESS == result && m_Flags & eBackupMode)
                          SecurityResult = CopySecurityAttributesByName(
                            pF->m_FileName, 
                            DestPath, 
                            &m_pSecDesc,
                            &m_SecDescSize,
                            FILE_FLAG_OPEN_REPARSE_POINT
                          );
                      }
                      else
                        result = ERROR_PATH_NOT_FOUND; 

                      //
                      // It could be a junction pointing to a junction, which might fail, because
                      // the junction could not be there because it will be created in the future,
                      // because it is e.g after the current junction in alphabectical order:
                      // e.g A_Junction pointing to Z_Junction will most propably fail
                      //
                      if (ERROR_PATH_NOT_FOUND == result)
                      {
                        // If this finally was a dead reparse point, then add it to the statistics
                        if (FILE_ATTRIBUTE_DEAD_REPARSE_POINT & pF->m_Type)
                        {
                          // Create dead symlink
                          CreateSymboliclink(
                            DestPath, 
                            ReparseDestTarget,
                            m_Flags & eBackupMode ? aSymlinkFlags | FILE_FLAG_BACKUP_SEMANTICS : aSymlinkFlags,
                            DestPath
                          );
                          
                          // Set back the dead attribute
                          pF->m_Type &= ~FILE_ATTRIBUTE_DEAD_REPARSE_POINT;
                          break;
                        }
                        
                        // If we can not create the reparse point we have 5 choices
                        // a) the reparse point is dead in the destination and in the source evers since
                        // b) [not yet handled] the reparse point is dead in the destinationit is alive in the source
                        // c) it is a nested reparse points
                        // d) it is a hidden reparse points
                        // e) circular reparse points
                        // 
                        // c) and d) can be resolved, but we want to know also a) and e)
                        // The solution is to postpone such reparse point, and try it again.
                        // We do this until the number of postponed reparse points shrinks for
                        // each iteration. If it stays the same we break, and decide that the
                        // remaining reparse points are a) b) and e)
                        //
                        // b) [not yet handled] can happen if one used --exclude on directories in 
                        // the source and wants to have them in via --unroll. Kind of overrule --exclude 
                        // via --unroll. Don't know how to solve this, but this would mean that a inner 
                        // reparse point converts to a outer reparse point and afterwards gets unrolled. 
                        // Tricky.
                        //
                        pF->m_Type |= FILE_ATTRIBUTE_NESTED_REPARSE_POINT;

                        // Since the creation of the nested reparse point might work in the next pass
                        // we don't want to have statistics on this now. CopyReparsePoints_Symlink_Statistics()
                        // does nothing if SourceFilename == NULL
                        SourceFilename = NULL;
                      }
                      else
                      {
                        // If it was a nested reparse point, then set it back, so that this loop comes to an end
                        pF->m_Type &= ~FILE_ATTRIBUTE_NESTED_REPARSE_POINT;
                      }
                    }
                    break;
                  }

                  CopyReparsePoints_Symlink_Statistics(
                    result,
                    SourceFilename,
                    pStats,
                    aPathNameStatusList
                  );
                } // case REPARSE_POINT_SYMBOLICLINK:
                break;
              }
            } 
            else // if (DoSplice || pF->m_ReparseSrcTargetHint)
            {
              // We wanted to splice but the regexp didn't allow it to us
              // but anyway we have to keep track of the statistics. 
              // 
              // But Smartmove also ends here, because it does nothing with
              // outer reparse points, so there is no need to count items
              // for the statistics with SmartMove
              if (!DoSplice && !(aFlags & eSmartMove))
              {
                // It ends up here because it was a outer reparse point, and the reparse points
                // were not spliced, and we are also not in SmartMove. So this is cropping

                // Check if the destination is a very long path. It could also be a UNC Path
                int PathParseSwitchOffSize;
                if (IsVeryLongPath(pF->m_FileName))
                  PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
                else
                  // UNC Path
                  PathParseSwitchOffSize = 0;

                switch (ReparsePointType)
                {
                  case REPARSE_POINT_JUNCTION:
                    pStats->m_JunctionsCropped++;
                    Log(L"#j %s\n", &pF->m_FileName[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                  break;

                  case REPARSE_POINT_SYMBOLICLINK:
                    pStats->m_SymlinksCropped++;
                    Log(L"#s %s\n", &pF->m_FileName[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                  break;

                  case REPARSE_POINT_MOUNTPOINT:
                    pStats->m_MountpointsCropped++;
                    Log(L"#m %s\n", &pF->m_FileName[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                  break;

                  default:
                    // unknown reparse point cropped
                    pStats->m_ReparsePointCropped++;
                    Log(L"#r %s\n", &pF->m_FileName[PathParseSwitchOffSize], m_Flags, eLogVerbose);
                  break;

                }

                // But... The story goes on. If we are in Mirror, it might happen that at the location in the 
                // destination, where the reparse-point should go to, an item already exists, and only then we 
                // have to delete this very item in the destination
                if (aFlags & eSmartMirror)
                {
                  // Append the source to DestPath 
                  wcscat_s(DestPath, HUGE_PATH, &pF->m_FileName[SourcePathLen]);  

                  FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
                  if (pLookAside)
                  {
                    if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(DestPath))
                      DeleteItem(DestPath, pLookAside, aPathNameStatusList, pStats);
                  }
                }

              } // if (!DoSplice && !(aFlags & eSmartMove))
            } // if (DoSplice || pF->m_ReparseSrcTargetHint)
          }

          // Restore DestPath for the next string concatenate
          DestPath[DestPathLen] = 0x00;

        } // if (REPARSE_POINT_FAIL != ReparsePointType)
        else
        {
          pStats->m_ReparsePointRestoreFailed++;
          PathNameStatus pns(PlusR, &pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], ERROR_REPARSE_TAG_INVALID);
          aPathNameStatusList->push_back(pns);
        }
      
        // At the end of this iteration delete the content of m_ReparseSrcTargetHint
        // because it will not be used anymore except for the situation if it was a nested
        // reparse point, which is processed in the next iteration of this loop. So in that
        // case keep it.
        if (pF->m_ReparseSrcTargetHint && (!(pF->m_Type & FILE_ATTRIBUTE_NESTED_REPARSE_POINT)) )
        {
          pStats->m_HeapDeletionTime.Start();
          delete [] pF->m_ReparseSrcTargetHint;
          pStats->m_HeapDeletionTime.Stop();

          pF->m_ReparseSrcTargetHint = NULL;
        }
      } // for (iter = ReparseBegin; iter != ReparseEnd; ++iter)

      // Split the remaining reparse point in already processed reparse points and NESTED_REPARSE_POINT
      // The remaining nested reparse points, mut be processed
      ReparseBegin = partition (ReparseBegin, ReparseEnd, IsNestedReparsePoint());
      CurrentReparseSize = distance(ReparseBegin, ReparseEnd);
    
      if (apContext)
      {
        // Increment the progress by the created reparse points
        size_t ReparsePointsCreated = LastReparseSize - CurrentReparseSize;
        apContext->AddProgress(ReparsePointsCreated * eReparseWeight, ReparsePointsCreated, ReparsePointsCreated);
#if defined _DEBUG
        count += ReparsePointsCreated;
#endif
      }

    } while (CurrentReparseSize < LastReparseSize);

    // Catch the break and propagate it through the loops
    if (ERROR_REQUEST_ABORTED == RetVal)
      break;

    // All reparse point which are leftover now, are dead reparse points
    for (_Pathes::iterator iter = ReparseBegin; iter != ReparseEnd; ++iter)
    {
      FileInfo* pF = *iter;
      pF->m_Type |= FILE_ATTRIBUTE_DEAD_REPARSE_POINT;
      pF->m_Type &= ~FILE_ATTRIBUTE_NESTED_REPARSE_POINT;
    }
  } 
  return RetVal;
}

int
FileInfoContainer::
RestoreTimeStamps(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext,
  CopyReparsePointFlags     aFlags
)
{
  int RetVal = ERROR_SUCCESS;

  if (aBegin == aEnd)
    return RetVal;

  WCHAR	DestPath[HUGE_PATH] = { 0 };
  size_t  DestPathLen = 0;

  // Directories have to be sorted in Descending Order now
  // for applying the timestamps properly
  sort (aBegin, aEnd, PathIdxFilenameSorterDescending());

  // start with an invalid value
  int DestPathIdx = -1;

#if defined _DEBUG
  size_t effort = distance(aBegin, aEnd);
  int count = 0;
#endif

  // Restore timestamps on directories and junctions
  for (_Pathes::iterator iter = aBegin; iter != aEnd; ++iter)
  {
    FileInfo*	pF = *iter;

    if (pF->m_DestPathIdx != DestPathIdx)
    {
      DestPathLen = m_AnchorPathCache.GetDestPath(DestPath, pF->m_DestPathIdx);
      DestPathIdx = pF->m_DestPathIdx;
    }

    wcscat_s(DestPath, HUGE_PATH, &(pF->m_FileName[pF->m_SourcePathLen]));

    if (apContext)
    {
      // Increment the progress
      apContext->AddProgress(eTimeStampWeight, 1, 0);

#if defined _DEBUG
      count++;
#endif

      // Set the status, and check if RestoreTimeStamps has been cancelled from outside
      int r = apContext->PutStatus(&pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], &DestPath[PATH_PARSE_SWITCHOFF_SIZE]);
      if ( ERROR_REQUEST_ABORTED == r)
      {
        RetVal = ERROR_REQUEST_ABORTED;
        break;
      }
    }

    bool DoFixAttr = true;
    if (aFlags & eSmartMirror)
    {
      // If we are running in SmartMirror the timestamps have not to be restored    
      // if the file already exists in the destination and is not older
      FileInfo* pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
      if (pLookAside)
        DoFixAttr = pF->IsDifferent(pLookAside, m_DateTimeTolerance);
    }

    if (DoFixAttr)
    {
      // Restore the date/time of the directory or junction
      // With Smartmove we must do the timestamp repair within the original location, but 
      // with everything else than SmartMove the timestamps must be restored in the destination
      HANDLE fh = CreateFileW (aFlags & eSmartMove ? pF->m_FileName : DestPath, 
        GENERIC_READ | FILE_WRITE_ATTRIBUTES, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        pF->IsReparsePoint() ? FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS : FILE_FLAG_BACKUP_SEMANTICS, 
        NULL);

      if (INVALID_HANDLE_VALUE != fh)
      {
        SetFileBasicInformation(fh, pF->m_Type, &pF->m_CreationTime.FileTime, &pF->m_LastAccessTime.FileTime, &pF->m_LastWriteTime.FileTime);
        CloseHandle (fh);
      }
    }

    // restore Destination FullPath for the next string concatenate
    DestPath[DestPathLen] = 0x00;
  }

  return RetVal;
}

int
FileInfoContainer::
CopyDirectoryAttributes(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext,
  CopyReparsePointFlags     aFlags
)
{
  int RetVal = ERROR_SUCCESS;

  if (aBegin == aEnd)
    return RetVal;

  WCHAR	DestPath[HUGE_PATH] = { 0 };
  size_t  DestPathLen = 0;

  // Directories have to be sorted in Descending Order now
  // for applying the timestamps properly
  sort (aBegin, aEnd, PathIdxFilenameSorterDescending());

  // start with an invalid value
  int DestPathIdx = -1;

  int CopyDirFlags = COPY_FILE_COPY_DIRECTORY | COPY_FILE_COPY_WRITE_TIME;
  if (m_Flags & eBackupMode)
    CopyDirFlags |= COPY_FILE_COPY_SACL | COPY_FILE_COPY_ACCESS_TIME | COPY_FILE_COPY_CREATION_TIME;
  if (aFlags & eSmartMirror)
    CopyDirFlags |= COPY_FILE_COPY_ACCESS_TIME | COPY_FILE_COPY_CREATION_TIME;

#if defined _DEBUG
  size_t effort = distance(aBegin, aEnd);
  int count = 0;
#endif

  // Restore timestamps on directories and junctions
  for (_Pathes::iterator iter = aBegin; iter != aEnd; ++iter)
  {
    FileInfo*	pF = *iter;

    if (pF->m_DestPathIdx != DestPathIdx)
    {
      DestPathLen = m_AnchorPathCache.GetDestPath(DestPath, pF->m_DestPathIdx);
      DestPathIdx = pF->m_DestPathIdx;
    }

    if (apContext)
    {
      // Increment the progress
      apContext->AddProgress(eTimeStampWeight, 1, 0);
#if defined _DEBUG
      count++;
#endif
    }

    wcscat_s(DestPath, HUGE_PATH, &(pF->m_FileName[pF->m_SourcePathLen]));
    if (INVALID_FILE_ATTRIBUTES != GetFileAttributes(DestPath))
    {
      if (apContext)
      {
        size_t PathParseSwitchOffSize_Source = 0;
        if (IsVeryLongPath(pF->m_FileName))
          PathParseSwitchOffSize_Source = PATH_PARSE_SWITCHOFF_SIZE;

        size_t PathParseSwitchOffSize_Dest = 0;
        if (IsVeryLongPath(DestPath))
          PathParseSwitchOffSize_Dest = PATH_PARSE_SWITCHOFF_SIZE;

        // TODO: Vielleicht sollte man die PathParseSwitch Check in das PutStatus verlegen, denn für die LSE wird es dort
        // gebraucht um sauber auszugeben. Andererseits wird es auch für das Logging vom ln.exe gebraucht.... Vl sollte man es
        // auch dort zentral machen, denn es ist ja wirklich nur für das Logging bzw die Ansicht wichtig... Hmm nicht immer

        // Set the status, and check if RestoreTimeStamps has been cancelled from outside
        int r = apContext->PutStatus(&pF->m_FileName[PathParseSwitchOffSize_Source], &DestPath[PathParseSwitchOffSize_Dest]);
        if ( ERROR_REQUEST_ABORTED == r)
        {
          RetVal = ERROR_REQUEST_ABORTED;
          break;
        }
      }

      // Restore Alternative Streams & EA records of directories
      BOOL bCancel = FALSE;
      _PathNameStatusList        CopyFileError;
      CopyFileEx3(pF->m_FileName,
        DestPath,
        NULL, 
        NULL,
        &bCancel,
        CopyDirFlags,  
        &m_pSecDesc,
        &m_SecDescSize,
        m_DateTimeTolerance,
        &CopyFileError
      );
    }

    // restore Destination FullPath for the next string concatenate
    DestPath[DestPathLen] = 0x00;
  }

  return RetVal;
}

int
FileInfoContainer::
CloneFiles(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  CopyStatistics*		        pStats,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext
)
{
  int RetVal = ERROR_SUCCESS;

  // check for empty set
  if (aBegin == aEnd)
    return ERROR_SUCCESS;

  wchar_t DestPath[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE] = { 0 };
  size_t  DestPathLen{ 0 };

  // start with an invalid value
  int DestPathIdx = -1;

  int PathParseSwitchOffSize = 0;
  
  // Set up the copyFlags, because we might have to copy over if the HardlinkLimit is exceeded
  int CopyFlags = COPY_FILE_COPY_WRITE_TIME | COPY_FILE_FAIL_IF_EXISTS;
  if (m_Flags & eBackupMode)
    CopyFlags |= COPY_FILE_COPY_SACL | COPY_FILE_COPY_CREATION_TIME | COPY_FILE_COPY_ACCESS_TIME;

  if (m_Flags2 & eNoAds)
    CopyFlags |= COPY_FILE_COPY_SKIP_ADS;

  if (m_Flags2 & eNoEa)
    CopyFlags |= COPY_FILE_COPY_SKIP_EA;

#if defined _DEBUG
  size_t effort = distance(aBegin, aEnd);
  int count = 0;
#endif

  // Go through all files
  _Pathes::iterator iter = aBegin;
  do
  {
    FileInfo*	pF = *iter;
    ++iter;

    // See if the DestPath has changed since the last time
    if (pF->m_DestPathIdx != DestPathIdx)
    {
      DestPathLen = m_AnchorPathCache.GetDestPath(DestPath, pF->m_DestPathIdx);
      DestPathIdx = pF->m_DestPathIdx;

      // Check if the destination is a very long path. It could also be a UNC Path
      if (IsVeryLongPath(DestPath))
        PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
      else
        // UNC Path
        PathParseSwitchOffSize = 0;

      // Check if destination directory already exists
      if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(DestPath))
      {
        int rResult = CreateDirectoryHierarchy(DestPath, DestPathLen);
        if (ERROR_SUCCESS == rResult)
        {
          pStats->m_DirectoryCreated++;
          Log(L"+d %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
        }
        else
        {
          pStats->m_DirectoryCreateFailed++;
          PathNameStatus pns(PlusD, &DestPath[PathParseSwitchOffSize], rResult);
        }
      }
/*
      else
      {
        pStats->m_DirectoryCreateSkipped++;

        Log(L"=d %s\n", &DestPath[PATH_PARSE_SWITCHOFF_SIZE], m_Flags, eLogVerbose);
      }
*/
    }

    int SourcePathLen = pF->m_SourcePathLen;
    wcscat_s(DestPath, HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE, &pF->m_FileName[SourcePathLen]);

    // Check whether we have to go async
    if (apContext)
    {
      // Increment the progress
      apContext->AddProgress(eHardlinkWeight, 1, 1);

#if defined _DEBUG
      count++;
#endif
      // Set the status, and check if CloneFile has been cancelled from outside
      int r = apContext->PutStatus(&pF->m_FileName[PathParseSwitchOffSize], &DestPath[PathParseSwitchOffSize]);
      if (ERROR_REQUEST_ABORTED == r)
      {
        RetVal = ERROR_REQUEST_ABORTED;
        break;
      }
    }

    // Decide whether to make hard or symbolic links
    int result;
    if (m_Flags & eSymboliclinkClone)
    {
      //
      // Symbolic Links
      // 
      DWORD SymLinkFlags = m_Flags & eBackupMode ? SYMLINK_FLAG_FILE | FILE_FLAG_BACKUP_SEMANTICS : SYMLINK_FLAG_FILE;
      SymLinkFlags |= m_Flags & eRelativeSymboliclinks;

      if (m_Flags2 & eUnprivSymlinkCreate)
        SymLinkFlags |= SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

      result = CreateSymboliclink(
        DestPath, 
        pF->m_FileName,
        SymLinkFlags
      );
      if (ERROR_SUCCESS != result)
      {
        if (ERROR_ALREADY_EXISTS == result)
        {
          pStats->m_SymlinksRestoreSkipped++;
          Log(L"=s %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
        }
        else
        {
          pStats->m_SymlinksRestoreFailed++;
          PathNameStatus pns(PlusS, &DestPath[PathParseSwitchOffSize], GetLastError());
          aPathNameStatusList->push_back(pns);
        }
      }
      else
      {
        // Everything fine, go on
        pStats->m_FilesLinked++;
        pStats->m_BytesLinked += pF->m_FileSize.ul64;
        Log(L"+s %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
      }
    }
    else // if (m_Flags & eSymboliclinkClone)
    {
      
      // With hardlink the 1023 limit can be exceeded, which is a pitty. 
      // To overcome this apply the following algorithm, but only if the container is in --traditional,
      // because only then we have the refcount. The fast enumerator does not deliver the refcount
      //
      // * Check if we are close to the limit 
      // * Assume the siblings are adjacent to each other due to SantityCheck
      // * SanityCheck has also put the number of siblings of this FileInfoContainer into 
      //   the LeaderSibling
      // * Copy the 'LeaderSibling'
      // * Hardlink the 'CrowdSibling' to the LeaderSibling
      //
      if ( (m_Flags & eTraditional) && ((pF->m_RefCount + pF->m_HardlinkInSet) > m_HardlinkLimit) )
      {
        // We have reached the HardlinkLimit and have to copy over
        DWORD                 LastError;
        BOOL                  bCancel = FALSE;
        int                   CopyOkInt = ERROR_SUCCESS;

        // 
        // Copy new LeaderSibling
        // 
        CopyOkInt = CopyFileEx3(
          pF->m_FileName, 
          DestPath, 
          NULL, 
          NULL,
          &bCancel, 
          CopyFlags, 
          &m_pSecDesc, 
          &m_SecDescSize,
          m_DateTimeTolerance,
          aPathNameStatusList
        );
        LastError = GetLastError();

        // <Print statistics of CopyFile>
        //
        switch (CopyOkInt)
        {
          case ERROR_ALREADY_EXISTS:
          {
            pStats->m_FilesCopySkipped++;
            pStats->m_BytesCopySkippped += pF->m_FileSize.ul64;

            Log(L"=f %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
          }
          break;

          case ERROR_SUCCESS:
          {
            pStats->m_FilesCopied++;
            pStats->m_BytesCopied += pF->m_FileSize.ul64;

            Log(L"+f %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
          }
          break;

          case ERROR_FILE_EXISTS:
          {
            // To be more specific about which file could not be opened, we abuse the ERROR_FILE_EXISTS
            // for indication of a problem in the *target file*, and ERROR_ACCESS_DENIED as usual as
            // indication for problems on the *source file*
            pStats->m_FilesCopyFailed++;
            pStats->m_BytesCopyFailed += pF->m_FileSize.ul64;
          }
          break;

          case ERROR_ACCESS_DENIED:
          default: 
          {
            // Copy Operation failed
            pStats->m_FilesCopyFailed++;
            pStats->m_BytesCopyFailed += pF->m_FileSize.ul64;
          }
          break;
        }
        // </Print statistics of CopyFile>

        wchar_t LeaderSibling[HUGE_PATH];
        wcscpy_s(LeaderSibling, HUGE_PATH, DestPath);

        DestPath[DestPathLen] = 0x00;

        // Link to LeaderSibling
        //
        int LinkCount = 1;
        while (LinkCount < pF->m_HardlinkInSet)
        {
          FileInfo*	pSibling = *iter;
          ++iter;

          int SourcePathLen = pSibling->m_SourcePathLen;
          wcscat_s(DestPath, HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE, &pSibling->m_FileName[SourcePathLen]);
          result = CreateHardlink(LeaderSibling, DestPath, pSibling);
          if (ERROR_SUCCESS != result)
          {
            // Hardlink Operation failed
            if (ERROR_ALREADY_EXISTS == result)
            {
              pStats->m_FilesLinkSkipped++;
              pStats->m_BytesLinkSkipped += pSibling->m_FileSize.ul64;

              Log(L"=h %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
            }
            else // if (ERROR_ALREADY_EXISTS == result)
            {
              pStats->m_FilesLinkFailed++;
              pStats->m_BytesLinkFailed += pSibling->m_FileSize.ul64;

              PathNameStatus pns(PlusH, &DestPath[PathParseSwitchOffSize], GetLastError());
              aPathNameStatusList->push_back(pns);
            }
          }
          else
          {
            // Everything fine, go on
            pStats->m_FilesLinked++;
            pStats->m_BytesLinked += pSibling->m_FileSize.ul64;
            Log(L"+h %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
          }

          DestPath[DestPathLen] = 0x00;
          LinkCount++;

        } // while (LinkCount < pF->m_HardlinkInSet)

      } // if ( (m_Flags & eTraditional) && ((pF->m_RefCount + pF->m_HardlinkInSet) > m_HardlinkLimit) )
      else
      {
        // The normal way
        result = CreateHardlink(pF->m_FileName, DestPath, pF);
        if (ERROR_SUCCESS != result)
        {
          // Hardlink Operation failed
          if (ERROR_ALREADY_EXISTS == result)
          {
            pStats->m_FilesLinkSkipped++;
            pStats->m_BytesLinkSkipped += pF->m_FileSize.ul64;

            Log(L"=h %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogVerbose);
          }
          else // if (ERROR_ALREADY_EXISTS == result)
          {
            pStats->m_FilesLinkFailed++;
            pStats->m_BytesLinkFailed += pF->m_FileSize.ul64;

            PathNameStatus pns(PlusH, &DestPath[PathParseSwitchOffSize], GetLastError());
            aPathNameStatusList->push_back(pns);
          }
        }
        else
        {
          // Everything fine, go on
          pStats->m_FilesLinked++;
          pStats->m_BytesLinked += pF->m_FileSize.ul64;
          Log(L"+h %s\n", &DestPath[PathParseSwitchOffSize], m_Flags, eLogChanged);
        }
      }
      DestPath[DestPathLen] = 0x00;
    } // if (m_Flags & eSymboliclinkClone)

    DestPath[DestPathLen] = 0x00;
  } while (iter != aEnd);

  return RetVal;
}

int
FileInfoContainer::
CleanItems(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  CopyStatistics*		        pStats,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext,
  DWORD                     aItemType
)
{
  int RetVal = ERROR_SUCCESS;

  // check for empty set
  if (aBegin == aEnd)
    return ERROR_SUCCESS;

  wchar_t DestPath[HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE] = { 0 };
  size_t  DestPathLen{ 0 };

  // start with an invalid value
  int DestPathIdx = -1;

  enum {
    FileCompare = 1,
    HardlinkCompare,
    SymbolicLinkCompare,
    NoCompare,
  };

  // There are different Compare Modes
  int CompOper = FileCompare;
  if ((eCloneMirror & m_Flags2) && (eSymboliclinkClone & m_Flags))
    CompOper = SymbolicLinkCompare;
  else 
    if (eCloneMirror & m_Flags2)
      CompOper = HardlinkCompare;

  // If we use SmartClean to delete complete trees, we don't have a lookasideContainer
  if (!m_pLookAsideFileInfoContainer)
    CompOper = NoCompare;


  // Go through all items
  for (_Pathes::iterator iter = aBegin; iter != aEnd; ++iter)
  {
    FileInfo*	pF = *iter;

    // When we are async we shall provide a proper progress increment
    if (apContext)
    {
      int progressIncrement = 0;
      switch (aItemType)
      {
        case FILE_ATTRIBUTE_NORMAL:
        {
          progressIncrement = eFileWeight;
          break;
        }

        case FILE_ATTRIBUTE_REPARSE_POINT:
        {
          progressIncrement = eReparseWeight;
          break;
        }
        case FILE_ATTRIBUTE_DIRECTORY:
        {
          progressIncrement = eDirectoryWeight;
          break;
        }
      }

      // We found it on the LookasideContainer, so don't do anything except counting the progress properly
      apContext->AddProgress(progressIncrement, 1, 1);
    }



    // With CleanItems we assume that the Destpath has a different meaning
    // than with the other SmartXXX function: If only one source is given during 
    // --delorean, then we are sure we only have one Destpath and all items have 
    // just the one and only DestPathIdx in pF->m_DespathIdx, because they were all
    // found under the same root of backup. ( Well this would cause trouble if the
    // backup was spread over different locations, but this is by definition not )
    //
    // If more than one --source locations are specified, than we need to check
    // if a path found under the backup is found in any given source location.
    // So we have to iterate through all possible source locations to make sure we
    // find the combination
    //
    FileInfo* pLookAside{ nullptr };
    int PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
    int CurrentDestPathIdx = 0;
    while ( CurrentDestPathIdx < m_AnchorPathCache.size() )
    {
      // See if the DestPath has changed since the last time
      if (CurrentDestPathIdx != DestPathIdx)
      {
        DestPathLen = m_AnchorPathCache.GetDestPath(DestPath, CurrentDestPathIdx);
        DestPathIdx = CurrentDestPathIdx;

        // Check if the destination is a very long path. It could also be a UNC Path
        if (IsVeryLongPath(DestPath))
          PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
        else
          // UNC Path
          PathParseSwitchOffSize = 0;
      }

      int SourcePathLen = pF->m_SourcePathLen;
      wcscat_s(DestPath, HUGE_PATH + PATH_PARSE_SWITCHOFF_SIZE, &pF->m_FileName[SourcePathLen]);

      if (apContext)
      {
        // Set the status, and check if CleanItems has been cancelled from outside
        int r = apContext->PutStatus(&pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], &DestPath[PathParseSwitchOffSize]);
        if (ERROR_REQUEST_ABORTED == r)
        {
          RetVal = ERROR_REQUEST_ABORTED;
          break;
        }
      }

      switch (CompOper)
      {
        case SymbolicLinkCompare:
          pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
          if (pLookAside)
          {
            // It was found on the other side. Determine if it is a symlink or file
            if (pF->m_Type & ( FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY) ) 
              ;
              // Assume it is fine, and the reparse point points back
              // TBD Even if this might not be true because it could point anywhere, but as a first guess assume it is ok
            else
              pLookAside = NULL;

          }
        break;

        case FileCompare:
          pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
        break;

        case HardlinkCompare:
        {
          // Compare the fileindex of source and destination file
          pLookAside = m_pLookAsideFileInfoContainer->Find(DestPath);
          if (pLookAside)
            if (pF->m_FileIndex.ul64 != pLookAside->m_FileIndex.ul64)
              // If the file is there on the other end, but has a different FileId, 
              // then it is not a hardlink. So indicate it was not found
              pLookAside = NULL;
        }
        break;

        case NoCompare:
          pLookAside = NULL;
        break;
      }
      
      if (pLookAside)
        // We found it on the LookasideContainer, so don't do anything to it
        break;
    
      CurrentDestPathIdx++;
    } 

    // If someone pressed cancel during the operation leave the loop
    if (ERROR_REQUEST_ABORTED == RetVal)
      break;

    if (!pLookAside)
    {
      // Check if the source is a very long path. It could also be a UNC Path
      if (IsVeryLongPath(pF->m_FileName))
        PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;
      else
        // UNC Path
        PathParseSwitchOffSize = 0;

      switch (aItemType)
      {
        case FILE_ATTRIBUTE_NORMAL:
        {
          BOOL DeletedFile;
          DeletedFile = DeleteSibling(pF->m_FileName, pF->m_Type);
          if (TRUE == DeletedFile)
          {
            pStats->m_FilesCleaned++;
            pStats->m_BytesCleaned += pF->m_FileSize.ul64;

            Log(L"-f %s\n", &(pF->m_FileName[PathParseSwitchOffSize]), m_Flags, eLogChanged);
          }
          else
          {
            pStats->m_FilesCleanedFailed++;

            PathNameStatus pns(MinusF, &(pF->m_FileName[PathParseSwitchOffSize]), GetLastError());
            aPathNameStatusList->push_back(pns);
          }
        }
        break;

        case FILE_ATTRIBUTE_REPARSE_POINT:
        {
          DWORD attr = GetFileAttributes(pF->m_FileName);
          if (attr & FILE_ATTRIBUTE_DIRECTORY)
          {
            int ReparsePointType = ProbeReparsePoint(pF->m_FileName, NULL);
            BOOL RemovedDirectory = RemoveDirectory(pF->m_FileName);
            if (!RemovedDirectory)
            {
              // There might be files which have flags set so that they can't be deleted
              SetFileAttributes(pF->m_FileName, FILE_ATTRIBUTE_NORMAL);

              // Lets give it one more try with fixed attributes
              RemovedDirectory = RemoveDirectory(pF->m_FileName);
            }

            if (RemovedDirectory)
            {
              // Statistics
              switch (ReparsePointType)
              {
                case REPARSE_POINT_JUNCTION:
                  pStats->m_JunctionsCleaned++;
                  Log(L"-j %s\n", &(pF->m_FileName[PathParseSwitchOffSize]), m_Flags, eLogChanged);
                break;

                case REPARSE_POINT_MOUNTPOINT:
                  pStats->m_MountpointsCleaned++;
                  Log(L"-m %s\n", &(pF->m_FileName[PathParseSwitchOffSize]), m_Flags, eLogChanged);
                break;

                case REPARSE_POINT_SYMBOLICLINK:
                  pStats->m_SymlinksCleaned++; 
                  Log(L"-s %s\n", &(pF->m_FileName[PathParseSwitchOffSize]), m_Flags, eLogChanged);
                break;
              }
            }
            else // if (RemoveDirectory(pF->m_FileName))
            {
              // Statistics
              switch (ReparsePointType)
              {
                case REPARSE_POINT_JUNCTION:
                {
                  pStats->m_JunctionsCleanedFailed++;

                  PathNameStatus pns(MinusJ, &(pF->m_FileName[PathParseSwitchOffSize]), GetLastError());
                  aPathNameStatusList->push_back(pns);
                }
                break;

                case REPARSE_POINT_MOUNTPOINT:
                {
                  pStats->m_MountpointsCleanedFailed++;

                  PathNameStatus pns(MinusM, &(pF->m_FileName[PathParseSwitchOffSize]), GetLastError());
                  aPathNameStatusList->push_back(pns);
                }
                break;

                case REPARSE_POINT_SYMBOLICLINK:
                {
                  pStats->m_SymlinksCleanedFailed++; 

                  PathNameStatus pns(MinusS, &(pF->m_FileName[PathParseSwitchOffSize]), GetLastError());
                  aPathNameStatusList->push_back(pns);
                }
                break;
              }
            } // if (RemoveDirectory(pF->m_FileName))
          }
          else // if (attr & FILE_ATTRIBUTE_DIRECTORY)
          {
            // Here we  end up if this is a symlink file
            BOOL DeletedFile = DeleteFile(pF->m_FileName);
            if (!DeletedFile)
            {
              // There might be files which have flags set so that they
              // can't be deleted
              SetFileAttributes(pF->m_FileName, FILE_ATTRIBUTE_NORMAL);

              // Lets give it one more try with fixed attributes
              DeletedFile = DeleteFile(pF->m_FileName);
            }

            if (DeletedFile)
            {
              pStats->m_SymlinksCleaned++;
              Log(L"-s %s\n", &(pF->m_FileName[PathParseSwitchOffSize]), m_Flags, eLogChanged);
            }
            else
            {
              pStats->m_SymlinksCleanedFailed++;

              PathNameStatus pns(MinusS, &(pF->m_FileName[PathParseSwitchOffSize]), GetLastError());
              aPathNameStatusList->push_back(pns);
            }
          }
        }
        break;

        case FILE_ATTRIBUTE_DIRECTORY:
        {
          // Plain directories
          BOOL RemovedDir = RemoveDirectory(pF->m_FileName);
          if (!RemovedDir)
          {
            // There might be directories which have flags set so that they
            // can't be deleted
            SetFileAttributes(pF->m_FileName, FILE_ATTRIBUTE_NORMAL);

            // Lets give it one more try with fixed attributes
            RemovedDir = RemoveDirectory(pF->m_FileName);
          }

          if (RemovedDir)
          {
            pStats->m_DirectoriesCleaned++;
            Log(L"-d %s\n", &(pF->m_FileName[PathParseSwitchOffSize]), m_Flags, eLogChanged);
          }
          else
          {
            pStats->m_DirectoriesCleanedFailed++;

            PathNameStatus pns(MinusD, &(pF->m_FileName[PathParseSwitchOffSize]), GetLastError());
            aPathNameStatusList->push_back(pns);
          }
        }
        break;
      }
    }

    DestPath[DestPathLen] = 0x00;

  }

  return RetVal;
}

int
FileInfoContainer::
CalcSize(
  _Pathes::iterator&	      aBegin,
  _Pathes::iterator&	      aEnd,
  CopyStatistics*		        pStats,
  _PathNameStatusList*      aPathNameStatusList,
  AsyncContext*             apContext
)
{
  int RetVal = ERROR_SUCCESS;
  int r = 0;

  // check for empty set
  if (aBegin == aEnd)
    return ERROR_SUCCESS;

  // Go through all files
  _Pathes::iterator	iter = aBegin;
  _Pathes::iterator	last = aBegin;
  while (++iter != aEnd)
  {
    if ((*iter)->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64 || (*iter)->m_DiskIndex != (*last)->m_DiskIndex )
    {
      // border of set
      pStats->m_HardlinksTotal++;
      pStats->m_HardlinksTotalBytes += (*last)->m_FileSize.ul64;

      last = iter;
      if (ERROR_REQUEST_ABORTED == r)
      {
        RetVal = ERROR_REQUEST_ABORTED;
        break;
      }
    }
  }

  // Do the last file too
  if (ERROR_REQUEST_ABORTED != RetVal)
  {
      pStats->m_HardlinksTotal++;
      pStats->m_HardlinksTotalBytes += (*last)->m_FileSize.ul64;

      if (ERROR_REQUEST_ABORTED == r)
      RetVal = ERROR_REQUEST_ABORTED;
  }

  return RetVal;
}



void
FileInfoContainer::
ChangePath(
  const wchar_t*  aNewSourcePath,
  _ArgvList&      aNewDestPath 
)
{
  // When the PathMap changes it has to be deleted ...
#if defined USE_VECTOR
  m_PathVector.clear();
#else    
  m_PathMap.clear();
#endif

  size_t  Length = wcslen(aNewSourcePath);
  _Pathes::iterator	iter;
  for (iter = m_Filenames.begin(); iter != m_Filenames.end(); ++iter)
  {
    FileInfo*	pF = *iter;
    pF->ReplaceSourcePath(aNewSourcePath, Length);

    // and rebuild otherwise the search will not work
#if defined USE_VECTOR
    m_PathVector.push_back(_PathMap_Pair(pF->m_FileName, pF));
#else    
    m_PathMap.insert(_PathMap_Pair(pF->m_FileName, pF));
#endif
  }

#if defined USE_VECTOR
  sort( m_PathVector.begin(), m_PathVector.end(), vector_comp());
#endif

  m_AnchorPathCache.m_AnchorPaths.clear();
  m_AnchorPathCache.Add(aNewDestPath);
}

//--------------------------------------------------------------------
// 
// Sorts the list of files for Smartcopy properly
// 
// In general the order in the list after this method is
//   ReparsePoint :  m_Filenames.begin()
//   Files        :  m_HardlinkBegin
//   Directories  :  m_DirectoryBegin
// 
//--------------------------------------------------------------------
int
FileInfoContainer::
Prepare(
  FileInfoContainer::CopyReparsePointFlags  aMode,
  CopyStatistics*	                          pStats,
  Effort *                                  aEffort
)
{
  if (m_Prepared)
    return -1;

  m_Prepared = true;

  // Group all files into several partitions
  // |--Reparsepoints--|--Delayed Reparsepoints--|--Hardlinks--|--Files--|--Directories--|
  // |                                           |             |         |
  // m_Filenames.begin()                         |             |         |
  //                                             m_HardlinkBegin         |
  //                                             |             FATFilenameBegin
  //                                                                     |
  //                                                                     m_DirectoryBegin
  // At least initialize members, which are used later on
  m_DirectoryBegin = m_Filenames.begin();
  m_HardlinkBegin = m_Filenames.begin();
  m_FATFilenameBegin = m_Filenames.begin();

  // return on an empty set
  if (m_Filenames.begin() != m_Filenames.end())
  {
  m_FilenamesSave = m_Filenames;

  // #define SMARTCOPY_DEBUG // DEBUG_DEFINES
#if defined SMARTCOPY_DEBUG
  // Dump all entries
  HTRACE(L"PrepareSmartCopy All Entries %d\n", distance(m_Filenames.begin(), m_Filenames.end()));
  Dump(m_Filenames.begin(), m_Filenames.end());
#endif

  // divide into Reparse Point and the rest
  // m_HardlinkBegin
  _Pathes::iterator DelayedReparsePointBegin = partition(m_Filenames.begin(), m_Filenames.end(), IsReparsePoint());

  // divide the rest(==Directories & Files) into filenames and directories
  m_DirectoryBegin = partition(DelayedReparsePointBegin, m_Filenames.end(), IsDirectory());

  // The files might contain delayed Reparse Points, which will either 
  // be ReparsePoints or files, once it has been decided
  m_HardlinkBegin = partition(DelayedReparsePointBegin, m_DirectoryBegin, IsDelayedReparsePoint());

#if defined SMARTCOPY_DEBUG
  // Dump the reparse points
  HTRACE(L"PrepareSmartCopy reparse points: %d\n", distance(m_Filenames.begin(), DelayedReparsePointBegin));
  Dump(m_Filenames.begin(), DelayedReparsePointBegin);

  // Dump the Delayed ReparsePoints
  HTRACE(L"PrepareSmartCopy Delayed ReparsePoints %d\n", distance(DelayedReparsePointBegin, m_HardlinkBegin));
  Dump(DelayedReparsePointBegin, m_HardlinkBegin);
#endif

  // Sort the files so that hardlink sibblings are adjacent to each other
  sort(m_HardlinkBegin, m_DirectoryBegin, PathIdxFileIndexSorter());

  // Decide which Symbolic link ReparsePoints are inner or outer
  _Pathes::iterator NewFilenameBegin = ResolveDelayedReparsePoints(DelayedReparsePointBegin, m_HardlinkBegin, m_DirectoryBegin, pStats);
  if (NewFilenameBegin != m_HardlinkBegin)
  {
    m_HardlinkBegin = NewFilenameBegin;

    // And sort again, because we have new items among the FILE_ATTRIBUTE_NORMAL set
    sort(m_HardlinkBegin, m_DirectoryBegin, PathIdxFileIndexSorter());
  }

  // Hardlinks contain possible hardlinks and plain files, if this container 
  // traversed a FAT partition. Plain files have a disk ID of 0xffffffff, and
  // a random file ID 
  m_FATFilenameBegin = partition(m_HardlinkBegin, m_DirectoryBegin, IsPlainFile());

  //  Dump(m_HardlinkBegin, m_DirectoryBegin);

    // To avoid hardlink clustering of plain files, each files get a uniq file id,
    // so that the normal mechanisms of Smartcopy can be used
  __int64 Idx = 0;
  for (_Pathes::iterator iter = m_FATFilenameBegin; iter != m_DirectoryBegin; ++iter)
    (*iter)->m_FileIndex.ul64 = Idx++;

#if defined SMARTCOPY_DEBUG
  // Dump the reparse points
  HTRACE(L"PrepareSmartCopy reparse points %d\n", distance(m_Filenames.begin(), m_HardlinkBegin));
  Dump(m_Filenames.begin(), m_HardlinkBegin);

  // Dump the Filenames
  HTRACE(L"PrepareSmartCopy Filenames %d\n", distance(m_HardlinkBegin, m_DirectoryBegin));
  Dump(m_HardlinkBegin, m_DirectoryBegin);

  // Dump the directories
  HTRACE(L"PrepareSmartCopy Directories %d\n", distance(m_DirectoryBegin, m_Filenames.end()));
  Dump(m_DirectoryBegin, m_Filenames.end());

#endif
  }
  // Only calc effort with a valid mode. There are situations, where PrepareSmartCopy is only called to
  // prepare the container
  return aEffort ? EstimateEffort(aMode, aEffort) : 0;
}

// Estimate effort for hardlink groups
void
FileInfoContainer::
EstimateHardlinkGroupEffort(
  _Pathes::iterator&	a_Begin,
  _Pathes::iterator&	a_End,
  Effort* a_Effort
)
{
      // Check if there are hardlinks at all
  if (a_Begin != a_End)
      {
        // Smart Copy overall amount calculation
    _Pathes::iterator	iter = a_Begin;
    _Pathes::iterator	last = a_Begin;

        // Go through all files and only the size of the first sibbling
        // and for all sibblings just add eHardlinkWeight
    while (++iter != a_End)
        {
          if ((*iter)->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64 || (*iter)->m_DiskIndex != (*last)->m_DiskIndex)
          {
            // border of set
            auto nSiblings = distance(last, iter);
        a_Effort->m_Items += nSiblings;

            // With size we calculate hardlinks as 1 and count one sibling with full size
        a_Effort->m_Size += nSiblings - 1;
        a_Effort->m_Size += (*last)->m_FileSize.ul64;

            // During calculation hardlinks are a bit heavier than just the size
        a_Effort->m_Points += (nSiblings - 1) * eHardlinkWeight;
        a_Effort->m_Points += (*last)->m_FileSize.ul64;

            last = iter;
          }
        }

        // Last set
        auto nSiblings = distance(last, iter);
    a_Effort->m_Items += nSiblings;

        // With size we calculate hardlinks as 1 and count one sibling with full size
    a_Effort->m_Size += nSiblings - 1;
    a_Effort->m_Size += (*last)->m_FileSize.ul64;

        // During calculation hardlinks are a bit heavier than just the size
    a_Effort->m_Points += (nSiblings - 1) * eHardlinkWeight;
    a_Effort->m_Points += (*last)->m_FileSize.ul64;
  }
      }

// Estimate effort for the operations on containers
//
int
FileInfoContainer::
EstimateEffort(
  FileInfoContainer::CopyReparsePointFlags  aMode,
  Effort *                                  aEffort
)
{
  // TODO: Wenn viele Daten in Alternative datastreans kopiert werden stimmt die Vorhersage nicht, und der Progressbar läuft
  // drüber. Aber wie kommt man vorab auf die Anzahl der Bytes in den e.g. Alternative DataStreams? Schwierig
  switch (aMode)
  {
    case FileInfoContainer::eSmartMirror:
    case FileInfoContainer::eSmartCopy:
    {
      // Weight for Copy Directories
      auto nDirectories = distance(m_DirectoryBegin, m_Filenames.end());
      aEffort->m_Items = nDirectories;
      aEffort->m_Size = nDirectories;
      aEffort->m_Points = nDirectories * eDirectoryWeight;
      auto points = aEffort->m_Points.load();
      HTRACE(L"Effort CM nDirectories: %I64d, %I64d\n", nDirectories, points);

      if (m_Flags2 & eCloneMirror)
      {
        // During Clonemirror and DeloreanMerge the effort calculation is is different than during copy.
        //  Since there is no copy the effort for linking is always the single weight
        auto nFiles = distance(m_HardlinkBegin, m_DirectoryBegin);
        aEffort->m_Points += nFiles * eHardlinkWeight;
        aEffort->m_Size += nFiles;
        aEffort->m_Items += nFiles;
      }
      else
      {
        EstimateHardlinkGroupEffort(m_HardlinkBegin, m_DirectoryBegin, aEffort);
      }

      HTRACE(L"Effort CM nFiles: %I64d, %I64d\n", aEffort->m_Items - nDirectories, aEffort->m_Points - points);

      // Add effort for Reparse Points
      auto nReparsePoints = distance(m_Filenames.begin(), m_HardlinkBegin);
      aEffort->m_Items += nReparsePoints;
      aEffort->m_Size += nReparsePoints;
      aEffort->m_Points += nReparsePoints * eReparseWeight;
      HTRACE(L"Effort CM nReparsepoints: %I64d\n", nReparsePoints);

      // Add effort for CopyDirectoriesSilent
      auto nDateTime = distance(m_DateTimeRestore.begin(), m_DateTimeRestore.end());
      aEffort->m_Items += nDateTime;
      aEffort->m_Size += nDateTime;
      aEffort->m_Points += nDateTime * eDirectoryWeight;
      HTRACE(L"Effort CM CopyDirectoriesSilent: %I64d\n", nDateTime);

      // Add effort for restoring the timestamps on reparsepoints
//      aEffort->m_Items += nReparsePoints;
      aEffort->m_Size += nReparsePoints;
      aEffort->m_Points += nReparsePoints * eTimeStampWeight;
      HTRACE(L"Effort CM timestamps nReparsepoints: %I64d\n", nReparsePoints);

      // Add effort for restoring the timestamps on directories
//      aEffort->m_Items += nDirectories;
      aEffort->m_Size += nDirectories;
      aEffort->m_Points += nDirectories * eTimeStampWeight;
      HTRACE(L"Effort CM timestamps nDirectories: %I64d\n", nDirectories);

      // Add effort for restoring the Attributes on directories
//      aEffort->m_Items += nDateTime;
      aEffort->m_Size += nDateTime;
      aEffort->m_Points += nDateTime * eTimeStampWeight;
      HTRACE(L"Effort CM nAttributes: %I64d\n", nDateTime);
    }
    break;

    case FileInfoContainer::eSmartClean:
    {
      // Smart Clean overall amount calculation.
      // Files & Hardlinks
      auto nFiles = distance(m_HardlinkBegin, m_DirectoryBegin);
      aEffort->m_Points = nFiles * eFileWeight;
      for (auto fileEntry = m_HardlinkBegin; fileEntry != m_DirectoryBegin; ++fileEntry)
        aEffort->m_Size += (*fileEntry)->m_FileSize.ul64;
      aEffort->m_Items = nFiles;
      HTRACE(L"Effort CL timestamps nFiles: %I64d\n", nFiles);

      // Reparsepoints
      auto nReparsePoints = distance(m_Filenames.begin(), m_HardlinkBegin);
      aEffort->m_Points += nReparsePoints * eReparseWeight;
      aEffort->m_Size++;
      aEffort->m_Items += nReparsePoints;
      HTRACE(L"Effort CL timestamps nReparsePoints: %I64d\n", nReparsePoints);

      // Directories
      auto nDirectories = distance(m_DirectoryBegin, m_Filenames.end());
      aEffort->m_Points += nDirectories * eDirectoryWeight;
      aEffort->m_Size++;
      aEffort->m_Items += nDirectories;
      HTRACE(L"Effort CL timestamps nDirectories: %I64d\n", nDirectories);
    }
    break;

    // case FileInfoContainer::eDupemerge:
    case FileInfoContainer::eSmartClone:
    {
      // Smart Clone overall amount calculation

      // Add effort for Directories
      auto nDirectories = distance(m_DirectoryBegin, m_Filenames.end());
      aEffort->m_Points = nDirectories * eDirectoryWeight;
      aEffort->m_Size = nDirectories;
      aEffort->m_Items = nDirectories;
      HTRACE(L"Effort CE nDirectories: %I64d\n", nDirectories);

      // Add effort for Files
      auto nFiles = distance(m_HardlinkBegin, m_DirectoryBegin);
      aEffort->m_Points += nFiles * eHardlinkWeight;
      aEffort->m_Size += nFiles;
      aEffort->m_Items += nFiles;
      HTRACE(L"Effort CE nFiles: %I64d\n", nFiles);

      // Add effort for Reparsepoints
      auto nReparsePoints = distance(m_Filenames.begin(), m_HardlinkBegin);
      aEffort->m_Points += nReparsePoints * eReparseWeight;
      aEffort->m_Size += nReparsePoints;
      aEffort->m_Items += nReparsePoints;
      HTRACE(L"Effort CE nReparsepoints: %I64d\n", nReparsePoints);

      // Add effort for restoring the timestamps on reparsepoints
      aEffort->m_Points += nReparsePoints * eTimeStampWeight;
      aEffort->m_Size += nReparsePoints;
//      aEffort->m_Items += nReparsePoints;
      HTRACE(L"Effort CE timestamps nReparsepoints: %I64d\n", nReparsePoints);

      // Add effort for restoring the timestamps on directories
      aEffort->m_Points += nDirectories * eTimeStampWeight;
      aEffort->m_Size += nDirectories;
//      aEffort->m_Items += nDirectories;
      HTRACE(L"Effort CE timestamps nDirectories: %I64d\n", nDirectories);

      // Add effort for restoring the Attributes on directories
      auto nAttributes = distance(m_DateTimeRestore.begin(), m_DateTimeRestore.end());
      aEffort->m_Points += nAttributes * eTimeStampWeight;
      aEffort->m_Size += nAttributes;
//      aEffort->m_Items += nAttributes;
      HTRACE(L"Effort CE nAttributes: %I64d\n", nAttributes);
    }
    break;

    case FileInfoContainer::eSmartMove:
    {
      // Add effort for Reparsepoints
      auto nReparsePoints = distance(m_Filenames.begin(), m_HardlinkBegin);
      aEffort->m_Items = nReparsePoints;
      aEffort->m_Size = nReparsePoints;
      aEffort->m_Points = nReparsePoints * eReparseWeight;

      // Add effort for restoring the timestamps on reparsepoints
//      aEffort->m_Items += nReparsePoints;
      aEffort->m_Size += nReparsePoints;
      aEffort->m_Points += nReparsePoints * eTimeStampWeight;

      // Add effort for restoring the timestamps on directories
      auto nDirectories = distance(m_DirectoryBegin, m_Filenames.end());
//      aEffort->m_Items += nDirectories;
      aEffort->m_Size += nDirectories;
      aEffort->m_Points += nDirectories * eTimeStampWeight;
    }
    break;
  }
  return 1;
}



//--------------------------------------------------------------------
// 
// Walks through all delayed ReparsePoints and searched the destination
// among files. There are two results
// o) If it finds them, then the symbolic link is a new inner symbolic
//    link. The ReparseSrcTargetHint is created, and the type is changed
//    to FILE_ATTRIBUTE_REPARSE_POINT
// o) It does not find it, then a copy will be created.
//    The file attribute is changed to FILE_ATTRIBUTE_NORMAL
// 
//--------------------------------------------------------------------
_Pathes::iterator
FileInfoContainer::
ResolveDelayedReparsePoints(
   _Pathes::iterator      aDelayedReparsePointBegin, 
   _Pathes::iterator      aFilenameBegin, 
   _Pathes::iterator      aDirectoryBegin,
   CopyStatistics*        pStats
)
{
  _Pathes::iterator found;
  for (_Pathes::iterator iter = aDelayedReparsePointBegin; iter !=  aFilenameBegin; ++iter)
  {
    FileInfo* pI = *iter;
    found = lower_bound(aFilenameBegin, aDirectoryBegin, *iter, FileIndexCompare());
    if (found != aDirectoryBegin && (*found)->m_FileIndex.ul64 == (*iter)->m_FileIndex.ul64 && (*found)->m_DiskIndex == (*iter)->m_DiskIndex)
    {
      // It is a ReparsePoint

      FileInfo* pF = *found;
      // Assemble the ReparseSrcTargetHint
      const wchar_t* pReparseSrcTargetHint = &pF->m_FileName[pF->m_SourcePathLen];
      size_t ReparseSrcTargetHintLength = wcslen(pReparseSrcTargetHint) + 1;
      pStats->m_HeapAllocTime.Start();
      (*iter)->m_ReparseSrcTargetHint = new wchar_t[wcslen(pF->m_FileName) + 1];
      pStats->m_HeapAllocTime.Stop();

      wcscpy_s ((*iter)->m_ReparseSrcTargetHint, ReparseSrcTargetHintLength, pReparseSrcTargetHint);
    }
    else
    {
      // Couldn't find it, so change file type to FILE_ATTRIBUTE_NORMAL, because
      // it points to somewhere
      (*iter)->m_Type = FILE_ATTRIBUTE_NORMAL;
      // files or hardlinks
      pStats->m_FilesTotal++;
      pStats->m_BytesTotal += (*iter)->m_FileSize.ul64;
    }
  }

  // Newly partition the processed items
  _Pathes::iterator NewFilenameBegin = partition (aDelayedReparsePointBegin, aFilenameBegin, IsDelayedReparsePoint());
  for (_Pathes::iterator iter = aDelayedReparsePointBegin; iter !=  NewFilenameBegin; ++iter)
  {
    (*iter)->m_Type = FILE_ATTRIBUTE_REPARSE_POINT;
    pStats->m_SymlinksTotal++;
  }

  return NewFilenameBegin;
}


//--------------------------------------------------------------------
// 
// Iterates through the collected reparse points and sees if one
// is a Symbolic Link
// 
//--------------------------------------------------------------------
BOOL
FileInfoContainer::
CheckSymbolicLinks()
{
  CopyStatistics	      aStats;
  Prepare(eSmartClone, &aStats);

  WCHAR	DummyReparsePointTarget[HUGE_PATH] = { 0 };
  for (_Pathes::iterator iter = m_Filenames.begin(); iter !=  m_HardlinkBegin; ++iter)
  {
    FileInfo*	pF = *iter;
    if ( REPARSE_POINT_SYMBOLICLINK == ProbeReparsePoint(pF->m_FileName, DummyReparsePointTarget) )
      return TRUE;
  }
  return FALSE;
}

int
FileInfoContainer::
SmartCopy(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
  if (apContext)
  {
    SmartCopyParams* pSmartCopyParams = new SmartCopyParams;
    pSmartCopyParams->m_Stats = pStats;
    pSmartCopyParams->m_PathNameStatusList = aPathNameStatusList;
    pSmartCopyParams->m_pContext = apContext;

    pSmartCopyParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunSmartCopy), 
      pSmartCopyParams,
      0, 
      &ThreadId);

    return 0;
  }
  else
    return _SmartCopy(pStats, aPathNameStatusList, apContext);
}

DWORD
__stdcall
FileInfoContainer::
RunSmartCopy(SmartCopyParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_SmartCopy(p->m_Stats, p->m_PathNameStatusList, pCtx);
  delete p;

  return 1;
}

int
FileInfoContainer::
_SmartCopy(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
#if 0 && defined _DEBUG // DEBUG_DEFINES
  // This is the unit test for the PathIdxFileIndexSorter()
  AddFile (L"\\\\?\\e:\\data\\test\\000.txt", FILE_ATTRIBUTE_NORMAL, 0x00390000, 0x0000d2b1, 0x04, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\001.txt", FILE_ATTRIBUTE_NORMAL, 0x00490000, 0x0000d2b1, 0x04, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\002.txt", FILE_ATTRIBUTE_NORMAL, 0x00190000, 0x0000d2b1, 0x04, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\003.txt", FILE_ATTRIBUTE_NORMAL, 0x00390000, 0x0000d2b1, 0x02, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\004.txt", FILE_ATTRIBUTE_NORMAL, 0x00490000, 0x0000d2b1, 0x02, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\005.txt", FILE_ATTRIBUTE_NORMAL, 0x00190000, 0x0000d2b1, 0x02, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\006.txt", FILE_ATTRIBUTE_NORMAL, 0x00090000, 0x0000d2b1, 0x04, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\007.txt", FILE_ATTRIBUTE_NORMAL, 0x00890000, 0x0000d2b1, 0x06, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\010.txt", FILE_ATTRIBUTE_NORMAL, 0x00390000, 0x0000d2b1, 0x04, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\011.txt", FILE_ATTRIBUTE_NORMAL, 0x00490000, 0x0000d2b1, 0x04, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\012.txt", FILE_ATTRIBUTE_NORMAL, 0x00190000, 0x0000d2b1, 0x04, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\013.txt", FILE_ATTRIBUTE_NORMAL, 0x00390000, 0x0000d2b1, 0x02, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\014.txt", FILE_ATTRIBUTE_NORMAL, 0x00490000, 0x0000d2b1, 0x02, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\015.txt", FILE_ATTRIBUTE_NORMAL, 0x00190000, 0x0000d2b1, 0x02, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\016.txt", FILE_ATTRIBUTE_NORMAL, 0x00090000, 0x0000d2b1, 0x04, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\017.txt", FILE_ATTRIBUTE_NORMAL, 0x00890000, 0x0000d2b1, 0x06, 0x01);

  AddFile (L"\\\\?\\e:\\data\\test\\000.txt", FILE_ATTRIBUTE_NORMAL, 0x00390000, 0x0000d2b1, 0x14, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\001.txt", FILE_ATTRIBUTE_NORMAL, 0x00490000, 0x0000d2b1, 0x14, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\002.txt", FILE_ATTRIBUTE_NORMAL, 0x00190000, 0x0000d2b1, 0x14, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\003.txt", FILE_ATTRIBUTE_NORMAL, 0x00390000, 0x0000d2b1, 0x12, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\004.txt", FILE_ATTRIBUTE_NORMAL, 0x00490000, 0x0000d2b1, 0x12, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\005.txt", FILE_ATTRIBUTE_NORMAL, 0x00190000, 0x0000d2b1, 0x12, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\006.txt", FILE_ATTRIBUTE_NORMAL, 0x00090000, 0x0000d2b1, 0x14, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\007.txt", FILE_ATTRIBUTE_NORMAL, 0x00890000, 0x0000d2b1, 0x16, 0x00);
  AddFile (L"\\\\?\\e:\\data\\test\\010.txt", FILE_ATTRIBUTE_NORMAL, 0x00390000, 0x0000d2b1, 0x14, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\011.txt", FILE_ATTRIBUTE_NORMAL, 0x00490000, 0x0000d2b1, 0x14, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\012.txt", FILE_ATTRIBUTE_NORMAL, 0x00190000, 0x0000d2b1, 0x14, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\013.txt", FILE_ATTRIBUTE_NORMAL, 0x00390000, 0x0000d2b1, 0x12, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\014.txt", FILE_ATTRIBUTE_NORMAL, 0x00490000, 0x0000d2b1, 0x12, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\015.txt", FILE_ATTRIBUTE_NORMAL, 0x00190000, 0x0000d2b1, 0x12, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\016.txt", FILE_ATTRIBUTE_NORMAL, 0x00090000, 0x0000d2b1, 0x14, 0x01);
  AddFile (L"\\\\?\\e:\\data\\test\\017.txt", FILE_ATTRIBUTE_NORMAL, 0x00890000, 0x0000d2b1, 0x16, 0x01);

  sort (m_Filenames.begin(), m_Filenames.end(), PathIdxFileIndexSorter());

  Dump (m_Filenames.begin(), m_Filenames.end());
#endif

#if defined _M_IX86
  PVOID OldValue = NULL;
  Wow64DisableWow64FsRedirection(&OldValue);
#endif

  Prepare(eSmartCopy, pStats);

  // If NTFS is broken invalidate the hardlink indices
  if (errOk != SanityCheck(m_HardlinkBegin, m_DirectoryBegin) )
  {
    InvalidateFileIndex(m_HardlinkBegin, m_DirectoryBegin);

    // Print an error the message if we dupemerge was not forced
    if (!(m_Flags2 & eDupemerge))
    {
      // NTFS is broken
      wchar_t BrokenDrive[HUGE_PATH];
      wcscpy_s(BrokenDrive, HUGE_PATH, &(*m_HardlinkBegin)->m_FileName[PATH_PARSE_SWITCHOFF_SIZE]);
      PathStripToRoot(BrokenDrive);
      PathNameStatus pns(StarH, BrokenDrive, ERROR_INVALID_INDEX);
      aPathNameStatusList->push_back(pns);
    }
  }

  // See if we should go for on the fly dupemerge
  if (m_Flags2 & eDupemerge)
  {
    Log(L"'Entering on the fly dupemerge. This may take a while...\n", L" ", m_Flags, eLogVerbose);

    DmFindDupes(m_HardlinkBegin, m_DirectoryBegin, aPathNameStatusList, pStats);
    CalcFileIndex(m_HardlinkBegin, m_DirectoryBegin);
    m_Prepared = false;
    Prepare(eSmartCopy, pStats);
  }

  // Copy Directories 
  int r = CopyDirectories(
    m_DirectoryBegin, 
    m_Filenames.end(), 
    pStats, 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  if (!(m_Flags & eSkipFiles))
  {
    // Copy Files and Hardlinks before reparsepoints, because we also 
    // have symbolic links, which are reparse points, and they might 
    // refer to files, so we need them when we later on copy symbolic links
    r = CopyHardlinks(
      m_HardlinkBegin,
      m_DirectoryBegin,
      pStats, 
      aPathNameStatusList,
      apContext,
      eSmartCopy
	  );

    if (ERROR_REQUEST_ABORTED == r)
    {
#if defined _M_IX86
      Wow64RevertWow64FsRedirection(OldValue);
#endif
      return ERROR_REQUEST_ABORTED;
    }

  }
  else
  {
  // TODO: Fake-Advance the progress bei SkipFiles. Da aber SkipFiles nur von der Kommandozeile aus aufgerufen 
  // werden kann ist das nicht wichtig
  }

  // Copy Reparse Points
  r = CopyReparsePoints(
    m_Filenames.begin(), 
    m_HardlinkBegin, 
    pStats, 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }


  // Create Directories, which have not been created up till now.
  // Most propably they were empty Root Dir
  r = CopyDirectoriesSilent(
    m_DateTimeRestore.begin(), 
    m_DateTimeRestore.end(), 
    apContext,
    eSmartCopy
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore timestamps on ReparsePoints
  r = RestoreTimeStamps(
	  m_Filenames.begin(),
    m_HardlinkBegin, 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore timestamps on Directories when everything is done
  r = RestoreTimeStamps(
	  m_DirectoryBegin,
    m_Filenames.end(), 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore the date time of the very first directory
  CopyDirectoryAttributes(
    m_DateTimeRestore.begin(), 
    m_DateTimeRestore.end(), 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

#if defined _M_IX86
  Wow64RevertWow64FsRedirection(OldValue);
#endif
  return ERROR_SUCCESS;
}

int
FileInfoContainer::
TrueSize(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
  if (apContext)
  {
    return _TrueSize(pStats, aPathNameStatusList, NULL);
  }
  else
    return _TrueSize(pStats, aPathNameStatusList, apContext);
}

int
FileInfoContainer::
_TrueSize(
   CopyStatistics*	      pStats,
   _PathNameStatusList*  aPathNameStatusList,
   AsyncContext*         apContext
)
{
#if defined _M_IX86
  PVOID OldValue = NULL;
  Wow64DisableWow64FsRedirection(&OldValue);
#endif

  Prepare(eSmartCopy, pStats);

  if (errOk != SanityCheck(m_HardlinkBegin, m_DirectoryBegin))
  {
    // NTFS is broken
    wchar_t BrokenDrive[HUGE_PATH];
    wcscpy_s(BrokenDrive, HUGE_PATH, &(*m_HardlinkBegin)->m_FileName[PATH_PARSE_SWITCHOFF_SIZE]);
    PathStripToRoot(BrokenDrive);
    PathNameStatus pns(StarH, BrokenDrive, ERROR_INVALID_INDEX);
    aPathNameStatusList->push_back(pns);

    return ERROR_INVALID_INDEX;
  }

  int r = CalcSize(
    m_HardlinkBegin,
    m_DirectoryBegin,
    pStats, 
    aPathNameStatusList,
    apContext
  );

#if defined _M_IX86
  Wow64RevertWow64FsRedirection(OldValue);
#endif
  return ERROR_SUCCESS;
}


int
FileInfoContainer::
SmartClone(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
  if (apContext)
  {
    SmartCopyParams* pSmartCopyParams = new SmartCopyParams;
    pSmartCopyParams->m_Stats = pStats;
    pSmartCopyParams->m_PathNameStatusList = aPathNameStatusList;
    pSmartCopyParams->m_pContext = apContext;

    pSmartCopyParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunSmartClone), 
      pSmartCopyParams,
      0, 
      &ThreadId);

    return 0;
  }
  else
    return _SmartClone(pStats, aPathNameStatusList, apContext);
}

DWORD
__stdcall
FileInfoContainer::
RunSmartClone(SmartCopyParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_SmartClone(p->m_Stats, p->m_PathNameStatusList, pCtx);
  delete p;

  return 1;
}

int
FileInfoContainer::
_SmartClone(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
#if defined _M_IX86
  PVOID OldValue = NULL;
  Wow64DisableWow64FsRedirection(&OldValue);
#endif
  Prepare(eSmartClone, pStats);

  if (errOk != SanityCheck(m_HardlinkBegin, m_DirectoryBegin))
  {
    // NTFS is broken
    InvalidateFileIndex(m_HardlinkBegin, m_DirectoryBegin);

    // Print an error  message if dupemerge was not forced
    if (!(m_Flags2 & eDupemerge))
    {
      wchar_t BrokenDrive[HUGE_PATH];
      wcscpy_s(BrokenDrive, HUGE_PATH, &(*m_HardlinkBegin)->m_FileName[PATH_PARSE_SWITCHOFF_SIZE]);
      PathStripToRoot(BrokenDrive);
      PathNameStatus pns(StarH, BrokenDrive, ERROR_INVALID_INDEX);
      aPathNameStatusList->push_back(pns);
    }
  }

  // See if we should go for on the fly dupemerge
  if (m_Flags2 & eDupemerge )
  {
    Log(L"'Entering on the fly dupemerge. This may take a while...\n", L" ", m_Flags, eLogVerbose);

    DmFindDupes(m_HardlinkBegin, m_DirectoryBegin, aPathNameStatusList, pStats);
    CalcFileIndex(m_HardlinkBegin, m_DirectoryBegin);
    Prepare(eSmartCopy, pStats);
  }

  // Copy Directories 
  int r = CopyDirectories(
    m_DirectoryBegin, 
    m_Filenames.end(), 
    pStats, 
    aPathNameStatusList,
    apContext,
    eSmartCopy
    );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Clone Files and Hardlinks
  if (!(m_Flags & eSkipFiles))
  {
    r = CloneFiles(
      m_HardlinkBegin,
      m_DirectoryBegin,
      pStats, 
      aPathNameStatusList,
      apContext
	  );

    if (ERROR_REQUEST_ABORTED == r)
    {
#if defined _M_IX86
      Wow64RevertWow64FsRedirection(OldValue);
#endif
      return ERROR_REQUEST_ABORTED;
    }
  }

  // Copy Reparse Points
  r = CopyReparsePoints(
    m_Filenames.begin(), 
    m_HardlinkBegin, 
    pStats, 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore timestamps on ReparsePoints
  r = RestoreTimeStamps(
	  m_Filenames.begin(), 
    m_HardlinkBegin, 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore timestamps on directories
  r = RestoreTimeStamps(
	  m_DirectoryBegin, 
    m_Filenames.end(), 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore the date time of the very first directory
  CopyDirectoryAttributes(
    m_DateTimeRestore.begin(), 
    m_DateTimeRestore.end(), 
    aPathNameStatusList,
    apContext,
    eSmartCopy
  );

#if defined _M_IX86
  Wow64RevertWow64FsRedirection(OldValue);
#endif
  return ERROR_SUCCESS;
}

int
FileInfoContainer::
SmartClean(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
  if (apContext)
  {
    SmartCopyParams* pSmartCopyParams = new SmartCopyParams;
    pSmartCopyParams->m_Stats = pStats;
    pSmartCopyParams->m_PathNameStatusList = aPathNameStatusList;
    pSmartCopyParams->m_pContext = apContext;

    pSmartCopyParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunSmartClean), 
      pSmartCopyParams,
      0, 
      &ThreadId);

    return 0;
  }
  else
    return _SmartClean(pStats, aPathNameStatusList, apContext);
}

DWORD
__stdcall
FileInfoContainer::
RunSmartClean(SmartCopyParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_SmartClean(p->m_Stats, p->m_PathNameStatusList, pCtx);
  delete p;

  return 1;
}

int
FileInfoContainer::
_SmartClean(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
#if defined _M_IX86
  PVOID OldValue = NULL;
  Wow64DisableWow64FsRedirection(&OldValue);
#endif

  Prepare(eSmartClean, pStats);

  if (errOk != SanityCheck(m_HardlinkBegin, m_DirectoryBegin))
  {
    // NTFS is broken
    InvalidateFileIndex(m_HardlinkBegin, m_DirectoryBegin);

    wchar_t BrokenDrive[HUGE_PATH];
    wcscpy_s(BrokenDrive, HUGE_PATH, &(*m_HardlinkBegin)->m_FileName[PATH_PARSE_SWITCHOFF_SIZE]);
    PathStripToRoot(BrokenDrive);
    PathNameStatus pns(StarH, BrokenDrive, ERROR_INVALID_INDEX);
    aPathNameStatusList->push_back(pns);
  }

  // No need to find the dupes on the fly here

  int r = ERROR_SUCCESS;
  if (!(m_Flags & eSkipFiles))
  {
    // |--Reparsepoints--|--Files--|--Directories--| 
    // Clean files first
    r = CleanItems(
      m_HardlinkBegin,
      m_DirectoryBegin,
      pStats, 
      aPathNameStatusList,
      apContext,
      FILE_ATTRIBUTE_NORMAL
	  );

    if (ERROR_REQUEST_ABORTED == r)
    {
#if defined _M_IX86
      Wow64RevertWow64FsRedirection(OldValue);
#endif
      return ERROR_REQUEST_ABORTED;
    }
  }

  // Clean Reparse Points
  r = CleanItems(
    m_Filenames.begin(), 
    m_HardlinkBegin, 
    pStats, 
    aPathNameStatusList,
    apContext,
    FILE_ATTRIBUTE_REPARSE_POINT
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Create all Directories. Sorting brings directories in such an order
  // so that we can easily delete them
  sort (m_DirectoryBegin, m_Filenames.end(), PathIdxFilenameSorterDescending());

  // Clean Directories 
  r = CleanItems(
    m_DirectoryBegin, 
    m_Filenames.end(), 
    pStats, 
    aPathNameStatusList,
    apContext,
    FILE_ATTRIBUTE_DIRECTORY
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

#if defined _M_IX86
  Wow64RevertWow64FsRedirection(OldValue);
#endif
  return ERROR_SUCCESS;
}

int
FileInfoContainer::
SmartMirror(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
  if (apContext)
  {
    SmartCopyParams* pSmartCopyParams = new SmartCopyParams;
    pSmartCopyParams->m_Stats = pStats;
    pSmartCopyParams->m_PathNameStatusList = aPathNameStatusList;
    pSmartCopyParams->m_pContext = apContext;

    pSmartCopyParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunSmartMirror), 
      pSmartCopyParams,
      0, 
      &ThreadId);

    return 0;
  }
  else
    return _SmartMirror(pStats, aPathNameStatusList, apContext);
}

DWORD
__stdcall
FileInfoContainer::
RunSmartMirror(SmartCopyParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_SmartMirror(p->m_Stats, p->m_PathNameStatusList, pCtx);
  delete p;

  return 1;
}

int
FileInfoContainer::
_SmartMirror(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
#if defined _M_IX86
  PVOID OldValue = NULL;
  Wow64DisableWow64FsRedirection(&OldValue);
#endif

  Prepare(eSmartMirror, pStats);

  if (errOk != SanityCheck(m_HardlinkBegin, m_DirectoryBegin))
  {
    // NTFS is broken
    InvalidateFileIndex(m_HardlinkBegin, m_DirectoryBegin);

    // Print an error message for broken NTFS implementation
    if (!(m_Flags2 & eDupemerge))
    {
      wchar_t BrokenDrive[HUGE_PATH];
      wcscpy_s(BrokenDrive, HUGE_PATH, &(*m_HardlinkBegin)->m_FileName[PATH_PARSE_SWITCHOFF_SIZE]);
      PathStripToRoot(BrokenDrive);
      PathNameStatus pns(StarH, BrokenDrive, ERROR_INVALID_INDEX);
      aPathNameStatusList->push_back(pns);
    }
  }

  // See if we should go for on the fly dupemerge
  if (m_Flags2 & eDupemerge )
  {
    Log(L"'Entering on the fly dupemerge. This may take a while...\n", L" ", m_Flags, eLogVerbose);

    DmFindDupes(m_HardlinkBegin, m_DirectoryBegin, aPathNameStatusList, pStats);
    CalcFileIndex(m_HardlinkBegin, m_DirectoryBegin);
    Prepare(eSmartCopy, pStats);
  }


  // Copy Directories 
  int r = CopyDirectories(
    m_DateTimeRestore.begin(), 
    m_DateTimeRestore.end(), 
    pStats, 
    aPathNameStatusList,
    apContext,
    eSmartMirror
  );

  // Copy Directories 
  r = CopyDirectories(
    m_DirectoryBegin, 
    m_Filenames.end(), 
    pStats, 
    aPathNameStatusList,
    apContext,
    eSmartMirror
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Copy Files and Hardlinks
  if (!(m_Flags & eSkipFiles))
  {
    // Copy Files and Hardlinks
    if (m_Flags2 & eCloneMirror)
    {
      r = CloneFiles(
        m_HardlinkBegin,
        m_DirectoryBegin,
        pStats, 
        aPathNameStatusList,
        apContext
	    );
    }
    else
    {
      r = CopyHardlinks(
        m_HardlinkBegin,
        m_DirectoryBegin,
        pStats, 
        aPathNameStatusList,
        apContext,
        eSmartMirror
	    );

      if (ERROR_REQUEST_ABORTED == r)
      {
#if defined _M_IX86
        Wow64RevertWow64FsRedirection(OldValue);
#endif
        return ERROR_REQUEST_ABORTED;
      }
    }
    
    if (ERROR_REQUEST_ABORTED == r)
    {
#if defined _M_IX86
      Wow64RevertWow64FsRedirection(OldValue);
#endif
      return ERROR_REQUEST_ABORTED;
    }
  }

  // Copy Reparse Points
  r = CopyReparsePoints(
    m_Filenames.begin(), 
    m_HardlinkBegin, 
    pStats, 
    aPathNameStatusList,
    apContext,
    eSmartMirror
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore timestamps on ReparsePoints
  r = RestoreTimeStamps(
    m_Filenames.begin(),
    m_HardlinkBegin,
    aPathNameStatusList,
    apContext,
    eSmartMirror
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore timestamps on directories
  //
  r = RestoreTimeStamps(
    m_DirectoryBegin,
    m_Filenames.end(),
    aPathNameStatusList,
    apContext,
    eSmartMirror
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore the date time of the very first directory
  r = CopyDirectoryAttributes(
    m_DateTimeRestore.begin(),
    m_DateTimeRestore.end(),
    aPathNameStatusList,
    apContext,
    eSmartMirror
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

#if defined _M_IX86
  Wow64RevertWow64FsRedirection(OldValue);
#endif
  return ERROR_SUCCESS;
}



//
// Async Dispose 
//
int
FileInfoContainer::
Dispose(
  AsyncContext*         apContext,
  CopyStatistics*	      apStats
)
{
  if (apContext)
  {
    DisposeParams* pDisposeParams = new DisposeParams;
    pDisposeParams->m_pContext = apContext;
    pDisposeParams->m_pStats = apStats;

    pDisposeParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunDispose), 
      pDisposeParams,
      0, 
      &ThreadId);

    return 0;
  }
  else
    return _Dispose(apContext, apStats);
}

DWORD
__stdcall
FileInfoContainer::
RunDispose(DisposeParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  CopyStatistics*	pStats = p->m_pStats;
  pCtx->m_RetVal = p->m_This->_Dispose(pCtx, pStats);
  delete p;

  return 1;
}

void
FileInfoContainer::
DisposeRegExpList(
  _RegExpList&      a_RegExpList
)
{
  // Cleanup regular expression
  for (auto iter : a_RegExpList)
  {
#if defined REGEXP_STL
    delete iter;
#else
    regex_t*  pRegEx = iter;
    regfree (pRegEx);
    delete pRegEx;
#endif
  }
  a_RegExpList.clear();
}


int
FileInfoContainer::
DisposeMmf(
  _Pathes::iterator	aBegin,
	_Pathes::iterator	aEnd
)
{
  // Close all open file mappings
	for (_Pathes::iterator iter = aBegin; iter != aEnd; ++iter)
  {
    FileInfo* pF = *iter;
    if (pF->m_pMmfObject)
		{
			delete pF->m_pMmfObject;
			pF->m_pMmfObject = NULL;
		}
  }
  return errOk;
}

int
FileInfoContainer::
Dispose(
  _Pathes::iterator	    aBegin,
	_Pathes::iterator	    aEnd,
  CopyStatistics*	      apStats
)
{
  DisposeMmf(aBegin, aEnd);

  for (_Pathes::iterator iter = aBegin; iter != aEnd; ++iter)
  {
    FileInfo*	pFileInfo = *iter;
    if (pFileInfo->m_FileName)
    {
      apStats->m_HeapDeletionTime.Start();
      delete[] pFileInfo->m_FileName;
      apStats->m_HeapDeletionTime.Stop();
    }
      
    apStats->m_HeapDeletionTime.Start();
    delete pFileInfo;
    apStats->m_HeapDeletionTime.Stop();
  }

  if (m_pSecDesc)
  {
    apStats->m_HeapDeletionTime.Start();
    free(m_pSecDesc);
    apStats->m_HeapDeletionTime.Stop();
  }

  m_pSecDesc = NULL;

  return errOk;
}


int
FileInfoContainer::
_Dispose(
  AsyncContext*         apContext,
  CopyStatistics*	      apStats
)
{
#if defined USE_VECTOR
  m_PathVector.clear();
#else    
  m_PathMap.clear();
#endif

	m_DupeGroups.clear ();

  // Cleanup all FileInfo members. Long story but worth to mention:
  // 
  // Problem: With more than 3gig priv bytes deallocation of memory has an awful performance.
  // 
  // Solution: Even 64bit(!!!) processes start paging out the heap if more than 
  // 3gig are allocated, even if there are 7gig of mem are available
  // 
  // If memory gets freed in the same order as it was allocated, or at least 
  // the location of memory access during free on the heap is strictly 
  // increasing, everything is fine
  // 
  // If for some reason the location of memory access on the heap during free 
  // is completely chaotic, then the paging mechanism comes in and the heap is 
  // moved back and forth between phys mem and page file.
  // 
  // This costs some so much time, that freeing of 7mio items the chaotic takes 
  // 34 minutes, and freeing in clean increasing way, takes 3 seconds. Freeing 
  // 13mio items the chaotic way took 5 hours.
  // 
  // Now for the things I have learned
  // o) I didn't expect that this so *much* influences performance
  // o) I thought 64bit processes due to its 4tb adress space are *unaffected* by 
  //    this behaviour, especially if there is enough free memory
  // o) Memory has to be deleted in the way it was allocated

  // So try to use m_FilenamesSave, which contains the original order, and eases
  // deletion. If m_FilenamesSave is not there, because one cancelled during
  // enumeration, we delete via m_Filenames, and then it contains the original
  // order of items and is fast again.
  if (m_FilenamesSave.begin() != m_FilenamesSave.end())
  {
    Dispose(m_FilenamesSave.begin(), m_FilenamesSave.end(), apStats);
	  m_FilenamesSave.clear();
  }
  else
  {
    Dispose(m_Filenames.begin(), m_Filenames.end(), apStats);
  }
  m_Filenames.clear();
  m_DateTimeRestore.clear();

  // Cleanup regular expression
  DisposeRegExpList(m_RegExpInclFileList);
  DisposeRegExpList(m_RegExpInclDirList);
  DisposeRegExpList(m_RegExpExclFileList);
  DisposeRegExpList(m_RegExpExclDirList);
  DisposeRegExpList(m_RegExpUnrollDirList);
  DisposeRegExpList(m_RegExpSpliceDirList);

  m_SpliceDirList.clear();

  return ERROR_SUCCESS;
}

int
FileInfoContainer::
SmartMove(
  CopyStatistics*	      pStats,
  _PathNameStatusList*  aPathNameStatusList,
  AsyncContext*         apContext
)
{
  if (apContext)
  {
    SmartCopyParams* pSmartCopyParams = new SmartCopyParams;
    pSmartCopyParams->m_Stats = pStats;
    pSmartCopyParams->m_PathNameStatusList = aPathNameStatusList;
    pSmartCopyParams->m_pContext = apContext;

    pSmartCopyParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunSmartMove), 
      pSmartCopyParams,
      0, 
      &ThreadId);

    return 0;
  }
  else
    return _SmartMove(pStats, aPathNameStatusList, apContext);
}

DWORD
__stdcall
FileInfoContainer::
RunSmartMove(SmartCopyParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_SmartMove(p->m_Stats, p->m_PathNameStatusList, pCtx);
  delete p;

  return 1;
}

int
FileInfoContainer::
_SmartMove(
          CopyStatistics*	      pStats,
          _PathNameStatusList*  aPathNameStatusList,
          AsyncContext*         apContext
          )
{
#if defined _M_IX86
  PVOID OldValue = NULL;
  Wow64DisableWow64FsRedirection(&OldValue);
#endif
  Prepare(eSmartMove, pStats);

  // Is this NTFS ok?
  if (errOk != SanityCheck(m_HardlinkBegin, m_DirectoryBegin))
  {
    InvalidateFileIndex(m_HardlinkBegin, m_DirectoryBegin);

    wchar_t BrokenDrive[HUGE_PATH];
    wcscpy_s(BrokenDrive, HUGE_PATH, &(*m_HardlinkBegin)->m_FileName[PATH_PARSE_SWITCHOFF_SIZE]);
    PathStripToRoot(BrokenDrive);
    PathNameStatus pns(StarH, BrokenDrive, ERROR_INVALID_INDEX);
    aPathNameStatusList->push_back(pns);
  }

  // No need to find the dupes on the fly here

  // Rename the junctions/symlinks using CopyReparsePoints in eSmartMove mode
  int r = CopyReparsePoints(
    m_Filenames.begin(), 
    m_HardlinkBegin,
    pStats, 
    aPathNameStatusList,
    apContext,
    eSmartMove
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore timestamps on ReparsePoints
  r = RestoreTimeStamps(
	  m_Filenames.begin(),
    m_HardlinkBegin, 
    aPathNameStatusList,
    apContext,
    eSmartMove
  );

  if (ERROR_REQUEST_ABORTED == r)
  {
#if defined _M_IX86
    Wow64RevertWow64FsRedirection(OldValue);
#endif
    return ERROR_REQUEST_ABORTED;
  }

  // Restore timestamps on directories
  // Sorting brings directories in such an order so that we can easily set the timestamps.
  //
  RestoreTimeStamps(
    m_DirectoryBegin,
    m_Filenames.end(), 
    aPathNameStatusList,
    apContext,
    eSmartMove
  );

  wchar_t DestPath[HUGE_PATH];
  m_AnchorPathCache.GetDestPath(DestPath, 0);
  
  if (gVersionInfo.dwMajorVersion < 6)
  {
    // TODO Why can't we delete this with Windows7. IF we do regression fails in t_SmartMove02.bat horribly

    // With WindowsXP we had to temporarily create target directories for junctions 
    // so that we were able to create the junctions. Now we have to delete them.
    XDelStatistics  rXDelStatistics;
    XDel(DestPath, HUGE_PATH, rXDelStatistics, TRUE);
  }
  
  RemoveDir(DestPath, TRUE);

#if defined _M_IX86
  Wow64RevertWow64FsRedirection(OldValue);
#endif
  return r;
}

// Regular Expression Handling
//
void
FileInfoContainer::
CopyRegExpList(
  _StringList* a_pStringList, 
  _StringList& a_StringList
)
{
  for (_StringListIterator iter = a_pStringList->begin(); iter != a_pStringList->end(); ++iter)
    a_StringList.push_back (iter->c_str());
}

void
FileInfoContainer::
CompileRegExpList(_StringList* a_pStringList, _RegExpList& a_RegExpList)
{
  for (_StringListIterator iter = a_pStringList->begin(); iter != a_pStringList->end(); ++iter)
  {
#if defined REGEXP_STL
    wregex* pRegEx = new wregex(*iter);
    a_RegExpList.push_back(pRegEx);
#else
    regex_t*  compiled = new regex_t;
    int			retval = regwcomp (compiled, (*iter).c_str(), REG_ICASE | REG_EXTENDED);
    if (REG_OK == retval)
      a_RegExpList.push_back (compiled);
#endif
  }
}

void
FileInfoContainer::
SetIncludeList(_StringList* a_pStringList)
{
  CompileRegExpList(a_pStringList, m_RegExpInclFileList);
}

void
FileInfoContainer::
SetIncludeDirList(_StringList* a_pStringList)
{
  CompileRegExpList(a_pStringList, m_RegExpInclDirList);
}

void
FileInfoContainer::
SetExcludeList(_StringList* a_pStringList)
{
  CompileRegExpList(a_pStringList, m_RegExpExclFileList);
}

void
FileInfoContainer::
SetExcludeDirList(_StringList* a_pStringList)
{
  CompileRegExpList(a_pStringList, m_RegExpExclDirList);
}

// Set a list of regular expressions, which should be unrolled
// during FindHardlink. If the list is NULL unroll any directory 
void
FileInfoContainer::
SetUnrollDirList(_StringList* a_pStringList)
{
  if (a_pStringList)
    CompileRegExpList(a_pStringList, m_RegExpUnrollDirList);
  else
    SetFlags(eAlwaysUnrollDir);
}

void
FileInfoContainer::
SetSpliceDirList(_StringList* a_pStringList)
{
  if (a_pStringList)
  {
    CompileRegExpList(a_pStringList, m_RegExpSpliceDirList);

    // Save the regexps in text, because if we need to persist
    // the container, we need it
    CopyRegExpList(a_pStringList, m_SpliceDirList);
  }
  else
    SetFlags(eAlwaysSpliceDir);
}


bool
FileInfoContainer::
MatchRegExpList(
  wchar_t*      a_String,
  _RegExpList&  a_RegExpList
)
{
  bool RegExpMatch = false;
#if defined REGEXP_STL
  wcmatch match;
#else
  regmatch_t pmatch[1];
#endif
  for (auto iter : a_RegExpList)
  {
#if defined REGEXP_STL
    if (regex_search(a_String, match, *iter))
    {
      RegExpMatch = true;
      break;
    }
#else
    int retval = regwexec (iter, a_String, 1, pmatch, 0);
	  if (REG_OK == retval)
    {
		  RegExpMatch = true;
      break;
    }
#endif
  }

  return RegExpMatch;
}


FileInfo* 
FileInfoContainer::
Find(const wchar_t* a_pFilename)
{
#if defined USE_VECTOR
  _PathVector::iterator found = lower_bound(m_PathVector.begin(), m_PathVector.end(), _PathMap_Pair(a_pFilename, NULL), vector_comp());
  if ( found !=  m_PathVector.end() && !wcscmp(found->first, a_pFilename))
  {
    FileInfo* pF = (*found).second;
    return pF;
  }
  else
    return NULL;

#else    
  _PathMap::iterator pF = m_PathMap.find(a_pFilename);
  if ( pF !=  m_PathMap.end() )
    return (*pF).second;
  else
    return NULL;
#endif
}

#if defined _DEBUG
void
FileInfoContainer::
DumpPathMap()
{
#if defined USE_VECTOR
  HTRACE(L"#m_PathVector.size() %d#\n", m_PathVector.size());
  for (auto iter : m_PathVector)
    HTRACE(L"#%s#\n", iter.first);
#else    
  for (_PathMap::iterator iter = m_PathMap.begin(); iter != m_PathMap.end(); ++iter)
    HTRACE(L"#%s#\n", (*iter).first);
#endif
}
#endif

FILE*
FileInfoContainer::
StartLogging(
  LSESettings&   r_LSESettings,
  wchar_t*        a_Tag
)
{
  FILE* LogFile = NULL;
  if (r_LSESettings.GetFlags() & eLogOutput)
  {
    wchar_t LogFileName[MAX_PATH];
    GetTempPath(MAX_PATH, LogFileName);
    wcscat_s(LogFileName, L"LinkShellExtension.log");
    
    _wfopen_s(&LogFile, LogFileName, L"wt,ccs=UNICODE");
    if (LogFile)
    {
      SetFlags(eLogVerbose);
      SetFlags2(eLogFileOutput);
      SetOutputFile(LogFile);
      fwprintf(LogFile, L"%s\n", a_Tag);
      fwprintf(LogFile, L"Flags: %08x\n", r_LSESettings.GetFlags());
    }
    else
      SetFlags(FileInfoContainer::eLogQuiet);
  }
  return LogFile;
}

FILE*
FileInfoContainer::
AppendLogging(
)
{
  FILE* LogFile = NULL;
  if (m_Flags2 & eLogFileOutput)
  {
    wchar_t LogFileName[MAX_PATH];
    GetTempPath(MAX_PATH, LogFileName);
    wcscat_s(LogFileName, L"LinkShellExtension.log");
    
    _wfopen_s(&LogFile, LogFileName, L"at,ccs=UNICODE");
    if (LogFile)
      SetOutputFile(LogFile);
  }
  return LogFile;
}

void
FileInfoContainer::
HeadLogging(
  FILE*                 a_File
)
{
  if(a_File)
    m_AnchorPathCache.Save(a_File);
}

void
FileInfoContainer::
EndLogging(
  FILE*                 a_File,
  _PathNameStatusList&  r_PathNameStatusList,
  CopyStatistics&       r_CopyStatistics,
  FileInfoContainer::CopyReparsePointFlags  aMode
)
{
  if(a_File)
  {
    // LSE does not support Json Logging right now: JsonWriterState == NULL !!!
    AnalysePathNameStatus(a_File, r_PathNameStatusList, true, FileInfoContainer::eLogVerbose, false, NULL);
    PrintDeloreanCopyStats(a_File, r_CopyStatistics, aMode, false, false);

    fclose(a_File);
  }
}

void
FileInfoContainer::
StopLogging(
  FILE*                 a_File
)
{
  if(a_File)
  {
    fclose(a_File);
    SetOutputFile(NULL);
  }
}


void 
FileInfoContainer::
Log(
  const wchar_t*  a_pTag,
  const wchar_t*  a_pPathName,
  int       a_Flags,
  int       a_LogLevel
)
{
  int LogLevel = a_Flags & eLogMask;

  if (LogLevel <= a_LogLevel && m_OutputFile)
  {
    if (a_Flags & eJson)
    {
      wchar_t JsonPath[HUGE_PATH];
      wcsesc_s(JsonPath, a_pPathName, HUGE_PATH, L'\\');

      if (*m_pJsonWriterState)
        fwprintf (m_OutputFile, L",\n");
      else
        *m_pJsonWriterState = true;

      if (a_pTag[0] == '\'')
      {
        wchar_t CommentText[MAX_PATH];
        wcscpy_s(CommentText, MAX_PATH, &a_pTag[1]);
        size_t length = wcslen(CommentText);
        CommentText[length - 1] = 0x0;

        fwprintf (m_OutputFile, L"{\"op\":\"%c\",\"er\":0,\"ty\":\"%s\",\"pa\":\"%s\"}", a_pTag[0], CommentText, JsonPath);
      }
      else
        fwprintf (m_OutputFile, L"{\"op\":\"%c\",\"er\":0,\"ty\":\"%c\",\"pa\":\"%s\"}", a_pTag[0], a_pTag[1], JsonPath);
    }
    else
    {
      fwprintf (m_OutputFile, a_pTag, a_pPathName);
    }

  }
}

FileInfoContainer::
~FileInfoContainer()
{
  CopyStatistics	      stats;
  Dispose (NULL, &stats);
}


//
// AnchorPathCache
//
wchar_t* 
AnchorPathCache::
ContainsSource(
  const wchar_t*  a_Path,
  wchar_t*        a_AlternativeDestPath,
  int*            a_AlternativeDestPathIdx
)
{
  wchar_t* FoundPos;
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
  {
    // Assuming a_Path contains t:\f0\f0_f0 and a path under anchorpath is a root path, e.g. 
    // t:\ we only want to compare with t: to a_Path, so that we receive e.g. \f0\f0_f0.
    // The first \ is important. So make a local copy... not the best performance...
    wchar_t Argv[HUGE_PATH];
    wcscpy_s(Argv, HUGE_PATH, iter->Argv.c_str());
    size_t ArgvLength = iter->Argv.length();

    // PathIsRoot does not work with XP with long path name (\\?\) but works in Windows7
    if (PathIsRoot(&Argv[PATH_PARSE_SWITCHOFF_SIZE]))
      Argv[ArgvLength - 1] = 0x00;

    FoundPos = wcseistr(a_Path, Argv);
    if ( FoundPos )
    {
      if (a_AlternativeDestPath)
      {
        wcscpy_s(a_AlternativeDestPath, HUGE_PATH, iter->ArgvDest.c_str());
        *a_AlternativeDestPathIdx = (int)distance(m_AnchorPaths.begin(), iter);
      }
      return FoundPos;
    }
  }

  return NULL;
}

int
AnchorPathCache::
ContainsExactSource(
  const wchar_t*  a_Path
)
{
  int FoundPos;
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
  {
    FoundPos = !_wcsicmp(a_Path, iter->Argv.c_str());
    if ( FoundPos )
      return FoundPos;
  }

  return 0;
}

#if !defined EXACT_PATH_INDEX
int
AnchorPathCache::
GetDestPathIndex(
  const wchar_t*  a_Path
)
{
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
  {
    if (!_wcsicmp(iter->ArgvDest.c_str(), a_Path))
      return (int)distance (m_AnchorPaths.begin(), iter);
  }

  return -1;
}

#else
int
AnchorPathCache::
GetDestPathIndex(
  _ArgvListIterator& a_AnchorPath
)
{
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
  {
    if ( !_wcsicmp(iter->ArgvDest.c_str(), a_AnchorPath->ArgvDest.c_str())&& 
         !_wcsicmp(iter->Argv.c_str(), a_AnchorPath->Argv.c_str()) 
       )
      return (int)distance (m_AnchorPaths.begin(), iter);
  }

  return -1;
}
#endif

//
// The FileInfoContainer basically contains a list of files, which could have many destinations
// because each entry in FileInfoContainer references to a destination. The destinations are
// stored in m_DestPath
//
// In a 'normal' usage there will be only one destination for all entries in FileInfoContainer
// but LSE in some situations has the need to add files to the container, which have certain
// destinations. This happens if files get selected alongside directories, and these files
// have a different destinations as the directories do. So it is needed.
//
// One more reason to have different DestPath in one FileInfoContainer is that in LSE
// the Progressbar is bound to the FileInfoContainer. Since we only want to have one 
// Progressbar per operation popping up, we need to maintain all in one FileInfoContainer
//
// Currently it is implemented as a list, which is not optimal, but since only little
// entries are in it, it is ok from a performance point of view
//
void 
AnchorPathCache::
Add(
  _ArgvList&  a_AnchorPath
)
{
  for (_ArgvListIterator iter = a_AnchorPath.begin(); iter != a_AnchorPath.end(); ++iter)
    m_AnchorPaths.push_back(*iter);
}

// Extract the Anchorpath Cache and delete its content
void 
AnchorPathCache::
Get(
  _ArgvList&  a_AnchorPath
)
{
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
    a_AnchorPath.push_back(*iter);

  m_AnchorPaths.clear();
}

size_t
AnchorPathCache::
GetDestPath(
  wchar_t*      a_DestPath,
//  const size_t  a_DestSize,
  int           a_DestPathIdx
)
{
  assert(a_DestPathIdx < m_AnchorPaths.size());
  
  size_t DestPathLength = m_AnchorPaths[a_DestPathIdx].ArgvDest.length();
  wcscpy(a_DestPath, m_AnchorPaths[a_DestPathIdx].ArgvDest.c_str());
  if (a_DestPath[DestPathLength - 1] == '\\')
    a_DestPath[--DestPathLength] = 0x00;

  return DestPathLength;
}

#if defined EXACT_PATH_INDEX
void 
AnchorPathCache::
MarkCreated(
  wchar_t*    a_DestPath
)
{
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
    if (!_wcsicmp(iter->ArgvDest.c_str(), a_DestPath))
      iter->Flags |= _ArgvPath::Created;
}
#endif

// [REMOTE_JUNCTION_RESOLVE]
// First read the comment in IsOuterReparsePoint(). Normally the cache hold pairs of 
// source and destinations, but for each entry with a network drive an additional member 
// is created here, which is 'translated' from the network path to the real path.
//
// Pimping the m_AnchorPaths like this avoids wrong unrolling in IsOuterReparsePoint()
// and creates proper ReparsePoints in CreateReparsePoinnt() for junctions on mapped 
// network drives
void
AnchorPathCache::
ResolveRemote(
)
{
  _ArgvList NewArgv;
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
  {
    // Check whether it is a remote drive or not. For remote drives we are interested
    // in the remote local path under which the share is exposed. We need this to resolve
    // junctions & reparse points later on
    //
    if (iter->DriveType == DRIVE_REMOTE)
    {
      // Resolve remote drive mappings into \\remote\share
      //
      DWORD cbBuff = HUGE_PATH;
      WCHAR szBuff[HUGE_PATH];
      REMOTE_NAME_INFO* prni = (REMOTE_NAME_INFO*)&szBuff;

      const wchar_t* Argv_cstr = iter->Argv.c_str();
      DWORD r = WNetGetUniversalName(&Argv_cstr[PATH_PARSE_SWITCHOFF_SIZE],
        REMOTE_NAME_INFO_LEVEL,
        prni,
        &cbBuff);

      if (NERR_Success == r)
      {
        // Split the remote name into 'remote' and 'share'
        PSHARE_INFO_502	BufPtr;

        wchar_t*  pShare = wcsrchr(prni->lpConnectionName, '\\');
        *pShare = 0x00;
        pShare++;
        NET_API_STATUS	nas = NetShareGetInfo (prni->lpConnectionName, pShare, 502, (LPBYTE*) &BufPtr);
        if (NERR_Success == nas)
        {
          // Assemble a new Argv, which contains the remote location
          _ArgvPath RemoteArgv;
          RemoteArgv.Argv = PATH_PARSE_SWITCHOFF;
          RemoteArgv.Argv += BufPtr->shi502_path;
          wchar_t* Path = PathSkipRoot(&Argv_cstr[PATH_PARSE_SWITCHOFF_SIZE]);
          if (*Path)
          {
            RemoteArgv.Argv += L"\\";
            RemoteArgv.Argv += Path;
          }
          RemoteArgv.ArgvDest = iter->ArgvDest;
          RemoteArgv.ArgvOrg = iter->ArgvOrg;
          RemoteArgv.FileAttribute = iter->FileAttribute;
          RemoteArgv.DriveType = DRIVE_REMOTE_SHARED;

          NewArgv.push_back(RemoteArgv);
        }
      }
    }
  }

  for (_ArgvListIterator iter = NewArgv.begin(); iter != NewArgv.end(); ++iter)
    m_AnchorPaths.push_back(*iter);
}


size_t
AnchorPathCache::
Save(
  FILE*         aFile
)
{
  fwprintf(aFile, L"%zx\n", m_AnchorPaths.size()); 
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
    fwprintf (aFile, L"%s\"%s\"%x,%x,%x\n", 
      iter->Argv.c_str(), 
      iter->ArgvDest.c_str(),
      iter->DriveType,
      iter->FileAttribute,
      iter->Flags
    );

  return ERROR_SUCCESS;
}

size_t
AnchorPathCache::
Load(
  FILE*         aFile
)
{
  // Load Destpath
  int Size = 0;
  wchar_t SourcePath[HUGE_PATH];
  wchar_t DestPath[HUGE_PATH];

  fwscanf_s(aFile, L"%x\n", &Size);
  while (!feof(aFile) && Size-- > 0)
  {
    _ArgvPath AnchorPath;
    fwscanf_s (aFile, L"%[^\"]\"%[^\"]\"%x,%x,%x\n", 
      SourcePath, 
      HUGE_PATH, 
      DestPath, 
      HUGE_PATH, 
      &AnchorPath.DriveType, 
      &AnchorPath.FileAttribute,
      &AnchorPath.Flags
    );
    AnchorPath.Argv = SourcePath;
    AnchorPath.ArgvDest = DestPath;
    m_AnchorPaths.push_back(AnchorPath);
  }

  return ERROR_SUCCESS;
}

size_t
AnchorPathCache::
Logon(
  wchar_t*   a_Username,
  wchar_t*   a_Passwort
)
{
  for (_ArgvListIterator iter = m_AnchorPaths.begin(); iter != m_AnchorPaths.end(); ++iter)
  {

    // TBD: Check if this is a UNC path. Could also be a mapped network drive. And only of them do the log on procedure
    wchar_t UNCPath[MAX_PATH];
    GetShareNameFromUNC(iter->Argv.c_str(), UNCPath);
    NETRESOURCE nr;

    // Delete trailing slash
    size_t Len = wcslen(UNCPath);
    if (UNCPath[Len - 1] == '\\')
      UNCPath[Len - 1] = 0x00;

    // Logon to remote drive
    nr.dwType = RESOURCETYPE_ANY;
    nr.lpLocalName = NULL;
    nr.lpRemoteName = UNCPath;
    nr.lpProvider = NULL;

    // TBD: Determine if a logon is neccesary, by either accessing a file or other means

    DWORD dw = WNetAddConnection3(NULL, &nr, a_Username, a_Passwort, CONNECT_INTERACTIVE | CONNECT_PROMPT);  

    // TBD: Also log on for Destination path in Anchor
/*

    fwprintf (aFile, L"%s\"%s\"%x,%x,%x\n", 
      iter->Argv.c_str(), 
      iter->ArgvDest.c_str(),
      iter->DriveType,
      iter->FileAttribute,
      iter->Flags
    );
*/
  }

  return ERROR_SUCCESS;
}



//
// DiskSerialCache
//
int 
DiskSerialCache::
Lookup(
  wchar_t*  a_pRootPathName,
  DWORD&    a_pVolumeSerialNumber
)
{
  int RetVal = ERROR_SUCCESS;

  int PathParseSwitchOffSize = 0;
  if (IsVeryLongPath(a_pRootPathName))
    PathParseSwitchOffSize = PATH_PARSE_SWITCHOFF_SIZE;

  wchar_t LocalRootPath[HUGE_PATH];
  wcscpy_s(LocalRootPath, HUGE_PATH, &a_pRootPathName[PathParseSwitchOffSize]);
  PathStripToRoot(LocalRootPath);
  _wcslwr_s(LocalRootPath, HUGE_PATH);

  _DiskSerialMap::iterator  iterDsm =  m_DiskSerialMap.find(LocalRootPath);
  if (iterDsm == m_DiskSerialMap.end())
  {
    wchar_t FileSystemname[MAX_PATH];
    FileSystemname[0] = 0x00;

    if (PathIsUNC(&a_pRootPathName[PathParseSwitchOffSize]))
    {
			// Calculate a virtual DiskSerialID for UNC pathes
      size_t LocalRootPathLength = wcslen(LocalRootPath);
      MD5	MD5Sum;
			MD5Sum.Init ();

      // The casting looks strange, but we only want an MD5 hash, so we don't care 
      MD5Sum.Update ((unsigned char*)LocalRootPath, LocalRootPathLength * 2);
			MD5Sum.Final();
			MD5Sum.GetLowerDWord (&a_pVolumeSerialNumber);

      // To retrieve the Filesystemname of a UNC Path we have to call with a trailing \ 
      wcscat_s(LocalRootPath, HUGE_PATH, L"\\");
      BOOL b = GetVolumeInformation(
        LocalRootPath, 
        NULL, 
        0,
        NULL,
        NULL,
        NULL,
        FileSystemname,
        MAX_PATH
      );
    }
    else
    {
      // Check for native windows path
      if (!wcsncmp(a_pRootPathName, PATH_LONG_GLOBALROOT, PATH_LONG_GLOBALROOT_SIZE))
      {
        // Restore the path to filesystem for native windows path
        wcscpy_s(LocalRootPath, HUGE_PATH, a_pRootPathName);
        FindGlobalRootFromPath(LocalRootPath);
      }
      else
      {
        // Check for volume guid path
        if (!wcsncmp(a_pRootPathName, PATH_LONG_VOLUME_GUID, PATH_LONG_VOLUME_GUID_SIZE))
          wcsncpy_s(LocalRootPath, HUGE_PATH, a_pRootPathName, VOLUME_GUID_LENGTH);
      }

      // RootPath not found so retrieve the SerialNumber and add it to the map
      BOOL b = GetVolumeInformation(
        LocalRootPath, 
        NULL, 
        0,
        &a_pVolumeSerialNumber,
        NULL,
        NULL,
        FileSystemname,
        MAX_PATH
      );

      // There might be disk serial numbers, which are identical, but the disk drives 
      // are different. This happens if e.g truecrypt containers are copied, and used
      // for different drive letters. Maybe we should have a better combination of 
      // disk serial number and drive letter, and not just xor one byte of them
      a_pVolumeSerialNumber ^= LocalRootPath[0];

    }
    // All disks, which are not NTFS get the serialnumber FAT_SERIAL_NUMBER. Also ReFS 
    // drives, because the currently do not support hardlinks
    if (!_wcsicmp(FileSystemname, L"NTFS"))
    {
      // NTFS Drives only

      // Check if serial number is already there
      // This is important, because we must not continue enumerating files, if there
      // is a system which has two disks with the same disk serial numbers.
      _DiskSerialInverseMap::iterator  iterDsim =  m_DiskSerialInverseMap.find(a_pVolumeSerialNumber);
      if (iterDsim == m_DiskSerialInverseMap.end())
      {
        size_t  Len = wcslen(LocalRootPath) + 1;
        wchar_t* RootPathName = new wchar_t[Len];
        wcscpy_s(RootPathName, Len, LocalRootPath);

        m_DiskSerialMap.insert(_DiskSerialMap_Pair(RootPathName, a_pVolumeSerialNumber));
        m_DiskSerialInverseMap.insert(_DiskSerialInverseMap_Pair(a_pVolumeSerialNumber, RootPathName));
      }
      else
      {
        // We have two different disks with the same disk serial number, which is 
        // a showstopper to enumerating files, and getting a uniq diskid/fileid ids
        RetVal = ERROR_WRONG_DISK;
      }
    }
    else
    {
      a_pVolumeSerialNumber = FAT_SERIAL_NUMBER;
    }
  }
  else
  {
    // Rootpath already in the map
    a_pVolumeSerialNumber = (*iterDsm).second;
  }

  return RetVal;
}

int 
DiskSerialCache::
Load(
  FILE*     a_File
)
{
  int size;
  fwscanf_s(a_File, L"%x\n", &size);
  for (int i = 0; i < size; ++i)
  {
    wchar_t LocalRootPathName[MAX_PATH] = { 0 };
    DWORD VolumeSerialNumber;
    fwscanf_s (a_File, L"%[^\"]\"%x\n", LocalRootPathName, MAX_PATH, &VolumeSerialNumber);

    size_t  Len = wcslen(LocalRootPathName) + 1;
    wchar_t* RootPathName = new wchar_t[Len];
    wcscpy_s(RootPathName, Len, LocalRootPathName);

    m_DiskSerialMap.insert(_DiskSerialMap_Pair(RootPathName, VolumeSerialNumber));
    m_DiskSerialInverseMap.insert(_DiskSerialInverseMap_Pair(VolumeSerialNumber, RootPathName));
  }

  return 42;
}

int 
DiskSerialCache::
Save(
  FILE*     a_File
)
{
  _DiskSerialMap::iterator iter;
  fwprintf(a_File, L"%zx\n", m_DiskSerialMap.size());
  for (iter = m_DiskSerialMap.begin(); iter != m_DiskSerialMap.end(); ++iter)
    fwprintf(a_File, L"%s\"%x\n", iter->first, iter->second);

  return 42;
}


DiskSerialCache::
~DiskSerialCache()
{
  for ( _DiskSerialMap::iterator  iter =  m_DiskSerialMap.begin(); iter != m_DiskSerialMap.end(); ++iter )
    delete [] (*iter).first;

  m_DiskSerialInverseMap.clear();
  m_DiskSerialMap.clear();
}





// defines
#define MAX_FILENAME_LEN HUGE_PATH

// The number of characters at the start of an absolute filename.  e.g. in DOS,
// absolute filenames start with "X:\" so this value should be 3, in UNIX they start
// with "\" so this value should be 1.
#define ABSOLUTE_NAME_START 3

// set this to '\\' for DOS or '/' for UNIX
#define SLASH '\\'



/*
 *
 *
 * DupeMerge related stuff
 *
 *
 */

int
FileInfoContainer::
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
)
{
  SetIncludeList(aIncludeFileList);
  SetIncludeDirList(aIncludeDirList);
  SetExcludeList(aExcludeFileList);
  SetExcludeDirList(aExcludeDirList);

  if (apContext)
  {
    DupeMergeParams* pDupeMergeParams = new DupeMergeParams;
    pDupeMergeParams->m_SrcPathList = aSrcPathList;
    pDupeMergeParams->m_DebugFlags = aDebugFlags;
    pDupeMergeParams->m_Stats = pStats;
    pDupeMergeParams->m_PathNameStatusList = aPathNameStatusList;
    pDupeMergeParams->m_pContext = apContext;

    pDupeMergeParams->m_This = this;

    // Run on a thread when given a context
    DWORD ThreadId;
    apContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunDupeMerge), 
      pDupeMergeParams,
      0, 
      &ThreadId);

    return 0;
  }
  else
    return _DupeMerge(aSrcPathList, aDebugFlags, pStats, aPathNameStatusList, apContext);
}

DWORD
__stdcall
FileInfoContainer::
RunDupeMerge(      
  DupeMergeParams* p
)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_DupeMerge(p->m_SrcPathList, p->m_DebugFlags, p->m_Stats, p->m_PathNameStatusList, pCtx);
  delete p;

	return errOk;
}

int
FileInfoContainer::
_DupeMerge(
  _ArgvList&            aSrcPathList, 
  int                   aDebugFlags,
  CopyStatistics*	      pStats,
  _PathNameStatusList*  a_PathNameStatusList,
  AsyncContext*         apContext
)
{
	pStats->Proceed (CopyStatistics::eVoid);
	GetSystemTime(&pStats->m_StartTime);

	// Scan all the files given via the command line arguments
	pStats->Proceed (CopyStatistics::eScanning);

  // Slurp After Find
  FILE* SlurpAfterFind = NULL;
  if (aDebugFlags & eSlurpAfterFind)
  {
    // Use this code to save a container which contains uniqe constellation of testdata
    _ArgvListIterator iter = aSrcPathList.begin();
		_wfopen_s(&SlurpAfterFind, iter->Argv.c_str(), L"rb");

    aSrcPathList.erase(aSrcPathList.begin());
  }

  // Dump After Find
  FILE* DumpAfterFind = NULL;
  if (aDebugFlags & eDumpAfterFind)
  {
    // Use this code to save a container which contains uniqe constellation of testdata
    _ArgvListIterator iter = aSrcPathList.begin();
		_wfopen_s(&DumpAfterFind, iter->Argv.c_str(), L"wb");

    aSrcPathList.erase(aSrcPathList.begin());
  }

  // Slurp After Calc
  FILE* SlurpAfterCalc = NULL;
  if (aDebugFlags & eSlurpAfterCalc)
  {
    // Use this code to save a container which contains uniqe constellation of testdata
    _ArgvListIterator iter = aSrcPathList.begin();
		_wfopen_s(&SlurpAfterCalc, iter->Argv.c_str(), L"rb");

    aSrcPathList.erase(aSrcPathList.begin());
  }

  // Dump After Calc
  FILE* DumpAfterCalc = NULL;
  if (aDebugFlags & eDumpAfterCalc)
  {
    // Use this code to save a container which contains uniqe constellation of testdata
    _ArgvListIterator iter = aSrcPathList.begin();
		_wfopen_s(&DumpAfterCalc, iter->Argv.c_str(), L"wb");

    aSrcPathList.erase(aSrcPathList.begin());
  }

  if (SlurpAfterFind)
  {
    Load(SlurpAfterFind, eDupeMergePersistance);
    fclose(SlurpAfterFind);
  }
  else
  {
    FindHardLink(aSrcPathList, 1, pStats, a_PathNameStatusList, NULL);
  }

  // Dump After Find
  if (DumpAfterFind)
  {
    Save(DumpAfterFind, eDupeMergePersistance);
    fclose(DumpAfterFind);
  }

  // Preparation steps for DupeMerge
  Prepare(eSmartClone, pStats);

	_Pathes::iterator	begin = m_HardlinkBegin;
	_Pathes::iterator	end = m_FATFilenameBegin;
  CalcRefCount(begin, end);

  pStats->Proceed (CopyStatistics::eScanned);

	// Create Hashes for the files
	pStats->Proceed (CopyStatistics::ePreparation);

	// Remove files ocuring twice or more after the search 
	sort (begin, end, PathesSorter());
	end = unique(begin, end, PathCompare());
	

#if defined DMDUMP
	wprintf (L"[Begin] After scan Dump\n");
	DmDump(begin, end);
	wprintf (L"[End] After scan Dump\n");
#endif

	DmFindDupes (begin, end, a_PathNameStatusList, pStats);

  // Slurp After Calc
  if (SlurpAfterCalc)
  {
    Load(SlurpAfterCalc, eDupeMergePersistance);
    fclose(SlurpAfterCalc);
  }

  // Dump After Calc
  if (DumpAfterCalc)
  {
    Save(DumpAfterCalc, eDupeMergePersistance);
    fclose(DumpAfterCalc);
  }

  pStats->Proceed (CopyStatistics::ePreparated);

	if (!m_Filenames.empty())
	{
		// Merge the dupes
		pStats->Proceed (CopyStatistics::eDupeMerge);
		InitCreateHardlink ();

		MergeDupes (begin, end, a_PathNameStatusList, pStats);
	}
	pStats->Proceed (CopyStatistics::eDupeMerged);

	GetSystemTime(&pStats->m_EndTime);

	pStats->Proceed (CopyStatistics::eFinished);
	pStats->Proceed (CopyStatistics::eVoid);

	return errOk;
}


// --- Dupe helper stuff

//
// FileIndex Calculation. There are broken NTFS implementations, and thus calculated
// hash sums over all files, and based on this we are recreating file-indices
//

int
FileInfoContainer::
SetFileIndex(
  _Pathes::iterator&	start,
  _Pathes::iterator&	end,
  UL64&               a_FileIndex
)
{
  // Hardlinks do can have a max refcount of 1024 which fits into a int, so we cast
  int RefCount = (int)distance(start, end);
  for (_Pathes::iterator iter = start; iter != end; ++iter)
  {
    (*iter)->m_FileIndex.ul64 = a_FileIndex.ul64;
    (*iter)->m_RefCount = RefCount;
  }

  return errOk;
}

// Run through the files, which have hash-twins, and assign all files with same
// hashes a common file-index
//
int
FileInfoContainer::
CalcFileIndex(
  _Pathes::iterator&	      start,
  _Pathes::iterator&	      end
)
{
	if (start != end)
	{
		// Go through the files which have at least one hash-twin
    // Assume that matching files already are adjacent and have the same hash sum
		_Pathes::iterator	iter = start;
		_Pathes::iterator	last = iter;

		bool							equal = false;
    UL64              FileIndex;
    FileIndex.ul64 = 0x10;
		while (++iter != end)
		{
			if (memcmp ((*iter)->m_Hash, (*last)->m_Hash, FileInfo::cHashSize))
			{
				if (equal)
        {
					SetFileIndex (last, iter, FileIndex);
          FileIndex.ul64++;
        }

				last = iter;
				equal = false;
			}
			else
				equal = true;
		} 
		if (equal)
    {
			SetFileIndex (last, iter, FileIndex);
      FileIndex.ul64++;
    }
	}

	return errOk;
}


//
// RefCount calculation
//
// With the fast enumerate, we do not get refcounts from NTFS, but we get proper
// fileindices. This can be used to calculate the reference count. Kind of emulation
// of the GetFileInfoByHandle() functionality

int
FileInfoContainer::
SetRefCount(
  _Pathes::iterator	start,
  _Pathes::iterator	end
)
{
  // Hardlinks do can have a max refcount of 1024 which fits into a int, so we cast
  int RefCount = (int)distance(start, end);
  for (_Pathes::iterator iter = start; iter != end; ++iter)
    (*iter)->m_RefCount = RefCount;

  return errOk;
}

int
FileInfoContainer::
CalcRefCount(
  _Pathes::iterator	start,
	_Pathes::iterator	end
)
{
	// Go through the files which have at least one FileIndex twin
	_Pathes::iterator	iter = start;
	_Pathes::iterator	last = iter;

  if (start == end)
    return errOk;

  bool							equal = false;
	while (++iter != end)
	{
		if ((*iter)->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64)
		{
			if (equal)
				SetRefCount (last, iter);

			last = iter;
			equal = false;
		}
		else
			equal = true;
	} 
	if (equal)
		SetRefCount (last, iter);

  // at the end delete all fileidx, which are not hardlinks
  for (_Pathes::iterator iter = start; iter != end; ++iter)
  {
    if ( !(*iter)->m_RefCount ) 
    {
      (*iter)->m_FileIndex.ul64 = 0;
      (*iter)->m_RefCount = 1;
    }
    (*iter)->m_Hash64_1 = 0;
  }

  return errOk;
}


// 
//
//

int
FileInfoContainer::
CheckIfModified(
  FileInfo* a_pFileInfoReference
)
{
	int				RetVal = errFileHasChanged;
	HANDLE		fh = CreateFileW (a_pFileInfoReference->m_FileName, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (fh)
	{
		BY_HANDLE_FILE_INFORMATION	FileInformation;
		BOOL	r = GetFileInformationByHandle (fh, &FileInformation);
		if (r)
		{
			// Check if the write times are the same since last access during
			// recursing the filesystem
			int	c = memcmp (&(a_pFileInfoReference->m_LastWriteTime), &FileInformation.ftLastWriteTime, sizeof (FILETIME));
			if (!c)
				RetVal = errOk;
		}
		CloseHandle (fh);
	}
	return RetVal;
}

int
FileInfoContainer::
OutputDupes(
  int     aMode
)
{
  int LogLevel = m_Flags & eLogMask;
  if (eLogQuiet != LogLevel)
	{
		if (m_Flags & eSortBySize)
		  sort (m_DupeGroups.begin (), m_DupeGroups.end (), DupeGroup::FileSizeSorter ());
		else 
			sort (m_DupeGroups.begin (), m_DupeGroups.end (), DupeGroup::CardinalitySorter ());

    for (_DupeGroups::iterator	iter = m_DupeGroups.begin (); iter != m_DupeGroups.end (); ++iter)
    {
			DupeGroup* pDupeGroup = &*iter;
      if ( (pDupeGroup->m_State & aMode) == aMode)
      {
        sort(pDupeGroup->m_start, pDupeGroup->m_end, PathesSorter());
        DmDump (pDupeGroup->m_start, pDupeGroup->m_end);
      }
    }
	}
	return errOk;
}


// Count the number of different hardlinkgroups in a section
// If a refcount of a file == 1, each of these will get counted
// as single group, so that e.g.  { 1, 1, 1, 2, 2, 3, 3, 3 }
// results in 5 groups
ULONG
FileInfoContainer::
NumberOfHardLinkGroups(
	_Pathes::iterator		start,
	_Pathes::iterator		end
	)
{
		int nHlGroups = 0;
		if (start != end)
		{
			sort(start, end, RefCountSorter());
			
			// Go through the files which have at least one FileIndex twin
			_Pathes::iterator	iter = start;
			_Pathes::iterator	last = iter;

			bool							equal = false;
			while (++iter != end)
			{
				if ((*iter)->m_RefCount != (*last)->m_RefCount )
				{
					if (equal)
						nHlGroups++;
					else
						if ( 1 == (*last)->m_RefCount)
							nHlGroups++;

					last = iter;
					equal = false;
				}
				else
				{
					if ( 1 == (*iter)->m_RefCount)
						nHlGroups++;
					equal = true;
				}
			} 
			if (equal)
				nHlGroups++;
			else
				if ( 1 == (*last)->m_RefCount)
					nHlGroups++;
		}

		return nHlGroups;
}


int
FileInfoContainer::
HardlinkDupes(
  _Pathes::iterator	      start,
	_Pathes::iterator	      end,
  _PathNameStatusList*    a_PathNameStatusList,
  CopyStatistics*	        pStats
)
{
  DupeGroup	dg;

  // Save the start, end and size of a dupegroup, so that
  // output can be sorted later 
  dg.m_start = start;
  dg.m_end = end;
  dg.m_cardinality = distance (start, end);

  // Check if this is a HashDupegroup, which is already 
  // hardlinked. If yes then do not merge them
  if ((distance (start, end) != (*start)->m_RefCount) )
  {
    // We have to sort
    int nHlGroups = NumberOfHardLinkGroups(start, end);

    if (!(m_Flags & eListOnly))
    {
      int								r;

      // Search for an entry which has not changed since the last scan
      // This will be the reference file. Usually the first file
      _Pathes::iterator	reference;
      FileInfo* pFileInfoReference{ nullptr };
      for (reference = start; reference != end; ++reference)
      {
        pFileInfoReference = *reference;
        r = CheckIfModified(pFileInfoReference);
        if (errOk == r)
          break;
      }

      // Now hardlink all files found to the reference file
      if (reference != end && pFileInfoReference)
      {
        WIN32_FILE_ATTRIBUTE_DATA	HardlinkAttributeOldest;

        FILETIME64 HardlinkLastWriteOldest;
        HardlinkLastWriteOldest.ul64DateTime = pFileInfoReference->m_LastWriteTime.ul64DateTime;

        int Hardlinked = 0;
        int LastRefCount = 0;
        for (_Pathes::iterator hardlink = reference + 1; hardlink != end; ++hardlink)
        {
          FileInfo* pFileInfoHardlink = *hardlink;
          r = CheckIfModified (pFileInfoHardlink);
          if (errOk == r)
          {
            // Check if we do not exceed the hardlink limit
            int r = ProbeHardlink(pFileInfoReference->m_FileName);
            if (r < 1023 && r > 0)
            {
              // Save the oldest fileattribute
              if (pFileInfoHardlink->m_LastWriteTime.ul64DateTime < HardlinkLastWriteOldest.ul64DateTime)
              {
                // Preserve timestamp from oldest file
                GetFileAttributesEx(pFileInfoHardlink->m_FileName,
                  GetFileExInfoStandard,
                  &HardlinkAttributeOldest
                );

                HardlinkLastWriteOldest.ul64DateTime = pFileInfoHardlink->m_LastWriteTime.ul64DateTime;
              }

              // Switch the Attributes to normal so that we can operate on the file
              SetFileAttributes(pFileInfoHardlink->m_FileName, FILE_ATTRIBUTE_NORMAL);

              // In General we have to make sure that Createhardlink might fail for whatever reason
              // and in that case the Hardlinking must be rolled back. This can not be acchieved if 
              // the file has already been deleted. We first rename it to a temporary name,
              // see if hardlinking works. If it failed we rename the temporary name back
              // and the 'transaction' is kind of rolled back.
              //

              // temporary rename the file, and after it was successfully hardlinked delete it
              //
              wchar_t* pFileName = PathFindFileName(pFileInfoHardlink->m_FileName);
              if ( pFileName != pFileInfoHardlink->m_FileName )
              {
                // If we found a Filename we are always sure we can step back one character.
                *(pFileName - 1) = 0x00;

                SYSTEMTIME  time;
                GetSystemTime(&time);

                wchar_t TempFileName[HUGE_PATH];
                swprintf_s(TempFileName, HUGE_PATH, L"%s\\DupeMerge%4d%02d%02d%02d%02d%02d%03d%s", 
                  pFileInfoHardlink->m_FileName,
                  time.wYear,
                  time.wMonth,
                  time.wDay,
                  time.wHour,
                  time.wMinute,
                  time.wSecond,
                  time.wMilliseconds,
                  pFileName
                  );

                // restore the path
                *(pFileName - 1) = L'\\';

#if 0
                BOOL MoveOk = false;
#else
                BOOL MoveOk = MoveFile(pFileInfoHardlink->m_FileName, TempFileName);
#endif
                if (MoveOk)
                {
#if 0
                  // DWORD result = ERROR_SUCCESS;
                  DWORD result = ERROR_INVALID_FUNCTION;
#else
                  DWORD result = CreateHardlink (pFileInfoReference->m_FileName, pFileInfoHardlink->m_FileName);
#endif
                  if (ERROR_SUCCESS == result)
                  {
                    // If hardlinking was successfull then delete the file
#if 0
                    BOOL bDeleteOk = false;
#else
                    BOOL bDeleteOk = DeleteFileW (TempFileName);
#endif
                    if (bDeleteOk)
                    {
                      pStats->m_DupeFilesHardlinked++;
                      Hardlinked++;

                      // To calculate the savings we must be carefull, because we can already have dupegroups
                      // { 1, 1, 1, 2, 2, 3, 3, 3 } e.g saves once for each with refcount one, and then 
                      // one time for each dupegroup. In this case the result is 5
                      if (pFileInfoHardlink->m_RefCount == 1)
                      {
                        pStats->m_DupeBytesSaved += pFileInfoHardlink->m_FileSize.ul64;
                      }
                      else
                      {
                        // For each change in reference count add once the filesize as saving
                        if (LastRefCount != pFileInfoHardlink->m_RefCount)
                          pStats->m_DupeBytesSaved += pFileInfoHardlink->m_FileSize.ul64;
                      }
                      LastRefCount = pFileInfoHardlink->m_RefCount;
                    }
                    else
                    {
                      DWORD DeleteError = GetLastError();

                      // Trace a message that deletion of the temorary file didn't work
                      PathNameStatus pns(MinusF, &TempFileName[PATH_PARSE_SWITCHOFF_SIZE], DeleteError);
                      a_PathNameStatusList->push_back(pns);

                      pStats->m_DupeFilesHardlinkFailed++;
                    }


                    // Restore FileAttributes
                    // SetFileAttributes((*hardlink)->m_FileName, FileAttribute.dwFileAttributes);

                  }
                  else // if (ERROR_SUCCESS == result)
                  {
                    // We failed on creating a hardlink
                    DWORD HardlinkError = GetLastError();
                    PathNameStatus pns(StarH, &pFileInfoHardlink->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], HardlinkError);
                    a_PathNameStatusList->push_back(pns);

                    // Rollback the transaction
#if 0
                    BOOL RollBackOk = false;
#else
                    BOOL RollBackOk = MoveFile(TempFileName, pFileInfoHardlink->m_FileName);
#endif
                    if (!RollBackOk)
                    {
                      // Report failure of Rollback
                      DWORD RollBackError = GetLastError();

                      // We failed rolling back via Move from the temporary file
                      PathNameStatus pns(EqualF, &TempFileName[PATH_PARSE_SWITCHOFF_SIZE], RollBackError);
                      a_PathNameStatusList->push_back(pns);
                    }
                  }
                } 
                else // if (MoveOk)
                {
                  DWORD MoveError = GetLastError();

                  // We failed moving a file
                  PathNameStatus pns(EqualF, &pFileInfoHardlink->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], MoveError);
                  a_PathNameStatusList->push_back(pns);

                  pStats->m_DupeFilesHardlinkFailed++;
                } // if (MoveOk)
              }
              else
              {
                // It didn't have a Filename  
              }
            }
            else // if (r < 1023 && r > 0)
            {
              // The limit for hardlinks per file was exceeded
              PathNameStatus pns(StarH, &pFileInfoHardlink->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], ERROR_TOO_MANY_LINKS);
              a_PathNameStatusList->push_back(pns);
            }

          }
          else // if (errOk == r)
          {
            // The file has changed since it was enumerated  
            PathNameStatus pns(StarH, &pFileInfoHardlink->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], ERROR_TIMEOUT);
            a_PathNameStatusList->push_back(pns);
          }
        } // for (_Pathes::iterator hardlink = reference + 1; hardlink != end; ++hardlink)

        // Set the timestamp of the oldest to the hardlink
        HANDLE h = CreateFile(pFileInfoReference->m_FileName,
          FILE_WRITE_ATTRIBUTES,
          FILE_SHARE_READ,
          NULL, 
          OPEN_EXISTING,
          FILE_ATTRIBUTE_NORMAL,
          NULL);

        if (INVALID_HANDLE_VALUE != h ) 
        {
          SetFileTime(h,
            &HardlinkAttributeOldest.ftCreationTime,
            &HardlinkAttributeOldest.ftLastAccessTime,
            &HardlinkAttributeOldest.ftLastWriteTime);

          CloseHandle(h);
        }

        if (Hardlinked > 0)
        {
          pStats->m_DupeGroupsNew++;
          dg.m_State = DupeGroup::eNew;
        }
      }
    } // if (!(m_Flags & eListOnly))
    else
    {
      // Calculate the savings due to hardlinking wrt to already existing hardlingroups. 
      // Do this here, because when used with --list we also must have a proper savings value
      pStats->m_DupeBytesSaved += (nHlGroups - 1) * (*start)->m_FileSize.ul64;

      pStats->m_DupeGroupsNew++;

      pStats->m_DupeFilesHardlinked += distance(start, end) - 1;

      dg.m_State = DupeGroup::eNew;
    } // if (!(m_Flags & eListOnly))
  }

  if (eVoid == dg.m_State)
    pStats->m_DupeGroupsOld++;
  m_DupeGroups.push_back (dg);
  pStats->m_DupeGroupsTotal++;

  return errOk;
}

ULONG64 // aCheckOffset
FileInfoContainer::
CalcDataSize(
  int       a_Round,
  ULONG64&  a_CheckSize
)
{
  ULONG64 CheckOffset;
  if( a_Round <= m_MaxRound)
  {
    CheckOffset = (ULONG64)m_SystemAllocGranularity << (a_Round - eRoundOffset);
    a_CheckSize = CheckOffset;
  }
  else
  {
    int shifts = m_MaxRound - eRoundOffset;
    CheckOffset = (ULONG64)m_SystemAllocGranularity << shifts;
    CheckOffset += ( a_Round - m_MaxRound ) * m_MaxAdressSpace;

    a_CheckSize = m_MaxAdressSpace;
  }

  return CheckOffset;
}

bool
FileInfoContainer::
CreateHashes(
  _Pathes::iterator	      start,
  _Pathes::iterator	      end,
	int					            round,
  _PathNameStatusList*    a_PathNameStatusList,
  CopyStatistics*	        pStats
)
{
	bool							finished = false;

  for (_Pathes::iterator i = start; i != end; ++i)
	{
		
    FileInfo*         pFileInfo = *i;
    switch (round)
		{
				// Read out the round 0 hash, which is the first byte
			case 0:
			{
				// Open the file if we come around the first time
        pStats->m_HeapAllocTime.Start();
        MmfObject* pMmfObject = new MmfObject ();
        pStats->m_HeapAllocTime.Stop();

        int	r = pMmfObject->Open (pFileInfo->m_FileName, a_PathNameStatusList, 0, min(pFileInfo->m_FileSize.ul64, m_SystemAllocGranularity));
				if (errOk == r)
				{
					pFileInfo->m_pMmfObject = pMmfObject;
					unsigned char* pMemView = (unsigned char *)pMmfObject->GetMemoryView ();

          int iResult = MemCpy(pFileInfo->m_Hash, pMemView, 1);
          if (ERROR_SUCCESS != iResult)
          {
            // Failed to read from the view.
            pMmfObject->Close();
            pStats->m_HeapDeletionTime.Start();
            delete pMmfObject;
            pStats->m_HeapDeletionTime.Stop();

            pFileInfo->m_pMmfObject = NULL;
            pFileInfo->InvalidHash();
          }
				}
				else
				{
          pMmfObject->Close();
          pStats->m_HeapDeletionTime.Start();
          delete pMmfObject;
          pStats->m_HeapDeletionTime.Stop();

          pFileInfo->m_pMmfObject = NULL;
					pFileInfo->InvalidHash ();
				}
			}
			break;

				// Read out the round 1 hash, which is the first 16 bytes
			case 1:
			{
				// Did we successfully open that file
				MmfObject* pMmfObject = pFileInfo->m_pMmfObject;
				if (pMmfObject)
				{
					ULONG64	aFileSize = pMmfObject->GetSize ();
					unsigned char* pMemView = (unsigned char *)(pMmfObject->GetMemoryView ());

					ULONG64 aCheckSize;
					if (aFileSize <= 16)
					{
						aCheckSize = aFileSize;
						finished = true;
						pFileInfo->HashFinished();
					}
					else
					{
						aCheckSize = 16;
					}

          // Have to relay to a function, because otherwise exception SEH does not compile
          int iResult = MemCpy(pFileInfo->m_Hash, pMemView, aCheckSize);
          if (ERROR_SUCCESS != iResult)
          {
            // Failed to read from the view.
            pMmfObject->Close();
            pStats->m_HeapDeletionTime.Start();
            delete pMmfObject;
            pStats->m_HeapDeletionTime.Stop();

            pFileInfo->m_pMmfObject = NULL;
            pFileInfo->InvalidHash();
          }
				}
			}
			break;

				// Read out the round 2 hash, which is a MD5 hash for the first 4k
			case 2:
			{
				// Did we successfully open that file
				MmfObject* pMmfObject = pFileInfo->m_pMmfObject;
				if (pMmfObject)
				{
					ULONG64 aFileSize = pMmfObject->GetSize ();
					unsigned char* pMemView = (unsigned char *)(pMmfObject->GetMemoryView ());

					ULONG64 aCheckSize;
					if (aFileSize <= m_SystemAllocGranularity)
					{
						aCheckSize = aFileSize;
						finished = true;
						pFileInfo->HashFinished();
					}
					else
					{
						aCheckSize = m_SystemAllocGranularity;
					}


          MD5	MD5Sum;
			    MD5Sum.Init();
				  int iResult = MD5Sum.Update(pMemView, aCheckSize);
          if (ERROR_SUCCESS != iResult)
          {
            // Failed to read from the view.
            pMmfObject->Close();
            pStats->m_HeapDeletionTime.Start();
            delete pMmfObject;
            pStats->m_HeapDeletionTime.Stop();

            pFileInfo->m_pMmfObject = NULL;
            pFileInfo->InvalidHash();
          }
          else
          {
            MD5Sum.Final();
			      MD5Sum.Get(pFileInfo->m_Hash);

					  pStats->m_DupeCurrentBytesHashed += aCheckSize;

					  pMmfObject->Close ();
          }
				}
			}
			break;

				// Read out the round n hash, which is a MD5 hash
			default:
			{
				ULONG64	aCheckOffset;
        ULONG64	aCheckSize = 0;
        aCheckOffset = CalcDataSize(round, aCheckSize);

        MmfObject* pMmfObject = pFileInfo->m_pMmfObject;
				if (pMmfObject)
				{
          int	r = pMmfObject->Open (pFileInfo->m_FileName, a_PathNameStatusList, aCheckOffset, aCheckSize);
					if (errOk == r)
					{
            ULONG64 aFileSize = pMmfObject->GetSize ();

						// Calculate one round ahead and see if we will fail in the next round
            ULONG64 NextCheckSize = 0;
            ULONG64 NextCheckOffset = CalcDataSize(round + 1, NextCheckSize);
            if (NextCheckOffset >= aFileSize)
						{
							// During the last round we will reach the end of the file, so only check the remaining bytes
              aCheckSize = aFileSize - aCheckOffset;
							finished = true;
							pFileInfo->HashFinished();
						}

						unsigned char* pMemView = (unsigned char*)pMmfObject->GetMemoryView ();

						// Chain the hashsums togehther							
						MD5	MD5Sum;
						MD5Sum.Init ();
            MD5Sum.Update (pFileInfo->m_Hash, FileInfo::cHashSize);
						int iResult = MD5Sum.Update(pMemView, aCheckSize);
            if (ERROR_SUCCESS != iResult)
            {
              pMmfObject->Close();
              pStats->m_HeapDeletionTime.Start();
              delete pMmfObject;
              pStats->m_HeapDeletionTime.Stop();

              pFileInfo->m_pMmfObject = NULL;
						  pFileInfo->InvalidHash ();
            }
            else
            {
						  MD5Sum.Final();
						  MD5Sum.Get(pFileInfo->m_Hash);

						  pStats->m_DupeCurrentBytesHashed += aCheckSize;

						  pMmfObject->Close ();
            }
					}
					else // if (errOk == r)
					{
            pMmfObject->Close();
            pStats->m_HeapDeletionTime.Start();
            delete pMmfObject;
            pStats->m_HeapDeletionTime.Stop();

            pFileInfo->m_pMmfObject = NULL;
						pFileInfo->InvalidHash ();
					}
				}
			}
			break;
		}
	}

	return finished;
}

// Marks one representative of a group of hardlinks
// This also marks properly if the group only contains 
// one member
int
FileInfoContainer::
MarkRep(
  _Pathes::iterator	start,
	_Pathes::iterator	end
)
{
	if (start == end)
		return errEmptySet;

	_Pathes::iterator	iter = start;
	_Pathes::iterator	last = start;
	(*start)->HardlinkRep();
	while (++iter != end)
	{
		(*iter)->HardlinkRep();
		if ((*iter)->m_FileIndex.ul64 == (*last)->m_FileIndex.ul64)
			(*last)->NoHardlinkRep ();
	
		last = iter;
	} 
	(*last)->HardlinkRep ();

	return errOk;
}

int
FileInfoContainer::
MarkSingleHash(
  _Pathes::iterator	start,
  _Pathes::iterator	end
)
{
	_Pathes::iterator	iter = start;
	_Pathes::iterator	last = iter;

	if (iter == end)
		return errEmptySet;

	int	same = 1;
	while (++iter != end)
	{
		if (memcmp ((*iter)->m_Hash, (*last)->m_Hash, FileInfo::cHashSize))
		{
			if (1 == same)
				(*last)->Single ();

			same = 1;
		}
		else if (same)
			same++;

		last = iter;
	} 
	if (1 == same)
		(*last)->Single ();

	return errOk;
}

int
FileInfoContainer::
MarkSingleSize(
  _Pathes::iterator	start,
	_Pathes::iterator	end
)
{
	_Pathes::iterator	iter = start;
	_Pathes::iterator	last = iter;

	if (iter == end)
		return errEmptySet;

	int	same = 1;
	while (++iter != end)
	{
		if (0 == (*last)->m_FileSize.ul64)
			(*last)->Single ();

		if ((*iter)->m_FileSize.ul64 != (*last)->m_FileSize.ul64)
		{
			if (1 == same)
				(*last)->Single ();

			same = 1;
		}
		else if (same)
			same++;

		last = iter;
	} 
	if (1 == same)
		(*last)->Single ();

	return errOk;
}

int
FileInfoContainer::
MergeDupes(
  _Pathes::iterator	      start,
  _Pathes::iterator	      end,
  _PathNameStatusList*    a_PathNameStatusList,
  CopyStatistics*	        pStats
)
{
	if (start != end)
	{
		// Go through the files which have at least one FileIndex twin
    // Assume that matching files already are adjacent and have the same hash sum
		_Pathes::iterator	iter = start;
		_Pathes::iterator	last = iter;

		bool							equal = false;
		while (++iter != end)
		{
			if (memcmp ((*iter)->m_Hash, (*last)->m_Hash, FileInfo::cHashSize))
			{
				if (equal)
					HardlinkDupes (last, iter, a_PathNameStatusList, pStats);

				last = iter;
				equal = false;
			}
			else
				equal = true;
		} 
		if (equal)
			HardlinkDupes (last, iter, a_PathNameStatusList, pStats);
	}

	return errOk;
}

int
FileInfoContainer::
CopyHashes(
  _Pathes::iterator	start,
  _Pathes::iterator	end
)
{
	// Copying hashes only is ok, if the first element
	// is a HlRep
	if (!(*start)->IsHardlinkRep ())
	{
		_Pathes::iterator	iter = start;
		int hf = (*start)->m_Flags & (int)FileInfo::inf_HashFinished;
		for (iter = start + 1; iter != end; ++iter)
		{
			memcpy ((*iter)->m_Hash, (*start)->m_Hash, FileInfo::cHashSize);
			(*iter)->m_Flags |= hf;
		}
	}

	return errOk;
}

/// UpdateHlReps sorts files with same size by FileIndex and 
/// the HlRep flag, so that the HlRep is the first of that
/// group.
/// Then it copies the hash from the first element of that
/// group, the HlRep, to the other elements, with same 
/// fileindex, so that all already hardlinked files,
/// for which no hash has been calculated, have the same
/// hash, so that dupes can be found by sorting with 
/// respect to the hash.
int
FileInfoContainer::
UpdateHlReps(
  _Pathes::iterator	start,
	_Pathes::iterator	end
)
{
	sort (start, end, FileIndexHlSorter ());

	// Go through the files which have at least one FileIndex twin
	_Pathes::iterator	iter = start;
	_Pathes::iterator	last = iter;

	bool							equal = false;
	while (++iter != end)
	{
		FileInfo* pF = *iter;
    if (pF->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64)
		{
			if (equal)
				CopyHashes (last, iter);

			last = iter;
			equal = false;
		}
		else
			equal = true;
	} 
	if (equal)
		CopyHashes (last, iter);

	
  // When we are done with this, it can not be assumed, that the files with the same 
  // hash are adjacent. e.g.

  // 0400:!!70240, 01 0000000000000000 f0921b343c1618a9600f449eda1ea16f v:1 s:0 hl:0 hf:1
  // 0300:!!70240, 01 0000000000000000 f0921b343c1618a9600f449eda1ea16f v:1 s:0 hl:0 hf:1
  // 5100:  70240, 01 0000000000000000 f813e05a4028b528c619151ecc158edf v:1 s:0 hl:0 hf:1
  // 0210:!!70240, 02 000a0000000017aa f0921b343c1618a9600f449eda1ea16f v:1 s:0 hl:1 hf:1
  // 0200:!!70240, 02 000a0000000017aa f0921b343c1618a9600f449eda1ea16f v:1 s:0 hl:0 hf:1
  // 0120:!!70240, 03 0026000000002222 f0921b343c1618a9600f449eda1ea16f v:1 s:0 hl:1 hf:1
  // 0100:!!70240, 03 0026000000002222 f0921b343c1618a9600f449eda1ea16f v:1 s:0 hl:0 hf:1
  // 0110:!!70240, 03 0026000000002222 f0921b343c1618a9600f449eda1ea16f v:1 s:0 hl:0 hf:1
  // 5000:  70240, 04 002d000000002219 f813e05a4028b528c619151ecc158edf v:1 s:0 hl:1 hf:1
  // 5030:  70240, 04 002d000000002219 f813e05a4028b528c619151ecc158edf v:1 s:0 hl:0 hf:1

  // So sort files of same size by hash
  sort(start, end, HashSorter());

	return errOk;
}

// 
int
FileInfoContainer::
FindHashDupes(
  _Pathes::iterator	start,
  _Pathes::iterator	end
)
{
	if (start == end)
		return errEmptySet;

	// Go through the files which have at least one FileSize twin, and for each group with the 
  // same filesize see if we can easily update the hashes.
  //
  // The idea of hlreps is to represent an already existing hardlink group so that a hash sum 
  // is calculated for it. This has happened already when we arrive here. UpdateHlReps now tries to
  // update the other files of this group with for the already calculated hash for the hlRep
  // so that by the end all files have a valid hash.
	_Pathes::iterator	iter = start;
	_Pathes::iterator	last = iter;

	bool							equal = false;
	while (++iter != end)
	{
		FileInfo* pF = *iter;
    if (pF->m_FileSize.ul64 != (*last)->m_FileSize.ul64)
		{
			if (equal)
				UpdateHlReps (last, iter);

			last = iter;
			equal = false;
		}
		else
			equal = true;
	} 
	if (equal)
		UpdateHlReps (last, iter);

	return errOk;
}

int
FileInfoContainer::
FindDupeGroups(
  _Pathes::iterator	      start,
  _Pathes::iterator	      end,
  _PathNameStatusList*    a_PathNameStatusList,
  CopyStatistics*	        pStats
)
{
	if (start != end)
	{
    AsyncContext  CheckDupeContext;
    WaitQueue Pool(m_SystemInfo.dwNumberOfProcessors *12);
//    WaitQueue Pool(2);
    
    // Go through the files which have at least one size twin
		_Pathes::iterator	iter = start;
		_Pathes::iterator	last = iter;

		bool							equal = false;
		while (++iter != end)
		{
		  FileInfo* pF = *iter;
			if (pF->m_FileSize.ul64 != (*last)->m_FileSize.ul64)
			{
				if (equal)
        {
					CheckDupes (last, iter, a_PathNameStatusList, pStats, &CheckDupeContext);
          Pool.Insert(CheckDupeContext.m_WaitEvent);
        }

				last = iter;
				equal = false;
			}
			else
				equal = true;
		} 
		if (equal)
    {
			CheckDupes (last, iter, a_PathNameStatusList, pStats, &CheckDupeContext);
      Pool.Insert(CheckDupeContext.m_WaitEvent);
    }
    
    Pool.WaitMultiple();
    CheckDupeContext.m_WaitEvent = NULL;
	}

  

	return errOk;
}

int
FileInfoContainer::
CheckDupes(
  _Pathes::iterator	      a_Start,
  _Pathes::iterator	      a_End,
  _PathNameStatusList*    a_PathNameStatusList,
  CopyStatistics*	        a_pStats,
  AsyncContext*           a_pContext
)
{
  if (a_pContext)
  {
    CheckDupesParams* pCheckDupesParams  = new CheckDupesParams;
    pCheckDupesParams->m_Start = a_Start;
    pCheckDupesParams->m_End = a_End;
    pCheckDupesParams->m_PathNameStatusList = a_PathNameStatusList;
    pCheckDupesParams->m_pStats = a_pStats;
    pCheckDupesParams->m_pContext = a_pContext;
    pCheckDupesParams->m_This = this;
    pCheckDupesParams->m_pTlsStats = new CopyStatistics;

    // Run on a thread when given a context
    DWORD ThreadId;
    a_pContext->m_WaitEvent = CreateThread (NULL, 
      STACKSIZE,
      (ThrFuncType) & (FileInfoContainer::RunCheckDupes), 
      pCheckDupesParams,
      0, 
      &ThreadId
    );

    return 1;
  }
  else
    return _CheckDupes(a_Start, a_End, a_PathNameStatusList, a_pStats, a_pContext);
  
}

DWORD
__stdcall
FileInfoContainer::
RunCheckDupes(CheckDupesParams* p)
{
  AsyncContext* pCtx = p->m_pContext;
  pCtx->m_RetVal = p->m_This->_CheckDupes(p->m_Start, p->m_End, p->m_PathNameStatusList, p->m_pTlsStats, p->m_pContext);
  
  p->m_pStats->AddTlsStat(p->m_pTlsStats);
  delete p->m_pTlsStats;

  delete p;

  return 1;
}

int
FileInfoContainer::
_CheckDupes(
  _Pathes::iterator	      start,
  _Pathes::iterator	      end,
  _PathNameStatusList*    a_PathNameStatusList,
  CopyStatistics*	        pStats,
  AsyncContext*           apContext
)

{
	bool							finished;
	int								round = 0;
	_Pathes::iterator	middle = end;

	ULONG64						SaveCurrentBytesHashed = pStats->m_DupeCurrentBytesHashed;
	// Find dupes wrt to incremental hash generation
	do
	{
		finished = CreateHashes (start, middle, round, a_PathNameStatusList, pStats);

		sort (start, middle, HashSorter ());
		int	r = MarkSingleHash (start, middle);
		if (errEmptySet == r)
			break;

		// divide into files with twins and singles
		middle = partition (start, middle, IsUseful ());

		round++;
	}
	while (!finished);

  DisposeMmf(start, end);

	pStats->m_DupeCurrentBytesHashed = SaveCurrentBytesHashed + ((*start)->m_FileSize.ul64) * distance (start, end);
	return errOk;
}


//
// SanityCheck
//

// Invalidate a range of Fileindex. This is needed if the NTFS is broken. See SanityCheck()
int
FileInfoContainer::
InvalidateFileIndex(
  _Pathes::iterator	      start,
  _Pathes::iterator	      end
)
{
  UL64 idx;
  idx.ul64 = 1;
  for (_Pathes::iterator iter = start; iter != end; ++iter)
  {
    (*iter)->m_FileIndex.ul64 = idx.ul64++;
    (*iter)->m_RefCount = 1;
  }

  return errOk;
}

// Check if all given elements have the same filesize
int
FileInfoContainer::
SanitySizeCheck(
  _Pathes::iterator&      start,
  _Pathes::iterator&      end,
  UL64                    filesize
)
{
  int RetVal =  errOk;
  (*start)->m_HardlinkInSet = (int)distance(start, end);
  for (_Pathes::iterator iter = start; iter != end; ++iter)
  {
    if ((*iter)->m_FileSize.ul64 != filesize.ul64)
    {
      RetVal = errBrokenNTFS;
      break;
    }
  }

  return RetVal;
}


// There are NTFS implemenations, which return the same file-index 
// for files of different size. This is impossible. So we have to check 
// in detail
int
FileInfoContainer::
SanityCheck(
  _Pathes::iterator	start,
	_Pathes::iterator	end
)
{
  int RetVal = errOk; 

  // avoid operation on emtpy lists
  if (start == end)
    return RetVal;
  
  // Go through the files which have at least one FileIndex twin
	_Pathes::iterator	iter = start;
	_Pathes::iterator	last = iter;

 	bool							equal = false;
	FileInfo* pF =    *iter;
  UL64              FileSize;
  FileSize.ul64 = (*start)->m_FileSize.ul64;
	while (++iter != end)
	{
		pF = *iter;
    if (pF->m_FileIndex.ul64 != (*last)->m_FileIndex.ul64)
		{
			if (equal)
      {
        if (errOk != SanitySizeCheck(last, iter, FileSize))
        {
          RetVal = errBrokenNTFS;
          break;
        }
      }

			last = iter;
			equal = false;

      FileSize.ul64 = (*last)->m_FileSize.ul64;
		}
		else
			equal = true;
	} 
	if (equal && errOk == RetVal)
  {
		if ( errOk != SanitySizeCheck (last, iter, FileSize) )
      RetVal = errBrokenNTFS;
  }

	return RetVal;
}


// 
// DmFindDupes
// 
int
FileInfoContainer::
DmFindDupes(
  _Pathes::iterator&      begin,
  _Pathes::iterator&      end,
  _PathNameStatusList*    a_PathNameStatusList,
  CopyStatistics*	        pStats
)
{
#if defined DMDUMP
  SetFlags(FileInfoContainer::eDebugDupeMerge);
#endif

  if (begin == end)
		return errEmptySet;
	/*
		 Algorithmus
		 x = Partition (alle files)(files mit fileindex(==hardlinks); rest)
		 sortiere x nach Fileindex
		 Markiere jeweils einen aus jeder hardlinkgruppe in x
		 y = partition (x)(markierte Files; unmarkierte Files)
		 z = [markierte File;Rest]
		 sortiere z nach Filesize
		 Am Schluss wieder ruecksortieren, sodass man 
		 die hardlinks wieder findet
	*/

	//	x = Partition (alle files)(files mit fileindex(==hardlinks); rest)
	_Pathes::iterator	hlborder;
	hlborder = partition (begin, end, IsHardLink ());
	/*
	 * begin()
	 * hlborder
	 * end()
	 */ 

	_Pathes::iterator	TwinBegin;
	// Check if there are hardlinks among the found files
	if (begin != hlborder)
	{
		// If there are hardlinks among the files, we have
		// to get one of them as 'representative' in the list
		// and thus set TwinBegin properly
		
		// sortiere x nach Fileindex
		sort (begin, hlborder, FileIndexSorter ());
#if defined DMDUMP
    DmDump(begin, hlborder);
#endif

		// There are NTFS implemenation, which return the same file-index 
    // for files of different size. This is impossible. 
    if (errOk != SanityCheck(begin, hlborder))
    {
      InvalidateFileIndex(begin, hlborder);

      // No hardlinks among the files, so TwinBegin is
		  // equal to begin
		  TwinBegin = begin;

      // NTFS is broken
      wchar_t BrokenDrive[HUGE_PATH];
      wcscpy_s(BrokenDrive, HUGE_PATH, &(*begin)->m_FileName[PATH_PARSE_SWITCHOFF_SIZE]);
      PathStripToRoot(BrokenDrive);
      PathNameStatus pns(StarH, BrokenDrive, ERROR_INVALID_INDEX);
      a_PathNameStatusList->push_back(pns);
    }
    
		// Markiere jeweils einen aus jeder hardlinkgruppe in x
		MarkRep (begin, hlborder);

		// y = partition (x)(markierte Files; unmarkierte Files)
		TwinBegin = partition (begin, hlborder, IsHardlinkRep (true));
		// Dump all HlReps
#if defined DMDUMP
    DmDump(TwinBegin, hlborder);
#endif
		/*
		 * begin()
		 * TwinBegin
		 * hlBorder^^
		 * end()
		 */ 
	}
	else
		// No hardlinks among the files, so TwinBegin is
		// equal to begin
		TwinBegin = begin;

	// {05
	// Divide into two parts: files with size twin, and without
	sort (TwinBegin, end, DmSizeSorter ());
	// 05}

	// {06
	// We need to mark all files, which have sizes, that occur only once
	// To accomplish this, we had to sort by size in 05
	MarkSingleSize (TwinBegin, end);
	// 06}
	
	// {07
	// We want to get rid off files, which size only occurs once
	// doesn't matter if they are hlreps or just single files
	// So here we are only interested in files of same size 
	// and they should be between TwinBegin and HlRepBegin
	_Pathes::iterator	HlRepBegin;
	HlRepBegin = stable_partition (TwinBegin, end, IsUseful ());
	// Dump all Twins
  //	DmDump(TwinBegin, HlRepBegin);
	/*
	 * begin()
	 * TwinBegin
	 * HlRepBegin
	 * end()
	 */ 
	// 07}

	// Statistics. Sum up the file which are sizedupes
	_Pathes::iterator	iter;

	if (TwinBegin != HlRepBegin)
	{
		for (iter = TwinBegin; iter != HlRepBegin; ++iter)
			pStats->m_DupeTotalBytesToHash += (*iter)->m_FileSize.ul64;

		// Do the real calc on files, to find out if they
		// have dupes
		sort (TwinBegin, HlRepBegin, DmSizeSorter ());
		FindDupeGroups (TwinBegin, HlRepBegin, a_PathNameStatusList, pStats);

#if defined DMDUMP
		HTRACE (L"1 Dump(TwinBegin, HlRepBegin) begin\n");
		DmDump(TwinBegin, HlRepBegin);
		HTRACE (L"1 Dump(TwinBegin, HlRepBegin) end\n");
		_Pathes::iterator	SaveHlRepBegin = HlRepBegin;
#endif

		// Place the singles at the end of the list
		HlRepBegin = stable_partition (TwinBegin, HlRepBegin, IsADupe ());
#if defined DMDUMP
		HTRACE (L"2a Dump(TwinBegin, HlRepBegin) begin\n");
		DmDump(TwinBegin, HlRepBegin);
		HTRACE (L"2a Dump(TwinBegin, HlRepBegin) end\n");

		HTRACE (L"2b Dump(HlRepBegin, SaveHlRepBegin) before begin\n");
		DmDump(HlRepBegin, SaveHlRepBegin);
		HTRACE (L"2b Dump(HlRepBegin, SaveHlRepBegin) before end\n");
#endif
  }


	// Make sure no hardlink rep gets disposed as single.
	// This may happen, if a HlRep got no non hardlinked 
	// file with the same size
	_Pathes::iterator	SingleBegin;
	SingleBegin = stable_partition (HlRepBegin, end, IsHardlinkRep (false));

#if defined DMDUMP
	HTRACE (L"3 Dump(TwinBegin, HlRepBegin) begin\n");
	DmDump(TwinBegin, HlRepBegin);
	HTRACE (L"3 Dump(TwinBegin, HlRepBegin) end\n");

	HTRACE (L"3 Dump(HlRepBegin, SingleBegin) before begin\n");
	DmDump(HlRepBegin, SingleBegin);
	HTRACE (L"3 Dump(HlRepBegin, SingleBegin) before end\n");
#endif

#if defined DMDUMP
	HTRACE (L"3a Dump(SingleBegin, end) before begin\n");
	DmDump(SingleBegin, end);
	HTRACE(L"3a Dump(SingleBegin, end) before end\n");
#endif

  // 
	// Create a pseudo hash for HlReps
	// 
	for (iter = HlRepBegin; iter != SingleBegin; ++iter)
	{
		FileInfo* pF = (*iter);

		MD5	MD5Sum;
		MD5Sum.Init ();
		MD5Sum.Update (pF->m_FileIndex.ulchar, sizeof(pF->m_FileIndex.ul64));
		MD5Sum.Final ();
		MD5Sum.Get (pF->m_Hash);
	}

	// Free up size-singles in the list to satisfy the algorithm
  //
  // We are not allowed to really delete the items via Dispose, because this would punch
  // holes into the allocated memory. See _Dispose() on that problem. To satisfy the alogorithm
  // which relies on not having the singles in the list, we at least delete the Singles from 
  // m_Filesnames and later on delete all the files via m_FilenamesSave
	m_Filenames.erase (SingleBegin, end);
	end = m_Filenames.end();

	// If no dupes were there then give up
	if (m_Filenames.empty())
		return errOk;

	// Sort all Files by size, even the non-HlRep hardlinks, 
	// which have been parked at the beginning of the list
	// This step is necessary, because all dupe groups should 
	// be shown, even the dupe groups, which are already 
	// hardlinked together
  //
  // stable_sort is very important, otherwise already adjacent dupes might get lost
	stable_sort (begin, end, DmSizeSorter());

  // Dump all files, which are in a dupegroup
#if defined DMDUMP
  HTRACE (L"sorted list of dupegroups: Dump(begin, end) before sort\n");
	DmDump(begin, end);
#endif
	/*
	 * begin()
	 * end()
	 */ 

	// By the end find n-tupel and hardlink them 
	FindHashDupes (begin, end);
	
#if defined DMDUMP
  HTRACE (L"sorted list of dupegroups: Dump(begin, end) after FindHashDupes\n");
	DmDump(begin, end);
#endif

	return errOk;
}

int
FileInfoContainer::
DmDump(
  _Pathes::iterator	aBegin,
	_Pathes::iterator	aEnd
)
{
	for (_Pathes::iterator	iter = aBegin; iter != aEnd; ++iter)
	{
		FileInfo*	pF = *iter;

		if (m_Flags & eDebugDupeMerge)
		{
#if !defined LSE_DEBUG
      fwprintf (m_OutputFile, 
#else
      HTRACE(
#endif
        L"%s: %6d, %02d, %08x%08x, %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x, v:%x, s:%x, hl:%x, hf:%x\n",
				PathFindFileNameW (pF->m_FileName), pF->m_FileSize.ul32l,
				pF->m_RefCount, pF->m_FileIndex.ul32h, pF->m_FileIndex.ul32l,
				(UCHAR) pF->m_Hash[0], (UCHAR) pF->m_Hash[1], (UCHAR) pF->m_Hash[2], (UCHAR) pF->m_Hash[3], 
				(UCHAR) pF->m_Hash[4], (UCHAR) pF->m_Hash[5], (UCHAR) pF->m_Hash[6], (UCHAR) pF->m_Hash[7], 
				(UCHAR) pF->m_Hash[8], (UCHAR) pF->m_Hash[9], (UCHAR) pF->m_Hash[10], (UCHAR) pF->m_Hash[11], 
				(UCHAR) pF->m_Hash[12], (UCHAR) pF->m_Hash[13], (UCHAR) pF->m_Hash[14], (UCHAR) pF->m_Hash[15], 
				pF->IsHashValid (), pF->IsSingle (), !pF->IsHardlinkRep (), pF->IsHashFinished());
		}
		else
		{
#if !defined LSE_DEBUG
      fwprintf (m_OutputFile, 
#else
      HTRACE(
#endif
//        L"'%s' %I64d\n", &pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], pF->m_RefCount);
        L"'%s' %I64d\n", &pF->m_FileName[PATH_PARSE_SWITCHOFF_SIZE], pF->m_FileSize.ul64);
		}
	}

#if !defined LSE_DEBUG
      fwprintf (m_OutputFile, 
#else
      HTRACE(
#endif
        L"\n");
	return errOk;
}

