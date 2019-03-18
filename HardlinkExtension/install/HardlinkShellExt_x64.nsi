!define ALL_USERS

!define PLATTFORM "X64"
!define PLATTFORM_ALTERNATIVE "win32"

!define CONFIGURATION "Release"
!define SP1_DOWNLOAD "Vcredist_x64.exe can be downloaded from$\r$\nhttps://aka.ms/vs/15/release/vc_redist.x64.exe"
!define WIN32_REG_PREFIX_ "32"

OutFile HardLinkShellExt_${PLATTFORM}.exe

!include "HardlinkShellExt.nsi"
