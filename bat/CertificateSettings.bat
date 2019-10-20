REM Settings for certification
REM
@set TIME_SERVER="http://timestamp.entrust.net/TSS/RFC3161sha2TS"
@set SIGNTOOL=%~dp0..\tools\signtool.exe

@set CERTIFICATE_SCHINAGL=%~dp0..\Shared\certificate\schinagl.priv.at.pfx
@if exist %CERTIFICATE_SCHINAGL% (
  @set CERTIFICATE=%CERTIFICATE_SCHINAGL%
  @set CERT_PWD_LOC=%~dp0..\Shared\certificate\schinagl.priv.at.txt
) else (
  @set CERTIFICATE=%~dp0..\Shared\certificate\LinkShellextension.pfx
  @set CERT_PWD_LOC=%~dp0..\Shared\certificate\LinkShellextension.txt
)


@if exist %CERT_PWD_LOC% goto ReadPassword
	echo ### Error: Password for certificate missing
	exit /b 1

:ReadPassword
@set CERTIFICATE_PASSWORD=
@set /p CERTIFICATE_PASSWORD=<%CERT_PWD_LOC%

exit /b 0

