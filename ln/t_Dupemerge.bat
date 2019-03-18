@echo off
REM
REM 1: directory
REM 2: copy/recursive
REM 3: junction/symbolic
REM 4: absolute during copy
REM 5: unroll/splice


REM
REM Check 
REM
call t_Dupemerge01Preperation.bat  %1

@echo off
REM
REM Test
REM
@echo on
%LN% --dupemerge --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --enum %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT% > nul
@echo on

