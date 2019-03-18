
set BJKHOME=%~dp0
set ZIP=c:\tools\zip.exe

set HARDLINKHOME=..\..\..\..\..\cpp\hardlinks
copy %HARDLINKHOME%\dupemerge\Doc\dupemerge.html

set BINDIR=%HARDLINKHOME%\Bin\win32\Release
set ARCHIVE=dupemerge.zip
pushd %BINDIR%
%ZIP% %BJKHOME%\%ARCHIVE% "dupemerge.exe"
popd
%ZIP% %ARCHIVE% dupemerge.html
%ZIP% %ARCHIVE% dupemerge.png
%ZIP% %ARCHIVE% amazon.de.png
%ZIP% %ARCHIVE% license.txt
%ZIP% %ARCHIVE% license_tre.txt
%ZIP% %ARCHIVE% license_ultragetop.txt
%ZIP% %ARCHIVE% "Rockall v4.0 Tool EULA 101204.doc"

set BINDIR=%HARDLINKHOME%\Bin\x64\Release
set ARCHIVE=dupemerge64.zip
pushd %BINDIR%
%ZIP% %BJKHOME%\%ARCHIVE% "dupemerge.exe"
popd
%ZIP% %ARCHIVE% dupemerge.html
%ZIP% %ARCHIVE% dupemerge.png
%ZIP% %ARCHIVE% amazon.de.png
%ZIP% %ARCHIVE% license.txt
%ZIP% %ARCHIVE% license_tre.txt
%ZIP% %ARCHIVE% license_ultragetop.txt
%ZIP% %ARCHIVE% "Rockall v4.0 Tool EULA 101204.doc"

set BINDIR=%HARDLINKHOME%\Bin\Itanium\Release
set ARCHIVE=dupemergeItanium.zip
pushd %BINDIR%
%ZIP% %BJKHOME%\%ARCHIVE% "dupemerge.exe"
popd
%ZIP% %ARCHIVE% dupemerge.html
%ZIP% %ARCHIVE% dupemerge.png
%ZIP% %ARCHIVE% amazon.de.png
%ZIP% %ARCHIVE% license.txt
%ZIP% %ARCHIVE% license_tre.txt
%ZIP% %ARCHIVE% license_ultragetop.txt
%ZIP% %ARCHIVE% "Rockall v4.0 Tool EULA 101204.doc"


:ausmausraus

REM
REM wait 10 seconds
REM
@ping 127.0.0.1 -n 10 -w 1000 > nul

