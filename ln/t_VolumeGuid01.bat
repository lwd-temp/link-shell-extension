@echo off
REM
REM 1: directory
REM 2: unroll/splice
REM 3: junction/symbolic
REM 4: absolute during copy
REM [5: absolute during creation]

REM
REM SUCC: Check Unroll of Junctions
REM
@echo on

REM
REM Mount VolumeGuids
REM
@echo off
set TESTROOT=%1
set TESTROOTSRC=%TESTROOT%\src
set TESTROOTDST=%TESTROOT%\DST
set TESTROOTSRCDRV=%EMPTYTESTDRIVE%
%TRUECRYPT% test\NtfsTestDrive.tc /letter %EMPTYTESTDRIVELETTER% /password lntest /quit
%MKDIR% %TESTROOTSRC% > nul
%MKDIR% %TESTROOTDST% > nul
%MKDIR% %TESTROOTSRCDRV%\tmp > nul
%COPY% test\ln.h %TESTROOTSRCDRV%tmp > nul
%COPY% test\ln.h %TESTROOTSRCDRV% > nul
%LN% %TESTROOTSRCDRV%\ln.h %TESTROOTSRCDRV%\ln_hardlink.h  > nul


for /f %%f in ('mountvol %EMPTYTESTDRIVE% /L') do set VOLUME_GUID=%%f
echo %VOLUME_GUID% > .volumeguid

@echo on
REM
REM Mount Tests
REM
%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTSRC%\JuncEmptyTestDrive
%LN% --noitcnuj %VOLUME_GUID%tmp %TESTROOTSRC%\JuncEmptyTestDrive
%LN% --noitcnuj %TESTROOTSRC%\JuncEmptyTestDrive
%WHERE% *.* %TESTROOTSRC%\JuncEmptyTestDrive

%LN% --symbolic %VOLUME_GUID%tmp %TESTROOTSRC%\SymlinkEmptyTestDrive
%LN% --symbolic %VOLUME_GUID%tmp %TESTROOTSRC%\SymlinkEmptyTestDrive
%LN% --symbolic %TESTROOTSRC%\SymlinkEmptyTestDrive
%WHERE% *.* %TESTROOTSRC%\SymlinkEmptyTestDrive

%LN% --noitcnuj %VOLUME_GUID% %TESTROOTSRC%\JuncEmptyTestDriveRoot
%LN% --noitcnuj %VOLUME_GUID% %TESTROOTSRC%\JuncEmptyTestDriveRoot
%LN% --noitcnuj %TESTROOTSRC%\JuncEmptyTestDriveRoot
%WHERE% *.* %TESTROOTSRC%\JuncEmptyTestDrive

%LN% --symbolic %VOLUME_GUID% %TESTROOTSRC%\SymlinkEmptyTestDriveRoot
%LN% --symbolic %VOLUME_GUID% %TESTROOTSRC%\SymlinkEmptyTestDriveRoot
%LN% --symbolic %TESTROOTSRC%\SymlinkEmptyTestDriveRoot
%WHERE% *.* %TESTROOTSRC%\SymlinkEmptyTestDriveRoot

REM
REM Test if mountpoints are handled poperly
REM
%LN% --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --splice --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%
%LN% --noitcnuj %TESTROOTDST%\JuncEmptyTestDrive
%LN% --noitcnuj %TESTROOTDST%\JuncEmptyTestDriveRoot
%LN% --symbolic %TESTROOTDST%\SymlinkEmptyTestDrive
%LN% --symbolic %TESTROOTDST%\SymlinkEmptyTestDriveRoot

@%RD% %TESTROOTDST%\JuncEmptyTestDrive
@%RD% %TESTROOTDST%\JuncEmptyTestDriveRoot
@%RD% %TESTROOTDST%
%LN% --excludedir System* --unroll --copy %TESTROOTSRC% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Copy Volume GUID
REM
%LN% --copy test\Poi %VOLUME_GUID% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --copy %VOLUME_GUID% %TESTROOTDST%\ttt  > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --copy %VOLUME_GUID%Res %TESTROOTDST%\ttt  > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --copy test\Poi %VOLUME_GUID%junk > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --copy %VOLUME_GUID%Src %VOLUME_GUID%junk\Res2 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

REM
REM Mirror Volume GUID
REM
%LN% --mirror test\Poi %VOLUME_GUID% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --mirror test\Poi %VOLUME_GUID% > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --mirror test\Poi %VOLUME_GUID%\Poi > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

%LN% --mirror %VOLUME_GUID%Poi %VOLUME_GUID%\Poi2 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@move %TESTROOTSRC%\SymlinkEmptyTestDriveRoot\Poi\history.txt %TESTROOTSRC%\SymlinkEmptyTestDriveRoot\Poi\geschichte.dat > nul

%LN% --mirror %VOLUME_GUID%Poi %VOLUME_GUID%\Poi2 > sortout
@echo off
set ERRLEV=%errorlevel%
sort sortout
@echo on
@echo ErrorLevel == %ERRLEV%

@echo off
%RD% %TESTROOTSRC%\JuncEmptyTestDriveRoot\Res
%RD% %TESTROOTSRC%\JuncEmptyTestDriveRoot\Src
%RD% %TESTROOTSRC%\JuncEmptyTestDriveRoot\Junk
%RD% %TESTROOTDST%\ttt
%RD% %TESTROOTDST%\Poi
%RD% %TESTROOTDST%\Poi2
del %TESTROOTSRC%\JuncEmptyTestDriveRoot\history.txt

@echo off
REM 
REM Clean up
REM
%RD% %TESTROOTSRC%\SymlinkEmptyTestDrive
%RD% %TESTROOTSRC%\JuncEmptyTestDrive
%RD% %TESTROOTSRC%\JuncEmptyTestDriveRoot
%RD% %TESTROOTSRC%\SymlinkEmptyTestDriveRoot
%RD% %TESTROOTSRCDRV%tmp
%RD% %TESTROOTSRCDRV%Poi
%RD% %TESTROOTSRCDRV%Poi2

@echo off
REM
REM Shadow Copy 
REM
REM For testing: Persistent Snapshots are created by
REM     %VSHADOW% -p x:
REM They are deleted by
REM     %VSHADOW% -ds={SNAPSHOT ID}
REM One can map the snapshot onto a share
REM     %VSHADOW% -er={SNAPSHOT ID},sharename
REM     vshadow64 -er={c6107cb4-fdea-4470-8309-fe46af7ca2ee},2013-05-04

REM
REM Prepare test source data
REM 
@set TESTROOTSRC=%~d0\tmp\src
@if not exist %~d0\tmp %MKDIR% %~d0\tmp
@%MKDIR% %TESTROOTSRC%
@%MKDIR% %TESTROOTSRC%\PA_F1
@%COPY% test\ln.h %TESTROOTSRC% > nul


@echo on
@REM
@REM Mount UNC Shares in \\?\GLOBALROOT via vshadow
@REM
@%VSHADOW% -p -script=t_vss-setvar.cmd %~d0 > nul
@set VSS_SHARE=vss_share
@call t_vss-setvar.cmd > nul

REM 
REM Copy from unexposed snapshot and path
REM 
@%RD% %TESTROOTDST%
@%LN% --copy %SHADOW_DEVICE_1%\tmp\src %TESTROOTDST% > sortout
@echo %LN% --copy \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy\tmp\src %TESTROOTDST%  1^>sortout 
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%


REM 
REM Copy without path after share, aka from the 'root' of the share
REM 
@%RD% %TESTROOTDST%
@%VSHADOW% -er=%shadow_id_1%,"%VSS_SHARE%",tmp\src > nul
%LN% --copy \\%LH%\%VSS_SHARE% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%

@REM Cleanup volume shadow
@REM
@%VSHADOW% -ds=%shadow_id_1% > nul



@%LN% %TESTROOTSRC%\ln.h %TESTROOTSRC%\ln_hardlink.h  > nul


@REM
@REM Mount UNC Shares in \\?\GLOBALROOT via vshadow
@REM
@%LN% --noitcnuj %TESTROOTSRC%\PA_F1 %TESTROOTSRC%\PA_J1 > nul
@%LN% --absolute --symbolic %TESTROOTSRC%\PA_F1 %TESTROOTSRC%\PA_S1 > nul
@%LN% --symbolic %TESTROOTSRC%\PA_F1 %TESTROOTSRC%\PA_S2_R > nul
@%VSHADOW% -p -script=t_vss-setvar.cmd %~d0 > nul
@call t_vss-setvar.cmd > nul
@%VSHADOW% -er=%shadow_id_1%,"%VSS_SHARE%" > nul

@echo on
REM 
REM Copy with path after share
REM 
%LN% --copy \\%LH%\%VSS_SHARE%\tmp\src %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@for /F "tokens=4 delims=\" %%i in ('echo %SHADOW_DEVICE_1%') do (
  @set SHADOW_DEVICE=%%i 
)
@%GSAR% -i -s%SHADOW_DEVICE% -rHarddiskVolumeShadowCopy -o sortout > nul
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM 
REM --anchor
REM 
%LN% --keepsymlinkrelation --anchor %TESTROOTSRC% --copy \\%LH%\%VSS_SHARE%\tmp\src %TESTROOTDST%\anchor > sortout
@set ERRLEV=%errorlevel%
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckReparsePoints01 %TESTROOTDST%\anchor



REM 
REM --copy from Globalroot with --anchor & --keepsymlinkrelation
REM 
@%RD% %TESTROOTDST%
@%LN% --keepsymlinkrelation --anchor %~d0\tmp\src --copy %SHADOW_DEVICE_1%\tmp\src %TESTROOTDST%\anchor > sortout
@set ERRLEV=%errorlevel%
@echo %LN% --keepsymlinkrelation --anchor %~d0\tmp\src --copy \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy\tmp\src %TESTROOTDST%\anchor  1^>sortout 
@for /F "tokens=4 delims=\" %%i in ('@echo %SHADOW_DEVICE_1%') do @(
  @set SHADOW_DEVICE=%%i 
)
@%GSAR% -i -s%SHADOW_DEVICE% -rHarddiskVolumeShadowCopy -o sortout > nul
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckReparsePoints01 %TESTROOTDST%\anchor

REM 
REM --mirror from Globalroot with --anchor & --keepsymlinkrelation
REM 
@%LN% --keepsymlinkrelation --anchor %~d0\tmp\src --mirror %SHADOW_DEVICE_1%\tmp\src %TESTROOTDST%\anchor > sortout
@set ERRLEV=%errorlevel%
@echo %LN% --keepsymlinkrelation --anchor %~d0\tmp\src --mirror \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy\tmp\src %TESTROOTDST%\anchor  1^>sortout 
@for /F "tokens=4 delims=\" %%i in ('@echo %SHADOW_DEVICE_1%') do @(
  @set SHADOW_DEVICE=%%i 
)
@%GSAR% -i -s%SHADOW_DEVICE% -rHarddiskVolumeShadowCopy -o sortout > nul
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckReparsePoints01 %TESTROOTDST%\anchor


REM 
REM --mirror from Globalroot with --anchor & --backup
REM 
@%LN% --backup --anchor %~d0\tmp\src --mirror %SHADOW_DEVICE_1%\tmp\src %TESTROOTDST%\anchor > sortout
@set ERRLEV=%errorlevel%
@echo %LN% --backup --anchor %~d0\tmp\src --mirror \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy\tmp\src %TESTROOTDST%\anchor  1^>sortout 
@for /F "tokens=4 delims=\" %%i in ('@echo %SHADOW_DEVICE_1%') do @(
  @set SHADOW_DEVICE=%%i 
)
@%GSAR% -i -s%SHADOW_DEVICE% -rHarddiskVolumeShadowCopy -o sortout > nul
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckReparsePoints01 %TESTROOTDST%\anchor


REM Change the link relations
REM
@%VSHADOW% -ds=%shadow_id_1% > nul
@%RD% %TESTROOTSRC%\PA_J1
@%RD% %TESTROOTSRC%\PA_S1
@%RD% %TESTROOTSRC%\PA_S2_R
@MKDIR %TESTROOTSRC%\PA_F1_
@%LN% --noitcnuj %TESTROOTSRC%\PA_F1_ %TESTROOTSRC%\PA_J1 > nul
@%LN% --absolute --symbolic %TESTROOTSRC%\PA_F1_ %TESTROOTSRC%\PA_S1 > nul
@%LN% --symbolic %TESTROOTSRC%\PA_F1_ %TESTROOTSRC%\PA_S2_R > nul
@%VSHADOW% -p -script=t_vss-setvar.cmd %~d0 > nul
@call t_vss-setvar.cmd > nul

REM --mirror from Globalroot with --anchor & --backup
REM 
@%LN% --backup --anchor %~d0\tmp\src --mirror %SHADOW_DEVICE_1%\tmp\src %TESTROOTDST%\anchor > sortout
@set ERRLEV=%errorlevel%
@echo %LN% --backup --anchor %~d0\tmp\src --mirror \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy\tmp\src %TESTROOTDST%\anchor  1^>sortout 
@for /F "tokens=4 delims=\" %%i in ('@echo %SHADOW_DEVICE_1%') do @(
  @set SHADOW_DEVICE=%%i 
)
@%GSAR% -i -s%SHADOW_DEVICE% -rHarddiskVolumeShadowCopy -o sortout > nul
@sort sortout
@echo ErrorLevel == %ERRLEV%
@call :CheckReparsePoints01 %TESTROOTDST%\anchor


@%RD% %TESTROOTSRC%

@REM Clean up
@REM
@%VSHADOW% -ds=%shadow_id_1% > nul

@echo on
@REM
@REM Prepare test source data on a completley empty native drive. These tests are needed
@REM for verification if \\GLOBALROOT syntax works even from root
@REM 
@set EMTPYSNAPHOTDRIVE=O:
@set TESTROOTSRC=%EMTPYSNAPHOTDRIVE%\tmp\src
@if not exist %EMTPYSNAPHOTDRIVE%\tmp %MKDIR% %EMTPYSNAPHOTDRIVE%\tmp
@%MKDIR% %TESTROOTSRC%
@%MKDIR% %TESTROOTSRC%\PA_F1
@%COPY% test\ln.h %TESTROOTSRC% > nul
@%LN% --noitcnuj %TESTROOTSRC%\PA_F1 %TESTROOTSRC%\PA_J1 > nul
@%LN% --absolute --symbolic %TESTROOTSRC%\PA_F1 %TESTROOTSRC%\PA_S1 > nul
@%LN% --symbolic %TESTROOTSRC%\PA_F1 %TESTROOTSRC%\PA_S2_R > nul


@REM
@REM Mount UNC Shares in \\?\GLOBALROOT via vshadow
@REM
@%VSHADOW% -p -script=t_vss-setvar.cmd %EMTPYSNAPHOTDRIVE% > nul
@call t_vss-setvar.cmd > nul

REM 
REM Copy from unexposed snapshot root
REM 
@%RD% %TESTROOTDST%
@%LN% --copy %SHADOW_DEVICE_1% %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@echo %LN% --copy \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy %TESTROOTDST%  1^>sortout 
@for /F "tokens=4 delims=\" %%i in ('@echo %SHADOW_DEVICE_1%') do @(
  @set SHADOW_DEVICE=%%i 
)
@%GSAR% -i -s%SHADOW_DEVICE% -rHarddiskVolumeShadowCopy -o sortout > nul
@sort sortout
@echo ErrorLevel == %ERRLEV%

REM 
REM Copy from unexposed snapshot root with traling slash
REM 
@%RD% %TESTROOTDST%
@%LN% --copy %SHADOW_DEVICE_1%\ %TESTROOTDST% > sortout
@set ERRLEV=%errorlevel%
@echo %LN% --copy \\?\GLOBALROOT\Device\HarddiskVolumeShadowCopy\ %TESTROOTDST%  1^>sortout 
@%GSAR% -i -s%SHADOW_DEVICE% -rHarddiskVolumeShadowCopy -o sortout > nul
@sort sortout
@echo ErrorLevel == %ERRLEV%

@REM Clean up
@REM 
@%VSHADOW% -ds=%shadow_id_1% > nul
@%RD% %EMTPYSNAPHOTDRIVE%\

:fasttest
REM symbolic link on reparsepoints
REM
@%MKDIR% %TESTROOTSRC%\F0 > nul
@%MKDIR% %TESTROOTSRC%\F1 > nul
@%MKDIR% %TESTROOTSRCDRV%\F0_D > nul
@%MKDIR% %TESTROOTSRCDRV%\F1_D > nul

@%LN% --symbolic %TESTROOTSRCDRV%\F0_D %TESTROOTSRC%\F0\S0 > nul
@%LN% --symbolic %TESTROOTSRC%\F1 %TESTROOTSRC%\F0\S0\S1 > nul
@%LN% --absolute --symbolic %TESTROOTSRC%\F1 %TESTROOTSRC%\F0\S0\S1_absolute > nul

@%LN% --noitcnuj %TESTROOTSRCDRV%\F1_D %TESTROOTSRC%\F0\J0 > nul
@%LN% --symbolic %TESTROOTSRC%\F1 %TESTROOTSRC%\F0\J0\S2 > nul

@REM Check
%LN% --symbolic %TESTROOTSRC%\F0\J0\S2
%LN% --symbolic %TESTROOTSRC%\F0\S0\S1
%LN% --symbolic %TESTROOTSRC%\F0\S0\S1_absolute

@REM Clean up
@%RD% %TESTROOTSRCDRV%\ 
@%RD% %EMTPYSNAPHOTDRIVE%\

@REM Overall clean up
@REM
:ausmausraus
@%RD% %TESTROOT% 
@%TRUECRYPT% /dismount %EMPTYTESTDRIVELETTER% /quit
@echo off
@echo on

@goto :EOF

:CheckReparsePoints01
@%LN% --noitcnuj %1\PA_J1
@%LN% --symbolic %1\PA_S1
@%LN% --symbolic %1\PA_S2_R
@%LN% --symbolic %1\ln.h
@exit /b