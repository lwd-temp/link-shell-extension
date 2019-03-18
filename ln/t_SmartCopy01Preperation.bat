REM
REM Preparation for Junction test
REM

@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest

@if exist %TESTROOT% %RD% %TESTROOT% 

mkdir %TESTROOTSRC%\Folder0 > nul
mkdir "%TESTROOTSRC%\Folder0\Outer Junction Target 1" > nul
mkdir %TESTROOTSRC%\Folder1 > nul
mkdir %TESTROOTSRC%\Folder1\Folder2 > nul
mkdir %TESTROOTSRC%\Folder1\Folder3 > nul
mkdir "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 1" > nul
mkdir "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 2" > nul
mkdir "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 3" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\File A in Folder0" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\File B in Folder0" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\File C in Folder0" > nul

copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\File A in Folder3" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\File B in Folder3" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\File C in Folder3" > nul

copy test\ln.h "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder2\File B in Folder2" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder2\File C in Folder2" > nul

copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 1\File A in Inner Junction Target 1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 2\File A in Inner Junction Target 2" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 2\File B in Inner Junction Target 2" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 3\File A in Inner Junction Target 3" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 3\File B in Inner Junction Target 3" > nul
copy test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 3\File C in Inner Junction Target 3" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Outer Junction Target 1\File A in Outer Junction Target 1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Outer Junction Target 1\File B in Outer Junction Target 1" > nul
copy test\ln.h "%TESTROOTSRC%\Folder0\Outer Junction Target 1\File C in Outer Junction Target 1" > nul

%LN% --junction "%TESTROOTSRC%\Folder1\Outer Junction 1" "%TESTROOTSRC%\Folder0\Outer Junction Target 1" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Folder2\Inner Junction 1" "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 1"  > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Folder2\Inner Junction 2" "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 2" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Folder2\Inner Junction 3" "%TESTROOTSRC%\Folder1\Folder3\Inner Junction Target 3" > nul

%LN% "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" "%TESTROOTSRC%\Folder1\Folder3\Hardlink to File A in Folder2 Ref 3" > nul
%LN% "%TESTROOTSRC%\Folder1\Folder2\File B in Folder2" "%TESTROOTSRC%\Folder1\Folder3\Hardlink to File B in Folder2 Ref 3" > nul
%LN% "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" "%TESTROOTSRC%\Folder0\Hardlink to File A in Folder2 Ref 3" > nul
%LN% "%TESTROOTSRC%\Folder1\Folder2\File B in Folder2" "%TESTROOTSRC%\Folder0\Hardlink to File B in Folder2 Ref 3" > nul
%LN% "%TESTROOTSRC%\Folder1\Folder2\File C in Folder2" "%TESTROOTSRC%\Folder1\Folder3\Hardlink to File C in Folder2 Ref 2" > nul

%LN% "%TESTROOTSRC%\Folder1\Folder3\File C in Folder3" "%TESTROOTSRC%\Folder1\Folder3\Hardlink to File C in Folder3 Ref 2 Loopback" > nul

REM Create a dead inner junction
REM
mkdir "%TESTROOTSRC%\Folder1\Folder3\Inner Dead Junction Target 1" > nul
mkdir "%TESTROOTSRC%\Folder1\Folder3\Inner Dead Junction Target 2" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Folder2\Inner Dead Junction 1" "%TESTROOTSRC%\Folder1\Folder3\Inner Dead Junction Target 1" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Folder2\Inner Dead Junction 2" "%TESTROOTSRC%\Folder1\Folder3\Inner Dead Junction Target 2" > nul
rmdir "%TESTROOTSRC%\Folder1\Folder3\Inner Dead Junction Target 1" > nul
rmdir "%TESTROOTSRC%\Folder1\Folder3\Inner Dead Junction Target 2" > nul

REM Create a dead outer junction
REM
mkdir "%TESTROOTSRC%\Folder0\Outer Dead Junction Target 1" > nul
mkdir "%TESTROOTSRC%\Folder0\Outer Dead Junction Target 2" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Outer Dead Junction 1" "%TESTROOTSRC%\Folder0\Outer Dead Junction Target 1" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Outer Dead Junction 2" "%TESTROOTSRC%\Folder0\Outer Dead Junction Target 2" > nul
rmdir "%TESTROOTSRC%\Folder0\Outer Dead Junction Target 1" > nul
rmdir "%TESTROOTSRC%\Folder0\Outer Dead Junction Target 2" > nul

echo.
%LN% --junction "%TESTROOTSRC%\Folder1\Outer Junction 1" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Folder2\Inner Junction 1" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Folder2\Inner Junction 2" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Folder2\Inner Junction 3" > nul

@echo on
%WHERE%  *.* %TESTROOT% | sort

