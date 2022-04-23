@echo off
REM -----------------

REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\source
set TESTROOTDST=%TESTROOT%\dest
set TESTROOTBKP=%TESTROOT%\bk1

call t_SmartCopy03Prepare.bat %1 %3 %5

REM
REM SmartMirror 
REM
@echo off
%LN% --junction "%TESTROOTSRC%\Junction0"  "%TESTROOTSRC%\Folder0" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Junction0"  "%TESTROOTSRC%\Folder0" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\JunctionTobeChanged"  "%TESTROOTSRC%\Folder0" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Junction1"  "%TESTROOTSRC%\Folder0\Outer SymLink Target 1" > nul
copy test\resource.h "%TESTROOTSRC%" > nul
@echo on

REM
REM Copy by default with relative symbolic links
REM
@%RD% %TESTROOTDST%
%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
@echo on
@echo ErrorLevel == %ERRLEV%

@echo off
REM Delete a few files in the source
REM
del "%TESTROOTSRC%\Folder0\File A in Folder0" > nul
del "%TESTROOTSRC%\Folder0\SymLink to File A in Folder2" > nul

REM
REM Delete a whole directory in the source
REM
@%RD% "%TESTROOTSRC%\Folder1\Folder3\Inner SymLink Target 3" > nul

REM
REM Make a file newer in the source
REM
%TOUCH% "%TESTROOTSRC%\resource.h" > nul

REM
REM Make a hardlink newer in the source, which means, that all siblings
REM alo get copied, because siblings do only have one timestamp
REM
%TOUCH% "%TESTROOTSRC%\Folder1\Folder2\File A in Folder2" > nul

REM
REM Make a symbolic link file newer in the source, which changes the timestamp
REM of the symlink target
REM
%TOUCH% "%TESTROOTSRC%\Folder1\SymLink to File A in Outer SymLink Target 1" > nul

REM
REM Make a symbolic link file newer in the source, This only works if we delete
REM and create it newly
REM
del /q /f "%TESTROOTSRC%\Folder1\SymLink to File C in Outer SymLink Target 1" > nul
%LN% --symbolic "%TESTROOTSRC%\Folder0\Outer SymLink Target 1\File C in Outer SymLink Target 1" "%TESTROOTSRC%\Folder1\SymLink to File C in Outer SymLink Target 1"  > nul

REM
REM Make a junction newer in the source, This only works if we delete
REM and create it newly
REM
rd /q "%TESTROOTSRC%\Folder1\Junction1" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Junction1"  "%TESTROOTSRC%\Folder0\Outer SymLink Target 1" > nul

REM
REM Make a junction point to something completely different even
REM if it has the same name
REM
rd /q "%TESTROOTSRC%\Folder1\JunctionTobeChanged" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\JunctionTobeChanged"  "%TESTROOTSRC%\Folder1\Folder3" > nul

REM
REM Make a symbolic link directory point to something completely different even
REM if it has the same name
REM
rd /q "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 1" > nul
%LN% --symbolic "%TESTROOTSRC%\Folder0" "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 1"  > nul


REM
REM Add a few files to the source
REM
Copy "%TESTROOTSRC%\Folder0\File B in Folder0" "%TESTROOTSRC%\Folder0\Additional File AA in Folder0"  > nul
Copy "%TESTROOTSRC%\Folder0\File B in Folder0" "%TESTROOTSRC%\Folder1\Folder3\Additional File AA in Folder3"  > nul

REM
REM Add a few symbolic link files in the source
REM
%LN% --absolute --symbolic "%TESTROOTSRC%\Folder0\File B in Folder0" "%TESTROOTSRC%\Folder1\Folder2\Additional Symbolic Link to File B in Folder0"  > nul

REM
REM Add a few Junctions in the source
REM
%LN% --junction "%TESTROOTSRC%\Folder1\Junction2"  "%TESTROOTSRC%\Folder1\Folder3" > nul

REM
REM Delete a junction in the source
REM
rd /q "%TESTROOTSRC%\Folder1\Junction0" > nul

REM
REM Delete a symbolic link directory in the source
REM
rd /q "%TESTROOTSRC%\Folder1\Outer SymLink 1" > nul

REM
REM Add total new hardlink sibblings 
REM
mkdir "%TESTROOTSRC%\Sibbling0" > nul
Copy "%TESTROOTSRC%\Folder0\File B in Folder0" "%TESTROOTSRC%\Sibbling0\Additional Siblings AA in Sibbling0"  > nul
%LN% "%TESTROOTSRC%\Sibbling0\Additional Siblings AA in Sibbling0" "%TESTROOTSRC%\Sibbling0\Additional Siblings AB in Sibbling0"  > nul
%LN% "%TESTROOTSRC%\Sibbling0\Additional Siblings AA in Sibbling0" "%TESTROOTSRC%\Sibbling0\Additional Siblings AC in Sibbling0"  > nul
@%TIMESTAMP% --writetime --accesstime --creationtime --ctime  4e1402f1 "%TESTROOTSRC%"

@echo on
REM
REM Do the SmartMirror
REM
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Check Attributes of first Dir
REM 
@echo off
@for /F "tokens=1,2,3,4,5,6 delims= " %%i in ('%TIMESTAMP% %TESTROOTDST%') do (
  @set WRITE_TIME=%%i %%j
  @set CREATION_TIME=%%k %%l
  @set ACCESS_TIME=%%m %%n
)
echo %WRITE_TIME%  %CREATION_TIME%  %ACCESS_TIME%
@%TIMESTAMP% --streamprobe "%TESTROOTDST%:AlterStream"
@%TIMESTAMP% --eaprobe %TESTROOTDST%
@echo on

REM
REM Do the Symbolic CloneMirror
REM
@%RD% %TESTROOTDST%
%LN% --symbolic --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTSRC%\Folder0
%LN% --symbolic --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@del %TESTROOTDST%\resource.h > nul
@copy test\resource.h %TESTROOTDST% > nul
%LN% --symbolic --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM Do the Hard CloneMirror
REM
@%RD% %TESTROOTDST%
%LN% --recursive --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTSRC%\Folder1\Folder2
@%RD% %TESTROOTSRC%\Folder1\Folder3
%LN% --recursive --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTSRC%\Folder1
@del %TESTROOTDST%\resource.h > nul
@copy test\resource.h %TESTROOTDST% > nul
%LN% --recursive --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%


REM Try to mirror from non existing directory
REM
%LN% --mirror %TESTROOTSRC%_non_existant %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

:ausmausraus

@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%
echo on



