@set PROJECT_DIR=%1
@set FILE=%2

@set /p FTP_SITE=<%~dp0ftp_site.txt
@set /p FTP_USERNAME=<%~dp0ftp_username.txt
@set /p FTP_PASSWORD=<%~dp0ftp_password.txt

@set FTP=%~dp0..\tools\ncftpput.exe

@%FTP% -v -m -u %FTP_USERNAME% -p %FTP_PASSWORD%  %FTP_SITE% /html/schinagl/nt/%PROJECT_DIR% %FILE%
popd