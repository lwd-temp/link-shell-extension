REM goto Win10_arachnida
REM goto Win10_sleipnir
REM goto atgrzwn312178
REM goto atgrzwn312178_c
goto Win7_quadtatz
REM goto VmWareW2K12

REM goto VmWareVista
REM goto VmWareW764
REM goto VmWareW2K8
REM goto VmWareNt4


:atgrzwn312178
REM
REM atgrzwn312178
REM
set HOME=N:\data\cpp\hardlinks\HardlinkExtension
set INST=C:\Program Files\LinkShellExtension
set TOOLS=c:\tools
set PLATT=X64
set OS=Win7
set NSISDIR=C:\Program Files (x86)\NSIS
set LH=atgrzwn312178
set USERNAME=hermann
set MT="C:\Program Files (x86)\Windows Kits\8.1\bin\x86\mt.exe"
set TRUECRYPT="C:\Program Files\TrueCrypt\TrueCrypt.exe"  /force 
goto eof

:atgrzwn312178_c
REM
REM atgrzwn312178 on Drive c:
REM
set HOME=c:\data\cpp\hardlinks\HardlinkExtension
set INST=C:\Program Files\LinkShellExtension
set TOOLS=c:\tools
set PLATT=X64
set OS=Win7
set NSISDIR=C:\Program Files (x86)\NSIS
set LH=atgrzwn312178
set USERNAME=hermann
set MT=c:\vs2005\Common7\Tools\Bin
set TRUECRYPT="C:\Program Files\TrueCrypt\TrueCrypt.exe"  /force 
goto eof

:Win7_quadtatz
REM
REM Win7_quadtatz
REM
set HOME=c:\Data\cpp\git\hardlinks\HardlinkExtension
set INST=c:\Program Files\LinkShellExtension
set TOOLS=d:\tools
set PLATT=X64
set OS=Win7
set NSISDIR=c:\Program Files\NSIS
set LH=quadtatz
set USERNAME=schinagl
set MT=C:\vs2005\Common7\Tools\Bin
set TRUECRYPT="C:\Program Files\TrueCrypt\TrueCrypt.exe" /force 
goto eof


:Win10_sleipnir
REM
REM Win10_sleipnir
REM
set HOME=D:\Data\cpp\hardlinks\HardlinkExtension
set INST=c:\Program Files\LinkShellExtension
set TOOLS=d:\tools
set PLATT=X64
set OS=Win7
REM set OS=Xp
set NSISDIR=c:\Program Files\NSIS
set LH=sleipnir
set USERNAME=schinagl
set MT=C:\vs2005\Common7\Tools\Bin
set TRUECRYPT="C:\Program Files\TrueCrypt\TrueCrypt.exe" /force 
goto eof

:Win10_arachnida
REM
REM Win10_arachnida
REM
set HOME=c:\Data\cpp\hardlinks\HardlinkExtension
set INST=c:\Program Files\LinkShellExtension
set TOOLS=c:\tools
set PLATT=X64
set OS=Win7
REM set OS=Xp
set NSISDIR=c:\Program Files\NSIS
set LH=arachnida
set USERNAME=schinagl
set MT=C:\vs2005\Common7\Tools\Bin
set TRUECRYPT="C:\Program Files\TrueCrypt\TrueCrypt.exe" /force 
goto eof


:VmWareW2K8
REM
REM VmWareW2K8
REM
set HOME=C:\data\cpp\hardlinks\HardlinkExtension
set INST=C:\tmp
set TOOLS=c:\tools
set PLATT=Win32
set OS=Win7
set NSISDIR=c:\Program Files\NSIS
set LH=NONONONONO
set USERNAME=NONONONONO
set MT=c:\vs2005\Common7\Tools\Bin
goto eof

:VmWareW2K12
REM
REM VmWareW2K12
REM
set HOME=C:\data\hardlinks\HardlinkExtension
set INST=C:\tmp
set TOOLS=c:\tools
set PLATT=X64
set OS=Win7
set NSISDIR=c:\Program Files\NSIS
set LH=ws2012
set USERNAME=administrator
set MT=c:\vs2005\Common7\Tools\Bin
goto eof

:VmWareNt4
REM
REM VmWareNt4
REM
set HOME=C:\data\cpp\hardlinks\HardlinkExtension
set INST=C:\Program Files\LinkShellExtension
set TOOLS=c:\tools
set PLATT=Win32
set NSISDIR=c:\Program Files\NSIS
set LH=NONONONONO
set USERNAME=NONONONONO
set MT=c:\vs2005\Common7\Tools\Bin
goto eof

:eof
set PSKILL=%TOOLS%\pskill.exe
