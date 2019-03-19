@echo off
set MEDIA=Media

REM Set the version info 
REM
set MAJOR_VERSION=3
set MINOR_VERSION=9
set PATCH_VERSION=1
set HOTFIX_VERSION=3

@echo generating Version info
REM Generate version info for c++
REM
set VERSION_FILE=Shared\version\version.h
(echo #define MAJOR_VERSION %MAJOR_VERSION%)>%VERSION_FILE%
(echo #define MINOR_VERSION %MINOR_VERSION%)>>%VERSION_FILE%
(echo #define PATCH_VERSION %PATCH_VERSION%)>>%VERSION_FILE%
(echo #define HOTFIX_VERSION %HOTFIX_VERSION%)>>%VERSION_FILE%
(echo #define LSE_CURRENT_VERSION %MAJOR_VERSION%%MINOR_VERSION%%PATCH_VERSION%%HOTFIX_VERSION%)>>%VERSION_FILE%

REM Generate version info for install media stamp
REM
set VERSION_FILE=HardlinkExtension\install\stampver.inf
(echo FileVersion=%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%HOTFIX_VERSION%)>%VERSION_FILE%
(echo ProductVersion=%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%HOTFIX_VERSION%)>>%VERSION_FILE%
(echo FileFormat=%%a.%%b.%%c.%%d)>>%VERSION_FILE%
(echo ProductFormat=%%a.%%b.%%c.%%d)>>%VERSION_FILE%

REM Generate version info for installer script
REM
set VERSION_FILE=HardlinkExtension\install\LSE_version.nsh
(echo !define LSE_VERSION "%MAJOR_VERSION%.%MINOR_VERSION%.%PATCH_VERSION%.%HOTFIX_VERSION%")>%VERSION_FILE%

REM Rebuild
REM
@echo Rebuild lnk.sln and press enter afertwards
pause

REM Create installer
REM
@echo Generating Install media
pushd HardlinkExtension\install
call CodeSign.bat
popd

REM Pack ln.exe
REM
pushd ln
call PackMedia.bat %MAJOR_VERSION%%MINOR_VERSION%%PATCH_VERSION%%HOTFIX_VERSION%
popd

REM dupemerge.exe
REM
pushd dupemerge
call PackMedia.bat %MAJOR_VERSION%%MINOR_VERSION%%PATCH_VERSION%%HOTFIX_VERSION%
popd


