REM
REM Basically it is very easy to have a version resource in the installer
REM One just has to tag the installer with the resource
REM

call ..\..\..\bat\Settings.bat

call :AddSupportedOS lzma
call :AddSupportedOS lzma_solid
pause
goto :EOF


:AddSupportedOS 
%MT% -nologo -manifest "%1.manifest" -outputresource:%1;1
exit /b