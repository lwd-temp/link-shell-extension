@echo off
REM
REM 1: directory

REM
REM prepare
REM
call t_Dupemerge01Preperation.bat %1 %2 %3
@echo off

set SHARENAME=UNCsimple
if [%DEEPPATH%] == [] %RMTSHARE% \\%LH%\%SHARENAME%=%~dp0%TESTROOT% /GRANT Everyone:FULL  > nul

