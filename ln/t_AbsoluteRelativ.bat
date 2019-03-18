@echo off
REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM See if absolute relative creation works
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
REM Check if reparse Points, which have their common ancestor in
REM a root path, are created realtivley if needed
REM
@echo off
set TESTROOT=%1
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
%TRUECRYPT% test\NtfsTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit

REM
REM Preparation
REM 
mkdir %TESTROOTSRCDRV%\F0 > nul
copy test\resource.h %TESTROOTSRCDRV%\F0\F0.txt > nul
mkdir %TESTROOTSRCDRV%\F0\F0_F0 > nul
copy test\resource.h %TESTROOTSRCDRV%\F0\F0_F0\F0F0F0.txt > nul

mkdir %TESTROOTSRCDRV%\F1 > nul
copy test\resource.h %TESTROOTSRCDRV%\F1\F1.txt > nul


%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\F0\F0_F0 %TESTROOTSRCDRV%\F1\F1_J0 > nul

%LN% %REPARSEOPT% %TESTROOTSRCDRV%\F1\F1_J0

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%
@%RD% %TESTROOTSRCDRV%\F0
@%RD% %TESTROOTSRCDRV%\F1

REM
REM Dismount Truecrypt Drive
REM
%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit


REM
REM Test the log levels
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst

REM
REM Preparation
REM 
mkdir %TESTROOTSRC% > nul
mkdir %TESTROOTSRC%\F0 > nul
mkdir %TESTROOTSRC%\F0\F0_F0 > nul
copy test\resource.h %TESTROOTSRC%\F0\F0_F0\F0F0F0.txt > nul

@type test\y | cacls %TESTROOTSRC%\F0\F0_F0\F0F0F0.txt /D "%USERNAME%" > nul

@echo on

REM 
REM For --traditional it is normal to give !?f here, because the inaccessibility is
REM already detected during enumeration. For fast-mode this is detected just during
REM copy
REM
REM normal --quiet. Don't show any message
REM
%LN% --quiet %ABS_REL% %OPTION% --copy %TESTROOTSRC% %TESTROOTDST% > nul> sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM 
REM Only show errors
REM
@%RD% %TESTROOTDST%
%LN% --quiet 1 %ABS_REL% %OPTION% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@%GSAR% -i -s?f -r+f -o sortout 
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM 
REM Show changes and errors
REM
@%RD% %TESTROOTDST%
%LN% --quiet 2 %ABS_REL% %OPTION% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@%GSAR% -i -s?f -r+f -o sortout 
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM 
REM Show changes and errors, but nothing changed so it only shows errors
REM
%LN% --quiet 2 %ABS_REL% %OPTION% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@%GSAR% -i -s?f -r+f -o sortout 
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM 
REM Show all
REM
copy test\resource.h %TESTROOTSRC%\F0\F0_F0\F0F0F1.txt > nul
%LN% --quiet 3 %ABS_REL% %OPTION% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@%GSAR% -i -s?f -r+f -o sortout 
@sort sortout
@echo ErrorLevel == %ERRLEV%

@type test\y | cacls %TESTROOTSRC%\F0\F0_F0\F0F0F0.txt /G "%USERNAME%":F > nul

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo off
:ausmausraus
@echo on

@goto :EOF 
