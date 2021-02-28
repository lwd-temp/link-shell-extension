%RMTSHARE% \\%LH%\test=%~dp0test > nul

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
REM ERR: link file but have no write access to source
REM
@set ITEM=hardlink_no_write_access
@mkdir test\%ITEM% > nul
@copy test\ln.h test\%ITEM%\%ITEM% > nul
@type test\y | cacls test\%ITEM%\%ITEM% /D "%USERNAME%" > nul
%LN% test\%ITEM%\%ITEM% test\%ITEM%\%ITEM%_hardlink > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
type test\y | cacls test\%ITEM%\%ITEM% /G "%USERNAME%":F > nul
@%RD% test\%ITEM%

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
@if exist test\poi3 %RD% test\poi3
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
@if exist test\poi3 %RD% test\poi3
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
@if exist test\poi3 %RD% test\poi3
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
@%RD% test\poi3
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
@%RD% test\poi3 

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

REM SUC symbolic link tests
REM
%LN% --symbolic
@echo ErrorLevel == %errorlevel%

call t_HardSymLink.bat symbolic
call t_HardSymLink.bat 

REM
REM SUC: par1 has a trailing \
REM
%LN% --recursive test\Poi\ test\poi3 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%RD% test\poi3


REM
REM SUC: Destination is specified with no drive letter
REM
set DIR=%~p0\test\t
%MKDIR% %DIR%
%LN% --recursive  test\poi %DIR% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%RD% %DIR%

REM
REM SUC: --recursive on an empty directory
REM
@set SRC_DIR=test\poi\empty_src
@set DST_DIR=test\poi\empty_dst
@mkdir %SRC_DIR%
%LN% --recursive %SRC_DIR% %DST_DIR% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@ATTRIB %DST_DIR%
@%RD% %SRC_DIR%
@%RD% %DST_DIR%

REM
REM SUC: Destination is a path which does not exist
REM      so call mkdirhier to create the path to the file
REM
set BASE=test\not
set DIR=%BASE%\there\at\all
%LN% test\poi\history.txt %DIR%\history_link.txt
@echo ErrorLevel == %errorlevel%
@if exist %BASE% %RD% %BASE%


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
if exist test\enum\1 %RD% test\enum\1 > nul
if exist test\enum\2 %RD% test\enum\2 > nul
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
@if exist test\enum %RD% test\enum > nul

REM
REM SUCC: Copy Hardlink
REM
REM Preparation
REM
@echo off
if exist test\copy %RD% test\copy > nul
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
@if exist test\copy %RD% test\copy > nul

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
@if exist %TESTROOT% %RD% %TESTROOT% 

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
@if exist %TESTROOT% %RD% %TESTROOT% 

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
@if exist %TESTROOT% %RD% %TESTROOT% 

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
@if exist %TESTROOT% %RD% %TESTROOT% 

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

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOTSRC%\B_Test\AA_Test
@%RD% %TESTROOT%

del test\ln_link.h > nul
del test\ln_share_link.h > nul
del test\ln_share_unc_link.h > nul
@%RD% test\poi2 > nul
@%RD% test\poi_share > nul
@%RD% test\poi_share_share > nul
@%RD% test\poi_quiet > nul

%RMTSHARE% \\%LH%\test /delete > nul
@echo on
