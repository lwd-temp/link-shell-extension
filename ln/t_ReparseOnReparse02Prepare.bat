REM
REM Preparation for nested junctions
REM

@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest
set TESTROOTBKP=%TESTROOT%%DEEPPATH%\bk1

set OPTION_PREP=%2

if [%3] == [] ( 
  set OPTION_PREP=--%2
) else (
  set OPTION_PREP=--%2 --%3
)


REM
REM The simple case of hidden reparse points
REM
%MKDIR% %TESTROOTSRC%\B_F1 > nul
%MKDIR% %TESTROOTSRC%\B_F1\B_F1_F0 > nul

%LN% %OPTION_PREP% %TESTROOTSRC%\B_F1 %TESTROOTSRC%\C_J0 > nul
%LN% %OPTION_PREP% %TESTROOTSRC%\C_J0\B_F1_F0 %TESTROOTSRC%\A_J1 > nul

