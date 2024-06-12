@echo off

setlocal enabledelayedexpansion

rem switch to the npc folder
cd ..\..\npc\

rem store the output destination
set OUT=%CD%\scripts_custom.conf

rem switch to the custom folder
cd custom\

rem newline
echo. >> !OUT!
rem header
echo // Custom Scripts >> !OUT!

rem store the current directory
set C=%CD%
rem make sure that no paranthesis close is unescaped inside the path
set C=!C:^)=^^^)!

for /R . %%f in (*.txt) do (
	rem store it to allow delayed expansion
	set B=%%f
	rem store relative path for compare
	set R=!B:%C%\=!

	echo npc: npc\custom\!R!>>!OUT!
)

rem switch to the test folder
cd ..\test

rem header
echo // Test scripts >> !OUT!

rem store the current directory
set C=%CD%
rem make sure that no paranthesis close is unescaped inside the path
set C=!C:^)=^^^)!

for /R . %%f in (*.txt) do (
	rem store it to allow delayed expansion
	set B=%%f

	echo npc: npc\test\!B:%C%\=!>>!OUT!
)
