REM
REM Preparation for Symlink test
REM

@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%%DEEPPATH%\source
set TESTROOTDST=%TESTROOT%%DEEPPATH%\dest

set OPTION_PREP=%2

if [%3] == [] ( 
  set OPTION_PREP=--%2
) else (
  set OPTION_PREP=--%2 --%3
)

@%RD% %TESTROOT% 
%MKDIR% %TESTROOTSRC%\Base > nul

%MKDIR% %TESTROOTSRC%\Base\Folder0 > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder0\File BaFo0" > nul


%MKDIR% %TESTROOTSRC%\Base\Folder0\Sufo0 > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder0\Sufo0\File Fo0Sufo0" > nul

%LN% --junction %TESTROOTSRC%\Base\Folder1\Junction_Fo0Sufo0 %TESTROOTSRC%\Base\Folder0\Sufo0 > nul
%MKDIR% %TESTROOTSRC%\Base\Folder1\Sufo0  > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder1\Sufo0\File Fo1Sufo0" > nul
%MKDIR% %TESTROOTSRC%\Base\Folder1\Sufo1 > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder1\Sufo1\File Fo1Sufo1" > nul
%MKDIR% %TESTROOTSRC%\Base\Folder1\Sufo2 > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder1\Sufo2\File Fo1Sufo2" > nul
%MKDIR% %TESTROOTSRC%\Base\Folder1\Sufo4 > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder1\Sufo4\File Fo1Sufo4" > nul
%LN% --junction %TESTROOTSRC%\Base\Folder1\Junction_Fo1Sufo2 %TESTROOTSRC%\Base\Folder1\Sufo2 > nul
%LN% %OPTION_PREP% %TESTROOTSRC%\Base\Folder1\Sufo2 %TESTROOTSRC%\Base\Folder1\Symlink_Fo1Sufo2 > nul

%MKDIR% %TESTROOTSRC%\Base\Folder1\Sufo2\SufoSufo0 > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder1\Sufo2\SufoSufo0\File Fo1Sufo2SufoSufo0" > nul

%MKDIR% %TESTROOTSRC%\Base\Folder2 > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder2\File BaFo2" > nul
%LN% --junction %TESTROOTSRC%\Base\Folder2\Junction_Fo1Sufo4 %TESTROOTSRC%\Base\Folder1\Sufo4 > nul
%MKDIR% %TESTROOTSRC%\Base\Folder2\Sufo0 > nul
%COPY% test\ln.h "%TESTROOTSRC%\Base\Folder2\Sufo0\File Fo2Sufo0" > nul

%LN% %OPTION_PREP%  %TESTROOTSRC%\Base\Folder2\Sufo0 %TESTROOTSRC%\Base\Symlink_Fo2Sufo0 > nul

%LN% --junction %TESTROOTSRC%\Base\Folder1\Junction_Fo0Sufo0 %TESTROOTSRC%\Base\Folder0\Sufo0 > nul

%LN% --junction %TESTROOTSRC%\Base\Folder1\Sufo2\Junction_Fo1JuFo0Sufo0 %TESTROOTSRC%\Base\Folder1\Junction_Fo0Sufo0 > nul

%LN% --junction %TESTROOTSRC%\Base\Folder1\Sufo2\SufoSufo0\Junction_Fo0 %TESTROOTSRC%\Base\Folder0 > nul

%LN% %OPTION_PREP% %TESTROOTSRC%\Base\Folder1\Sufo2\SufoSufo0\Junction_Fo0 %TESTROOTSRC%\Base\Symlink_JuFo0 > nul 

@if [%2] == [symbolic] ( 
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Base\Folder0\Sufo0\File Fo0Sufo0" "%TESTROOTSRC%\Base\Folder0\Sufo0\Symlink File Fo0Sufo0" > nul
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Base\Folder1\Sufo4\File Fo1Sufo4" "%TESTROOTSRC%\Base\Folder2\Symlink File Fo1Sufo4" > nul
  %LN% %OPTION_PREP% "%TESTROOTSRC%\Base\Folder2\Symlink File Fo1Sufo4"
)
@echo on
%WHERE%  *.* %TESTROOT% | sort

