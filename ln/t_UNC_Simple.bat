@echo off
REM
REM 1: directory

REM
REM Check 
REM
call t_UNC_Simple_Preperation.bat %1 %2 %3
call t_IpAdress.bat
@echo off
for /f %%f in ('type .ipadress') do set IP_ADRESS=%%f

@echo on
REM
REM --copy UNC to UNC via Localhost
REM
%LN% --copy \\%LH%\%SHARENAME%\source \\%LH%\%SHARENAME%\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%

REM
REM --delorean UNC to UNC
REM
@del %TESTROOTSRC%\Folder0\0_D	
@%RD% %TESTROOTSRC%\_F\F0
%LN% --delorean \\%LH%\%SHARENAME%\source \\%LH%\%SHARENAME%\dest \\%LH%\%SHARENAME%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTBKP%

@%RD% %TESTROOTBKP% > nul
@%RD% %TESTROOTDST% > nul
@copy test\readme.txt "%TESTROOTSRC%\Folder0\0_D" > nul
@%MKDIR% %TESTROOTSRC%\_F\F0 > nul

REM
REM --copy Drive to UNC via Localhost
REM
%LN% --copy %TESTROOTSRC% \\%LH%\%SHARENAME%\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%


REM
REM --delorean Drive to UNC via Localhost
REM
@del %TESTROOTSRC%\Folder0\0_D	
@%RD% %TESTROOTSRC%\_F\F0
%LN% --delorean %TESTROOTSRC% \\%LH%\%SHARENAME%\dest \\%LH%\%SHARENAME%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTBKP%

@%RD% %TESTROOTBKP% > nul
@%RD% %TESTROOTDST% > nul
@copy test\readme.txt "%TESTROOTSRC%\Folder0\0_D" > nul
@%MKDIR% %TESTROOTSRC%\_F\F0 > nul


REM
REM --copy UNC to UNC via sharename, nolocaluncresolve
REM
@call :CheckSymbolicLinks %2 %TESTROOTSRC%
%LN% --nolocaluncresolve --copy \\%LH%\%SHARENAME%\source \\%LH%\%SHARENAME%\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%

REM
REM --delorean UNC to UNC via sharename, nolocaluncresolve
REM
@del %TESTROOTSRC%\Folder0\0_D	
@%RD% %TESTROOTSRC%\_F\F0
%LN% --nolocaluncresolve --delorean \\%LH%\%SHARENAME%\source \\%LH%\%SHARENAME%\dest \\%LH%\%SHARENAME%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTBKP%

@%RD% %TESTROOTBKP% > nul
@%RD% %TESTROOTDST% > nul

@REM repair
@copy test\readme.txt "%TESTROOTSRC%\Folder0\0_D" > nul
@%MKDIR% %TESTROOTSRC%\_F\F0 > nul

REM
REM --copy Drive to UNC via sharename, nolocaluncresolve
REM
%LN% --nolocaluncresolve --copy %TESTROOTSRC% \\%LH%\%SHARENAME%\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%

REM
REM --delorean Drive to UNC via sharename, nolocaluncresolve
REM
@del %TESTROOTSRC%\Folder0\0_D	
@%RD% %TESTROOTSRC%\_F\F0
@%COPY% test\resource.h "%TESTROOTSRC%\Folder0\0_C" > nul
%LN% --nolocaluncresolve --delorean %TESTROOTSRC% \\%LH%\%SHARENAME%\dest \\%LH%\%SHARENAME%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@%LISTSTREAMS% %TESTROOTBKP%\Folder0\0_C
@%LISTSTREAMS% %TESTROOTDST%\Folder0\0_C
@call :CheckSymbolicLinks %2 %TESTROOTBKP%

@%RD% %TESTROOTBKP% > nul
@%RD% %TESTROOTDST% > nul

@REM repair
@copy test\readme.txt %TESTROOTSRC%\Folder0\0_D > nul
@copy test\readme.txt %TESTROOTSRC%\Folder0\0_C > nul
@%MKDIR% %TESTROOTSRC%\_F\F0 > nul

REM
REM --delorean Drive to UNC via sharename, nolocaluncresolve, clean fails
REM
@%LN% --nolocaluncresolve --copy %TESTROOTSRC% \\%LH%\%SHARENAME%\dest > nul
@type test\y | @%CACLS% %TESTROOTDST%\Folder0\0_D /D "%USERNAME%" > nul
@del %TESTROOTSRC%\Folder0\0_D
%LN% --nolocaluncresolve --delorean %TESTROOTSRC% \\%LH%\%SHARENAME%\dest \\%LH%\%SHARENAME%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTBKP%

REM
REM --mirror Drive to UNC via sharename, nolocaluncresolve, clean fails
REM
%LN% --nolocaluncresolve --mirror %TESTROOTSRC% \\%LH%\%SHARENAME%\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%

@REM repair
@type test\y | @%CACLS% %TESTROOTDST%\Folder0\0_D /G "%USERNAME%":F > nul
@%RD% %TESTROOTDST% > nul
@%RD% %TESTROOTBKP% > nul

@copy test\readme.txt "%TESTROOTSRC%\Folder0\0_D" > nul



REM
REM --copy UNC to UNC via Ip-Adresse
REM
@%RD% %TESTROOTDST% > nul
%LN% --copy \\%IP_ADRESS%\%SHARENAME%\source \\%IP_ADRESS%\%SHARENAME%\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%

REM
REM --delorean UNC to UNC via sharename, nolocaluncresolve
REM
@del %TESTROOTSRC%\Folder0\0_D	
@%RD% %TESTROOTSRC%\_F\F0
%LN% --delorean \\%IP_ADRESS%\%SHARENAME%\source \\%LH%\%SHARENAME%\dest \\%LH%\%SHARENAME%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTBKP%

@%RD% %TESTROOTBKP% > nul
@%RD% %TESTROOTDST% > nul
@copy test\readme.txt "%TESTROOTSRC%\Folder0\0_D" > nul
@%MKDIR% %TESTROOTSRC%\_F\F0 > nul

REM
REM --recursive UNC test
REM
@%RD% %TESTROOTDST% > nul
%LN% --recursive \\%IP_ADRESS%\%SHARENAME%\source \\%IP_ADRESS%\%SHARENAME%\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%

REM
REM --copy UNC test with noexistant share
REM
@%RD% %TESTROOTDST% > nul
%LN% --copy \\%IP_ADRESS%\%SHARENAME%\source \\%IP_ADRESS%\%SHARENAME%_notthere\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM --delorean UNC 
REM
@%RD% %TESTROOTDST% > nul
@%LN% --copy \\%IP_ADRESS%\%SHARENAME%\source \\%IP_ADRESS%\%SHARENAME%\dest > nul
@%RD% \\%IP_ADRESS%\UNCsimple\dest\Folder0
%LN% --delorean \\%IP_ADRESS%\%SHARENAME%\source \\%IP_ADRESS%\%SHARENAME%\dest \\%IP_ADRESS%\%SHARENAME%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTBKP%

@del \\%LH%\%SHARENAME%\source\Folder1\1_C
@%RD% \\%IP_ADRESS%\%SHARENAME%\bk1
%LN% --delorean \\%IP_ADRESS%\%SHARENAME%\source \\%IP_ADRESS%\%SHARENAME%\dest \\%IP_ADRESS%\%SHARENAME%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%


REM
REM --copy Drive to Mapped Drive
REM
@net use %EMPTYTESTDRIVE% \\%LH%\%SHARENAME% > nul
@%RD% %TESTROOTBKP% > nul
@%RD% %TESTROOTDST% > nul
@copy test\readme.txt %TESTROOTSRC%\Folder0\0_D > nul
@call :CheckSymbolicLinks %2 %TESTROOTSRC%
%LN% --copy %TESTROOTSRC% %EMPTYTESTDRIVE%\dest > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckSymbolicLinks %2 %TESTROOTDST%

REM
REM --delorean Drive to Mapped Drive
REM
@del %TESTROOTSRC%\Folder0\0_D
@%RD% %TESTROOTSRC%\_F\F0
@%COPY% test\resource.h "%TESTROOTSRC%\Folder0\0_C" > nul

%LN% --delorean %TESTROOTSRC% %EMPTYTESTDRIVE%\dest %EMPTYTESTDRIVE%\bk1 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@%LISTSTREAMS% %TESTROOTBKP%\Folder0\0_C
@%LISTSTREAMS% %TESTROOTDST%\Folder0\0_C
@call :CheckSymbolicLinks %2 %TESTROOTBKP%

@%RD% %TESTROOTBKP% > nul
@%RD% %TESTROOTDST% > nul

@copy test\readme.txt %TESTROOTSRC%\Folder0\0_D > nul
@copy test\readme.txt "%TESTROOTSRC%\Folder0\0_C" > nul
@%MKDIR% %TESTROOTSRC%\_F\F0 > nul

@net use %EMPTYTESTDRIVE% /Delete > nul


REM
REM --probefs for UNC names & Drives
REM
@%RD% %TESTROOTDST% > nul
%LN% --probefs \\%IP_ADRESS%\%SHARENAME%
%LN% --probefs \\%IP_ADRESS%\%SHARENAME%

@echo off
REM 
REM Clean up
REM
if [%DEEPPATH%] == [] %NETSHAREDEL% %SHARENAME% > nul
@%RD% %TESTROOT% > nul
@echo on

@goto :EOF 

:CheckSymbolicLinks 
@if [%1] == [symbolic] ( 
  @%LN% --%1 "%2\_F\0_A"
  @%LN% --%1 "%2\_F\1_A"
  @%LN% --%1 "%2\_F\1_C"
)
@exit /b

