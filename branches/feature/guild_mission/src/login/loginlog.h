/**
 * @file loginlog.h
 * Module purpose is to register (log) events into a file or sql database.
 * Licensed under GNU GPL.
 *  For more information, see LICENCE in the main folder.
 * @author Athena Dev Teams rev < 15k
 * @author rAthena Dev Team
 */

#ifndef __LOGINLOG_H_INCLUDED__
#define __LOGINLOG_H_INCLUDED__

#ifdef	__cplusplus
extern "C" {
#endif

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

#ifdef	__cplusplus
}
#endif

#endif // __LOGINLOG_H_INCLUDED__
