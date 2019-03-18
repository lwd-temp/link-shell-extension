%~d0
cd %~dp0

call t_Settings.bat 

prompt $g
time /t > XdelTest_current.txt 2>&1 
call t_XdelTest.bat >> XdelTest_current.txt 2>&1 
time /t >> XdelTest_current.txt 2>&1 
prompt $p$g

copy XdelTest_current.txt XdelTest_current_org.txt
%GSAR% -s%~d0:%~p0 -rBasePath\ -o XdelTest_current.txt
%GSAR% -i -s%LH% -rLocalHost -o XdelTest_current.txt
%GSAR% -s%PLATT% -rPLATT -o XdelTest_current.txt
%GSAR% -s%CONFIG% -rCONFIG -o XdelTest_current.txt

%GSAR% -s%~d0:\ -rRootPath\ -o XdelTest_current.txt

%GSAR% -s%~p0 -r\BasePath -o XdelTest_current.txt

%GSAR% -s..\Tools\where.exe -rWHERE -o XdelTest_current.txt
%GSAR% -s--Xp -r -o XdelTest_current.txt
%GSAR% -s--automated_traditional -r -o XdelTest_current.txt
%GSAR% -s--automated_test -r -o XdelTest_current.txt


REM
REM wait 20 seconds
REM
REM @ping 127.0.0.1 -n 20 -w 1000 > nul

REM psshutdown -h

