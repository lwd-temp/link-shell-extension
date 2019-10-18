@echo.
@echo ######## Symbolserver upload ######## 
@echo.

@if [X%1] NEQ [X] goto upoload
  echo ### Error: Argument 1 with Version missing
  exit /b 1

:upoload
pushd %~dp0

@set SYMTOOL=%~dp0..\tools\symstore.exe
@set GREP=%~dp0..\tools\grep.exe
@set SYMSTORE=%~dp0..\.symserv
@set LOGFILE=SymServUpload.txt

%SYMTOOL% add /o /compress /s %SYMSTORE% /f @FilesToUpload.txt /t "Link Shell Extension" /v %1 /d %LOGFILE%

%GREP% -i "Number of errors" %LOGFILE% 
@set /p TRANSACTION_ID=<%SYMSTORE%\000Admin\LastId.txt
@echo TRANSACTION_ID: %TRANSACTION_ID%

popd

