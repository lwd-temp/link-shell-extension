@echo off
REM
REM 1: directory
REM 2: traditional
REM 3: hardlinklimit
REM 4: unused
REM 5: unused

REM
REM Check 
REM
setLocal EnableDelayedExpansion
call t_SiblingsPrepare.bat %1
if [%2] == [traditional] ( 
  SET OPTION=--%2
) else (
  SET OPTION=
)

if [%3] NEQ [] ( 
  set "LinkLimit="&for /f "delims=0123456789" %%i in ("%3") do set LinkLimit=%%i
  if not defined LinkLimit (
    SET OPTION=%OPTION% --hardlinklimit %3
  )
)

@echo on
REM
REM Show Hardlink Sibblings
REM
%LN% %OPTION% --list %TESTROOTSRC%\Folder0\A > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM List Hardlinks via local UNC Path
REM
%LN% %OPTION% --list \\%LH%\%SHARENAME%\source\e > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM List Hardlinks via local UNC Path
REM
%LN% %OPTION% --list \\%IP_ADRESS%\%SHARENAME%\source\e > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM List Hardlinks referenced by Symbolic links
REM
@%LN% --symbolic %TESTROOTSRC%\Folder1\Folder2\C %TESTROOTSRC%\Folder1\Folder2\Sym_C > nul
%LN% %OPTION% --list %TESTROOTSRC%\Folder1\Folder2\C > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM
REM Add a few files for delorean Merge tests
REM 

@%COPY% test\ln.h %TESTROOTSRC%\Folder0\A_1 > nul
@%LN% %TESTROOTSRC%\Folder0\A_1 %TESTROOTSRC%\Folder0\A_2 > nul
@%LN% %TESTROOTSRC%\Folder0\A_1 %TESTROOTSRC%\Folder0\A_3 > nul
@%LN% %TESTROOTSRC%\Folder0\A_1 %TESTROOTSRC%\Folder1\A_4 > nul

@REM @%RD% %TESTROOTSRC%\Folder1
@REM @%RD% %TESTROOTSRC%\E
@REM @%RD% %TESTROOTSRC%\Folder0\A

@%COPY% test\ln.h %TESTROOTSRC%\Folder0\F > nul
@%COPY% test\ln.h %TESTROOTSRC%\Folder0\G > nul
@%COPY% test\ln.h %TESTROOTSRC%\Folder0\H > nul
@%COPY% test\ln.h %TESTROOTSRC%\Folder0\V > nul
@%ATTR% +r %TESTROOTSRC%\Folder0\V 
@%COPY% test\ln.h %TESTROOTSRC%\Folder0\K > nul

@%LN% --copy %TESTROOTSRC% %TESTROOTDST%\set1\bkp01 > nul
@%LN% --delorean %TESTROOTSRC% %TESTROOTDST%\set1\bkp01 %TESTROOTDST%\set1\bkp02 > nul
@%COPY% test\ln.h %TESTROOTSRC%\Folder0\Z > nul
@%LN% --delorean %TESTROOTSRC% %TESTROOTDST%\set1\bkp02 %TESTROOTDST%\set1\bkp03 > nul
@type test\y | cacls %TESTROOTDST%\set1\bkp03\Folder0\K /d "%USERNAME%" > nul

@%COPY% %TESTROOTSRC%\Folder0\H %TESTROOTSRC%\Folder0\I > nul
@%RD% %TESTROOTSRC%\Folder0\H  > nul
@%LN% --copy %TESTROOTSRC% %TESTROOTDST%\set2\bkp04 > nul
@%LN% --deeppathdelete %TESTROOTSRC%\Folder0\G  > nul
@%LN% --delorean %TESTROOTSRC% %TESTROOTDST%\set2\bkp04 %TESTROOTDST%\set2\bkp05 > nul
@%LN% --delorean %TESTROOTSRC% %TESTROOTDST%\set2\bkp04 %TESTROOTDST%\set2\bkp06 > nul

%LN% %OPTION% --merge %TESTROOTDST%\set1\bkp03 %TESTROOTDST%\set2\bkp04 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo.

REM Check the outcome
REM
@%LN% -s %TESTROOTDST%\set2\bkp04\Folder0\A_1
@%LN% -s %TESTROOTDST%\set2\bkp04\Folder0\F
@%LN% -s %TESTROOTDST%\set2\bkp04\Folder0\G
@%LN% -s %TESTROOTDST%\set1\bkp03\Folder0\H
@%LN% -s %TESTROOTDST%\set2\bkp04\Folder0\Z
@%LN% -s %TESTROOTDST%\set2\bkp04\Folder1\Folder3\D
@%LN% -s %TESTROOTDST%\set2\bkp06\Folder0\I
@%LN% -s %TESTROOTDST%\set2\bkp06\Folder0\V
@echo.
%LN% --list %TESTROOTDST%\set2\bkp04\Folder0\A_1 | sort
%LN% --list %TESTROOTDST%\set2\bkp04\Folder0\G | sort
%LN% --list %TESTROOTDST%\set1\bkp03\Folder0\H | sort
%LN% --list %TESTROOTDST%\set2\bkp04\Folder0\F | sort
%LN% --list %TESTROOTDST%\set2\bkp04\Folder0\Z | sort
%LN% --list %TESTROOTDST%\set2\bkp06\Folder0\I | sort
%LN% --list %TESTROOTDST%\set2\bkp06\Folder0\V | sort
@echo.
%ATTR% %TESTROOTDST%\set2\bkp06\Folder0\V

REM Merge already hardlinked folders
@REM
%LN% %OPTION% --merge %TESTROOTDST%\set1\bkp03 %TESTROOTDST%\set2\bkp04 > sortout
@set ERRLEV=%errorlevel%
@sort sortout

@type test\y | cacls %TESTROOTDST%\set1\bkp03\Folder0\K /g "%USERNAME%":F > nul

REM Delete files with ReadOnly from a hardlink-set with --deeppathdelete
@REM
%ATTR% +r %TESTROOTDST%\set2\bkp04\Folder0\A_1

@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp05\Folder0\A_3
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp05\Folder1\A_4
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp06\Folder0\A_1

@%RD% %TESTROOTDST%\set2\bkp04 > sortout
@set ERRLEV=%errorlevel%
@sort sortout

@REM To update the attributes a few bytes of a file must be read
@REM
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp05\Folder0\A_3
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp05\Folder1\A_4
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp06\Folder0\A_1

REM and see if the siblings keep their attributes
@REM
@%ATTR% %TESTROOTDST%\set2\bkp05\Folder0\A_3
@%ATTR% %TESTROOTDST%\set2\bkp05\Folder1\A_4
@%ATTR% %TESTROOTDST%\set2\bkp06\Folder0\A_1

REM Handling sibling attributes also in deloreancopy. A file which does not exist
REM in the source anymore, and if deleted during --delorean, must not change all
REM attributes of that file in past backups
REM
%ATTR% +r+s+h %TESTROOTDST%\set2\bkp06\E
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp06\E
@%ATTR% %TESTROOTDST%\set2\bkp06\E
@%TIMESTAMP% --readfile %TESTROOTDST%\set1\bkp02\E
@%ATTR% %TESTROOTDST%\set1\bkp02\E

@%RD% %TESTROOTSRC%\E
%LN% %OPTION% --delorean %TESTROOTSRC% %TESTROOTDST%\set2\bkp06 %TESTROOTDST%\set2\bkp07 > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo.

@%TIMESTAMP% --readfile %TESTROOTDST%\set1\bkp01\E
@%ATTR% %TESTROOTDST%\set1\bkp01\E
@%TIMESTAMP% --readfile %TESTROOTDST%\set1\bkp02\E
@%ATTR% %TESTROOTDST%\set1\bkp02\E
@%TIMESTAMP% --readfile %TESTROOTDST%\set1\bkp03\E
@%ATTR% %TESTROOTDST%\set1\bkp03\E
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp05\E
@%ATTR% %TESTROOTDST%\set2\bkp05\E
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp06\E
@%ATTR% %TESTROOTDST%\set2\bkp06\E
@echo.
%LN% --list %TESTROOTDST%\set2\bkp06\E | sort

@echo.
@echo.
@%TIMESTAMP% --readfile %TESTROOTDST%\set1\bkp01\Folder0\A_1
@%ATTR% %TESTROOTDST%\set1\bkp01\Folder0\A_1
@%TIMESTAMP% --readfile %TESTROOTDST%\set1\bkp02\Folder0\A_1
@%ATTR% %TESTROOTDST%\set1\bkp02\Folder0\A_1
@%TIMESTAMP% --readfile %TESTROOTDST%\set1\bkp03\Folder0\A_1
@%ATTR% %TESTROOTDST%\set1\bkp03\Folder0\A_1
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp05\Folder0\A_1
@%ATTR% %TESTROOTDST%\set2\bkp05\Folder0\A_1
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp06\Folder0\A_1
@%ATTR% %TESTROOTDST%\set2\bkp06\Folder0\A_1
@%TIMESTAMP% --readfile %TESTROOTDST%\set2\bkp07\Folder0\A_1
@%ATTR% %TESTROOTDST%\set2\bkp07\Folder0\A_1
@echo.
%LN% --list %TESTROOTDST%\set2\bkp07\Folder0\A_1 | sort
@echo.
%LN% --list %TESTROOTDST%\set1\bkp01\Folder0\A_1 | sort



@echo off
REM 
REM Clean up
REM
if [%DEEPPATH%] == [] %NETSHAREDEL% %SHARENAME% > nul
@%RD% %TESTROOT% > nul


