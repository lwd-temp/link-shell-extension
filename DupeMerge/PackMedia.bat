@echo off

call ..\bat\settings.bat
call ..\bat\CertificateSettings.bat

set BJKHOME=%~dp0
set VERSION=_%1
set zip=%BJKHOME%..\tools\zip.exe -q

set HARDLINKHOME=..
set MEDIA=%BJKHOME%..\Media

REM x86
REM
set BINDIR=%HARDLINKHOME%\Bin\win32\Release
set ARCHIVE=dupemerge%VERSION%.zip
call :ZipBinary %MEDIA%\%ARCHIVE% %BINDIR%
call :ZipAllFiles %MEDIA%\%ARCHIVE%
copy %MEDIA%\%ARCHIVE% %MEDIA%\dupemerge.zip

REM x64
REM
set BINDIR=%HARDLINKHOME%\Bin\x64\Release
set ARCHIVE=dupemerge64%VERSION%.zip
call :ZipBinary %MEDIA%\%ARCHIVE% %BINDIR%
call :ZipAllFiles %MEDIA%\%ARCHIVE%
copy %MEDIA%\%ARCHIVE% %MEDIA%\dupemerge64.zip

:ausmausraus

REM
REM wait 10 seconds
REM
REM @ping 127.0.0.1 -n 10 -w 1000 > nul

goto :EOF

:ZipAllFiles
%ZIP% %1 Doc\dupemerge.html
%ZIP% %1 Doc\dupemerge.png
%ZIP% %1 Doc\amazon.de.png
%ZIP% %1 Doc\bitcoinlogo.png
%ZIP% %1 Doc\bitcoinlseqr.png
%ZIP% %1 Doc\license.txt
%ZIP% %1 Doc\license_tre.txt
%ZIP% %1 Doc\license_ultragetop.txt
exit /b

:ZipBinary
pushd %2
@set FILE_TO_SIGN="dupemerge.exe"
%SIGNTOOL% sign /p "%CERTIFICATE_PASSWORD%" /v /fd sha256 /td sha256 /f  %CERTIFICATE% /tr %TIME_SERVER% %FILE_TO_SIGN%
if %ERRORLEVEL% == 0 goto ZipIt
	echo ### Error Signing of ln.exe failed
	exit /b 2
:ZipIt
%ZIP% %1 %FILE_TO_SIGN%
popd
exit /b

