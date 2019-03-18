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
set OPTION=--%2
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
REM SUCC: Dead Junction destroying Disk-ID for hardlinks
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
%TRUECRYPT% test\NtfsTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit

mkdir %TESTROOTSRC% > nul

mkdir %TESTROOTSRCDRV%\RD_F1 > nul
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\RD_F1 "%TESTROOTSRC%\Junction PA_J2" > nul
@%RD% %TESTROOTSRCDRV%\RD_F1

copy test\resource.h "%TESTROOTSRC%\PA_F1.txt" > nul
%LN% "%TESTROOTSRC%\PA_F1.txt" "%TESTROOTSRC%\Hardlink PA_F1.txt" > nul

REM
REM Action
REM
%LN% --enum %TESTROOTSRC%
@echo ErrorLevel == %ERRLEV%

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit

@echo off
:ausmausraus
@echo on

@goto :EOF 
