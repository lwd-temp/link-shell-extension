@echo off

REM
REM Mount Fat Drive
REM
set TESTROOT=%1
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTBKP=%TESTROOT%\bkp
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
%TRUECRYPT% test\FatTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit
%MKDIR% %TESTROOTSRCDRV%\F00 > nul
%COPY% test\resource.h %TESTROOTSRCDRV%\F00 > nul

@echo on
REM
REM SmartCopy from FAT drive
REM
@%RD% %TESTROOTDST%
%LN% --copy %TESTROOTSRCDRV%\F00 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM SmartMirror from FAT drive
REM
@%RD% %TESTROOTDST%
@echo on
%LN% --mirror %TESTROOTSRCDRV%\F00 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM SmartClone from FAT drive
REM
@echo on
%LN% --recursive %TESTROOTSRCDRV%\F00 %TESTROOTDST%

REM
REM Delorean from FAT drive
REM
@echo on
%LN% --delorean %TESTROOTSRCDRV%\F00 %TESTROOTDST% %TESTROOTBKP%> sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@echo.
@%LN% -s %TESTROOTBKP%\resource.h

:ausmausraus
@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%
@%RD% %TESTROOTSRCDRV%\F00
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit

