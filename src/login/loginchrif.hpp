// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef LOGINCHRIF_HPP
#define LOGINCHRIF_HPP

#include "../common/cbasetypes.hpp"

/**
 * Entry point from char-server to log-server.
 * Function that checks incoming command, then splits it to the correct handler.
 * @param fd: file descriptor to parse, (link to char-serv)
 * @return 0=invalid server,marked for disconnection,unknow packet; 1=success
 */
int logchrif_parse(int fd);

/**
 * Packet send to all char-servers, except one. (wos: without our self)
 * @param sfd: fd to discard sending to
 * @param buf: packet to send in form of an array buffer
 * @param len: size of packet
 * @return : the number of char-serv the packet was sent to
 */
int logchrif_sendallwos(int sfd, uint8* buf, size_t len);

/**
 * loginchrif constructor
 *  Initialisation, function called at start of the login-serv.
 */
void do_init_loginchrif(void);
/**
 * Signal handler
 *  This function attempts to properly close the server when an interrupt signal is received.
 *  current signal catch : SIGTERM, SIGINT
 */
void do_shutdown_loginchrif(void);
/**
 * loginchrif destructor
 *  dealloc..., function called at exit of the login-serv
 */
void do_final_loginchrif(void);

#endif	/* LOGINCHRIF_HPP */
