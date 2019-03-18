REM
REM Preparation for Junction test
REM

@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\000_@_Source
set TESTROOTDST=%TESTROOT%\000_@_SmartKopie

if exist %TESTROOT% %RD% %TESTROOT% > nul

mkdir %TESTROOTSRC%> nul
mkdir %TESTROOTSRC%\A_Test > nul
mkdir %TESTROOTSRC%\A_Test\AA_Test > nul
mkdir %TESTROOTSRC%\B_Test > nul
%LN% --junction %TESTROOTSRC%\B_Test\AA_Test %TESTROOTSRC%\A_Test\AA_Test > nul


echo.
%WHERE%  *.* %TESTROOT% | sort
echo.

@echo on
