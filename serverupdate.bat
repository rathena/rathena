@ECHO OFF
Title Automatic update the latest Git from the rAyam by.null#6385
ECHO Getting the latest Git.
ECHO please wait...
git pull https://github.com/il3ol2ed/rA_united.git

timeout /t 3
cls

Echo 2. 17.1 Dungeon - Odin Past. #6486
Echo please wait...
git pull https://github.com/mundussan/rathena.git hotfix/issue6469

timeout /t 3
cls

Echo 3.Episode 17.2 - Sage's Legacy #6799
git pull https://github.com/rathena/rathena.git script/episode_17_2

timeout /t 3
cls

Echo 4. Prevent opening vending UI multiple times in one vending session #6430
git pull https://github.com/rathena/rathena.git hotfix/vending_autosave_fix

timeout /t 3
cls

Echo 5. 4th / Expanded Job Skill-Updates as of KRO 20220602 incl. 3rd Job fixes / Homunculus skills #7024
git pull https://github.com/datawulf/rathena.git skillupd

timeout /t 3
cls

Echo 6. Minor Expanded Class Job Level Adjust #7029
git pull https://github.com/admkakaroto/rathena.git patch-2

timeout /t 3
cls

Echo ---------------------------------------------------
Set /p Wait="Finished."