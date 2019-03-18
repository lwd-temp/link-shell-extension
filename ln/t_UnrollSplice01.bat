set OPTION=%2

REM -----------------

REM
REM SUCC: Check Unroll of Junctions
REM
call t_SmartCopy01Preperation.bat %1
@echo off
%MKDIR% "%TESTROOTSRC%\Folder0\Outer Junction Target 1\SubFolder" > nul
%COPY% test\resource.h "%TESTROOTSRC%\Folder0\Outer Junction Target 1\SubFolder" > nul

%MKDIR% "%TESTROOTSRC%\Folder0\Outer Junction Target 2" > nul
%COPY% test\resource.h "%TESTROOTSRC%\Folder0\Outer Junction Target 2" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Outer Junction 2" "%TESTROOTSRC%\Folder0\Outer Junction Target 2" > nul

%MKDIR% "%TESTROOTSRC%\Folder0\Outer Juncdion Target 1" > nul
%COPY% test\resource.h "%TESTROOTSRC%\Folder0\Outer Juncdion Target 1" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Outer Juncdion 1" "%TESTROOTSRC%\Folder0\Outer Juncdion Target 1" > nul

@echo on

REM
REM Unroll/Splice all junctions
REM
@%RD% %TESTROOTDST%
%LN% --%OPTION% --copy %TESTROOTSRC%\Folder1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% --junction "%TESTROOTDST%\Outer Junction 1"
@%LN% --junction "%TESTROOTDST%\Outer Juncdion 1"
@%LN% --junction "%TESTROOTDST%\Outer Junction 2"

REM
REM Unroll/Splice junctions via wildcards
REM
@%RD% %TESTROOTDST%
%LN% --%OPTION% "Outer Junction*" --copy %TESTROOTSRC%\Folder1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% --junction "%TESTROOTDST%\Outer Junction 1"
@%LN% --junction "%TESTROOTDST%\Outer Juncdion 1"
@%LN% --junction "%TESTROOTDST%\Outer Junction 2"

REM
REM Unroll/Splice junctions via wildcards
REM
@%RD% %TESTROOTDST%
%LN% --%OPTION% "* 1" --copy %TESTROOTSRC%\Folder1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% --junction "%TESTROOTDST%\Outer Junction 1"
@%LN% --junction "%TESTROOTDST%\Outer Juncdion 1"
@%LN% --junction "%TESTROOTDST%\Outer Junction 2"

REM
REM Add a few symbolic links
REM
@%LN% -s "%TESTROOTSRC%\Folder0\Outer Junction Target 1\File A in Outer Junction Target 1" "%TESTROOTSRC%\Folder1\Symlink File A in Outer Junction Target 1"
@%LN% -s "%TESTROOTSRC%\Folder0\Outer Junction Target 1\File B in Outer Junction Target 1" "%TESTROOTSRC%\Folder1\Folder2\Symlink File B in Outer Junction Target 1"
@%LN% -s "%TESTROOTSRC%\Folder0\Outer Junction Target 1\File C in Outer Junction Target 1" "%TESTROOTSRC%\Folder1\Folder3\Symlink File C in Outer Junction Target 1"

REM
REM Unroll/Splice all junctions and symbolic links
REM
@%RD% %TESTROOTDST%
%LN% --%OPTION% --copy %TESTROOTSRC%\Folder1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% --junction "%TESTROOTDST%\Outer Junction 1"
@%LN% --junction "%TESTROOTDST%\Outer Juncdion 1"
@%LN% --junction "%TESTROOTDST%\Outer Junction 2"

@%LN% --symbolic "%TESTROOTDST%\Symlink File A in Outer Junction Target 1"
@%LN% --symbolic "%TESTROOTDST%\Folder2\Symlink File B in Outer Junction Target 1"
@%LN% --symbolic "%TESTROOTDST%\Folder3\Symlink File C in Outer Junction Target 1"

REM
REM Unroll/Splice symbolic links via wildcards
REM
@%RD% %TESTROOTDST%
%LN% --%OPTION% "Symlink File A*" --copy %TESTROOTSRC%\Folder1 %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%LN% --junction "%TESTROOTDST%\Outer Junction 1"
@%LN% --junction "%TESTROOTDST%\Outer Juncdion 1"
@%LN% --junction "%TESTROOTDST%\Outer Junction 2"

@%LN% --symbolic "%TESTROOTDST%\Symlink File A in Outer Junction Target 1"
@%LN% --symbolic "%TESTROOTDST%\Folder2\Symlink File B in Outer Junction Target 1"
@%LN% --symbolic "%TESTROOTDST%\Folder3\Symlink File C in Outer Junction Target 1"

:ausmausraus
@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%

@echo on

