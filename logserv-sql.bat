@echo off
rem Writen by Jbain
:end
login-server_sql.exe
echo .
echo .
echo Login server crashed! restarting in 15 seconds! press ctl+C to cancel restart!
PING -n 15 127.0.0.1 >nul
goto end