@REM
@REM 1: symbolic
@if [%1] == [symbolic] ( 
  SET OPTION=--%1
) else (
  SET OPTION=
)

REM file -> file
@set SYMBOLIC_TESTFILE=test\ln_symlink.h
%LN% %OPTION% test\ln.h %SYMBOLIC_TESTFILE% > nul
@set ERRLEV=%errorlevel%
@%LN% --symbolic %SYMBOLIC_TESTFILE%
@echo ErrorLevel == %ERRLEV%

REM file -> file - already exists
%LN% %OPTION% test\ln.h %SYMBOLIC_TESTFILE%
@echo ErrorLevel == %errorlevel%

@del %SYMBOLIC_TESTFILE%

REM not_existant_file -> file
@set SYMBOLIC_TESTFILE=test\ln_symlink.h
%LN% %OPTION% test\ln_not_existant.h %SYMBOLIC_TESTFILE% 
@echo ErrorLevel == %errorlevel%

REM file -> dir/dir
@set SYMBOLIC_TESTDIR=test\symbolic
@%MKDIR% %SYMBOLIC_TESTDIR%
%LN% %OPTION% test\ln.h %SYMBOLIC_TESTDIR%  > nul
@set ERRLEV=%errorlevel%
@%LN% --symbolic %SYMBOLIC_TESTDIR%\ln.h
@echo ErrorLevel == %ERRLEV%

REM file -> dir/dir - already exists
%LN% %OPTION% test\ln.h %SYMBOLIC_TESTDIR% 
@echo ErrorLevel == %errorlevel%

@%RD% %SYMBOLIC_TESTDIR%

REM file -> dir
@set SYMBOLIC_TESTDIR=symbolic
@pushd test
@%MKDIR% %SYMBOLIC_TESTDIR%
..\%LN% %OPTION% ln.h %SYMBOLIC_TESTDIR%  > nul
@set ERRLEV=%errorlevel%
@..\%LN% --symbolic %SYMBOLIC_TESTDIR%\ln.h
@echo ErrorLevel == %ERRLEV%
@..\%RD% %SYMBOLIC_TESTDIR%
@popd

REM not_existant_file -> dir/dir
@set SYMBOLIC_TESTDIR=test\symbolic
@%MKDIR% %SYMBOLIC_TESTDIR%
%LN% %OPTION% test\ln_not_existant.h %SYMBOLIC_TESTDIR%  > nul
@set ERRLEV=%errorlevel%
@%LN% --symbolic %SYMBOLIC_TESTDIR%\ln.h
@echo ErrorLevel == %ERRLEV%
@%RD% %SYMBOLIC_TESTDIR%

REM file -> dir/symlink_dir
@set SYMBOLIC_TESTDIR=test\symbolic
@set SYMBOLIC_TESTREPARSE=test\symlink_dir
@%MKDIR% %SYMBOLIC_TESTDIR%
@%LN% --symbolic %SYMBOLIC_TESTDIR% %SYMBOLIC_TESTREPARSE% > nul
%LN% %OPTION% test\ln.h %SYMBOLIC_TESTREPARSE%  > nul
@set ERRLEV=%errorlevel%
@%LN% --symbolic %SYMBOLIC_TESTREPARSE%\ln.h
@echo ErrorLevel == %ERRLEV%
@%RD% %SYMBOLIC_TESTDIR%
@%RD% %SYMBOLIC_TESTREPARSE%

REM file -> dir/junction
@set SYMBOLIC_TESTDIR=test\symbolic
@set SYMBOLIC_TESTREPARSE=test\junction
@%MKDIR% %SYMBOLIC_TESTDIR%
@%LN% --noitcnuj %SYMBOLIC_TESTDIR% %SYMBOLIC_TESTREPARSE% > nul
%LN% %OPTION% test\ln.h %SYMBOLIC_TESTREPARSE%  > nul
@set ERRLEV=%errorlevel%
@%LN% --symbolic %SYMBOLIC_TESTREPARSE%\ln.h
@echo ErrorLevel == %ERRLEV%
@%RD% %SYMBOLIC_TESTDIR%
@%RD% %SYMBOLIC_TESTREPARSE%

REM dir -> dir/dir
@set SYMBOLIC_TESTDIR=test\symbolic
@set SYMBOLIC_TARGET=test\sym_target
@%MKDIR% %SYMBOLIC_TESTDIR%
@%MKDIR% %SYMBOLIC_TARGET%
%LN% %OPTION% %SYMBOLIC_TARGET% %SYMBOLIC_TESTDIR%
@echo ErrorLevel == %errorlevel%

REM dir -> dir/dir/
%LN% %OPTION% %SYMBOLIC_TARGET% %SYMBOLIC_TESTDIR%\  > nul
@set ERRLEV=%errorlevel%
@%LN% --symbolic %SYMBOLIC_TESTDIR%\sym_target
@echo ErrorLevel == %ERRLEV%

REM dir -> dir/dir/ - already exists
%LN% %OPTION% %SYMBOLIC_TARGET% %SYMBOLIC_TESTDIR%\
@echo ErrorLevel == %errorlevel%

@%RD% %SYMBOLIC_TESTDIR%
@%RD% %SYMBOLIC_TARGET%
