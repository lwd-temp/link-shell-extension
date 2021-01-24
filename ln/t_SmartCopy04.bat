@echo off
REM -----------------

REM
REM 1: directory
REM 2: copy/recursive/mirror
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM Check SmartCopy/Clone/Mirror with multiple destinations
REM

call t_SmartCopy04Prepare.bat %1 %3 %5
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
REM Smart Copy/Clone/Mirror with multiple destinations
REM
@%RD% %TESTROOTDST%
@if [%2] == [mirror] @mkdir %TESTROOTDST%
%LN% %ABS_REL% --source %TESTROOTSRC%\F0 --destination %TESTROOTDST%\F0 --source %TESTROOTSRC%\F1 --destination %TESTROOTDST%\F1 %OPTION% %TESTROOTSRC%\S1 %TESTROOTDST%\S1 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@echo.
@%LN% %REPARSEOPT% %TESTROOTDST%\F0\F0_J0


REM
REM Smart Copy/Clone/Mirror with multiple destinations
REM
@%RD% %TESTROOTDST%
@if [%2] == [mirror] @mkdir %TESTROOTDST%
%LN% %ABS_REL% --unroll --source %TESTROOTSRC%\F0 --destination %TESTROOTDST%\F0 --source %TESTROOTSRC%\F1 --destination %TESTROOTDST%\F1 %OPTION% %TESTROOTSRC%\S1 %TESTROOTDST%\S1 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@echo.
@%LN% %REPARSEOPT% %TESTROOTDST%\F0\F0_J0


REM
REM Smart Copy/Clone/Mirror with multiple sources and different source path length
REM
@%RD% %TESTROOTDST%
%LN% %ABS_REL% --source %TESTROOTSRC%\F20 %OPTION% %TESTROOTSRC%\F21xy %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@echo.
@%LN% %REPARSEOPT% %TESTROOTDST%\F20_F0\F20_F0_J0

REM
REM Delete a few items with --mirror
REM
@if [%2] == [mirror] (
	@%LN% %ABS_REL% --source %TESTROOTSRC%\F0 --destination %TESTROOTDST%\F0 --source %TESTROOTSRC%\F1 --destination %TESTROOTDST%\F1 %OPTION% %TESTROOTSRC%\S1 %TESTROOTDST%\S1 > nul

	@del "%TESTROOTSRC%\F1\F1_F0\F1_F0.h"
	@%RD% "%TESTROOTSRC%\F1\F1_F0"

	%LN% %ABS_REL% --source %TESTROOTSRC%\F0 --destination %TESTROOTDST%\F0 --source %TESTROOTSRC%\F1 --destination %TESTROOTDST%\F1 %OPTION% %TESTROOTSRC%\S1 %TESTROOTDST%\S1 > sortout
	@echo off
	set ERRLEV=%errorlevel%
	sort sortout
	@echo on
	@echo ErrorLevel == %ERRLEV%

	@echo.
	@%LN% %REPARSEOPT% %TESTROOTDST%\F0\F0_J0
)


:ausmausraus
@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo on
