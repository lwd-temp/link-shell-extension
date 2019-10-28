@echo.
@echo ######## Symbolserver delete ######## 
@echo.
@setlocal enabledelayedexpansion

@if [X%1] NEQ [X] goto delete
  @echo ### Error: TransactionID missing as argument
  @exit /b 1

:delete
@pushd %~dp0

@set SYMTOOL=%~dp0..\tools\symstore.exe
@set SYMSTORE=%~dp0..\.symserv
@set LOGFILE=SymServUpload.txt

@%SYMTOOL% del /i %1 /s %SYMSTORE% /d %LOGFILE%
