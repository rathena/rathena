@echo off
rem Writen by Jbain
:end
map-server.exe
echo .
echo .
echo Map server crashed! restarting in 15 seconds! press ctl+C to cancel restart!
PING -n 15 127.0.0.1 >nul
goto end