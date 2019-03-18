@echo off
set TESTROOT=\__C
set TESTROOTDRIVE=%~d0
set TESTROOTDST=%TESTROOTDRIVE%\__C

REM 
REM Preparation
REM
@%RD% %TESTROOT% > nul
%MKDIR% %TESTROOTDST% > nul
%MKDIR% %TESTROOTDST%\eigenedateien > nul
%MKDIR% %TESTROOTDST%\windows > nul
%MKDIR% %TESTROOTDST%\zzbackups > nul

%COPY% test\*.h %TESTROOTDST% > nul
%COPY% test\*.h %TESTROOTDST%\eigenedateien > nul
%COPY% test\*.h %TESTROOTDST%\windows > nul

@echo on
REM 
REM Delorean with exclude directory
REM
%LN% --excludedir "%TESTROOTDRIVE%\%TESTROOT%\\zzbackups" --excludedir "%TESTROOTDRIVE%\%TESTROOT%\\windows" --copy %TESTROOTDST%   %TESTROOTDST%\zzbackups\000 > nul

%LN% --excludedir "%TESTROOTDRIVE%\%TESTROOT%\\zzbackups" --delorean %TESTROOTDST%  %TESTROOTDST%\zzbackups\000 %TESTROOTDST%\zzbackups\001 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@echo.

@%LN% -s %TESTROOTDST%\zzbackups\000\resource.h
@%LN% -s %TESTROOTDST%\zzbackups\000\ln.h
@%LN% -s %TESTROOTDST%\zzbackups\001\resource.h
@%LN% -s %TESTROOTDST%\zzbackups\001\ln.h

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT% > nul
@echo on


