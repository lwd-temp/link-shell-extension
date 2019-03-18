@echo off
REM
REM 1: directory
REM 2: copy/recursive
REM 3: junction/symbolic
REM 4: absolute during copy
REM 5: unroll/splice


REM
REM Check a more complex structure
REM
call t_Unroll01Prepare02.bat %1 %3

@echo off
set REPARSEOPT=--%3
if [%3] == [symbolic] ( 
  if [%2] == [recursive] ( 
    REM --backup --[unroll|splice|] --symbolic --recursive  
    SET OPTION=--backup --%5 --%3 --%2
  ) else (
    REM --backup --[unroll|splice|] --copy 
    SET OPTION=--backup --%5 --%2
  )
) else (
  REM --backup --[unroll|splice|] --copy 
  set OPTION=--backup --%5 --%2
)

if [%4] == [relative] ( 
  SET ABS_REL=
) else (
  SET ABS_REL=--%4
)


REM
REM Change permissions on various files/junctions/symbolic links
REM

@echo on
REM
REM Straight forward Smart Copy/Clone
REM
@echo off
@%RD% %TESTROOTDST%
@attrib /L +h +s %TESTROOTSRC%\F1\F1_F0\F1_F0_J0 > nul
@attrib /L +h +s %TESTROOTSRC%\F2\F2_J0 > nul
@%MKDIR% %TESTROOTSRC%\F1\F1_F0\F1_F0_D0
call :ChangePermissions %TESTROOTSRC% nul %3 /D "%USERNAME%" 
@echo on
%LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
call :ChangePermissions %TESTROOTDST% outfile %3 
@echo off
call :ChangePermissions %TESTROOTDST% nul %3 /G "%USERNAME%":F

@attrib /L %TESTROOTDST%\F1\F1_F0\F1_F0_J0
@attrib /L %TESTROOTDST%\F2\F2_J0

@attrib /L -h -s %TESTROOTDST%\F1\F1_F0\F1_F0_J0 > nul
@attrib /L -h -s %TESTROOTDST%\F2\F2_J0 > nul

@%RD% %TESTROOTDST%



:ausmausraus
@echo off
REM 
REM Clean up
REM
call :ChangePermissions %TESTROOTSRC% nul %3 /G "%USERNAME%":F
@%RD% %TESTROOT%

@goto :EOF 

:ChangePermissions
@REM Lock Directory Reparse Point
@type test\y | @%CACLS% %1\F2\F2_J0 /L %4 %5 >> %2
@REM Lock Directories. The 'root' and an arbitrary directory
@type test\y | @%CACLS% %1\F1\F1_F0\F1_F0_D0 /L %4 %5 >> %2
@type test\y | @%CACLS% %1 /L %4 %5 >> %2

@if [%2] NEQ [nul] (
  @%GSAR% -i -s%LH%\%USERNAME% -rUSERNAME -o %2 > nul
)
@type %2
@if [%2] NEQ [nul] @del %2 > nul

@exit /b


:CheckReparsePoints
@%LN% %REPARSEOPT% %2\F2\F2_J0
@exit /b




