/*
 * Copyright (C) 1999 - 2019, Hermann Schinagl, hermann@schinagl.priv.at
 */


#include "stdafx.h"

#include "LinkShellMenu.h"

// TODO: Im BackupMode über Netzwerklaufwerke haben die FIles 0 Bytes bei Smartmirror. Wenn das in-proc LSE ausgeführt wird geht es
// Das kommt daher, dass man die Rechte nicht hat um die SACL via SMB anzugreifen. Se_SECURITY_NAME reicht nicht. Ob das am Server
// liegt oder am Client hab ich noch nicht rausgefunden

extern HINSTANCE g_hInstance;
extern LSESettings gLSESettings;
extern UINT    g_cRefThisDll;
extern PWCHAR  MenuEntries[eMenue__Free__ * 2];
extern PWCHAR  HelpTextW[eCommandType__Free__];
extern PCHAR   HelpTextA[eCommandType__Free__];
extern PWCHAR  TopMenuEntries[eTopMenu__Free__];


HRESULT
HardLinkExt::
FreeShellSelection (
)
{
  if (NULL != m_pTargets)
  {
    HTRACE(L"delete FreeShellSelection\n");
    delete[] m_pTargets;
    m_pTargets = 0;
    m_nTargets = 0;
    m_bTargetsFlag = 0;
  }
  else
  {
    HTRACE(L"none\n");
  }

  return S_OK;
}

HRESULT
HardLinkExt::
GetShellSelection (
	LPDATAOBJECT			pDataObj
)
{
  HRESULT eRetVal = S_OK;

  // Get the objects we are working into m_pTarget
  if (pDataObj)
  {
    FORMATETC fmte = 
    {
      CF_HDROP, (DVTARGETDEVICE FAR *)NULL,
      DVASPECT_CONTENT, -1,
      TYMED_HGLOBAL 
    };
    STGMEDIUM medium;
    HRESULT hres = pDataObj->GetData(&fmte, &medium);

    if (SUCCEEDED(hres))
    {
      if (medium.hGlobal)
      {
        FreeShellSelection();
        // fetch targets from the Clipboard

        m_nTargets = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, 0, 0);
        m_pTargets = new Target[m_nTargets];
        m_bTargetsFlag = 0;

        HTRACE(L"LSE::GetShellSelection 10: m_nTargets = %08x\n", m_nTargets);

        // Calc the size, so that we can alloc the proper 
        // space on the clipboard
        m_ClipboardSize = 0;

        for (UINT i = 0; i < m_nTargets; i++)
        {
          // Get the path
          DragQueryFile((HDROP)medium.hGlobal, i, m_pTargets[i].m_Path, HUGE_PATH);
          m_ClipboardSize += wcslen(m_pTargets[i].m_Path) * sizeof WCHAR + sizeof WCHAR + sizeof ULONG;
          m_pTargets[i].m_Flags = 0x00;

          HTRACE(L"LSE::GetShellSelection 20: m_pTargets[i].m_Path = %s\n", m_pTargets[i].m_Path);
        }
      }
    }
    eRetVal = hres;
  }
  else
  {
    HTRACE(L"LSE::GetShellSelection 30\n");
    m_nTargets = 0;
    eRetVal = E_FAIL;
  }

  return eRetVal;
}


void
HardLinkExt::
GetFileAttr(
  Target&     aTarget,
  ULONG&      aTargetsFlag,
  const bool  aIsNtfs
)
{
  struct _stat stat;

  _wstat(aTarget.m_Path, &stat);
  if (stat.st_mode & _S_IFDIR)
  {
    // Deal with Directories, Mountpoints, symbolic Links and Junctions

    // Check if the selected dir is a reparse point, and which one
    int ReparseType = ProbeReparsePoint(aTarget.m_Path, NULL);
    switch (ReparseType)
    {
      case REPARSE_POINT_JUNCTION:
      {
        aTarget.m_Flags |= eJunction;
        aTargetsFlag |= eJunction;
        break;
      }

      case REPARSE_POINT_MOUNTPOINT:
      {
        aTarget.m_Flags |= eMountPoint;
        aTargetsFlag |= eMountPoint;
        break;
      }

      case REPARSE_POINT_SYMBOLICLINK:
      {
        aTarget.m_Flags |= eSymbolicLink;
        aTargetsFlag |= eSymbolicLink;
        break;
      }
    }

    // Check if this is a root path, so that we can offer mount points
    if (PathIsRoot(aTarget.m_Path))
    {
      // It was a root dir so it is a Volume
      aTarget.m_Flags |= eVolume;
      aTargetsFlag |= eVolume;
    }
    else
    {
      if (REPARSE_POINT_FAIL == ReparseType)
      {
        // It is a just a plain directory
        aTarget.m_Flags |= eDir;
        aTargetsFlag |= eDir;
      }
    }
  }
  else
  {
    // Deal with files, dangling Symbolic links or dangling Junctions
    if (ProbeSymbolicLink(aTarget.m_Path, NULL))
    {
      aTarget.m_Flags |= eSymbolicLink;
      aTargetsFlag |= eSymbolicLink;
    }
    else
    {
      // It could be a dangling Junction
      if (ProbeJunction(aTarget.m_Path, NULL))
      {
        aTarget.m_Flags |= eJunction;
        aTargetsFlag |= eJunction;
      }
      else 
      {
        // No it is a file
        aTarget.m_Flags |= eFile;
        aTargetsFlag |= eFile;
      }
      
    }
  }

  if (aIsNtfs)
  {
    aTarget.m_Flags |= eNTFS;
    aTargetsFlag |= eNTFS;
  }
}


void EnumerateFolder()
{
  return;

  LPMALLOC pMalloc = NULL; // memory manager, for freeing up PIDLs
  HRESULT hr = SHGetMalloc(&pMalloc);

  LPSHELLFOLDER psfDesktop = NULL; // namespace root for parsing the path
  hr = SHGetDesktopFolder(&psfDesktop);

  // parse path for absolute PIDL, and connect to target folder
  LPITEMIDLIST pidl = NULL; // general purpose
  // ::{1f4de370-d627-11d1-ba4f-00a0c91eedba} Network Computers
  // ::{20d04fe0-3aea-1069-a2d8-08002b30309d} My Computer
  // ::{208d2c60-3aea-1069-a2d7-08002b30309d} My Network Places
  // ::{7007acc7-3202-11d1-aad2-00805fc1270e} Network Connections
//   hr = psfDesktop->ParseDisplayName(NULL, NULL, L"j:\\", NULL, &pidl, NULL);
  hr = psfDesktop->ParseDisplayName(NULL, NULL, L"::{7007acc7-3202-11d1-aad2-00805fc1270e}", NULL, &pidl, NULL);
  LPSHELLFOLDER psfFolder = NULL;
  hr = psfDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&psfFolder);
  pMalloc->Free(pidl);

  LPENUMIDLIST penumIDL = NULL; // IEnumIDList interface for reading contents
  hr = psfFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_SHAREABLE, &penumIDL);

  while (1) {
    // retrieve a copy of next local item ID list
    hr = penumIDL->Next(1, &pidl, NULL);
    if (hr == NOERROR) {
#if 1 // DEBUG_DEFINES

#if 0 // DEBUG_DEFINES
      WIN32_FIND_DATA ffd; // let's cheat a bit :)
      hr = SHGetDataFromIDList(psfFolder, pidl, SHGDFIL_FINDDATA, &ffd, sizeof(WIN32_FIND_DATA));
      if (S_OK == hr)
      {
        HTRACE(L"Name = %s\n", ffd.cFileName);
        // HTRACE ("Type = " << ( (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "dir\n" : "file\n" );
        // HTRACE ("Size = " << ffd.nFileSizeLow << endl;
      }
#else
      struct : public NETRESOURCE
      {
        BYTE  bytes[8192];
      } nr;

      hr = SHGetDataFromIDList(psfFolder, pidl, SHGDFIL_NETRESOURCE, &nr, sizeof(nr));
      if (SUCCEEDED(hr))
      {
        HTRACE(L"Name = %s\n", nr.lpRemoteName);
      }
#endif

#else
      STRRET strDispName;
      WCHAR szDisplayName[MAX_PATH];

      hr = psfFolder->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strDispName);
      hr = StrRetToBuf(&strDispName, pidl, szDisplayName,
        sizeof(szDisplayName));
      HTRACE(L"Name = '%s'\n", szDisplayName);

      //         DWORD dwAttributes = SFGAO_FOLDER;
      DWORD dwAttributes = SFGAO_FILESYSTEM;

      hr = psfFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &dwAttributes);

      struct : public NETRESOURCE
      {
        BYTE  bytes[8192];
      } nr;

      // Now extract info drive info
      LPITEMIDLIST pDriveIdl = NULL; // general purpose
      hr = psfDesktop->ParseDisplayName(NULL, NULL, szDisplayName, NULL, &pDriveIdl, NULL);

      LPSHELLFOLDER psfDriveFolder = NULL;
      hr = psfDesktop->BindToObject(pDriveIdl, NULL, IID_IShellFolder, (void**)&psfDriveFolder);
      pMalloc->Free(pDriveIdl);

      LPENUMIDLIST penumDriveIDL = NULL; // IEnumIDList interface for reading contents
      hr = psfDriveFolder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penumDriveIDL);
      if (SUCCEEDED(hr))
      {
        hr = penumDriveIDL->Next(1, &pDriveIdl, NULL);

        hr = SHGetDataFromIDList(psfDriveFolder, pDriveIdl, SHGDFIL_NETRESOURCE, &nr, sizeof(nr));
        if (SUCCEEDED(hr))
          HTRACE(L"NetWork resource '%s'\n", nr.lpRemoteName);

        psfDriveFolder->Release();
        pMalloc->Free(pDriveIdl);
      }

#endif 

      pMalloc->Free(pidl);
    }
    // the expected "error" is S_FALSE, when the list is finished
    else break;
  }

  // release all remaining interface pointers
  penumIDL->Release();
  psfFolder->Release();
  psfDesktop->Release(); // no longer required
  pMalloc->Release();

}

// Called by the Shell when initializing an extension.
// called each time before QueryContextmenu() is called
STDMETHODIMP
HardLinkExt::
Initialize(
  LPCITEMIDLIST			    pIDFolder,
  const LPDATAOBJECT		pDataObj, 
  const HKEY				    hKeyID
)
{
  HTRACE(L"LSE::Initialize: %08x, %08x, %08x\n", pIDFolder, pDataObj, hKeyID);

  HRESULT		RetVal = E_NOINTERFACE;

  m_ForceContext = false;

  HRESULT r = GetShellSelection(pDataObj);

  // DV_E_FORMATETC: This happens if someone drags a link from IE Adress bar
  //   to explorer, and in this situation LSE should do nothing
  //   if it would do something, it can prevent drag and drop
  //   of URLs completely.
  // E_INVALIDARG: happens if somone drags the Recycle Bin or
  //   any other special folder on the desktop
  if (DV_E_FORMATETC == r || E_INVALIDARG == r)
  {
    HTRACE(L"LSE::Initialize 10 %08x\n", r);
    return E_NOINTERFACE;
  }


  HTRACE(L"LSE::Initialize 11 %08x\n", r);
  m_DragDrop = false;
  // Get the DropTarget
  if (pIDFolder) 
  {
    // pIDFolder is non NULL if Initialize gets called because of
    // the Background or the DragDrop handler
    if (SHGetPathFromIDList ( pIDFolder, m_DropTarget.m_Path ))
    {
      RetVal = S_OK;

      DWORD	FileSystemFlags;
      int   DriveType;
      bool b = IsFileSystemNtfs(m_DropTarget.m_Path, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);
      m_DropTarget.m_Flags = 0x00;
      DWORD dummy = 0;
      GetFileAttr(m_DropTarget, dummy, b);

      // When Folders are not visible and pick drop is selected
      // This is only needed for W2K
      if ( E_FAIL == r )
        m_ForceContext = true;

      m_DragDrop = true;

      HTRACE(L"LSE::Initialize 15 m_ForceContext = %d, m_DragDrop = %d\n", m_ForceContext, m_DragDrop);
    }
    else
    {
      HTRACE(L"LSE::Initialize 20 SHGetPathFromIDList failed\n");
      RetVal = E_INVALIDARG;
    }
  }
  else
  {
    // pIDFolder is NULL if selection got clicked on the
    // left Directory-Only Pane
    if (m_nTargets)
    {
      wcsncpy(m_DropTarget.m_Path, m_pTargets[0].m_Path, HUGE_PATH);
      m_DropTarget.m_Flags = 0x00;

      DWORD	FileSystemFlags;
      int   DriveType;
      bool bIsNtfs = IsFileSystemNtfs(m_DropTarget.m_Path, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);
      DWORD dummy;
      GetFileAttr(m_DropTarget, dummy, bIsNtfs);

      HTRACE(L"LSE::Initialize 25 m_nTargets == %d\n", m_nTargets);
    }
    else
    {
      HTRACE(L"LSE::Initialize 30 m_nTargets == 0\n");
      RetVal = E_INVALIDARG;
    }
  }

  // Only enter this clause, if we were able to select an item
  if ( S_OK == r )
  {
    // This happens if it was from a context menue, either for
    HTRACE(L"LSE::Initialize 40 %08x\n", r);
    BOOL valid = IsClipboardFormatAvailable(m_ClipFormat);
    if (valid && !m_DragDrop)
    {
      // Drop
      BOOL bDrop = FALSE;

      // Check if it is a single target
      if (m_nTargets == 1)
      {
        HTRACE(L"LSE::Initialize 41 Drop ok\n");
        RetVal = NOERROR;
      }
      else
      {
        HTRACE(L"LSE::Initialize 42 Drop multiple\n");
        RetVal = E_INVALIDARG;
      }
    }
    else
    {
      // Pick
      HTRACE(L"LSE::Initialize 43 '%s'\n", m_pTargets[0].m_Path);
      RetVal = NOERROR;

      if (m_nTargets > 0)
      {
        DWORD	FileSystemFlags;
        int   DriveType;
        bool bIsNtfs = IsFileSystemNtfs(m_pTargets[0].m_Path, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);

        // Check the properties of each selected file
        for (UINT i = 0; i < m_nTargets;i++)
        {
          GetFileAttr(m_pTargets[i], m_bTargetsFlag, bIsNtfs);
          HTRACE(L"LSE::Initialize 441 '%s':%08x\n", m_pTargets[i].m_Path, m_pTargets[i].m_Flags);
        }
        HTRACE(L"LSE::Initialize 442 %08x\n", m_bTargetsFlag);
      }

      if (NOERROR != RetVal)
      {
        HTRACE(L"LSE::Initialize 47 m_nTargets %d\n", m_nTargets);
        FreeShellSelection();
      }
    }
  }
  else
  {
    // If DragDrop == true and r == E_FAIL, the extension
    // was called via the background handler, which means someone
    // clicked on the empty background of the right explorer pane
    HTRACE(L"LSE::Initialize 50 %08x\n", r);
  }
  return RetVal;
}


void
HardLinkExt::
InsertCommand(
  HMENU&        a_Submenu,
  UINT&         a_SubmenuIdx,
  UINT&         a_idCmd,
  int           a_MenuIdx,
  CommandType   a_Command,
  UINT&         a_CommandIdx,
  INT&          a_nEntries
)
{
  // Check if a command is already on the list
  for (UINT i = 0; i < a_CommandIdx; i++)
    if (a_Command == m_Command[i])
      return;

  // Count mode
  if (a_nEntries < 0)
  {
    // No it was not on the list so add it
    InsertMenu(a_Submenu, a_SubmenuIdx++, MF_STRING | MF_BYPOSITION, a_idCmd++, MenuEntries[a_MenuIdx]);
    m_Command[a_CommandIdx++] = a_Command;
  }
  else
    // We are only in the count mode
    a_nEntries++;
}

void
HardLinkExt::
CreateContextMenu(
  HMENU&				a_hSubmenu,
  UINT&					a_idCmd,
  UINT&					a_CommandIdx,
  UINT					a_MenuOffset,
  INT&          a_nEntries,
  bool          a_SrcDstOnSameDrive
)
{
  // Create the menue
  UINT SubmenuIdx = 0;

  ULONG bJunction = gLSESettings.GetFlags() & eDisableJunction ? 0 : eJunction;

  // Check if we offer plain hardlinks
  if ((m_bTargetsFlag & eFile) && a_SrcDstOnSameDrive)
  {
    // [0700] Normal hardlinks
    InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardlink + a_MenuOffset, eDropHardLink, a_CommandIdx, a_nEntries);
  }

  // [0100] 
  if (m_bTargetsFlag & (eFile | eDir | eVolume | bJunction | eMountPoint))
  {
    // [0130] Drop a everything but non file object onto everything
    // [0120] Drop a file, create a symbolic link
    InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSymbolicLink + a_MenuOffset, eDropSymbolicLink, a_CommandIdx, a_nEntries);
  }

  // DropSource is a Directory?
  if (m_bTargetsFlag & eDir)
  {
    // DropTarget is a Junction?
    if (m_DropTarget.m_Flags & (eDir | eVolume | bJunction | eMountPoint))
    {
      // [0300] Drop a directory on a directory, create a normal junction
      // [0310] Drop a directory on Volume, create a normal junction
      // [0320] Drop a directory on Junction, create a normal junction
      // [0330] Drop a directory on Mountpoint, create a normal junction
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);

      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }
    }

    // This is for hardlink clones, so check if they are on the same drive
    if (a_SrcDstOnSameDrive && m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint))
    {
      // [0405] Drop a Directory on a Directory, which is create Hardlink clone
      // [0410] Drop a directory on a Junction, which is create Hardlink clone
      // [0415] Drop a directory on a Volume, which is create Hardlink clone
      // [0420] Drop a directory on a Mountpoint, which is create Hardlink clone
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
    }

  } // if ( m_bTargetsFlag & eDir )

  // DropSource is a Junction?
  if (m_bTargetsFlag & bJunction)
  {
    // Droptarget is a Junction, a Directory, a Mountpoint?
    if (m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint))
    {
      // [0200] Drop a junction on an already existing junction, create chains of junctions
      // [0210] Drop a junction on a directory, create chains of junctions
      // [0213] Drop a junction on a Volume, create chains of junctions
      // [0215] Drop a junction on a mountpoint, create chains of junctions
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);

      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }
    }

    // This is for hardlink clones, so check if they are on the same drive
    if (a_SrcDstOnSameDrive && m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint))
    {
      // [0230] Drop a Directory on a Directory, which is create Hardlink clone
      // [0235] Drop a directory on a Junction, which is create Hardlink clone
      // [0240] Drop a directory on a Volume, which is create Hardlink clone
      // [0245] Drop a directory on a Mountpoint, which is create Hardlink clone
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
    }
  } // if ( m_bTargetsFlag & bJunction )

  // DropSource is a Volume?
  if (m_bTargetsFlag & eVolume)
  {
    if (m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint))
    {
      // [0602] Drop a Volume onto a Mountpoint, which is junction creation
      // [0620] Drop a Volume onto a directory, which is junction creation
      // [0635] Drop a Volume onto a Junction, which is junction creation
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);
    }

    if (m_DropTarget.m_Flags & (eDir | bJunction))
    {
      // [0610] Drop a Volume onto a directory, which is the normal mountpoint creation
      // [0630] Drop a Volume on an already existing junction, create a mountpoint inside
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuCreateMountPoint + a_MenuOffset, eDropCreateMountPoint, a_CommandIdx, a_nEntries);
    }

    if (m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint))
    {
      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }
    }

    // This is for hardlink clones, so check if they are on the same drive
    if (a_SrcDstOnSameDrive && m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint))
    {
      // [0635] Drop a Directory on a Directory, which is create Hardlink clone
      // [0640] Drop a directory on a Junction, which is create Hardlink clone
      // [0645] Drop a directory on a Volume, which is create Hardlink clone
      // [0650] Drop a directory on a Mountpoint, which is create Hardlink clone
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
    }

  } // if ( m_bTargetsFlag & eVolume )

  // DropSource is a MountPoint?
  if (m_bTargetsFlag & eMountPoint)
  {
    if (m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint))
    {
      // [0902] Drop a MountPoint onto a MountPoint, which is junction creation
      // [0915] Drop a MountPoint onto a Directory, which is junction creation
      // [0920] Drop a MountPoint onto a Volume, which is junction creation
      // [0925] Drop a MountPoint onto a Junction, which is junction creation
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);

      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }
    }

    if (a_SrcDstOnSameDrive && m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint))
    {
      // [0935] Drop a MountPoint on a Directory, which is create Hardlink clone
      // [0940] Drop a MountPoint on a Junction, which is create Hardlink clone
      // [0945] Drop a MountPoint on a Volume, which is create Hardlink clone
      // [0950] Drop a MountPoint on a Mountpoint, which is create Hardlink clone
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
    }
  } // if ( m_bTargetsFlag & eMountPoint )


  // [0100]
  // DropSource is a Directory?
  if (m_bTargetsFlag & eDir)
  {
    // DropTarget is a Symbolic Link
    if (m_DropTarget.m_Flags & eSymbolicLink)
    {
      // [0427] Drop a Volume on a Symbolic Link can create Junctions
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);

      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }
      if (a_SrcDstOnSameDrive)
      {
        // [0425] Drop a Directory on a Directory, which is create Hardlink clone
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
      }
    }

    // The below if clause is useless but it illustrates the logical flow
    // if (m_DropTarget.m_Flags & (eDir|bJunction|eVolume|eMountPoint|eSymbolicLink) )
    {
      // [0140] Symbolic Link Clones are allowed on almost any type of object except for files
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSymbolicLinkClone + a_MenuOffset, eDropSymbolicLinkClone, a_CommandIdx, a_nEntries);
    }
  }

  // DropSource is a Volume?
  if (m_bTargetsFlag & eVolume)
  {
    // DropTarget is Symbolic Link
    if (m_DropTarget.m_Flags & eSymbolicLink)
    {
      // [0192] Drop a Volume on a Symbolic Link can create Junctions
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);

      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }

      if (a_SrcDstOnSameDrive)
      {
        // [0190] Drop a Volume on a Symbolic Link can create Hardlink Clones
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
      }
    }

    // DropTarget is a Everything
    if (m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint | eSymbolicLink))
    {
      // [0160] Drop a Volume on a Directory can create Symbolic Link Clones
      // [0162] Drop a Volume on a Junction can create Symbolic Link Clones
      // [0164] Drop a Volume on a Volume can create Symbolic Link Clones
      // [0166] Drop a Volume on a Mountpoint can create Symbolic Link Clones
      // [0168] Drop a Volume on a Mountpoint can create Symbolic Link Clones
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSymbolicLinkClone + a_MenuOffset, eDropSymbolicLinkClone, a_CommandIdx, a_nEntries);
    }
  }

  // DropSource is a Junction?
  if (m_bTargetsFlag & bJunction)
  {
    // DropTarget is Symbolic Link
    if (m_DropTarget.m_Flags & eSymbolicLink)
    {
      // [0182] Drop a Junction on a Symbolic Link can create Junctions
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);

      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }

      if (a_SrcDstOnSameDrive)
      {
        // [0180] Drop a Junction on a Symbolic Link can create Hardlink Clones
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
      }
    }

    // DropTarget is everything
    if (m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint | eSymbolicLink))
    {
      // [0170] Drop a Junction on a Directory can create Symbolic Link Clones
      // [0172] Drop a Junction on a Junction can create Symbolic Link Clones
      // [0174] Drop a Junction on a Volume can create Symbolic Link Clones
      // [0176] Drop a Junction on a Mountpoint can create Symbolic Link Clones
      // [0178] Drop a Junction on a Symbolic Link can create Symbolic Link Clones
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSymbolicLinkClone + a_MenuOffset, eDropSymbolicLinkClone, a_CommandIdx, a_nEntries);
    }
  }

  // DropSource is a Mountpoint?
  if (m_bTargetsFlag & eMountPoint)
  {
    // DropTarget is Symbolic Link
    if (m_DropTarget.m_Flags & eSymbolicLink)
    {
      // [0982] Drop a MountPoint on a Symbolic Link can create Junctions
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);

      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }

      if (a_SrcDstOnSameDrive)
      {
        // [0980] Drop a MountPoint on a Symbolic Link can create Hardlink Clones
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
      }
    }

    if (m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint | eSymbolicLink))
    {
      // [0970] Drop a MountPoint on a Directory can create Symbolic Link Clones
      // [0972] Drop a MountPoint on a Junction can create Symbolic Link Clones
      // [0974] Drop a MountPoint on a Volume can create Symbolic Link Clones
      // [0976] Drop a MountPoint on a Mountpoint can create Symbolic Link Clones
      // [0978] Drop a MountPoint on a Symbolic Link can create Symbolic Link Clones
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSymbolicLinkClone + a_MenuOffset, eDropSymbolicLinkClone, a_CommandIdx, a_nEntries);
    }
  }

  // DropSource is a SymbolicLink?
  if (m_bTargetsFlag & eSymbolicLink)
  {
    if (m_DropTarget.m_Flags & (eDir | bJunction | eVolume | eMountPoint | eSymbolicLink))
    {
      // [1000] Drop a Symbolic Link on a Directory can create chains of Symbolic Link
      // [1010] Drop a Symbolic Link on a Junction can create Symbolic Link
      // [1020] Drop a Symbolic Link on a Volume can create Symbolic Link
      // [1030] Drop a Symbolic Link on a Mountpoint can create Symbolic Link
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSymbolicLink + a_MenuOffset, eDropSymbolicLink, a_CommandIdx, a_nEntries);

      // [1100] Drop a Symbolic Link on a Directory can create Junctions
      // [1110] Drop a Symbolic Link on a Junction can create Junctions
      // [1120] Drop a Symbolic Link on a Volume can create Junctions
      // [1130] Drop a Symbolic Link on a Mountpoint can create Junctions
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuJunction + a_MenuOffset, eDropJunction, a_CommandIdx, a_nEntries);

      // [0800] Smart Copies can be done even on the same drive or not so we increment for Smart Copy anywhere
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartCopy + a_MenuOffset, eDropSmartCopy, a_CommandIdx, a_nEntries);

      // [0810] Smart Mirror can be done even on the same drive or not so we increment for Smart Mirror anywhere
      if (!(gLSESettings.GetFlags() & eDisableSmartMirror))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSmartMirror + a_MenuOffset, eDropSmartMirror, a_CommandIdx, a_nEntries);
      }

      // [0810] DeloreanCopy can be done even on the same drive or not so we increment for DeloreanCopy anywhere
      if (!(gLSESettings.GetFlags() & eDisableDeloreanCopy))
      {
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuDeloreanCopy + a_MenuOffset, eDropDeloreanCopy, a_CommandIdx, a_nEntries);
      }

      if (a_SrcDstOnSameDrive)
      {
        // [1400] Drop a Symbolic Link on a Directory can create Hardlink Clones
        // [1410] Drop a Symbolic Link on a Junction can create Hardlink Clones
        // [1420] Drop a Symbolic Link on a Volume can create Hardlink Clones
        // [1430] Drop a Symbolic Link on a Mountpoint can create Hardlink Clones
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuHardLinkClone + a_MenuOffset, eDropHardLinkClone, a_CommandIdx, a_nEntries);
      }

      // [1200] Drop a Symbolic Link on a Directory can create Symbolic Link Clones
      // [1210] Drop a Symbolic Link on a Junction can create Symbolic Link Clones
      // [1220] Drop a Symbolic Link on a Volume can create Symbolic Link Clones
      // [1230] Drop a Symbolic Link on a Mountpoint can create Symbolic Link Clones
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuSymbolicLinkClone + a_MenuOffset, eDropSymbolicLinkClone, a_CommandIdx, a_nEntries);
    }
  }

  // 
  // The Replacement Stuff is always at the end of the menues
  // 

  // [0100]
  // DropSource is a Everything?
  if (m_bTargetsFlag & (eFile | eDir | eVolume | bJunction | eMountPoint | eSymbolicLink))
  {
    if ((m_DropTarget.m_Flags & eSymbolicLink) && m_nTargets == 1)
    {
      // [0110] Drop a Directory on an already existing Symbolic Link, do the replace stuff
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuReplaceSymbolicLink + a_MenuOffset, eDropReplaceSymbolicLink, a_CommandIdx, a_nEntries);
    }
  }

  if (m_bTargetsFlag & (eDir | eVolume | bJunction | eMountPoint))
  {
    // DropTarget is a Junction?
    if ((m_DropTarget.m_Flags & bJunction) && m_nTargets == 1)
    {
      // [0400] Drop a directory on an already existing junction, do the replace stuff
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuReplaceJunction + a_MenuOffset, eDropReplaceJunction, a_CommandIdx, a_nEntries);
    }
  }

  if (m_bTargetsFlag & eVolume)
  {
    if ((m_DropTarget.m_Flags & eMountPoint) && m_nTargets == 1)
    {
      // [0600] Drop a Volume onto a a already existing Mountpoint, so replace it
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuReplaceMountPoint + a_MenuOffset, eDropReplaceMountPoint, a_CommandIdx, a_nEntries);
    }
  }

  //
  // Copy Symbolic Links & Junctions
  //

  // DropSource is a Symbolic Link
  if (m_bTargetsFlag & eSymbolicLink)
  {
    if (m_DropTarget.m_Flags & (eDir | eVolume))
    {
      // [1450] Drop a Symbolic Link on a Directory can create Copy Symbolic Link
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuCopySymbolicLink + a_MenuOffset, eDropCopySymbolicLink, a_CommandIdx, a_nEntries);
    }
  }

  // DropSource is a Junction
  if (m_bTargetsFlag & bJunction)
  {
    // Droptarget is a Directory or a Volume
    if (m_DropTarget.m_Flags & (eDir | eVolume))
    {
      // [1460] Drop a junction on an already existing junction, create chains of junctions
      if (!(gLSESettings.GetFlags() & eDisableJunction))
        InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuCopyJunction + a_MenuOffset, eDropCopyJunction, a_CommandIdx, a_nEntries);
    }
  }

  // DropSource is a Mountpoint
  if (m_bTargetsFlag & eMountPoint)
  {
    // DropTarget is Directory or a Volume
    if (m_DropTarget.m_Flags & (eDir | eVolume))
      InsertCommand(a_hSubmenu, SubmenuIdx, a_idCmd, eMenuCopyMountpoint + a_MenuOffset, eDropCopyMountpoint, a_CommandIdx, a_nEntries);
  }

}

void
HardLinkExt::
CreateContextMenu(
	HMENU&				a_hMenu,
	UINT&					a_indexMenu,
	UINT&					a_idCmd,
	UINT&					a_CommandIdx,
	UINT					a_MenuOffset
)
{
  // Check the number of entries to be added

  wchar_t     DropTarget[HUGE_PATH];
  wchar_t     Targets[HUGE_PATH];
  if (ERROR_SUCCESS != ReparseCanonicalize(m_DropTarget.m_Path, DropTarget, HUGE_PATH))
    return;
  if (ERROR_SUCCESS != ReparseCanonicalize(m_pTargets[0].m_Path, Targets, HUGE_PATH))
    return;
  bool SrcDstOnSameDrive = CheckIfOnSameDrive(Targets, DropTarget);

  ULONG bJunction = gLSESettings.GetFlags() & eDisableJunction ? 0 : eJunction;
  if ((m_DropTarget.m_Flags & eFile) && (m_bTargetsFlag & (eDir | eVolume | bJunction | eMountPoint | eSymbolicLink)))
    // It is not allowed to do something on a file
    return;

  // Count the entries, which are needed for the menu
  int nEntries = 0;
  CreateContextMenu(a_hMenu, a_idCmd, a_CommandIdx, a_MenuOffset, nEntries, SrcDstOnSameDrive);

  if (nEntries > 1)
  {
    // This clause expects just to offer a submenue for many entries on the main drop down menue
    HMENU hSubmenu = CreatePopupMenu();

    // This time really add the menue
    nEntries = -1;
    CreateContextMenu(hSubmenu, a_idCmd, a_CommandIdx, a_MenuOffset, nEntries, SrcDstOnSameDrive);


    // Insert the submenu into the ctx menu provided by Explorer.
    MENUITEMINFO mii = { sizeof(MENUITEMINFO) };

    mii.fMask = MIIM_SUBMENU | MIIM_STRING | MIIM_ID;
    mii.wID = a_idCmd++;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = MenuEntries[eMenuDropeAs + a_MenuOffset];
    m_Command[a_CommandIdx++] = eDropAs;

    InsertMenuItem(a_hMenu, a_indexMenu, TRUE, &mii);
  }
  else
  {
    // This clause expects just to offer *one* menue entry on the main drop down menue
    nEntries = -1;
    if (m_DropTarget.m_Flags & eNTFS)
    {
      int TargetsFlag = m_bTargetsFlag & (eFile | eDir | eJunction | eVolume | eMountPoint | eSymbolicLink);
      switch (TargetsFlag)
      {
        case eFile:
        {
          if (m_DropTarget.m_Flags & eNTFS)
          {
            // Some could try to create a symbolic link on a different drive
            InsertCommand(a_hMenu, a_indexMenu, a_idCmd, eMenuDropSymbolicLink + a_MenuOffset, eDropSymbolicLink, a_CommandIdx, nEntries);
          }
          else
          {
            // All other options are hardlinks if we are on the same drive
            if (SrcDstOnSameDrive)
            {
              InsertCommand(a_hMenu, a_indexMenu, a_idCmd, eMenuDropHardlink + a_MenuOffset, eDropHardLink, a_CommandIdx, nEntries);
            }
          }
        }
        break;

        case eDir:
          HTRACE(L"### 1 Menue eDir");
        break;

        case eJunction:
          // Was the destination a junction ...
          if (m_DropTarget.m_Flags & eJunction)
          {
            // ... it was a junction, so lets re-link this junction to a new location
            if (!(gLSESettings.GetFlags() & eDisableJunction))
              InsertCommand(a_hMenu, a_indexMenu, a_idCmd, eMenuDropReplaceJunction + a_MenuOffset, eDropReplaceJunction, a_CommandIdx, nEntries);
          }
          else
          {
            // A  directory was picked, so lets create a junction from a directory
            if (!(gLSESettings.GetFlags() & eDisableJunction))
              InsertCommand(a_hMenu, a_indexMenu, a_idCmd, eMenuDropJunction + a_MenuOffset, eDropJunction, a_CommandIdx, nEntries);
          }
        break;

        case eVolume:
          HTRACE(L"### 1 Menue eVolume");
        break;

        case eMountPoint:
          HTRACE(L"### 1 Menue eMountPint");
        break;

        case eSymbolicLink:
          // A Junction was picked, and with Vista a Symbolic Link can point to a Junction
          if (m_DropTarget.m_Flags & eNTFS)
          {
            InsertCommand(a_hMenu, a_indexMenu, a_idCmd, eMenuSymbolicLink + a_MenuOffset, eDropSymbolicLink, a_CommandIdx, nEntries);
          }
        break;

        default:
          HTRACE(L"### 1 Menue ILLEGAL");
        break;

      } // switch (TargetsFlag)
    } // if (m_DropTarget.m_Flags & eNTFS)
  }
}


// Called by the Shell just before the context menu is displayed. 
// Got to add our menu items here...
STDMETHODIMP			
HardLinkExt::
QueryContextMenu(
	HMENU					hMenu,
	UINT					indexMenu, 
	UINT					idCmdFirst,
	UINT					idCmdLast, 
	UINT					uFlags
)
{
  HTRACE(L"LSE::QueryContextMenu %08x, %08x, %08x, %08x, %08x\n", hMenu, indexMenu, idCmdFirst, idCmdLast, uFlags);

  UINT idCmd = idCmdFirst;
  UINT aCommandIdx = 0;

//  int* p = NULL;
//  *p = 22;


  if (uFlags & CMF_DEFAULTONLY)
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

#if 1  
  const int CMF_RIGHTPANE = 0x00020000;

  // Code for review and tryout
  // If the extended mode is on, then only show the menues if shift is pressed
  // Do this only for the file (right) pane, but not for the explore(left) pane, 
  // since explorer does not send us CMF_EXTENDEDVERBS in the left pane.
  if (gLSESettings.GetFlags() & eEnableExtended)
    if (!(uFlags & CMF_EXTENDEDVERBS) && (uFlags & CMF_RIGHTPANE))
      return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
#endif

  if (uFlags & CMF_EXTENDEDVERBS)
    m_DropTarget.m_Flags |= eExtendedVerbs;

  // Decide on whether we are in a ContextMenu Handler or a Drag-Drop-Handler
  if (uFlags || m_ForceContext)
  {
    // ContextMenu Handler
    BOOL valid = IsClipboardFormatAvailable(m_ClipFormat);
    if (valid)
    {
      // Drop via Contextmenu
      ClipboardToSelection(true);

      InsertMenu(hMenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

      InsertMenu(hMenu, indexMenu++, MF_STRING | MF_BYPOSITION, idCmd++, TopMenuEntries[eTopMenuCancelLinkCreation]);
      m_Command[aCommandIdx++] = eCancelPickLink;

      // Make sure we only drop onto valid items
      if (m_DropTarget.m_Flags & (eDir | eJunction | eVolume | eMountPoint | eSymbolicLink))
        CreateContextMenu(hMenu, indexMenu, idCmd, aCommandIdx, 0);

      FreeShellSelection();
    }
    else
    {
      // Pick via Contextmenu
      if (m_nTargets)
      {
        InsertMenu(hMenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
        InsertMenu(hMenu, indexMenu++, MF_STRING | MF_BYPOSITION, idCmd++, TopMenuEntries[eTopMenuPickLinkSource]);
        m_Command[aCommandIdx++] = ePickLinkSource;
#if !defined REMOVE_DELETE_JUNCTION
        if (m_bTargetsFlag & eJunction)
        {
          InsertMenu(hMenu, indexMenu++, MF_STRING | MF_BYPOSITION, idCmd++, TopMenuEntries[eTopMenuDeleteJunction]);
          m_Command[aCommandIdx++] = eDeleteJunction;
        }
#endif
        if (m_bTargetsFlag & eMountPoint)
        {
          InsertMenu(hMenu, indexMenu++, MF_STRING | MF_BYPOSITION, idCmd++, TopMenuEntries[eTopMenuDeleteMountPoint]);
          m_Command[aCommandIdx++] = eDropDeleteMountPoint;
        }
        InsertMenu(hMenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
      }
    }
  }
  else
  {
    // This is for the drag and drop handler
    CreateContextMenu(hMenu, indexMenu, idCmd, aCommandIdx, eMenue__Free__);
  }

  //Must return number of menu items we added.
  return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, idCmd - idCmdFirst);
}

// Called by the Shell after the user has selected one of the
// menu items that was added through QueryContextMenu().
STDMETHODIMP			
HardLinkExt::
InvokeCommand	(
	LPCMINVOKECOMMANDINFO lpcmi
)
{
  /*/
    if (lpcmi->cbSize == sizeof (CMINVOKECOMMANDINFOEX) )
    {
      HTRACE (L"CMINVOKECOMMANDINFOEX: %08x\n", lpcmi->fMask);
    }
    else
    {
      HTRACE (L"CMINVOKECOMMANDINFO: %08x\n", lpcmi->fMask);
    }
  /*/

  HRESULT hr = E_INVALIDARG;

  if (lpcmi)
  {
    HTRACE(L"LSE::InvokeCommand lpcmi:%08x, HIWORD(lpcmi->lpVerb):%08x, LOWORD(lpcmi->lpVerb):%08x\n", lpcmi, HIWORD(lpcmi->lpVerb), LOWORD(lpcmi->lpVerb));
    if (!HIWORD(lpcmi->lpVerb))
    {
      UINT idCmd = LOWORD(lpcmi->lpVerb);


      switch (m_Command[idCmd])
      {
        case eCancelPickLink:
          CancelPickLink();
        break;

        case ePickLinkSource:
          SelectionToClipboard();
        break;

        case eDropHardLink:
          DropHardLink(m_DropTarget);
        break;

        case eDropSymbolicLink:
          DropSymbolicLink(m_DropTarget);
        break;

        case eDropJunction:
          DropJunction(m_DropTarget);
        break;

        case eDropHardLinkClone:
          DropHardLinkClone(m_DropTarget, lpcmi);
        break;

        case eDropSymbolicLinkClone:
          DropSymbolicLinkClone(m_DropTarget, lpcmi);
        break;

  #if !defined REMOVE_DELETE_JUNCTION
        case eDeleteJunction:
          DeleteJunction();
        break;
  #endif
        case eDropCreateMountPoint:
          DropMountPoint(m_DropTarget);
        break;

        case eDropDeleteMountPoint:
          DeleteMountPoint();
        break;

        case eDropReplaceJunction:
          DropReplaceJunction(m_DropTarget);
        break;

        case eDropSmartCopy:
          DropSmartCopy(m_DropTarget, lpcmi);
        break;

        case eDropReplaceSymbolicLink:
          DropReplaceSymbolicLink(m_DropTarget, lpcmi);
        break;

        case eDropReplaceMountPoint:
          DropReplaceMountPoint(m_DropTarget, lpcmi);
        break;

        case eDropSmartMirror:
          DropSmartMirror(m_DropTarget, lpcmi);
        break;

        case eDropDeloreanCopy:
          DropDeloreanCopy(m_DropTarget, lpcmi);
        break;

        case eDropCopySymbolicLink:
          DropSymbolicLink(m_DropTarget, true);
        break;

        case eDropCopyJunction:
          DropJunction(m_DropTarget, true);
        break;

        case eDropCopyMountpoint:
          DropMountPoint(m_DropTarget, true);
        break;

        default:
          break;
      }
      FreeShellSelection();
      return NOERROR;
    }
  }
  return hr;
}

// Called by the Shell to retrieve the help text 
// for a menu command.
STDMETHODIMP 
HardLinkExt::
GetCommandString(
	UINT_PTR				idCmd,
	UINT					uFlags,
	UINT FAR*				reserved,
	LPSTR					pszName,
	UINT					cchMax
)
{
  HRESULT hr = E_INVALIDARG;

  if (idCmd < eCommandType__Free__)
  {
    switch (uFlags)
    {
      case GCS_HELPTEXTA:
        lstrcpynA(pszName, HelpTextA[m_Command[idCmd]], cchMax);
        hr = S_OK;
      break;

      case GCS_HELPTEXTW:
        lstrcpynW((LPWSTR)pszName, HelpTextW[m_Command[idCmd]], cchMax);
        hr = S_OK;
      break;

      case GCS_VERBA:
        lstrcpynA(pszName, VerbsA[m_Command[idCmd]], cchMax);
        hr = S_OK;
      break;

      case GCS_VERBW:
        lstrcpynW((LPWSTR)pszName, VerbsW[m_Command[idCmd]], cchMax);
        hr = S_OK;
      break;
    }
  }
  return hr;
}

HRESULT
HardLinkExt::
SelectionToClipboard
(
)
{
  // Copy to clipboard in proprietary format
  HGLOBAL clipbuffer = GlobalAlloc(
    GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE,
    m_ClipboardSize + sizeof(ULONG) + sizeof(ULONG)
  );
  BYTE* pData = (BYTE*)GlobalLock(clipbuffer);

  // Put the overall flag on the clipboard
  PULONG pCount = (PULONG)pData;
  *pCount = m_bTargetsFlag;
  pData += sizeof m_bTargetsFlag;

  // Put the number of entries on the clipboard
  pCount = (PULONG)pData;
  *pCount = m_nTargets;
  pData += sizeof m_nTargets;

  size_t	len;
  for (ULONG i = 0; i < m_nTargets; i++)
  {
    // Put the string on the clipboard
    len = wcslen(m_pTargets[i].m_Path);

    // Multiply by two for the wide character support
    len *= sizeof WCHAR;

    // Copy data
    memcpy(pData, m_pTargets[i].m_Path, len);
    pData += len;
    *pData++ = 0x00;
    *pData++ = 0x00;

    // Put the flags on the clipboard
    memcpy(pData, &m_pTargets[i].m_Flags, sizeof m_pTargets[i].m_Flags);
    pData += sizeof m_pTargets[i].m_Flags;
  }

  FreeShellSelection();

  if (OpenClipboard(NULL))
  {
    EmptyClipboard();
    SetClipboardData(RegisterClipboardFormat(CFSTR_HARDLINK), clipbuffer);
    GlobalUnlock(clipbuffer);
  }
  CloseClipboard();


  HTRACE(L"LSE::SelectionToClipboard success\n");
  return NOERROR;
}


HRESULT 
HardLinkExt::
ClipboardToSelection( 
	bool aProbe
)
{
	if (OpenClipboard(NULL))
	{
		// Get the data from the clipboard
		PBYTE		buffer = (PBYTE)GetClipboardData(m_ClipFormat);
		PULONG		pLong = (PULONG)buffer;

		if (buffer)
		{
			// Get the overall flag from the clipboard
			m_bTargetsFlag = *pLong;
			buffer += sizeof m_bTargetsFlag;

			// Get the number of targets from the clipboard
			pLong = (PULONG)buffer;
			m_nTargets = *pLong;
			buffer += sizeof ULONG;
			
			HTRACE (L"LSE::new ClipboardToSelection %d\n", aProbe);
			m_pTargets = new Target[m_nTargets];
			size_t		len;
			for (ULONG i = 0; i < m_nTargets; i++)
			{
				// Get data from the clipboard
				wcsncpy(m_pTargets[i].m_Path, (PWCHAR)buffer, HUGE_PATH);
				len = wcslen(m_pTargets[i].m_Path);
				len++;
				len *= sizeof WCHAR;
				buffer += len;

				memcpy (&m_pTargets[i].m_Flags, buffer, sizeof m_pTargets[i].m_Flags);
				buffer += sizeof m_pTargets[i].m_Flags;

				if (aProbe)
					break;
			}

			if (!aProbe)
				EmptyClipboard();
		}
		CloseClipboard();
	}
	return NOERROR;
}

HRESULT 
HardLinkExt::
CancelPickLink(
)
{
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		CloseClipboard();
	}
	return NOERROR;
}

HRESULT 
HardLinkExt::
DropHardLink(
	Target&		aTarget
)
{
	WCHAR		dest[HUGE_PATH];

#if defined _DEBUG
  BOOL b = EnableTokenPrivilege(SE_BACKUP_NAME);
  BOOL r = EnableTokenPrivilege(SE_RESTORE_NAME);
  BOOL c = EnableTokenPrivilege(SE_SECURITY_NAME );
#endif

  PathAddBackslash(aTarget.m_Path);

	if (!m_nTargets)
		ClipboardToSelection(false);

  bool ElevationNeeded = false;
  for (ULONG i = 0; i < m_nTargets; i++)
	{
		CreateFileName(
      g_hInstance,
      gLSESettings.GetLanguageID(),
			TopMenuEntries[eTopMenuHardlink], 
			TopMenuEntries[eTopMenuTo], 
			aTarget.m_Path, 
			PathFindFileName(m_pTargets[i].m_Path), 
			dest,
      IDS_STRING_eTopMenuOfOrderXP_1);

		if (m_pTargets[i].m_Flags & eFile)
		{
			int result = CreateHardlink(m_pTargets[i].m_Path, dest);
			if (ERROR_SUCCESS != result)
			{
				// Check if we propably failed due to missing elevation
        if (0 == i && ERROR_ACCESS_DENIED == result)
        {
          ElevationNeeded = true;
          break;
        }
        else
        {
          switch (result)
          {
            case ERROR_TOO_MANY_LINKS:
					    ErrorCreating(dest, 
						    IDS_STRING_ErrExplorerCanNotCreateHardlink, 
						    IDS_STRING_ErrCreatingHardlink,
						    IDS_STRING_ErrHardlinkFailed
					    );
            break;

            case ERROR_ALREADY_EXISTS:
					    ErrorCreating(dest, 
						    IDS_STRING_ErrExplorerAutoRenAlreadyExists, 
						    IDS_STRING_ErrCreatingHardlink,
						    IDS_STRING_ErrHardlinkFailed
					    );
            break;

            default:
					    ErrorFromSystem(result);
            break;

          }
        }
			}
		}
	}
  
  // It could turn out, that we need to elevate the hardlink creation, because the 
  // directories are e.g SYSTEM_ROOT or PROGRAM_FILES
  if (ElevationNeeded)
  {
    UACHelper uacHelper;

    for (ULONG i = 0; i < m_nTargets; i++)
	  {
		  CreateFileName(
        g_hInstance,
        gLSESettings.GetLanguageID(),
			  TopMenuEntries[eTopMenuHardlink], 
			  TopMenuEntries[eTopMenuTo], 
			  aTarget.m_Path, 
			  PathFindFileName(m_pTargets[i].m_Path), 
			  dest,
        IDS_STRING_eTopMenuOfOrderXP_1);

		  if (m_pTargets[i].m_Flags & eFile)
      {
			  // Write the command file, which is read by the elevated process
        uacHelper.WriteArgs('h', m_pTargets[i].m_Path, dest);
      }
    }
    
    DWORD r = uacHelper.Fork();
    if (r)
    {
      switch (r)
      {
        case ERROR_ALREADY_EXISTS:
          ErrorCreating(dest, 
            IDS_STRING_ErrExplorerAutoRenAlreadyExists, 
            IDS_STRING_ErrCreatingHardlink,
            IDS_STRING_ErrHardlinkFailed
            );
        break;

        default:
          ErrorFromSystem(r);
        break;
      }
    }

  } // if (ElevationNeeded)


	return NOERROR;
}

HRESULT 
HardLinkExt::
DropSymbolicLink(
	Target&		aTarget,
  bool      aCopy
)
{
  WCHAR		dest[HUGE_PATH];

  bool Elevation = ElevationNeeded();
  PathAddBackslash(aTarget.m_Path);

  // Retrieve the light settings
  gLSESettings.ReadLSESettings(false);

  if (!m_nTargets)
    ClipboardToSelection(false);

  UACHelper uacHelper;

	// Write the args
  for (ULONG i = 0; i < m_nTargets; ++i)
  {
    wchar_t		dp[MAX_PATH];
    wchar_t*	pFilename = DrivePrefix(m_pTargets[i].m_Path, dp);

    CreateFileName(
      g_hInstance,
      gLSESettings.GetLanguageID(),
      TopMenuEntries[eTopMenuSymbolicLink],
      TopMenuEntries[eTopMenuTo],
      aTarget.m_Path,
      pFilename,
      dest,
      IDS_STRING_eTopMenuOfOrderXP_1);

    // If chains of symbolic links should be created, we have to check wether it is a directory or file, 
    // and set the bits accordingly
    DWORD attr = INVALID_FILE_ATTRIBUTES;
    if (m_pTargets[i].m_Flags & eSymbolicLink)
    {
      attr = GetFileAttributes(m_pTargets[i].m_Path);
      if (attr & FILE_ATTRIBUTE_DIRECTORY)
        m_pTargets[i].m_Flags |= eDir;
      else
        m_pTargets[i].m_Flags |= eFile;
    }

    int SymbolicLinkRelation = gLSESettings.GetFlags() & eForceAbsoluteSymbolicLinks ? 0 : SYMLINK_FLAG_RELATIVE;
    wchar_t target[HUGE_PATH];
    if (aCopy)
    {
      ProbeSymbolicLink(m_pTargets[i].m_Path, target);
      if (PathIsRelative(target))
      {
        SymbolicLinkRelation = SYMLINK_FLAG_RELATIVE;

        wchar_t OrgSymlink[HUGE_PATH];
        LPWSTR pFileName = PathFindFileName(m_pTargets[i].m_Path);
        if (pFileName)
        {
          *pFileName = 0x00;
          wcscpy_s(OrgSymlink, HUGE_PATH, m_pTargets[i].m_Path);
          wcscat_s(OrgSymlink, HUGE_PATH, target);

          // Recalculate a relative target between dest and target
          BOOL bRelPath = PathRelativePathTo(target, dest, FILE_ATTRIBUTE_NORMAL, OrgSymlink, attr & ~FILE_ATTRIBUTE_REPARSE_POINT);
          if (FALSE == bRelPath)
          {
            SymbolicLinkRelation &= ~SYMLINK_FLAG_RELATIVE;
            wcscpy_s(target, HUGE_PATH, OrgSymlink);
          }
        }
      }
      else
        SymbolicLinkRelation = 0;
    }
    else
      wcscpy_s(target, HUGE_PATH, m_pTargets[i].m_Path);

    // Check if two files should be linked together
    if (m_pTargets[i].m_Flags & eFile)
    {
      // With Windows10/14972 a flag can be specified to allow unelevated symlink creation
      DWORD dwSymLinkAllowUnprivilegedCreation = 0x0;
      if (SymLinkAllowUnprivilegedCreation(&gVersionInfo))
      {
        Elevation = false;
        dwSymLinkAllowUnprivilegedCreation = SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
      }

#if defined UAC_FORCE
      if (UAC_OUTPROC)
#else
      if (Elevation)
#endif
      {
        if (SymbolicLinkRelation)
          uacHelper.WriteArgs('f', dest, target);
        else
          uacHelper.WriteArgs('F', dest, target);
      }
      else
      {
        // Used when UAC is switched off thus making it possible  to call CreateSymboliclink directly from explorer
        int SymlinkCreated = CreateSymboliclink(dest,
          target,
          SymbolicLinkRelation | dwSymLinkAllowUnprivilegedCreation
        );

        if (ERROR_ACCESS_DENIED == SymlinkCreated)
        {
          // There is a weird situation. Symbolic Link Creation in developer mode works, but not for Program Files and other protected folders
          // So if we fail to create above, we anyhow have to enable the elevation process. sic
          Elevation = true;
          if (SymbolicLinkRelation)
            uacHelper.WriteArgs('f', dest, target);
          else
            uacHelper.WriteArgs('F', dest, target);
        }
      }
    }

    // Check if two directories should be linked together
    if (m_pTargets[i].m_Flags & (eDir | eJunction | eVolume | eMountPoint | eSymbolicLink))
    {
      WCHAR	DestNoSymlink[HUGE_PATH];
      WCHAR	SourceNoSymlink[HUGE_PATH];

      // Check if recursive symbolic links are about to be created
      ReparseCanonicalize(target, SourceNoSymlink, HUGE_PATH);
      PathAddBackslash(SourceNoSymlink);
      ReparseCanonicalize(dest, DestNoSymlink, HUGE_PATH);
      if (StrStrI(DestNoSymlink, SourceNoSymlink))
      {
        HTRACE(L"LSE::DropSymboliclink ERR: '%s' -> '%s', %ld\n", SourceNoSymlink, DestNoSymlink, m_pTargets[i].m_Flags);
        ErrorCreating(dest,
          IDS_STRING_ErrExplorerCanNotCreateSymlink,
          IDS_STRING_ErrCreatingSymlink,
          IDS_STRING_ErrDestSrcCircular
        );
      }
      else
      {
        // With Windows10/14972 a flag can be specified to allow unelevated symlink creation
        DWORD dwSymLinkAllowUnprivilegedCreation = 0x0;
        if (SymLinkAllowUnprivilegedCreation(&gVersionInfo))
        {
          Elevation = false;
          dwSymLinkAllowUnprivilegedCreation = SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
        }

#if defined UAC_FORCE
        if (UAC_OUTPROC)
#else
        if (Elevation)
#endif
        {
          if (SymbolicLinkRelation)
            uacHelper.WriteArgs('d', dest, target);
          else
            uacHelper.WriteArgs('D', dest, target);
        }
        else
        {
          // Used for debugging purposes, when UAC is switched off thus making it possible to call CreateSymboliclink 
          // directly from explorer, but not from LSEUacHelper.exe
          CreateSymboliclink(dest,
            target,
            SymbolicLinkRelation | dwSymLinkAllowUnprivilegedCreation | SYMLINK_FLAG_DIRECTORY
          );
        }
      }
    }
  }


#if defined UAC_FORCE
  if (UAC_OUTPROC)
#else
  if (Elevation)
#endif
  {
    // Create the Symbolic Links
    DWORD r = uacHelper.Fork();
    if (r)
    {
      switch (r)
      {
      case ERROR_ALREADY_EXISTS:
        ErrorCreating(dest,
          IDS_STRING_ErrExplorerAutoRenAlreadyExists,
          IDS_STRING_ErrCreatingSymlink,
          IDS_STRING_ErrHardlinkFailed
        );
        break;

      default:
        ErrorFromSystem(r);
        break;
      }
    }
  }
  return NOERROR;
}

HRESULT 
HardLinkExt::
DropJunction(
	Target&		aTarget,
  bool      aCopy
)
{
  WCHAR		dest[HUGE_PATH];

  PathAddBackslash(aTarget.m_Path);

  // Only get the data from the clipboard if we do not already
  // have data in m_pTargets
  // The idea behind is to support Pick/Drop in parallel to the
  // drag-drop handler, which does not put data onto the clipboard
  // but does it only via m_pTargets
  if (!m_nTargets)
    ClipboardToSelection(false);

  // Under e.g c:\program Files (x86) it is not allowed to create
  // junctions without UAC, so we have to fork in that case and
  // have all variables prepared
  bool CreateJunctionsViaHelperExe = false;
  UACHelper uacHelper;

  for (ULONG i = 0; i < m_nTargets; i++)
  {
    if (m_pTargets[i].m_Flags & (eDir | eJunction | eVolume | eMountPoint | eSymbolicLink))
    {
      WCHAR	DestNoJunction[HUGE_PATH];
      WCHAR	SourceNoJunction[HUGE_PATH];

      wchar_t		dp[MAX_PATH];
      wchar_t*	pFilename = DrivePrefix(m_pTargets[i].m_Path, dp);

      CreateFileName(
        g_hInstance,
        gLSESettings.GetLanguageID(),
        TopMenuEntries[eTopMenuJunction],
        TopMenuEntries[eTopMenuTo],
        aTarget.m_Path,
        pFilename,
        dest,
        IDS_STRING_eTopMenuOfOrderXP_1);

      wchar_t target[HUGE_PATH];
      if (aCopy)
        ProbeJunction(m_pTargets[i].m_Path, target);
      else
        wcscpy_s(target, HUGE_PATH, m_pTargets[i].m_Path);

      // Check if recursive junctions are about to be created
      ReparseCanonicalize(target, SourceNoJunction, HUGE_PATH);
      PathAddBackslash(SourceNoJunction);
      ReparseCanonicalize(dest, DestNoJunction, HUGE_PATH);
      if (StrStrI(DestNoJunction, SourceNoJunction))
      {
        HTRACE(L"LSE::DropJunction ERR: '%s' -> '%s', %ld\n", SourceNoJunction, DestNoJunction, m_pTargets[i].m_Flags);
        ErrorCreating(dest,
          IDS_STRING_ErrExplorerCanNotCreateJunction,
          IDS_STRING_ErrCreatingJunction,
          IDS_STRING_ErrDestSrcCircular
        );
      }
      else
      {
        HTRACE(L"LSE::DropJunction: '%s' -> '%s', %ld\n", target, dest, m_pTargets[i].m_Flags);
        DWORD ret;
#if defined UAC_FORCE
        if (UAC_OUTPROC)
          ret = ERROR_ALREADY_EXISTS;
        else
          ret = CreateJunction(dest, target);
#else
        ret = CreateJunction(dest, target);
#endif
        if (ERROR_SUCCESS != ret)
        {
          // With Vista & W7, we are not allowed to create Junctions in folders like
          // c:\Program Files (x86) wihtout UAC, so in this special case, the creation
          // of junction has to be relayed to an elevated .exe as done with Symbolic links
#if defined UAC_FORCE
          if (UAC_OUTPROC)
#else
          if ((ERROR_ALREADY_EXISTS != ret) && ElevationNeeded())
#endif
          {
            // Write the command file, which is read by the elevated process
            uacHelper.WriteArgs('j', dest, target);
            CreateJunctionsViaHelperExe = true;
          }
          else
          {
            switch (ret)
            {
            case ERROR_ALREADY_EXISTS:
              ErrorCreating(dest,
                IDS_STRING_ErrExplorerAutoRenAlreadyExists,
                IDS_STRING_ErrCreatingHardlink,
                IDS_STRING_ErrHardlinkFailed
              );
              break;

            default:
              ErrorFromSystem(ret);
              break;
            }
          }
        }
      }

    }

    // If files were selected aside directories or junction
    // create hardlinks for the files
    if (m_pTargets[i].m_Flags & eFile)
    {
      int result = CreateHardlink(m_pTargets[i].m_Path, dest);
      if (ERROR_SUCCESS != result)
        ErrorFromSystem(result);
    }
  }

#if defined UAC_FORCE
  if (UAC_OUTPROC)
#else
  if (CreateJunctionsViaHelperExe)
#endif
  {
    DWORD r = uacHelper.Fork();
    if (r)
    {
      switch (r)
      {
      case ERROR_ALREADY_EXISTS:
        ErrorCreating(dest,
          IDS_STRING_ErrExplorerAutoRenAlreadyExists,
          IDS_STRING_ErrCreatingHardlink,
          IDS_STRING_ErrHardlinkFailed
        );
        break;

      default:
        ErrorFromSystem(r);
        break;
      }
    }
  }

  return NOERROR;
}

HRESULT 
HardLinkExt::
DropHardLinkClone(
	Target&		              aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi
)
{
	return SmartXXX(
    aTarget, 
    lpcmi, 
    IDS_STRING_ProgressCloning,
    eTopMenuHardlinkClone,
    IDS_STRING_ErrExplorerCanNotCreateHardlinkClone,
    IDS_STRING_ErrCreatingHardlinkClone,
    FileInfoContainer::eSmartClone,
    false
  );
}

HRESULT 
HardLinkExt::
DropSymbolicLinkClone(
	Target&		              aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi
)
{
  return SmartXXX(
    aTarget, 
    lpcmi, 
    IDS_STRING_ProgressCloning,
    eTopMenuSymbolicLinkClone,
    IDS_STRING_ErrExplorerCanNotCreateSymlinkClone,
    IDS_STRING_ErrCreatingSymlinkClone,
    FileInfoContainer::eSmartClone,
    true
  );
}

#if !defined REMOVE_DELETE_JUNCTION
HRESULT 
HardLinkExt::
DeleteJunction(
)
{
  bool CreateJunctionsViaHelperExe = false;
  UACHelper uacHelper;

  // Walk through the selection and delete the directories
  // which are junctions
  for (UINT i = 0; i < m_nTargets; i++)
  {
    if (m_pTargets[i].m_Flags & eJunction)
    {
      BOOL b = RemoveDirectory(m_pTargets[i].m_Path);
      if (!b)
      {
        // With Vista & W7, we are not allowed to delete Directories in folders like
        // c:\Program Files (x86) wihtout UAC, so in this special case, the deletion
        // of junction has to be relayed to an elevated .exe as done with Symbolic links
#if defined UAC_FORCE
        if (UAC_OUTPROC)
#else
        if (ElevationNeeded())
#endif
        {
          // Write the command file, which is read by the elevated process
          uacHelper.WriteArgs('k', L"empty", m_pTargets[i].m_Path);
          CreateJunctionsViaHelperExe = true;
        }
        else
        {
          ErrorFromSystem(GetLastError());
        }
      }
    }
  }
#if defined UAC_FORCE
  if (UAC_OUTPROC)
#else
  if (CreateJunctionsViaHelperExe)
#endif
  {
    DWORD r = uacHelper.Fork();
    if (r)
      ErrorFromSystem(r);
  }

  return NOERROR;
}
#endif



HRESULT 
HardLinkExt::
DropMountPoint(
	Target&		aTarget,
  bool      aCopy
)
{
  WCHAR		dest[HUGE_PATH];

  PathAddBackslash(aTarget.m_Path);

  if (!m_nTargets)
    ClipboardToSelection(false);

  // Either Mountpoint creation or Mountpoint Copy are let in
  if ((m_pTargets[0].m_Flags & eVolume) || (m_pTargets[0].m_Flags & eMountPoint && aCopy))
  {
    WCHAR	DestNoJunction[HUGE_PATH];
    WCHAR	SourceNoJunction[HUGE_PATH];

    // Automagically create a directory with the volume name
    // of the source drive. This only happens if you drag
    wchar_t		dp[MAX_PATH];
    wchar_t*	pFilename = DrivePrefix(m_pTargets[0].m_Path, dp);

    CreateFileName(
      g_hInstance,
      gLSESettings.GetLanguageID(),
      TopMenuEntries[eTopMenuMountPoint],
      TopMenuEntries[eTopMenuOf],
      aTarget.m_Path,
      pFilename,
      dest,
      IDS_STRING_eTopMenuOfOrderXP_1);


    wchar_t target[HUGE_PATH], volumeName[HUGE_PATH];
    if (aCopy)
      ProbeMountPoint(m_pTargets[0].m_Path, target, HUGE_PATH, volumeName);
    else
      wcscpy_s(target, HUGE_PATH, m_pTargets[0].m_Path);

    // Check if recursive volume mountpoints are about to be created
    ReparseCanonicalize(target, SourceNoJunction, HUGE_PATH);
    PathAddBackslash(SourceNoJunction);
    ReparseCanonicalize(dest, DestNoJunction, HUGE_PATH);
    if (StrStrI(DestNoJunction, SourceNoJunction))
    {
      ErrorCreating(aTarget.m_Path,
        IDS_STRING_ErrExplorerCanNotCreateMountpoint,
        IDS_STRING_ErrCreatingMountpoint,
        IDS_STRING_ErrDestSrcCircular
      );
    }
    else
    {
      CreateDirectory(dest, NULL);

      int RetVal;
      HTRACE(L"LSE::MountPoint: '%s' -> '%s', %ld\n", target, dest, m_pTargets[0].m_Flags);

#if defined UAC_FORCE
      if (UAC_OUTPROC)
#else
      // Check if we are on Vista. With Vista, thanks to UAC, 
      // we have to fork an extra process to perform this operation
      if (ElevationNeeded())
#endif
      {
        UACHelper uacHelper;

        // Write the command file, which is read by the elevated process
        uacHelper.WriteArgs('m', target, dest);

        uacHelper.Fork();
      }
      else
      {
        RetVal = CreateMountPoint(target, dest);
      }

      if (S_OK != RetVal)
      {
        switch (RetVal)
        {
          case ERROR_ALREADY_EXISTS:
          ErrorCreating(dest,
            IDS_STRING_ErrExplorerAutoRenAlreadyExists,
            IDS_STRING_ErrCreatingHardlink,
            IDS_STRING_ErrHardlinkFailed
          );
          break;

          default:
          ErrorCreating(m_pTargets[0].m_Path,
            IDS_STRING_ErrExplorerCanNotCreateMountpoint,
            IDS_STRING_ErrCreatingMountpoint,
            0
          );
          break;
        }
        RemoveDirectory(dest);
      }
    }
  }
  /*
      // If files were selected aside directories or junction
      // create hardlinks for the files
      if (m_pTargets[i].m_Flags & eFile)
        CreateHardlink(m_pTargets[i].m_Path, dest);
  */
	return NOERROR;
}


HRESULT 
HardLinkExt::
DeleteMountPoint(
)
{
  UACHelper uacHelper;
  bool RelayToSymlink = false;

  for (UINT i = 0; i < m_nTargets; i++)
  {
    DWORD RetVal;
    if (m_pTargets[i].m_Flags & eMountPoint)
    {
      // With Vista it is not allowed to unmount a drive
      // without being Administrator
      if (ElevationNeeded())
      {
        // Write the command file, which is read by the elevated process
        uacHelper.WriteArgs('n', m_pTargets[i].m_Path, L"empty");
        RelayToSymlink = true;
      }
      else
      {
        RetVal = ::DeleteMountPoint(m_pTargets[i].m_Path);
        RemoveDirectory(m_pTargets[i].m_Path);
        if (S_OK != RetVal)
          ErrorFromSystem(RetVal);
      }
    }
  } // for (UINT i = 0; i < m_nTargets; i++)

#if defined UAC_FORCE
  if (UAC_OUTPROC)
#else
  if (RelayToSymlink)
#endif
  {
    DWORD r = uacHelper.Fork();
    if (r)
      ErrorFromSystem(GetLastError());
  }
  return NOERROR;
}

HRESULT
HardLinkExt::
SmartMirror(
	Target&		              aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi,
  int                     aIDS_ProgressBarHeading,
  int                     aIDS_AutoRename,
  int                     aIDS_Failed1,
  int                     aIDS_Failed2,
  FileInfoContainer::CopyReparsePointFlags  aMode
)
{
  WCHAR		dest[HUGE_PATH];
  PathAddBackslash(aTarget.m_Path);

  if (!m_nTargets)
    ClipboardToSelection(false);

  _PathNameStatusList MirrorPathNameStatusList;
  _PathNameStatusList CleanPathNameStatusList;

  // Retrieve the light settings
  gLSESettings.ReadLSESettings(false);

  // ProgressBar
  { 
    Progressbar* pProgressbar = new Progressbar();

    int progress = 0;

    FileInfoContainer MirrorList;
    FileInfoContainer CleanList;

    CopyStatistics	MirrorStatistics;
    GetLocalTime(&MirrorStatistics.m_StartTime);

    CopyStatistics	CleanStatistics;
    GetLocalTime(&CleanStatistics.m_StartTime);

    if (!(gLSESettings.GetFlags() & eForceAbsoluteSymbolicLinks))
      MirrorList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

    if (gLSESettings.GetFlags() & eUnrollReparsePoints)
      MirrorList.SetUnrollDirList(NULL);

    if (gLSESettings.GetFlags() & eSpliceReparsePoints)
      MirrorList.SetSpliceDirList(NULL);
      
    if (gLSESettings.GetFlags() & eBackupMode)
      MirrorList.SetFlags(FileInfoContainer::eBackupMode);
      
    FILE* LogFile = MirrorList.StartLogging(gLSESettings, L"SmartMirror");
    if (LogFile)
    {
      CleanList.SetOutputFile(LogFile);
      CleanList.SetFlags(FileInfoContainer::eLogVerbose);
    }
    else
      CleanList.SetFlags(FileInfoContainer::eLogQuiet);

    // Some could have chosen to copy a few files alongside
    // the directories. That's why we have to add those file
    // to the MirrorList, and add one destination for all files
    // selected alongside the dirs
    size_t FilesSourcePathlen = 0;
    size_t FilesDestPathIdx = -1;

    _ArgvList MirrorPathList;
    _ArgvList CleanPathList;
    // First search all the files, which are about to be copied
    for (ULONG i = 0; i < m_nTargets; i++)
    {
    	_ArgvPath MirrorPath;
    	_ArgvPath CleanPath;
      // Create a filename in 'dest' according to the 
      // e.g 'Hardlink(n) of' syntax
		  wchar_t		dp[MAX_PATH];
		  wchar_t*	pFilename = DrivePrefix(m_pTargets[i].m_Path, dp);

      wcscpy_s(dest, HUGE_PATH, aTarget.m_Path);
      size_t DestLen = wcslen(dest);
      if ('\\' != dest[DestLen - 1] )
        wcscat_s(dest, HUGE_PATH, L"\\");
      wcscat_s(dest, HUGE_PATH, pFilename);

      if (m_pTargets[i].m_Flags & (eDir|eJunction|eVolume|eMountPoint|eSymbolicLink))
      {
        WCHAR	DestNoJunction[HUGE_PATH];
        WCHAR	SourceNoJunction[HUGE_PATH];

        // Check if a recursive Junction is about t be created 
        ReparseCanonicalize(m_pTargets[i].m_Path, SourceNoJunction, HUGE_PATH);
        PathAddBackslash(SourceNoJunction);
        ReparseCanonicalize(dest, DestNoJunction, HUGE_PATH);
        if (StrStrI(DestNoJunction, SourceNoJunction))
        {
          // Bail out. Someone wanted to create recursive <something>
          ErrorCreating(dest, 
            aIDS_Failed1, 
            aIDS_Failed2,
            IDS_STRING_ErrDestSrcCircular
            );
        }
        else
        {
          // For each source location decide which enumeration mode to go
          int DriveType = DRIVE_UNKNOWN;
          DWORD FileSystemFlags;
          IsFileSystemNtfs(m_pTargets[i].m_Path, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);

          if (!PathIsUNC(dest))
            MirrorPath.ArgvDest = PATH_PARSE_SWITCHOFF;
          MirrorPath.ArgvDest += dest;
          CreateDirectory(dest, NULL);

          if (!PathIsUNC(m_pTargets[i].m_Path))
            MirrorPath.Argv =  PATH_PARSE_SWITCHOFF;
          MirrorPath.Argv += m_pTargets[i].m_Path;
          MirrorPath.DriveType = DriveType;
          MirrorPathList.push_back(MirrorPath);

          // Record all path for enumeration
          DriveType = DRIVE_UNKNOWN;
          FileSystemFlags;
          IsFileSystemNtfs(dest, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);

          if (!PathIsUNC(m_pTargets[i].m_Path))
            CleanPath.ArgvDest = PATH_PARSE_SWITCHOFF;
          CleanPath.ArgvDest += m_pTargets[i].m_Path;

          if (!PathIsUNC(dest))
            CleanPath.Argv =  PATH_PARSE_SWITCHOFF;
          CleanPath.Argv += dest;
          CleanPath.DriveType = DriveType;
          CleanPathList.push_back(CleanPath);
        }
      } // if (m_pTargets[i].m_Flags & (eDir|eJunction|eVolume|eMountPoint|eSymbolicLink))
      else
      {
        // If files were selected aside directories or junction
        // copy the files
        if (m_pTargets[i].m_Flags & eFile)
        {
          // Only once add the destination if files are selected alongside directories
          if ( 0 == FilesSourcePathlen )
          {
            PathRemoveBackslash(aTarget.m_Path);

            wchar_t RestoreChar = m_pTargets[i].m_Path[3];
            PathRemoveFileSpec(m_pTargets[i].m_Path);

            _ArgvPath Anchor;
            if (!PathIsUNC(m_pTargets[i].m_Path))
              Anchor.Argv = PATH_PARSE_SWITCHOFF;
            Anchor.Argv += m_pTargets[i].m_Path;

            if (!PathIsUNC(aTarget.m_Path))
              Anchor.ArgvDest = PATH_PARSE_SWITCHOFF;
            Anchor.ArgvDest += aTarget.m_Path;

            FilesDestPathIdx = MirrorList.AddAnchorPath(Anchor);
            size_t len = wcslen(aTarget.m_Path);
            if (aTarget.m_Path[len - 1] != '\\') 
              wcscat_s(aTarget.m_Path, HUGE_PATH, L"\\");

            FilesSourcePathlen = wcslen(m_pTargets[i].m_Path);
            if (PathIsRoot(m_pTargets[i].m_Path))
              m_pTargets[i].m_Path[FilesSourcePathlen--] = RestoreChar;
            else
              m_pTargets[i].m_Path[FilesSourcePathlen] = '\\';

            FilesSourcePathlen += PATH_PARSE_SWITCHOFF_SIZE;

            MirrorList.Lookup(m_pTargets[i].m_Path);
          }

          // TBD With the current implementation files get added properly
          // but when it comes to copying files, it is assumed, that the
          // files in the destination location do not exist. This means
          // that the 'copy of' mechanism does not work. The files are simply
          // not copied.
          // The reason for this is that MirrorList contains a list of Sourcefiles
          // and a destination. During copy source and destination are concatenated
          // and the already existing CreateFileName() is not called
          MirrorList.SetCurSourcePathlen(FilesSourcePathlen);

          // Prepend \\?\ to the sourcepath
          WCHAR	SrcPath[HUGE_PATH];
          wcscpy_s(SrcPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
          wcsncat_s(&SrcPath[PATH_PARSE_SWITCHOFF_SIZE], HUGE_PATH, m_pTargets[i].m_Path, HUGE_PATH);

          MirrorList.SetDestPathIdx(FilesDestPathIdx);

          // If we are in BackupMode we need the privs to add files, which we most propably have the permissions
          // But it will not work with out privs. But we can't get them in context of explorer. So we do a hack
          // and temporarily remove the BackupMode from the container
          if (gLSESettings.GetFlags() & eBackupMode)
            MirrorList.ClearFlags(FileInfoContainer::eBackupMode);
          MirrorList.AddFile(SrcPath, FILE_ATTRIBUTE_NORMAL, 0, &MirrorStatistics, NULL, NULL, REPARSE_POINT_UNDEFINED, false);
          if (gLSESettings.GetFlags() & eBackupMode)
            MirrorList.SetFlags(FileInfoContainer::eBackupMode);

        } // if (m_pTargets[i].m_Flags & eFile )
      }
    } // for (ULONG i = 0; i < m_nTargets; i++)

    // In BackupMode we also have to run the FindHardlink eleavted and thus in LSEUacHelper.exe
    if (gLSESettings.GetFlags() & eBackupMode)
    {
      // Stop bar
      RECT  ProgressbarPosition;
      ZeroMemory(&ProgressbarPosition, sizeof(RECT));
      int ProgessbarVisible = pProgressbar->GetWindowPos(ProgressbarPosition);
      delete pProgressbar;

      // Create File
      UACHelper uacHelper;

      uacHelper.WriteArgs('w', L"not used", L"not used");

      // persist MirrorList
      MirrorList.SetAnchorPath(MirrorPathList);
      MirrorList.Save(uacHelper.File());
      CleanList.SetAnchorPath(CleanPathList);
      CleanList.Save(uacHelper.File());
		  
      // Save the position of the progress bar
      if (!ProgessbarVisible)
        ProgressbarPosition.left = -1;

      uacHelper.SaveProgressbarPosition(ProgressbarPosition);
      MirrorList.StopLogging(LogFile);

      // Fork Process
      DWORD r = uacHelper.Fork();
    }
    else // if (gLSESettings.GetFlags() & eBackupMode)
    {
      // Enumerate all files in the source
      AsyncContext    Context;
      MirrorList.FindHardLink (
        MirrorPathList, 
        0,
        &MirrorStatistics, 
        &MirrorPathNameStatusList,
        &Context
      );

      AsyncContext    CleanContext;
      CleanList.FindHardLink(
        CleanPathList, 
        0,
        &CleanStatistics, 
        &CleanPathNameStatusList,
        &CleanContext
      );

      // In parallel wait for the searches to finish
      while (!Context.Wait(50) || !CleanContext.Wait(50))
      {
        if (pProgressbar->Update(Context, PDM_PREFLIGHT))
        {
          Context.Cancel();
          CleanContext.Cancel();
          Context.Wait(INFINITE);
          CleanContext.Wait(INFINITE);
          break;
        }
      }
      pProgressbar->SetMode(PDM_DEFAULT);

      MirrorList.HeadLogging(LogFile);

      // Only continue if nobody pressed cancel
      if (Context.IsRunning())
      {
        // Reset the Progressbar since we now know the number of files to be copied
        // The cost of an operation depends on the size of files
        Effort MirrorEffort;
        MirrorList.Prepare(FileInfoContainer::eSmartMirror, &MirrorStatistics, &MirrorEffort);
        
        // During Clean the cost of an operation is always 1 so we go with eSmartClone
        Effort CleanEffort;
        CleanList.Prepare(FileInfoContainer::eSmartClean, &CleanStatistics, &CleanEffort);

        // If during search either one file was a symbolic link, we have to elevate the rest
#if defined UAC_FORCE
        if (UAC_OUTPROC)
#else
        int ContainsSymlinks = MirrorList.CheckSymbolicLinks() | CleanList.CheckSymbolicLinks();
        if (ContainsSymlinks)
        {
          // With Windows10/14972 a flag can be specified to allow unelevated symlink creation
          if (SymLinkAllowUnprivilegedCreation(&gVersionInfo))
          {
            // Do not elevate
            ContainsSymlinks = FALSE;
            MirrorList.SetFlags2(FileInfoContainer::eUnprivSymlinkCreate);
            CleanList.SetFlags2(FileInfoContainer::eUnprivSymlinkCreate);
          }
        }
        
        if (
          TRUE == ContainsSymlinks && 
          ElevationNeeded()
        )
#endif
        {
#pragma region ForkSmartCopy
          MirrorList.StopLogging(LogFile);

          // Stop bar
          RECT  ProgressbarPosition;
          ZeroMemory(&ProgressbarPosition, sizeof(RECT));
          int ProgessbarVisible = pProgressbar->GetWindowPos(ProgressbarPosition);
          delete pProgressbar;

          // Create File
          UACHelper uacHelper;

          uacHelper.WriteArgs('w', L"not used", L"not used");

          // persist MirrorList
          MirrorList.Save(uacHelper.File());
          CleanList.Save(uacHelper.File());

          // Save the position of the progress bar
          if (!ProgessbarVisible)
            ProgressbarPosition.left = -1;

          uacHelper.SaveProgressbarPosition(ProgressbarPosition);
          MirrorList.StopLogging(LogFile);
#pragma endregion 

  #if 0 // DEBUG_DEFINES
          // Use this coding to test the integrity of container persistence

          //
          // Read the just written File into a second FileList2
          //
          FILE* SmartCopyReadArgs = _wfopen (&sla_quoted[1], L"rb");
          FileInfoContainer FileList2(PATH_PARSE_SWITCHOFF_SIZE);
          wchar_t line[HUGE_PATH];
          wchar_t* t = fgetws(line, HUGE_PATH, SmartCopyReadArgs);
          FileList2.Load(SmartCopyReadArgs);
          fclose(SmartCopyReadArgs);

          //
          // Then write the content again to a file, and compare the first and second file.
          // If they are equal everything is fine
          //
          uacHelper.WriteArgs('s', L"not used", L"not used");
          FileList2.Save(uacHelper.File());
  #endif
          // persist Pathnamestatuslist

          // Fork Process
          DWORD r = uacHelper.Fork();
        }
        else
        {
          pProgressbar->SetRange(MirrorEffort + CleanEffort);

          Effort        Progress;
          _SmartMirror(CleanList, MirrorList, MirrorStatistics, MirrorPathNameStatusList, nullptr, Progress, pProgressbar);
          delete pProgressbar;

        } // Does not contain symlinks
      } // if Context is still running
      else
      {
        // In case of Cancel during FindHardlink the progressbar has to be deleted
        delete pProgressbar;
      }

      DisposeAsync(MirrorList, CleanList, MirrorStatistics, CleanStatistics);

      GetLocalTime(&CleanStatistics.m_EndTime);
      GetLocalTime(&MirrorStatistics.m_EndTime);
      MirrorList.EndLogging(LogFile, MirrorPathNameStatusList, MirrorStatistics, FileInfoContainer::eSmartMirror);
    } // ProgressBar

    // TBD do the errorhandling based on PathNameStatusList. 
    if (MirrorPathNameStatusList.size())
    {
      // Dump Errors
      _PathNameStatusList::iterator	iter;
      for (iter = MirrorPathNameStatusList.begin (); iter != MirrorPathNameStatusList.end (); ++iter)
      {
        PathNameStatus p = *iter;
        HTRACE (L"%s, %d\n", p.m_PathName, p.m_ErrorCode);
      }
      /*
      if (ERROR_TOO_MANY_LINKS == r)
      ErrorCreating(FailedPath, 
      IDS_STRING_ErrExplorerCanNotCreateHardlink, 
      IDS_STRING_ErrCreatingHardlink,
      IDS_STRING_ErrHardlinkFailed
      );
      else
      ErrorFromSystem(r);
      */
    } // if (PathNameStatusList.size())

    DeletePathNameStatusList(MirrorPathNameStatusList);
    DeletePathNameStatusList(CleanPathNameStatusList);
  }

  return NOERROR;
}


HRESULT
HardLinkExt::
SmartXXX(
  Target&                 aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi,
  int                     aIDS_ProgressBarHeading,
  int                     aIDS_AutoRename,
  int                     aIDS_Failed1,
  int                     aIDS_Failed2,
  FileInfoContainer::CopyReparsePointFlags  aMode,
  bool                    aHardVsSymbolic
)
{
  PathAddBackslash(aTarget.m_Path);

  if (!m_nTargets)
    ClipboardToSelection(false);

  _PathNameStatusList PathNameStatusList;

  // Retrieve the light settings
  gLSESettings.ReadLSESettings(false);

  // ProgressBar
  { 
    Progressbar* pProgressbar = new Progressbar();

    int progress = 0;

    FileInfoContainer FileList;

    CopyStatistics	aStats;
    GetLocalTime(&aStats.m_StartTime);

    FileList.SetFlags(FileInfoContainer::eKeepSymlinkRelation);

    if (aHardVsSymbolic)
      FileList.SetFlags(FileInfoContainer::eSymboliclinkClone);
    
    if (!(gLSESettings.GetFlags() & eForceAbsoluteSymbolicLinks))
      FileList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

    if (gLSESettings.GetFlags() & eUnrollReparsePoints)
      FileList.SetUnrollDirList(NULL);

    if (gLSESettings.GetFlags() & eSpliceReparsePoints)
      FileList.SetSpliceDirList(NULL);
      
    if (gLSESettings.GetFlags() & eBackupMode)
      FileList.SetFlags(FileInfoContainer::eBackupMode);
      
    FILE* LogFile = FileList.StartLogging(gLSESettings, aMode == FileInfoContainer::eSmartCopy ? L"SmartCopy" : L"SmartClone");

    // One could have chosen to copy a few files alongside
    // the directories. That's why we have to add those file
    // to the FileList, and add one destination for all files
    // selected alongside the dirs
    size_t FilesSourcePathlen = 0;
    size_t FilesDestPathIdx = -1;

    // First search all the files, which are about to be copied
    _ArgvList TargetPathList;
    for (ULONG i = 0; i < m_nTargets; i++)
    {
      _ArgvPath TargetPath;

      // Create a filename in 'dest' according to the 
      // e.g 'Hardlink(n) of' syntax
      wchar_t		dp[MAX_PATH];
      wchar_t*	pFilename = DrivePrefix(m_pTargets[i].m_Path, dp);

      WCHAR		dest[HUGE_PATH];

      int rr = CreateFileName(
        g_hInstance,
        gLSESettings.GetLanguageID(),
        TopMenuEntries[aIDS_AutoRename],
        TopMenuEntries[eTopMenuOf], 
        aTarget.m_Path, 
        pFilename,
        dest,
        IDS_STRING_eTopMenuOfOrderXP_1);


      if (m_pTargets[i].m_Flags & (eDir|eJunction|eVolume|eMountPoint|eSymbolicLink))
      {
        WCHAR	DestNoJunction[HUGE_PATH];
        WCHAR	SourceNoJunction[HUGE_PATH];

        // Check if a recursive Junction is about to be created 
        ReparseCanonicalize(m_pTargets[i].m_Path, SourceNoJunction, HUGE_PATH);
        PathAddBackslash(SourceNoJunction);
        ReparseCanonicalize(dest, DestNoJunction, HUGE_PATH);
        if (StrStrI(DestNoJunction, SourceNoJunction))
        {
          // Bail out. Someone wanted to create recursive junctions
          ErrorCreating(dest, 
            aIDS_Failed1, 
            aIDS_Failed2,
            IDS_STRING_ErrDestSrcCircular
            );
        }
        else
        {
          // Record all path for enumeration
          int DriveType = DRIVE_UNKNOWN;
          DWORD FileSystemFlags;
          IsFileSystemNtfs(m_pTargets[i].m_Path, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);

          if (!PathIsUNC(dest))
            TargetPath.ArgvDest = PATH_PARSE_SWITCHOFF;
          TargetPath.ArgvDest += dest;

          if (!PathIsUNC(m_pTargets[i].m_Path))
            TargetPath.Argv = PATH_PARSE_SWITCHOFF;
          TargetPath.Argv += m_pTargets[i].m_Path;
          TargetPath.DriveType = DriveType;
          TargetPath.FileAttribute = FILE_ATTRIBUTE_DIRECTORY;
          TargetPathList.push_back(TargetPath);

        }
      } // if (m_pTargets[i].m_Flags & (eDir|eJunction|eVolume|eMountPoint|eSymbolicLink))
      else
      {
        // If files were selected aside directories or junction
        // copy the files
        if (m_pTargets[i].m_Flags & eFile)
        {
          // Only once add the destination if files are selected alongside directories
          if ( 0 == FilesSourcePathlen )
          {
            PathRemoveBackslash(aTarget.m_Path);

            wchar_t RestoreChar = m_pTargets[i].m_Path[3];
            PathRemoveFileSpec(m_pTargets[i].m_Path);

            _ArgvPath Anchor;
            if (!PathIsUNC(m_pTargets[i].m_Path))
              Anchor.Argv = PATH_PARSE_SWITCHOFF;
            Anchor.Argv += m_pTargets[i].m_Path;

            if (!PathIsUNC(aTarget.m_Path))
              Anchor.ArgvDest = PATH_PARSE_SWITCHOFF;
            Anchor.ArgvDest += aTarget.m_Path;
            Anchor.FileAttribute = FILE_ATTRIBUTE_NORMAL;

            FilesDestPathIdx = FileList.AddAnchorPath(Anchor);
            size_t len = wcslen(aTarget.m_Path);
            if (aTarget.m_Path[len - 1] != '\\') 
            {
              aTarget.m_Path[len] = '\\';
              aTarget.m_Path[len + 1] = 0x00;
            }

            FilesSourcePathlen = wcslen(m_pTargets[i].m_Path);
            if (PathIsRoot(m_pTargets[i].m_Path))
              m_pTargets[i].m_Path[FilesSourcePathlen--] = RestoreChar;
            else
              m_pTargets[i].m_Path[FilesSourcePathlen] = '\\';

            FilesSourcePathlen += PATH_PARSE_SWITCHOFF_SIZE;

            FileList.Lookup(m_pTargets[i].m_Path);
          }

          // TBD With the current implementation files get added properly
          // but when it comes to copying files, it is assumed, that the
          // files in the destination location do not exist. This means
          // that the 'copy of' mechanism does not work. The files are simply
          // not copied.
          // The reason for this is that FileList contains a list of Sourcefiles
          // and a destination. During copy source and destination are concatenated
          // and the already existing CreateFileName() is not called
          FileList.SetCurSourcePathlen(FilesSourcePathlen);

          // Prepend \\?\ to the sourcepath
          WCHAR	SrcPath[HUGE_PATH];
          wcscpy_s(SrcPath, HUGE_PATH, PATH_PARSE_SWITCHOFF);
          wcsncat_s(&SrcPath[PATH_PARSE_SWITCHOFF_SIZE], HUGE_PATH, m_pTargets[i].m_Path, HUGE_PATH);

          FileList.SetDestPathIdx(FilesDestPathIdx);

          // If we are in BackupMode we need the privs to add files, which we most propably have the permissions
          // But it will not work with out privs. But we can't get them in context of explorer. So we do a hack
          // and temporarily remove the BackupMode from the container
          if (gLSESettings.GetFlags() & eBackupMode)
            FileList.ClearFlags(FileInfoContainer::eBackupMode);
          FileList.AddFile(SrcPath, FILE_ATTRIBUTE_NORMAL, 0, &aStats, NULL, NULL, REPARSE_POINT_UNDEFINED, false);
          if (gLSESettings.GetFlags() & eBackupMode)
            FileList.SetFlags(FileInfoContainer::eBackupMode);
        } // if (m_pTargets[i].m_Flags & eFile )
      }
    } // for (ULONG i = 0; i < m_nTargets; i++)

    // Check if we are in Backup Mode. If yes we also have to do the FindHardlink elevated, because
    // it might happen, that FindHardlink should run over directories, which the explorer does not
    // have access permissions.
    if (gLSESettings.GetFlags() & eBackupMode)
    {
      
      // Stop bar
      RECT  ProgressbarPosition;
      ZeroMemory(&ProgressbarPosition, sizeof(RECT));
      int ProgessbarVisible = pProgressbar->GetWindowPos(ProgressbarPosition);
      delete pProgressbar;

      // Create File
      UACHelper uacHelper;

      switch (aMode)
      {
        case FileInfoContainer::eSmartCopy:
          uacHelper.WriteArgs('s', L"not used", L"not used");
          break;
      
        case FileInfoContainer::eSmartClone:
          uacHelper.WriteArgs('t', L"not used", L"not used");
          break;
      }

      // persist FileList
      FileList.SetAnchorPath(TargetPathList);
      FileList.Save(uacHelper.File());
		  
      // Save the position of the progress bar
      if (!ProgessbarVisible)
        ProgressbarPosition.left = -1;
        
      uacHelper.SaveProgressbarPosition(ProgressbarPosition);
      FileList.StopLogging(LogFile);

      // Fork Process
      DWORD r = uacHelper.Fork();
    }
    else // if (gLSESettings.GetFlags() & eBackupMode)
    {
      // Enumerate all files in-proc in .dll
      AsyncContext    Context;

      int r = FileList.FindHardLink (
        TargetPathList,
        0,
        &aStats, 
        &PathNameStatusList,
        &Context
      );

      while (!Context.Wait(250))
      {
        if (pProgressbar->Update(Context, PDM_PREFLIGHT))
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          break;
        }
      }
      pProgressbar->SetMode(PDM_DEFAULT);

      FileList.HeadLogging(LogFile);

      // Only continue if nobody pressed cancel
      if (Context.IsRunning())
      {
        Effort MaxProgress;
        FileList.Prepare(aMode, &aStats, &MaxProgress);

        // If during SmartCopy either one file was a symbolic link, we have
        // to elevate the rest. The same applies if a SymbolicLink Clone should
        // be created. But do this only under > Windows Vista


  #if defined UAC_FORCE
        if (UAC_OUTPROC)
  #else
        int ContainsSymlinks = FileList.CheckSymbolicLinks();
        if (ContainsSymlinks)
        {
          // With Windows10/14972 a flag can be specified to allow unelevated symlink creation
          if (SymLinkAllowUnprivilegedCreation(&gVersionInfo))
          {
            // Do not elevate
            ContainsSymlinks = FALSE;
            FileList.SetFlags2(FileInfoContainer::eUnprivSymlinkCreate);
          }
        }
        
        // Determine if we have to elevate
        if (
          ( TRUE == ContainsSymlinks || aHardVsSymbolic ) && 
          ElevationNeeded()
        )
  #endif
        {
          // Yes Elevation is needed
          FileList.StopLogging(LogFile);
          
          // Stop bar
          RECT  ProgressbarPosition;
          ZeroMemory(&ProgressbarPosition, sizeof(RECT));
          int ProgessbarVisible = pProgressbar->GetWindowPos(ProgressbarPosition);
          delete pProgressbar;

          // Create File
          UACHelper uacHelper;

          switch (aMode)
          {
            case FileInfoContainer::eSmartCopy:
              uacHelper.WriteArgs('s', L"not used", L"not used");
              break;
          
            case FileInfoContainer::eSmartClone:
              uacHelper.WriteArgs('t', L"not used", L"not used");
              break;
          }

          // persist FileList
          FileList.Save(uacHelper.File());
  			  
          // Save the position of the progress bar
          if (!ProgessbarVisible)
            ProgressbarPosition.left = -1;
            
          uacHelper.SaveProgressbarPosition(ProgressbarPosition);
          FileList.StopLogging(LogFile);

  #if 0 // DEBUG_DEFINES
          // Use this coding to test the integrity of container persistence

          //
          // Read the just written File into a second FileList2
          //
          FILE* SmartCopyReadArgs = _wfopen (&sla_quoted[1], L"rb");
          FileInfoContainer FileList2;
				  wchar_t line[HUGE_PATH];
				  wchar_t* t = fgetws(line, HUGE_PATH, SmartCopyReadArgs);
          FileList2.Load(SmartCopyReadArgs);
          fclose(SmartCopyReadArgs);

          //
          // Then write the content again to a file, and compare the first and second file.
          // If they are equal everything is fine
          //
          SmartCopyArgs = Open(curdir, sla_quoted);
          WriteArgs(SmartCopyArgs, 's', L"not used", L"not used");
          FileList2.Save(SmartCopyArgs);
          fclose(SmartCopyArgs);
  #endif

          // Fork Process
	  DWORD r = uacHelper.Fork();
        }
        else
        {
          // Do not elevate because the operation did not contain symlinks
          // Reset the Progressbar since we now know the number of files to be copied
          HTRACE (L"Progress Start%08x\n", MaxProgress);
          pProgressbar->SetRange(MaxProgress);

          AsyncContext  Context;
      
          // TODO ALLOWCREATION: Hand over AllowCreation Flag to whole operation
          switch (aMode)
          {
            case FileInfoContainer::eSmartCopy:
              FileList.SmartCopy (&aStats, &PathNameStatusList, &Context);
            break;
          
            case FileInfoContainer::eSmartClone:
              FileList.SmartClone (&aStats, &PathNameStatusList, &Context);
            break;
          }

          while (!Context.Wait(250))
          {
            if (pProgressbar->Update(Context, Context.GetProgress()))
            {
              Context.Cancel();
              Context.Wait(INFINITE);
              break;
            }
          }
          delete pProgressbar;

        } // Does not contain symlinks
      } // if Context is still running
      else
      {
        delete pProgressbar;
      }

      GetLocalTime(&aStats.m_EndTime);
      FileList.EndLogging(LogFile, PathNameStatusList, aStats);
    }

  } // ProgressBar

  // TBD do the errorhandling based on PathNameStatusList. 
  if (PathNameStatusList.size())
  {
    // Dump Errors
    _PathNameStatusList::iterator	iter;
    for (iter = PathNameStatusList.begin (); iter != PathNameStatusList.end (); ++iter)
    {
      PathNameStatus p = *iter;
      HTRACE (L"%s, %d\n", p.m_PathName, p.m_ErrorCode);
    }
    /*
    if (ERROR_TOO_MANY_LINKS == r)
    ErrorCreating(FailedPath, 
    IDS_STRING_ErrExplorerCanNotCreateHardlink, 
    IDS_STRING_ErrCreatingHardlink,
    IDS_STRING_ErrHardlinkFailed
    );
    else
    ErrorFromSystem(r);
    */
  } // if (PathNameStatusList.size())

  DeletePathNameStatusList(PathNameStatusList);

  return NOERROR;
}


HRESULT 
HardLinkExt::
DropSmartCopy(
	Target&		              aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi
)
{
  return SmartXXX(
    aTarget,
    lpcmi,
    IDS_STRING_ProgressCreatingSmartCopy,
    eTopMenuSmartCopy,
    IDS_STRING_ErrExplorerCanNotCreateSmartMirror,
    IDS_STRING_ErrCreatingSmartCopy,
    FileInfoContainer::eSmartCopy,
    false
  );
}

int 
ReplaceJunction(
  wchar_t*  aSource,
  wchar_t*  aTarget
)
{
  DWORD RetVal = ERROR_SUCCESS;
  BOOL bRemoveDir = FALSE;
  DWORD JunctionAttributes = INVALID_FILE_ATTRIBUTES;

  // If we are not in BackupMode, then delete the directory, otherwise do this elevated
  // because we have to read existing attributes of aSource.
  if (!(gLSESettings.GetFlags() & eBackupMode))
  {
    JunctionAttributes = GetFileAttributes(aSource);
    SetFileAttributes(aSource, JunctionAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
    bRemoveDir = RemoveDirectory(aSource);

    // return the error code of RemoveDir()
    RetVal = GetLastError();
  }

#if defined UAC_FORCE
  if (UAC_OUTPROC)
#else
  // With Vista & W7, we are not allowed to operate on Junctions in folders like
  // c:\Program Files (x86) without UAC, so in this very case the modification
  // of Junctions has to be relayed to an elevated .exe as done with Symbolic links
  if (FALSE == bRemoveDir)
#endif
  {
#if defined UAC_FORCE
    if (UAC_OUTPROC)
#else
    if (ElevationNeeded() || (gLSESettings.GetFlags() & eBackupMode))
#endif
    {
      // Write the command file, which is read by the elevated process
      UACHelper uacHelper;
      uacHelper.WriteArgs('l', aSource, aTarget);
      RetVal = uacHelper.Fork();
    }
  }
  else
  {
    RetVal = CreateJunction(aSource, aTarget);
    if (INVALID_FILE_ATTRIBUTES != JunctionAttributes)
      SetFileAttributes(aSource, JunctionAttributes);
  }

  return RetVal;
}

HRESULT 
HardLinkExt::
DropReplaceJunction(
	Target&		aTarget
)
{
  PathAddBackslash(aTarget.m_Path);

  if (!m_nTargets)
    ClipboardToSelection(false);

  DWORD RetVal = ERROR_SUCCESS;
  if (m_pTargets[0].m_Flags & (eDir | eJunction | eVolume | eMountPoint))
  {
    WCHAR	SourceNoJunction[HUGE_PATH];

    ReparseCanonicalize(m_pTargets[0].m_Path, SourceNoJunction, HUGE_PATH);
    PathAddBackslash(SourceNoJunction);
    if (StrStrI(aTarget.m_Path, SourceNoJunction))
    {
      ErrorCreating(aTarget.m_Path,
        IDS_STRING_ErrExplorerCanNotCreateJunction,
        IDS_STRING_ErrCreatingJunction,
        IDS_STRING_ErrDestSrcCircular
      );
    }
    else
    {
      HTRACE(L"LSE::DropReplaceJunction: '%s' -> '%s', %ld\n", aTarget.m_Path, m_pTargets[0].m_Path, m_pTargets[0].m_Flags);
      RetVal = ReplaceJunction(aTarget.m_Path, m_pTargets[0].m_Path);
      if (S_OK != RetVal)
        ErrorFromSystem(RetVal);
    }

  }
  // If files were selected aside directories or junction
  // create hardlinks for the files
  // if (m_pTargets[i].m_Flags & eFile)
  // 	CreateHardlink(m_pTargets[i].m_Path, dest);
  return NOERROR;
}

int
ReplaceSymbolicLink(
  wchar_t*  aSource,
  wchar_t*  aTarget,
  bool      aKeepAbsRel
)
{
	DWORD RetVal = ERROR_SUCCESS;
  gLSESettings.ReadLSESettings(false);

  PathRemoveBackslash(aSource);
  DWORD Attribs = GetFileAttributes(aSource);

  // Determine the relation between source and destination. KeepAbsRel means: do not change
  int SymbolicLinkRelation = gLSESettings.GetFlags() & eForceAbsoluteSymbolicLinks ? 0 : SYMLINK_FLAG_RELATIVE;
  if (aKeepAbsRel)
  {
    if (PathIsRelative(aTarget))
      SymbolicLinkRelation = SYMLINK_FLAG_RELATIVE;
    else
      SymbolicLinkRelation = 0;
  }

  // With Windows10/14972 a flag can be specified to allow unelevated symlink creation
  if (SymLinkAllowUnprivilegedCreation(&gVersionInfo))
    SymbolicLinkRelation |= SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    
#if defined UAC_FORCE
  if (UAC_OUTPROC)
#else
  // Check if we have to elevate
  if (
    (ElevationNeeded() && !(SymbolicLinkRelation & SYMLINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)) || 
    (gLSESettings.GetFlags() & eBackupMode)
  )
#endif
  {
    UACHelper uacHelper;

    if (Attribs & FILE_ATTRIBUTE_DIRECTORY)
    {
      // Write the command file, which is read by the elevated process
      if (SYMLINK_FLAG_RELATIVE == SymbolicLinkRelation)
        uacHelper.WriteArgs('e', aSource, aTarget);
      else
        uacHelper.WriteArgs('E', aSource, aTarget);
    }
    else
    {
      PathRemoveBackslash(aSource);
      if (SYMLINK_FLAG_RELATIVE == SymbolicLinkRelation)
        uacHelper.WriteArgs('g', aSource, aTarget);
      else
        uacHelper.WriteArgs('G', aSource, aTarget);
    }

    RetVal = uacHelper.Fork();
  }
  else
  {
    // Used when UAC is switched off thus making it possible to call CreateSymboliclink directly from explorer
    // 
    SetFileAttributes(aSource, Attribs & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
    if (Attribs & FILE_ATTRIBUTE_DIRECTORY)
    {
      RemoveDirectory(aSource);
      RetVal = CreateSymboliclink(aSource,
        aTarget,
        SymbolicLinkRelation | SYMLINK_FLAG_DIRECTORY
      );
    }
    else
    {
      DeleteFile(aSource);
      RetVal = CreateSymboliclink(aSource,
        aTarget,
        SymbolicLinkRelation
      );
    }
    SetFileAttributes(aSource, Attribs);
  }

  return RetVal;
}

HRESULT
HardLinkExt::
DropReplaceSymbolicLink(
	Target&		              aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi
)
{
  PathAddBackslash(aTarget.m_Path);

  if (!m_nTargets)
    ClipboardToSelection(false);

  WCHAR	PurePath[HUGE_PATH];

  ReparseCanonicalize(m_pTargets[0].m_Path, PurePath, HUGE_PATH);
  PathAddBackslash(PurePath);
  if (StrStrI(aTarget.m_Path, PurePath))
  {
    ErrorCreating(aTarget.m_Path,
      IDS_STRING_ErrExplorerCanNotCreateMountpoint,
      IDS_STRING_ErrCreatingMountpoint,
      IDS_STRING_ErrDestSrcCircular
    );
  }
  else
  {
    DWORD RetVal = ReplaceSymbolicLink(aTarget.m_Path, m_pTargets[0].m_Path, false);
    if (S_OK != RetVal)
      ErrorFromSystem(RetVal);
  }
  return NOERROR;
}


int
ReplaceMountPoint(
  wchar_t*  aTarget,
  wchar_t*  aSource
)
{
  DWORD RetVal = ERROR_SUCCESS;
  BOOL bRemoveDir = FALSE;

  DWORD MountPointAttributes = GetFileAttributes(aTarget);

#if defined UAC_FORCE
  if (UAC_OUTPROC)
#else
  // With Windows7 elevation is needed to unmount a drive. Furthermore when in 
  // Backup Mode we also have to elevate
  if (ElevationNeeded() || (gLSESettings.GetFlags() & eBackupMode))
#endif
  {
    UACHelper uacHelper;

    // Write the command file, which is read by the elevated process
    uacHelper.WriteArgs('o', aSource, aTarget);

    RetVal = uacHelper.Fork();
  }
  else
  {
    RetVal = ::DeleteMountPoint(aTarget);
    SetFileAttributes(aSource, MountPointAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM));
    RemoveDirectory(aTarget);
    if (S_OK == RetVal)
    {
      CreateDirectory(aTarget, NULL);
      RetVal = CreateMountPoint(aSource, aTarget);
      SetFileAttributes(aTarget, MountPointAttributes);
    }
  }

  return RetVal;
}


HRESULT
HardLinkExt::
DropReplaceMountPoint(
  Target&		              aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi
)
{
  PathAddBackslash(aTarget.m_Path);

  if (!m_nTargets)
    ClipboardToSelection(false);

  if (m_pTargets[0].m_Flags & eVolume)
  {
    WCHAR	PurePath[HUGE_PATH];

    ReparseCanonicalize(m_pTargets[0].m_Path, PurePath, HUGE_PATH);
    PathAddBackslash(PurePath);
    if (StrStrI(aTarget.m_Path, PurePath))
    {
      ErrorCreating(aTarget.m_Path,
        IDS_STRING_ErrExplorerCanNotCreateMountpoint,
        IDS_STRING_ErrCreatingMountpoint,
        IDS_STRING_ErrDestSrcCircular
      );
    }
    else
    {
      HTRACE(L"LSE::DropReplaceMountPoint: '%s' -> '%s', %ld\n", aTarget.m_Path, m_pTargets[0].m_Path, m_pTargets[0].m_Flags);

      DWORD RetVal;
      RetVal = ReplaceMountPoint(aTarget.m_Path, m_pTargets[0].m_Path);
      if (S_OK != RetVal)
        ErrorFromSystem(RetVal);
    }
  }
  return NOERROR;
}

HRESULT
HardLinkExt::
DropSmartMirror(
	Target&		              aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi
)
{
  // Retrieve the light settings
  gLSESettings.ReadLSESettings(false);

	return SmartMirror(
    aTarget, 
    lpcmi, 
    IDS_STRING_ProgressCreatingSmartMirror,
    eTopMenuSmartCopy,
    IDS_STRING_ErrExplorerCanNotCreateSmartMirror,
    IDS_STRING_ErrCreatingSmartMirror,
    FileInfoContainer::eSmartCopy
  );

  return NOERROR;
}



HRESULT
HardLinkExt::
DropDeloreanCopy(
	Target&		              aTarget,
  LPCMINVOKECOMMANDINFO   lpcmi
)
{
  if (!m_nTargets)
		ClipboardToSelection(false);

  _PathNameStatusList ClonePathNameStatusList;
  _PathNameStatusList MirrorPathNameStatusList;

  // Retrieve the light settings
  gLSESettings.ReadLSESettings(false);

  // ProgressBar
  { 
    Progressbar* pProgressbar = new Progressbar();

    Effort Progress;

    // Data for Mirrorlist
    CopyStatistics  MirrorStats;
    GetLocalTime(&MirrorStats.m_StartTime);

    AsyncContext    MirrorContext;

    // Data for Filelist
    FileInfoContainer CloneList;
    FileInfoContainer	MirrorList;
    FILE* LogFile = MirrorList.StartLogging(gLSESettings, L"DeloreanCopy");
    if (LogFile)
    {
      CloneList.SetOutputFile(LogFile);
      CloneList.SetFlags(FileInfoContainer::eLogVerbose);
      CloneList.SetFlags2(FileInfoContainer::eLogFileOutput);
    }
    else
      CloneList.SetFlags(FileInfoContainer::eLogQuiet);

    if (gLSESettings.GetFlags() & eBackupMode)
      CloneList.SetFlags(FileInfoContainer::eBackupMode);
      
    wchar_t Backup0[HUGE_PATH];
    wchar_t Backup1[HUGE_PATH];

    CopyStatistics	aStats;
    GetLocalTime(&aStats.m_StartTime);

    _ArgvList Backup0PathList;
    _ArgvList MirrorPathList;

    // First search all the files, which are about to be copied
    for (ULONG i = 0; i < m_nTargets; i++)
    {
      _ArgvPath Backup0Path;
      _ArgvPath MirrorPath;

      // Create a filename in 'dest' according to the 
      // e.g 'Hardlink(n) of' syntax
      wchar_t		dp[MAX_PATH];
      wchar_t*	pFilename = DrivePrefix(m_pTargets[i].m_Path, dp);

      wcscpy_s(Backup0, HUGE_PATH, PATH_PARSE_SWITCHOFF);
      wcscpy_s(Backup1, HUGE_PATH, PATH_PARSE_SWITCHOFF);
      CreateTimeStampedFileName(
        aTarget.m_Path,
        &Backup0[PATH_PARSE_SWITCHOFF_SIZE],
        &Backup1[PATH_PARSE_SWITCHOFF_SIZE],
        pFilename,
        &aStats.m_StartTime
      );

      if (m_pTargets[i].m_Flags & (eDir|eJunction|eVolume|eMountPoint|eSymbolicLink))
      {
        WCHAR	DestReparsePointFree[HUGE_PATH];
        WCHAR	SourceReparsePointFree[HUGE_PATH];

        // Check if a recursive junction is about to be created 
        ReparseCanonicalize(m_pTargets[i].m_Path, SourceReparsePointFree, HUGE_PATH);
        PathAddBackslash(SourceReparsePointFree);
        ReparseCanonicalize(aTarget.m_Path, DestReparsePointFree, HUGE_PATH);
        if (StrStrI(DestReparsePointFree, SourceReparsePointFree))
        {
          // Bail out. Someone wanted to create a recursive delorean copy
          ErrorCreating(aTarget.m_Path, 
            IDS_STRING_ErrExplorerCanNotCreateDeloreanCopy,
            IDS_STRING_ErrCreatingDeloreanCopy,
            IDS_STRING_ErrDestSrcCircular
          );
        }
        else // if (m_pTargets[i].m_Flags & (eDir|eJunction|eVolume|eMountPoint|eSymbolicLink))
        {
          // With DeloreanCopy the behaviour depends whether an intial backup
          // already exists or not. If no initial backup exists, a plain smartcopy
          // is created, if a initial backup already exists the clone/mirror mechanism
          // is done
          Backup0Path.ArgvDest = Backup1;
          if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
          {
            // No inital backup exists so clone and thus enum files in Backup0

            // If junctions should be spliced, then already spliced
            // junctions from Backup0 should also be spliced in the clone
            // from Backup0 to Backup1
            if (gLSESettings.GetFlags() & eSpliceReparsePoints)
              CloneList.SetSpliceDirList(NULL);
            
            // But junctions should not be unrolled from Backup0 to Backup1

            // Record all path for enumeration

            int DriveType = DRIVE_UNKNOWN;
            DWORD FileSystemFlags;
            IsFileSystemNtfs(Backup0, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);

            Backup0Path.Argv = Backup0;
            Backup0Path.DriveType = DriveType;
            Backup0PathList.push_back(Backup0Path);
          }
          else // if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
          {
            // In case of copying set the unroll/splice flags, if necessary
            if (gLSESettings.GetFlags() & eUnrollReparsePoints)
              CloneList.SetUnrollDirList(NULL);

            if (gLSESettings.GetFlags() & eSpliceReparsePoints)
              CloneList.SetSpliceDirList(NULL);
              
            // Record all path for enumeration
            int DriveType = DRIVE_UNKNOWN;
            DWORD FileSystemFlags;
            IsFileSystemNtfs(m_pTargets[i].m_Path, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);

            if (!PathIsUNC(m_pTargets[i].m_Path))
              Backup0Path.Argv =  PATH_PARSE_SWITCHOFF;
            Backup0Path.Argv += m_pTargets[i].m_Path;
            Backup0Path.DriveType = DriveType;
            Backup0PathList.push_back(Backup0Path);
          }

          // If we are in DeloreanCopy mode start the enum of files in parallel to 
          // the Clone
          if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
          {
            // Mirror the clone: Source -> Backup
            if (gLSESettings.GetFlags() & eUnrollReparsePoints)
              MirrorList.SetUnrollDirList(NULL);

            if (gLSESettings.GetFlags() & eSpliceReparsePoints)
              MirrorList.SetSpliceDirList(NULL);
              
            if (!(gLSESettings.GetFlags() & eForceAbsoluteSymbolicLinks))
              MirrorList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

            if (gLSESettings.GetFlags() & eBackupMode)
              MirrorList.SetFlags(FileInfoContainer::eBackupMode);
              
            MirrorPath.ArgvDest = Backup1;

            // Record all path for enumeration
            int DriveType = DRIVE_UNKNOWN;
            DWORD FileSystemFlags;
            IsFileSystemNtfs(m_pTargets[i].m_Path, &FileSystemFlags, gLSESettings.GetFlags() & eEnableRemote, &DriveType);

            if (!PathIsUNC(m_pTargets[i].m_Path))
              MirrorPath.Argv =  PATH_PARSE_SWITCHOFF;
            MirrorPath.Argv += m_pTargets[i].m_Path;
            MirrorPath.DriveType = DriveType ;
            MirrorPathList.push_back(MirrorPath);
          }

        }
      } // if (m_pTargets[i].m_Flags & (eDir|eJunction|eVolume|eMountPoint|eSymbolicLink))
      else
      {
        // No files selected directories alongside with DeloreanCopy
      }
    } // for (ULONG i = 0; i < m_nTargets; i++)

    // Check if we are in Backup Mode. If yes we also have to do the FindHardlink elevated, because
    // it might happen, that FindHardlink should run over directories, which the explorer does not
    // have access permissions.
    if (gLSESettings.GetFlags() & eBackupMode)
    {
      // Stop bar
      RECT  ProgressbarPosition;
      ZeroMemory(&ProgressbarPosition, sizeof(RECT));
      int ProgessbarVisible = pProgressbar->GetWindowPos(ProgressbarPosition);

      // Create File
      UACHelper uacHelper;

      if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
        // Delorean Copy
        uacHelper.WriteArgs('v', L"not used", L"not used");
      else
        // SmartCopy
        uacHelper.WriteArgs('s', L"not used", L"not used");

      // persist CloneList
      CloneList.SetAnchorPath(Backup0PathList);
      CloneList.Save(uacHelper.File());
		  
      // Save the position of the progress bar
      if (!ProgessbarVisible)
        ProgressbarPosition.left = -1;
        
      uacHelper.SaveProgressbarPosition(ProgressbarPosition);

      // If we are going for the smartmirror, we have to save this
      if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
      {
        MirrorList.SetAnchorPath(MirrorPathList);
        MirrorList.Save(uacHelper.File());
        fwprintf(uacHelper.File(), L"%s\n", Backup1);
      }
      delete pProgressbar;

      MirrorList.StopLogging(LogFile);

      // Fork Process
      DWORD r = uacHelper.Fork();
    }
    else // if (gLSESettings.GetFlags() & eBackupMode)
    {
      AsyncContext    Context;

      // Find the files in Backup0
      CloneList.FindHardLink (
        Backup0PathList,
        0,
        &aStats,
        &ClonePathNameStatusList,
        &Context
      );

      if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
        MirrorList.FindHardLink (
          MirrorPathList, 
          0, 
          &MirrorStats, 
          &MirrorPathNameStatusList, 
          &MirrorContext
        );


      // First we have to wait for the find on the Clone side
      // 
      while (!Context.Wait(50))
      {
        if (pProgressbar->Update(Context, PDM_PREFLIGHT))
        {
          Context.Cancel();
          Context.Wait(INFINITE);
          break;
        }
      }

      MirrorList.HeadLogging(LogFile);

      // If we are going for the smartmirror, we also have to wait for FindHardlink on MirrorList
      if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
      {
        while (!MirrorContext.Wait(250))
        {
          // Check for cancel
          if (pProgressbar->HasUserCancelled())
          {
            MirrorContext.Cancel();
            MirrorContext.Wait(INFINITE);
            break;
          } // if

        } // while (!MirrorContext.Wait(250))
      } // if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
      pProgressbar->SetMode(PDM_DEFAULT);


      // Only continue if nobody pressed cancel
      if (MirrorContext.IsRunning())
      {
        // The containers have to be prepared here because we need to sort the reparse points early
        Effort MaxProgress;
        if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
        {
          CloneList.Prepare(FileInfoContainer::eSmartClone, &aStats, &MaxProgress);

          Effort CleanEffort;
          CloneList.EstimateEffort(FileInfoContainer::eSmartClean, &CleanEffort);
          MaxProgress += CleanEffort;

          Effort MirrorEffort;
          MirrorList.Prepare(FileInfoContainer::eSmartMirror, &MirrorStats, &MirrorEffort);
          MaxProgress += MirrorEffort;
        }
        else
        {
          // Initial Copy during Delorean process
          CloneList.Prepare(FileInfoContainer::eSmartCopy, &aStats, &MaxProgress);
        }

        if (!(gLSESettings.GetFlags() & eForceAbsoluteSymbolicLinks))
          CloneList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

        // If there is one symbolic link among the list, we have
        // to elevate the whole operation. 
  #if defined UAC_FORCE 
        if (UAC_OUTPROC)
  #else
        int ContainsSymlinks = MirrorList.CheckSymbolicLinks() | CloneList.CheckSymbolicLinks();
        if (ContainsSymlinks)
        {
          // With Windows10/14972 a flag can be specified to allow unelevated symlink creation
          if (SymLinkAllowUnprivilegedCreation(&gVersionInfo))
          {
            // Do not elevate
            ContainsSymlinks = FALSE;
            MirrorList.SetFlags2(FileInfoContainer::eUnprivSymlinkCreate);
            CloneList.SetFlags2(FileInfoContainer::eUnprivSymlinkCreate);
          }
        }
        
        if (
          TRUE == ContainsSymlinks && 
          ElevationNeeded()
        )
  #endif
        {
          // Delegate the operation to LSEUacHelper.exe
          //
          MirrorList.StopLogging(LogFile);
          
          // TODO Das Positionieren des Progressbars funktioniert nicht mehr im LSEUacHelper.exe

          // TODO Das Mirror & Delorean mit Backup Mode geht überhaupt nicht mehr. Es wird zwar das uachelper.args geschrieben
          // aber danach tut sich nix mehr.

          // Stop bar
          RECT  ProgressbarPosition;
          ZeroMemory(&ProgressbarPosition, sizeof(RECT));
          int ProgessbarVisible = pProgressbar->GetWindowPos(ProgressbarPosition);

          // Create File
          UACHelper uacHelper;

          if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
            // Delorean Copy
            uacHelper.WriteArgs('v', L"not used", L"not used");
          else
            // SmartCopy
            uacHelper.WriteArgs('s', L"not used", L"not used");

          // persist CloneList
          CloneList.Save(uacHelper.File());
  			  
          // Save the position of the progress bar
          if (!ProgessbarVisible)
            ProgressbarPosition.left = -1;
            
          uacHelper.SaveProgressbarPosition(ProgressbarPosition);

          // If we are going for the smartmirror, we have to save this
          if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
          {
            MirrorList.Save(uacHelper.File());
            fwprintf(uacHelper.File(), L"%s\n", Backup1);
          }
          delete pProgressbar;

          MirrorList.StopLogging(LogFile);

  #if 0 // DEBUG_DEFINES
          // Use this coding to test the integrity of container persistence

          //
          // Read the just written File into a second FileList2
          //
          FILE* SmartCopyReadArgs = _wfopen (&sla_quoted[1], L"rb");
          FileInfoContainer FileList2(PATH_PARSE_SWITCHOFF_SIZE);
				  wchar_t line[HUGE_PATH];
				  wchar_t* t = fgetws(line, HUGE_PATH, SmartCopyReadArgs);
          FileList2.Load(SmartCopyReadArgs);
          fclose(SmartCopyReadArgs);

          //
          // Then write the content again to a file, and compare the first and second file.
          // If they are equal everything is fine
          //
          SmartCopyArgs = Open(curdir, sla_quoted);
          WriteArgs(SmartCopyArgs, 's', L"not used", L"not used");
          FileList2.Save(SmartCopyArgs);
          fclose(SmartCopyArgs);
  #endif
          // persist Pathnamestatuslist

          if (MirrorContext.IsRunning())
          {
            // Fork Process
            DWORD r = uacHelper.Fork();
          }
        }
        else // if DO ELEVATE
        {
          // No ELevation, we stay in DLL
          Effort CurrentProgress;
          Effort LastProgress;

          pProgressbar->SetRange(MaxProgress);

          AsyncContext  Context;

          // Decide wether we are in the mid of Delorean or just do the initial Copy
          if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
          {
            // In the middle of Delorean
            CloneList.SmartClone(&aStats, &ClonePathNameStatusList, &Context);
          }
          else
          {
            // Initial Copy of Delorean
            CloneList.SmartCopy(&aStats, &ClonePathNameStatusList, &Context);
          }

          // Wait for the SmartCopy/SmartClone to finish
          while (!Context.Wait(250))
          {
            // Progress calculation over multiple operations
            CurrentProgress = Context.GetProgress();
            Progress += CurrentProgress - LastProgress;
            LastProgress = CurrentProgress;

            if (pProgressbar->Update(Context, Progress))
            {
              Context.Cancel();
              Context.Wait(INFINITE);
              break;
            }
          }

          // Sum up the rest of progress, which happened during wait
          Progress += Context.GetProgress() - LastProgress;
          pProgressbar->SetProgress(Progress);

          // if it is not the initial backup start cleaning up
          if (Backup0[PATH_PARSE_SWITCHOFF_SIZE])
          {
            // Only continue if Cancel was not pressed
            if (Context.IsRunning())
            {
              // Iterate over all source path and replace all source path in Clonelist
              for (_ArgvListIterator iter = MirrorPathList.begin(); iter != MirrorPathList.end(); ++iter)
                iter->ArgvDest = iter->Argv;
              CloneList.ChangePath(Backup1, MirrorPathList);

              _SmartMirror(CloneList, MirrorList, MirrorStats, MirrorPathNameStatusList, &MirrorContext, Progress, pProgressbar);
            }
            else
            {
              // Cancel was pressed, so wait for the mirror file scan to finish
              MirrorContext.Cancel();
              MirrorContext.Wait(INFINITE);
            }
          }

          GetLocalTime(&aStats.m_EndTime);
          GetLocalTime(&MirrorStats.m_EndTime);

          delete pProgressbar;

        } // Does not contain symlinks
      } // if Context is still running
      else
      {
        delete pProgressbar;
      }

      DisposeAsync(MirrorList, CloneList, MirrorStats, aStats);

      GetLocalTime(&MirrorStats.m_EndTime);
      MirrorList.EndLogging(LogFile, MirrorPathNameStatusList, MirrorStats, FileInfoContainer::eSmartMirror);

    } // ProgressBar

    // TBD do the errorhandling based on PathNameStatusList. 
    if (ClonePathNameStatusList.size())
    {
      // Dump Errors
      _PathNameStatusList::iterator	iter;
      for (iter = ClonePathNameStatusList.begin(); iter != ClonePathNameStatusList.end(); ++iter)
      {
        PathNameStatus pns = *iter;
        if (pns.m_ErrorCode == ERROR_TOO_MANY_LINKS)
        {
          ErrorCreating(pns.m_PathName,
            IDS_STRING_ErrExplorerCanNotCreateHardlink,
            IDS_STRING_ErrCreatingHardlink,
            IDS_STRING_ErrHardlinkFailed
          );
          break;
        }
      }
    } // if (PathNameStatusList.size())

    DeletePathNameStatusList(ClonePathNameStatusList);
  }

  return NOERROR;
}

void
HardLinkExt::
ErrorCreating(
	const wchar_t*	aDirectory,
	int			aMessage,
	int			aCaption,
	int			aReason
)
{
  // Assemble message string
  wchar_t *pMsgTxt = GetFormattedMlgMessage(g_hInstance, gLSESettings.GetLanguageID(), aMessage, aDirectory);

  wchar_t	MsgCaption[MAX_PATH];
  LoadStringEx(g_hInstance, aCaption, MsgCaption, MAX_PATH, gLSESettings.GetLanguageID());

  wchar_t	MsgReason[MAX_PATH];
  if (aReason)
    LoadStringEx(g_hInstance, aReason, MsgReason, MAX_PATH, gLSESettings.GetLanguageID());
  else
    MsgReason[0] = 0x00;

  // Concat Message + Reason
  wchar_t msg[HUGE_PATH];
  wcscpy_s(msg, HUGE_PATH, pMsgTxt);
  wcscat_s(msg, HUGE_PATH, MsgReason);

  int mr = MessageBox(NULL,
    msg,
    MsgCaption,
    MB_ICONERROR);
  LocalFree(pMsgTxt);
}

void
ErrorFromSystem(
	DWORD	aErrorCode
)
{
  if (aErrorCode > ERROR_SUCCESS)
  {
    wchar_t	FullMsg[MAX_PATH];

    // Try to use the built in error system to generate a readable message
    LPTSTR SysErrMsg;
    DWORD rFM = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, aErrorCode, 0, (LPTSTR)&SysErrMsg, 0, NULL
    );
    if (rFM == 0)
    {
      // it failed, because e.g the error code was not supported
      wsprintf(FullMsg, L"Link Shell Extension: Operation failed with System Error Code %d", aErrorCode);
      int mr = MessageBox(NULL,
        FullMsg,
        L"Link Shell Extension",
        MB_ICONERROR);
    }
    else
    {
      // FormatMessage worked and produced a proper string
      wsprintf(FullMsg, L"Link Shell Extension: (%d) %s", aErrorCode, SysErrMsg);
      int mr = MessageBox(NULL,
        FullMsg,
        L"Link Shell Extension",
        MB_ICONERROR);
    }
  }
}


wchar_t*
HardLinkExt::
DrivePrefix(
	const wchar_t*	aSource,
	wchar_t*	aDrivePrefix
)
{
  wchar_t*	pFilename;

  if (PathIsRoot(aSource) && !PathIsUNC(aSource)) // PathIsNetworkPath()
  {
    wchar_t	lpVolumeNameBuffer[MAX_PATH] = { 0x00 };
    DWORD lpVolumeSerialNumber;
    DWORD lpMaximumComponentLength;
    DWORD lpFileSystemFlags;
    wchar_t	lpFileSystemNameBuffer[MAX_PATH];

    GetVolumeInformation(
      aSource,
      lpVolumeNameBuffer,
      MAX_PATH,
      &lpVolumeSerialNumber,
      &lpMaximumComponentLength,
      &lpFileSystemFlags,
      lpFileSystemNameBuffer,
      MAX_PATH
    );

    if (lpVolumeNameBuffer[0] == '\0')
      LoadStringEx(g_hInstance, IDS_STRING_DrivePrefix, lpVolumeNameBuffer, MAX_PATH, gLSESettings.GetLanguageID());

    wcscpy(aDrivePrefix, lpVolumeNameBuffer);
    wcscat(aDrivePrefix, L" (_");
    size_t l = wcslen(aDrivePrefix);
    aDrivePrefix[l - 1] = aSource[0];
    wcscat(aDrivePrefix, L")");

    pFilename = aDrivePrefix;
  }
  else
    pFilename = PathFindFileName(aSource);

  return pFilename;
}

