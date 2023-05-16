// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef TRADE_HPP
#define TRADE_HPP

class map_session_data;

void trade_traderequest(map_session_data *sd, map_session_data *target_sd);
void trade_tradeack(map_session_data *sd,int type);
void trade_tradeadditem(map_session_data *sd,short index,short amount);
void trade_tradeaddzeny(map_session_data *sd,int amount);
void trade_tradeok(map_session_data *sd);
void trade_tradecancel(map_session_data *sd);
void trade_tradecommit(map_session_data *sd);

#endif /* TRADE_HPP */
