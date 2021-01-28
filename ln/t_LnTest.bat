call t_Settings.bat

call t_IpAdress.bat
for /f %%f in ('type .ipadress') do set IP_ADRESS=%%f

call t_BaseTest.bat

REM
REM Smart Copy Test
REM
call t_SmartCopy03.bat test\symlink copy noitcnuj relative
call t_SmartCopy03.bat test\symlink copy symbolic relative absolute
call t_SmartCopy03.bat test\symlink copy symbolic absolute absolute
call t_SmartCopy03.bat test\symlink copy symbolic relative
call t_SmartCopy03.bat test\symlink copy symbolic absolute

REM
REM Smart Clone Test
REM
call t_SmartCopy03.bat test\smartclone recursive noitcnuj relative
call t_SmartCopy03.bat test\smartclone recursive symbolic relative absolute
call t_SmartCopy03.bat test\smartclone recursive symbolic relative
call t_SmartCopy03.bat test\smartclone recursive symbolic absolute
call t_SmartCopy03.bat test\smartclone recursive symbolic absolute absolute

REM
REM Smart Copy Test Multiple Destinations
REM
call t_SmartCopy04.bat test\MultiDest copy noitcnuj relative
call t_SmartCopy04.bat test\MultiDest copy symbolic relative absolute
call t_SmartCopy04.bat test\MultiDest copy symbolic absolute absolute
call t_SmartCopy04.bat test\MultiDest copy symbolic relative
call t_SmartCopy04.bat test\MultiDest copy symbolic absolute

REM
REM Smart Clone Test Multiple Destinations
REM
call t_SmartCopy04.bat test\MultiDest recursive noitcnuj relative
call t_SmartCopy04.bat test\MultiDest recursive symbolic relative absolute
call t_SmartCopy04.bat test\MultiDest recursive symbolic absolute absolute
call t_SmartCopy04.bat test\MultiDest recursive symbolic relative
call t_SmartCopy04.bat test\MultiDest recursive symbolic absolute

REM
REM Smart Mirror Test Multiple Destinations
REM
call t_SmartCopy04.bat test\MultiDest mirror noitcnuj relative
call t_SmartCopy04.bat test\MultiDest mirror symbolic relative absolute
call t_SmartCopy04.bat test\MultiDest mirror symbolic absolute absolute
call t_SmartCopy04.bat test\MultiDest mirror symbolic relative
call t_SmartCopy04.bat test\MultiDest mirror symbolic absolute

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
@if exist %TESTROOT% %RD% %TESTROOT% 

REM -----------------


REM
REM SUCC: Check Move Rename Operations 
REM
call t_SmartMove02.bat test\SmartMove move noitcnuj relative
call t_SmartMove02.bat test\SmartMove move symbolic relative absolute
call t_SmartMove02.bat test\SmartMove move symbolic relative 
call t_SmartMove02.bat test\SmartMove move symbolic absolute absolute
call t_SmartMove02.bat test\SmartMove move symbolic absolute

call t_DeepPathOn.bat

call t_SmartMove02.bat test\SmartMove move noitcnuj relative
call t_SmartMove02.bat test\SmartMove move symbolic relative absolute
call t_SmartMove02.bat test\SmartMove move symbolic relative 
call t_SmartMove02.bat test\SmartMove move symbolic absolute absolute
call t_SmartMove02.bat test\SmartMove move symbolic absolute

call t_DeepPathOff.bat

REM
REM SUCC: Check Move Rename Operations 
REM
call t_SmartMove02.bat test\SmartRename smartrename noitcnuj relative
call t_SmartMove02.bat test\SmartRename smartrename symbolic relative absolute

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
@%RD% %TESTROOTBKP%
@echo on

@REM Failing Dirs
@REM
%LN% --delorean %TESTROOTSRC%NotExisting %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@echo ErrorLevel == %ERRLEV%

@REM
%LN% --delorean %TESTROOTSRC% %TESTROOTDST%NotExisting %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@echo ErrorLevel == %ERRLEV%

@REM
%LN% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP%NotExisting > sortout
@set ERRLEV=%errorlevel%
@echo ErrorLevel == %ERRLEV%


call t_RootCopy01.bat test\rootcopy

REM
REM Check DeepPathCopy 
REM
call t_DeepPathOn.bat

call t_SmartCopy03.bat test\DeepPath copy noitcnuj relative
call t_SmartCopy03.bat test\DeepPath copy symbolic absolute absolute
call t_SmartCopy03.bat test\DeepPath copy symbolic relative absolute
call t_SmartCopy03.bat test\DeepPath copy symbolic relative
call t_SmartCopy03.bat test\DeepPath copy symbolic absolute

REM
REM Check DeepPathClone
REM
call t_SmartCopy03.bat test\SmartClone recursive noitcnuj relative
call t_SmartCopy03.bat test\SmartClone recursive symbolic absolute absolute
call t_SmartCopy03.bat test\SmartClone recursive symbolic relative absolute
call t_SmartCopy03.bat test\SmartClone recursive symbolic relative
call t_SmartCopy03.bat test\SmartClone recursive symbolic absolute
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
REM call t_Delorean01.bat test\tm none noitcnuj none absolute

REM
REM Check timestamp handling
REM
call t_timestamp.bat test\timestamp

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
call t_UnrollSplice02.bat test\OuterSymlinkUnroll unroll symbolic relative
call t_UnrollSplice02.bat test\OuterSymlinkUnroll unroll symbolic relative absolute
call t_UnrollSplice02.bat test\OuterSymlinkUnroll unroll symbolic absolute
call t_UnrollSplice02.bat test\OuterSymlinkUnroll unroll symbolic absolute absolute

call t_Delorean02.bat test\DeloreanUnroll unroll noitcnuj relative
call t_Delorean02.bat test\DeloreanUnroll unroll symbolic relative
call t_Delorean02.bat test\DeloreanUnroll unroll symbolic relative absolute
call t_Delorean02.bat test\DeloreanUnroll unroll symbolic absolute
call t_Delorean02.bat test\DeloreanUnroll unroll symbolic absolute absolute

call t_MultiSource01.bat test\MultiSourceUnroll unroll noitcnuj relative
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic relative
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic relative absolute
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic absolute
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic absolute absolute

REM
REM DeepPath Unroll
REM
call t_DeepPathOn.bat

call t_UnrollSplice02.bat test\DeepOuterJunctionUnroll unroll noitcnuj relative
call t_UnrollSplice02.bat test\DeepOuterSymlinkUnroll unroll symbolic relative
call t_UnrollSplice02.bat test\DeepOuterSymlinkUnroll unroll symbolic relative absolute
call t_UnrollSplice02.bat test\DeepOuterSymlinkUnroll unroll symbolic absolute
call t_UnrollSplice02.bat test\DeepOuterSymlinkUnroll unroll symbolic absolute absolute

call t_Delorean02.bat test\DeepDeloreanUnroll unroll noitcnuj relative
call t_Delorean02.bat test\DeepDeloreanUnroll unroll symbolic relative
call t_Delorean02.bat test\DeepDeloreanUnroll unroll symbolic relative absolute
call t_Delorean02.bat test\DeepDeloreanUnroll unroll symbolic absolute
call t_Delorean02.bat test\DeepDeloreanUnroll unroll symbolic absolute absolute

call t_MultiSource01.bat test\MultiSourceUnroll unroll noitcnuj relative
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic relative
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic relative absolute
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic absolute
call t_MultiSource01.bat test\MultiSourceUnroll unroll symbolic absolute absolute

call t_DeepPathOff.bat

REM
REM Splice
REM
@echo off
call t_UnrollSplice01.bat test\OuterJunctionSplice splice

call t_UnrollSplice02.bat test\OuterJunctionSplice splice noitcnuj relative
call t_UnrollSplice02.bat test\OuterSymlinkSplice splice symbolic relative
call t_UnrollSplice02.bat test\OuterSymlinkSplice splice symbolic relative absolute
call t_UnrollSplice02.bat test\OuterSymlinkSplice splice symbolic absolute
call t_UnrollSplice02.bat test\OuterSymlinkSplice splice symbolic absolute absolute

call t_Delorean02.bat test\DeloreanSplice splice noitcnuj relative
call t_Delorean02.bat test\DeloreanSplice splice symbolic relative
call t_Delorean02.bat test\DeloreanSplice splice symbolic relative absolute
call t_Delorean02.bat test\DeloreanSplice splice symbolic absolute
call t_Delorean02.bat test\DeloreanSplice splice symbolic absolute absolute

call t_MultiSource01.bat test\MultiSourceSplice splice noitcnuj relative
call t_MultiSource01.bat test\MultiSourceSplice splice symbolic relative
call t_MultiSource01.bat test\MultiSourceSplice splice symbolic relative absolute
call t_MultiSource01.bat test\MultiSourceSplice splice symbolic absolute
call t_MultiSource01.bat test\MultiSourceSplice splice symbolic absolute absolute

REM
REM DeepPath Splice
REM
call t_DeepPathOn.bat

call t_UnrollSplice02.bat test\DeepOuterJunctionSplice splice noitcnuj relative
call t_UnrollSplice02.bat test\DeepOuterSymlinkSplice splice symbolic relative
call t_UnrollSplice02.bat test\DeepOuterSymlinkSplice splice symbolic relative absolute
call t_UnrollSplice02.bat test\DeepOuterSymlinkSplice splice symbolic absolute
call t_UnrollSplice02.bat test\DeepOuterSymlinkSplice splice symbolic absolute absolute

call t_Delorean02.bat test\DeepDeloreanSplice splice noitcnuj relative
call t_Delorean02.bat test\DeepDeloreanSplice splice symbolic relative
call t_Delorean02.bat test\DeepDeloreanSplice splice symbolic relative absolute
call t_Delorean02.bat test\DeepDeloreanSplice splice symbolic absolute
call t_Delorean02.bat test\DeepDeloreanSplice splice symbolic absolute absolute

call t_MultiSource01.bat test\DeepMultiSourceSplice splice noitcnuj relative
call t_MultiSource01.bat test\DeepMultiSourceSplice splice symbolic relative
call t_MultiSource01.bat test\DeepMultiSourceSplice splice symbolic relative absolute
call t_MultiSource01.bat test\DeepMultiSourceSplice splice symbolic absolute
call t_MultiSource01.bat test\DeepMultiSourceSplice splice symbolic absolute absolute

call t_DeepPathOff.bat


REM
REM SmartMirror
REM
call t_SmartMirror01.bat test\mirror none symbolic none absolute
call t_SmartMirror02.bat test\mirror

REM
REM ReparsePoints on RootDrive
REM
@echo off
call t_RootReparsePoint01.bat test\RootReparse unroll noitcnuj relative
call t_RootReparsePoint01.bat test\RootReparse unroll symbolic relative
call t_RootReparsePoint01.bat test\RootReparse unroll symbolic relative absolute
call t_RootReparsePoint01.bat test\RootReparse unroll symbolic absolute
call t_RootReparsePoint01.bat test\RootReparse unroll symbolic absolute absolute

call t_RootReparsePoint01.bat test\RootReparse splice noitcnuj relative
call t_RootReparsePoint01.bat test\RootReparse splice symbolic relative
call t_RootReparsePoint01.bat test\RootReparse splice symbolic relative absolute
call t_RootReparsePoint01.bat test\RootReparse splice symbolic absolute
call t_RootReparsePoint01.bat test\RootReparse splice symbolic absolute absolute

REM
REM Dead Reparse Points and Disk-Ids
REM
call t_RootReparsePoint02.bat test\DeadReparse unroll noitcnuj relative

REM
REM mountpoint syntax junctions and symlinks
REM
call t_VolumeGuid01.bat test\VolumeGuid

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
call t_CaseSensitiv.bat test\CaseSensitiv unroll symbolic

REM
REM Absolute relative tests
REM
call t_AbsoluteRelativ.bat test\Absrel none noitcnuj none
call t_AbsoluteRelativ.bat test\Absrel none symbolic none
call t_AbsoluteRelativ.bat test\Absrel none symbolic absolute

REM
REM Reparse points on reparse points
REM
call t_ReparseOnReparse01.bat test\ReparseOnReparse none noitcnuj relative
call t_ReparseOnReparse01.bat test\ReparseOnReparse none symbolic relative
call t_ReparseOnReparse01.bat test\ReparseOnReparse none symbolic absolute 
call t_ReparseOnReparse01.bat test\ReparseOnReparse none symbolic absolute absolute
call t_ReparseOnReparse01.bat test\ReparseOnReparse none symbolic relative absolute

REM
REM Show Hardlink Sibblings
REM
call t_Siblings.bat test\siblings 
call t_Siblings.bat test\siblings traditional
call t_Siblings.bat test\siblings normal 5
call t_Siblings.bat test\siblings traditional 5
call t_DeepPathOn.bat
call t_Siblings.bat test\siblings 
call t_Siblings.bat test\siblings traditional
call t_DeepPathOff.bat

REM
REM Copy alternative data streams
REM
call t_AdsCopy01.bat test\ads copy noitcnuj relative unroll

REM
REM On the Fly DupeMerge tests
REM
call t_Dupemerge.bat test\DupeMerge

REM
REM Very Basic UNC tests & Network Tests
REM
call t_UNC_Simple.bat test\UNCSimple
call t_UNC_Simple.bat test\UNCSimple symbolic
call t_UNC_Simple.bat test\UNCSimple symbolic absolute

REM
REM If System Error 64, then disable virus scanner
REM
call t_NetworkDrive01.bat test\netdrive copy symbolic absolute unroll
call t_NetworkDrive01.bat test\netdrive copy symbolic relative unroll
call t_NetworkDrive01.bat test\netdrive copy noitcnuj relative unroll

call t_NetworkDrive01.bat test\netdrive copy symbolic absolute
call t_NetworkDrive01.bat test\netdrive copy symbolic relative
call t_NetworkDrive01.bat test\netdrive copy noitcnuj relative

call t_NetworkDrive01.bat test\netdrive copy symbolic absolute splice
call t_NetworkDrive01.bat test\netdrive copy symbolic relative splice
call t_NetworkDrive01.bat test\netdrive copy noitcnuj relative splice

call t_NetworkDrive01.bat test\netdrive recursive symbolic absolute unroll
call t_NetworkDrive01.bat test\netdrive recursive symbolic relative unroll
call t_NetworkDrive01.bat test\netdrive recursive noitcnuj relative unroll

call t_NetworkDrive01.bat test\netdrive recursive symbolic absolute
call t_NetworkDrive01.bat test\netdrive recursive symbolic relative
call t_NetworkDrive01.bat test\netdrive recursive noitcnuj relative

call t_NetworkDrive01.bat test\netdrive recursive symbolic absolute splice
call t_NetworkDrive01.bat test\netdrive recursive symbolic relative splice
call t_NetworkDrive01.bat test\netdrive recursive noitcnuj relative splice

REM
REM --unroll should override --exclude ( but currently does not )
REM
call t_UnrollExcl01.bat test\unrollexcl


@REM goto ausmausraus

REM
REM Copy ACLs
REM
call t_AdsCopy02.bat test\ads copy noitcnuj relative unroll
call t_AdsCopy02.bat test\ads copy symbolic relative unroll
call t_AdsCopy02.bat test\ads copy noitcnuj relative splice
call t_AdsCopy02.bat test\ads copy symbolic relative splice

call t_AdsCopy02.bat test\ads recursive noitcnuj relative unroll
call t_AdsCopy02.bat test\ads recursive noitcnuj relative splice
call t_AdsCopy02.bat test\ads recursive symbolic relative unroll
call t_AdsCopy02.bat test\ads recursive symbolic relative splice

call t_AdsCopy02.bat test\ads mirror noitcnuj relative unroll
call t_AdsCopy02.bat test\ads mirror noitcnuj relative splice

set SAVE_LN=%LN%
set LN=%LN% --traditional

call t_AdsCopy02.bat test\ads copy noitcnuj relative unroll
call t_AdsCopy02.bat test\ads copy symbolic relative unroll
call t_AdsCopy02.bat test\ads copy noitcnuj relative splice
call t_AdsCopy02.bat test\ads copy symbolic relative splice

call t_AdsCopy02.bat test\ads recursive noitcnuj relative unroll
call t_AdsCopy02.bat test\ads recursive noitcnuj relative splice
call t_AdsCopy02.bat test\ads recursive symbolic relative unroll
call t_AdsCopy02.bat test\ads recursive symbolic relative splice

set LN=%SAVE_LN%

REM
REM Check if the symlink relation is copied
REM
set FLIPABSREL=flipabsrel
call t_AdsCopy02.bat test\ads copy symbolic keepsymlinkrelation unroll
call t_AdsCopy02.bat test\ads copy symbolic keepsymlinkrelation splice


REM Currently off because cacls.exe does not support very long path names
REM
REM call t_DeepPathOn.bat
REM call t_AdsCopy02.bat test\ads copy noitcnuj relative
REM call t_AdsCopy02.bat test\ads copy symbolic relative
REM call t_DeepPathOff.bat


REM
REM
REM
call t_AdsCopy03.bat test\ads copy noitcnuj relative unroll
call t_AdsCopy03.bat test\ads copy symbolic relative unroll


REM Delete test
REM
call t_Delete01.bat test\delete noitcnuj 
call t_Delete01.bat test\delete symbolic
call t_Delete01.bat test\delete symbolic absolute

call t_DeepPathOn.bat

call t_Delete01.bat test\delete noitcnuj 
call t_Delete01.bat test\delete symbolic
call t_Delete01.bat test\delete symbolic absolute

call t_DeepPathOff.bat



REM Json test
REM
call t_Json01.bat test\json copy noitcnuj relative unroll

REM
REM SUCC: Command line tests
REM
REM mkdir c:\1
REM %LN%  -e c:\1\
REM %LN%  -e "c:\1"
REM %LN%  -e c:\1
REM %LN%  -e "c:\1\"

:ausmausraus

:nodel
@echo on

