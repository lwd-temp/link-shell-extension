REM
REM Preparation for Smartcopy test
REM

REM
REM 1: directory
REM 2: noitcnuj/symbolic
REM 3: absolute during creation

set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest
set TESTROOTBKP=%TESTROOT%%DEEPPATH%\bk1

REM @echo off

if [%3] == [] ( 
  set OPTION_PREP=--%2
) else (
  set OPTION_PREP=--%2 --%3
)
@%RD% %TESTROOT%

REM
REM Testcase 1
REM
%MKDIR% %TESTROOTSRC%\F0 > nul
%MKDIR% %TESTROOTSRC%\F1 > nul
%MKDIR% "%TESTROOTSRC%\F1\F1_F0" > nul

%COPY% test\ln.h "%TESTROOTSRC%\F1\F0.h" > nul
%COPY% test\ln.h "%TESTROOTSRC%\F1\F1_F0\F1_F0.h" > nul

%LN% %OPTION_PREP% "%TESTROOTSRC%\F1\F1_F0" "%TESTROOTSRC%\F0\F0_J0" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\F1" "%TESTROOTSRC%\S1" > nul

REM
REM Testcase 2
REM
%MKDIR% %TESTROOTSRC%\F20 > nul
%MKDIR% %TESTROOTSRC%\F20\F20_F0 > nul

%MKDIR% %TESTROOTSRC%\F21xy > nul
%MKDIR% %TESTROOTSRC%\F21xy\F21xy_F0 > nul
%MKDIR% %TESTROOTSRC%\F21xy\F21xy_F0\F21xy_F0_F0 > nul

%COPY% test\ln.h "%TESTROOTSRC%\F21xy\F21xy_F0\F21xy_F0_F0\F21xy_F0_F0.h" > nul

%LN% %OPTION_PREP% "%TESTROOTSRC%\F21xy\F21xy_F0\F21xy_F0_F0" "%TESTROOTSRC%\F20\F20_F0\F20_F0_J0" > nul


