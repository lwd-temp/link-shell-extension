REM Settings for certification
REM @set CERTIFICATE=%~dp0\..\Shared\certificate\LinkShellextension.pfx
@set CERTIFICATE=%~dp0\..\Shared\certificate\schinagl.priv.at.pfx
@set PWD_LOC=%~dp0\..\Shared\certificate\schinagl.priv.at.txt
@set TIME_SERVER="http://timestamp.entrust.net/TSS/RFC3161sha2TS"
@set SIGNTOOL=%~dp0\..\tools\signtool.exe

if exist %PWD_LOC% goto ReadPassword
	echo ### Error: Password for certificate missing
	exit /b 1

:ReadPassword
@set /p PASSWORT=<%PWD_LOC%

exit /b 0

