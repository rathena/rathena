@echo off
rem This is and auto-restart script for the rAthena Ragnarok Online Server Emulator.
rem It will also keep the map server OPEN after it crashes to that errors may be
rem more easily identified
rem Writen by Jbain
rem modified by lighta

set SOURCE_DIR=%~dp0
cd %SOURCE_DIR%

if ["%~1"]==[""] (
 REM this is for backward compatibility
 set "target=watch"
) else set target=%~1
echo "target=%target%"

REM to avoid any localization issue
set "login_running=false"
set "char_running=false"
set "web_running=false"
set "map_running=false"


if "%target%" == "status" (
 call :getStatus
) else if "%target%" == "watch" (
 call :Watch
) else if "%target%" == "stop" (
 call :Stop
) else if "%target%" == "stop" (
 call :Stop
) else if "%target%" == "start" (
 call :Start
)
goto :EOF


:Stop
echo "Stoping all serv"
call :stopLogin
call :stopChar
call :stopWeb
call :stopMap
goto :EOF

:Watch
REM this is to align terminology with athena-start, (start with restart mode)
echo "Starting all serv"
set "restart_mode=on"
call :startLogin
call :startChar
call :startWeb
call :startMap
goto :EOF

:Start
echo "Starting all serv"
set "restart_mode=off"
call :startLogin
call :startChar
call :startWeb
call :startMap
goto :EOF

:getStatus
echo "Getting status of all serv"
call :getLoginStatus
call :getCharStatus
call :getWebStatus
call :getMapStatus

if "%login_running%" == "false" ( echo "login_serv is not running" 
) else echo "login_serv is running pid=%LoginServPID%"
if "%char_running%" == "false" ( echo "char_serv is not running"
) else echo "char_serv is running pid=%CharServPID%"
if "%web_running%" == "false" ( echo "web_serv is not running"
) else echo "web_serv is running pid=%WebServPID%"
if "%map_running%" == "false" ( echo "map_serv is not running"
) else echo "map_serv is running pid=%MapServPID%"

goto :EOF



REM ====
REM sub targets (a target per serv)
REM ====

REM stop sub targets
:stopLogin
call :getLoginStatus
if "%login_running%" == "true" Taskkill /PID %LoginServPID% /F
goto :EOF

:stopChar
call :getCharStatus
if "%char_running%" == "true"  Taskkill /PID %CharServPID% /F
goto :EOF

:stopWeb
call :getWebStatus
if "%web_running%" == "true" Taskkill /PID %WebServPID% /F
goto :EOF

:stopMap
call :getMapStatus
if "%map_running%" == "true" Taskkill /PID %MapServPID% /F
goto :EOF

REM start sub targets
:startLogin
call :getLoginStatus
if "%login_running%" == "false" ( start cmd /k logserv.bat %restart_mode%
) else echo "Login serv is already running pid=%LoginServPID%" 
goto :EOF

:startChar
call :getCharStatus
if "%char_running%" == "false" ( start cmd /k charserv.bat %restart_mode%
) else echo "Char serv is already running, pid=%CharServPID%" 
goto :EOF

:startWeb
call :getWebStatus
if "%web_running%" == "false" ( start cmd /k webserv.bat %restart_mode%
) else echo "Web serv is already running, pid=%WebServPID%"
goto :EOF

:startMap
call :getMapStatus
if "%map_running%" == "false" ( start cmd /k mapserv.bat %restart_mode%
) else echo "Map serv is already running, pid=%MapServPID%" 
goto :EOF  

REM status sub targets

:getLoginStatus
for /F "TOKENS=1,2,*" %%a in ('tasklist /FI "IMAGENAME eq login-server.exe"') do set LoginServPID=%%b
echo(%LoginServPID%|findstr "^[-][1-9][0-9]*$ ^[1-9][0-9]*$ ^0$">nul&& set "login_running=true" || set "login_running=false"
goto :EOF

:getCharStatus
for /F "TOKENS=1,2,*" %%a in ('tasklist /FI "IMAGENAME eq char-server.exe"') do set CharServPID=%%b
echo(%CharServPID%|findstr "^[-][1-9][0-9]*$ ^[1-9][0-9]*$ ^0$">nul&& set "char_running=true" || set "char_running=false"
goto :EOF

:getWebStatus
for /F "TOKENS=1,2,*" %%a in ('tasklist /FI "IMAGENAME eq web-server.exe"') do set WebServPID=%%b
echo(%WebServPID%|findstr "^[-][1-9][0-9]*$ ^[1-9][0-9]*$ ^0$">nul&& set "Web_running=true" || set "web_running=false"
goto :EOF

:getMapStatus
for /F "TOKENS=1,2,*" %%a in ('tasklist /FI "IMAGENAME eq map-server.exe"') do set MapServPID=%%b
echo(%MapServPID%|findstr "^[-][1-9][0-9]*$ ^[1-9][0-9]*$ ^0$">nul&& set "map_running=true" || set "map_running=false"
goto :EOF
