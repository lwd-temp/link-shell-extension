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
REM Unroll/Splice all junctions, small test
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --copy %TESTROOTSRC%\F3\F3_J1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_J0\F0_F1_J2"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_J1"
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_J0"
) else (
@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_F2\F1_F0_F2.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
)
)


%WHERE%  *.* %TESTROOTDST% | sort
%DIR% %TESTROOTDST% | sort

REM
REM Unroll/Splice all junctions, large test
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --copy %TESTROOTSRC%\F3 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1"
) else (
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1\F1_F0_J1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1\F1_F0_J0\F0_F1_J2"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_J1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_J2"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F1\F1_F1_J1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_F2\F3_F2_J1"
@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_F2\F3_F2.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_F1\F2_F2_F1.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F1\F2_F1.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_F2\F1_F0_F2.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F1\F1_F1_F0\F1_F1_F0.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2.syl"
)
)

%WHERE%  *.* %TESTROOTDST% | sort
%DIR% %TESTROOTDST% | sort

:ausmausraus
@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo on

