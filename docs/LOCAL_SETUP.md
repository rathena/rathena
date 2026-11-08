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

## Known issues

- A few training-ground floor textures are missing (`new_1-2.bmp`, `new_zone02.bmp`).
  Cosmetic, from using 2025 game data with the 2022 client.
