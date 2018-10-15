// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef LOGINCNSLIF_HPP
#define LOGINCNSLIF_HPP

/**
 * Console Command Parser
 * Transmited from command cli.cpp
 * note common name for all serv do not rename (extern in cli)
 * @author [Wizputer]
 * @param buf: buffer to parse, (from console)
 * @return 1=success
 */
int cnslif_parse(const char* buf);

/**
 * Read the option specified in command line
 *  and assign the confs used by the different server.
 * @param argc:
 * @param argv:
 * @return true or Exit on failure.
 */
int logcnslif_get_options(int argc, char ** argv);

/**
 * Initialize the module.
 * Launched at login-serv start, create db or other long scope variable here.
 */
void do_init_logincnslif(void);
/**
 * Handler to cleanup module, called when login-server stops.
 */
void do_final_logincnslif(void);

#endif	/* LOGINCNSLIF_HPP */
