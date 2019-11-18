@echo off
set MEDIA=Media

REM Set the version info for LinkShellExension
REM
set MAJOR_LSE_VERSION=3
set MINOR_LSE_VERSION=9
set PATCH_LSE_VERSION=3
set HOTFIX_LSE_VERSION=0

REM Set the version info for ln.exe
REM
set MAJOR_LN_VERSION=2
set MINOR_LN_VERSION=9
set PATCH_LN_VERSION=2
set HOTFIX_LN_VERSION=4

REM Set the version info for dupemerge.exe
REM
set MAJOR_DUPEMERGE_VERSION=1
set MINOR_DUPEMERGE_VERSION=1
set PATCH_DUPEMERGE_VERSION=0
set HOTFIX_DUPEMERGE_VERSION=2

@echo generating Version info
REM Generate version info for Shell Extension
REM
set VERSION_FILE=HardlinkExtension\lse_version.h
(echo #define MAJOR_VERSION %MAJOR_LSE_VERSION%)>%VERSION_FILE%
(echo #define MINOR_VERSION %MINOR_LSE_VERSION%)>>%VERSION_FILE%
(echo #define PATCH_VERSION %PATCH_LSE_VERSION%)>>%VERSION_FILE%
(echo #define HOTFIX_VERSION %HOTFIX_LSE_VERSION%)>>%VERSION_FILE%


REM Generate version info for ln.exe
REM
set VERSION_FILE=ln\ln_version.h
(echo #define MAJOR_VERSION %MAJOR_LN_VERSION%)>%VERSION_FILE%
(echo #define MINOR_VERSION %MINOR_LN_VERSION%)>>%VERSION_FILE%
(echo #define PATCH_VERSION %PATCH_LN_VERSION%)>>%VERSION_FILE%
(echo #define HOTFIX_VERSION %HOTFIX_LN_VERSION%)>>%VERSION_FILE%

REM Generate version info for dupemerge.exe
REM
set VERSION_FILE=dupemerge\dupemerge_version.h
(echo #define MAJOR_VERSION %MAJOR_DUPEMERGE_VERSION%)>%VERSION_FILE%
(echo #define MINOR_VERSION %MINOR_DUPEMERGE_VERSION%)>>%VERSION_FILE%
(echo #define PATCH_VERSION %PATCH_DUPEMERGE_VERSION%)>>%VERSION_FILE%
(echo #define HOTFIX_VERSION %HOTFIX_DUPEMERGE_VERSION%)>>%VERSION_FILE%

REM LSE contains a schema in the registry. If from version to version this schema is heavily
REM changed, then we need to increment the schema version number.
REM For normal code changes it is not neccessary to increase this scheme, but keep it
REM Changing the schema means, that the default values for various settings are copied over during initial start of LSE
REM which is cumbersome for the users. So the goal is not to change this schema
REM
REM Uncomment the below line if the schema should be changed
set VERSION_REG_SCHEMA=Shared\version\RegSchemaVersion.h
REM (echo #define LSE_CURRENT_VERSION %MAJOR_VERSION%%MINOR_VERSION%%PATCH_VERSION%%HOTFIX_VERSION%)>>%VERSION_REG_SCHEMA%

REM Generate version info for install media stamp
REM
set VERSION_FILE=HardlinkExtension\install\stampver.inf
(echo FileVersion=%MAJOR_LSE_VERSION%.%MINOR_LSE_VERSION%.%PATCH_LSE_VERSION%.%HOTFIX_LSE_VERSION%)>%VERSION_FILE%
(echo ProductVersion=%MAJOR_LSE_VERSION%.%MINOR_LSE_VERSION%.%PATCH_LSE_VERSION%.%HOTFIX_LSE_VERSION%)>>%VERSION_FILE%
(echo FileFormat=%%a.%%b.%%c.%%d)>>%VERSION_FILE%
(echo ProductFormat=%%a.%%b.%%c.%%d)>>%VERSION_FILE%

REM Generate version info for installer script
REM
set VERSION_FILE=HardlinkExtension\install\LSE_version.nsh
(echo !define LSE_VERSION "%MAJOR_LSE_VERSION%.%MINOR_LSE_VERSION%.%PATCH_LSE_VERSION%.%HOTFIX_LSE_VERSION%")>%VERSION_FILE%

REM Generate version info for LinkShellExtension.nuspec 
REM
set VERSION_FILE=HardlinkExtension\install\choco\LSE_version.txt
(echo %MAJOR_LSE_VERSION%.%MINOR_LSE_VERSION%.%PATCH_LSE_VERSION%.%HOTFIX_LSE_VERSION%)>%VERSION_FILE%

REM Generate version info for ln.nuspec 
REM
set VERSION_FILE=ln\choco\ln_version.txt
(echo %MAJOR_LN_VERSION%.%MINOR_LN_VERSION%.%PATCH_LN_VERSION%.%HOTFIX_LN_VERSION%)>%VERSION_FILE%

REM Generate version info for dupemerge.nuspec 
REM
set VERSION_FILE=dupemerge\choco\dupemerge_version.txt
(echo %MAJOR_DUPEMERGE_VERSION%.%MINOR_DUPEMERGE_VERSION%.%PATCH_DUPEMERGE_VERSION%.%HOTFIX_DUPEMERGE_VERSION%)>%VERSION_FILE%

REM Rebuild
REM
if not exist Media mkdir Media
@echo.
@echo Rebuild link.sln and press enter afertwards
@echo.
pause

@echo.
@echo Please commit to GIT now for the source
@echo.
pause

REM Source Indexing
REM
echo.
echo Provide Tools for Source indexing
echo.
@echo F|@xcopy /y ..\hardlinks.supl\KnowledgeBase\SourceIndex\git_source_index.exe tools\git_source_index.exe > nul
@echo F|@xcopy /y ..\hardlinks.supl\KnowledgeBase\SourceIndex\git_source_index_fetch.exe tools\git_source_index_fetch.exe > nul

REM Source index pdbs
REM
call bat\SourceIndex.bat
@del tools\git_source_index.exe
@del tools\git_source_index_fetch.exe

REM Upload to symbolserver
REM
call bat\SymServUpload.bat %MAJOR_VERSION%%MINOR_VERSION%%PATCH_VERSION%%HOTFIX_VERSION%
@REM 
@echo.
@echo Please commit to GIT now for he symbol transaction-id
@echo The symbolserver - transactionId is kept unter bat/TransactionId.txt
@echo If neccessary the SymUpload can be deleted via bat/SymStoreDel ^<TransactionId^>
@echo.
pause

REM Copy over certificate
REM
echo.
echo Provide Certificate for signing
echo.
@echo F|@xcopy /y ..\hardlinks.supl\KnowledgeBase\certificate\schinagl.priv.at.pfx shared\certificate\schinagl.priv.at.pfx > nul
@echo F|@xcopy /y ..\hardlinks.supl\KnowledgeBase\certificate\schinagl.priv.at.txt shared\certificate\schinagl.priv.at.txt > nul

REM Create installer
REM
echo.
echo ######## Link Shell Extension ######## 
echo.
@echo Generating InstallMedia
pushd HardlinkExtension\install
call CodeSign.bat
popd
@echo F|@xcopy /y HardlinkExtension\Doc\linkshellextension.html HardlinkExtension\Doc\hardlinkshellext.html > nul

REM Pack ln.exe
REM
echo.
echo ######## ln.exe  ######## 
echo.
pushd ln
call PackMedia.bat %MAJOR_LN_VERSION%%MINOR_LN_VERSION%%PATCH_LN_VERSION%%HOTFIX_LN_VERSION%
popd

REM dupemerge.exe
REM
echo.
echo ######## Dupemerge.exe ######## 
echo.
pushd dupemerge
call PackMedia.bat %MAJOR_LN_VERSION%%MINOR_LN_VERSION%%PATCH_LN_VERSION%%HOTFIX_LN_VERSION%
popd
