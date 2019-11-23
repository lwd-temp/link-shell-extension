@%~d0

@echo.
@echo ######## Media Symbolserver upload ######## 

@set ROOT=%~dp0..

@REM Upload LinkshellExtension
@REM
:No_ChocoUpload
@REM goto No_LseUpLoad
@call %ROOT%\HardlinkExtension\install\choco\NugetGen.bat

@call FtpUpload.bat hardlinkshellext %ROOT%\Media\HardLinkShellExt_X64.exe
@call FtpUpload.bat hardlinkshellext %ROOT%\Media\HardLinkShellExt_win32.exe
@call FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\linkshellextension.html
@call FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\linkshellextension_fr.html
@call FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\lse.xml
@copy %ROOT%\HardlinkExtension\Doc\linkshellextension.html %ROOT%\HardlinkExtension\Doc\hardlinkshellext.html
@call FtpUpload.bat hardlinkshellext %ROOT%\HardlinkExtension\Doc\hardlinkshellext.html

@REM Upload ln.exe
@REM
:No_LseUpLoad
@REM goto No_LnUpLoad
@call %ROOT%\ln\choco\NugetGen.bat

@call FtpUpload.bat ln %ROOT%\Media\ln.zip
@call FtpUpload.bat ln %ROOT%\Media\ln64.zip
@call FtpUpload.bat ln %ROOT%\ln\Doc\ln.html

@REM Upload dupemerge.exe
@REM
:No_LnUpLoad
@REM goto :EOF
@call %ROOT%\dupemerge\choco\NugetGen.bat

@call FtpUpload.bat dupemerge %ROOT%\Media\dupemerge.zip
@call FtpUpload.bat dupemerge %ROOT%\Media\dupemerge64.zip
@call FtpUpload.bat dupemerge %ROOT%\dupemerge\Doc\dupemerge.html
