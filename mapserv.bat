@echo off
:mapserv 
start /min /wait map-server.exe conf\map_athena.conf 
goto mapserv