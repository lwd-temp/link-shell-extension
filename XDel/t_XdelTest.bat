call t_Settings.bat

REM
REM Only delete reparse points
REM
@echo off
set TESTROOT=test\reparse
set TESTROOTSRC=%TESTROOT%\source

mkdir %TESTROOTSRC%\Folder0 > nul
copy test\resource.h "%TESTROOTSRC%\Folder0\File A in Folder0" > nul
%LN% --noitcnuj "%TESTROOTSRC%\Folder0" "%TESTROOTSRC%\junc" > nul
%LN% --symbolic "%TESTROOTSRC%\Folder0" "%TESTROOTSRC%\junc" > nul

mkdir %TESTROOTSRC%\Folder0\F1 > nul
copy test\resource.h %TESTROOTSRC%\Folder0\F1 > nul
mkdir %TESTROOTSRC%\Folder0\F1\F1_F1 > nul
mkdir %TESTROOTSRC%\Folder0\F2 > nul
copy test\resource.h %TESTROOTSRC%\Folder0\F2 > nul
mkdir %TESTROOTSRC%\Folder0\F3 > nul
@echo on

pause

REM %XDEL% --reparse %TESTROOTSRC% 
@echo ErrorLevel == %errorlevel%
%WHERE% *.* %TESTROOTDST% | sort

@echo off
REM 
REM Clean up
REM
%RD% %TESTROOTSRC%
@echo on

