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
call t_Unroll01Prepare01.bat %1 %3

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
call :ChangePermissions %TESTROOTSRC% nul %3 /D "%USERNAME%" 
@echo on
%LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
call :ChangePermissions %TESTROOTDST% outfile %3 
@echo off
call :ChangePermissions %TESTROOTDST% nul %3 /G "%USERNAME%":F
@if [%4] == [keepsymlinkrelation] call :CheckReparsePoints %3 %TESTROOTDST% %5

REM
REM With mirror bend reparsepoint to different locations and mirror over
REM
if [%2] == [mirror] (
  echo on
  call ::ChangePermissionsSmall %TESTROOTSRC% nul %3 /G "%USERNAME%":F
  %RD% %TESTROOTSRC%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J0
  %LN% %ABS_REL% --%3 %TESTROOTSRC%\F2\F2_F2\F2_F2_F1 %TESTROOTSRC%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J0 > nul
  %RD% %TESTROOTSRC%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J1
  %LN% %ABS_REL% --%3 %TESTROOTSRC%\F2\F2_F1\F2_F1_F1 %TESTROOTSRC%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J1 > nul
  call :ChangePermissionsSmall %TESTROOTSRC% nul %3 /D "%USERNAME%" 
  
  REM Mirror and check if reparse point get copied over
  REM
  %LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
  @set ERRLEV=%errorlevel%
  @sort sortout
  @echo ErrorLevel == %ERRLEV%
  call :ChangePermissionsSmall %TESTROOTDST% outfile %3 
  @echo off
  call :ChangePermissions %TESTROOTDST% nul %3 /G "%USERNAME%":F
  @if [%4] == [keepsymlinkrelation] call :CheckReparsePoints %3 %TESTROOTDST% %5

  %LN% --%3 %TESTROOTDST%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J0
  %LN% --%3 %TESTROOTDST%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J1

  REM revert bending
  REM
  call ::ChangePermissionsSmall %TESTROOTSRC% nul %3 /G "%USERNAME%":F
  %RD% %TESTROOTSRC%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J0
  %LN% %ABS_REL% --%3 %TESTROOTSRC%\F2\F2_F2\F2_F2_F2 %TESTROOTSRC%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J0 > nul
  %RD% %TESTROOTSRC%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J1
  %LN% %ABS_REL% --%3 %TESTROOTSRC%\F2\F2_F1\F2_F1_F0 %TESTROOTSRC%\F2\F2_F2\F2_F2_F3\F2_F2_F3_J1 > nul
  call :ChangePermissionsSmall %TESTROOTSRC% nul %3 /D "%USERNAME%" 
)


@%RD% %TESTROOTDST%

@echo on
REM
REM Straight Copy/Clone with locked directories
REM
@echo off
call :ChangeDirectoryPermissions %TESTROOTSRC% nul %3 /D "%USERNAME%" 
@echo on
call :ChangeDirectoryPermissions %TESTROOTSRC% outfile %3 
%LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
call :ChangeDirectoryPermissions %TESTROOTDST% outfile %3 

@echo off
call :ChangeDirectoryPermissions %TESTROOTDST% nul %3 /G "%USERNAME%":F
call :ChangePermissions %TESTROOTDST% outfile %3 

call :ChangeDirectoryPermissions %TESTROOTSRC% nul %3 /G "%USERNAME%":F
call :ChangePermissions %TESTROOTDST% nul %3 /G "%USERNAME%":F
@%RD% %TESTROOTDST%


@echo on
REM
REM Unroll/Splice all junctions, small test
REM
%LN% %ABS_REL% %OPTION% %TESTROOTSRC%\F3\F3_J1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
call :ChangePermissionsSmallTest %TESTROOTDST% outfile %3 %5

@echo off
call :ChangePermissionsSmallTest  %TESTROOTDST% nul %3 %5 /G "%USERNAME%":F
@%RD% %TESTROOTDST%


@echo on
REM
REM Unroll/Splice all junctions, large test
REM
%LN% %ABS_REL% %OPTION% %TESTROOTSRC%\F3 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
call :ChangePermissionsLargeTest %TESTROOTDST% outfile %3 %5

@echo off
call :ChangePermissionsLargeTest %TESTROOTDST% nul %3 %5 /G "%USERNAME%":F
@if [%4] == [keepsymlinkrelation] call :CheckReparsePointsLargeTest %3 %TESTROOTDST% %5
@%RD% %TESTROOTDST%
@echo on

:ausmausraus
@echo off
REM 
REM Clean up
REM
call :ChangePermissions %TESTROOTSRC% nul %3 /G "%USERNAME%":F
@%RD% %TESTROOT%

@goto :EOF 

:ChangePermissionsSmallTest
@if [%4] == [splice] ( 
  @type test\y | @%CACLS% "%1\F1_F0_J0" /L %5 %6 >> %2
) else (
  @if [%3] == [symbolic] ( 
    @type test\y | @%CACLS% "%1\F1_F0_F2\F1_F0_F2.syl" /L %5 %6 >> %2
    @type test\y | @%CACLS% "%1\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl" /L %5 %6 >> %2
  )
  @type test\y | @%CACLS% "%1\F1_F0_J0\F0_F1_J2" /L %5 %6 >> %2
)
@type test\y | @%CACLS% "%1\F1_F0_J1" /L %5 %6 >> %2

@if [%2] NEQ [nul] (
  @%GSAR% -i -s%LH%\%USERNAME% -rUSERNAME -o %2 > nul
)
@type %2
@if [%2] NEQ [nul] @del %2 > nul

@exit /b

:ChangePermissionsLargeTest
@if [%4] == [splice] ( 
  @type test\y | @%CACLS% "%1\F3_J0" /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_J1" /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_F2\F3_F2_J1" /L %5 %6 >> %2
) else (
@type test\y | @%CACLS% "%1\F3_J1\F1_F0_J1" /L %5 %6 >> %2
@type test\y | @%CACLS% "%1\F3_J1\F1_F0_J0\F0_F1_J2" /L %5 %6 >> %2
@type test\y | @%CACLS% "%1\F3_J0\F2_J0\F1_F0\F1_F0_J1" /L %5 %6 >> %2
@type test\y | @%CACLS% "%1\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_J2" /L %5 %6 >> %2
@type test\y | @%CACLS% "%1\F3_J0\F2_J0\F1_F1\F1_F1_J1" /L %5 %6 >> %2
@type test\y | @%CACLS% "%1\F3_J0\F2_F2\F2_F2_J0" /L %5 %6 >> %2
@type test\y | @%CACLS% "%1\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J0" /L %5 %6 >> %2
@type test\y | @%CACLS% "%1\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J1" /L %5 %6 >> %2
@type test\y | @%CACLS% "%1\F3_F2\F3_F2_J1" /L %5 %6 >> %2
@if [%3] == [symbolic] ( 
  @type test\y | @%CACLS% "%1\F3_F2\F3_F2.syl" /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3.syl"  /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_J0\F2_F2\F2_F2_F1\F2_F2_F1.syl" /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_J0\F2_F1\F2_F1.syl" /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_J0\F2_J0\F1_F0\F1_F0_F2\F1_F0_F2.syl" /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_J0\F2_J0\F1_F1\F1_F1_F0\F1_F1_F0.syl" /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl" /L %5 %6 >> %2
  @type test\y | @%CACLS% "%1\F3_J0\F2_F2\F2_F2.syl" /L %5 %6 >> %2
)
)

@if [%2] NEQ [nul] (
  @%GSAR% -i -s%LH%\%USERNAME% -rUSERNAME -o %2 > nul
)
@type %2
@if [%2] NEQ [nul] @del %2 > nul

@exit /b

:ChangeDirectoryPermissions
@REM Lock a directory
@type test\y | @%CACLS% %1\F2\F2_F2 %4 %5 >> %2

@if [%2] NEQ [nul] (
  @%GSAR% -i -s%LH%\%USERNAME% -rUSERNAME -o %2 > nul
)
@type %2
@if [%2] NEQ [nul] @del %2 > nul

@exit /b


:ChangePermissions
@REM Lock Directory Reparse Point
@type test\y | @%CACLS% %1\F0\F0_F1\F0_F1_J2 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F1\F1_F0\F1_F0_J0 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F1\F1_F0\F1_F0_J1 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F1\F1_F1\F1_F1_J1 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F2\F2_J0 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F2\F2_F2\F2_F2_J0 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F2\F2_F2\F2_F2_F3\F2_F2_F3_J0 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F2\F2_F2\F2_F2_F3\F2_F2_F3_J1 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F3\F3_J0 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F3\F3_J1 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F3\F3_F2\F3_F2_J1 /L %4 %5 >> %2

@if [%3] == [symbolic] ( 
  @REM Lock all symlinks
  @type test\y | @%CACLS% %1\F3\F3_F2\F3_F2.syl /L %4 %5 >> %2
  @type test\y | @%CACLS% %1\F2\F2_F2\F2_F2_F3\F2_F2_F3.syl /L %4 %5 >> %2
  @type test\y | @%CACLS% %1\F2\F2_F2\F2_F2_F1\F2_F2_F1.syl /L %4 %5 >> %2
  @type test\y | @%CACLS% %1\F2\F2_F1\F2_F1.syl /L %4 %5 >> %2
  @type test\y | @%CACLS% %1\F1\F1_F1\F1_F1_F0\F1_F1_F0.syl /L %4 %5 >> %2
  @type test\y | @%CACLS% %1\F1\F1_F0\F1_F0_F2\F1_F0_F2.syl /L %4 %5 >> %2
  @type test\y | @%CACLS% %1\F0\F0_F1\F0_F1_F0\F0_F1_F0.syl /L %4 %5 >> %2
  @type test\y | @%CACLS% %1\F2\F2_F2\F2_F2.syl /L %4 %5 >> %2

  @REM Lock all symlink destination files
  @type test\y | @%CACLS% %1\F3\F3_F2\F3_F2_F0\F3_F2_F0.txt %4 %5 >> %2
  @type test\y | @%CACLS% %1\F2\F2_F2\F2_F2_F2\F2_F2_F2.txt %4 %5 >> %2
  @type test\y | @%CACLS% %1\F2\F2_F1\F2_F1_F1\F2_F1_F1.txt %4 %5 >> %2
  @type test\y | @%CACLS% %1\F1\F1_F1\F1_F1.txt %4 %5 >> %2
  @type test\y | @%CACLS% %1\F0\F0_F1\F0_F1.txt %4 %5 >> %2
  @type test\y | @%CACLS% %1\F1\F1_F0\F1_F0.txt %4 %5 >> %2
  @type test\y | @%CACLS% %1\F0\F0_F1\F0_F1_F1\F0_F1_F1.txt %4 %5 >> %2
  @type test\y | @%CACLS% %1\F0\F0.txt %4 %5 >> %2
) 

@if [%2] NEQ [nul] (
  @%GSAR% -i -s%LH%\%USERNAME% -rUSERNAME -o %2 > nul
)
@type %2
@if [%2] NEQ [nul] @del %2 > nul

@exit /b

:ChangePermissionsSmall
@REM Lock Directory Reparse Point
@type test\y | @%CACLS% %1\F2\F2_F2\F2_F2_F3\F2_F2_F3_J0 /L %4 %5 >> %2
@type test\y | @%CACLS% %1\F2\F2_F2\F2_F2_F3\F2_F2_F3_J1 /L %4 %5 >> %2

@if [%2] NEQ [nul] (
  @%GSAR% -i -s%LH%\%USERNAME% -rUSERNAME -o %2 > nul
)
@type %2
@if [%2] NEQ [nul] @del %2 > nul

@exit /b



:CheckReparsePoints
@%LN% %REPARSEOPT% %2\F0\F0_F1\F0_F1_J2
@%LN% %REPARSEOPT% %2\F1\F1_F0\F1_F0_J0
@%LN% %REPARSEOPT% %2\F1\F1_F0\F1_F0_J1
@%LN% %REPARSEOPT% %2\F1\F1_F1\F1_F1_J1
@%LN% %REPARSEOPT% %2\F2\F2_J0
@%LN% %REPARSEOPT% %2\F2\F2_F2\F2_F2_J0
@%LN% %REPARSEOPT% %2\F2\F2_F2\F2_F2_F3\F2_F2_F3_J0
@%LN% %REPARSEOPT% %2\F2\F2_F2\F2_F2_F3\F2_F2_F3_J1
@%LN% %REPARSEOPT% %2\F3\F3_J0
@%LN% %REPARSEOPT% %2\F3\F3_J1
@%LN% %REPARSEOPT% %2\F3\F3_F2\F3_F2_J1

@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% %2\F3\F3_F2\F3_F2.syl
  @%LN% %REPARSEOPT% %2\F2\F2_F2\F2_F2_F3\F2_F2_F3.syl
  @%LN% %REPARSEOPT% %2\F2\F2_F2\F2_F2_F1\F2_F2_F1.syl
  @%LN% %REPARSEOPT% %2\F2\F2_F1\F2_F1.syl
  @%LN% %REPARSEOPT% %2\F1\F1_F1\F1_F1_F0\F1_F1_F0.syl
  @%LN% %REPARSEOPT% %2\F1\F1_F0\F1_F0_F2\F1_F0_F2.syl
  @%LN% %REPARSEOPT% %2\F0\F0_F1\F0_F1_F0\F0_F1_F0.syl
  @%LN% %REPARSEOPT% %2\F2\F2_F2\F2_F2.syl
) 

@exit /b


:CheckReparsePointsLargeTest
@%LN% %REPARSEOPT% "%2\F3_F2\F3_F2_J1"
@if [%3] == [splice] ( 
    @%LN% %REPARSEOPT% "%2\F3_J0"
    @%LN% %REPARSEOPT% "%2\F3_J1"
  @if [%1] == [symbolic] ( 
    @%LN% %REPARSEOPT% "%2\F3_F2\F3_F2.syl"
  )
) else (
  @%LN% %REPARSEOPT% "%2\F3_J1\F1_F0_J1"
  @%LN% %REPARSEOPT% "%2\F3_J1\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F0\F1_F0_J1"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F1\F1_F1_J1"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_J0"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J0"
  @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J1"
  @if [%1] == [symbolic] ( 
    @%LN% %REPARSEOPT% "%2\F3_F2\F3_F2.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2_F1\F2_F2_F1.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_F1\F2_F1.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F0\F1_F0_F2\F1_F0_F2.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F1\F1_F1_F0\F1_F1_F0.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%2\F3_J0\F2_F2\F2_F2.syl"
  )
)
@exit /b


