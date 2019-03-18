@echo off
REM -----------------

REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM Check SmartCopy/Clone with ReparsePoints
REM

call t_SmartCopy03Prepare.bat %1 %3 %5
set REPARSEOPT=--%3

if [%3] == [symbolic] ( 
  if [%2] == [recursive] ( 
    SET OPTION=--%3 --%2
  ) else (
    SET OPTION=--%2
  )
) else (
  set OPTION=--%2
)

if [%4] == [absolute] ( 
  SET ABS_REL=--%4
) else (
  SET ABS_REL=
)
@echo on

REM
REM Straight forward Smart Copy/Clone
REM
@%RD% %TESTROOTDST%
%LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
%WHERE% *.* %TESTROOTDST% | sort 

REM
REM Check Attributes of first Dir
REM 
@echo off
@for /F "tokens=1,2,3,4,5,6 delims= " %%i in ('%TIMESTAMP% %TESTROOTDST%') do (
  @set WRITE_TIME=%%i %%j
  @set CREATION_TIME=%%k %%l
  @set ACCESS_TIME=%%m %%n
)
if [%2] == [recursive] echo %WRITE_TIME%
if [%2] == [copy] echo %WRITE_TIME%
@%TIMESTAMP% --streamprobe "%TESTROOTDST%:AlterStream"
@%TIMESTAMP% --eaprobe %TESTROOTDST%
@echo on

@echo.
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Outer SymLink 1" 
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder2\Inner SymLink 1" 
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder2\Inner SymLink 2" 
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder2\Inner SymLink 3" 

@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder3\SymLink to File A in Folder2" 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder3\SymLink to File B in Folder2" 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder0\SymLink to File A in Folder2" 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder0\SymLink to File B in Folder2" 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder3\SymLink to File C in Folder2" 

  @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\SymLink to File A in Outer SymLink Target 1"  
  @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\SymLink to File B in Outer SymLink Target 1"  
  @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\SymLink to File C in Outer SymLink Target 1"  

  @if [%2] == [recursive] ( 
    @%LN% %REPARSEOPT% "%TESTROOTDST%\Folder0\Outer SymLink Target 1\File A in Outer SymLink Target 1"
  )
)

@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder2\Inner Dead SymLink 1" 
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder2\Inner Dead SymLink 2" 

@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Outer Dead SymLink 1" 
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Outer Dead SymLink 2" 

REM
REM Now ommit some outer dead junctions
REM
@%RD% %TESTROOTDST%
%LN% %ABS_REL% %OPTION% %TESTROOTSRC%\Folder1 %TESTROOTDST%\Folder1 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
%WHERE% *.* %TESTROOTDST% | sort

@echo.
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder2\Inner SymLink 1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder2\Inner SymLink 2"
@%LN% %REPARSEOPT% "%TESTROOTDST%\Folder1\Folder2\Inner SymLink 3"

:ausmausraus
@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo on
