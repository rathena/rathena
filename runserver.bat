@echo off

rem ----- Configuration -----
	rem Defines the server type (txt or sql).
	set SERVER_TYPE=txt
	rem Defines how long to wait before restarting (in seconds).
	set SLEEP_TIME=15
	rem Defines whether to run all servers in one window (yes or no).
	set SINGLE_WINDOW=no
rem ----- ------------- -----

:L_Init
	set this=%0
	if %SERVER_TYPE% == txt set suffix=
	if %SERVER_TYPE% == sql set suffix=_sql
	if %SINGLE_WINDOW% == yes set wndswitch=/B

:L_Main
	set command=%1
	if "%command%" == "" goto L_DefaultAction
	
	if %command% == exec goto L_ExecServerExe
	if %command% == start goto L_StartServerExe
	if %command% == stop goto L_StopServerExe
	if %command% == restart echo "TODO"
	goto L_EOF

:L_DefaultAction
:L_StartServer
	call %this% start login-server%suffix%.exe
	call %this% start char-server%suffix%.exe
	call %this% start map-server%suffix%.exe
	goto L_EOF

:L_StopServer
	call %this% stop login-server%suffix%.exe
	call %this% stop char-server%suffix%.exe
	call %this% stop map-server%suffix%.exe
	goto L_EOF

:L_StartServerExe
	set filename=%2
	if "%filename%" == "" goto L_StartServer
	if exist %filename% goto L_HaveExe
	echo Cannot start '%filename%' because the file is missing!
	goto L_EOF

	:L_HaveExe
	echo Starting %filename%...
	start "%filename%" %wndswitch% %this% exec %filename%
	goto L_EOF

:L_StopServerExe
	set filename=%2
	if "%filename%" == "" goto L_StopServer
	if exist %windir%\system32\taskkill.exe goto L_HaveTaskKill
	echo The 'stop' command is not available on your system.
	exit

	:L_HaveTaskKill
	rem CAUTION! This will kill all processes called %filename%.
	echo Stopping '%filename%'...
	taskkill /F /FI "WINDOWTITLE eq %filename% - %this%   exec %filename%"
	taskkill /F /IM "%filename%"
	goto L_EOF

:L_ExecServerExe
	%filename%
	echo .
	echo .
	echo Server exited, restarting in %SLEEP_TIME% seconds! Press CTRL+C to abort!
	ping.exe -n %SLEEP_TIME% 127.0.0.1 > nul
	goto L_ExecServerExe

:L_EOF
