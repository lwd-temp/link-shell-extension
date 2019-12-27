@setlocal

@set CERTUTIL=certutil.exe
@set ROOT=%~dp0..\..\..
@set HEAD=%ROOT%\tools\head.exe
@set TAIL=%ROOT%\tools\tail.exe
@set GSAR=%ROOT%\tools\gsar.exe
@set CPACK=cpack.exe

@REM Prepare the nuspec file
@set /p LSE_VERSION=<%~dp0LSE_version.txt
@%GSAR% %~dp0linkshellextension.nuspec.template -f -s##LSE_VERSION## -r%LSE_VERSION% %~dp0linkshellextension.nuspec

@REM Prepare the hash sums for the installer
@call :genhash %ROOT%\Media\HardLinkShellExt_win32.exe
@%GSAR% %~dp0chocolateyInstall.ps1.template -f -s##CHECKSUM32## -r%SHA256% %~dp0tools\chocolateyInstall.ps1 >nul
@call :genhash %ROOT%\Media\HardLinkShellExt_X64.exe
@%GSAR% %~dp0tools\chocolateyInstall.ps1 -s##CHECKSUM64## -r%SHA256% -o >nul
@set PACKAGE_NAME=LinkShellExtension.%LSE_VERSION%.nupkg
@set LSE_VERSION=%LSE_VERSION:.=%
@%GSAR% %~dp0tools\chocolateyInstall.ps1 -s##LSE_VERSION## -r%LSE_VERSION% -o >nul

@REM Generate the packages
@%CPACK% %~dp0linkshellextension.nuspec --outputdirectory %ROOT%\Media

@REM Verify package
@REM choco install LinkShellExtension -source "'%cd%;https://chocolatey.org/api/v2/'"

@REM Upload the media
@call %ROOT%\bat\FtpUpload.bat hardlinkshellext/save/%LSE_VERSION% %ROOT%/media/HardLinkShellExt_X64.exe
@call %ROOT%\bat\FtpUpload.bat hardlinkshellext/save/%LSE_VERSION% %ROOT%/media/HardLinkShellExt_win32.exe

@REM Push package to chocolatey
@choco push %ROOT%\media\%PACKAGE_NAME% --source https://push.chocolatey.org/

@goto :EOF

:genhash 
@FOR /F "tokens=* USEBACKQ" %%F IN (`@%CERTUTIL% -hashfile %1 SHA256 ^| @%HEAD% -n 2^| @%TAIL% -n 1^|@%GSAR% -F -s:032 -r`) DO @(
@set SHA256=%%F
)
@exit /b 