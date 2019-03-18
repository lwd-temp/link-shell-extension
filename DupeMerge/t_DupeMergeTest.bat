call t_Settings.bat
set TESTBASE=test

REM Der Witz hier ist, dass alle files gleich gross sind
REM aber ein paar nach 0x8000 unterschiedlich werden
REM und somit eine neue Gruppe bilden. Das muss erkannt werden
@echo off

set TESTROOT=%TESTBASE%\general00
set TESTROOTSRC=%TESTBASE%\6.base
set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

mkdir %TESTROOTDST% > nul
copy %TESTROOTSRC%\0000 %TESTROOTDST%  > nul
copy %TESTROOTSRC%\0000 %TESTROOTDST%\0001 > nul
copy %TESTROOTSRC%\0000 %TESTROOTDST%\0002 > nul
copy %TESTROOTSRC%\0000 %TESTROOTDST%\0003 > nul

copy %TESTROOTSRC%\0010 %TESTROOTDST% > nul
copy %TESTROOTSRC%\0010 %TESTROOTDST%\0011 > nul
copy %TESTROOTSRC%\0010 %TESTROOTDST%\0012 > nul
copy %TESTROOTSRC%\0010 %TESTROOTDST%\0013 > nul
copy %TESTROOTSRC%\0010 %TESTROOTDST%\0014 > nul


@echo on
%DUPEMERGE% --list %TESTROOTDST%
%DUPEMERGE% %TESTROOTDST%

%LN% --enum %TESTROOTDST% | sort 
@echo off

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%
@echo on

REM
REM normale dupegroups
REM
@echo off
set TESTROOT=%TESTBASE%\general00
set TESTROOTSRC=%TESTBASE%\7.base
set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

@echo off
mkdir %TESTROOTDST% > nul
copy %TESTROOTSRC%\0000 %TESTROOTDST%  > nul
%LN% %TESTROOTDST%\0000 %TESTROOTDST%\0010 > nul
%LN% %TESTROOTDST%\0000 %TESTROOTDST%\0020 > nul
%LN% %TESTROOTDST%\0000 %TESTROOTDST%\0030 > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST%\0100 > nul
%LN% %TESTROOTDST%\0100 %TESTROOTDST%\0110 > nul
%LN% %TESTROOTDST%\0100 %TESTROOTDST%\0120 > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST%\0200 > nul
%LN% %TESTROOTDST%\0200 %TESTROOTDST%\0210 > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST%\0300 > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST%\0400 > nul


copy %TESTROOTSRC%\1000 %TESTROOTDST% > nul
copy %TESTROOTSRC%\1000 %TESTROOTDST%\1100 > nul


copy %TESTROOTSRC%\2000 %TESTROOTDST%\2000 > nul
%LN% %TESTROOTDST%\2000 %TESTROOTDST%\2010 > nul
%LN% %TESTROOTDST%\2000 %TESTROOTDST%\2020 > nul
%LN% %TESTROOTDST%\2000 %TESTROOTDST%\2030 > nul


copy %TESTROOTSRC%\3000 %TESTROOTDST% > nul


copy %TESTROOTSRC%\5000 %TESTROOTDST%\5000 > nul
%LN% %TESTROOTDST%\5000 %TESTROOTDST%\5010 > nul
%LN% %TESTROOTDST%\5000 %TESTROOTDST%\5020 > nul
%LN% %TESTROOTDST%\5000 %TESTROOTDST%\5030 > nul

copy %TESTROOTSRC%\5000 %TESTROOTDST%\5100 > nul


copy %TESTROOTSRC%\6000 %TESTROOTDST% > nul


copy %TESTROOTSRC%\7000 %TESTROOTDST% > nul


copy %TESTROOTSRC%\8000 %TESTROOTDST% > nul
copy %TESTROOTSRC%\8000 %TESTROOTDST%\8100 > nul

@echo on
%DUPEMERGE% --list %TESTROOTDST%
%DUPEMERGE% %TESTROOTDST%

%LN% --enum %TESTROOTDST% | sort 
@echo off

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%
@echo on


REM
REM
REM
@echo off
set TESTROOT=%TESTBASE%\general00
set TESTROOTSRC=%TESTBASE%\8.base
set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

mkdir %TESTROOTDST% > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.exe > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d100.exe > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d200.exe > nul
%LN% %TESTROOTDST%\d000.exe %TESTROOTDST%\d010.exe > nul
copy %TESTROOTSRC%\d001.aa %TESTROOTDST%\d001.exe > nul
copy %TESTROOTSRC%\d001.aa %TESTROOTDST%\d101.exe > nul
%LN% %TESTROOTDST%\d001.exe %TESTROOTDST%\d011.exe > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST% > nul
%LN% %TESTROOTDST%\0000 %TESTROOTDST%\0010 > nul
%LN% %TESTROOTDST%\0000 %TESTROOTDST%\0020 > nul
%LN% %TESTROOTDST%\0000 %TESTROOTDST%\0030 > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST%\0100 > nul
%LN% %TESTROOTDST%\0100 %TESTROOTDST%\0110 > nul
%LN% %TESTROOTDST%\0100 %TESTROOTDST%\0120 > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST%\0200 > nul
%LN% %TESTROOTDST%\0200 %TESTROOTDST%\0210 > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST%\0300 > nul

copy %TESTROOTSRC%\0000 %TESTROOTDST%\0400 > nul

@echo on
%DUPEMERGE% --list %TESTROOTDST%
%DUPEMERGE% %TESTROOTDST%
@echo off

%LN% --enum %TESTROOTDST% | sort 

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%



@echo on
REM
REM Read Only files
REM
@echo off
set TESTROOT=%TESTBASE%\general00
set TESTROOTSRC=%TESTBASE%\8.base
set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

mkdir %TESTROOTDST% > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.exe > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d100.exe > nul

attrib +r %TESTROOTDST%\d000.exe
attrib +r %TESTROOTDST%\d100.exe


@echo on
%DUPEMERGE% --list %TESTROOTDST%
%DUPEMERGE% %TESTROOTDST%
@echo off

%LN% --enum %TESTROOTDST% | sort 

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

@echo on
REM
REM access file for read
REM
@echo off
set TESTROOT=%TESTBASE%\general00
set TESTROOTSRC=%TESTBASE%\8.base
set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%


mkdir %TESTROOTDST% > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.exe > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d100.exe > nul

type test\y | cacls %TESTROOTDST%\d100.exe /D "%USERNAME%" > nul

@echo on
%DUPEMERGE% --list %TESTROOTDST%
%DUPEMERGE% %TESTROOTDST%
@echo off

type test\y | cacls %TESTROOTDST%\d100.exe /G "%USERNAME%":F > nul

%LN% --enum %TESTROOTDST% | sort 

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%
@echo on

REM
REM Can not create dupegroup because file is locked
REM
@echo off
set TESTROOT=%TESTBASE%\general00
set TESTROOTSRC=%TESTBASE%\8.base
set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

mkdir %TESTROOTDST% > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.exe > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d100.exe > nul

type test\y | cacls %TESTROOTDST%\d100.exe /D "%USERNAME%" > nul
cacls %TESTROOTDST%\d100.exe /E /P "%USERNAME%":R > nul
type test\y | cacls %TESTROOTDST% /D "%USERNAME%" > nul
cacls %TESTROOTDST% /E /P "%USERNAME%":R > nul

@echo on
%DUPEMERGE% --list %TESTROOTDST%
%DUPEMERGE% %TESTROOTDST%
@echo off

type test\y | cacls %TESTROOTDST% /G "%USERNAME%":F > nul
type test\y | cacls %TESTROOTDST%\d100.exe /G "%USERNAME%":F > nul

%LN% --enum %TESTROOTDST% | sort 

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%
@echo on


REM
REM --exclude
REM
@set TESTROOT=%TESTBASE%\general00
@set TESTROOTSRC=%TESTBASE%\8.base
@set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

@mkdir %TESTROOTDST% > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.exe > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d200.exe > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d100.dat > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.dat > nul

%DUPEMERGE% --list --exclude *.dat %TESTROOTDST%
@echo *.dat> ExList.txt
%DUPEMERGE% --list --exclude @ExList.txt %TESTROOTDST%
@del ExList.txt
%DUPEMERGE% --exclude *.dat %TESTROOTDST% 

@%LN% --enum %TESTROOTDST% | sort 

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

REM
REM --excludedir
REM
@set TESTROOT=%TESTBASE%\general00
@set TESTROOTSRC=%TESTBASE%\8.base
@set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

@mkdir %TESTROOTDST% > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.exe > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d200.exe > nul
@mkdir %TESTROOTDST%\ExcludeDir > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\ExcludeDir\d100.dat > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\ExcludeDir\d000.dat > nul

%DUPEMERGE% --list --excludedir ExcludeDir %TESTROOTDST%
@echo ExcludeDir> ExList.txt
%DUPEMERGE% --list --excludedir @ExList.txt %TESTROOTDST%
@del ExList.txt

@mkdir %TESTROOTDST%\IncludeDir > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\IncludeDir\d100.dat > nul
@copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\IncludeDir\d000.dat > nul
@echo IncludeDir> IncList.txt

%DUPEMERGE% --list --includedir IncludeDir %TESTROOTDST%
%DUPEMERGE% --list --includedir @IncList.txt %TESTROOTDST%
@del IncList.txt
%DUPEMERGE% --excludedir ExcludeDir --excludedir IncludeDir %TESTROOTDST% 

@%LN% --enum %TESTROOTDST% | sort 

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%
@echo on


REM
REM --wildcard
REM
@echo off
set TESTROOT=%TESTBASE%\general00
set TESTROOTSRC=%TESTBASE%\8.base
set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%

@echo off

mkdir %TESTROOTDST% > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.exe > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d200.exe > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d100.dat > nul
copy %TESTROOTSRC%\d000.aa %TESTROOTDST%\d000.dat > nul
mkdir %TESTROOTDST%\emtpy
mkdir %TESTROOTDST%\emtpy1
mkdir %TESTROOTDST%\emtpy1\empty1

@echo on
%DUPEMERGE% --list --wildcard *.dat %TESTROOTDST%
@echo *.dat> IncList.txt
%DUPEMERGE% --list --wildcard @IncList.txt %TESTROOTDST%
@del IncList.txt
%DUPEMERGE% --wildcard *.dat %TESTROOTDST% 
@echo off


%LN% --enum %TESTROOTDST% | sort 

@if exist %TESTROOTDST% rmdir /Q /S %TESTROOTDST%
@echo on


REM
REM Create a file, which has 1023 hardlinks, so that during DupeMerge
REM the error TOO_MANY_LINKS comes up
REM
@echo off
set TESTROOT=%TESTBASE%\TooManyLinks
set TESTROOTSRC=%TESTBASE%\6.base
set TESTROOTDST=%TESTROOT%\dest

mkdir "%TESTROOT%" > nul
copy "%TESTROOTSRC%\0000" "%TESTROOT%\ttt0" > nul
for /L %%a in (1,1,1010) do %LN% "%TESTROOT%\ttt0" "%TESTROOT%\ttt%%a" > nul
copy "%TESTROOTSRC%\0000" "%TESTROOT%\ddd1" > nul
for /L %%a in (1,1,50) do %LN% "%TESTROOT%\ddd1" "%TESTROOT%\ddd%%a" > nul
copy "%TESTROOTSRC%\0000" "%TESTROOT%\eee1" > nul
for /L %%a in (1,1,50) do %LN% "%TESTROOT%\eee1" "%TESTROOT%\eee%%a" > nul
@echo on
%DUPEMERGE% %TESTROOT%

%LN% --enum %TESTROOT% | sort 

@if exist %TESTROOT% rmdir /Q /S %TESTROOT%
@echo on


REM
REM Create two files and check if after the merge the older timestamp survives
REM
@echo off
set TESTROOT=%TESTBASE%\OldTimeStamp
set TESTROOTSRC=%TESTBASE%\6.base
set TESTROOTDST=%TESTROOT%\dest

mkdir "%TESTROOT%" > nul
copy "%TESTROOTSRC%\0000" "%TESTROOT%\ttt0" > nul
copy "%TESTROOTSRC%\0000" "%TESTROOT%\ttt1" > nul
@%TIMESTAMP% --writetime --accesstime --creationtime --ctime 4e1402f1 "%TESTROOT%\ttt0" > nul
@%TIMESTAMP% --writetime --accesstime --creationtime --ctime 4e1202f1 "%TESTROOT%\ttt1" > nul
@echo on
%DUPEMERGE% %TESTROOT%
@%TIMESTAMP% "%TESTROOT%\ttt0"
@%TIMESTAMP% "%TESTROOT%\ttt1"

@if exist %TESTROOT% rmdir /Q /S %TESTROOT%
@echo on




:ausmausraus