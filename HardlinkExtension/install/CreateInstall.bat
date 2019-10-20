REM
REM This setup can be compiled with NSIS201
REM Install NSIS201 to the location of your
REM choice and edit the line below with respect to
REM

%~d0
cd %~dp0
call ..\..\bat\Settings.bat

set NSIS="%NSISDIR%\makensis.exe"
set COPYCMD=Y
set STAMPVER=%HOME%\..\tools\stampver.exe
set GREP=%HOME%\..\tools\grep.exe

REM
REM Compile install script
REM

REM Check if artefacts for setup generation are available
REM
set PROCESSES_DLL=setup-processes\bin\win32\release\Processes.dll
set LANG_DLL=setup-processes\bin\win32\release\LangDll.dll

if not exist %PROCESSES_DLL% (
	echo ### Please recompile Processes.dll. See setup-processes\TheBigPicture.txt
	exit /b 1
)

if not exist %LANG_DLL% (
	echo ### Please recompile Lang.dll. See setup-processes\TheBigPicture.txt
	exit /b 2
)

REM VS2017 - Win32 Version, Plattform 0x601
REM
:win32
copy /y %PROCESSES_DLL% "%NSISDIR%\Plugins\Processes.dll" > nul
copy /y %LANG_DLL% "%NSISDIR%\Plugins\LangDll.dll" > nul

echo Compiling HardLinkShellExt_win32.nsi ...
copy Stubs.org\lzma* "%NSISDIR%\Stubs" > nul
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma" | %GREP% -i error
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma_solid" | %GREP% -i error
%NSIS% HardLinkShellExt_win32.nsi > HardLinkShellExt_win32.nsi.log
if %ERRORLEVEL% == 0 goto x64
	echo ### Error: compiling HardLinkShellExt_win32.nsi
	exit /b 3

REM
REM VS2017 - X64 Version
REM
:x64
copy /y %PROCESSES_DLL% "%NSISDIR%\Plugins\Processes.dll"  > nul
copy /y %LANG_DLL% "%NSISDIR%\Plugins\LangDll.dll" > nul

echo Compiling HardLinkShellExt_x64.nsi ...
copy Stubs.64\lzma* "%NSISDIR%\Stubs" > nul
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma" | %GREP% -i error
%STAMPVER% -vstampver.inf "%NSISDIR%\Stubs\lzma_solid" | %GREP% -i error
%NSIS% HardLinkShellExt_x64.nsi > HardLinkShellExt_x64.nsi.log
if %ERRORLEVEL% == 0 goto ausmausraus
	echo ### Error: compiling HardLinkShellExt_x64.nsi
	exit /b 4

copy Stubs.org\lzma* "%NSISDIR%\Stubs" > nul


:ausmausraus
REM
REM wait 5 seconds
REM
echo Install created successfully
REM @ping 127.0.0.1 -n 10 -w 1000 > nul

exit /b 0


