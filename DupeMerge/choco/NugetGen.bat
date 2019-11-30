@setlocal

@set CERTUTIL=certutil.exe
@set ROOT=%~dp0..\..
@set HEAD=%ROOT%\tools\head.exe
@set TAIL=%ROOT%\tools\tail.exe
@set GSAR=%ROOT%\tools\gsar.exe
@set CPACK=cpack.exe

@REM Prepare the hash sums for the installer
@call :genhash %ROOT%\Media\dupemerge.zip
@%GSAR% %~dp0chocolateyInstall.ps1.template -f -s##CHECKSUM32## -r%SHA256% %~dp0tools\chocolateyInstall.ps1 >nul
@call :genhash %ROOT%\Media\dupemerge64.zip
@%GSAR% %~dp0tools\chocolateyInstall.ps1 -s##CHECKSUM64## -r%SHA256% -o >nul

@Rem Prepare the version number
@set /p DUPEMERGE_VERSION=<%~dp0DupeMerge_version.txt
@set UPLOAD_DIR=%DUPEMERGE_VERSION:.=%
@set DUPEMERGE_VERSION=%UPLOAD_DIR:~0,1%.%UPLOAD_DIR:~1,3%
@%GSAR% %~dp0tools\chocolateyInstall.ps1 -s##DUPEMERGE_VERSION## -r%UPLOAD_DIR% -o >nul

@REM Prepare the nuspec file
@%GSAR% %~dp0dupemerge.nuspec.template -f -s##DUPEMERGE_VERSION## -r%DUPEMERGE_VERSION% %~dp0dupemerge.nuspec


@REM Generate the packages
@%CPACK% %~dp0dupemerge.nuspec --outputdirectory %ROOT%\Media

@REM Verify package
@REM choco install dupemerge -source "'%cd%;https://chocolatey.org/api/v2/'"

@REM Upload the media
@call %ROOT%\bat\FtpUpload.bat dupemerge/save/%UPLOAD_DIR% %ROOT%/media/dupemerge.zip
@call %ROOT%\bat\FtpUpload.bat dupemerge/save/%UPLOAD_DIR% %ROOT%/media/dupemerge64.zip


@REM Push package to chocolatey
@choco push %ROOT%\media\dupemerge.%DUPEMERGE_VERSION%.nupkg --source https://push.chocolatey.org/

@goto :EOF

:genhash 
@FOR /F "tokens=* USEBACKQ" %%F IN (`@%CERTUTIL% -hashfile %1 SHA256 ^| @%HEAD% -n 2^| @%TAIL% -n 1^|@%GSAR% -F -s:032 -r`) DO @(
@set SHA256=%%F
)
@exit /b 