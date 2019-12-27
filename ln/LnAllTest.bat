%~d0
cd %~dp0

call t_Settings.bat 

set OUTPUT=LnTest_current.txt
prompt $g
time /t > %OUTPUT% 2>&1 
call t_LnTest.bat >> %OUTPUT% 2>&1 
time /t >> %OUTPUT% 2>&1 
prompt $p$g

copy %OUTPUT% LnTest_current_org.txt
set BASEPATH=%~d0:%~p0
set BASEPATH_DS=%BASEPATH:\=\\%
%GSAR% -s%BASEPATH_DS% -rBasePath\ -o %OUTPUT%
%GSAR% -s%BASEPATH% -rBasePath\ -o %OUTPUT%
%GSAR% -i -s%LH% -rLocalHost -o %OUTPUT%
%GSAR% -s%PLATT% -rPLATT -o %OUTPUT%
%GSAR% -s%CONFIG% -rCONFIG -o %OUTPUT%

%GSAR% -s%~d0:\ -rRootPath\ -o %OUTPUT%

%GSAR% -s%~p0 -r\BasePath -o %OUTPUT%

%GSAR% -s..\Tools\where.exe -rWHERE -o %OUTPUT%
%GSAR% -s"%LN_TEST_OPTIONS%" -r -o %OUTPUT%


%GSAR% -s%EMPTYTESTDRIVE%:\ -rEMPTYTESTDRIVE\ -o %OUTPUT%

for /f %%f in ('type .ipadress') do set IP_ADRESS=%%f
%GSAR% -s%IP_ADRESS% -rIP_ADRESS -o %OUTPUT%

for /F "tokens=2 delims={}" %%i in (.volumeguid) do set VOLUME_GUID=%%i
%GSAR% -s%VOLUME_GUID% -rVOLUME_GUID -o %OUTPUT%


REM
REM wait 20 seconds
REM
@ping 127.0.0.1 -n 20 -w 1000 > nul

REM psshutdown -h

