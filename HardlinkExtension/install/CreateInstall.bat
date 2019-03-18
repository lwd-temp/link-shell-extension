REM
REM This setup can be compiled with NSIS201
REM Install NSIS201 to the location of your
REM choice and edit the line below with respect to
REM

%~d0
cd %~dp0
call ..\Settings.bat

set NSIS="%NSISDIR%\makensis.exe"
set COPYCMD=Y
set STAMPVER=%HOME%\..\tools\stampver.exe

goto win32_502
REM
REM Compile install script
REM


REM VS2017 - Win32 Version, Plattform 0x502
REM
:win32_502
copy /y setup-processes\bin\win32\release_502\Processes.dll "%NSISDIR%\Plugins\Processes.dll"
copy /y setup-processes\bin\win32\release_502\LangDll.dll "%NSISDIR%\Plugins\LangDll.dll"

copy Stubs.org\lzma* "%NSISDIR%\Stubs"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma_solid"
%NSIS% HardLinkShellExt_win32_502.nsi

REM VS2017 - Win32 Version, Plattform 0x601
REM
:win32
copy /y setup-processes\bin\win32\release\Processes.dll "%NSISDIR%\Plugins\Processes.dll"
copy /y setup-processes\bin\win32\release\LangDll.dll "%NSISDIR%\Plugins\LangDll.dll"

copy Stubs.org\lzma* "%NSISDIR%\Stubs"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma_solid"
%NSIS% HardLinkShellExt_win32.nsi

REM goto ausmausraus


REM
REM VS2017 - X64 Version
REM
:x64
copy /y setup-processes\bin\win32\release\Processes.dll "%NSISDIR%\Plugins\Processes.dll"
copy /y setup-processes\bin\win32\release\LangDll.dll "%NSISDIR%\Plugins\LangDll.dll"

copy Stubs.64\lzma* "%NSISDIR%\Stubs"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma"
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma_solid"
%NSIS% HardLinkShellExt_x64.nsi
REM %NSIS% HardLinkShellExt_x64_Debug.nsi
copy Stubs.org\lzma* "%NSISDIR%\Stubs"


goto ausmausraus


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



