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
@echo off

REM
REM Delorean Copy
REM
%LN% --junction "%TESTROOTSRC%\Junction0"  "%TESTROOTSRC%\Folder0" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Junction0"  "%TESTROOTSRC%\Folder0" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\JunctionTobeChanged"  "%TESTROOTSRC%\Folder0" > nul
%LN% --junction "%TESTROOTSRC%\Folder1\Junction1"  "%TESTROOTSRC%\Folder0\Outer SymLink Target 1" > nul
%COPY% test\resource.h "%TESTROOTSRC%" > nul
%COPY% test\resource.h "%TESTROOTSRC%\SizeChangeButSameDate.txt" > nul

REM
REM Copy by default with relative symbolic links
REM
%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
%WHERE% *.* %TESTROOTDST% | sort 

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
REM Change the size of a file, but keep the date the same
REM
%COPY% test\y %TESTROOTSRC%\SizeChangeButSameDate.txt > nul
%TOUCH% --reference=test\resource.h %TESTROOTSRC%\SizeChangeButSameDate.txt > nul

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
%COPY% "%TESTROOTSRC%\Folder0\File B in Folder0" "%TESTROOTSRC%\Folder0\Additional File AA in Folder0"  > nul
%COPY% "%TESTROOTSRC%\Folder0\File B in Folder0" "%TESTROOTSRC%\Folder1\Folder3\Additional File AA in Folder3"  > nul

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
%MKDIR% "%TESTROOTSRC%\Sibbling0" > nul
%COPY% "%TESTROOTSRC%\Folder0\File B in Folder0" "%TESTROOTSRC%\Sibbling0\Additional Siblings AA in Sibbling0"  > nul
%LN% "%TESTROOTSRC%\Sibbling0\Additional Siblings AA in Sibbling0" "%TESTROOTSRC%\Sibbling0\Additional Siblings AB in Sibbling0"  > nul
%LN% "%TESTROOTSRC%\Sibbling0\Additional Siblings AA in Sibbling0" "%TESTROOTSRC%\Sibbling0\Additional Siblings AC in Sibbling0"  > nul

@echo on
REM
REM Do the timewarp
REM
%LN% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Check a few things
REM
@%LN% --symbolic "%TESTROOTBKP%\Folder1\SymLink to File C in Outer SymLink Target 1" 
@%LN% --junction "%TESTROOTBKP%\Folder1\JunctionTobeChanged" 
@%LN% --symbolic "%TESTROOTSRC%\Folder1\Folder2\Inner SymLink 1" 

REM
REM Do the timewarp again, to see how 'skipped' works in the statistics
REM
%LN% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@echo off
REM Clean up
REM 
@%RD% %TESTROOTSRC% 
@%RD% %TESTROOTDST% 
@%RD% %TESTROOTBKP%


@echo on
REM 
REM Only the attribute changes. See if AttribChangedButContentTimeSame gets copied.
REM 
@echo off
%MKDIR% "%TESTROOTSRC%\Dir" > nul
%COPY% test\resource.h "%TESTROOTSRC%\Dir\AttribChangedButContentTimeSame" > nul
%LN% --copy "%TESTROOTSRC%" "%TESTROOTDST%"  > nul
attrib +r "%TESTROOTSRC%\Dir\AttribChangedButContentTimeSame"
%TIMESTAMP% --backup "%TESTROOTDST%\Dir\AttribChangedButContentTimeSame" "%TESTROOTSRC%\Dir\AttribChangedButContentTimeSame"

@echo on
%LN% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%


@echo off
REM Clean up
REM 
@%RD% %TESTROOTSRC% 
@%RD% %TESTROOTDST% 
@%RD% %TESTROOTBKP%

@echo on
REM
REM Create a file, which has 1023 hardlinks, so that during copy
REM the error TOO_MANY_LINKS comes up
REM
@%MKDIR% "%TESTROOTSRC%\TooManyLinks" > nul
@%COPY% test\resource.h "%TESTROOTSRC%\TooManyLinks\ttt" > nul
@%LN% --symbolic --generatehardlinks 1023 %TESTROOTSRC%\TooManyLinks\ttt > nul 
@%LN% --copy "%TESTROOTSRC%\TooManyLinks" "%TESTROOTDST%\TooManyLinks"  > nul

@REM 
@REM Clean up testbackup for second copy
@REM
@%RD% %TESTROOTBKP%

REM
REM Do the timewarp again, to check the error messages already during clone of delorean
REM
%LN% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@REM
@REM Do the timewarp again, to check the error messages during mirror of delorean
@REM
@REM TBD

@REM Clean up
@REM
@%RD% %TESTROOT%

REM
REM Break the hardlink limit by inserting a copy into the chain
REM the error TOO_MANY_LINKS comes up
REM
@set TESTROOT=%1
@set TESTROOTSRC=%TESTROOT%\src
@set TESTROOTDST=%TESTROOT%\dst
@set TESTROOTBKP=%TESTROOT%\bk1
@set TESTROOTBK2=%TESTROOT%\bk2
@%MKDIR% %TESTROOT% > nul
@%MKDIR% %TESTROOTSRC% > nul
@%COPY% test\resource.h %TESTROOTSRC%\ln > nul
@%LN% --symbolic --generatehardlinks 5 %TESTROOTSRC%\ln > nul 
@%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@%LN% --delorean %TESTROOTSRC% %TESTROOTDST% %TESTROOTBKP% > sortout

REM splitting needed
REM
%LN% --1023safe --hardlinklimit 10 --delorean %TESTROOTSRC% %TESTROOTBKP% %TESTROOTBK2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@%LN% --list %TESTROOTBKP%\ln | sort
@echo.
@%LN% --list %TESTROOTBK2%\ln | sort

REM no splitting
REM
@%RD% %TESTROOTBK2%
%LN% --1023safe --hardlinklimit 18 --delorean %TESTROOTSRC% %TESTROOTBKP% %TESTROOTBK2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@%LN% --list %TESTROOTBKP%\ln | sort
@echo.
@%LN% --list %TESTROOTBK2%\ln | sort

REM inner set splitting
REM
@%RD% %TESTROOTBK2%
%LN% --1023safe --hardlinklimit 3 --delorean %TESTROOTSRC% %TESTROOTBKP% %TESTROOTBK2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@%LN% --list %TESTROOTBKP%\ln | sort
@echo.
@%LN% --list %TESTROOTBK2%\ln | sort

REM Try to copy from non extisting source directory
REM
@%RD% %TESTROOTBK2%
%LN% --delorean %TESTROOTSRC%_non_existant %TESTROOTBKP% %TESTROOTBK2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM Try to copy from non extisting backup directory
REM
@%RD% %TESTROOTBK2%
%LN% --delorean %TESTROOTSRC% %TESTROOTBKP%_non_existant %TESTROOTBK2% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%


@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT%
@echo on


:ausmausraus

