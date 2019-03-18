@echo off
REM
REM Test the change of file types 

set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest

@REM 
@REM Clean up
@REM
@%RD% %TESTROOT% > nul

@echo on
REM
REM --timetolerance with --mirror
REM
@echo off
%MKDIR% %TESTROOTSRC% > nul
%COPY% test\ln.h %TESTROOTSRC%\A > nul
%TIMESTAMP% --writetime --ctime  4e1402f1 %TESTROOTSRC%\A 
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > nul
%TIMESTAMP% --writetime --ctime  4e1402f2 %TESTROOTDST%\A 

@echo on
%LN% --timetolerance 1002 --mirror %TESTROOTSRC% %TESTROOTDST%
%LN% --timetolerance 997 --mirror %TESTROOTSRC% %TESTROOTDST%

REM
REM --timetolerance with --copy
REM 
%TIMESTAMP% --writetime --ctime  4e1402f2 %TESTROOTDST%\A 
%LN% --timetolerance 1002 --copy %TESTROOTSRC% %TESTROOTDST%
%LN% --timetolerance 997 --copy %TESTROOTSRC% %TESTROOTDST%
%LN% --timetolerance 997 --copy %TESTROOTSRC% %TESTROOTDST%

REM
REM --timetolerance with --copy but no write access to destination
REM but it should says is the same
REM 
@%TIMESTAMP% --writetime --ctime  4e1402f2 %TESTROOTDST%\A 
@attrib +r %TESTROOTDST%\A 
%LN% --timetolerance 1002 --copy %TESTROOTSRC% %TESTROOTDST%
%LN% --timetolerance 997 --copy %TESTROOTSRC% %TESTROOTDST%

REM
REM when there is no write access to destination, CopyfileEx3 will try to switch 
REM the attributes, and copy over. So there must be no +r on %TESTROOTDST%\A at the end
REM 
@attrib %TESTROOTDST%\A 

REM
REM Files with different compression status are different, even if in tolerance
REM
@%TIMESTAMP% --writetime --ctime  4e1402f2 %TESTROOTDST%\A 
@attrib +r %TESTROOTDST%\A 
@compact /C %TESTROOTDST%\A > nul
@%TIMESTAMP% --xattr %TESTROOTDST%\A 

%LN% --timetolerance 1002 --copy %TESTROOTSRC% %TESTROOTDST%
@%TIMESTAMP% --xattr %TESTROOTDST%\A 

REM 
REM Different size, but same timestamp is different
REM
%COPY% test\readme.txt %TESTROOTDST%\A > nul
@%TIMESTAMP% --backup %TESTROOTSRC%\A %TESTROOTDST%\A 
%LN% --copy %TESTROOTSRC% %TESTROOTDST%

@REM 
@REM Clean up
@REM
@%RD% %TESTROOT% > nul

@echo on
REM
REM 01 --mirror -d +f
REM
@%MKDIR% %TESTROOTSRC% > nul
@%COPY% test\ln.h %TESTROOTSRC%\A > nul
@%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > nul

@del %TESTROOTSRC%\A > nul
@%MKDIR% %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST%

REM
REM 02 --mirror -f +d
REM
@%RD% %TESTROOTSRC%\A > nul
@%COPY% test\ln.h %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 03 --mirror -f +sf
REM
@del %TESTROOTSRC%\A > nul
@%COPY% test\ln.h %TESTROOTSRC%\A_SymlinkDest > nul
@%LN% -s %TESTROOTSRC%\A_SymlinkDest %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 04 --mirror -sf +f
REM
@del %TESTROOTSRC%\A > nul
@del %TESTROOTSRC%\A_SymlinkDest  > nul
@%COPY% test\ln.h %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 05 --mirror -f +j
REM
@del %TESTROOTSRC%\A > nul
@%MKDIR% %TESTROOTSRC%\A_ReparseDest > nul
@%LN% --noitcnuj %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 22 --mirror =j
REM
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 06 --mirror -j +d
REM
@%RD% %TESTROOTSRC%\A > nul
@%RD% %TESTROOTSRC%\A_ReparseDest > nul
@%MKDIR% %TESTROOTSRC%\A

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 21 --mirror -j +d
REM
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 07 --mirror -d +j
REM
@%RD% %TESTROOTSRC%\A > nul
@%MKDIR% %TESTROOTSRC%\A_ReparseDest > nul
@%LN% --noitcnuj %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A  > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 08 --mirror -j +s
REM
@%RD% %TESTROOTSRC%\A > nul
@%LN% --symbolic %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A  > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 09 --mirror -s +j
REM
@%RD% %TESTROOTSRC%\A > nul
@%LN% --noitcnuj %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A  > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 10 --mirror -j +f
REM
@%RD% %TESTROOTSRC% > nul
@%MKDIR% %TESTROOTSRC% > nul
@%COPY% test\ln.h %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 11 --mirror -s +sf, :: Restart
REM
@%RD% %TESTROOTSRC% > nul
@%MKDIR% %TESTROOTSRC%\A_ReparseDest > nul
@%LN% --symbolic %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A > nul
@%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > nul


@%RD% %TESTROOTSRC%\A > nul
@%RD% %TESTROOTSRC%\A_ReparseDest > nul
@%COPY% test\ln.h %TESTROOTSRC%\A_ReparseDest > nul
@%LN% --symbolic %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 23 --mirror =s
REM
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 12 --mirror -sf +s
REM
@del %TESTROOTSRC%\A > nul
@del %TESTROOTSRC%\A_ReparseDest > nul
@%MKDIR% %TESTROOTSRC%\A_ReparseDest > nul
@%LN% --symbolic %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 13 --mirror -s +f 
REM
@%RD% %TESTROOTSRC%\A > nul
@%RD% %TESTROOTSRC%\A_ReparseDest > nul
@%COPY% test\ln.h %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 20 --mirror =f
REM
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 14 --mirror -d +s, :: Restart
REM
@%RD% %TESTROOT% > nul
@%MKDIR% %TESTROOTDST%\A > nul

@%MKDIR% %TESTROOTSRC%\A_ReparseDest > nul
@%LN% --symbolic %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 15 --mirror -f +s, :: Restart
REM
@%RD% %TESTROOTDST% > nul
@%MKDIR% %TESTROOTDST% > nul
@%COPY% test\ln.h %TESTROOTDST%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 16 --mirror -s +d, Restart
REM
@%RD% %TESTROOT% > nul
@%MKDIR% %TESTROOTDST%\A_ReparseDest > nul
@%LN% --symbolic %TESTROOTDST%\A_ReparseDest %TESTROOTDST%\A > nul

@%MKDIR% %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 17 --mirror -d +sf
REM
@%RD% %TESTROOTSRC% > nul
@%MKDIR% %TESTROOTSRC% > nul
@%COPY% test\ln.h %TESTROOTSRC%\A_ReparseDest > nul
@%LN% --symbolic %TESTROOTSRC%\A_ReparseDest %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 19 --mirror =sf
REM
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 18 --mirror -sf +d
REM
@%RD% %TESTROOTSRC% > nul
@%MKDIR% %TESTROOTSRC%\A > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

REM
REM 24 --mirror -d +h
REM
@%RD% %TESTROOT% > nul
@%MKDIR% %TESTROOTSRC%\11 > nul
@%LN% --noitcnuj %TESTROOTSRC%\11 %TESTROOTSRC%\11_junction  > nul

@%COPY% test\ln.h %TESTROOTSRC%\x > nul
@%LN% %TESTROOTSRC%\x %TESTROOTSRC%\x_hard > nul

@%MKDIR% %TESTROOTDST%\x > nul
@%COPY% test\ln.h %TESTROOTDST%\x_hard > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout

:Outer_ReparsePoints

@set OPTION=--splice
@%RD% %TESTROOT% > nul

REM
REM  108 --mirror -j +s
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir > nul
@%MKDIR% %TESTROOTSRC%\out\A_JunctionDest\junction > nul
@%LN% --noitcnuj %TESTROOTSRC%\out\A_JunctionDest %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%MKDIR% %TESTROOTSRC%\out\A_SymlinkDest\symlink > nul
@%RD% %TESTROOTSRC%\out\A_JunctionDest > nul
@%RD% %TESTROOTSRC%\dir\A

@%LN% --symbolic %TESTROOTSRC%\out\A_SymlinkDest %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  109 --mirror -s +j
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir > nul
@%MKDIR% %TESTROOTSRC%\out\A_SymlinkDest\symlink > nul
@%LN% --symbolic %TESTROOTSRC%\out\A_SymlinkDest %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%MKDIR% %TESTROOTSRC%\out\A_JunctionDest\junction > nul
@%RD% %TESTROOTSRC%\out\A_SymlinkDest > nul
@%RD% %TESTROOTSRC%\dir\A

@%LN% --noitcnuj %TESTROOTSRC%\out\A_JunctionDest %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  106 --mirror -j +d
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir > nul
@%MKDIR% %TESTROOTSRC%\out\A_JunctionDest\junction > nul
@%LN% --noitcnuj %TESTROOTSRC%\out\A_JunctionDest %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%RD% %TESTROOTSRC%\out\A_JunctionDest > nul
@%RD% %TESTROOTSRC%\dir\A
@%MKDIR% %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  107 --mirror -d +j
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%MKDIR% %TESTROOTSRC%\out\A_JunctionDest\junction > nul
@%RD% %TESTROOTSRC%\dir\A
@%LN% --noitcnuj %TESTROOTSRC%\out\A_JunctionDest %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  105 --mirror -f +j
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir > nul
@%COPY% test\ln.h %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%MKDIR% %TESTROOTSRC%\out\A_JunctionDest\junction > nul
@del %TESTROOTSRC%\dir\A
@%LN% --noitcnuj %TESTROOTSRC%\out\A_JunctionDest %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  110 --mirror -j +f
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir > nul
@%MKDIR% %TESTROOTSRC%\out\A_JunctionDest\junction > nul
@%LN% --noitcnuj %TESTROOTSRC%\out\A_JunctionDest %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%RD% %TESTROOTSRC%\dir\A
@%COPY% test\ln.h %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  115 --mirror -f +s
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir > nul
@%COPY% test\ln.h %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%MKDIR% %TESTROOTSRC%\out\A_SymlinkDest\symlink > nul
@del %TESTROOTSRC%\dir\A
@%LN% --symbolic %TESTROOTSRC%\out\A_SymlinkDest %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  113 --mirror -s +f
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir > nul
@%MKDIR% %TESTROOTSRC%\out\A_SymlinkDest\symlink > nul
@%LN% --symbolic %TESTROOTSRC%\out\A_SymlinkDest %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%RD% %TESTROOTSRC%\dir\A
@%COPY% test\ln.h %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  114 --mirror -d +s
REM
@%MKDIR% %TESTROOTDST% > nul
@%MKDIR% %TESTROOTSRC%\dir\A > nul
@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul
@%MKDIR% %TESTROOTSRC%\out\A_SymlinkDest\symlink > nul
@%RD% %TESTROOTSRC%\dir\A
@%LN% --symbolic %TESTROOTSRC%\out\A_SymlinkDest %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  116 --mirror -s +d
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTSRC%\dir > nul
@%MKDIR% %TESTROOTSRC%\out\A_SymlinkDest\symlink > nul
@%LN% --symbolic %TESTROOTSRC%\out\A_SymlinkDest %TESTROOTSRC%\dir\A > nul

@%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > nul

@%RD% %TESTROOTSRC%\dir\A
@%MKDIR% %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  150 --mirror crop -sf +j 
REM
@%MKDIR% %TESTROOTDST% > nul

@%MKDIR% %TESTROOTDST%\dir > nul
@%MKDIR% %TESTROOTDST%\out > nul
@%COPY% test\ln.h %TESTROOTDST%\out\A_SymlinkDest > nul
@%LN% --symbolic %TESTROOTDST%\out\A_SymlinkDest %TESTROOTDST%\dir\A > nul

@%MKDIR% %TESTROOTSRC%\out\A_JunctionDest\junction > nul
@%MKDIR% %TESTROOTSRC%\Dir > nul
@%LN% --noitcnuj %TESTROOTSRC%\out\A_JunctionDest %TESTROOTSRC%\dir\A > nul

%LN% %OPTION% --mirror %TESTROOTSRC%\Dir %TESTROOTDST%\Dir > sortout
@sort sortout

@%RD% %TESTROOT% > nul

REM
REM  151 --mirror crop delete -sf +j 
REM
@set OPTION=
@%MKDIR% %TESTROOT%\src > nul
@%MKDIR% %TESTROOT%\src2 > nul
@%COPY% test\ln.h %TESTROOT%\src\A_SymlinkDest > nul
@%LN% --symbolic %TESTROOT%\src\A_SymlinkDest %TESTROOT%\src2\A > nul

@%MKDIR% %TESTROOT%\dst2 > nul
@%MKDIR% %TESTROOT%\dst\A_JunctionDest > nul
@%LN% --noitcnuj %TESTROOT%\dst\A_JunctionDest %TESTROOT%\dst2\A > nul

%LN% %OPTION% --mirror %TESTROOT%\src2 %TESTROOT%\dst2 > sortout
@sort sortout

:mopuntpoint_test
@%RD% %TESTROOT% > nul

@REM
@REM Tests for Mountpoints
@REM 
@set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
@%TRUECRYPT% test\NtfsTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit
@for /f %%f in ('mountvol %EMPTYTESTDRIVE% /L') do @set VOLUME_GUID=%%f
@echo %VOLUME_GUID% > .volumeguid

@%MKDIR% %TESTROOTSRC% > nul
@%MKDIR% %TESTROOTSRCDRV%\tmp

REM
REM  200 --mirror -d +m
REM
@%MKDIR% %TESTROOTDST%\A_JuncEmptyTestDrive
@%MKDIR% %TESTROOTDST%\A_JuncEmptyTestDriveRoot
@%LN% --noitcnuj %VOLUME_GUID% %TESTROOTSRC%\A_JuncEmptyTestDriveRoot > nul
@%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTSRC%\A_JuncEmptyTestDrive > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%LN% --noitcnuj %TESTROOTDST%\A_JuncEmptyTestDrive
@%LN% --noitcnuj %TESTROOTDST%\A_JuncEmptyTestDriveRoot

REM
REM  200a --mirror -m #m
REM
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%LN% --noitcnuj %VOLUME_GUID% %TESTROOTDST%\A_JuncEmptyTestDriveRoot > nul
@%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTDST%\A_JuncEmptyTestDrive > nul

REM
REM  201 --mirror +d -m
REM
@%RD% %TESTROOTSRC%\A_JuncEmptyTestDrive
@%RD% %TESTROOTSRC%\A_JuncEmptyTestDriveRoot
@%MKDIR% %TESTROOTSRC%\A_JuncEmptyTestDrive
@%MKDIR% %TESTROOTSRC%\A_JuncEmptyTestDriveRoot
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@dir /b %TESTROOTDST%

REM
REM  202 --mirror -f +m
REM
@%RD% %TESTROOT%
@%MKDIR% %TESTROOTDST% > nul
@%MKDIR% %TESTROOTSRC% > nul
@%COPY% test\ln.h %TESTROOTDST%\A_FileDestRoot > nul
@%COPY% test\ln.h %TESTROOTDST%\A_FileDest > nul
@%LN% --noitcnuj %VOLUME_GUID% %TESTROOTSRC%\A_FileDestRoot > nul
@%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTSRC%\A_FileDest > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%LN% --noitcnuj %TESTROOTDST%\A_FileDest
@%LN% --noitcnuj %TESTROOTDST%\A_FileDestRoot

REM
REM  203 --mirror +f -m
REM
@%RD% %TESTROOTSRC%\A_FileDest
@%RD% %TESTROOTSRC%\A_FileDestRoot
@%COPY% test\ln.h %TESTROOTSRC%\A_FileDestRoot > nul
@%COPY% test\ln.h %TESTROOTSRC%\A_FileDest > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%WHERE% *.* %TESTROOTDST%

REM
REM  204 --mirror -j +m
REM
@%RD% %TESTROOT%
@%MKDIR% %TESTROOTDST% > nul
@%MKDIR% %TESTROOTSRC% > nul
@%MKDIR% %TESTROOTDST%\A_JuncTarget > nul
@%MKDIR% %TESTROOTDST%\A_JuncTargetRoot > nul
@%LN% --noitcnuj %TESTROOTDST%\A_JuncTarget %TESTROOTDST%\A_Junc > nul
@%LN% --noitcnuj %TESTROOTDST%\A_JuncTargetRoot %TESTROOTDST%\A_JuncRoot > nul
@%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTSRC%\A_Junc > nul
@%LN% --noitcnuj %VOLUME_GUID% %TESTROOTSRC%\A_JuncRoot > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout 
@%LN% --noitcnuj %TESTROOTDST%\A_Junc
@%LN% --noitcnuj %TESTROOTDST%\A_JuncRoot

REM
REM  205 --mirror +j -m
REM
@%RD% %TESTROOTSRC%\A_Junc
@%RD% %TESTROOTSRC%\A_JuncRoot
@%MKDIR% %TESTROOTSRC%\A_JuncTarget > nul
@%MKDIR% %TESTROOTSRC%\A_JuncTargetRoot > nul
@%LN% --noitcnuj %TESTROOTSRC%\A_JuncTarget %TESTROOTSRC%\A_Junc > nul
@%LN% --noitcnuj %TESTROOTSRC%\A_JuncTargetRoot %TESTROOTSRC%\A_JuncRoot > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout 
@%LN% --noitcnuj %TESTROOTDST%\A_Junc
@%LN% --noitcnuj %TESTROOTDST%\A_JuncRoot

REM
REM  206 --mirror -s +m
REM
@%RD% %TESTROOT%
@%MKDIR% %TESTROOTDST% > nul
@%MKDIR% %TESTROOTSRC% > nul
@%MKDIR% %TESTROOTDST%\A_SymlinkTarget > nul
@%MKDIR% %TESTROOTDST%\A_SymlinkTargetRoot > nul
@%LN% --symbolic %TESTROOTDST%\A_SymlinkTarget %TESTROOTDST%\A_Symlink > nul
@%LN% --symbolic %TESTROOTDST%\A_SymlinkTargetRoot %TESTROOTDST%\A_SymlinkRoot > nul
@%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTSRC%\A_Symlink > nul
@%LN% --noitcnuj %VOLUME_GUID% %TESTROOTSRC%\A_SymlinkRoot > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%LN% --noitcnuj %TESTROOTDST%\A_Symlink
@%LN% --noitcnuj %TESTROOTDST%\A_SymlinkRoot

REM
REM  207 --mirror +s -m
REM
@%RD% %TESTROOTSRC%\A_Symlink
@%RD% %TESTROOTSRC%\A_SymlinkRoot
@%MKDIR% %TESTROOTSRC%\A_SymlinkTarget > nul
@%MKDIR% %TESTROOTSRC%\A_SymlinkTargetRoot > nul
@%LN% --symbolic %TESTROOTSRC%\A_SymlinkTarget %TESTROOTSRC%\A_Symlink > nul
@%LN% --symbolic %TESTROOTSRC%\A_SymlinkTargetRoot %TESTROOTSRC%\A_SymlinkRoot > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%LN% --symbolic %TESTROOTDST%\A_Symlink
@%LN% --symbolic %TESTROOTDST%\A_SymlinkRoot

REM
REM  208 --mirror -sf +m
REM
@%RD% %TESTROOT%
@%MKDIR% %TESTROOTDST% > nul
@%MKDIR% %TESTROOTSRC% > nul
@%COPY% test\ln.h %TESTROOTDST%\A_SymlinkTarget > nul
@%COPY% test\ln.h %TESTROOTDST%\A_SymlinkTargetRoot > nul
@%LN% --symbolic %TESTROOTDST%\A_SymlinkTarget %TESTROOTDST%\A_Symlink > nul
@%LN% --symbolic %TESTROOTDST%\A_SymlinkTargetRoot %TESTROOTDST%\A_SymlinkRoot > nul
@%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTSRC%\A_Symlink > nul
@%LN% --noitcnuj %VOLUME_GUID% %TESTROOTSRC%\A_SymlinkRoot > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%LN% --noitcnuj %TESTROOTDST%\A_Symlink
@%LN% --noitcnuj %TESTROOTDST%\A_SymlinkRoot

REM
REM  209 --mirror +sf -m
REM
@%RD% %TESTROOTSRC%\A_Symlink
@%RD% %TESTROOTSRC%\A_SymlinkRoot
@%COPY% test\ln.h %TESTROOTSRC%\A_SymlinkTarget > nul
@%COPY% test\ln.h %TESTROOTSRC%\A_SymlinkTargetRoot > nul
@%LN% --symbolic %TESTROOTSRC%\A_SymlinkTarget %TESTROOTSRC%\A_Symlink > nul
@%LN% --symbolic %TESTROOTSRC%\A_SymlinkTargetRoot %TESTROOTSRC%\A_SymlinkRoot > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%LN% --symbolic %TESTROOTDST%\A_Symlink
@%LN% --symbolic %TESTROOTDST%\A_SymlinkRoot

REM
REM  210 --mirror -m +m
REM
@%RD% %TESTROOT%
@%MKDIR% %TESTROOTDST% > nul
@%MKDIR% %TESTROOTSRC% > nul
@%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTDST%\A_Mountpoint > nul
@%LN% --noitcnuj %VOLUME_GUID% %TESTROOTDST%\A_MountpointRoot > nul
@%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTSRC%\A_Mountpoint > nul
@%LN% --noitcnuj %VOLUME_GUID% %TESTROOTSRC%\A_MountpointRoot > nul
%LN% --splice --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@sort sortout
@%LN% --noitcnuj %TESTROOTDST%\A_Mountpoint
@%LN% --noitcnuj %TESTROOTDST%\A_MountpointRoot

@%RD% %TESTROOTDST%\A_Mountpoint
@%RD% %TESTROOTSRC%\A_Mountpoint
@%RD% %TESTROOTDST%\A_MountpointRoot
@%RD% %TESTROOTSRC%\A_MountpointRoot

@echo off
REM
REM Clean up
REM
:mountpoint_cleanup
@%RD% %TESTROOTSRCDRV%\tmp
@%RD% %TESTROOT% > nul
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit

REM
REM Final clean-up
REM
:ausmausraus
@%RD% %TESTROOT% > nul
