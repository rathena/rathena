// $Id: trade.h,v 1.2 2004/09/25 05:32:19 MouseJstr Exp $
#ifndef	_TRADE_H_
#define	_TRADE_H_

#include "map.h"
void trade_traderequest(struct map_session_data *sd,int target_id);
void trade_tradeack(struct map_session_data *sd,int type);
void trade_tradeadditem(struct map_session_data *sd,int index,int amount);
void trade_tradeok(struct map_session_data *sd);
void trade_tradecancel(struct map_session_data *sd);
void trade_tradecommit(struct map_session_data *sd);

#endif	// _TRADE_H_
