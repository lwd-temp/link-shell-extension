@echo off

REM
REM Check for commandline args
REM
if "[%~1]" == "[]" goto error

set XDEL=xdel.exe
REM set XDEL=echo

REM
REM List all directories matching the wildcard
REM
for /f "delims=" %%a in ('dir /b /AD "%~1"') do (
  %XDEL% "%%a"
)

goto ausmausraus

:error
echo WildcardXdel: Argument is missing. Usage WildcardXdel ^<directorywildcard^>
echo e.g. WildcardXdel c:\data\myDir*

:ausmausraus
echo on

