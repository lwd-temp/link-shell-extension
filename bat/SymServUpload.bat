@echo.
@echo ######## Symbolserver upload ######## 
@echo.
@setlocal enabledelayedexpansion

@if [X%1] NEQ [X] goto upoload
  @echo ### Error: Argument 1 with Version missing
  @exit /b 1

:upoload
@pushd %~dp0

@set SYMTOOL=%~dp0..\tools\symstore.exe
@set GREP=%~dp0..\tools\grep.exe
@set SYMSTORE=%~dp0..\.symserv
@set LOGFILE=SymServUpload.txt

@%SYMTOOL% add /o /compress /s %SYMSTORE% /f @FilesToUpload.txt /t "Link Shell Extension" /v %1 /d %LOGFILE%
@set ERRLEV=%ERRORLEVEL%
@set /p TRANSACTION_ID=<%SYMSTORE%\000Admin\LastId.txt
@echo TRANSACTION_ID: %TRANSACTION_ID%

@for /f "tokens=1,2 delims==" %%a in ('@%GREP% -i "Number of errors" %LOGFILE%') do @set ERRORS=%%b
@call :TRIM ERRORS
@if [X%ERRORS%] == [X0] goto :EOF
@echo.
@echo WARNING: Problems uploading to Symbolserver
@echo Number of errors:  %ERRORS%
@echo WARNING: Please check %LOGFILE%
@echo.
@pause

@popd

goto :EOF

:TRIM
@SetLocal EnableDelayedExpansion
@Call :TRIMSUB %%%1%%
@EndLocal & set %1=%tempvar%
@GOTO :EOF

:TRIMSUB
@set tempvar=%*
@GOTO :EOF
