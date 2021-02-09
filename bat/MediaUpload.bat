@%~d0

@echo.
@echo ######## Media upload ######## 

@set ROOT=%~dp0..

@REM Upload general files
@REM
@call %~dp0FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\lse.xml

@REM Upload LinkshellExtension
@REM
:No_ChocoUpload
@REM goto No_LseUpLoad
@call %ROOT%\HardlinkExtension\install\choco\NugetGen.bat

@call %~dp0FtpUpload.bat hardlinkshellext %ROOT%\Media\HardLinkShellExt_X64.exe
@call %~dp0FtpUpload.bat hardlinkshellext %ROOT%\Media\HardLinkShellExt_win32.exe
@call %~dp0FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\linkshellextension.html
@call %~dp0FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\linkshellextension_fr.html
@call %~dp0FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\linkshellextension_de.html
@copy %ROOT%\HardlinkExtension\Doc\linkshellextension.html %ROOT%\HardlinkExtension\Doc\hardlinkshellext.html > nul
@call %~dp0FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\hardlinkshellext.html

@REM Upload ln.exe
@REM
:No_LseUpLoad
REM @goto No_LnUpLoad
@call %ROOT%\ln\choco\NugetGen.bat

@call %~dp0FtpUpload.bat ln %ROOT%\ln\Doc\ln.html
@call %~dp0FtpUpload.bat ln %ROOT%\Media\ln.zip
@call %~dp0FtpUpload.bat ln %ROOT%\Media\ln64.zip
@call %~dp0FtpUpload.bat ln %ROOT%\Media\lnstatic.zip
@call %~dp0FtpUpload.bat ln %ROOT%\Media\ln64static.zip

@REM Upload dupemerge.exe
@REM
:No_LnUpLoad
REM @goto :EOF
@call %ROOT%\dupemerge\choco\NugetGen.bat

@call %~dp0FtpUpload.bat dupemerge %ROOT%\Media\dupemerge.zip
@call %~dp0FtpUpload.bat dupemerge %ROOT%\Media\dupemerge64.zip
@call %~dp0FtpUpload.bat dupemerge %ROOT%\dupemerge\Doc\dupemerge.html
