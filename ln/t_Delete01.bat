@echo off
REM
REM 1: directory
REM 2: junction/symbolic
REM 3: absolute/relative


REM
REM Check a more complex structure
REM
call t_Unroll01Prepare02.bat %1 %2


REM Delete and check if we stop on inner/outer reparse boundaries
REM
%LN% --delete %TESTROOTSRC%\F1 > sortout
@sort sortout
@%WHERE%  *.* %TESTROOTSRC% | sort

REM Delete and go over the boundary for a certain wildcard
REM
@%MKDIR% %TESTROOTSRC%\F1\F1_FX
%LN% --follow F2_J0 --delete %TESTROOTSRC%\F2 > sortout
@sort sortout
@%WHERE%  *.* %TESTROOTSRC% | sort

REM Check if locked anchor can not be delted
REM
@call :ChangePermissions %TESTROOTSRC% nul empty /D "%USERNAME%" 
%LN% --delete %TESTROOTSRC% > sortout
@sort sortout
@call :ChangePermissions %TESTROOTSRC% nul empty /G "%USERNAME%":F

@REM
@REM Cleanup
@%RD% %TESTROOT%

@goto :EOF 

:ChangePermissions
@REM Lock/Unlock item
@type test\y | @%CACLS% %1 /L %4 %5 >> %2
@exit /b

