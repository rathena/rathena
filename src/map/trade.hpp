// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_TRADE_HPP_
#define	_TRADE_HPP_

struct map_session_data;

void trade_traderequest(struct map_session_data *sd, struct map_session_data *target_sd);
void trade_tradeack(struct map_session_data *sd,int type);
void trade_tradeadditem(struct map_session_data *sd,short index,short amount);
void trade_tradeaddzeny(struct map_session_data *sd,int amount);
void trade_tradeok(struct map_session_data *sd);
void trade_tradecancel(struct map_session_data *sd);
void trade_tradecommit(struct map_session_data *sd);

#endif /* _TRADE_HPP_ */
