@echo off

call %~dp0\settings.bat

set CONFIG=Debug
REM set CONFIG=Release

set EXPLORER=explorer.exe

REM
REM Do not Edit below this line
REM
set PROJECTNAME=Hardlinkshellext
set MODULENAME=%PROJECTNAME%.dll

set TSTAMP=%TIME%
for /f %%a in ('echo %TSTAMP:~0,2%') do set s0=%%a
for /f %%a in ('echo %TSTAMP:~3,2%') do set s1=%%a
for /f %%a in ('echo %TSTAMP:~6,2%') do set s2=%%a
for /f %%a in ('echo %TSTAMP:~9,2%') do set s3=%%a

set DSTAMP=%DATE%
for /f %%a in ('echo %DSTAMP:~0,2%') do set s4=%%a
for /f %%a in ('echo %DSTAMP:~3,2%') do set s5=%%a
for /f %%a in ('echo %DSTAMP:~6,4%') do set s6=%%a

set STAMP=%s6%%s5%%s4%%s0%%s1%%s2%%s3%
echo %STAMP%
@echo on
taskkill /F /IM explorer.exe
REM %PSKILL% explorer.exe
move "%INST%\%MODULENAME%" "%INST%\%MODULENAME%.%STAMP%"
copy %HOME%\bin\%PLATT%\%CONFIG%\%MODULENAME% "%INST%"
copy %HOME%\..\bin\%PLATT%\%CONFIG%\symlink.exe "%INST%"

copy %HOME%\bin\%PLATT%\%CONFIG%\%PROJECTNAME%.pdb "%INST%"
copy %HOME%\..\bin\%PLATT%\%CONFIG%\symlink.pdb "%INST%"

copy %HOME%\..\LSEConfig\bin\%PLATT%\%CONFIG%\LSEConfig.exe "%INST%"
copy %HOME%\..\LSEConfig\bin\%PLATT%\%CONFIG%\LSEConfig.pdb "%INST%"


if "%PLATT%"=="X64" (
  REM
  REM Make sure cmd64 is copied over from the servicepack files
  REM 
  if exist %systemroot%\cmd64.exe (
    start cmd64 start /k explorer.exe
    taskkill /im cmd64.exe
  ) else  (
    start %EXPLORER%
  )
)
if "%PLATT%"=="Win32" (
  start %EXPLORER%
)

:ausmausraus
REM
REM wait 10 seconds
REM
@ping 127.0.0.1 -n 10 -w 1000 > nul

