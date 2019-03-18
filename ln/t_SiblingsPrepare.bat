REM
REM Preparation for Siblings Test
REM

@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest
set SHARENAME=SiblingTest

%MKDIR% %TESTROOTSRC%\Folder0 > nul
%MKDIR% %TESTROOTSRC%\Folder1 > nul
%MKDIR% %TESTROOTSRC%\Folder1\Folder2 > nul
%MKDIR% %TESTROOTSRC%\Folder1\Folder3 > nul
%COPY% test\ln.h %TESTROOTSRC%\Folder0\A > nul
%LN% %TESTROOTSRC%\Folder0\A %TESTROOTSRC%\Folder1\B > nul
%LN% %TESTROOTSRC%\Folder0\A %TESTROOTSRC%\Folder1\Folder2\C > nul
%LN% %TESTROOTSRC%\Folder0\A %TESTROOTSRC%\Folder1\Folder3\D > nul
%LN% %TESTROOTSRC%\Folder0\A %TESTROOTSRC%\E > nul

if [%DEEPPATH%] == [] %RMTSHARE% \\%LH%\%SHARENAME%=%~dp0%TESTROOT% /GRANT Everyone:FULL > nul



