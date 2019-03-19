%~d0
cd %~dp0
call ..\Settings.bat

@set CERTIFICATE=..\..\Shared\certificate\LinkShellextension.pfx
@set SIGNTOOL=..\..\tools\signtool.exe

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\x64\Release\HardlinkShellExt.dll
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\win32\Release\HardlinkShellExt.dll

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\x64\Release\symlink.exe
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\win32\Release\symlink.exe

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\x64\Release\LSEConfig.exe
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\win32\Release\LSEConfig.exe


call CreateInstall.bat

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Media\HardLinkShellExt_X64.exe
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Media\HardLinkShellExt_win32.exe






