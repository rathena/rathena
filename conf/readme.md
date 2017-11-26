# Import Directories

## What is the import directory for?

The `import/` directory provides a way for you to change your config settings without the need to even touch the main `/conf/` and `/db/` files.

By placing your custom entries into the `import/` directory within these two locations, your core files will not need to have any conflicts resolved when you update your server. You store your changes, and the rest is updated with rAthena.

## How does this work?

Think of "import" as in "override". Place only the settings you have changed in the import files, or settings you are "overriding".

For example, when setting up a server there are always a few config settings that users would like to change in order for rAthena to suit their needs. The following example will show you how to use the `/conf/import/` directory correctly. (for `/db/import/` examples, see [/db/readme.md](/db/readme.md))

### Login Server
---
We want to use MD5 passwords and disable `_m/f` account creation methods.

#### /conf/import/login_conf.txt

	new_account: no
	use_MD5_passwords: yes


### Char Server
---
We want to change the server name to "Odin".

#### /conf/import/char_conf.txt

	server_name: Odin


### Map Server
---
We want to hide all error messages and add a few custom maps.

#### /conf/import/map_conf.txt

	//Makes server output more silent by omitting certain types of messages:
	//16: Hide Error and SQL Error messages.
	console_silent: 16
	map: 1@toy
	map: 1@valley
	map: shops


### Inter Server
---
We want to use MySQL tables instead of .txt files.

#### /conf/import/inter_conf.txt

	use_sql_db: yes


### Logging Settings
---
We want to log all items and all chat messages.

#### /conf/import/log_conf.txt

	log_filter: 1
	// Log CHAT (Global, Whisper, Party, Guild, Main chat, Clan) (Note 3)
	// log_chat: 63 = logs everything
	log_chat: 63


### Battle Configs
---
We want to change the way various mechanics work. For anything that would be configured in the `/conf/battle/` directory, it will go into `import/battle_conf.txt`. To help you find which configs came from where, it's generally a good idea to comment out the name of the file that specific collection of configs came from.

#### /conf/import/battle_conf.txt

	// guild.conf
	guild_exp_limit: 90

	// items.conf
	vending_over_max: no
	vending_tax: 100
	weapon_produce_rate: 200
	potion_produce_rate: 200
	produce_item_name_input: 0x03

	// misc.conf
	duel_time_interval: 2
	at_mapflag: yes
	at_monsterignore: yes
	cashshop_show_points: yes
	hide_fav_sell: yes
	// Whether or not mail box status is displayed upon login.
	// Default: 0
	// 0 = No
	// 1 = Yes
	// 2 = Yes, when there are unread mails
	mail_show_status: 2

	// monster.conf
	show_mob_info: 3

	// party.conf
	party_hp_mode: 1
	display_party_name: yes

	// pet.conf
	pet_rename: yes

	// player.conf
	max_aspd: 196
	max_third_aspd: 196
	max_extended_aspd: 196
	vip_disp_rate: no

	// status.conf
	debuff_on_logout: 3


We cannot stress enough how helpful this system is for everyone. The majority of git conflicts will simply go away if users make use of the `import/` system.
