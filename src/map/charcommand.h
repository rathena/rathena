// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARCOMMAND_H_
#define _CHARCOMMAND_H_

enum CharCommandType {
	CharCommand_None = -1,
	CharCommandJobChange,
	CharCommandPetRename,
	CharCommandPetFriendly,
	CharCommandReset,
	CharCommandStats,
	CharCommandOption,
	CharCommandSave,
	CharCommandStatsAll,
	CharCommandSpiritball,
	CharCommandItemList,
	CharCommandEffect,
	CharCommandStorageList,
	CharCommandItem, // by MC Cameri
	CharCommandWarp,
	CharCommandZeny,
	CharCommandFakeName,
	CharCommandBaseLevel,
	CharCommandJobLevel,
	CharCommandQuestSkill,
	CharCommandLostSkill,
	CharCommandSkReset,
	CharCommandStReset,
	CharCommandModel,
	CharCommandSKPoint,
	CharCommandSTPoint,
	CharCommandChangeSex,
	CharCommandFeelReset, // Komurka
	CharCommandHelp,

#ifdef TXT_ONLY
/* TXT_ONLY */

/* TXT_ONLY */
#else
/* SQL-only */

/* SQL Only */
#endif
	
	// End. No more commans after this line.
	CharCommand_Unknown,
	CharCommand_MAX
};

typedef enum CharCommandType CharCommandType;
typedef struct CharCommandInfo {
	CharCommandType type;
	const char* command;
	int level;
	int (*proc)(const int, struct map_session_data*,
		const char* command, const char* message);
} CharCommandInfo;

CharCommandType
is_charcommand(const int fd, struct map_session_data* sd, const char* message);
CharCommandType 
charcommand_sub(const int fd, struct map_session_data* sd, const char* str, int gmlvl);

CharCommandType charcommand(
	struct map_session_data* sd, const int level, const char* message, CharCommandInfo* info);
int get_charcommand_level(const CharCommandType type);

int charcommand_config_read(const char *cfgName);

#endif

