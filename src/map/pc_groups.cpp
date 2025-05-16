// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pc_groups.hpp"

#include <common/showmsg.hpp>
#include <common/socket.hpp> // set_eof
#include <common/strlib.hpp> // strcmpi
#include <common/utilities.hpp>

#include "atcommand.hpp" // AtCommandType
#include "pc.hpp" // map_session_data

using namespace rathena;

const std::string PlayerGroupDatabase::getDefaultLocation() {
	return std::string( conf_path ) + "/groups.yml";
}

bool PlayerGroupDatabase::parseCommands( const ryml::NodeRef& node, std::vector<std::string>& commands ){
	for( const auto& it : node.children() ){
		std::string command;

		c4::from_chars( it.key(), &command );

		bool allowed;

		if( !this->asBool( node, command, allowed ) ){
			return false;
		}

		if( !atcommand_exists( command.c_str() ) ){
			const char* command_str = command.c_str();
			const char* alias_str = atcommand_alias_lookup( command );

			if( strcmp( command_str, alias_str ) == 0 ){
				this->invalidWarning( it, "Unknown atcommand: %s\n", command.c_str() );
				return false;
			}

			command = alias_str;
		}

		util::tolower( command );

		if( allowed ){
			if( util::vector_exists( commands, command ) ){
				this->invalidWarning( it, "Group already has command \"%s\". Please check your data.\n", command.c_str() );
				return false;
			}else{
				commands.push_back( command );
			}
		}else{
			if( !util::vector_exists( commands, command ) ){
				this->invalidWarning( it, "Group does not have command \"%s\". Please check your data.\n", command.c_str() );
				return false;
			}else{
				util::vector_erase_if_exists( commands, command );
			}
		}
	}

	return true;
}

uint64 PlayerGroupDatabase::parseBodyNode( const ryml::NodeRef& node ){
	uint32 groupId;

	if( !this->asUInt32( node, "Id", groupId ) ){
		return 0;
	}

	std::shared_ptr<s_player_group> group = this->find( groupId );
	bool exists = group != nullptr;

	if( !exists ){
		if( !this->nodesExist( node, { "Name", "Level" })) {
			return 0;
		}

		group = std::make_shared<s_player_group>();
		group->id = groupId;
		group->permissions.reset();
	}

	if( this->nodeExists( node, "Name" ) ){
		std::string name;

		if( !this->asString( node, "Name", name ) ){
			return 0;
		}

		group->name = name;
	}

	if( this->nodeExists( node, "Level" ) ){
		uint32 level;

		if( !this->asUInt32( node, "Level", level ) ){
			return 0;
		}

		if (level > 99) {
			this->invalidWarning(node["Level"], "Group level %u exceeds 99, capping.\n", level);
			level = 99;
		}

		group->level = level;
	}

	if( this->nodeExists( node, "LogCommands" ) ){
		bool log;

		if( !this->asBool( node, "LogCommands", log ) ){
			return 0;
		}

		group->log_commands = log;
	}else{
		if( !exists ){
			group->log_commands = false;
		}
	}

	if( this->nodeExists( node, "Commands" ) && !this->parseCommands( node["Commands"], group->commands ) ){
		return 0;
	}

	if( this->nodeExists( node, "CharCommands" ) && !this->parseCommands( node["CharCommands"], group->char_commands ) ){
		return 0;
	}

	if( this->nodeExists( node, "Permissions" ) ){
		const auto& permissions = node["Permissions"];

		for( const auto& it : permissions ){
			std::string permission;

			c4::from_chars( it.key(), &permission );

			bool allowed;

			if( !this->asBool( permissions, permission, allowed ) ){
				return 0;
			}

			const char* str = permission.c_str();
			bool found = false;

			for( const auto& permission_name : pc_g_permission_name ){
				if( strcmpi( "all_permission", str ) == 0 ){
					if( allowed ){
						group->permissions.set();
					}else{
						group->permissions.reset();
					}

					found = true;
					break;
				}else if( strcmpi( permission_name.name, str ) == 0 ){
					if( allowed ){
						group->permissions.set( permission_name.permission );
					}else{
						group->permissions.reset( permission_name.permission );
					}

					found = true;
					break;
				}
			}

			if( !found ){
				this->invalidWarning( it, "Unknown permission: %s\n", str );
				return 0;
			}
		}
	}

	if( this->nodeExists( node, "Inherit" ) ){
		const auto& inherits = node["Inherit"];

		auto& inheritanceVector = this->inheritance[groupId];

		for( const auto& it : inherits ){
			std::string inherit;

			c4::from_chars( it.key(), &inherit );

			bool enable;

			if( !this->asBool( inherits, inherit, enable ) ){
				return 0;
			}

			util::tolower( inherit );

			if( enable ){
				if( !util::vector_exists( inheritanceVector, inherit ) ){
					inheritanceVector.push_back( inherit );
				}
			}else{
				if( !util::vector_exists( inheritanceVector, inherit ) ){
					this->invalidWarning( it, "Trying to remove inheritance of non-inherited group %s\n", inherit.c_str() );
					return 0;
				}

				util::vector_erase_if_exists( inheritanceVector, inherit );
			}
		}
	}

	if( !exists ){
		this->put( groupId, group );
	}

	return 1;
}

void PlayerGroupDatabase::loadingFinished(){
	static const int32 MAX_CYCLES = 10;
	int32 i;

	for( i = 0; i < MAX_CYCLES; i++ ){
		auto inheritanceIt = this->inheritance.begin();

		while( inheritanceIt != this->inheritance.end() ){
			auto& entry = *inheritanceIt;

			if( entry.second.empty() ){
				inheritanceIt = this->inheritance.erase( inheritanceIt );
				continue;
			}

			std::shared_ptr<s_player_group> group = this->find( entry.first );

			auto it = entry.second.begin();

			while( it != entry.second.end() ){
				std::string& otherName = *it;
				bool found = false;
				bool inherited = false;

				for( const auto& it : *this ){
					std::shared_ptr<s_player_group> otherGroup = it.second;
					// Copy the string
					std::string otherGroupName = otherGroup->name;

					util::tolower( otherGroupName );

					if( otherName == otherGroupName ){
						found = true;

						auto* otherGroupInheritance = util::map_find( this->inheritance, otherGroup->id );

						if( otherGroupInheritance != nullptr && !otherGroupInheritance->empty() ){
							// Try it again in the next cycle
							break;
						}

						// Inherit atcommands
						for( auto& command : otherGroup->commands ){
							if( !util::vector_exists( group->commands, command ) ){
								group->commands.push_back( command );
							}
						}

						// Inherit charcommands
						for( auto& command : otherGroup->char_commands ){
							if( !util::vector_exists( group->char_commands, command ) ){
								group->char_commands.push_back( command );
							}
						}

						// Inherit permissions
						group->permissions |= otherGroup->permissions;

						inherited = true;
						break;
					}
				}

				if( inherited ){
					it = entry.second.erase( it );
					continue;
				}else if( !found ){
					ShowError( "Inherited group \"%s\" for group id %u does not exist.\n", otherName.c_str(), group->id );
					it = entry.second.erase( it );
					continue;
				}else{
					it++;
				}
			}
		}

		if( this->inheritance.empty() ){
			break;
		}
	}

	if( i == MAX_CYCLES && !this->inheritance.empty() ){
		ShowError( "Could not process inheritance rules, check your config for cycles...\n" );
	}

	// Not needed anymore
	this->inheritance.clear();

	uint32 index = 0;
	for( auto& it : *this ){
		it.second->index = index++;
	}

	// Initialize command cache
	atcommand_db_load_groups();

	TypesafeYamlDatabase::loadingFinished();
}

PlayerGroupDatabase player_group_db;

/**
 * Checks if player group can use @/#command
 * @param command Command name without @/# and params
 * @param type enum AtCommanndType { COMMAND_ATCOMMAND = 1, COMMAND_CHARCOMMAND = 2 }
 */
bool s_player_group::can_use_command( const std::string& command, AtCommandType type ){
	if( this->has_permission( PC_PERM_USE_ALL_COMMANDS ) ){
		return true;
	}

	std::string command_lower = command;

	util::tolower( command_lower );

	switch( type ){
		case COMMAND_ATCOMMAND:
			return util::vector_exists( this->commands, command_lower );
		case COMMAND_CHARCOMMAND:
			return util::vector_exists( this->char_commands, command_lower );
	}

	return false;
}

/**
 * Load permission for player based on group id
 * @param sd Player
 */
void pc_group_pc_load(map_session_data * sd) {
	std::shared_ptr<s_player_group> group = player_group_db.find( sd->group_id );

	if( group == nullptr ){
		ShowWarning("pc_group_pc_load: %s (AID:%d) logged in with unknown group id (%d)! kicking...\n",
					sd->status.name,
					sd->status.account_id,
					sd->group_id);
		set_eof(sd->fd);
		return;
	}

	sd->group = group;
	sd->permissions = group->permissions;
}

/**
 * Checks if player group has a permission
 * @param permission permission to check
 */
bool s_player_group::has_permission( e_pc_permission permission ){
	return this->permissions.test( permission );
}

/**
 * Checks commands used by player group should be logged
 */
bool s_player_group::should_log_commands(){
	return this->log_commands;
}

/**
 * Initialize PC Groups and read config.
 */
void do_init_pc_groups( void ){
	player_group_db.load();
}

/**
 * Finalize PC Groups
 */
void do_final_pc_groups( void ){
	player_group_db.clear();
}

/**
 * Reload PC Groups
 * Used in @reloadatcommand
 */
void pc_groups_reload( void ){
	player_group_db.reload();
	
	/* refresh online users permissions */
	struct s_mapiterator* iter = mapit_getallusers();
	for( map_session_data* sd = (map_session_data*)mapit_first(iter); mapit_exists(iter); sd = (map_session_data*)mapit_next(iter) ){
		pc_group_pc_load(sd);
	}
	mapit_free(iter);
}
