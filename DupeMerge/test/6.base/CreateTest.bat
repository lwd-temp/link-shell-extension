REM Der Witz hier ist, dass alle files gleich gross sind
REM aber ein paar nach 0x8000 unterschiedlich werden
REM und somit eine neue Gruppe bilden. Das muss erkannt werden


set LN=ln.exe
set DEST=..\6

rmdir /Q /S %DEST%

mkdir %DEST%
copy 0000 %DEST%
copy 0000 %DEST%\0001
copy 0000 %DEST%\0002
copy 0000 %DEST%\0003

copy 0010 %DEST%
copy 0010 %DEST%\0011
copy 0010 %DEST%\0012
copy 0010 %DEST%\0013
copy 0010 %DEST%\0014


