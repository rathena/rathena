@echo off
rem Transcended RO - Item Checker -> Discord relay
rem Double-click to run (keeps checking every 30s). Close the window to stop.
rem Run once only:  discord_item_report.bat --once
title Item Report Relay
cd /d "%~dp0"

set PHP=C:\AppServ\php7\php.exe
set PHPARGS=-n -d extension_dir=C:\AppServ\php7\ext -d extension=pdo_mysql -d extension=openssl -d date.timezone=Asia/Bangkok

if /i "%~1"=="--once" (
	"%PHP%" %PHPARGS% discord_item_report.php
	pause
) else (
	"%PHP%" %PHPARGS% discord_item_report.php --loop
)
