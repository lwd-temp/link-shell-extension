call t_Settings.bat
net share test=%~dp0test

REM
REM SUC: link a file
REM
%LN% test\ln.h test\ln_link.h
@echo ErrorLevel == %errorlevel%

REM
REM SUC: link a file but source does not exist
REM
%LN% test\ln_.h test\ln_link.h
@echo ErrorLevel == %errorlevel%

REM
REM ERR: Destination already exists
REM
%LN% test\ln.h test\ln_link.h
@echo ErrorLevel == %errorlevel%

REM
REM ERR: Destinationfile missing
REM
%LN% test\ln.h
@echo ErrorLevel == %errorlevel%

REM
REM ERR: Source file not there
REM
%LN% test\ln_nothere.h test\ln_link.h
@echo ErrorLevel == %errorlevel%

REM
REM ERR: Call without parameters
REM
%LN% 
@echo ErrorLevel == %errorlevel%

REM
REM SUC: link via UNC names to filename
REM
%LN% \\%LH%\test\ln.h test\ln_share_link.h
@echo ErrorLevel == %errorlevel%

REM
REM SUC: link via UNC names to UNC name
REM
%LN% \\%LH%\test\ln.h \\%LH%\test\ln_share_unc_link.h
@echo ErrorLevel == %errorlevel%

REM
REM ERR: link via UNC names but 2nd arg lacks
REM
%LN% \\%LH%\test\ln.h 
@echo ErrorLevel == %errorlevel%

REM
REM ERR: recursivly link a dir, can not create dir
REM
@echo off
mkdir test\poi3
type test\y | cacls test\poi3 /D "%USERNAME%" > nul
@echo on
%LN% --recursive test\Poi test\poi3\poi3 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@echo off
type test\y | cacls test\poi3 /G "%USERNAME%":F > nul
if exist test\poi3 rd /s /q test\poi3
@echo on

REM
REM ERR: recursivly link a dir, can not create dir, but be quiet
REM
@echo off
mkdir test\poi3
type test\y | cacls test\poi3 /D "%USERNAME%" > nul
@echo on
%LN% --quiet --recursive test\Poi test\poi3\poi3 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@echo off
type test\y | cacls test\poi3 /G "%USERNAME%":F > nul
if exist test\poi3 rd /s /q test\poi3
@echo on

REM
REM ERR: recursivly link a dir, but source dir can not be read
REM
@echo off
type test\y | cacls test\poi /D "%USERNAME%" > nul
@echo on
%LN% --recursive test\Poi test\poi3 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@echo off
type test\y | cacls test\poi /G "%USERNAME%":F > nul
if exist test\poi3 rd /s /q test\poi3
@echo on

REM
REM ERR: recursivly link a dir, can not link in denied dir
REM
@echo off
mkdir test\poi3
type test\y | cacls test\poi3 /D "%USERNAME%" > nul
@echo on
%LN% --recursive test\Poi test\poi3 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@echo off
type test\y | cacls test\poi3 /G "%USERNAME%":F > nul
rd /s /q test\poi3
@echo on

REM
REM SUC: recursivly link a dir
REM
%LN% --recursive test\Poi test\poi2 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM ERR: recursivly link dir but 2nd arg lacks
REM
%LN% --recursive test\Poi
@echo ErrorLevel == %errorlevel%

REM
REM SUC: recursivly link dir to file dir
REM
%LN% --recursive \\%LH%\test\Poi test\poi_share > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM SUC: recursivly link dir to share dir
REM
%LN% --recursive \\%LH%\test\Poi \\%LH%\test\poi_share_share > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM ERR: recursivly link a dir but destdir exists and already has files
REM
%LN% --recursive test\Poi test\poi2 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM SUC: recursivly link a dir but destdir exists but dir is empty
REM
mkdir test\poi3
%LN% --recursive test\Poi test\poi3 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
rd /s /q test\poi3

REM
REM SUC: recursivly link a dir but dest is a file and exists
REM
%LN% --recursive test\Poi test\resource.h > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM ERR: recursivly link dir but first dir does not exist
REM
%LN% --recursive test\Poi_ test\Poi2 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM ERR: recursivly link dir but first arg is not a dir
REM
%LN% --recursive test\ln.h test\Poi2 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM SUC: recursivly link dir to file dir but be quiet
REM
%LN% --quiet --recursive \\%LH%\test\Poi test\poi_quiet > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM ERR: ommit a few parameters
REM
%LN% --recursive
@echo ErrorLevel == %errorlevel%

REM
REM SUC: par1 has a trailing \
REM
%LN% --recursive test\Poi\ test\poi3 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
rd /s /q test\poi3


REM
REM SUC: Destination is specified with no drive letter
REM
set DIR=%~p0\test\t
mkdir %DIR%
%LN% --recursive  test\poi %DIR% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
rd /s /q %DIR%

REM
REM SUC: Destination is a path which does not exist
REM      so call mkdirhier to create the path to the file
REM
set BASE=test\not
set DIR=%BASE%\there\at\all
%LN% test\poi\history.txt %DIR%\history_link.txt
@echo ErrorLevel == %errorlevel%
if exist %BASE% rd /s /q %BASE%


REM
REM TODO SUC: Remote Hardlink creation 
REM
REM ln \\ba\file1 \\bla\file2

REM
REM TODO ERR: Remote Hardlink creation 
REM
REM ln \\blu\file1 \\bla\file2

REM
REM TODO SUC: Remote Hardlink creation 
REM
REM ln --recursive \\ba\dir1 \\bla\dir2

REM
REM TODO ERR: Remote Hardlink creation 
REM
REM ln --recursive \\blu\file1 \\bla\file2

REM
REM TODO ERR: Check enumerate
REM
REM ln --enum \\blu\file1 

REM
REM SUCC: Enumerate only saturated hardlinks
REM
@echo off
if exist test\enum\1 rd /s /q test\enum\1 > nul
if exist test\enum\2 rd /s /q test\enum\2 > nul
mkdir test\enum\1 > nul
mkdir test\enum\2 > nul
copy test\ln.h test\enum\1\ln.h.0 > nul
%LN% test\enum\1\ln.h.0 test\enum\1\ln.h.1 > nul
%LN% test\enum\1\ln.h.0 test\enum\1\ln.h.2 > nul
%LN% test\enum\1\ln.h.0 test\enum\1\ln.h.3 > nul
%LN% test\enum\1\ln.h.0 test\enum\1\ln.h.4 > nul
%LN% test\enum\1\ln.h.0 test\enum\1\ln.h.5 > nul
%LN% test\enum\1\ln.h.0 test\enum\1\ln.h.6 > nul
copy test\ln.h test\enum\1\ln.h.c1 > nul
copy test\ln.h test\enum\1\ln.h.c2 > nul
echo on
%LN% --enum test\enum\1
@echo ErrorLevel == %errorlevel%

REM
REM SUCC: Now make the saturated group unsaturated
REM
@%LN% test\enum\1\ln.h.0 test\enum\2\ln.h.us1 > nul
%LN% --enum test\enum\1
@echo ErrorLevel == %errorlevel%

REM
REM SUCC: Show both now
REM
%LN% --enum test\enum
@echo ErrorLevel == %errorlevel%
if exist test\enum rd /s /q test\enum > nul

REM
REM SUCC: Copy Hardlink
REM
REM Preparation
REM
@echo off
if exist test\copy rd /s /q test\copy > nul
mkdir test\copy\source\Folder0 > nul
mkdir test\copy\source\Folder1 > nul
mkdir test\copy\source\Folder1\Folder2 > nul
mkdir test\copy\source\Folder1\Folder3 > nul
copy test\ln.h test\copy\source\Folder0\A > nul
%LN% test\copy\source\Folder0\A test\copy\source\Folder1\C > nul
%LN% test\copy\source\Folder0\A test\copy\source\Folder1\Folder2\D > nul

copy test\resource.h test\copy\source\Folder1\B > nul

copy test\y test\copy\source\Folder1\Folder2\E > nul
%LN% test\copy\source\Folder1\Folder2\E test\copy\source\Folder1\Folder3\F> nul
@echo on

%LN% --enum test\copy\source\Folder1
@echo ErrorLevel == %errorlevel%
%LN% --enum test\copy\source
@echo ErrorLevel == %errorlevel%

REM -----------------

REM
REM SUCC: Copy Hardlink
REM
%LN% --copy test\copy\source\Folder1 test\copy\dest\Folder1 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
%LN% --enum test\copy\dest
@echo ErrorLevel == %errorlevel%

REM 
REM Clean up
REM
if exist test\copy rd /s /q test\copy > nul

REM -----------------

REM
REM SUCC: ln -r but one file is read only
REM
set TESTROOT=test\hlc03
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest

mkdir %TESTROOTSRC%\Folder0 > nul
mkdir %TESTROOTSRC%\Folder0\Folder1 > nul
mkdir %TESTROOTDST% > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File A in Folder1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File B in Folder1" > nul
attrib +r "%TESTROOTSRC%\Folder0\Folder1\File A in Folder1"

%LN% --recursive %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
%WHERE% *.* %TESTROOTDST% | sort 

@REM 
@REM Clean up
@REM
@if exist %TESTROOT% (
  del %TESTROOT%\*.* /f /s /q > nul
  rd /s /q %TESTROOT% 
)

REM -----------------

REM
REM SUCC: ln -r but one file is read only and one is hidden
REM
set TESTROOT=test\hlc03
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest

mkdir %TESTROOTSRC%\Folder0 > nul
mkdir %TESTROOTSRC%\Folder0\Folder1 > nul
mkdir %TESTROOTDST% > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File A in Folder1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File B in Folder1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File C in Folder1" > nul
attrib +r "%TESTROOTSRC%\Folder0\Folder1\File A in Folder1"
attrib +h "%TESTROOTSRC%\Folder0\Folder1\File B in Folder1"

%LN% --recursive %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
%WHERE% *.* %TESTROOTDST% | sort 

@REM 
@REM Clean up
@REM
@if exist %TESTROOT% (
  del %TESTROOT%\*.* /f /s /q > nul
  rd /s /q %TESTROOT% 
)

REM -----------------

REM
REM SUCC: Check if attributes of directories are copied
REM
set TESTROOT=test\acp
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest

mkdir %TESTROOTSRC%\Folder0 > nul
mkdir %TESTROOTSRC%\Folder0\Folder1 > nul
mkdir %TESTROOTDST% > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File A in Folder1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File B in Folder1" > nul
attrib +r +h "%TESTROOTSRC%\Folder0\Folder1"
attrib +r "%TESTROOTSRC%\Folder0\Folder1\File A in Folder1"

%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
ATTRIB %TESTROOT%\*.* /S /D

@REM 
@REM Clean up
@REM
@if exist %TESTROOT% (
  del %TESTROOT%\*.* /f /s /q > nul
  rd /s /q %TESTROOT% 
)

REM -----------------

REM
REM SUCC: Check if attributes of directories are cloned
REM
set TESTROOT=test\acp
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest

mkdir %TESTROOTSRC%\Folder0 > nul
mkdir %TESTROOTSRC%\Folder0\Folder1 > nul
mkdir %TESTROOTDST% > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File A in Folder1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Folder1\File B in Folder1" > nul
attrib +r +h "%TESTROOTSRC%\Folder0\Folder1"
attrib +r "%TESTROOTSRC%\Folder0\Folder1\File A in Folder1"
attrib +r "%TESTROOTSRC%"

%LN% --recursive %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
ATTRIB %TESTROOT%\*.* /S /D

@REM 
@REM Clean up
@REM
@if exist %TESTROOT% (
  del %TESTROOT%\*.* /f /s /q > nul
  rd /s /q %TESTROOT% 
)

REM -----------------

REM
REM SUCC: Check Copy Junction
REM
call t_SmartCopy02Preperation.bat test\junction

REM
REM Do the test
REM
%LN% --copy %TESTROOTSRC% %TESTROOTDST%
@echo ErrorLevel == %errorlevel%
%WHERE% *.* %TESTROOT% | sort

REM 
REM Clean up
REM
if exist %TESTROOTSRC%\B_Test\AA_Test rd %TESTROOTSRC%\B_Test\AA_Test
@%RD% %TESTROOT%

REM
REM Smart Copy Test
REM
call t_SmartCopy03.bat test\symlink copy noitcnuj relative

REM
REM Smart Clone Test
REM
call t_SmartCopy03.bat test\smartclone recursive noitcnuj relative

REM -----------------

REM
REM FAIL: Check Move Rename Operations if Destination already there
REM
set TESTROOT=test\move01
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest

mkdir %TESTROOTSRC% > nul
mkdir %TESTROOTDST% > nul

%LN% --move %TESTROOTSRC% %TESTROOTDST%
@echo ErrorLevel == %errorlevel%

@REM 
@REM Clean up
@REM
@if exist %TESTROOT% (
	del %TESTROOT%\*.* /f /s /q > nul
	rd /s /q %TESTROOT% 
)

REM -----------------


REM
REM SUCC: Check Move Rename Operations 
REM
call t_SmartMove02.bat test\SmartMove move noitcnuj relative

call t_DeepPathOn.bat

call t_SmartMove02.bat test\SmartMove move noitcnuj relative

call t_DeepPathOff.bat

REM
REM SUCC: Check Move Rename Operations 
REM
call t_SmartMove02.bat test\SmartRename smartrename noitcnuj relative

REM
REM SUCC: Smartcopy just one folder and a file
REM
@set TESTROOT=test\smartcopy
@set TESTROOTSRC=%TESTROOT%\source
@set TESTROOTDST=%TESTROOT%\dest

@mkdir %TESTROOTSRC%\Folder0 > nul
@mkdir %TESTROOTDST% > nul
@copy test\ln.h "%TESTROOTSRC%\Folder0\File A in Folder0" > nul

%LN% --copy %TESTROOTSRC%\Folder0 %TESTROOTDST%\Folder0
@echo ErrorLevel == %errorlevel%
%WHERE%  *. %TESTROOTDST%  | sort

REM 
REM Clean up
REM
@%RD% %TESTROOT%

REM -----------------

REM
REM SUCC: Copy from the root
REM
@echo off
set TESTROOT=%~d0
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest

mkdir %TESTROOTSRC%\Folder0 > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\File A in Folder0" > nul
%LN% --junction "%TESTROOTSRC%\junc" "%TESTROOTSRC%\Folder0" > nul
@echo on

%LN% --copy %TESTROOTSRC% %TESTROOTDST%\Folder0
@echo ErrorLevel == %errorlevel%
%WHERE% *.* %TESTROOTDST% | sort

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOTSRC%
@%RD% %TESTROOTDST%
@echo on

REM -----------------

REM
REM SUCC: Delorean from the root
REM
@echo off
set TESTROOT=%~d0
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest
set TESTROOTBKP=%TESTROOT%\bkp

mkdir %TESTROOTSRC%\Folder0 > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\File A in Folder0" > nul
%LN% --junction "%TESTROOTSRC%\junc" "%TESTROOTSRC%\Folder0" > nul
@echo on

%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

%LN% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

%WHERE% *.* %TESTROOTBKP% | sort

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOTSRC%
@%RD% %TESTROOTDST%
if exist %TESTROOTBKP% rd /s /q %TESTROOTBKP%
@echo on

REM -----------------

REM
REM SUCC: copy from root 2
REM
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
%LN% -X "System*" --copy %TESTROOTSRC% "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

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

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOTSRC%SRC
@%RD% %TESTROOTDST%
%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit
@echo on

REM
REM Check DeepPathCopy 
REM
call t_DeepPathOn.bat

call t_SmartCopy03.bat test\DeepPath copy noitcnuj relative


REM
REM Check DeepPathClone
REM
call t_SmartCopy03.bat test\SmartClone recursive noitcnuj relative
call t_DeepPathOff.bat


REM -----------------

REM
REM Create Symbolic links to directories and files
REM
@echo off
set TESTROOT=test\symlink
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest

mkdir %TESTROOTSRC%\Folder0 > nul
mkdir "%TESTROOTSRC%\Folder1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\File A in Folder0" > nul

@echo on
REM SUCC:
%LN% -s "%TESTROOTSRC%\Folder0\File A in Folder0" "%TESTROOTDST%\Folder0\File A in Folder0"

@echo ErrorLevel == %errorlevel%
%WHERE% *.* %TESTROOTDST% | sort
%LN% -s "%TESTROOTDST%\Folder0\File A in Folder0"
if exist "%TESTROOTDST%\Folder0\File A in Folder0" del /q "%TESTROOTDST%\Folder0\File A in Folder0"

REM SUCC:
%LN% -a -s "%TESTROOTSRC%\Folder0\File A in Folder0" "%TESTROOTDST%\Folder0\File A in Folder0"
@echo ErrorLevel == %errorlevel%
%WHERE% *.* %TESTROOTDST% | sort
%LN% -s "%TESTROOTDST%\Folder0\File A in Folder0"

REM FAIL:
%LN% -s "%TESTROOTSRC%\Folder0\File A in Folder___0" "%TESTROOTDST%\Folder0\File A in Folder0"
@echo ErrorLevel == %errorlevel%

REM FAIL:
%LN% -s %TESTROOTSRC%\Folder0 %TESTROOTDST%\Folder0
@echo ErrorLevel == %errorlevel%
%WHERE% *.* %TESTROOTDST% | sort

REM SUCC:
%LN% -s %TESTROOTSRC%\Folder1 %TESTROOTDST%\Folder1
@echo ErrorLevel == %errorlevel%
%WHERE% *.* %TESTROOTDST% | sort
%LN% -s %TESTROOTDST%\Folder1
@%RD% %TESTROOTDST%\Folder1

REM SUCC:
%LN% -a -s %TESTROOTSRC%\Folder1 %TESTROOTDST%\Folder1
@echo ErrorLevel == %errorlevel%
%WHERE%  *.* %TESTROOTDST% | sort
%LN% -s %TESTROOTDST%\Folder1

REM FAIL:
%LN% -s %TESTROOTSRC%\Folder___0 %TESTROOTDST%\Folder0
@echo ErrorLevel == %errorlevel%

REM 
REM Clean up
REM
@%RD% %TESTROOTDST%\Folder0
@%RD% %TESTROOT%


REM
REM TBD SUCC: Create a test where there are only directories or junction
REM

REM
REM Timemachine
REM
call t_Delorean01.bat test\tm none symbolic none absolute

REM
REM Regular Expression
REM
@echo off
call t_RegExp01.bat

REM
REM Unroll
REM
@echo off
call t_UnrollSplice01.bat test\OuterJunctionUnroll unroll

call t_UnrollSplice02.bat test\OuterJunctionUnroll unroll noitcnuj relative

call t_Delorean02.bat test\DeloreanUnroll unroll noitcnuj relative

call t_MultiSource01.bat test\MultiSourceUnroll unroll noitcnuj relative

REM
REM DeepPath Unroll
REM
call t_DeepPathOn.bat

call t_UnrollSplice02.bat test\DeepOuterJunctionUnroll unroll noitcnuj relative

call t_Delorean02.bat test\DeepDeloreanUnroll unroll noitcnuj relative

call t_MultiSource01.bat test\MultiSourceUnroll unroll noitcnuj relative

call t_DeepPathOff.bat

REM
REM Splice
REM
@echo off
call t_UnrollSplice01.bat test\OuterJunctionSplice splice

call t_UnrollSplice02.bat test\OuterJunctionSplice splice noitcnuj relative

call t_Delorean02.bat test\DeloreanSplice splice noitcnuj relative

call t_MultiSource01.bat test\MultiSourceSplice splice noitcnuj relative

REM
REM DeepPath Splice
REM
call t_DeepPathOn.bat

call t_UnrollSplice02.bat test\DeepOuterJunctionSplice splice noitcnuj relative

call t_Delorean02.bat test\DeepDeloreanSplice splice noitcnuj relative

call t_MultiSource01.bat test\DeepMultiSourceSplice splice noitcnuj relative

call t_DeepPathOff.bat


REM
REM SmartMirror
REM
call t_SmartMirror01.bat test\mirror none symbolic none absolute

REM
REM ReparsePoints on RootDrive
REM
@echo off
call t_RootReparsePoint01.bat test\RootReparse unroll noitcnuj relative

call t_RootReparsePoint01.bat test\RootReparse splice noitcnuj relative

REM
REM Circularities
REM
call t_FatSmartXXX.bat test\FatSmartXXX 

REM
REM Circularities
REM
call t_Circularity01.bat test\Circularity none noitcnuj relative
call t_Circularity01.bat test\Circularity unroll noitcnuj relative

REM
REM Delorean with exclude
REM
call t_DeloreanExclude.bat

REM
REM Hardlink tests
REM
call t_Hardlink01.bat test\hardlink copy
call t_Hardlink01.bat test\hardlink mirror
call t_Hardlink01.bat test\hardlink recursive


REM
REM Case sensitive tests
REM
call t_CaseSensitiv.bat test\CaseSensitiv unroll noitcnuj

REM
REM Absolute relative tests
REM
call t_AbsoluteRelativ.bat test\Absrel none noitcnuj none


REM
REM Reparse points on reparse points
REM
call t_ReparseOnReparse01.bat test\ReparseOnReparse none noitcnuj relative

REM
REM Show Hardlink Sibblings
REM
call t_Siblings.bat test\sibblings 
call t_Siblings.bat test\sibblings traditional
call t_DeepPathOn.bat
call t_Siblings.bat test\sibblings 
call t_Siblings.bat test\sibblings traditional
call t_DeepPathOff.bat

REM
REM SUCC: Command line tests
REM
REM mkdir c:\1
REM %LN%  -e c:\1\
REM %LN%  -e "c:\1"
REM %LN%  -e c:\1
REM %LN%  -e "c:\1\"

:ausmausraus

@echo off
del test\ln_link.h > nul
del test\ln_share_link.h > nul
del test\ln_share_unc_link.h > nul
@%RD% test\poi2 > nul
@%RD% test\poi_share > nul
@%RD% rd /s /q test\poi_share_share > nul
@%RD% test\poi_quiet > nul

net share /delete test > nul
:nodel
@echo on

