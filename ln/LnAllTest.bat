%~d0
cd %~dp0

call t_Settings.bat 

prompt $g
time /t > LnTest_current.txt 2>&1 
call t_LnTest.bat >> LnTest_current.txt 2>&1 
time /t >> LnTest_current.txt 2>&1 
prompt $p$g

copy LnTest_current.txt LnTest_current_org.txt
%GSAR% -s%~d0:%~p0 -rBasePath\ -o LnTest_current.txt
%GSAR% -i -s%LH% -rLocalHost -o LnTest_current.txt
%GSAR% -s%PLATT% -rPLATT -o LnTest_current.txt
%GSAR% -s%CONFIG% -rCONFIG -o LnTest_current.txt

%GSAR% -s%~d0:\ -rRootPath\ -o LnTest_current.txt

%GSAR% -s%~p0 -r\BasePath -o LnTest_current.txt

%GSAR% -s..\Tools\where.exe -rWHERE -o LnTest_current.txt
%GSAR% -s--Xp -r -o LnTest_current.txt
%GSAR% -s--automated_traditional -r -o LnTest_current.txt
%GSAR% -s--automated_test -r -o LnTest_current.txt


%GSAR% -s%EMPTYTESTDRIVE%:\ -rEMPTYTESTDRIVE\ -o LnTest_current.txt

for /f %%f in ('type .ipadress') do set IP_ADRESS=%%f
%GSAR% -s%IP_ADRESS% -rIP_ADRESS -o LnTest_current.txt

for /F "tokens=2 delims={}" %%i in (.volumeguid) do set VOLUME_GUID=%%i
%GSAR% -s%VOLUME_GUID% -rVOLUME_GUID -o LnTest_current.txt


REM
REM wait 20 seconds
REM
@ping 127.0.0.1 -n 20 -w 1000 > nul

REM psshutdown -h

