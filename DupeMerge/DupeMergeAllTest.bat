
call t_Settings.bat 

prompt $g
call t_DupeMergeTest.bat > DupeMergeTest_current.txt 2>&1 
prompt $p$g

set GSAR=..\Tools\gsar.exe

%GSAR% -s%~d0:%~p0 -rBasePath\ -o DupeMergeTest_current.txt
%GSAR% -s%LH% -rLocalHost -o DupeMergeTest_current.txt
%GSAR% -s%PLATT% -rPLATT -o DupeMergeTest_current.txt
%GSAR% -s%CONFIG% -rCONFIG -o DupeMergeTest_current.txt

%GSAR% -s%~d0:\ -rRootPath\ -o DupeMergeTest_current.txt

%GSAR% -s%~p0 -r\BasePath -o DupeMergeTest_current.txt

%GSAR% -s..\Tools\where.exe -rWHERE -o DupeMergeTest_current.txt
%GSAR% -s--Xp -r -o DupeMergeTest_current.txt
%GSAR% -s--traditional -r -o DupeMergeTest_current.txt

%GSAR% -s%EMPTYTESTDRIVE%:\ -rEMPTYTESTDRIVE\ -o DupeMergeTest_current.txt

REM
REM wait 5 seconds
REM
@ping 127.0.0.1 -n 5 -w 1000 > nul

