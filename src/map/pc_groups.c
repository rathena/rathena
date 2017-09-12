// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/conf.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/strlib.h" // strcmp
#include "../common/socket.h"

#include "atcommand.h" // AtCommandType
#include "pc_groups.h"
#include "pc.h" // e_pc_permission

typedef struct GroupSettings GroupSettings;

// Cached config settings/pointers for quick lookup
struct GroupSettings {
	unsigned int id; // groups.[].id
	int level; // groups.[].level
	const char *name; // groups.[].name
	config_setting_t *commands; // groups.[].commands
	unsigned int e_permissions; // packed groups.[].permissions
	bool log_commands; // groups.[].log_commands
	/// Following are used only during config reading
	config_setting_t *permissions; // groups.[].permissions
	config_setting_t *inherit; // groups.[].inherit
	bool inheritance_done; // have all inheritance rules been evaluated?
	config_setting_t *root; // groups.[]
	int group_pos;/* pos on load */
};

int pc_group_max; /* known number of groups */

static config_t pc_group_config;
static DBMap* pc_group_db; // id -> GroupSettings
static DBMap* pc_groupname_db; // name -> GroupSettings

/**
 * @retval NULL if not found
 * @private
 */
static inline GroupSettings* id2group(int group_id)
{
	return (GroupSettings*)idb_get(pc_group_db, group_id);
}

/**
 * @retval NULL if not found
 * @private
 */
static inline GroupSettings* name2group(const char* group_name)
{
	return (GroupSettings*)strdb_get(pc_groupname_db, group_name);
}

/**
 * Loads group configuration from config file into memory.
 * @private
 */
static void read_config(void)
{
	config_setting_t *groups = NULL;
	const char *config_filename = "conf/groups.conf"; // FIXME hardcoded name
	int group_count = 0;
	
	if (conf_read_file(&pc_group_config, config_filename))
		return;

	groups = config_lookup(&pc_group_config, "groups");

	if (groups != NULL) {
		GroupSettings *group_settings = NULL;
		DBIterator *iter = NULL;
		int i, loop = 0;

		group_count = config_setting_length(groups);
		for (i = 0; i < group_count; ++i) {
			int id = 0, level = 0;
			const char *groupname = NULL;
			int log_commands = 0;
			config_setting_t *group = config_setting_get_elem(groups, i);

			if (!config_setting_lookup_int(group, "id", &id)) {
				ShowConfigWarning(group, "pc_groups:read_config: \"groups\" list member #%d has undefined id, removing...", i);
				config_setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			if (id2group(id) != NULL) {
				ShowConfigWarning(group, "pc_groups:read_config: duplicate group id %d, removing...", i);
				config_setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			config_setting_lookup_int(group, "level", &level);
			config_setting_lookup_bool(group, "log_commands", &log_commands);

			if (!config_setting_lookup_string(group, "name", &groupname)) {
				char temp[20];
				config_setting_t *name = NULL;
				snprintf(temp, sizeof(temp), "Group %d", id);
				if ((name = config_setting_add(group, "name", CONFIG_TYPE_STRING)) == NULL ||
				    !config_setting_set_string(name, temp)) {
					ShowError("pc_groups:read_config: failed to set missing group name, id=%d, skipping... (%s:%d)\n",
					          id, config_setting_source_file(group), config_setting_source_line(group));
					continue;
				}
				config_setting_lookup_string(group, "name", &groupname); // Retrieve the pointer
			}

			if (name2group(groupname) != NULL) {
				ShowConfigWarning(group, "pc_groups:read_config: duplicate group name %s, removing...", groupname);
				config_setting_remove_elem(groups, i);
				--i;
				--group_count;
				continue;
			}

			CREATE(group_settings, GroupSettings, 1);
			group_settings->id = id;
			group_settings->level = level;
			group_settings->name = groupname;
			group_settings->log_commands = (bool)log_commands;
			group_settings->inherit = config_setting_get_member(group, "inherit");
			group_settings->commands = config_setting_get_member(group, "commands");
			group_settings->permissions = config_setting_get_member(group, "permissions");
			group_settings->inheritance_done = false;
			group_settings->root = group;
			group_settings->group_pos = i;

			strdb_put(pc_groupname_db, groupname, group_settings);
			idb_put(pc_group_db, id, group_settings);
			
		}
		group_count = config_setting_length(groups); // Save number of groups
		
		// Check if all commands and permissions exist
		iter = db_iterator(pc_group_db);
		for (group_settings = (GroupSettings *)dbi_first(iter); dbi_exists(iter); group_settings = (GroupSettings *)dbi_next(iter)) {
			config_setting_t *commands = group_settings->commands, *permissions = group_settings->permissions;
			int count = 0, j;

			// Make sure there is "commands" group
			if (commands == NULL)
				commands = group_settings->commands = config_setting_add(group_settings->root, "commands", CONFIG_TYPE_GROUP);
			count = config_setting_length(commands);

			for (j = 0; j < count; ++j) {
				config_setting_t *command = config_setting_get_elem(commands, j);
				const char *name = config_setting_name(command);
				if (!atcommand_exists(name)) {
					ShowConfigWarning(command, "pc_groups:read_config: non-existent command name '%s', removing...", name);
					config_setting_remove(commands, name);
					--j;
					--count;
				}
			}

			// Make sure there is "permissions" group
			if (permissions == NULL)
				permissions = group_settings->permissions = config_setting_add(group_settings->root, "permissions", CONFIG_TYPE_GROUP);
			count = config_setting_length(permissions);

			for(j = 0; j < count; ++j) {
				config_setting_t *permission = config_setting_get_elem(permissions, j);
				const char *name = config_setting_name(permission);
				int p;

				ARR_FIND(0, ARRAYLENGTH(pc_g_permission_name), p, strcmp(pc_g_permission_name[p].name, name) == 0);
				if (p == ARRAYLENGTH(pc_g_permission_name)) {
					ShowConfigWarning(permission, "pc_groups:read_config: non-existent permission name '%s', removing...", name);
					config_setting_remove(permissions, name);
					--p;
					--count;
				}
			}
		}
		dbi_destroy(iter);

		// Apply inheritance
		i = 0; // counter for processed groups
		while (i < group_count) {
			iter = db_iterator(pc_group_db);
			for (group_settings = (GroupSettings *)dbi_first(iter); dbi_exists(iter); group_settings = (GroupSettings *)dbi_next(iter)) {
				config_setting_t *inherit = NULL,
				                 *commands = group_settings->commands,
					             *permissions = group_settings->permissions;
				int j, inherit_count = 0, done = 0;
				
				if (group_settings->inheritance_done) // group already processed
					continue; 

				if ((inherit = group_settings->inherit) == NULL ||
				    (inherit_count = config_setting_length(inherit)) <= 0) { // this group does not inherit from others
					++i;
					group_settings->inheritance_done = true;
					continue;
				}
				
				for (j = 0; j < inherit_count; ++j) {
					GroupSettings *inherited_group = NULL;
					const char *groupname = config_setting_get_string_elem(inherit, j);

					if (groupname == NULL) {
						ShowConfigWarning(inherit, "pc_groups:read_config: \"inherit\" array member #%d is not a name, removing...", j);
						config_setting_remove_elem(inherit,j);
						continue;
					}
					if ((inherited_group = name2group(groupname)) == NULL) {
						ShowConfigWarning(inherit, "pc_groups:read_config: non-existent group name \"%s\", removing...", groupname);
						config_setting_remove_elem(inherit,j);
						continue;
					}
					if (!inherited_group->inheritance_done)
						continue; // we need to do that group first

					// Copy settings (commands/permissions) that are not defined yet
					if (inherited_group->commands != NULL) {
						int l = 0, commands_count = config_setting_length(inherited_group->commands);
						for (l = 0; l < commands_count; ++l)
							config_setting_copy(commands, config_setting_get_elem(inherited_group->commands, l));
					}

					if (inherited_group->permissions != NULL) {
						int l = 0, permissions_count = config_setting_length(inherited_group->permissions);
						for (l = 0; l < permissions_count; ++l)
							config_setting_copy(permissions, config_setting_get_elem(inherited_group->permissions, l));
					}

					++done; // copied commands and permissions from one of inherited groups
				}
				
				if (done == inherit_count) { // copied commands from all of inherited groups
					++i;
					group_settings->inheritance_done = true; // we're done with this group
				}
			}
			dbi_destroy(iter);

			if (++loop > group_count) {
				ShowWarning("pc_groups:read_config: Could not process inheritance rules, check your config '%s' for cycles...\n",
				            config_filename);
				break;
			}
		} // while(i < group_count)

		// Pack permissions into GroupSettings.e_permissions for faster checking
		iter = db_iterator(pc_group_db);
		for (group_settings = (GroupSettings *)dbi_first(iter); dbi_exists(iter); group_settings = (GroupSettings *)dbi_next(iter)) {
			config_setting_t *permissions = group_settings->permissions;
			int c, count = config_setting_length(permissions);

			for (c = 0; c < count; ++c) {
				config_setting_t *perm = config_setting_get_elem(permissions, c);
				const char *name = config_setting_name(perm);
				int val = config_setting_get_bool(perm);
				int j;

				if (val == 0) // does not have this permission
					continue;
				ARR_FIND(0, ARRAYLENGTH(pc_g_permission_name), j, strcmp(pc_g_permission_name[j].name, name) == 0);
				group_settings->e_permissions |= pc_g_permission_name[j].permission;
			}
		}
		dbi_destroy(iter);
	}

	ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' groups in '"CL_WHITE"%s"CL_RESET"'.\n", group_count, config_filename);

	
	if( ( pc_group_max = group_count ) ) {
		DBIterator *iter = db_iterator(pc_group_db);
		GroupSettings *group_settings = NULL;
		int* group_ids = (int*)aMalloc( pc_group_max * sizeof(int) );
		int i = 0;
		for (group_settings = (GroupSettings *)dbi_first(iter); dbi_exists(iter); group_settings = (GroupSettings *)dbi_next(iter)) {
			group_ids[i++] = group_settings->id;
		}
		
		atcommand_db_load_groups(group_ids);
		
		aFree(group_ids);
		
		dbi_destroy(iter);
	}
}

/**
 * Removes group configuration from memory.
 * @private
 */
static void destroy_config(void)
{
	config_destroy(&pc_group_config);
}

/**
 * In group configuration file, setting for each command is either
 * <commandname> : <bool> (only atcommand), or
 * <commandname> : [ <bool>, <bool> ] ([ atcommand, charcommand ])
 * Maps AtCommandType enums to indexes of <commandname> value array,
 * COMMAND_ATCOMMAND (1) being index 0, COMMAND_CHARCOMMAND (2) being index 1.
 * @private
 */
static inline int AtCommandType2idx(AtCommandType type) { return (type-1); }

/**
 * Checks if player group can use @/#command
 * @param group_id ID of the group
 * @param command Command name without @/# and params
 * @param type enum AtCommanndType { COMMAND_ATCOMMAND = 1, COMMAND_CHARCOMMAND = 2 }
 */
bool pc_group_can_use_command(int group_id, const char *command, AtCommandType type)
{
	int result = 0;
	config_setting_t *commands = NULL;
	GroupSettings *group = NULL;

	if (pc_group_has_permission(group_id, PC_PERM_USE_ALL_COMMANDS))
		return true;

	if ((group = id2group(group_id)) == NULL)
		return false;

	commands = group->commands;
	if (commands != NULL) {
		config_setting_t *cmd = NULL;
		
		// <commandname> : <bool> (only atcommand)
		if (type == COMMAND_ATCOMMAND && config_setting_lookup_bool(commands, command, &result))
			return (bool)result;

		// <commandname> : [ <bool>, <bool> ] ([ atcommand, charcommand ])
		if ((cmd = config_setting_get_member(commands, command)) != NULL &&
		    config_setting_is_aggregate(cmd) && config_setting_length(cmd) == 2)
			return (bool)config_setting_get_bool_elem(cmd, AtCommandType2idx(type));
	}
	return false;
}
/**
 * Load permission for player based on group id
 * @param sd Player
 */
void pc_group_pc_load(struct map_session_data * sd) {
	GroupSettings *group = NULL;
	if ((group = id2group(sd->group_id)) == NULL) {
		ShowWarning("pc_group_pc_load: %s (AID:%d) logged in with unknown group id (%d)! kicking...\n",
					sd->status.name,
					sd->status.account_id,
					sd->group_id);
		set_eof(sd->fd);
		return;
	}
	sd->permissions = group->e_permissions;
	sd->group_pos = group->group_pos;
	sd->group_level = group->level;
}
/**
 * Checks if player group has a permission
 * @param group_id ID of the group
 * @param permission permission to check
 */
bool pc_group_has_permission(int group_id, int permission)
{
	GroupSettings *group = NULL;
	return (group = id2group(group_id)) == NULL ? false : (group->e_permissions&permission) != 0;
}

/**
 * Checks commands used by player group should be logged
 * @param group_id ID of the group
 */
bool pc_group_should_log_commands(int group_id)
{
	GroupSettings *group = NULL;
	return (group = id2group(group_id)) == NULL ? false : group->log_commands;
}

/**
 * Checks if player group with given ID exists.
 * @param group_id group id
 * @returns true if group exists, false otherwise
 */
bool pc_group_exists(int group_id)
{
	return idb_exists(pc_group_db, group_id);
}

/**
 * Group ID -> group name lookup. Used only in @who atcommands.
 * @param group_id group id
 * @return group name
 * @public
 */
const char* pc_group_id2name(int group_id)
{
	GroupSettings *group = id2group(group_id);
	if (group == NULL)
		return "Non-existent group!";
	return group->name;
}

/**
 * Group ID -> group level lookup. A way to provide backward compatibility with GM level system.
 * @param group id
 * @return group level
 * @public
 */
int pc_group_id2level(int group_id)
{
	GroupSettings *group = id2group(group_id);
	if (group == NULL)
		return 0;
	return group->level;
}

/**
 * Initialize PC Groups: allocate DBMaps and read config.
 * @public
 */
void do_init_pc_groups(void)
{
	pc_group_db = idb_alloc(DB_OPT_RELEASE_DATA);
	pc_groupname_db = stridb_alloc(DB_OPT_DUP_KEY, 0);
	read_config();
}

/**
 * Finalize PC Groups: free DBMaps and config.
 * @public
 */
void do_final_pc_groups(void)
{
	if (pc_group_db != NULL)
		db_destroy(pc_group_db);
	if (pc_groupname_db != NULL )
		db_destroy(pc_groupname_db);
	destroy_config();
}

/**
 * Reload PC Groups
 * Used in @reloadatcommand
 * @public
 */
void pc_groups_reload(void) {
	struct map_session_data* sd = NULL;
	struct s_mapiterator* iter;

	do_final_pc_groups();
	do_init_pc_groups();
	
	/* refresh online users permissions */
	iter = mapit_getallusers();
	for (sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter))	{
		pc_group_pc_load(sd);
	}
	mapit_free(iter);
}
