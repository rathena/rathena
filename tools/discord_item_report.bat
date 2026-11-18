@echo off
rem Transcended RO - Item Checker -> Discord relay
rem Double-click to run (keeps checking every 30s). Close the window to stop.
rem Run once only:  discord_item_report.bat --once
title Item Report Relay
cd /d "%~dp0"

rem Find PHP: XAMPP first, then AppServ. Add more paths here if needed.
set "PHPDIR="
if exist "C:\xampp\php\php.exe" set "PHPDIR=C:\xampp\php"
if not defined PHPDIR if exist "D:\xampp\php\php.exe" set "PHPDIR=D:\xampp\php"
if not defined PHPDIR if exist "C:\AppServ\php7\php.exe" set "PHPDIR=C:\AppServ\php7"

if not defined PHPDIR (
	echo PHP not found. Edit this .bat and add your php.exe folder.
	pause
	exit /b 1
)
echo Using PHP: %PHPDIR%\php.exe

set "PHPARGS=-n -d extension_dir=%PHPDIR%\ext -d extension=pdo_mysql -d extension=openssl -d date.timezone=Asia/Bangkok"

set "MODE=--loop"
if /i "%~1"=="--once" set "MODE="

"%PHPDIR%\php.exe" %PHPARGS% discord_item_report.php %MODE%

echo.
echo Relay stopped.
pause
