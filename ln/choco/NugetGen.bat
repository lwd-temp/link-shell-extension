@setlocal

@set CERTUTIL=certutil.exe
@set ROOT=%~dp0..\..
@set HEAD=%ROOT%\tools\head.exe
@set TAIL=%ROOT%\tools\tail.exe
@set GSAR=%ROOT%\tools\gsar.exe
@set CPACK=cpack.exe

@REM Prepare the nuspec file
@set /p LN_VERSION=<%~dp0LN_version.txt
@%GSAR% %~dp0ln.nuspec.template -f -s##LN_VERSION## -r%LN_VERSION% %~dp0ln.nuspec

@REM Prepare the hash sums for the installer
@call :genhash %ROOT%\Media\ln.zip
@%GSAR% %~dp0chocolateyInstall.ps1.template -f -s##CHECKSUM32## -r%SHA256% %~dp0tools\chocolateyInstall.ps1 >nul
@call :genhash %ROOT%\Media\ln64.zip
@%GSAR% %~dp0tools\chocolateyInstall.ps1 -s##CHECKSUM64## -r%SHA256% -o >nul
@set PACKAGE_NAME=ln.%LN_VERSION%.nupkg
@set LN_VERSION=%LN_VERSION:.=%
@%GSAR% %~dp0tools\chocolateyInstall.ps1 -s##LN_VERSION## -r%LN_VERSION% -o >nul

@REM Generate the packages
@%CPACK% %~dp0ln.nuspec --outputdirectory %ROOT%\Media

@REM Verify package
@REM choco install ln -source "'%cd%;https://chocolatey.org/api/v2/'"

@REM Upload the media
@call %ROOT%\bat\FtpUpload.bat ln/save/%LN_VERSION% %ROOT%/media/ln.zip
@call %ROOT%\bat\FtpUpload.bat ln/save/%LN_VERSION% %ROOT%/media/ln64.zip

@REM Push package to chocolatey
@choco push %ROOT%\media\%PACKAGE_NAME% --source https://push.chocolatey.org/

@goto :EOF

:genhash 
@FOR /F "tokens=* USEBACKQ" %%F IN (`@%CERTUTIL% -hashfile %1 SHA256 ^| @%HEAD% -n 2^| @%TAIL% -n 1^|@%GSAR% -F -s:032 -r`) DO @(
@set SHA256=%%F
)
@exit /b 