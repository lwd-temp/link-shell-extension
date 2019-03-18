REM set MT=%VS2005TOOLS_PATH%\..\..\..\vc\bin\mt.exe
set MT=.\mt.exe
%MT% -nologo -manifest "HardLinkShellExt_X64.exe.manifest" -outputresource:"HardLinkShellExt_x64.exe";1
