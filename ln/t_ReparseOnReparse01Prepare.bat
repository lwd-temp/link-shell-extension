REM
REM Preparation for Unroll test
REM

@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest
set TESTROOTBKP=%TESTROOT%%DEEPPATH%\bk1

if [%3] == [] ( 
  set OPTION_PREP=--%2
) else (
  set OPTION_PREP=--%2 --%3
)

%RD% %TESTROOT% 

%MKDIR% %TESTROOTSRC%\F0 > nul
%MKDIR% %TESTROOTSRC%\F1 > nul
%MKDIR% %TESTROOTSRC%\F1\B_Dir > nul
%MKDIR% %TESTROOTSRC%\F0\B_Dir > nul

%COPY% test\ln.h %TESTROOTSRC%\F0\B_Dir\B_Dir.txt > nul
%LN% %OPTION_PREP% %TESTROOTSRC%\F0\B_Dir %TESTROOTSRC%\F0\F0_Z_Reparse > nul
%LN% %OPTION_PREP% %TESTROOTSRC%\F0\F0_Z_Reparse %TESTROOTSRC%\F0\F0_A_Reparse > nul

%LN% %OPTION_PREP% %TESTROOTSRC%\F0 %TESTROOTSRC%\F1\F1_F0_Reparse > nul

%COPY% test\ln.h %TESTROOTSRC%\F1\B_Dir\B_Dir.txt > nul
%LN% %OPTION_PREP% %TESTROOTSRC%\F1\B_Dir %TESTROOTSRC%\F1\F1_Z_Reparse > nul
%LN% %OPTION_PREP% %TESTROOTSRC%\F1\F1_Z_Reparse %TESTROOTSRC%\F1\F1_A_Reparse > nul

REM now change the case of a reaprse point by creating it once more
REM
@%RD% %TESTROOTSRC%\F1\F1_Z_Reparse
%LN% %OPTION_PREP% %TESTROOTSRC%\F1\B_Dir %TESTROOTSRC%\F1\f1_z_rEPARSE > nul

@echo on
echo.

