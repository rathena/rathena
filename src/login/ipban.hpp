// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef IPBAN_HPP
#define IPBAN_HPP

#include <string>

#include "../common/cbasetypes.hpp"

struct s_ipban_config {
	uint16 ipban_db_port; ///< IP Ban port
	std::string ipban_db_hostname; ///< IP Ban IP
	std::string ipban_db_username; ///< IP Ban username
	std::string ipban_db_password; ///< IP Ban password
	std::string ipban_db_database; ///< IP Ban database
	std::string ipban_codepage; ///< Codepage [irmin]
	std::string ipban_table; ///< IP Ban SQL Table
};

extern s_ipban_config ipban_config;

/**
 * Check if ip is in the active bans list.
 * @param ip: ipv4 ip to check if ban
 * @return true if found or error, false if not in list
 */
bool ipban_check(uint32 ip);

/**
 * Log a failed attempt.
 *  Also bans the user if too many failed attempts are made.
 * @param ip: ipv4 ip to record the failure
 */
void ipban_log(uint32 ip);

/**
 * Read configuration options.
 * @param key: config keyword
 * @param value: config value for keyword
 * @return true if successful, false if config not complete or server already running
 */
bool ipban_config_read(const char* key, const char* value);

/**
 * Initialize the module.
 * Launched at login-serv start, create db or other long scope variable here.
 */
void ipban_init(void);

/**
 * Destroy the module.
 * Launched at login-serv end, cleanup db connection or other thing here.
 */
void ipban_final(void);

#endif /* IPBAN_HPP */
