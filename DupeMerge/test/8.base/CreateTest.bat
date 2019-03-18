set LN=ln.exe
set DEST=..\8

rmdir /Q /S %DEST%

mkdir %DEST%
copy d000.aa %DEST%\d000.exe
copy d000.aa %DEST%\d100.exe
copy d000.aa %DEST%\d200.exe
%LN% %DEST%\d000.exe %DEST%\d010.exe
copy d001.aa %DEST%\d001.exe
copy d001.aa %DEST%\d101.exe
%LN% %DEST%\d001.exe %DEST%\d011.exe

copy 0000 %DEST%
%LN% %DEST%\0000 %DEST%\0010
%LN% %DEST%\0000 %DEST%\0020
%LN% %DEST%\0000 %DEST%\0030

copy 0000 %DEST%\0100
%LN% %DEST%\0100 %DEST%\0110
%LN% %DEST%\0100 %DEST%\0120

copy 0000 %DEST%\0200
%LN% %DEST%\0200 %DEST%\0210

copy 0000 %DEST%\0300

copy 0000 %DEST%\0400

