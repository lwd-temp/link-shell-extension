set ARCHIVENAME=HardlinkShellExtSource3650
set SOURCEDIR=hardlinks
pushd ..\..

REM
REM Hardlinkshellext source
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Hardlink.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\HardlinkShellExt.dsp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\HardlinkShellExt.vcproj
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\HardLink.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\HardLink.rc
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\HardLinkMenu.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\HardLinkMenu.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\CopyHook.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\CopyHook.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\ColumnProvider.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\ColumnProvider.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\IconOverlay.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\IconOverlay.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\localisation.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\resource.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\StdAfx.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\StdAfx.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Utils.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Utils.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\HardLink.def
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\res\HardLink.rc2
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\PropertySheetPage.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\PropertySheetPage.h
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\restart.bat
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\settings.bat
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\res\Hardlink.ico
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\res\Hardlink_xp.ico
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\res\Hardlink_vista.ico
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\res\JunctionIcon.ico

REM
REM hardlink.lib source
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\hardlink\src\*.*
zip -u %ARCHIVENAME% %SOURCEDIR%\hardlink\hardlink.vcproj
zip -u %ARCHIVENAME% %SOURCEDIR%\hardlink\hardlink.dsp
zip -u %ARCHIVENAME% %SOURCEDIR%\hardlink\include\*.*

REM
REM Workspace
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\Link.sln
zip -u %ARCHIVENAME% %SOURCEDIR%\Link.dsw


REM
REM symlink source
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\stdafx.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\stdafx.h
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\symlink.cpp
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\symlink.hp
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\symlink.rc
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\symlink.vcproj
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\symlink.h
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\resource.h
zip -u %ARCHIVENAME% %SOURCEDIR%\symlink\symlink.exe.trust.manifest

REM
REM LSEConfig source
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\LSEConfig\*.*
zip -u %ARCHIVENAME% %SOURCEDIR%\LSEConfig\res\*.*

REM
REM Install stuff
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\HardlinkShellExt_vc6.nsi
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\HardlinkShellExt_win32.nsi
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\HardlinkShellExt_x64.nsi
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\HardlinkShellExt_Itanium.nsi
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\HardlinkShellExt.nsi
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\GetTime.nsh
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\GetWindowsVersion.nsh
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\Stubs.64\*
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\Stubs.Org\*
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\CreateInstall.bat
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\CodeSign.bat
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\install\CreateInstall.bat

REM
REM Certificate
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\KnowledgeBase\certificate\GenerateCertificate.bat
zip -u %ARCHIVENAME% %SOURCEDIR%\KnowledgeBase\certificate\LinkShellextension*.*

REM
REM Source Archiving
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\PackSource.bat

REM
REM Documentation
REM
zip -u %ARCHIVENAME% "%SOURCEDIR%\HardlinkExtension\hardlink bhs.txt"
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\License.txt
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\ReadMe.txt
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Doc\*.*

REM
REM Todo
REM
zip -u %ARCHIVENAME% "%SOURCEDIR%\Hardlinks Open Issue List.xls"

REM
REM Binaries for Crash Dump analysis
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Bin\win32\release\HardlinkShellExt.dll
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Bin\win32\release\HardlinkShellExt.pdb

zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Bin\x64\release\HardlinkShellExt.dll
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Bin\x64\release\HardlinkShellExt.pdb

zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Bin\itanium\release\HardlinkShellExt.dll
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Bin\itanium\release\HardlinkShellExt.pdb

zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Bin\vc6\release\HardlinkShellExt.dll
zip -u %ARCHIVENAME% %SOURCEDIR%\HardlinkExtension\Bin\vc6\release\HardlinkShellExt.pdb


zip -u %ARCHIVENAME% %SOURCEDIR%\Lib\win32\release\hardlink.pdb

zip -u %ARCHIVENAME% %SOURCEDIR%\Lib\x64\release\hardlink.pdb

zip -u %ARCHIVENAME% %SOURCEDIR%\Lib\itanium\release\hardlink.pdb

zip -u %ARCHIVENAME% %SOURCEDIR%\Lib\vc6\release\hardlink.pdb


zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\win32\release\symlink.exe
zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\win32\release\symlink.pdb

zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\x64\release\symlink.exe
zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\x64\release\symlink.pdb

zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\itanium\release\symlink.exe
zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\itanium\release\symlink.pdb

REM
REM LSEConfig
REM
zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\itanium\release\LSEConfig.pdb
zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\itanium\release\LSEConfig.exe

zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\win32\release\LSEConfig.pdb
zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\win32\release\LSEConfig.exe

zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\x64\release\LSEConfig.pdb
zip -u %ARCHIVENAME% %SOURCEDIR%\Bin\x64\release\LSEConfig.exe


del %SOURCEDIR%\archive\%ARCHIVENAME%.zip
move %ARCHIVENAME%.zip %SOURCEDIR%\HardlinkExtension\archive
popd


REM
REM wait 10 seconds
REM
@ping 127.0.0.1 -n 20 -w 1000 > nul

