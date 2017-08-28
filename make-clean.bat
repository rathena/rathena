@Echo OFF
Echo "Building solution/project file using batch file"
SET PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin
SET SolutionPath=%~dp0\rAthena.sln
Echo Start Time - %Time%
MSbuild.exe %SolutionPath% /t:Clean
Echo End Time - %Time%
Set /p Wait=Build Process Completed...
