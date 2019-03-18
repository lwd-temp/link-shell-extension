@echo off
SETLOCAL ENABLEDELAYEDEXPANSION 

REM
REM Get the IP Adress
REM
set IP_ADRESS=
if [Win7] == [%OS%] (
  set ip_address_string="IPv4 Address"
) else (
  set ip_address_string="IP Address"
)	
for /f "usebackq tokens=2 delims=:" %%f in (`ipconfig ^| findstr /c:%ip_address_string%`) do (
  if [!IP_ADRESS!] == [] set IP_ADRESS=%%f
)
echo !IP_ADRESS!>.ipadress

@echo on
