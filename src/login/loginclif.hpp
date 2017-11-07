/**
 * @file loginclif.h
 * Module purpose is to handle incoming and outgoing requests with client.
 * Licensed under GNU GPL.
 *  For more information, see LICENCE in the main folder.
 * @author Athena Dev Teams originally in login.c
 * @author rAthena Dev Team
 */

#pragma once
#ifndef _LOGINCLIF_HPP_
#define _LOGINCLIF_HPP_

/**
 * Entry point from client to log-server.
 * Function that checks incoming command, then splits it to the correct handler.
 * @param fd: file descriptor to parse, (link to client)
 * @return 0=invalid session,marked for disconnection,unknow packet, banned..; 1=success
 */
int logclif_parse(int fd);

/**
 * Initialize the module.
 * Launched at login-serv start, create db or other long scope variable here.
 */
void do_init_loginclif(void);

/**
 * loginclif destructor
 *  dealloc..., function called at exit of the login-serv
 */
void do_final_loginclif(void);

#endif	/* _LOGINCLIF_HPP_ */

