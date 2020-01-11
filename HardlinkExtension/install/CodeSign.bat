@%~d0
@cd %~dp0
@echo off

@REM MS also offers EV Code Certificates, which enable immediate acceptance by SmartScreeen of Windows Defender
@REM https://blogs.msdn.microsoft.com/ie/2012/08/14/microsoft-smartscreen-extended-validation-ev-code-signing-certificates/

call ..\..\bat\Settings.bat
call ..\..\bat\CertificateSettings.bat
if %ERRORLEVEL% == 0 goto SignBinaryFiles
	exit /b 1

:SignBinaryFiles
@set FILES_TO_SIGN=..\..\Bin\x64\Release\HardlinkShellExt.dll ..\..\Bin\win32\Release\HardlinkShellExt.dll ..\..\Bin\x64\Release\LSEUacHelper.exe ..\..\Bin\win32\Release\LSEUacHelper.exe ..\..\Bin\win32\Release\LSEConfig.exe ..\..\Bin\x64\Release\LSEConfig.exe
%SIGNTOOL% sign /p "%CERTIFICATE_PASSWORD%" /v /fd sha256 /td sha256 /f  %CERTIFICATE% /tr %TIME_SERVER% %FILES_TO_SIGN%
if %ERRORLEVEL% == 0 goto CompileInstall
	echo ### Error: Signing of binary files failed
	exit /b 2

:CompileInstall
echo.
call CreateInstall.bat
set ERRLEV=%ERRORLEVEL%
if %ERRORLEVEL% == 0 goto SignInstaller
	echo ### Error: Failed to create Installer: Error: %ERRLEV%
	exit /b 3

:SignInstaller
echo.
@set FILES_TO_SIGN=..\..\Media\HardLinkShellExt_X64.exe ..\..\Media\HardLinkShellExt_win32.exe
%SIGNTOOL% sign /p "%CERTIFICATE_PASSWORD%" /v /fd sha256 /td sha256 /f  %CERTIFICATE% /tr %TIME_SERVER% %FILES_TO_SIGN%
if %ERRORLEVEL% == 0 goto CleanupCodeSign
	echo ### Error: Signing of installer failed
	exit /b 4

:CleanupCodeSign
exit /b 0






