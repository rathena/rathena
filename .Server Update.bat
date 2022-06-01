@ECHO OFF
Title Automatic update the latest Git from the webservice rAyam by.null#6385
ECHO Getting the latest Git from the null webservice

Echo please wait...
git pull origin master
timeout /t 3
cls

Echo please wait...
git pull https://github.com/il3ol2ed/rathena_webservice.git
timeout /t 3
cls

Echo please wait...
git pull https://github.com/rathena/rathena.git feature/enchantgrade_ui
timeout /t 3
cls

Echo please wait...
git pull https://github.com/rathena/rathena.git script/episode_17_2
timeout /t 3
cls

Echo please wait...
git pull https://github.com/rathena/rathena.git update/4th_improvement_bundle
timeout /t 3
cls

Echo please wait...
git pull https://github.com/limitro/rathena.git NightWatchSkills
timeout /t 3
cls

echo Thank you for using our server.
Set /p Wait="Finished press a key to exit."
exit