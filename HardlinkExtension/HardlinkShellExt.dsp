# Microsoft Developer Studio Project File - Name="HardlinkShellExt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=HardlinkShellExt - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HardlinkShellExt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HardlinkShellExt.mak" CFG="HardlinkShellExt - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HardlinkShellExt - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "HardlinkShellExt - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HardlinkShellExt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Bin\vc6\Release"
# PROP Intermediate_Dir "Obj\vc6\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\hardlink\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_UNICODE" /D _WIN32_WINNT=0x0502 /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 hardlink.lib shlwapi.lib user32.lib shell32.lib Oleaut32.lib comctl32.lib ole32.lib uxtheme.lib uxtheme.lib Delayimp.lib mpr.lib gdi32.lib /nologo /stack:0x400000 /subsystem:windows /dll /debug /machine:I386 /libpath:"..\lib\vc6\Release" /OPT:NoWin98 /DELAYLOAD:"uxtheme.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "HardlinkShellExt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Bin\vc6\Debug"
# PROP Intermediate_Dir "Obj\vc6\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\hardlink\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_UNICODE" /D _WIN32_WINNT=0x0502 /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 hardlink.lib shlwapi.lib user32.lib shell32.lib Oleaut32.lib comctl32.lib ole32.lib uxtheme.lib Delayimp.lib mpr.lib gdi32.lib /nologo /stack:0x1000000,0x1000000 /subsystem:windows /dll /debug /machine:I386 /libpath:"..\lib\vc6\Debug" /OPT:NoWin98 /DELAYLOAD:"uxtheme.dll"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "HardlinkShellExt - Win32 Release"
# Name "HardlinkShellExt - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\ColumnProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\CopyHook.cpp
# End Source File
# Begin Source File

SOURCE=.\HardLink.cpp
# End Source File
# Begin Source File

SOURCE=.\HardLink.def
# End Source File
# Begin Source File

SOURCE=.\HardLinkMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\IconOverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertySheetPage.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\ColumnProvider.h
# End Source File
# Begin Source File

SOURCE=.\CopyHook.h
# End Source File
# Begin Source File

SOURCE=.\HardLink.h
# End Source File
# Begin Source File

SOURCE=.\HardLinkMenu.h
# End Source File
# Begin Source File

SOURCE=.\IconOverlay.h
# End Source File
# Begin Source File

SOURCE=.\localisation.h
# End Source File
# Begin Source File

SOURCE=.\Multilang.h
# End Source File
# Begin Source File

SOURCE=.\PropertySheetPage.h
# End Source File
# Begin Source File

SOURCE=.\Reparse.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\Hardlink.ico
# End Source File
# Begin Source File

SOURCE=.\HardLink.rc
# End Source File
# Begin Source File

SOURCE=.\res\HardLink.rc2
# End Source File
# Begin Source File

SOURCE=.\res\Hardlink_XP.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\JunctionIcon.ico
# End Source File
# Begin Source File

SOURCE=.\res\JunctionIcon_XP.ico
# End Source File
# End Group
# Begin Group "Html"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\install\CreateInstall.bat
# End Source File
# Begin Source File

SOURCE=.\Doc\hardlinkshellext.html
# End Source File
# Begin Source File

SOURCE=.\install\HardlinkShellExt_vc6.nsi
# End Source File
# Begin Source File

SOURCE=.\install\HardlinkShellExt_x64.nsi
# End Source File
# End Group
# Begin Source File

SOURCE=.\Todo.txt
# End Source File
# End Target
# End Project
