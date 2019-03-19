%~d0
cd %~dp0

@echo off
setlocal ENABLEDELAYEDEXPANSION

REM This batch file is expected to be in the Build/ folder of PumaOpen sourcetree
REM It runs a gitclean on any directory except GIT_CLEAN_EXCEPT and on except on
REM symbolic links


REM the name of the directory, which you would like to exclude
REM
set GIT_CLEAN_EXCEPT=

REM Do not edit below this line
REM

REM Run gitclean on any directory except %GIT_CLEAN_EXCEPT%
REM
set DIRPREFIX=.
@for /d %%D in (%DIRPREFIX%\*) do (
  
  REM Check if it is a symbolic link
  @for /f "delims=" %%a in ('fsutil reparsepoint query %%~fD ^| find "Symbolic Link" ^>nul ^&^& echo 1 ^|^| echo 0') do (
    if %%a == 0 (
      REM only run git clean on non symbolic links and not on %GIT_CLEAN_EXCEPT%
	  if /i %%D neq %DIRPREFIX%\%GIT_CLEAN_EXCEPT% @git clean -xfd %%~fD
    )
  )
)