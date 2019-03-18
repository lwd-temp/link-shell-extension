@echo off
cd /D "%~dp0"
echo Vergleiche Verzeichnisse
rem mkdir "%~dp0\AN-Daten"
rem mkdir "%~dp0\AF-Daten"

REM Remote pc74
REM Lokal pc75
REM Arbeitsverzeichnis d:\testing    -> weil ja zuvor mit %~dp0 dahingehangen wird, vielleicht ist das ja relevant

:Remote_Zugriff
REM pctransfer -> samba
REM K:        \\pctransfer\v5_config
REM S:        \\pctransfer\v5_config\bin
REM pc74 -> windows xp x64
REM T:        \\pc74\Daten (F)\W-Lan-AdHoc
REM X:        \\pc74\Daten (F)

:LN_MIRROR_LOKAL
REM Lokal Windows root
bin\ln64\ln.exe --timetolerance 3602000 --exclude "pagefile.sys" --includedir "lokal_test_folder" --mirror "D:\." "%~dp0\mirror_local_win_root"
REM Lokal Windows subfolder
bin\ln64\ln.exe --timetolerance 3602000 --mirror "D:\lokal_test_folder\." "%~dp0\mirror_local_win_subfolder"

:LN_COPY_LOKAL
REM Lokal Windows root
bin\ln64\ln.exe --timetolerance 3602000 --exclude "pagefile.sys" --includedir "lokal_test_folder" --copy "D:\." "%~dp0\copy_local_win_root"
REM Lokal Windows subfolder
bin\ln64\ln.exe --timetolerance 3602000 --copy "D:\lokal_test_folder\." "%~dp0\copy_local_win_subfolder"


:LN_MIRROR_REMOTE
REM Remote Windows root
bin\ln64\ln.exe --timetolerance 3602000 --includedir "W-Lan-AdHoc" --includedir "freigabe" --mirror "X:\." "%~dp0\mirror_remote_win_root_as_netdrive"
bin\ln64\ln.exe --timetolerance 3602000 --includedir "W-Lan-AdHoc" --includedir "freigabe" --mirror "\\pc74\Daten (F)\." "%~dp0\mirror_remote_win_root_as_unc"
REM Remote Windows subfolder
bin\ln64\ln.exe --timetolerance 3602000 --mirror "T:\." "%~dp0\mirror_remote_win_subfolder_as_netdrive"
bin\ln64\ln.exe --timetolerance 3602000 --mirror "\\pc74\Daten (F)\W-Lan-AdHoc\." "%~dp0\mirror_remote_win_subfolder_as_unc"
bin\ln64\ln.exe --timetolerance 3602000 --mirror "X:\W-Lan-AdHoc\." "%~dp0\mirror_remote_win_subfolder_as_netdrive_subfolder"



REM Remote Samba root
bin\ln64\ln.exe --timetolerance 3602000 --includedir "bin" --includedir "icons" --mirror "K:\." "%~dp0\mirror_remote_samba_root_as_netdrive"
bin\ln64\ln.exe --timetolerance 3602000 --includedir "bin" --includedir "icons" --mirror "\\pctransfer\v5_config\." "%~dp0\mirror_remote_samba_root_as_unc"
REM Remote Samba subfolder
bin\ln64\ln.exe --timetolerance 3602000 --mirror "S:\." "%~dp0\mirror_remote_samba_subfolder_as_netdrive"
bin\ln64\ln.exe --timetolerance 3602000 --mirror "\\pctransfer\v5_config\bin\." "%~dp0\mirror_remote_samba_subfolder_as_unc"
bin\ln64\ln.exe --timetolerance 3602000 --mirror "S:\bin\." "%~dp0\mirror_remote_samba_subfolder_as_netdrive_subfolder"


:LN_COPY_REMOTE
REM Remote Windows root
bin\ln64\ln.exe --timetolerance 3602000 --includedir "W-Lan-AdHoc" --includedir "freigabe" --copy "X:\." "%~dp0\copy_remote_win_root_as_netdrive"
bin\ln64\ln.exe --timetolerance 3602000 --includedir "W-Lan-AdHoc" --includedir "freigabe" --copy "\\pc74\Daten (F)\." "%~dp0\copy_remote_win_root_as_unc"
REM Remote Windows subfolder
bin\ln64\ln.exe --timetolerance 3602000 --copy "T:\." "%~dp0\copy_remote_win_subfolder_as_netdrive"
bin\ln64\ln.exe --timetolerance 3602000 --copy "\\pc74\Daten (F)\W-Lan-AdHoc\." "%~dp0\copy_remote_win_subfolder_as_unc"
bin\ln64\ln.exe --timetolerance 3602000 --copy "X:\W-Lan-AdHoc\." "%~dp0\copy_remote_win_subfolder_as_netdrive_subfolder"



REM Remote Samba root
bin\ln64\ln.exe --timetolerance 3602000 --includedir "bin" --includedir "icons" --copy "K:\." "%~dp0\copy_remote_samba_root_as_netdrive"
bin\ln64\ln.exe --timetolerance 3602000 --includedir "bin" --includedir "icons" --copy "\\pctransfer\v5_config\." "%~dp0\copy_remote_samba_root_as_unc"
REM Remote Samba subfolder
bin\ln64\ln.exe --timetolerance 3602000 --copy "S:\." "%~dp0\copy_remote_samba_subfolder_as_netdrive"
bin\ln64\ln.exe --timetolerance 3602000 --copy "\\pctransfer\v5_config\bin\." "%~dp0\copy_remote_samba_subfolder_as_unc"
bin\ln64\ln.exe --timetolerance 3602000 --copy "S:\bin\." "%~dp0\copy_remote_samba_subfolder_as_netdrive_subfolder"
