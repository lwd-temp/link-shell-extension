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
@set TESTROOTDST_SAVE=%TESTROOTDST%
@set TESTROOTDST=%TESTROOT%%DEEPPATH%\1\2\3\4\dest
@set TESTROOTSRC_SAVE=%TESTROOTSRC%

@echo off
set REPARSEOPT=--%3

if [%5] == [] ( 
  SET UNROLL_SPLICE=
) else (
  SET UNROLL_SPLICE=--%5
)

if [%3] == [symbolic] ( 
  if [%2] == [recursive] ( 
    REM --[unroll|splice|] --symbolic --recursive  
    SET OPTION=%UNROLL_SPLICE% --%3 --%2
  ) else (
    REM --[unroll|splice|] --copy 
    SET OPTION=%UNROLL_SPLICE% --%2
  )
) else (
  if [%2] == [recursive] ( 
    REM --[unroll|splice|] --symbolic --recursive
    SET OPTION=%UNROLL_SPLICE% --symbolic --%2
  ) else (
    REM --[unroll|splice|] --copy 
    set OPTION=%UNROLL_SPLICE% --%2
  )
)

if [%4] == [relative] ( 
  SET ABS_REL=
) else (
  SET ABS_REL=--%4
)

REM
REM Copy from network drive
REM

REM Setup loopback network connection
REM
set SHARENAME=nwdrv01
if [%DEEPPATH%] == [] %RMTSHARE% \\%LH%\%SHARENAME%=%~dp0%TESTROOTSRC% /GRANT Everyone:FULL  > nul
@net use %EMPTYTESTDRIVE% \\%LH%\%SHARENAME% > nul
set TESTROOTSRC=%EMPTYTESTDRIVE%\

@echo on
REM
REM Straight forward Smart Copy/Clone
REM
@echo off
@echo on
%LN% %ABS_REL% %OPTION% %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@%LN% %REPARSEOPT% "%TESTROOTDST%\F0\F0_F1\F0_F1_J2"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_F0\F1_F0_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F2\F2_J0"
@%LN% %REPARSEOPT% "%TESTROOTDST%\F3\F3_J0"

@if [%3] == [symbolic] ( 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F1\F1_F0\F1_F0.syl"
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F0\F0_F1\F0_F1_F0\F0_F1_F0.syl"
  
  @set REPARSEOPT_TMP=%REPARSEOPT%
) else (
  @set REPARSEOPT_TMP=--symbolic
)
@if [%2] == [recursive] ( 
  @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F0\F0.txt
  @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F0\F0_F1\F0_F1.txt
  @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F0\F0_F1\F0_F1_F1\F0_F1_F1.txt
  @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F1\F1.txt
  @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F1\F1_F0\F1_F0.txt
)

REM
REM Multiple sources are given for F0/ F3/, so unrolling is prevented, 
REM but to F1/ and F2/ unrolling is necessary
REM
@%RD% %TESTROOTDST%
%LN% %ABS_REL% --source %TESTROOTSRC%F0 %OPTION% %TESTROOTSRC%F3 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@if [%5] NEQ [splice] ( 
 @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0_J0"
) else (
 @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0"
)
@%LN% %REPARSEOPT% "%TESTROOTDST%\F0_F1\F0_F1_J2"

@if [%3] == [symbolic] ( 
  @if [%5] NEQ [splice] ( 
    @%LN% %REPARSEOPT% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0.syl"
    @set REPARSEOPT_TMP=%REPARSEOPT%
  ) 
  @%LN% %REPARSEOPT% "%TESTROOTDST%\F0_F1\F0_F1_F0\F0_F1_F0.syl"
) else (
  @set REPARSEOPT_TMP=--symbolic
)

@if [%2] == [recursive] ( 
  @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F0.txt
  @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F0_F1\F0_F1.txt
  @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F0_F1\F0_F1_F1\F0_F1_F1.txt
  @if [%5] == [unroll] ( 
    @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F3_J0\F2_J0\F1.txt
    @%LN% %REPARSEOPT_TMP% "%TESTROOTDST%\F3_J0\F2_J0\F1_F0\F1_F0.txt
  )
)

@if [%2] == [recursive] goto NoNetworkDelorean 
REM
REM Network Delorean:  
REM
@set TESTROOTBKP=%TESTROOT%%DEEPPATH%\1\2\3\4\bk1
@set OPTION=%OPTION:copy=delorean%
%LN% %ABS_REL% --source %TESTROOTSRC%F0 %OPTION% %TESTROOTSRC%F3 %TESTROOTDST% %TESTROOTBKP%> sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@if [%5] NEQ [splice] ( 
 @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F0\F1_F0_J0"
) else (
 @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0"
)
@%LN% %REPARSEOPT% "%TESTROOTBKP%\F0_F1\F0_F1_J2"

@if [%3] == [symbolic] ( 
  @if [%5] NEQ [splice] ( 
    @%LN% %REPARSEOPT% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F0\F1_F0.syl"
    @set REPARSEOPT_TMP=%REPARSEOPT%
  ) 
  @%LN% %REPARSEOPT% "%TESTROOTBKP%\F0_F1\F0_F1_F0\F0_F1_F0.syl"
) else (
  @set REPARSEOPT_TMP=--symbolic
)

@if [%2] == [recursive] ( 
  @%LN% %REPARSEOPT_TMP% "%TESTROOTBKP%\F0.txt
  @%LN% %REPARSEOPT_TMP% "%TESTROOTBKP%\F0_F1\F0_F1.txt
  @%LN% %REPARSEOPT_TMP% "%TESTROOTBKP%\F0_F1\F0_F1_F1\F0_F1_F1.txt
  @if [%5] == [unroll] ( 
    @%LN% %REPARSEOPT_TMP% "%TESTROOTBKP%\F3_J0\F2_J0\F1.txt
    @%LN% %REPARSEOPT_TMP% "%TESTROOTBKP%\F3_J0\F2_J0\F1_F0\F1_F0.txt
  )
)


@REM Cleanup
@REM
@net use %EMPTYTESTDRIVE% /Delete > nul
@if [%DEEPPATH%] == [] %NETSHAREDEL% %SHARENAME% > nul
@%RD% %TESTROOT%

REM DeloreanCopy with Network Drive as destination
REM
@REM Setup loopback network connection for network drive being the destination
@REM
@set TESTROOTDST=%TESTROOTDST_SAVE%
@%MKDIR% %TESTROOTDST%

@set SHARENAME=nwdrv01
@if [%DEEPPATH%] == [] %RMTSHARE% \\%LH%\%SHARENAME%=%~dp0%TESTROOTDST% /GRANT Everyone:FULL  > nul
@net use %EMPTYTESTDRIVE% \\%LH%\%SHARENAME% > nul
@set TESTROOTDST=%EMPTYTESTDRIVE%\dst
@set TESTROOTBKP=%EMPTYTESTDRIVE%\bkp

@set TESTROOTSRC=%TESTROOTSRC_SAVE%
@%MKDIR% %TESTROOTSRC%\F0 > nul
@%COPY% test\ln.h %TESTROOTSRC%\F0\F0.txt > nul
@%MKDIR% %TESTROOTSRC%\F0\F0_F1 > nul
@%COPY% test\ln.h %TESTROOTSRC%\F0\F0_F1\F0_F1.txt > nul

@REM --delorean mapped drive
@REM
@%LN% %ABS_REL% --copy %TESTROOTSRC% %TESTROOTDST%> nul
@del %TESTROOTSRC%\F0\F0.txt 
%LN% %ABS_REL% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

:NoNetworkDelorean

:ausmausraus
@echo off
REM 
REM Clean up
REM
net use %EMPTYTESTDRIVE% /Delete > nul
if [%DEEPPATH%] == [] %NETSHAREDEL% %SHARENAME% > nul
@%RD% %TESTROOT%





