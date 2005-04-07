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
	CharCommandShowDelay,
	CharCommandShowExp,

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
is_charcommand(const int fd, struct map_session_data* sd, const char* message, int gmlvl);

CharCommandType charcommand(
	const int level, const char* message, CharCommandInfo* info);
int get_charcommand_level(const CharCommandType type);

int charcommand_config_read(const char *cfgName);

#endif

