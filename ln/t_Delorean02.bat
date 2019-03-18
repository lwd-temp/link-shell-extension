@echo off
REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during %COPY%
REM [5: absolute during creation]

REM
REM SUCC: Check Unroll of Junctions
REM
call t_Unroll01Prepare01.bat %1 %3 %5
set OPTION=--%2
set REPARSEOPT=--%3

if [%4] == [absolute] ( 
  SET ABS_REL=--%4
) else (
  SET ABS_REL=
)

@echo on
REM
REM SmartClone
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --recursive %TESTROOTSRC%\F3 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_F2\F3_F2_J1"
)

REM
REM Copy
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --copy %TESTROOTSRC%\F3 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
%WHERE% *.* %TESTROOTDST% | sort 

@echo on
REM
REM Do the timewarp
REM
%LN% %OPTION% %ABS_REL% --delorean %TESTROOTSRC%\F3 %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@call :CheckReparsePoints %3 %TESTROOTDST% %2
@echo.
@call :CheckReparsePoints %3 %TESTROOTBKP% %2

REM
REM Do the timewarp again, to check the error messages already during clone of delorean
REM
@%RD% %TESTROOTBKP%
%LN% %OPTION% %ABS_REL% --delorean %TESTROOTSRC%\F3 %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM delete a few Junctions from DST and let it regenerate
REM
@%RD% %TESTROOTBKP%
@%RD% %TESTROOTDST%\F3_J0
@%RD% %TESTROOTDST%\F3_J1

%LN% %OPTION% %ABS_REL% --delorean %TESTROOTSRC%\F3 %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@call :CheckReparsePoints %3 %TESTROOTBKP% %2

REM
REM delete a few Junctions from DST and let it regenerate
REM delete a few junctions from SRC so that it is removed from
REM
@%RD% %TESTROOTDST%
%LN% --move %TESTROOTBKP% %TESTROOTDST%


@%RD% %TESTROOTDST%\F3_J1
@%RD% %TESTROOTSRC%\F3\F3_J0

%LN% %OPTION% %ABS_REL% --delorean %TESTROOTSRC%\F3 %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@call :CheckReparsePoints %3 %TESTROOTBKP% %2
%DIR% %TESTROOTBKP% | sort

:ausmausraus
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@goto :EOF 

:CheckReparsePoints
@%LN% %REPARSEOPT% "%2\F3_F2\F3_F2_J1"
@if [%3] == [splice] ( 
    @%LN% %REPARSEOPT% "%2\F3_J0"
    @%LN% %REPARSEOPT% "%2\F3_J1"
  @if [%1] == [symbolic] ( 
    @%LN% %REPARSEOPT% "%2\F3_F2\F3_F2.syl"
  )
) else (
  @%LN% %REPARSEOPT% "%2\F3_J1\F1_F0_J1"
  @%LN% %REPARSEOPT% "%2\F3_J1\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F0\F1_F0_J1"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F1\F1_F1_J1"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_J0"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J0"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J1"
  @if [%1] == [symbolic] ( 
    @%LN% %REPARSEOPT% "%2\F3_F2\F3_F2.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_F1\F2_F2_F1.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_F1\F2_F1.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F0\F1_F0_F2\F1_F0_F2.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F1\F1_F1_F0\F1_F1_F0.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2.syl"
  )
)
@exit /b

