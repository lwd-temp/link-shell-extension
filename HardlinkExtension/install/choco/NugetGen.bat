@set CERTUTIL=certutil.exe
@set ROOT=..\..\..
@set HEAD=%ROOT%\tools\head.exe
@set TAIL=%ROOT%\tools\tail.exe
@set GSAR=%ROOT%\tools\gsar.exe
@set CPACK=cpack.exe

@REM Prepare the has sums
@call :genhash %ROOT%\Media\HardLinkShellExt_win32.exe
@%GSAR% tools\chocolateyInstall.ps1.template -f -s##CHECKSUM32## -r%SHA256% tools\chocolateyInstall.ps1 >nul
@call :genhash %ROOT%\Media\HardLinkShellExt_X64.exe
@%GSAR% tools\chocolateyInstall.ps1 -s##CHECKSUM64## -r%SHA256% -o >nul

@REM Generate the packages
@%CPACK% --outputdirectory %ROOT%\Media

@REM Verify package
@REM choco install LinkShellExtension -source "'%cd%;https://chocolatey.org/api/v2/'"

@REM Push package to chocolatey
@choco push ..\..\..\media\LinkShellExtension.3.9.3.0.nupkg --source https://push.chocolatey.org/

@goto :EOF

:genhash 
@FOR /F "tokens=* USEBACKQ" %%F IN (`@%CERTUTIL% -hashfile %1 SHA256 ^| %HEAD% -n 2^| %TAIL% -n 1^|@%GSAR% -F -s:032 -r`) DO (
@set SHA256=%%F
)
@exit /b 