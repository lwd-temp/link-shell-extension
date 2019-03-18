REM -----------------

REM
REM Check regular expressions
REM
@set TESTROOT=test\regexp
@set TESTROOTSRC=%TESTROOT%\source
@set TESTROOTDST=%TESTROOT%\dest

@REM
@REM Test Preperation. Nice site to verify expressions is: https://www.regex101.com
@REM
@%RD% %TESTROOT%
@xcopy test\Poi %TESTROOTSRC%\ /s /e /h /k /c /q > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\file.h > nul
@echo on

@REM
@REM Copy by default with relative symbolic links
@REM
REM --exclude
REM
%LN% --exclude *.txt --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --exclude *.h --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --exclude *.rc --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --exclude *.cxx --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --exclude *.cxx --exclude *.h --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@REM Check Casesensitivness in extensions
@%RD% %TESTROOTDST%
%LN% --exclude *.cXx --exclude *.h --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@REM Check Casesensitivness in first letter: history.txt vs History.txt
@%RD% %TESTROOTDST%
%LN% --exclude History.txt --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@set FILTER_FILE=filter.txt
@echo *.cxx> %FILTER_FILE%
@echo *.h>> %FILTER_FILE%
%LN% --exclude @%FILTER_FILE% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@del %FILTER_FILE%

@REM Test empty lines in filter file
@%RD% %TESTROOTDST%
@set FILTER_FILE=filter.txt
@echo *.cxx> %FILTER_FILE%
@echo.>> %FILTER_FILE%
@echo.    >> %FILTER_FILE%
@echo.		>> %FILTER_FILE%
@echo *.h>> %FILTER_FILE%
%LN% --exclude @%FILTER_FILE% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@del %FILTER_FILE%

@%RD% %TESTROOTDST%
%LN% --exclude *.cXx --exclude *.H --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM --excluderegexp
REM
@%RD% %TESTROOTDST%
%LN% --excluderegexp "cxx$" --copy %TESTROOTSRC% %TESTROOTDST%  > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excluderegexp "^poi.*" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excluderegexp "^poi.+" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM --excludedir
REM 
@%RD% %TESTROOTDST%
%LN% --excludedir "s?c" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@set FILTER_FILE=filter.txt
@echo s?c> %FILTER_FILE%
%LN% --excludedir @%FILTER_FILE% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@del %FILTER_FILE%

@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Src\nction.cxx > nul
@%RD% %TESTROOTDST%
%LN% --exclude *nction.cxx --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@%MKDIR% %TESTROOTSRC%\src2
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Src2 > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\xyz.src > nul
%LN% --excludedir "src" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@%MKDIR% %TESTROOTSRC%\src2\subfolder
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\src2\subfolder > nul
%LN% --excludedir "src*" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir "src" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir "src*" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir "src2" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM --include
REM 
@%RD% %TESTROOTDST%
%LN% --include *.cxx --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%


@%RD% %TESTROOTDST%
%LN% --include *.cxx --exclude *n.cxx --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@set FILTER_FILE=filter.txt
@echo *.cxx> %FILTER_FILE%
%LN% --include @%FILTER_FILE% --exclude *n.cxx --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@del %FILTER_FILE%

REM --includedir
REM 
@%RD% %TESTROOTDST%
%LN% --includedir src --excludedir src2 --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@set FILTER_FILE=filter.txt
@echo src> %FILTER_FILE%
%LN% --includedir @%FILTER_FILE% --excludedir src2 --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@del %FILTER_FILE%

@%RD% %TESTROOTDST%
@%LN% --symbolic %TESTROOTSRC%\Res %TESTROOTSRC%\ResSymlink > sortout
@%LN% --noitcnuj %TESTROOTSRC%\Res %TESTROOTSRC%\ResJunction > sortout
%LN% --symbolic %TESTROOTSRC%\ResSymlink
%LN% --noitcnuj %TESTROOTSRC%\ResJunction
%LN% --include *.cxx --exclude *n.cxx --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@%MKDIR% %TESTROOTSRC%\src2\supfolder
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\src2\supfolder > nul
@%MKDIR% %TESTROOTSRC%\src2\supvolder
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\src2\supvolder > nul
@%MKDIR% %TESTROOTSRC%\src2\subvolder
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\src2\subvolder > nul
%LN% --includedir src --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir su??older --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@%MKDIR% %TESTROOTSRC%\src\supfolder
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\src\supfolder > nul
@%MKDIR% %TESTROOTSRC%\src\supvolder
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\src\supvolder > nul
@%MKDIR% %TESTROOTSRC%\src\subvolder
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\src\subvolder > nul
@%MKDIR% %TESTROOTSRC%\src\subfolder
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\src\subfolder > nul

@echo on
%LN% --excludedir u??older --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir *u??older --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir "src2\\su??older" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir "rc2\\su??older" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir "*rc2\\su??older" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excludedir "src*\\su??older" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
@set CRAZY_FILE=w^(^)^[^]^{^}^-^+^$^^
@copy %TESTROOTSRC%\history.txt "%TESTROOTSRC%\%CRAZY_FILE%" > nul
%LN% --exclude "%CRAZY_FILE%" --excludedir sr* --excludedir re* --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@del "%TESTROOTSRC%\%CRAZY_FILE%"

@%RD% %TESTROOTDST%
@set CRAZY_FILE=file^~1.txt
@copy %TESTROOTSRC%\history.txt "%TESTROOTSRC%\%CRAZY_FILE%" > nul
%LN% --exclude "~*.txt" --excludedir sr* --excludedir re* --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@del "%TESTROOTSRC%\%CRAZY_FILE%"


@%MKDIR% %TESTROOTSRC%\Res\sub1\unter1
@%MKDIR% %TESTROOTSRC%\Res\sub1\unter2
@%MKDIR% %TESTROOTSRC%\Res\sub1\Res
@%MKDIR% %TESTROOTSRC%\Res\sub1\Res\subsubRes
@%MKDIR% %TESTROOTSRC%\Res\sub2\s2s1
@%MKDIR% %TESTROOTSRC%\Res\sub2\s2s2
@%MKDIR% %TESTROOTSRC%\Res\sub2\Res

@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub1\unter1 > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub1\unter2 > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub1\Res > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub1\Res\subsubRes > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub2\s2s1 > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub2\s2s2 > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub2\Res > nul

@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\history.cpp > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub1\unter1\history.cpp > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub1\unter2\history.cpp > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub1\Res\history.cpp > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub1\Res\subsubRes\history.cpp > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub2\s2s1\history.cpp > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub2\s2s2\history.cpp > nul
@copy %TESTROOTSRC%\history.txt %TESTROOTSRC%\Res\sub2\Res\history.cpp > nul


@%RD% %TESTROOTDST%
%LN% --exclude "*\\Res\\*\\history.t*" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --include "*\\Res\\*\\history.t*" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
%LN% --excluderegexpdir "\\Res\\.+\\unter?\\?" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%RD% %TESTROOTDST%
REM This is a test which not yet works. It is related to DEBUG_REGEXP_SEAN_MALONEY
REM
%LN% --includedir "Res\\sub*" --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@REM 
@REM Clean up
@REM
@%RD% %TESTROOT%


:ausmausraus
