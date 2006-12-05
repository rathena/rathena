// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_VENDING_H_
#define	_VENDING_H_

#include "map.h"

void vending_closevending(struct map_session_data *sd);
void vending_openvending(struct map_session_data *sd,int len,char *message,int flag,unsigned char *p);
void vending_vendinglistreq(struct map_session_data *sd,int id);
void vending_purchasereq(struct map_session_data *sd,int len,int id,unsigned char *p);

#endif	// _VENDING_H_
