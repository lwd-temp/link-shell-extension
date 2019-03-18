%~d0
cd %~dp0
call ..\Settings.bat

@set SAVE_PATH=%PATH%
@set PATH=%PATH%;%VS2005TOOLS_PATH%

@set CERTIFICATE=..\..\KnowledgeBase\certificate\LinkShellextension.pfx

@set SIGNTOOL=%VS2005TOOLS_PATH%\signtool.exe

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\Bin\x64\Release\HardlinkShellExt.dll
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\Bin\win32\Release\HardlinkShellExt.dll

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\x64\Release\symlink.exe
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\win32\Release\symlink.exe

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\x64\Release\ln.exe
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\Bin\win32\Release\ln.exe

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\LSEConfig\Bin\x64\Release\LSEConfig.exe
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" ..\..\LSEConfig\Bin\win32\Release\LSEConfig.exe


call CreateInstall.bat

%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" HardLinkShellExt_X64.exe
%SIGNTOOL% sign /f %CERTIFICATE% /v /i "Hermann Schinagl" /d "Link Shellextension" /du "http://schinagl.priv.at" HardLinkShellExt_win32.exe























@set PATH=%SAVE_PATH%





