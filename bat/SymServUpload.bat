
pushd %~dp0

@set SYMTOOL=%~dp0\..\tools\symstore.exe
@set SYMSTORE=%~dp0\..\.symserv

%SYMTOOL% add /o /s %SYMSTORE% /f @FilesToUpload.txt /t "Link Shell Extension" /v  %MAJOR_VERSION%%MINOR_VERSION%%PATCH_VERSION%%HOTFIX_VERSION% /d SymServUpload.txt


popd
