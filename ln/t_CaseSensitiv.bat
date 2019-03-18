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

REM
REM Mount Truecrypt
REM 
%TRUECRYPT% test\NtfsTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit

REM 
REM Case Sensitivy Tests
REM
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTBKP=%TESTROOT%\bkp
set TESTROOTBKP2=%TESTROOT%\bkp2

REM 
REM Create junctions
REM
@%RD% %TESTROOTDST%
@%RD% %TESTROOTSRC%
mkdir %TESTROOTSRC%
mkdir %TESTROOTSRC%\F0\F0_F0
mkdir %TESTROOTSRC%\F1
copy test\resource.h %TESTROOTSRC%\F0\F0_F0\F0F0F0.txt > nul
mkdir %TESTROOTSRC%\F1\F1_F0
copy test\resource.h %TESTROOTSRC%\F1\F1_F0\F1F1F0.txt > nul
%LN% %REPARSEOPT% %TESTROOTSRC%\F1 %TESTROOTSRC%\F0\F0_J1

REM
REM Intial copy
REM 
@echo on
%LN% %ABS_REL% %OPTION% --copy %TESTROOTSRC%\F0 %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM Change case of junction, but directory stays the same
REM 
@if exist %TESTROOTSRC%\F0\F0_J1 rd /q %TESTROOTSRC%\F0\F0_J1
%LN% %REPARSEOPT% %TESTROOTSRC%\F1 %TESTROOTSRC%\F0\f0_j1

REM
REM Do a Delorean again
REM
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC%\F0 %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC%\F0 %TESTROOTBKP% %TESTROOTBKP2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%


REM
REM SUCC: Outer Reparse Points on rootdrive
REM
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTBKP=%TESTROOT%\bkp
set TESTROOTBKP2=%TESTROOT%\bkp2
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%

REM
REM Preparation
REM 
mkdir %TESTROOTSRCDRV%\Tools
copy test\resource.h %TESTROOTSRCDRV%\Tools\Tools.txt > nul
mkdir %TESTROOTSRCDRV%\tools\AAConsole
copy test\resource.h %TESTROOTSRCDRV%\tools\AAConsole\AAConsole.txt > nul
mkdir %TESTROOTSRC%
mkdir %TESTROOTSRC%\F0
mkdir %TESTROOTSRC%\F0\F0_F0
copy test\resource.h %TESTROOTSRC%\F0\F0_F0\F1F1F0.txt > nul
%LN% %REPARSEOPT% %TESTROOTSRCDRV%\Tools %TESTROOTSRC%\F0\F0_J1

REM
REM Intial copy
REM 
@echo on
%LN% %ABS_REL% %OPTION% --copy %TESTROOTSRC%\F0 %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM Change case of junction, but directory stays the same
REM 
@if exist %TESTROOTSRC%\F0\F0_J1 rd /q %TESTROOTSRC%\F0\F0_J1
%LN% %REPARSEOPT% %TESTROOTSRCDRV%\Tools %TESTROOTSRC%\F0\f0_j1

REM
REM Do a Delorean again
REM
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC%\F0 %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC%\F0 %TESTROOTBKP% %TESTROOTBKP2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%
@%RD% %TESTROOTSRCDRV%\Tools

REM
REM SUCC: Outer Reparse Points on rootdrive 
REM
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTBKP=%TESTROOT%\bkp
set TESTROOTBKP2=%TESTROOT%\bkp2
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%

REM
REM Preparation
REM 
mkdir %TESTROOTSRCDRV%\Tools
copy test\resource.h %TESTROOTSRCDRV%\Tools\Tools.txt > nul
mkdir %TESTROOTSRCDRV%\tools\AAConsole
copy test\resource.h %TESTROOTSRCDRV%\tools\AAConsole\AAConsole.txt > nul
mkdir %TESTROOTSRC%
mkdir %TESTROOTSRC%\F0
mkdir %TESTROOTSRC%\F0\F0_F0
copy test\resource.h %TESTROOTSRC%\F0\F0_F0\F1F1F0.txt > nul
%LN% %REPARSEOPT% %TESTROOTSRCDRV%\Tools %TESTROOTSRC%\F0\F0_J1

REM
REM Intial copy
REM 
@echo on
%LN% %ABS_REL% %OPTION% --copy %TESTROOTSRC%\F0 %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM Change case of junction, but directory stays the same but case of driveletter changes
REM 
@if exist %TESTROOTSRC%\F0\F0_J1 rd /q %TESTROOTSRC%\F0\F0_J1
%LN% %REPARSEOPT% %EMPTYTESTDRIVE_UPPERCASE%\Tools %TESTROOTSRC%\F0\f0_j1

REM
REM Do a Delorean again
REM
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC%\F0 %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC%\F0 %TESTROOTBKP% %TESTROOTBKP2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%
@%RD% %TESTROOTSRCDRV%\Tools


REM
REM Outer Reparse Points on rootdrive
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTBKP=%TESTROOT%\bkp
set TESTROOTBKP2=%TESTROOT%\bkp2
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%

REM
REM Preparation
REM 
mkdir %TESTROOTSRCDRV%\Tools > nul
copy test\resource.h %TESTROOTSRCDRV%\Tools\Tools.txt > nul
mkdir %TESTROOTSRC%
mkdir %TESTROOTSRC%\F0
mkdir %TESTROOTSRC%\F0\F0_F0
copy test\resource.h %TESTROOTSRC%\F0\F0_F0\F1F1F0.txt > nul
%LN% %REPARSEOPT% %TESTROOTSRCDRV%\Tools %TESTROOTSRC%\F0\F0_J1

REM
REM Intial copy
REM 
@echo on
%LN% %ABS_REL% %OPTION% --copy %TESTROOTSRC%\F0 %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM Change case of junction and directory
REM 
@%RD% %TESTROOTSRCDRV%\Tools
@mkdir %TESTROOTSRCDRV%\tools
@copy test\resource.h %TESTROOTSRCDRV%\tools\tools.txt > nul
@mkdir %TESTROOTSRCDRV%\tools\AAConsole
@copy test\resource.h %TESTROOTSRCDRV%\tools\AAConsole\AAConsole.txt > nul

@%RD% %TESTROOTSRC%\F0\F0_J1
%LN% %REPARSEOPT% %TESTROOTSRCDRV%\tools %TESTROOTSRC%\F0\F0_J1

REM
REM Do a Delorean again
REM
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC%\F0 %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC%\F0 %TESTROOTBKP% %TESTROOTBKP2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%


@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%
@%RD% %TESTROOTSRCDRV%\Tools

REM
REM Dismount Truecrypt Drive
REM
%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit



REM
REM Directory attributes are set to Read-Only
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTBKP=%TESTROOT%\bkp
set TESTROOTBKP2=%TESTROOT%\bkp2

REM
REM Preparation
REM 
mkdir %TESTROOTSRC% > nul
mkdir %TESTROOTSRC%\F0 > nul
mkdir %TESTROOTSRC%\F0\F0_F0 > nul
copy test\resource.h %TESTROOTSRC%\F0\F0_F0\F0F0F0.txt > nul
mkdir %TESTROOTSRC%\F0\F1_F0 > nul
copy test\resource.h %TESTROOTSRC%\F0\F1_F0\F1F0F0.txt > nul
attrib +r %TESTROOTSRC%\F0\F1_F0 > nul

REM
REM Intial copy
REM 
@echo on
%LN% %ABS_REL% %OPTION% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM Remove the directory with read-only attribute
REM 
@attrib -r %TESTROOTSRC%\F0\F1_F0 > nul
@%RD% %TESTROOTSRC%\F0\F1_F0 > nul

REM
REM Do a Delorean Copy again
REM
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
%LN% %ABS_REL% %OPTION% --delorean %TESTROOTSRC% %TESTROOTBKP% %TESTROOTBKP2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM Make sure F0\F1_F0 is not in the Backup
REM 
%WHERE% *.* %TESTROOTBKP% | sort 

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%



@echo off
:ausmausraus
@echo on

@goto :EOF 
