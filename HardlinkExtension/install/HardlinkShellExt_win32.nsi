!define ALL_USERS

!define PLATTFORM "win32"

!define CONFIGURATION "Release"
!define SP1_DOWNLOAD "Vcredist_x86.exe can be downloaded from$\r$\nhttps://aka.ms/vs/15/release/vc_redist.x86.exe"

OutFile HardLinkShellExt_${PLATTFORM}.exe

!include "HardlinkShellExt.nsi"
