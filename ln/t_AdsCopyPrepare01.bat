REM
REM for Alternative Data Stream Copy
REM

@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest
set TESTROOTBKP=%TESTROOT%%DEEPPATH%\bkp


REM
REM A file with alternative data streams
REM
%MKDIR% %TESTROOTSRC% > nul
%COPY% test\ln.h %TESTROOTSRC%\A > nul
echo AlternativeDataStream > %TESTROOTSRC%\A:ADS1
echo Und noch Einer Weil es so gut geht > "%TESTROOTSRC%\A:Alternative Stream 2"
echo Und ein letzter > "%TESTROOTSRC%\A:Der Letzte Stream"

%LN% %TESTROOTSRC%\A %TESTROOTSRC%\A_hardlink > nul

REM alternative data streams on directories
REM
%MKDIR% %TESTROOTSRC%\StreamDir > nul
echo Alternative Data Stream on Direcotories > "%TESTROOTSRC%\StreamDir:Directory Streams"
%TIMESTAMP% --eawrite %TESTROOTSRC%\StreamDir test\ln.h > nul

echo Alternative Data Stream on Direcotories > "%TESTROOTSRC%:Directory Streams"
%TIMESTAMP% --eawrite %TESTROOTSRC% test\ln.h > nul

REM
REM A compressed file
REM
%COPY% test\ln.h %TESTROOTSRC%\CompressedFile > nul
compact /C %TESTROOTSRC%\CompressedFile > nul

%MKDIR% %TESTROOTSRC%\CompressedDir > nul
%COPY% test\ln.h %TESTROOTSRC%\CompressedDir\A > nul


