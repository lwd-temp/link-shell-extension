@echo off

REM
REM Check for commandline args
REM
if "[%~1]" == "[]" goto error
if "[%~2]" == "[]" goto error

set LN=ln.exe

if not exist "%~1" goto error
if not exist "%~2" goto error

setLocal EnableDelayedExpansion

REM
REM Get date and time for timestamps on directories
REM
for /f "delims=" %%a in ('%LN% --datetime') do set DATETIMESTAMP=%%a

set SourceDir=%~n1
set DestDir=%~2

REM
REM Do the Delorean Copy
REM
pushd %DestDir%
set errlev=0
set BACKUPCREATED=%DestDir%\%SourceDir% - %DATETIMESTAMP%
set BACKUPLOG=%BACKUPCREATED%.log
set BACKUPOTIONS=
if exist "%SourceDir% - ????-??-?? ??-??-??" (
  for /f "delims=" %%a in ('dir /b /AD /O:N "%SourceDir% - ????-??-?? ??-??-??"') do set LastBackup=%%a
  popd
  %LN% %BACKUPOTIONS% --delorean "%~1" "%DestDir%\!LastBackup!" "%BACKUPCREATED%"
  set errlev=!errorlevel!
) else (
  popd
  %LN% %BACKUPOTIONS% --copy "%~1" "%DestDir%\%SourceDir% - %DATETIMESTAMP%"
  set errlev=!errorlevel!
)
if %errlev% NEQ 0 goto errorexit

REM
REM Remove old backup sets (if KeepMaxCopies has been provided)
REM
if not "%3"=="" (set /a KeepMax=%~3) else (set KeepMax=0)
pushd %DestDir%
if %KeepMax% GTR 0 (
  echo.
  for /f "skip=%KeepMax% tokens=* delims=" %%G in ('dir /b /A:D /O:-N "%SourceDir% - ????-??-?? ??-??-??"') do (
    echo Removing backup set "%%G" ...
    %LN% --quiet 1 --delete "%%G"
  )
)
popd

goto :EOF

REM
REM Usage
REM
:error
echo Usage: %~n0 ^<SourcePath^> ^<DestPath^> (^<KeepMaxCopies^>)
echo.
echo  SourcePath    directory containing the files to be backed up
echo  DestPath      directory where "DeLorean copy" sets will be created
echo  KeepMaxCopies (opt.) remove any exceeding number of DLC sets before copying
echo.
echo e.g. %~n0 c:\data\source c:\data\backup 90

:errorexit
exit /b %errlev%
