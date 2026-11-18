@echo off
rem Transcended RO - Item Checker -> Discord relay
rem Double-click to run (keeps checking every 30s). Close the window to stop.
rem Run once only:  discord_item_report.bat --once
title Item Report Relay
cd /d "%~dp0"

set "PHP=C:\AppServ\php7\php.exe"
set "PHPARGS=-n -d extension_dir=C:\AppServ\php7\ext -d extension=pdo_mysql -d extension=openssl -d date.timezone=Asia/Bangkok"

if not exist "%PHP%" (
	echo PHP not found at %PHP% - edit this .bat and fix the PHP path.
	pause
	exit /b 1
)

set "MODE=--loop"
if /i "%~1"=="--once" set "MODE="

"%PHP%" %PHPARGS% discord_item_report.php %MODE%

echo.
echo Relay stopped.
pause
