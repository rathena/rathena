# rAthena Language Library

### Table of Contents
1. [Generating Translation (.pot) File](#1-generating-translation-pot-file)
2. [Translating Language (.po) File](#2-translating-language-po-file)
3. [Adding A New Language](#3-adding-a-new-language)
4. [Set Default Language](#4-set-default-language)
5. [Changing Language](#5-changing-language)
6. [Other Translations](#6-other-translations)
7. [Contributions](#7-contributions)
8. [Known Translations](#8-known-translations)

## 1. Generating Translation (.pot) File
1. To begin, please make sure the map-server is compiled.
2. Once compiled, execute the following command:
### Windows:
```
map-server.exe --generate-translations [path/to/generated.file]
```
### Linux:
```
map-server --generate-translations [path/to/generated.file]
```

## 2. Translating Language (.po) File
1. After generating the translation/template .pot file, rename it to "My_Translation.po"
2. Open the file, then find this dialog:

    ```
    #: npc/re/jobs/novice/novice.txt
    # mes "Hello there! Welcome to the World of Ragnarok Online. My name is Sprakki and I'm in charge of giving you basic gameplay tips.";
    msgctxt "Sprakki#newbe01::NvSprakkiA"
    msgid "Hello there! Welcome to the World of Ragnarok Online. My name is Sprakki and I'm in charge of giving you basic gameplay tips."
    msgstr ""
    ```

3. Add/edit the dialog inside `msgstr ""` (Example in Indonesian from idRO)

    ```
    #: npc/re/jobs/novice/novice.txt
    # mes "Hello there! Welcome to the World of Ragnarok Online. My name is Sprakki and I'm in charge of giving you basic gameplay tips.";
    msgctxt "Sprakki#newbe01::NvSprakkiA"
    msgid "Hello there! Welcome to the World of Ragnarok Online. My name is Sprakki and I'm in charge of giving you basic gameplay tips."
    msgstr "Halo! Selamat datang di Ragnarok Online Indonesia. Namaku adalah Sprakki dan aku disini untuk membantumu mengenai pengetahuan dasar bermain."
    ```

4. Make sure the EOL is set to UNIX.
5. Save the file.

**NOTE:**
* Translation steps above are done manually using a text editor.

## 3. Adding A New Language
1. Open the conf/translation.conf file.
2. Add your new language name and the file for the translations.

Example:
```
	My_Language: {						// Language Name
		lang: (
			"lang/My_Translation.po"	// Translation file
		)
	}
```

**NOTE:**
* More than 1 language can be added.
* More than 1 file can be listed for a language; use commas to separate the file pathname.

## 4. Set Default Language
1. Before changing the default language of the server, make sure the new language is added.
2. Change the default language value in:
```
default_language: "My_Language"
```

## 5. Changing Language
Use `@langtype` in-game to change the user's default language.

Example: `@langtype My_Language`

The language value will be stored to an account variable on the player and will persist through log-outs or server restarts.

## 6. Other Translations
Besides NPC dialogs, translations are also available for:
* Atcommand messages (since multi-language for msg_conf was deprecated).
* Atcommand Help message (`@help`)
* Message of The Day (MOTD)

Example:
```
translations: {
	My_Language: {								// Language Name
		motd: "lang/Indonesian/motd.txt"		// MOTD Translation
		help: "lang/Indonesian/help.txt"		// Help Translation
		lang: (
			"lang/My_Translation/msg_conf.po"	// map_athena.conf Translation
		)
	}
}
```

## 7. Contributions
* rAthena **DOES NOT** officially manage the translation files. Please contact the respective party that provided the translation.
* rAthena will **ONLY** list translations that fit our regulations.
* Visit [rAthena International Forums](https://rathena.org/board/forum/6-international-forums/) for more translations.
* Credits:
  * Originally created by HerculesWS/Hercules@330e31c
  * Converted for rAthena by @aleos89

## 8. Known Translations
* *No information yet*
