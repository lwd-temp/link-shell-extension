@echo off
REM -----------------

REM
REM 1: directory
REM 2: move/smartrename
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM Check Junction and Symlink mixed SmartMove
REM

call t_SmartMove02Prepare.bat %1 %3 %5
set OPTION=--%2
set REPARSEOPT=--%3

if [%4] == [absolute] ( 
  SET ABS_REL=--%4
) else (
  SET ABS_REL=
)
@echo on

REM
REM SM02_T01 Move a whole linked branch
REM
@%RD% %TESTROOTDST%
%LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@if [%2] == [smartrename] ( 
  @call :CheckReparsePoints %TESTROOTSRC% %3
) else ( 
  %WHERE% *.* %TESTROOTDST% | sort 
  @call :CheckReparsePoints %TESTROOTDST% %3
)

@if [%2] NEQ [smartrename] ( 
  REM
  REM SM02_T02 Test if tree with dead junctions/symlinks can be moved
  REM
  @%LN% %ABS_REL% %OPTION% %TESTROOTDST% %TESTROOTSRC% > nul
  @%RD% %TESTROOTSRC%\Base\Folder0\Sufo0
  @%RD% %TESTROOTSRC%\Base\Folder1\Sufo2

  %LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
  @echo off
  set ERRLEV=%errorlevel%
  sort sortout
  @echo on
  @echo ErrorLevel == %ERRLEV%
  @call :CheckReparsePoints %TESTROOTDST% %3
)


:ausmausraus
@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo on


@goto :EOF 

:CheckReparsePoints		
@echo.
@%LN% %REPARSEOPT% %1\Base\Folder1\Symlink_Fo1Sufo2 
@%LN% %REPARSEOPT% %1\Base\Symlink_Fo2Sufo0 
@%LN% %REPARSEOPT% %1\Base\Symlink_JuFo0  
@%LN% %REPARSEOPT% %1\Base\Folder2\Junction_Fo1Sufo4
@%LN% %REPARSEOPT% %1\Base\Folder1\Sufo2\Junction_Fo1JuFo0Sufo0
@%LN% %REPARSEOPT% %1\Base\Folder1\Sufo2\SufoSufo0\Junction_Fo0
@%LN% %REPARSEOPT% %1\Base\Folder1\Junction_Fo0Sufo0
@%LN% %REPARSEOPT% %1\Base\Folder1\Junction_Fo1Sufo2

@if [%2] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%1\Base\Folder0\Sufo0\Symlink File Fo0Sufo0" 
  @%LN% %REPARSEOPT% "%1\Base\Folder2\Symlink File Fo1Sufo4" 
)
@exit /b
