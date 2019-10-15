%~d0
cd %~dp0
call ..\Settings.bat

REM @set CERTIFICATE=..\..\Shared\certificate\LinkShellextension.pfx
@set CERTIFICATE=..\..\Shared\certificate\schinagl.priv.at.pfx
@set PWD_LOC=..\..\Shared\certificate\schinagl.priv.at.txt
@set /p PASSWORT=<%PWD_LOC%
@set TIME_SERVER="http://timestamp.entrust.net/TSS/RFC3161sha2TS"
@set SIGNTOOL=..\..\tools\signtool.exe

set FILES_TO_SIGN=..\..\Bin\x64\Release\HardlinkShellExt.dll ..\..\Bin\win32\Release\HardlinkShellExt.dll ..\..\Bin\x64\Release\LSEUacHelper.exe ..\..\Bin\win32\Release\LSEUacHelper.exe ..\..\Bin\win32\Release\LSEConfig.exe ..\..\Bin\x64\Release\LSEConfig.exe
%SIGNTOOL% sign /p %PASSWORT% /v /fd sha256 /td sha256 /f  %CERTIFICATE% /tr %TIME_SERVER% %FILES_TO_SIGN%

call CreateInstall.bat

set FILES_TO_SIGN=..\..\Media\HardLinkShellExt_X64.exe ..\..\Media\HardLinkShellExt_win32.exe
%SIGNTOOL% sign /p %PASSWORT% /v /fd sha256 /td sha256 /f  %CERTIFICATE% /tr %TIME_SERVER% %FILES_TO_SIGN%

del %PWD_LOC%






