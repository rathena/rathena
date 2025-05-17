@echo off
rmdir sln /S/Q
mkdir sln
cd sln
cmake -G "Visual Studio 15 2017" ..

pause