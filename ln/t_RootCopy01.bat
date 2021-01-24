REM
REM 1: directory

@echo off
set TESTROOT=%EMPTYTESTDRIVE%\
set TESTROOTSRC=%TESTROOT%
set TESTROOTDST=%TESTROOT%dest
%TRUECRYPT% test\NtfsTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit

mkdir %TESTROOTSRC%\1 > nul
mkdir %TESTROOTSRC%\2 > nul
copy test\ln.h "%TESTROOTSRC%1" > nul
copy test\ln.h "%TESTROOTSRC%2" > nul

@echo on
REM 
REM Copy root directory and check attributes
REM 
%LN% -X "System*" --copy %TESTROOTSRC% "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
ATTRIB %TESTROOTDST%
%WHERE% *.* %TESTROOTDST% | sort

REM 
REM Mirror root directory and check attributes
REM 
@%RD% %TESTROOTDST%
%LN% -X "System*" --mirror %TESTROOTSRC% "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
ATTRIB %TESTROOTDST%
%WHERE% *.* %TESTROOTDST% | sort

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOTSRC%1
@%RD% %TESTROOTSRC%2
@%RD% %TESTROOTDST%
@echo on

REM -----------------

REM
REM SUCC: copy from root 3
REM
@echo off
set TESTROOT=%EMPTYTESTDRIVE%\
set TESTROOTSRC=%TESTROOT%
set TESTROOTDST=%~d0\1

mkdir %TESTROOTSRC%SRC\1 > nul
mkdir %TESTROOTSRC%SRC\3 > nul
copy test\resource.h "%TESTROOTSRC%SRC\1" > nul
copy test\y  "%TESTROOTSRC%SRC\1" > nul

%LN% --copy %TESTROOTSRC% "%TESTROOTDST%\dst" > nul
%LN% --junction "%TESTROOTSRC%SRC\3 - Junction" %TESTROOTSRC%SRC\3 > nul
@echo on

%LN% --delorean %TESTROOTSRC% "%TESTROOTDST%\dst" "%TESTROOTDST%\dst2" > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

dir /b /s %TESTROOTSRC%  | sort
dir /b /s %TESTROOTDST%  | sort
%LN% --junction "%TESTROOTDST%\dst2\SRC\3 - Junction"

@%RD% %TESTROOTSRC%SRC
@%RD% %TESTROOTDST%

REM
REM Test --anchor with varying drive: t_RootCopy01_anchor.png
REM
@mkdir %TESTROOTSRC%F0 > nul
@mkdir %TESTROOTSRC%F0\F0_F0 > nul
@%LN% --noitcnuj %TESTROOTSRC%F0\F0_F0 %TESTROOTSRC%F0\F0_J0 > nul
@%LN% --absolute --symbolic %TESTROOTSRC%F0\F0_F0 %TESTROOTSRC%F0\F0_S0 > nul
@%LN% --symbolic %TESTROOTSRC%F0\F0_F0 %TESTROOTSRC%F0\F0_S1_R > nul
@copy test\resource.h %TESTROOTSRC%F0\F0.txt > nul

@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit

@%TRUECRYPT% test\NtfsTestDrive.tc /letter %EMPTYTESTDRIVELETTER1% /password lntest /quit
@set TESTROOT=%EMPTYTESTDRIVE1%\
@set TESTROOTSRC=%TESTROOT%
@set TESTROOTDST=%1

REM Use normal path for --anchor
REM
%LN% --excludedir "System*" --keepsymlinkrelation --anchor %EMPTYTESTDRIVE%\F0 --copy %TESTROOTSRC%F0 %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%LN% --noitcnuj %TESTROOTDST%\F0_J0
@%LN% --symbolic %TESTROOTDST%\F0_S0
@%LN% --symbolic %TESTROOTDST%\F0_S1_R

@%RD% %TESTROOTDST%

REM Use root path for --anchor
REM
%LN% --excludedir "System*" --keepsymlinkrelation --anchor %EMPTYTESTDRIVE%\ --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%LN% --noitcnuj %TESTROOTDST%\F0\F0_J0
@%LN% --symbolic %TESTROOTDST%\F0\F0_S0
@%LN% --symbolic %TESTROOTDST%\F0\F0_S1_R

REM Copy back for --anchor
REM
@%RD% %TESTROOTSRC%F0
%LN% --keepsymlinkrelation --anchor %TESTROOTDST% --copy %TESTROOTDST% %TESTROOTSRC% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%LN% --noitcnuj %TESTROOTSRC%F0\F0_J0
@%LN% --symbolic %TESTROOTSRC%F0\F0_S0
@%LN% --symbolic %TESTROOTSRC%F0\F0_S1_R

@REM Cleanup
@REM
@%RD% %TESTROOTDST%
@%RD% %TESTROOTSRC%F0

REM
REM Test --anchor and dangling junctions: t_RootCopy01_anchor_dangling.png
REM
@mkdir %TESTROOTSRC%F0 > nul
@mkdir %TESTROOTDST%\F012 > nul
@mkdir %TESTROOTDST%\F012\F2345 > nul

@%LN% --noitcnuj %TESTROOTDST%\F012\F2345 %TESTROOTSRC%F0\F012_F2345_J0 > nul
@%LN% --absolute --symbolic %TESTROOTDST%\F012\F2345 %TESTROOTSRC%F0\F012_F2345_S0 > nul
@%LN% --symbolic %TESTROOTDST%\F012\F2345 %TESTROOTSRC%F0\F012_F2345_S1_R > nul

@REM Copy --anchor && --backup to create dangling junctions
@REM
%LN% --excludedir "System*" --backup --anchor %TESTROOTDST% --copy %TESTROOTSRC% %TESTROOTDST%\F_DST > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%LN% --noitcnuj %TESTROOTSRC%F0\F012_F2345_J0
@%LN% --symbolic %TESTROOTSRC%F0\F012_F2345_S0
@%LN% --symbolic %TESTROOTSRC%F0\F012_F2345_S1_R

@%LN% --noitcnuj %TESTROOTDST%\F_DST\F0\F012_F2345_J0
@%LN% --symbolic %TESTROOTDST%\F_DST\F0\F012_F2345_S0
@%LN% --symbolic %TESTROOTDST%\F_DST\F0\F012_F2345_S1_R

@REM Cleanup
@REM
@%RD% %TESTROOTDST%
@%RD% %TESTROOTSRC%F0



@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER1% /quit

REM 
REM --mirror must not delete the destination when used with --anchor
REM
@set TESTROOT=%1
@set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
@set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest
@set TESTROOTBKP=%TESTROOT%%DEEPPATH%\backup
@%MKDIR% %TESTROOTSRC% > nul
@%MKDIR% %TESTROOTSRC%\a > nul
@%MKDIR% %TESTROOTSRC%\b > nul
@%MKDIR% %TESTROOTDST%\a > nul
@%MKDIR% %TESTROOTDST%\b > nul

%LN% --anchor %~d0\ --mirror %TESTROOTSRC% %TESTROOTDST%  > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
dir /b %TESTROOTDST%

REM 
REM --delorean must not delete the destination when used with --anchor
REM
%LN% --anchor %~d0\ --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP%  > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
dir /b %TESTROOTDST%
dir /b %TESTROOTBKP%

@REM 
@REM Clean up
@REM
@%RD% %TESTROOT% > nul


@echo off
REM 
REM Clean up
REM
@echo on

