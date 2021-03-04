// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef LOGINLOG_HPP
#define LOGINLOG_HPP

#include <memory>

#include "../common/cbasetypes.hpp"

/**
 * Get the number of failed login attempts by the ip in the last minutes.
 * @param ip: ip to search attempt from
 * @param minutes: intervall to search
 * @return number of failed attempts
 */
unsigned long loginlog_failedattempts(uint32 ip, unsigned int minutes);

/**
 * Records an event in the login log.
 * @param ip:
 * @param username:
 * @param rcode:
 * @param message:
 */
void login_log(uint32 ip, const char* username, int rcode, const char* message);

/**
 * Read configuration options.
 * @param key: config keyword
 * @param value: config value for keyword
 * @return true if successful, false if config not complete or server already running
 */
bool loginlog_config_read(const char* w1, const char* w2);


/**
 * Initialize the module.
 * Launched at login-serv start, create db or other long scope variable here.
 * @return true if success else exit execution
 */
bool loginlog_init(void);

/**
 * Handler to cleanup module, called when login-server stops.
 * atm closing sql connection to log schema
 * @return true success
 */
bool loginlog_final(void);

#endif /* LOGINLOG_HPP */
