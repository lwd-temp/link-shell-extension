@echo off
@echo on

set OPTION=--%2

REM 
REM Hardlinking Tests
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\dst

REM 
REM Create a normal hardlink tupel
REM
@%RD% %TESTROOTDST%
@%RD% %TESTROOTSRC%
mkdir %TESTROOTSRC%
copy test\resource.h %TESTROOTSRC%\hl0_0.txt > nul
%LN% %TESTROOTSRC%\hl0_0.txt %TESTROOTSRC%\hl0_1.txt > nul
%LN% %TESTROOTSRC%\hl0_0.txt %TESTROOTSRC%\hl0_2.txt > nul
%LN% %TESTROOTSRC%\hl0_0.txt %TESTROOTSRC%\hl0_3.txt > nul

copy test\resource.h %TESTROOTSRC%\hl1_0.txt > nul
%LN% %TESTROOTSRC%\hl1_0.txt %TESTROOTSRC%\hl1_1.txt > nul
%LN% %TESTROOTSRC%\hl1_0.txt %TESTROOTSRC%\hl1_2.txt > nul
%LN% %TESTROOTSRC%\hl1_0.txt %TESTROOTSRC%\hl1_3.txt > nul

@echo on

REM
REM Do a simple hardlink copy
REM
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Copy again, to see that nothing is copied, because the date didn't change
REM
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST%   > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Make the leader newer, so that it has to be copied, and all hardlinks to newly tied
REM 
%TIMESTAMP% --writetime %TESTROOTSRC%\hl1_0.txt
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST%  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Add an additional hardlink, only one hardlink must be additionally tied.
REM 
@echo off
@%RD% %TESTROOTDST%
%LN% --copy %TESTROOTSRC% %TESTROOTDST% > nul
REM
REM Read a few bytes of a file, so that the timestamps get updated for hardlinks. In general
REM Windows is crazy in this area because it does not immediatley update the timestamps of 
REM hardlinked files, but does it only after a few bytes have been read. Since we don't want
REM to test the curiosities of Windows, we bring the system into a stable state by reading
REM just a  few bytes of each newly hardlinked file
REM
%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_1.txt > nul
%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_2.txt > nul
%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_3.txt > nul
%TIMESTAMP% --readfile %TESTROOTDST%\hl1_1.txt > nul
%TIMESTAMP% --readfile %TESTROOTDST%\hl1_2.txt > nul
%TIMESTAMP% --readfile %TESTROOTDST%\hl1_3.txt > nul
@echo on
%LN% %TESTROOTSRC%\hl1_0.txt %TESTROOTSRC%\hl1_4.txt > nul
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Lock the leader in the destination, so that a new leader has to be found
REM But since permissions are valid for siblings of a hardlink no new leader
REM can be found
REM 
%TIMESTAMP% -w %TESTROOTSRC%\hl1_0.txt
@%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_0.txt
@%TIMESTAMP% --readfile %TESTROOTDST%\hl1_0.txt
@type test\y | @%CACLS% %TESTROOTDST%\hl1_0.txt /D "%USERNAME%" > nul
@type test\y | @%CACLS% %TESTROOTDST%\hl1_0.txt /P "%USERNAME%":R > nul
@type test\y | @%CACLS% %TESTROOTDST% /D "%USERNAME%" > nul
@type test\y | @%CACLS% %TESTROOTDST% /P "%USERNAME%":R > nul
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@type test\y | @%CACLS% %TESTROOTDST% /G "%USERNAME%":F > nul
@type test\y | @%CACLS% %TESTROOTDST%\hl1_0.txt /G "%USERNAME%":F > nul

REM
REM Delete 2 of 5 files from the destination, and lock the leader in the 
REM destination, so that a new leader has to be found
REM 
@del %TESTROOTDST%\hl1_3.txt > nul
@del %TESTROOTDST%\hl1_4.txt > nul
%TIMESTAMP% -w %TESTROOTSRC%\hl1_0.txt
@type test\y | @%CACLS% %TESTROOTDST%\hl1_0.txt /D "%USERNAME%" > nul
@type test\y | @%CACLS% %TESTROOTDST%\hl1_0.txt /P "%USERNAME%":R > nul
@type test\y | @%CACLS% %TESTROOTDST% /D "%USERNAME%" > nul
@type test\y | @%CACLS% %TESTROOTDST% /P "%USERNAME%":R > nul
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@type test\y | @%CACLS% %TESTROOTDST% /G "%USERNAME%":F > nul
@type test\y | @%CACLS% %TESTROOTDST%\hl1_0.txt /G "%USERNAME%":F > nul

REM
REM The hardlink groups have been split now. But since we newly hardlinked 
REM hl1_4.txt, the date in the source is newer and relinking must take place.
REM 
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@REM
@REM after relinking, we have to update the timestamps of all
@REM hardlink siblings
@%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_0.txt
@%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_1.txt
@%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_2.txt
@%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_3.txt
@%TIMESTAMP% --readfile %TESTROOTSRC%\hl1_4.txt
@%TIMESTAMP% --readfile %TESTROOTDST%\hl1_0.txt

REM
REM Delete some hardlinks from the destination, and check that they are 
REM created again
REM 
@del %TESTROOTDST%\hl1_3.txt > nul
@del %TESTROOTDST%\hl1_4.txt > nul
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
@%TIMESTAMP% --readfile %TESTROOTDST%\hl1_0.txt
@%TIMESTAMP% --readfile %TESTROOTDST%\hl1_1.txt
@%TIMESTAMP% --readfile %TESTROOTDST%\hl1_2.txt
@%TIMESTAMP% --readfile %TESTROOTDST%\hl1_3.txt
@%TIMESTAMP% --readfile %TESTROOTDST%\hl1_4.txt
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Lock the hardlink tupel in the destination. But since
REM nothing has changed, no error is displayed
REM
@type test\y | @%CACLS% %TESTROOTDST%\hl1_0.txt /D "%USERNAME%" > nul
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@type test\y | @%CACLS% %TESTROOTDST%\hl1_0.txt /G "%USERNAME%":F > nul

REM
REM Do it again, but have a file with read only in the destination
REM
attrib +r %TESTROOTDST%\hl0_0.txt
@%TIMESTAMP% --writetime %TESTROOTSRC%\hl0_0.txt
@%TIMESTAMP% --readfile %TESTROOTSRC%\hl0_0.txt
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Do it again, but have a file with attributes so that it cant be accessed
REM
@type test\y | @%CACLS% %TESTROOTDST%\hl0_0.txt /D "%USERNAME%" > nul
@type test\y | @%CACLS% %TESTROOTDST% /D "%USERNAME%" > nul
@%TIMESTAMP% --writetime %TESTROOTSRC%\hl0_0.txt
@%TIMESTAMP% --readfile %TESTROOTSRC%\hl0_0.txt
%LN% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@type test\y | @%CACLS% %TESTROOTDST% /G "%USERNAME%":F > nul
@type test\y | @%CACLS% %TESTROOTDST%\hl0_0.txt /G "%USERNAME%":F > nul

REM Test the subst driveletters hardlinking
REM
@echo off
@set EMPTYTESTDRIVE2=q:
@subst.exe %EMPTYTESTDRIVE% %TESTROOTSRC% > nul
@subst.exe %EMPTYTESTDRIVE2% %TESTROOTSRC% > nul
@copy ln.h %TESTROOTSRC% > nul
@copy ln.h %EMPTYTESTDRIVE%\reverse.h > nul
%LN% %TESTROOTSRC%\ln.h %EMPTYTESTDRIVE%\ln_hardlink.h > nul
%LN% %EMPTYTESTDRIVE2%\ln.h %EMPTYTESTDRIVE%\ln_hardlink2.h > nul
@echo on
%LN% --symbolic %TESTROOTSRC%\ln_hardlink.h
%LN% --symbolic %TESTROOTSRC%\ln_hardlink2.h
%LN% --list %EMPTYTESTDRIVE%\ln_hardlink2.h | sort
%LN% --list %TESTROOTSRC%\ln_hardlink2.h | sort
@echo off
subst /d %EMPTYTESTDRIVE2%
subst %EMPTYTESTDRIVE2% %EMPTYTESTDRIVE%\
%LN% %EMPTYTESTDRIVE2%\ln.h %EMPTYTESTDRIVE%\ln_hardlink3.h > nul
@echo on
%LN% --symbolic %TESTROOTSRC%\ln_hardlink3.h

REM Test the subst driveletters symbolic linking
REM
%LN% --symbolic %TESTROOTSRC%\ln.h %EMPTYTESTDRIVE%\ln_symlink.h > nul
%LN% --symbolic %TESTROOTSRC%\ln_symlink.h

%LN% --absolute --symbolic %TESTROOTSRC%\ln.h %EMPTYTESTDRIVE%\ln_symlink_abs.h > nul
%LN% --symbolic %TESTROOTSRC%\ln_symlink_abs.h

REM reverse
%LN% --symbolic %EMPTYTESTDRIVE%\reverse.h %TESTROOTSRC%\reverse_symlink.h > nul
%LN% --symbolic %TESTROOTSRC%\reverse_symlink.h

%LN% --absolute --symbolic %EMPTYTESTDRIVE%\reverse.h %TESTROOTSRC%\reverse_symlink_abs.h > nul
%LN% --symbolic %TESTROOTSRC%\reverse_symlink_abs.h

REM within
%LN% --symbolic %EMPTYTESTDRIVE%\reverse.h %EMPTYTESTDRIVE%\within.h > nul
%LN% --symbolic %EMPTYTESTDRIVE%\within.h

%LN% --absolute --symbolic %EMPTYTESTDRIVE%\reverse.h %EMPTYTESTDRIVE%\within_abs.h > nul
%LN% --symbolic %EMPTYTESTDRIVE%\within_abs.h



@echo off
subst /d %EMPTYTESTDRIVE%
subst /d %EMPTYTESTDRIVE2%

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo off
:ausmausraus
@echo on

@goto :EOF 
