/*
	Copyright (C) 1999_2006, Hermann Schinagl, Hermann.Schinagl@gmx.net
*/

#include "stdafx.h"
#include "resource.h"
#include "Progressbar.h"
#include "CopyHook.h"
#include "Utils.h"
#include "HardlinkUtils.h"



extern UINT      g_cRefThisDll;    // Reference count of this DLL.

extern HINSTANCE g_hInstance;
extern _LSESettings gLSESettings;


///////////////////////////////////////////////////////////////
// CopyHookClassFactory

CopyHookClassFactory::CopyHookClassFactory() : m_cRef(0L)
{
    g_cRefThisDll++;
}

CopyHookClassFactory::~CopyHookClassFactory()
{
    g_cRefThisDll--;
}

STDMETHODIMP CopyHookClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)
{
    *ppv = NULL;
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CopyHookClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CopyHookClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP
CopyHookClassFactory::
CreateInstance(
	LPUNKNOWN pUnkOuter,
	REFIID riid,
	LPVOID *ppvObj
)
{
  *ppvObj = NULL;

  // it calls me with 00000000-0000-0000-C000-000000000046
  // Shell extensions typically don't support aggregation (inheritance)
  if (pUnkOuter)
    return CLASS_E_NOAGGREGATION;

  CopyHook* pCopyHook = new CopyHook();
  return pCopyHook->QueryInterface(riid, ppvObj);
}

STDMETHODIMP CopyHookClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

//
// ICopyHook Methods
//
CopyHook::CopyHook(): m_cRef(0L)
{
	g_cRefThisDll++;
}

CopyHook::~CopyHook()
{
	g_cRefThisDll--;
}

STDMETHODIMP CopyHook::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	*ppv = NULL;

    if (IsEqualIID(riid, IID_IShellCopyHook) || IsEqualIID(riid, IID_IUnknown))
	{
		*ppv = (ICopyHook*)this;
		AddRef();
		return NOERROR;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CopyHook::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CopyHook::Release()
{
	if (--m_cRef)
		return m_cRef;

	delete this;
	return 0L;
}


STDMETHODIMP_(UINT)
CopyHook::
CopyCallback ( HWND hwnd, 
		UINT wFunc, 
		UINT wFlags, 
		LPCWSTR pszSrcFile, 
		DWORD dwSrcAttribs,
		LPCWSTR pszDestFile, 
		DWORD dwDestAttribs
)
{
	HRESULT r = IDYES;
	
  HTRACE(L"LSE::CopyCallback CopyHook called\n");
	HTRACE(L"LSE::CopyCallback wFunc %08x\n", wFunc);
	HTRACE(L"LSE::CopyCallback wFlags %08x\n", wFlags);
	HTRACE(L"LSE::CopyCallback pszSrcFile '%s'\n", pszSrcFile);
	HTRACE(L"LSE::CopyCallback dwSrcAttribs %08x\n", dwSrcAttribs);
	HTRACE(L"LSE::CopyCallback pszDestFile '%s'\n", pszDestFile);
	HTRACE(L"LSE::CopyCallback dwDestAttribs %08x\n", dwDestAttribs);

	// Check if we are dealing with a junction
	if (dwSrcAttribs & 0x0400)
	{
		// Check for 'shift delete'
		if (wFunc == FO_DELETE && !(wFlags & FOF_ALLOWUNDO))
		{
      if (gVersionInfo.dwMajorVersion != 6)
      {
			  BOOL rrr = RemoveDirectory(pszSrcFile);
			  HTRACE(L"LSE::CopyCallback RemoveDirectory %08x, lasterr %08x\n", rrr, GetLastError());
			  r = IDNO;
      }
		}
		else
		{
			// Wenn man nur delete drueckt, und nicht shift del, dann
			// hat xp ab einer gewissen groesse die Eigenheit, dass es
			// junctions, die in den Recycle bin geschoben werden sollen, nicht
			// abhaengt, sondern auch die Quelle entleert. Wenn das aber nur
			// ein paar Byte sind, dann wird eine Junction sehr wohl abgehaengt
			// und in den recycle Bin geschoben.
			// Das limit liegt irgendwo bei 2.2 GB
			HTRACE(L"LSE::CopyCallback 03\n");
		}
	}
	else
	{
		HTRACE(L"LSE::CopyCallback 04\n");

    if (( wFunc == FO_RENAME || wFunc == FO_MOVE))
    {
      GetLSESettings(gLSESettings, false);

      if (!(gLSESettings.Flags & eDisableSmartmove))
      {
        DWORD DestAttr = GetFileAttributes(pszDestFile);
        HTRACE(L"LSE::CopyCallback pszSrcFile '%s' %08x\n", pszSrcFile, GetFileAttributes(pszSrcFile));
	      HTRACE(L"LSE::CopyCallback pszDestFile '%s' %08x\n", pszDestFile, DestAttr);

        // Check if just the case of a letter changed, then we don't have to do anything
        if (!_wcsicmp(pszSrcFile, pszDestFile))
          return IDYES;

        // Check if destination is already there
        if (INVALID_FILE_ATTRIBUTES != DestAttr)
          return IDYES;

        _PathNameStatusList PathNameStatusList;
        CopyStatistics	aStats;

        // ProgressBar
        { 
          CoInitialize(NULL);
          
          const int MaxEndlessProgress = 50;
          Progressbar* pProgressbar = new Progressbar(
            IDS_STRING_ProgressSmartMove, 
            IDS_STRING_ProgressCanceling,
            g_hInstance, 
            gLSESettings.LanguageID,
            hwnd,
            MaxEndlessProgress
            );

          int progress = 0;

    	    FileInfoContainer	FileList;

          _ArgvList MoveLocation;
          _ArgvPath MovePath;
          if (!PathIsUNC(pszSrcFile))
            MovePath.Argv = PATH_PARSE_SWITCHOFF;
          MovePath.Argv += pszSrcFile;

          if (!PathIsUNC(pszDestFile))
            MovePath.ArgvDest = PATH_PARSE_SWITCHOFF;
          MovePath.ArgvDest += pszDestFile;
          MoveLocation.push_back(MovePath);

          // Check if we are in Backup Mode. If yes we also have to do the FindHardlink elevated, because
          // it might happen, that FindHardlink should run over directories, which the explorer does not
          // have access permissions.
          if (gLSESettings.Flags & eBackupMode)
          {
            // Stop bar
            RECT  ProgressbarPosition;
            ZeroMemory(&ProgressbarPosition, sizeof(RECT));
            int ProgessbarVisible = pProgressbar->GetWindowPos(ProgressbarPosition);
            delete pProgressbar;

            // Symbolic links found ==> we have to handover to symlink.exe
            wchar_t sla_quoted[HUGE_PATH];
            wchar_t curdir[HUGE_PATH];	
            FILE* SmartMoveArgs = OpenFileForExeHelper(curdir, sla_quoted);
	          fwprintf(SmartMoveArgs, L"-r \"%s\" \"%s\"\n", L"not used", L"not used");

            FileList.SetFlags(FileInfoContainer::eBackupMode);

            // persist FileList
            FileList.SetAnchorPath(MoveLocation);
            FileList.Save(SmartMoveArgs);

            // Save the position of the progress bar
            if (!ProgessbarVisible)
              ProgressbarPosition.left = -1;
              
            fwprintf(SmartMoveArgs, L"%x,%x,%x,%x\n", ProgressbarPosition.left, ProgressbarPosition.top, ProgressbarPosition.right, ProgressbarPosition.bottom);
            fclose(SmartMoveArgs);

            // Fork Process
            DWORD r = ForkExeHelper(curdir, sla_quoted);
          }
          else
          {
            AsyncContext    Context;
            wchar_t         CurrentPath[HUGE_PATH];

            CopyStatistics	aStats;
            
            int   RefCount;
            if (gVersionInfo.dwMajorVersion >= 6)
              // With Vista & W7 we also have to search files, because symbolic links are 
              // also reparsepoints
              RefCount = 0;
            else
              // With XP, we know there are now symbolic links, so we can speed up search by
              // only collecting directories/junctions
              RefCount = -1;

            FILE* LogFile = FileList.StartLogging(gLSESettings, L"SmartMove");
            int r = FileList.FindHardLink (MoveLocation, RefCount, &aStats, &PathNameStatusList, &Context);
            
            if (!(gLSESettings.Flags & eForceAbsoluteSymbolicLinks))
              FileList.SetFlags(FileInfoContainer::eRelativeSymboliclinks);

            while (!Context.Wait(250))
            {
              Context.GetStatus(CurrentPath);
              pProgressbar->SetProgress (progress++);
              pProgressbar->SetCurrentPath(CurrentPath);
              pProgressbar->Show();
              if (pProgressbar->HasUserCancelled())
              {
                Context.Cancel();
                Context.Wait(INFINITE);
                progress = -1;
                break;
              }
              
              // This is an endless bar during searching for files
              if (progress >= MaxEndlessProgress)
              {
                pProgressbar->SetRange(MaxEndlessProgress);
                progress = 0;
              }
            } // while (!Context.Wait(250))


            if (progress >= 0)
            {
              // See if there are Symbolic Links
#if defined SYMLINK_INPROC
              if (0)
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
              
              // Do we have to elevate?
              if (
                TRUE == ContainsSymlinks && 
                !gbXpSymlinks && 
                ElevationNeeded()
              )
#endif
              {
                // Stop bar
                RECT  ProgressbarPosition;
                ZeroMemory(&ProgressbarPosition, sizeof(RECT));
                int ProgessbarVisible = pProgressbar->GetWindowPos(ProgressbarPosition);
                delete pProgressbar;

                // Symbolic links found ==> we have to handover to symlink.exe
                wchar_t sla_quoted[HUGE_PATH];
                wchar_t curdir[HUGE_PATH];	
                FILE* SmartMoveArgs = OpenFileForExeHelper(curdir, sla_quoted);
		            fwprintf(SmartMoveArgs, L"-r \"%s\" \"%s\"\n", L"not used", L"not used");

                // persist FileList
                FileList.Save(SmartMoveArgs);

                // Save the position of the progress bar
                if (!ProgessbarVisible)
                  ProgressbarPosition.left = -1;
                  
                fwprintf(SmartMoveArgs, L"%x,%x,%x,%x\n", ProgressbarPosition.left, ProgressbarPosition.top, ProgressbarPosition.right, ProgressbarPosition.bottom);
                fclose(SmartMoveArgs);
                FileList.StopLogging(LogFile);

                // Fork Process
                DWORD r = ForkExeHelper(curdir, sla_quoted);
              }
              else
              {
                FileList.HeadLogging(LogFile);

                // Reset the Progressbar since we now know the number of files to be renamed
                FileList.PrepareSmartCopy(FileInfoContainer::eSmartClone, &aStats);  // Call it for SmartClone, it is trashed anyway
                __int64 MaxProgress = FileList.PrepareSmartMove();
                HTRACE (L"Progress Start%08x\n", MaxProgress);
                pProgressbar->SetRange(MaxProgress);

                // Walk through all the files
                AsyncContext  Context;
                FileList.SmartMove (&aStats, 
                  &PathNameStatusList, 
                  &Context
                );

                while (!Context.Wait(250))
                {
                  Context.GetStatus(CurrentPath);
                  pProgressbar->SetProgress (Context.Get());
                  pProgressbar->SetCurrentPath(CurrentPath);
                  pProgressbar->Show();
                  if (pProgressbar->HasUserCancelled())
                  {
                    Context.Cancel();
                    Context.Wait(INFINITE);
                    break;
                  } // if
                } // while
                delete pProgressbar;
              }
            }
            else
            {
              delete pProgressbar;
            }

            GetLocalTime(&aStats.m_EndTime);
            FileList.EndLogging(LogFile, PathNameStatusList, aStats);
          }
        
          CoUninitialize();
        
          DeletePathNameStatusList(PathNameStatusList);
        } // end of Progress bar
      } // if (!(gLSESettings.Flags & eDisableSmartmove))
    } // if (( wFunc == FO_RENAME || wFunc == FO_MOVE)) 
	}

	return r;
}
