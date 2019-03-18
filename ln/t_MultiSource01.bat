@echo off
REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM SUCC: Check Unroll of Junctions
REM
call t_Unroll01Prepare01.bat %1 %3 %5
set OPTION=--%2
set REPARSEOPT=--%3

if [%4] == [absolute] ( 
  SET ABS_REL=--%4
) else (
  SET ABS_REL=
)
@echo on

@REM goto MultiSourceDelorean
REM
REM Unroll is on, but multiple specified sources prevent from unrolling
REM so in the whole tree no unrolling needed
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F0 --copy %TESTROOTSRC%\F1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0_F1\F0_F1_J2"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0\F1_F0_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0\F1_F0_J1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F1\F1_F1_J1"

@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F0_F1\F0_F1_F0\F0_F1_F0.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0\F1_F0_F2\F1_F0_F2.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F1\F1_F1_F0\F1_F1_F0.syl"
)


@if [%2] NEQ [splice] ( 
  @echo.
  %WHERE%  *.* %TESTROOTDST% | sort
  @echo.
  %DIR% %TESTROOTDST% | sort
)

REM
REM Multiple sources are given for F1/ F2/, so unrolling is prevented, 
REM but to F0/ unrolling is necessary
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F1 --copy %TESTROOTSRC%\F2 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0\F1_F0_J1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F1\F1_F1_J1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_F3\F2_F2_F3_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_F3\F2_F2_F3_J1"
@if [%2] NEQ [splice] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0\F1_F1\F1_F1_J1"
) else (
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0\F1_F0_J0"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0"
)
@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0\F1_F0_F2\F1_F0_F2.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F1\F2_F1.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_F1\F2_F2_F1.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_F3\F2_F2_F3.syl"
  @if [%2] NEQ [splice] ( 
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F1\F1_F1_F0\F1_F1_F0.syl"
  ) else (
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F1_F1\F1_F1_F0\F1_F1_F0.syl"

  )
)

@if [%2] NEQ [splice] ( 
  @echo.
  %WHERE%  *.* %TESTROOTDST% | sort
  @echo.
  %DIR% %TESTROOTDST% | sort
)
REM
REM Multiple sources are given for F2/ F3/, so unrolling is prevented, 
REM but to F0/ and F1/ unrolling is necessary
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F3 --copy %TESTROOTSRC%\F2 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_F3\F2_F2_F3_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_F3\F2_F2_F3_J1"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3_F2\F3_F2_J1"

@if [%2] NEQ [splice] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0\F1_F0\F1_F0_J1"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0\F1_F0\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0\F1_F1\F1_F1_J1"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_J0"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J0"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J1"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_J1"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F1\F1_F1_J1"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1\F1_F0_J1"
) else (
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1"
)

@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F1\F2_F1.syl"
  
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_F1\F2_F2_F1.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_F2\F2_F2_F3\F2_F2_F3.syl"
  
  @if [%2] NEQ [splice] ( 
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0\F1_F0\F1_F0_F2\F1_F0_F2.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F2_J0\F1_F1\F1_F1_F0\F1_F1_F0.syl"

    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F1\F2_F1.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_F1\F2_F2_F1.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_F2\F1_F0_F2.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F1\F1_F1_F0\F1_F1_F0.syl"

    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J1\F1_F0_F2\F1_F0_F2.syl"
  )

  @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_F2\F3_F2.syl"
)

@if [%2] NEQ [splice] ( 
  @echo.
  %WHERE%  *.* %TESTROOTDST% | sort
  @echo.
  %DIR% %TESTROOTDST% | sort
)

REM
REM Delorean is on, but multiple specified sources prevent from unrolling
REM so in the whole tree no unrolling needed
REM
@%RD% %TESTROOTDST%
@%RD% %TESTROOTBKP%
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F0 --copy %TESTROOTSRC%\F1 %TESTROOTDST% > nul
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F0 --delorean %TESTROOTSRC%\F1 %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@%LN% %REPARSEOPT% "%TESTROOTBKP%\F0_F1\F0_F1_J2"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_F0\F1_F0_J0"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_F0\F1_F0_J1"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_F1\F1_F1_J1"

@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F0_F1\F0_F1_F0\F0_F1_F0.syl"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_F0\F1_F0_F2\F1_F0_F2.syl"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F1_F1\F1_F1_F0\F1_F1_F0.syl"
)


@if [%2] NEQ [splice] ( 
  @echo.
  %WHERE%  *.* %TESTROOTBKP% | sort
)

:MultiSourceDelorean
REM
REM Multiple sources are given for F2/ F3/, so unrolling is prevented, 
REM but to F0/ and F1/ unrolling is necessary
REM
@%RD% %TESTROOTDST%
@%RD% %TESTROOTBKP%
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F2 --copy %TESTROOTSRC%\F3 %TESTROOTDST% > nul
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F2 --delorean %TESTROOTSRC%\F3 %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_F2\F2_F2_F3\F2_F2_F3_J0"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_F2\F2_F2_F3\F2_F2_F3_J1"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_F2\F2_F2_J0"
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_F2\F3_F2_J1"

@if [%2] NEQ [splice] ( 
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_J0\F1_F0\F1_F0_J1"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_J0\F1_F0\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_J0\F1_F1\F1_F1_J1"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_F2\F2_F2_J0"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J0"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3_J1"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F0\F1_F0_J1"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F1\F1_F1_J1"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J1\F1_F0_J0\F0_F1_J2"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J1\F1_F0_J1"
) else (
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_J0"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J1"
)

@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_F1\F2_F1.syl"
  
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_F2\F2_F2.syl"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_F2\F2_F2_F1\F2_F2_F1.syl"
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_F2\F2_F2_F3\F2_F2_F3.syl"
  
  @if [%2] NEQ [splice] ( 
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_J0\F1_F0\F1_F0_F2\F1_F0_F2.syl"
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_J0\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F2_J0\F1_F1\F1_F1_F0\F1_F1_F0.syl"

    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_F1\F2_F1.syl"
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_F2\F2_F2_F1\F2_F2_F1.syl"
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_F2\F2_F2_F3\F2_F2_F3.syl"
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F0\F1_F0_F2\F1_F0_F2.syl"
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F0\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F1\F1_F1_F0\F1_F1_F0.syl"

    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J1\F1_F0_J0\F0_F1_F0\F0_F1_F0.syl"
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J1\F1_F0_F2\F1_F0_F2.syl"
  )

  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_F2\F3_F2.syl"
)

@if [%2] NEQ [splice] ( 
  @echo.
  %WHERE%  *.* %TESTROOTBKP% | sort
  @echo.
  %DIR% %TESTROOTBKP% | sort
)



REM
REM Prepare a short example
REM
@%RD% %TESTROOT%
call t_Unroll01Prepare02.bat %1 %3 %5

REM
REM Multiple sources are given for F0/ F3/, so unrolling is prevented, 
REM but to F1/ and F2/ unrolling is necessary
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F0 --copy %TESTROOTSRC%\F3 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] NEQ [splice] ( 
 @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_J0"
) else (
 @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0"
)
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0_F1\F0_F1_J2"

@if [%3] == [symbolic] ( 
  @if [%2] NEQ [splice] ( 
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0.syl"
  ) 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F0_F1\F0_F1_F0\F0_F1_F0.syl"
)

REM
REM MS01_2: Multiple sources are given for F0/ F4. Check if F4 is created
REM
@%RD% %TESTROOTDST%
%LN% %OPTION% %ABS_REL% --source %TESTROOTSRC%\F0 --destination %TESTROOTDST%\F0 --copy %TESTROOTSRC%\J4 %TESTROOTDST%\J4 > sortout
@set ERRLEV=%errorlevel%
@echo on
@echo ErrorLevel == %ERRLEV%
@if [%2] NEQ [splice] ( 
 @%LN% %REPARSEOPT% %TESTROOTDST%\F4
)

:ausmausraus
@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo on
