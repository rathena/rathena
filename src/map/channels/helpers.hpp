// Copyright (C) rAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHANNEL_HELPERS_HPP
#define CHANNEL_HELPERS_HPP

class map_session_data;


// These are functions that are used by the @channel command.
// Maybe put these in map_session_data instead?

/**
 * A player is attempting to create a channel
 * @param sd: Player data
 * @param name: Channel name
 * @param pass: Channel password
 * @return 0 on success or -1 on failure
 */
int channel_pccreate(map_session_data& sd, const char *name, const char *pass);
int channel_pcdelete(map_session_data& sd, const char *name);
int channel_pcjoin(map_session_data& sd, const char *name, const char *pass);
int channel_pcleave(map_session_data &sd, const char *name);
int channel_pccolor(const map_session_data &sd, const char *name, const char *color);
int channel_pcbind(map_session_data &sd, const char *name);
int channel_pcunbind(map_session_data &sd);
int channel_pcban(map_session_data &sd, const char *name, const char *pname, int flag);
int channel_pckick(map_session_data &sd, const char *name, const char *pname);
int channel_pcsetopt(map_session_data &sd, char *name, const char *option, const char *val);

int channel_display_list(map_session_data *sd, const char *option);

#endif /* CHANNEL_HELPERS_HPP */
