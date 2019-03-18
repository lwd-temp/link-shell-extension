@echo off
REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM Tests for timestamp handling
REM

REM
REM Prepare Test
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\DST
%MKDIR% %TESTROOTSRC% > nul
%MKDIR% %TESTROOTDST% > nul
%MKDIR% %TESTROOTSRC%\tmp > nul
%COPY% test\ln.h %TESTROOTSRC%\tmp > nul

@echo on
REM 
REM Check if all timestamps are copied
REM With WindowsXP the last access time stamp handling is enabled, causing differences in the A: column of the logs below
REM 
@echo off
%MKDIR% "%TESTROOTSRC%\Dir" > nul
%COPY% test\resource.h "%TESTROOTSRC%\Dir\AllTimeStampstoCopy" > nul
@%TIMESTAMP% --writetime --accesstime --creationtime --ctime  4e1402f1 "%TESTROOTSRC%\Dir\AllTimeStampstoCopy"
@%TIMESTAMP% --writetime --accesstime --creationtime --ctime  4e1402f1 "%TESTROOTSRC%\Dir"

@echo on
@%TIMESTAMP% "%TESTROOTSRC%\Dir\AllTimeStampstoCopy"
@%TIMESTAMP% "%TESTROOTSRC%\Dir"

@REM
@REM The Access time and the creation time must not be copied, 
@REM thus not the same as the write time
@REM
%LN% --copy "%TESTROOTSRC%" "%TESTROOTDST%"  > nul
@echo off
@for /F "tokens=1,2 delims= " %%i in ('%TIMESTAMP% "%TESTROOTDST%\Dir\AllTimeStampstoCopy"') do (
  @echo %%i %%j
  @if [%%l] == [%%j] @echo FAIL_CREATIONTIME
  @if [%%n] == [%%j] @echo FAIL_ACCESSTIME
)
@for /F "tokens=1,2 delims= " %%i in ('%TIMESTAMP% "%TESTROOTDST%\Dir"') do (
  @echo %%i %%j
  @if [%%l] == [%%j] @echo FAIL_CREATIONTIME
  @if [%%n] == [%%j] @echo FAIL_ACCESSTIME
)
@echo on 
@%RD% %TESTROOTDST% 

%LN% --mirror "%TESTROOTSRC%" "%TESTROOTDST%"  > nul
@%TIMESTAMP% "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@%TIMESTAMP% "%TESTROOTSRC%\Dir"
@%RD% %TESTROOTDST% 

@REM
@REM With --backup we want to copy the timestamps anyway
@REM
%LN% --backup --copy "%TESTROOTSRC%" "%TESTROOTDST%"  > nul
@%TIMESTAMP% "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@%TIMESTAMP% "%TESTROOTSRC%\Dir"
@%RD% %TESTROOTDST% 
%LN% --backup --mirror "%TESTROOTSRC%" "%TESTROOTDST%"  > nul
@%TIMESTAMP% "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@%TIMESTAMP% "%TESTROOTSRC%\Dir"
@%RD% %TESTROOTDST% 

REM
REM Attrib tests
REM
REM 
@ATTRIB +s +h "%TESTROOTSRC%\Dir\AllTimeStampstoCopy"
@ATTRIB +s +h +r "%TESTROOTSRC%\tmp\ln.h"
%LN% --backup --copy "%TESTROOTSRC%" "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@ATTRIB "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@ATTRIB "%TESTROOTSRC%\tmp\ln.h"

REM the same again
REM
%LN% --backup --copy "%TESTROOTSRC%" "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@ATTRIB "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@ATTRIB "%TESTROOTSRC%\tmp\ln.h"

REM Now without --backup
REM
@%RD% %TESTROOTDST% 
%LN% --copy "%TESTROOTSRC%" "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@ATTRIB "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@ATTRIB "%TESTROOTSRC%\tmp\ln.h"

REM and repeat it
REM
%LN% --copy "%TESTROOTSRC%" "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@ATTRIB "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@ATTRIB "%TESTROOTSRC%\tmp\ln.h"

REM Now with --mirror
REM
@%RD% %TESTROOTDST% 
%LN% --mirror "%TESTROOTSRC%" "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@ATTRIB "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@ATTRIB "%TESTROOTSRC%\tmp\ln.h"

REM and repeat it
REM
%LN% --mirror "%TESTROOTSRC%" "%TESTROOTDST%"  > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@ATTRIB "%TESTROOTDST%\Dir\AllTimeStampstoCopy"
@ATTRIB "%TESTROOTSRC%\tmp\ln.h"


:ausmausraus
@echo off
REM 
REM Clean up
REM
%RD% %TESTROOT%

@echo on
