@echo off
REM
REM 1: directory
REM 2: copy/recursive
REM 3: junction/symbolic
REM 4: absolute during copy
REM 5: unroll/splice


REM
REM Check 
REM
call t_AdsCopyPrepare01.bat %1

REM
REM Copy Alternative Data Streams & EA Records
REM
@echo on
%TIMESTAMP% --eawrite %TESTROOTSRC%\A test\ln.h test\y test\readme.txt
%LN% --backup --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

more < %TESTROOTDST%\A:ADS1
more <  "%TESTROOTDST%\A:Alternative Stream 2"
more < "%TESTROOTDST%\A:Der Letzte Stream"
%TIMESTAMP% --eaprobe %TESTROOTDST%\A

more < "%TESTROOTDST%\StreamDir:Directory Streams"
%TIMESTAMP% --eaprobe %TESTROOTDST%\StreamDir

more < "%TESTROOTDST%:Directory Streams"
%TIMESTAMP% --eaprobe %TESTROOTDST%

REM Copy ADS to FAT drive. Will not work, because FAT can not hold ADS
REM 
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
@%TRUECRYPT% test\FatTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit
@echo on
%LN% --switchoffntfscheck --backup --copy %TESTROOTSRC% %TESTROOTSRCDRV%\test > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%RD% %TESTROOTSRCDRV%\test > nul 

REM Copy once more, but do not copy ADS & EA
@echo off
%LN% --switchoffntfscheck --noads --noea --backup --copy %TESTROOTSRC% %TESTROOTSRCDRV%\test > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
@%RD% %TESTROOTSRCDRV%\test > nul 

REM Cleanup
REM 
@%RD% %TESTROOTSRCDRV%\test > nul 
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit

REM
REM Check if chunks of data are correctly copied. The Chunk size is 16000000
REM
@%RD% %TESTROOTDST% > nul 
@set DUMMYFILE=..\tools\DummyFile.exe > nul
@set MD5=..\tools\md5.exe
@%DUMMYFILE% %TESTROOTSRC%\BigData 17000000 > nul
@for /F "tokens=4 delims= " %%i in ('%MD5% %TESTROOTSRC%\BigData') do @set SRCMD5=%%i
%LN% --backup --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@echo ErrorLevel == %ERRLEV%
@for /F "tokens=4 delims= " %%i in ('%MD5% %TESTROOTDST%\BigData') do @set DSTMD5=%%i
@if [%SRCMD5%] NEQ [%DSTMD5%] echo Copy Fail
@del %TESTROOTDST%\BigData

REM
REM Check if chunks are copied correctly. Chunk size: 16777216 == Internal Buffersize == 0x01000000
REM
@%RD% %TESTROOTDST% > nul 
@%DUMMYFILE% %TESTROOTSRC%\BigData 16777216 > nul
@for /F "tokens=4 delims= " %%i in ('%MD5% %TESTROOTSRC%\BigData') do @set SRCMD5=%%i
%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@echo ErrorLevel == %ERRLEV%
@for /F "tokens=4 delims= " %%i in ('%MD5% %TESTROOTDST%\BigData') do @set DSTMD5=%%i
@if [%SRCMD5%] NEQ [%DSTMD5%] echo Copy Fail
sort sortout
@del %TESTROOTDST%\BigData

REM
REM Check if chunks are copied correctly. Chunk size: 16777216 == Internal Buffersize == 0x01000000
REM
@%RD% %TESTROOTDST% > nul 
@%DUMMYFILE% %TESTROOTSRC%\BigData 18777216 > nul
@for /F "tokens=4 delims= " %%i in ('%MD5% %TESTROOTSRC%\BigData') do @set SRCMD5=%%i
%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@echo ErrorLevel == %ERRLEV%
@for /F "tokens=4 delims= " %%i in ('%MD5% %TESTROOTDST%\BigData') do @set DSTMD5=%%i
@if [%SRCMD5%] NEQ [%DSTMD5%] echo Copy Fail
sort sortout
@del %TESTROOTDST%\BigData

REM
REM Copy encrypted streams
REM
@set CIPHEROPT=/e /a
set CIPHEROPT=/e 
@cipher %CIPHEROPT% %TESTROOTSRC%\BigData > nul
@%TIMESTAMP% --eawrite %TESTROOTSRC%\BigData test\ln.h test\y test\readme.txt
echo AlternativeDataStream > %TESTROOTSRC%\BigData:ADS1
@%TIMESTAMP% --writetime --accesstime --creationtime --ctime  4e1402f1 %TESTROOTSRC%\BigData
%LN% --backup --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@echo ErrorLevel == %ERRLEV%
@%TIMESTAMP% %TESTROOTSRC%\BigData
more < %TESTROOTDST%\BigData:ADS1
%TIMESTAMP% --eaprobe %TESTROOTDST%\BigData
@%TIMESTAMP% --xattr %TESTROOTDST%\BigData
@%LISTSTREAMS% %TESTROOTDST%\BigData

@del %TESTROOTSRC%\BigData
@del %TESTROOTDST%\BigData

REM
REM Copy encrypted directories
REM
@%MKDIR% %TESTROOTSRC%\EncryptedDir
@%DUMMYFILE% %TESTROOTSRC%\EncryptedDir\EncData 170000 > nul
@cipher /e /s:%TESTROOTSRC%\EncryptedDir > nul
%LN% --backup --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@echo ErrorLevel == %ERRLEV%
@sort sortout
@%TIMESTAMP% --xattr %TESTROOTDST%\EncryptedDir
@%TIMESTAMP% --xattr %TESTROOTDST%\EncryptedDir\EncData
@%LISTSTREAMS% %TESTROOTDST%\EncryptedDir\EncData

REM
REM Copy encrypted directories without --backup
REM
@%RD% %TESTROOTDST%\EncryptedDir
%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
%WHERE% *.* %TESTROOTDST% 

@%RD% %TESTROOTSRC%\EncryptedDir
@%RD% %TESTROOTDST%\EncryptedDir

REM
REM Copy of copy 0 Bytes files, and 0 byte alternative streams
REM
@%MKDIR% %TESTROOTSRC%\zero
@%TOUCH% %TESTROOTSRC%\zero\zerobytes
@%TIMESTAMP% --streamwrite "" %TESTROOTSRC%\zero\zerobytes:zerostream
%LN% --backup --copy %TESTROOTSRC%\zero %TESTROOTDST%\zero > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@%LISTSTREAMS% %TESTROOTSRC%\zero\zerobytes
@%RD% %TESTROOTSRC%\zero
@%RD% %TESTROOTDST%\zero

REM
REM Copy sparse files and sparse alternative streams
REM
@%MKDIR% %TESTROOTSRC%\sparse
@%TIMESTAMP% --sparsecreate %TESTROOTSRC%\sparse\sparsefile
@%TIMESTAMP% --sparsecreate %TESTROOTSRC%\sparse\sparsefile:ads_sparse
%LN% --copy %TESTROOTSRC%\sparse %TESTROOTDST%\sparse > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@%TIMESTAMP% --xattr %TESTROOTDST%\sparse\sparsefile
%TIMESTAMP% --sparsemap %TESTROOTSRC%\sparse\sparsefile
%TIMESTAMP% --sparsemap %TESTROOTDST%\sparse\sparsefile
%TIMESTAMP% --sparsemap %TESTROOTDST%\sparse\sparsefile:ads_sparse
@%RD% %TESTROOTSRC%\sparse
@%RD% %TESTROOTDST%\sparse

REM
REM Check if overwrite works
REM
@%MKDIR% %TESTROOTSRC%\overwrite
@%MKDIR% %TESTROOTSRC%\overwrite\1
@%MKDIR% %TESTROOTSRC%\overwrite\2
@for %%a in (AAAA) do echo/|set /p ="%%a" > %TESTROOTSRC%\overwrite\1\example.txt
@for %%a in (BB) do echo/|set /p ="%%a" > %TESTROOTSRC%\overwrite\2\example.txt
%LN% --copy %TESTROOTSRC%\overwrite\2 %TESTROOTSRC%\overwrite\1 > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@echo on
@type %TESTROOTSRC%\overwrite\1\example.txt
@echo.
@%LISTSTREAMS% %TESTROOTSRC%\overwrite\1\example.txt

REM
REM Zerobyte overwrite
REM
@del %TESTROOTSRC%\overwrite\2\example.txt
@%TOUCH% %TESTROOTSRC%\overwrite\2\example.txt
@%TIMESTAMP% --streamwrite "" %TESTROOTSRC%\overwrite\2\example.txt:zerostream
%LN% --copy %TESTROOTSRC%\overwrite\2 %TESTROOTSRC%\overwrite\1 > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%
@echo on
@%LISTSTREAMS% %TESTROOTSRC%\overwrite\1\example.txt
@type %TESTROOTSRC%\overwrite\1\example.txt
@echo.


@%RD% %TESTROOTSRC%\overwrite > nul




REM
REM Check if compressed directories are copied
REM
@echo off
@%RD% %TESTROOTDST% > nul
@compact /S /C %TESTROOTSRC%\CompressedDir\*.* > nul
@compact /S /C %TESTROOTSRC%\CompressedDir > nul
@echo on
%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%

@echo off
%TIMESTAMP% --xattr %TESTROOTDST%\CompressedDir
%TIMESTAMP% --xattr %TESTROOTDST%\CompressedDir\A
%TIMESTAMP% --xattr %TESTROOTDST%\CompressedFile
%TIMESTAMP% --xattr %TESTROOTDST%\A
@echo on

REM Mirror and thus remove compression from source
REM
@echo off
@compact /S /U %TESTROOTSRC%\CompressedDir\*.* > nul
@compact /S /U %TESTROOTSRC%\CompressedDir > nul
@echo on
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%

@echo off
%TIMESTAMP% --xattr %TESTROOTDST%\CompressedDir
%TIMESTAMP% --xattr %TESTROOTDST%\CompressedDir\A
%TIMESTAMP% --xattr %TESTROOTDST%\CompressedFile
%TIMESTAMP% --xattr %TESTROOTDST%\A

REM Mirror and thus add compression to destination
REM
@echo off
@compact /S /C %TESTROOTSRC%\CompressedDir\*.* > nul
@compact /S /C %TESTROOTSRC%\CompressedDir > nul
@echo on
%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%TIMESTAMP% --xattr %TESTROOTDST%\CompressedDir
@%TIMESTAMP% --xattr %TESTROOTDST%\CompressedDir\A
@%TIMESTAMP% --xattr %TESTROOTDST%\CompressedFile
@%TIMESTAMP% --xattr %TESTROOTDST%\A

REM Change the compression of readonly destinations
REM
@compact /S /U %TESTROOTDST%\CompressedFile > nul
@attrib +r %TESTROOTDST%\CompressedFile > nul
@attrib +r %TESTROOTSRC%\CompressedFile > nul
@%TIMESTAMP% --backup %TESTROOTSRC%\CompressedFile %TESTROOTDST%\CompressedFile

@compact /S /U %TESTROOTDST%\CompressedDir > nul
@attrib +r %TESTROOTDST%\CompressedDir > nul
@attrib +r %TESTROOTSRC%\CompressedDir > nul
@%TIMESTAMP% --backup %TESTROOTSRC%\CompressedDir %TESTROOTDST%\CompressedDir

@REM %TIMESTAMP% --xattr %TESTROOTSRC%\CompressedDir
@REM %TIMESTAMP% --xattr %TESTROOTSRC%\CompressedFile
@REM %TIMESTAMP% --xattr %TESTROOTDST%\CompressedDir
@REM %TIMESTAMP% --xattr %TESTROOTDST%\CompressedFile
@REM attrib %TESTROOTSRC%\CompressedDir
@REM attrib %TESTROOTSRC%\CompressedFile

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%TIMESTAMP% --xattr %TESTROOTDST%\CompressedDir
@%TIMESTAMP% --xattr %TESTROOTDST%\CompressedFile
@attrib %TESTROOTDST%\CompressedDir
@attrib %TESTROOTDST%\CompressedFile

REM --mirror but the source file has +r +c and the destination is empty
REM
@compact /S /C %TESTROOTSRC%\CompressedFile > nul
@attrib +r %TESTROOTSRC%\CompressedFile > nul
@%RD% %TESTROOTDST% > nul

%LN% --mirror %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel% 
@sort sortout
@echo ErrorLevel == %ERRLEV%

@%TIMESTAMP% --xattr %TESTROOTDST%\CompressedFile
@attrib %TESTROOTDST%\CompressedFile



@echo off
REM 
REM Clean up
REM
@%RD% %TESTROOT% > nul

