@echo off
REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM SUCC: Check Unroll of Junctions
REM
if [%2] == [none] ( 
  SET OPTION=
) else (
  SET OPTION=--%2
)

set REPARSEOPT=--%3

if [%4] == [absolute] ( 
  SET ABS_REL=--%4
) else (
  SET ABS_REL=
)

if [%5] == [absolute] ( 
  SET ABS_REL_COPY=--%5
) else (
  SET ABS_REL_COPY=
)
@echo on


REM 
REM Inner Circularity 
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst

mkdir %TESTROOTSRC%
copy test\resource.h %TESTROOTSRC%\RD00.txt > nul
copy test\resource.h %TESTROOTSRC%\RD01.txt > nul
copy test\resource.h %TESTROOTSRC%\RD02.txt > nul

mkdir %TESTROOTSRC%\F0
copy test\resource.h %TESTROOTSRC%\F0\RD00.txt > nul
copy test\resource.h %TESTROOTSRC%\F0\RD01.txt > nul
copy test\resource.h %TESTROOTSRC%\F0\RD02.txt > nul
mkdir %TESTROOTSRC%\F0\F0_F0
copy test\resource.h %TESTROOTSRC%\F0\F0_F0\RD_F0_F0.txt > nul

mkdir %TESTROOTSRC%\F1
copy test\resource.h %TESTROOTSRC%\F1\RD00.txt > nul
copy test\resource.h %TESTROOTSRC%\F1\RD01.txt > nul
copy test\resource.h %TESTROOTSRC%\F1\RD02.txt > nul
mkdir %TESTROOTSRC%\F1\F1_F0
copy test\resource.h %TESTROOTSRC%\F1\F1_F0\RD_F1_F0.txt > nul

%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRC%\F1 %TESTROOTSRC%\F1\F1_F0\F1_F0_J0 
REM> nul
@echo on

REM
REM Inner Circularity Copy
REM
%LN% %OPTION% %ABS_REL_COPY% --copy %TESTROOTSRC% "%TESTROOTDST%"  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% %TESTROOTSRC%\F1\F1_F0\F1_F0_J0
)

REM 
REM Outer Circularity 
REM
@%RD% %TESTROOTDST%
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRC%\F1 %TESTROOTSRC%\F0\F0_F0\F0_F0_J0 
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRC%\F0 %TESTROOTSRC%\F1\F1_F0\F1_F0_J1 

REM
REM Outer Circularity Copy
REM
%LN% %OPTION% %ABS_REL_COPY% --copy %TESTROOTSRC%\F1 %TESTROOTDST%  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% %TESTROOTSRC%\F1\F1_F0\F1_F0_J0
)

@%LN% --junction %TESTROOTDST%\F1_F0\F1_F0_J0
@%LN% --junction %TESTROOTDST%\F1_F0\F1_F0_J1\F0_F0\F0_F0_J0 

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo off
:ausmausraus
@echo on

@goto :EOF 
