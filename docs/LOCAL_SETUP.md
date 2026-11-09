# Local setup

How the RO Sandbox test server and client run on one Windows 10 laptop.
Verified on 2026-11-07: GM login works and `@item 501` gives a Red Potion.

## Server (WSL)

- Windows 10 (19045), WSL 3.0.1, Ubuntu 24.04.
- Ubuntu lives on an external USB SSD (`E:\WSL\Ubuntu`), installed with
  `wsl --install -d Ubuntu-24.04 --location E:\WSL\Ubuntu`.
  Run `wsl --shutdown` in PowerShell before unplugging the SSD.
- Packages: `git make gcc g++ zlib1g-dev libpcre3-dev libmariadb-dev libmariadb-dev-compat mariadb-server`.
- Build:
  ```
  ./configure --enable-prere=yes --enable-packetver=20220406
  make clean && make server -j4
  ```
- Database: `ragnarok` database and `ragnarok@localhost` user, loaded from
  `sql-files/main.sql` and `sql-files/logs.sql`.
- `conf/import/` (not in git) holds:
  - `inter_conf.txt`: the six `*_pw` database password lines.
  - `char_conf.txt`: `char_ip: 127.0.0.1`
  - `map_conf.txt`: `map_ip: 127.0.0.1`
- Start and stop: `./athena-start start --enlog` and `./athena-start stop`.
  Check with `grep -a "ready" log/*-server.log` (login 6900, char 6121, map 5121, web 8888).
- GM account: one row in `login` with `group_id` 99. New accounts can't be
  registered from the client (`new_account: no`).

## Client (Windows, D:)

Gravity files stay on the laptop. Never commit them.

- Froggo Rö Folder (2022-04-06): `FroggoClient.exe`, `ROTP20220406.grf` (English),
  `official_data.grf`. https://gitlab.com/SapitoSucio/fROggo-folder
- Its `data.grf` isn't in the GitLab download, so it comes from the
  2025-07-16 kRO full client, copied next to `FroggoClient.exe`.
- `data/clientinfo.xml`: one connection, `localhost` port 6900. The author's
  second server entry was removed.
- Needs Visual C++ Redistributables and the DirectX runtime. In Setup.exe keep DirectX7.
- Don't run `rsu-kro-rag-lite-x64.exe`: it can pull a newer `data.grf` the 2022 client can't read.

## Updating items from the sheet

The RO_Items_PreRenewal sheet is shared as "Anyone with the link can view",
so the converter downloads it directly.

- `~/.bashrc` sets `RO` to the client folder:
  `/mnt/d/ProgramFiles/Games/Ragnarok Online/fROggo-folder-main`
- From the repo root:
  ```
  python3 tools/sandbox/sheet2yaml.py --client "$RO"
  ```
  This writes `db/import/item_db.yml`, `db/import/mob_db.yml` and the client's
  `SystemEN/itemInfo_C.lua`. If the sheet has a problem, it lists what to fix
  and writes nothing.
- `mob_db.yml` also gets the early-game boost for regular monsters up to Lv 30
  (more common loot, more EXP, 10% to 5% cards, easing to 0.5% by Lv 50). Its
  chances are worked out from `conf/battle/drops.conf`, so run the script
  again after changing drop rates there. The numbers are at the top of
  `sheet2yaml.py`.
- Restart the map server, and restart the client to see new item names.
- Commit the two `db/import` files. The client file stays out of git.

## Updating skills from the sheet

The RO_Skill_Trees sheet must also be shared as "Anyone with the link".

- From the repo root:
  ```
  python3 tools/sandbox/skills2yaml.py
  ```
  This writes `db/import/skill_tree.yml` and `db/import/skill_db.yml`, then
  prints what it changed: skills that keep classic prerequisites, entries
  copied from a parent job, raised max levels and timing changes. If the
  sheet has a problem, it lists what to fix and writes nothing.
- Restart the map server.
- Commit the two `db/import` files.

## Known issues

- A few training-ground floor textures are missing (`new_1-2.bmp`, `new_zone02.bmp`).
  Cosmetic, from using 2025 game data with the 2022 client.
