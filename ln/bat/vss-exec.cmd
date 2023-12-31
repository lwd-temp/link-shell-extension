REM
REM called from vshadow via commandline e.g
REM 
REM    vshadow.exe -script=vss-setvar.cmd -exec=vss-exec.cmd e:
REM

REM vss-setvar.cmd is generated by vshadow.exe
REM
call vss-setvar.cmd

REM assign a DOS device to the created snapshot
REM
dosdev x: %shadow_device_1%

REM run the intended copy job 
REM
set LN=c:\tools\ln.exe
%LN% --copy x:\path e:\backup

REM finally remove the drive letter
REM
dosdev -r -d x:

