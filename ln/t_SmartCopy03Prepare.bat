REM
REM Preparation for Smartcopy test
REM

REM
REM 1: directory
REM 2: noitcnuj/symbolic
REM 3: absolute during creation

set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest
set TESTROOTBKP=%TESTROOT%%DEEPPATH%\bk1

REM @echo off

if [%3] == [] ( 
  set OPTION_PREP=--%2
) else (
  set OPTION_PREP=--%2 --%3
)
@%RD% %TESTROOT%

%MKDIR% %TESTROOTSRC%\Folder0 > nul
%MKDIR% "%TESTROOTSRC%\Folder0\Outer SymLink Target 1" > nul
%MKDIR% %TESTROOTSRC%\Folder1 > nul
%MKDIR% %TESTROOTSRC%\Folder1\Folder2 > nul
%MKDIR% %TESTROOTSRC%\Folder1\Folder3 > nul
%MKDIR% "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 1" > nul
%MKDIR% "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 2" > nul
%MKDIR% "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 3" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder0\File A in Folder0" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder0\File B in Folder0" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder0\File C in Folder0" > nul

%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\File A in Folder3" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\File B in Folder3" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\File C in Folder3" > nul

%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder2\File B in Folder2" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder2\File C in Folder2" > nul

%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 1\File A in Inner SymLink Target 1" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 2\File A in Inner SymLink Target 2" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 2\File B in Inner SymLink Target 2" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 3\File A in Inner SymLink Target 3" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 3\File B in Inner SymLink Target 3" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 3\File C in Inner SymLink Target 3" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder0\Outer SymLink Target 1\File A in Outer SymLink Target 1" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder0\Outer SymLink Target 1\File B in Outer SymLink Target 1" > nul
%COPY% test\ln.h "%TESTROOTSRC%\Folder0\Outer SymLink Target 1\File C in Outer SymLink Target 1" > nul

%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder0\Outer SymLink Target 1" "%TESTROOTSRC%\Folder1\Outer SymLink 1" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 1" "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 1" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 2" "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 2" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 3" "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 3" > nul

%LN% "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" "%TESTROOTSRC%\Folder1\Folder3\Hardlink to File A in Folder2 Ref 3" > nul
%LN% "%TESTROOTSRC%\Folder1\Folder2\File B in Folder2" "%TESTROOTSRC%\Folder1\Folder3\Hardlink to File B in Folder2 Ref 3" > nul
%LN% "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" "%TESTROOTSRC%\Folder0\Hardlink to File A in Folder2 Ref 3" > nul
%LN% "%TESTROOTSRC%\Folder1\Folder2\File B in Folder2" "%TESTROOTSRC%\Folder0\Hardlink to File B in Folder2 Ref 3" > nul
%LN% "%TESTROOTSRC%\Folder1\Folder2\File C in Folder2" "%TESTROOTSRC%\Folder1\Folder3\Hardlink to File C in Folder2 Ref 2" > nul

@if [%2] == [symbolic] ( 
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" "%TESTROOTSRC%\Folder1\Folder3\SymLink to File A in Folder2" > nul
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder2\File B in Folder2" "%TESTROOTSRC%\Folder1\Folder3\SymLink to File B in Folder2" > nul
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" "%TESTROOTSRC%\Folder0\SymLink to File A in Folder2" > nul
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder2\File B in Folder2" "%TESTROOTSRC%\Folder0\SymLink to File B in Folder2" > nul
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder2\File C in Folder2" "%TESTROOTSRC%\Folder1\Folder3\SymLink to File C in Folder2" > nul

  %LN% %OPTION_PREP% "%TESTROOTSRC%\Folder0\Outer SymLink Target 1\File A in Outer SymLink Target 1" "%TESTROOTSRC%\Folder1\SymLink to File A in Outer SymLink Target 1"  > nul
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Folder0\Outer SymLink Target 1\File B in Outer SymLink Target 1" "%TESTROOTSRC%\Folder1\SymLink to File B in Outer SymLink Target 1"  > nul
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Folder0\Outer SymLink Target 1\File C in Outer SymLink Target 1" "%TESTROOTSRC%\Folder1\SymLink to File C in Outer SymLink Target 1"  > nul
)

%LN% "%TESTROOTSRC%\Folder1\Folder3\File C in Folder3" "%TESTROOTSRC%\Folder1\Folder3\Hardlink to File C in Folder3 Ref 2 Loopback" > nul

REM Create a dead inner symlink
REM
%MKDIR% "%TESTROOTSRC%\Folder1\Folder3\Inner Dead SymLink Target 1" > nul
%MKDIR% "%TESTROOTSRC%\Folder1\Folder3\Inner Dead SymLink Target 2" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder3\Inner Dead SymLink Target 1" "%TESTROOTSRC%\Folder1\Folder2\Inner Dead SymLink 1" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder3\Inner Dead SymLink Target 2" "%TESTROOTSRC%\Folder1\Folder2\Inner Dead SymLink 2" > nul
%RD% "%TESTROOTSRC%\Folder1\Folder3\Inner Dead SymLink Target 1" > nul
%RD% "%TESTROOTSRC%\Folder1\Folder3\Inner Dead SymLink Target 2" > nul

REM Create a dead outer symlink
REM
%MKDIR% "%TESTROOTSRC%\Folder0\Outer Dead SymLink Target 1" > nul
%MKDIR% "%TESTROOTSRC%\Folder0\Outer Dead SymLink Target 2" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder0\Outer Dead SymLink Target 1" "%TESTROOTSRC%\Folder1\Outer Dead SymLink 1" > nul
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder0\Outer Dead SymLink Target 2" "%TESTROOTSRC%\Folder1\Outer Dead SymLink 2" > nul
%RD% "%TESTROOTSRC%\Folder0\Outer Dead SymLink Target 1" > nul
%RD% "%TESTROOTSRC%\Folder0\Outer Dead SymLink Target 2" > nul

@%TIMESTAMP% --streamwrite AlterNativeStream "%TESTROOTSRC%:AlterStream"
@%TIMESTAMP% --eawrite %TESTROOTSRC% test\ln.h > nul
@%TIMESTAMP% --writetime --accesstime --creationtime --ctime  4e1402f1 "%TESTROOTSRC%"

@echo.
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Outer SymLink 1" 
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 1"
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 2"
%LN% %OPTION_PREP% "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 3"
echo.
@echo on
@%WHERE% *.* %TESTROOT% | sort

