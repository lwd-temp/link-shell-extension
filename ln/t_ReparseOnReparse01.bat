@echo off
REM -----------------

REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM SUCC: Check Unroll of Junctions
REM
call t_ReparseOnReparse01Prepare.bat %1 %3 %5
set OPTION=
set REPARSEOPT=--%3

if [%4] == [absolute] ( 
  SET ABS_REL=--%4
) else (
  SET ABS_REL=
)
@echo on

REM
REM Simply copy nested reparse points
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0\F0_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0\F0_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_F0_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_Z_Reparse"


REM
REM Move nested reparse points
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --move %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0\F0_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0\F0_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_F0_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_Z_Reparse"

%LN% %OPTION% %ABS_REL% --move %TESTROOTDST% %TESTROOTSRC% > sortout


REM
REM Mirror nested reparse points
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0\F0_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0\F0_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_F0_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_Z_Reparse"


REM
REM Mirror & unroll nested reparse points
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL%  --unroll --mirror %TESTROOTSRC%\F1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_Reparse\F0_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_Reparse\F0_A_Reparse"


REM
REM Copy & unroll nested reparse points
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL%  --unroll --copy %TESTROOTSRC%\F1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_Reparse\F0_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_Reparse\F0_A_Reparse"

REM
REM Copy & splice nested reparse points
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL%  --splice --copy %TESTROOTSRC%\F1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_Reparse"


REM
REM Delorean nested reparse points
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --copy %TESTROOTSRC% %TESTROOTDST% > nul
%LN% %OPTION% %ABS_REL% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F0\F0_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F0\F0_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1\F1_F0_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1\F1_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1\F1_Z_Reparse"


REM
REM Delorean & unroll nested reparse points
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL%  --unroll --copy %TESTROOTSRC%\F1 %TESTROOTDST% > nul
%LN% %OPTION% %ABS_REL%  --unroll --delorean %TESTROOTSRC%\F1 %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_A_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_F0_Reparse\F0_Z_Reparse"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_F0_Reparse\F0_A_Reparse"


REM
REM Hidden reparse points the simple way
REM
@%RD% %TESTROOT%
call t_ReparseOnReparse02Prepare.bat %1 %3 %5

REM
REM Copy
REM
@echo on
%LN% %OPTION% %ABS_REL%  --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@call :CheckReparseonReparse02 %3 %TESTROOTDST%

REM
REM Hidden reparse points the hard way
REM
@%RD% %TESTROOT%
call t_ReparseOnReparse03Prepare.bat %1 %3 %5

REM
REM Copy
REM
@echo on
%LN% %OPTION% %ABS_REL%  --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@call :CheckReparseonReparse03 %3 %TESTROOTDST%

:ausmausraus
@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo on

@goto :EOF 

:CheckReparseonReparse03
@%LN% %REPARSEOPT% %2\Z_J0
@%LN% %REPARSEOPT% %2\X_F0\C_F0_J0
@%LN% %REPARSEOPT% %2\I_F1\B_F1_F0\A_F1_F0_J0

@%LN% %REPARSEOPT% %2\V_J1
@%LN% %REPARSEOPT% %2\M_J2
@%LN% %REPARSEOPT% %2\G_J3

@%LN% %REPARSEOPT% %2\A_J4
@exit /b

:CheckReparseonReparse02
@%LN% %REPARSEOPT% %TESTROOTSRC%\C_J0
@%LN% %REPARSEOPT% %TESTROOTSRC%\A_J1
@exit /b
