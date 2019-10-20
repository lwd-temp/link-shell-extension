@echo.
@echo ######## Source Indexing ######## 
@echo.
@setlocal enableextensions enabledelayedexpansion

@set GIT_SRC_INDEX=%~dp0..\tools\git_source_index.exe
@set PDBSTR=%~dp0..\tools\pdbstr.exe
@set GREP=%~dp0..\tools\grep.exe
@set GIT_SOURCE_SERVER=SOURCE_SERVER
@set LOGFILE=Git_IndexLog.txt
@set FILESTOUPLOAD=%~dp0FilesToUpload.txt

@pushd %~dp0
@%GIT_SRC_INDEX% "%~dp0..\Bin\x64\Release" /svc git /ev %GIT_SOURCE_SERVER% > %LOGFILE%
@%GIT_SRC_INDEX% "%~dp0..\Bin\win32\Release" /svc git /ev %GIT_SOURCE_SERVER% >> %LOGFILE%

@REM Run through all files in 'FilesToUpload.txt', check if .pdb and then see if the .pdb 
@REM contains a stream with SRCSRV. if all is fine INDEXERROR stays 0, otherwise becomes 1
@set INDEXERROR=0
@for /f "delims=" %%a in (%FILESTOUPLOAD%) do @(
  @set "Filename=%%a"
  @if not "!Filename:pdb=!"=="!Filename!" (
    if exist !Filename! (
	  @!PDBSTR! -r -p:!Filename! -i:!Filename!.str -s:srcsrv 
	  if exist !Filename!.str (
        @for /f "tokens=1,2 delims=:" %%b in ('@%GREP% -i "SRCSRV: ini" !Filename!.str') do @(
   	      @set Content=%%b
   	      @if not !Content!==SRCSRV set INDEXERROR=1
	    ) 
        @del !Filename!.str
	  ) else (
	      set INDEXERROR=1
	  )
	) else (
	    set INDEXERROR=1
	)
  )
)
popd
@if !INDEXERROR! == 0 goto :EOF
@echo.
@echo WARNING: Problems indexing debug symbols
@echo WARNING: Please check %LOGFILE%
@echo.
@pause

goto :EOF

