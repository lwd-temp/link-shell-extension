REM
REM This setup can be compiled with NSIS201
REM Install NSIS201 to the location of your
REM choice and edit the line below with respect to
REM

%~d0
cd %~dp0
call ..\Settings.bat

copy /y setup-processes\bin\win32\release\Processes.dll "%NSISDIR%\Plugins\Processes.dll"
copy /y setup-processes\bin\win32\release\LangDll.dll "%NSISDIR%\Plugins\LangDll.dll"

set NSIS="%NSISDIR%\makensis.exe"
set COPYCMD=Y
set STAMPVER=%HOME%\..\tools\stampver.exe

REM goto x64
REM
REM Compile install script
REM

@echo off
REM
REM VS6 - Win32 Version, not supported anymore with 3.2.0.0
REM
REM copy Stubs.org\*.* "%NSISDIR%\Stubs"
REM %NSIS% HardLinkShellExt_vc6.nsi
REM del HardLinkShellExt.exe
REM ren HardLinkShellExt_vc6.exe HardlinkShellExt.exe
@echo on


REM
REM VS8 - Win32 Version
REM
:win32
copy Stubs.org\lzma* "%NSISDIR%\Stubs"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma_solid"
%NSIS% HardLinkShellExt_win32.nsi
REM %NSIS% HardLinkShellExt_win32_debug.nsi

REM goto ausmausraus


REM
REM VS8 - X64 Version
REM
:x64
copy Stubs.64\lzma* "%NSISDIR%\Stubs"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma_solid"
%NSIS% HardLinkShellExt_x64.nsi
REM %NSIS% HardLinkShellExt_x64_Debug.nsi
copy Stubs.org\lzma* "%NSISDIR%\Stubs"


REM goto ausmausraus


REM
REM VS8 - IA64 Version
REM
:IA64
copy Stubs.64\lzma* "%NSISDIR%\Stubs"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma_solid"
%NSIS% HardLinkShellExt_Itanium.nsi
copy Stubs.org\lzma* "%NSISDIR%\Stubs"

:ausmausraus
REM
REM wait 5 seconds
REM
REM @ping 127.0.0.1 -n 10 -w 1000 > nul



