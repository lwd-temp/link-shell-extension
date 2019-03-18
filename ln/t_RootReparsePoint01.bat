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
REM SUCC: Outer Reparse Points on rootdrive
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
%TRUECRYPT% test\NtfsTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit

mkdir %TESTROOTSRC%\PA_F1 > nul
mkdir %TESTROOTSRC%\PA_F3 > nul
copy test\resource.h "%TESTROOTSRC%\PA_F3\PA_F3.txt" > nul
copy test\resource.h "%TESTROOTSRC%\PA_F1\PA_F1.txt" > nul

mkdir %TESTROOTSRCDRV%\RD_F1 > nul
copy test\resource.h "%TESTROOTSRCDRV%\RD_F1\RD_F1.txt" > nul
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\RD_F1 %TESTROOTSRCDRV%\RD_J2 > nul

mkdir %TESTROOTSRCDRV%\RD_F3 > nul
copy test\resource.h "%TESTROOTSRCDRV%\RD_F3\RD_F3.txt" > nul

mkdir %TESTROOTSRCDRV%\RD_F3\RD_F3_F0 > nul
copy test\resource.h %TESTROOTSRCDRV%\RD_F3\RD_F3_F0\RD_F3_F0.txt > nul
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\RD_F3\RD_F3_F0 %TESTROOTSRCDRV%\RD_F3\RD_F3_J1 > nul

%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\ %TESTROOTSRC%\PA_J2 > nul
copy test\resource.h %TESTROOTSRCDRV%\RD.txt > nul


@if [%3] == [symbolic] ( 
  %LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\RD.txt %TESTROOTSRC%\PA_F1\PA_F1.syl > nul
)

@echo on
REM
REM Do the copy
REM
%LN% %OPTION% %ABS_REL_COPY% -X "System*" --copy %TESTROOTSRC% "%TESTROOTDST%"  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM 
REM Do a few checks
REM
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% %TESTROOTDST%\PA_J2
  @if [%3] == [symbolic] ( 
    @%LN% %REPARSEOPT% %TESTROOTDST%\PA_F1\PA_F1.syl
  )
) else (
@%LN% %REPARSEOPT% %TESTROOTDST%\PA_J2\RD_F3\RD_F3_J1
@%LN% %REPARSEOPT% %TESTROOTDST%\PA_J2\RD_J2
@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% %TESTROOTDST%\PA_F1\PA_F1.syl
)
)

@echo ErrorLevel == %errorlevel%
dir /ad /s /b %TESTROOTDST% | sort

@echo off
REM 
REM Clean up
REM
@if exist %TESTROOTSRCDRV%\RD_F1 %RD% %TESTROOTSRCDRV%\RD_F1
@if exist %TESTROOTSRCDRV%\RD_J2 %RD% %TESTROOTSRCDRV%\RD_J2
@if exist %TESTROOTSRCDRV%\RD_F3 %RD% %TESTROOTSRCDRV%\RD_F3
@del %TESTROOTSRCDRV%\RD.txt
@if exist %TESTROOT% %RD% %TESTROOT%
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit

@echo on
REM 
REM Double File ID Test
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%EMPTYTESTDRIVELETTER1%:\
set TESTROOTDST=%TESTROOT%\dst
%TRUECRYPT% test\lntest_fileid_src.tc /letter %EMPTYTESTDRIVELETTER1% /password lntest /quit
%TRUECRYPT% test\lntest_fileid_junctions.tc /letter %EMPTYTESTDRIVELETTER2% /password lntest /quit

@echo on
REM
REM Do it
REM
%LN% %OPTION% %ABS_REL_COPY% -X "System*" --copy %TESTROOTSRC% %TESTROOTDST%  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@if [%2] == [splice] ( 
  @%LN% --noitcnuj %TESTROOTDST%\JunctionToPTestjunction
)

@echo off
REM 
REM Clean up
REM
@if exist %TESTROOTDST% %RD% %TESTROOTDST%
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER1% /quit
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER2% /quit

REM 
REM Junction pointing to FAT Drive
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
%TRUECRYPT% test\FatTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit

copy test\resource.h %TESTROOTSRCDRV%\RD00.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\RD01.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\RD02.txt > nul

mkdir %TESTROOTSRCDRV%\F0
copy test\resource.h %TESTROOTSRCDRV%\F0\RD00.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\F0\RD01.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\F0\RD02.txt > nul

mkdir %TESTROOTSRCDRV%\F1
copy test\resource.h %TESTROOTSRCDRV%\F1\RD00.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\F1\RD01.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\F1\RD02.txt > nul

mkdir %TESTROOTSRC%
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\ %TESTROOTSRC%\J0 > nul
@echo on

REM
REM Do it
REM
@if not exist "%TESTROOTSRCDRV%\System Volume Information" @%MKDIR% "%TESTROOTSRCDRV%\System Volume Information"
%LN% %OPTION% %ABS_REL_COPY% -X "System*" --copy %TESTROOTSRC% "%TESTROOTDST%"  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% %TESTROOTDST%\J0
)

%WHERE% *.* %TESTROOTDST% | %GREP% -v "System" | %GREP% -v matching | sort

@echo off
REM 
REM Clean up
REM
@if exist %TESTROOTDST% %RD% %TESTROOTDST%
@if exist %TESTROOTSRC% %RD% %TESTROOTSRC%
@if exist %TESTROOTSRCDRV% %RD% %TESTROOTSRCDRV%
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit


REM 
REM Many junctions pointing to NTFS Drive
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTBKP=%TESTROOT%\bkp
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
%TRUECRYPT% test\NTFSTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit

copy test\resource.h %TESTROOTSRCDRV%\RD00.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\RD01.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\RD02.txt > nul

mkdir %TESTROOTSRCDRV%\F0
copy test\resource.h %TESTROOTSRCDRV%\F0\RD00.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\F0\RD01.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\F0\RD02.txt > nul
mkdir %TESTROOTSRCDRV%\F0\F0_F0
copy test\resource.h %TESTROOTSRCDRV%\F0\F0_F0\RD_F0_F0.txt > nul

mkdir %TESTROOTSRCDRV%\F1
copy test\resource.h %TESTROOTSRCDRV%\F1\RD00.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\F1\RD01.txt > nul
copy test\resource.h %TESTROOTSRCDRV%\F1\RD02.txt > nul
mkdir %TESTROOTSRCDRV%\F1\F1_F0
copy test\resource.h %TESTROOTSRCDRV%\F1\F1_F0\RD_F1_F0.txt > nul

mkdir %TESTROOTSRC%
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\F0 %TESTROOTSRC%\J0 > nul
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\F1 %TESTROOTSRC%\J1 > nul

@echo on
REM
REM Do it
REM
%LN% %OPTION% %ABS_REL_COPY% -X "System*" --copy %TESTROOTSRC% %TESTROOTDST%  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% %TESTROOTDST%\J0
  @%LN% %REPARSEOPT% %TESTROOTDST%\J1
)

%WHERE% *.* %TESTROOTDST% | sort

%LN% %OPTION% %ABS_REL_COPY% -X "System*" --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% %TESTROOTBKP%\J0
  @%LN% %REPARSEOPT% %TESTROOTBKP%\J1
)

%WHERE% *.* %TESTROOTBKP% | sort

@echo off
REM 
REM Clean up
REM
@if exist %TESTROOTBKP% %RD% %TESTROOTBKP%
@if exist %TESTROOTDST% %RD% %TESTROOTDST%
@if exist %TESTROOTSRC% %RD% %TESTROOTSRC%



REM 
REM Copy from root drive to root drive
REM 
set TESTROOTSRC=%EMPTYTESTDRIVELETTER1%:\
set TESTROOT=%1
set TESTROOTDST=%TESTROOT%\dst
set TESTROOTBKP=%TESTROOT%\bkp
%TRUECRYPT% test\NTFSTestDrive2.tc /letter %EMPTYTESTDRIVELETTER1% /password lntest /quit

REM Connect the two drives
REM
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\F0 %TESTROOTSRC%\J0 > nul
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\F1 %TESTROOTSRC%\J1 > nul
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\F0\F0_F0 %TESTROOTSRC%\J2 > nul
mkdir %TESTROOTSRC%\F0 > nul
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\F0\F0_F0 %TESTROOTSRC%\F0\F0_J2 > nul
mkdir %TESTROOTSRC%\F1 > nul
%LN% %ABS_REL% %REPARSEOPT% %TESTROOTSRCDRV%\F1\F1_F0 %TESTROOTSRC%\F1\F1_J2 > nul

mkdir %TESTROOTSRCDRV%\F0\F0_F0\F0_F0_F0
copy test\resource.h %TESTROOTSRCDRV%\F0\F0_F0\F0_F0_F0\F0_F0_F0.txt > nul

mkdir %TESTROOTSRCDRV%\F1\F1_F0\F1_F0_F0
copy test\resource.h %TESTROOTSRCDRV%\F1\F1_F0\F1_F0_F0\F1_F0_F0.txt > nul
@echo off

@echo on
REM
REM Do it
REM
%LN% %OPTION% %ABS_REL_COPY% -X "System*" --copy %TESTROOTSRC% %TESTROOTDST%  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% %TESTROOTDST%\J0
  @%LN% %REPARSEOPT% %TESTROOTDST%\J1
  @%LN% %REPARSEOPT% %TESTROOTDST%\F0\F0_J2
  @%LN% %REPARSEOPT% %TESTROOTDST%\F1\F1_J2
)

%WHERE% *.* %TESTROOTDST% | sort

%LN% %OPTION% %ABS_REL_COPY% -X "System*" --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] == [splice] ( 
  @%LN% %REPARSEOPT% %TESTROOTBKP%\J0
  @%LN% %REPARSEOPT% %TESTROOTBKP%\J1
  @%LN% %REPARSEOPT% %TESTROOTBKP%\F0\F0_J2
  @%LN% %REPARSEOPT% %TESTROOTBKP%\F1\F1_J2
)
%WHERE% *.* %TESTROOTBKP% | sort


@if exist %TESTROOTSRC%J0 %RD% %TESTROOTSRC%J0
@if exist %TESTROOTSRC%J1 %RD% %TESTROOTSRC%J1
@if exist %TESTROOTSRC%J2 %RD% %TESTROOTSRC%J2
@if exist %TESTROOTSRC%F0 %RD% %TESTROOTSRC%F0
@if exist %TESTROOTSRC%F1 %RD% %TESTROOTSRC%F1
@if exist %TESTROOTSRCDRV% %RD% %TESTROOTSRCDRV%
@if exist %TESTROOTBKP% %RD% %TESTROOTBKP%
@if exist %TESTROOTDST% %RD% %TESTROOTDST%
@if exist %TESTROOT% %RD% %TESTROOT%
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER1% /quit


@echo off
:ausmausraus
@echo on

@goto :EOF 
