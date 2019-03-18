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
    REM --json --[unroll|splice|] --symbolic --recursive  
    set OPTION=--json --%5 --%3 --%2
  ) else (
    REM --json --[unroll|splice|] --copy 
    set OPTION=--json --%5 --%2
    set OPTION_MIRROR=--json --%5 --mirror
    set OPTION_DELOREAN=--json --%5 --delorean
  )
) else (
  REM --json --[unroll|splice|] --copy 
  set OPTION=--json --%5 --%2
  set OPTION_MIRROR=--json --%5 --mirror
  set OPTION_DELOREAN=--json --%5 --delorean
)

if [%4] == [relative] ( 
  SET ABS_REL=
) else (
  SET ABS_REL=--%4
)


@echo on
REM Straight forward Smart Copy/Clone
REM
@%RD% %TESTROOTDST%
@call :ChangePermissions %TESTROOTSRC% nul %3 /D "%USERNAME%" 
%LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@type sortout
@type sortout > json_tmp.log
@%JSLINT% json_tmp.log

REM Mirror & Delorean
REM
if [%2] == [copy] ( 
  %LN% %ABS_REL% %OPTION_MIRROR% %TESTROOTSRC% %TESTROOTDST% > sortout
  @type sortout
  @type sortout > json_tmp.log
  @%JSLINT% json_tmp.log
)

if [%2] == [copy] ( 
  %LN% %ABS_REL% %OPTION_DELOREAN% %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
  @type sortout
  @type sortout > json_tmp.log
  @%JSLINT% json_tmp.log
)

if [%2] == [copy] ( 
  @%RD% %TESTROOTBKP%
  %LN% %ABS_REL% --deloreanverbose %OPTION_DELOREAN% %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
  @type sortout
  @type sortout > json_tmp.log
  @%JSLINT% json_tmp.log
)

if [%2] == [copy] ( 
  @%RD% %TESTROOTBKP%
  @%LN% --symbolic --generatehardlinks 1023 %TESTROOTDST%\F0\F0_F1\F0_F1.txt > nul 
  %LN% %ABS_REL% %OPTION_DELOREAN% %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
  @type sortout
  @type sortout > json_tmp1023.log
  @%JSLINT% json_tmp1023.log
)

REM Truesize
REM
%LN% --json --truesize %TESTROOTSRC%> sortout
@type sortout
@type sortout > json_tmp.log
@%JSLINT% json_tmp.log

REM Small json output
REM
@%RD% %TESTROOTDST%
@%RD% %TESTROOTBKP%
@%RD% %TESTROOTSRC%\F1
@%RD% %TESTROOTSRC%\F2
@%RD% %TESTROOTSRC%\F3
@%RD% %TESTROOTSRC%\J4
%LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@type sortout
@type sortout > json_tmp.log
@%JSLINT% json_tmp.log


@REM
@REM Cleanup
@call :ChangePermissions %TESTROOTSRC% nul %3 /G "%USERNAME%":F
@%RD% %TESTROOT%


@goto :EOF 

:ChangePermissions
@REM Lock Directory Reparse Point
@type test\y | @%CACLS% %1\F2\F2_J0 /L %4 %5 >> %2
@REM Lock Directories. The 'root' and an arbitrary directory
@type test\y | @%CACLS% %1\F1\F1_F0\F1_F0_J0 /L %4 %5 >> %2

@if [%2] NEQ [nul] (
  @%GSAR% -i -s%LH%\%USERNAME% -rUSERNAME -o %2 > nul
)
@type %2
@if [%2] NEQ [nul] @del %2 > nul

@exit /b


