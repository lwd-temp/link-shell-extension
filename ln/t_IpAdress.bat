@echo off
SETLOCAL ENABLEDELAYEDEXPANSION 

REM
REM Get the IP Adress
REM
set IP_ADRESS=
set ip_address_string="IPv4 Address"
for /f "usebackq tokens=2 delims=:" %%f in (`ipconfig ^| findstr /c:%ip_address_string%`) do (
  if [!IP_ADRESS!] == [] set IP_ADRESS=%%f
)
echo !IP_ADRESS!>.ipadress

@echo on
