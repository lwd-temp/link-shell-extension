REM
REM Preparation for Junction test
REM

@echo off


set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest
set TESTROOTBKP=%TESTROOT%\bk1

set OPTION_PREP=%2

if [%2] == [] ( 
  set OPTION_PREP=
) else (
  if [%3] == [] ( 
    set OPTION_PREP=--%2
  ) else (
    set OPTION_PREP=--%2 --%3
  )
)

@if exist %TESTROOT% %RD% %TESTROOT% 

mkdir %TESTROOTSRC%\outer > nul
mkdir %TESTROOTSRC%\outer\OF > nul

mkdir %TESTROOTSRC%\inner > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\outer\OF" "%TESTROOTSRC%\inner\outer_reparse" > nul

mkdir %TESTROOTSRC%\inner\_F > nul
mkdir %TESTROOTSRC%\inner\_F\F0 > nul

mkdir %TESTROOTSRC%\inner\Folder0 > nul
mkdir %TESTROOTSRC%\inner\Folder1 > nul

copy test\ln.h "%TESTROOTSRC%\inner\Folder0\0_A" > nul
copy test\resource.h "%TESTROOTSRC%\inner\Folder0\0_B" > nul
copy test\readme.txt "%TESTROOTSRC%\inner\Folder0\0_C" > nul
copy test\readme.txt "%TESTROOTSRC%\inner\Folder0\0_D" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\inner\Folder0\0_A" "%TESTROOTSRC%\inner\_F\0_A" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\inner\Folder0" "%TESTROOTSRC%\inner\inner_reparse" > nul

copy test\y "%TESTROOTSRC%\inner\Folder1\1_A" > nul
copy /b test\readme.txt + test\ln.vcproj "%TESTROOTSRC%\inner\Folder1\1_B" > nul
copy /b test\readme.txt + test\ln.vcproj "%TESTROOTSRC%\inner\Folder1\1_C" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\inner\Folder1\1_A" "%TESTROOTSRC%\inner\_F\1_A" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\inner\Folder1\1_C" "%TESTROOTSRC%\inner\_F\1_C" > nul


@echo on
%WHERE%  *.* %TESTROOT% | sort

