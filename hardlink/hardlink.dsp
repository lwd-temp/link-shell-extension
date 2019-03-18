# Microsoft Developer Studio Project File - Name="hardlink" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=hardlink - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hardlink.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hardlink.mak" CFG="hardlink - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hardlink - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "hardlink - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hardlink - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Lib\vc6\Release"
# PROP Intermediate_Dir "obj\vc6\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D _WIN32_WINNT=0x0502 /Yu"stdafx.h" /Fd"..\Lib\vc6\Release\hardlink.pdb" /FD /c
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "hardlink - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Lib\vc6\Debug"
# PROP Intermediate_Dir "obj\vc6\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_UNICODE" /D _WIN32_WINNT=0x0502 /FR /Yu"stdafx.h" /Fd"..\Lib\vc6\Debug\hardlink.pdb" /FD /GZ /c
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "hardlink - Win32 Release"
# Name "hardlink - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\AsyncContext.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DupeContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DupeStatus.cpp
# End Source File
# Begin Source File

SOURCE=.\src\hardlinks.cpp
# End Source File
# Begin Source File

SOURCE=.\src\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmfObject.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Multilang.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Progressbar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\AsyncContext.h
# End Source File
# Begin Source File

SOURCE=.\src\DupeContainer.h
# End Source File
# Begin Source File

SOURCE=.\src\DupeStatus.h
# End Source File
# Begin Source File

SOURCE=.\include\hardlink.h
# End Source File
# Begin Source File

SOURCE=.\src\hardlinks.h
# End Source File
# Begin Source File

SOURCE=.\src\md5.h
# End Source File
# Begin Source File

SOURCE=.\src\MmfObject.h
# End Source File
# Begin Source File

SOURCE=.\include\Multilang.h
# End Source File
# Begin Source File

SOURCE=.\include\Progressbar.h
# End Source File
# Begin Source File

SOURCE=.\src\Reparse.h
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.h
# End Source File
# End Group
# End Target
# End Project
