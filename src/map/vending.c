// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <string.h>

#include "../common/nullpo.h"
#include "clif.h"
#include "itemdb.h"
#include "atcommand.h"
#include "map.h"
#include "chrif.h"
#include "vending.h"
#include "pc.h"
#include "skill.h"
#include "battle.h"
#include "log.h"

#include "irc.h"

/*==========================================
 * ˜I“X•Â½
 *------------------------------------------
*/
void vending_closevending(struct map_session_data *sd)
{
	nullpo_retv(sd);

	sd->vender_id=0;
	clif_closevendingboard(&sd->bl,0);
	if(use_irc && irc_announce_shop_flag)
		irc_announce_shop(sd,0);
}

/*==========================================
 * ˜I“XƒAƒCƒeƒ€ƒŠƒXƒg—v‹
 *------------------------------------------
 */
void vending_vendinglistreq(struct map_session_data *sd,int id)
{
	struct map_session_data *vsd;

	nullpo_retv(sd);

	if( (vsd=map_id2sd(id)) == NULL )
		return;
	if(vsd->vender_id==0)
		return;
	clif_vendinglist(sd,id,vsd->vending);
}

/*==========================================
 * ˜I“XƒAƒCƒeƒ€w“ü
 *------------------------------------------
 */
void vending_purchasereq(struct map_session_data *sd,int len,int id,unsigned char *p)
{
	int i, j, w, new_ = 0, blank, vend_list[MAX_VENDING];
	double z;
	unsigned short amount;
	short idx;
	struct map_session_data *vsd = map_id2sd(id);
	struct vending vending[MAX_VENDING]; // against duplicate packets

	nullpo_retv(sd);

	if (vsd == NULL)
		return;
	if (vsd->vender_id == 0)
		return;
	if (vsd->vender_id == sd->bl.id)
		return;

	// check number of buying items
	if (len < 8 + 4 || len > 8 + 4 * MAX_VENDING) {
		clif_buyvending(sd, 0, 32767, 4); // not enough quantity (index and amount are unknown)
		return;
	}

	blank = pc_inventoryblank(sd); //number of free cells in the buyer's inventory

	// duplicate item in vending to check hacker with multiple packets
	memcpy(&vending, &vsd->vending, sizeof(struct vending) * MAX_VENDING); // copy vending list

	// some checks
	z = 0.;
	w = 0;
	for(i = 0; 8 + 4 * i < len; i++) {
		amount = *(unsigned short*)(p + 4 * i);
		idx = *(short*)(p + 2 + 4 * i) - 2;

		if (amount <= 0)
			return;

		// check of item index in the cart
		if (idx < 0 || idx >= MAX_CART)
			return;

		for(j = 0; j < vsd->vend_num; j++) {
			if (vsd->vending[j].index == idx) {
				vend_list[i] = j;
				break;
			}
		}
		if (j == vsd->vend_num)
			return; //picked non-existing item

		z += ((double)vsd->vending[j].value * (double)amount);
		if (z > (double)sd->status.zeny || z < 0. || z > (double)MAX_ZENY) { // fix positiv overflow (buyer)
			clif_buyvending(sd, idx, amount, 1); // you don't have enough zeny
			return; // zeny s'<
		}
		if (z + (double)vsd->status.zeny > (double)MAX_ZENY) { // fix positiv overflow (merchand)
			clif_buyvending(sd, idx, vsd->vending[j].amount, 4); // too much zeny = overflow
			return; // zeny s'<
		}
		w += itemdb_weight(vsd->status.cart[idx].nameid) * amount;
		if (w + sd->weight > sd->max_weight) {
			clif_buyvending(sd, idx, amount, 2); // you can not buy, because overweight
			return;
		}
		
		if (vending[j].amount > vsd->status.cart[idx].amount) //Check to see if cart/vend info is in sync.
			vending[j].amount = vsd->status.cart[idx].amount;
		
		// if they try to add packets (example: get twice or more 2 apples if marchand has only 3 apples).
		// here, we check cumulativ amounts
		if (vending[j].amount < amount) {
			// send more quantity is not a hack (an other player can have buy items just before)
			clif_buyvending(sd, idx, vsd->vending[j].amount, 4); // not enough quantity
			return;
		} else
			vending[j].amount -= amount;

		switch(pc_checkadditem(sd, vsd->status.cart[idx].nameid, amount)) {
		case ADDITEM_EXIST:
			break;	//We'd add this item to the existing one (in buyers inventory)
		case ADDITEM_NEW:
			new_++;
			if (new_ > blank)
				return; //Buyer has no space in his inventory
			break;
		case ADDITEM_OVERAMOUNT:
			return; //too many items
		}
	}

	//Logs (V)ending Zeny [Lupus]
	if(log_config.zeny > 0 )
		log_zeny(vsd, "V", sd, (int)z);
	//Logs

	pc_payzeny(sd, (int)z);
	pc_getzeny(vsd, (int)z);

	for(i = 0; 8 + 4 * i < len; i++) {
		amount = *(short*)(p + 4 *i);
		idx = *(short*)(p + 2 + 4 * i) - 2;
		//if (amount < 0) break; // tested at start of the function

		//Logs sold (V)ending items [Lupus]
		if(log_config.enable_logs&0x4) {
			log_pick_pc(vsd, "V", vsd->status.cart[idx].nameid, -amount, (struct item*)&vsd->status.cart[idx]);
			log_pick_pc( sd, "V", vsd->status.cart[idx].nameid,  amount, (struct item*)&vsd->status.cart[idx]);
		}
		//Logs

		// vending item
		pc_additem(sd, &vsd->status.cart[idx], amount);
		vsd->vending[vend_list[i]].amount -= amount;
		pc_cart_delitem(vsd, idx, amount, 0);
		clif_vendingreport(vsd, idx, amount);

		//print buyer's name
		if(battle_config.buyer_name) {
			char temp[256];
			sprintf(temp, msg_txt(265), sd->status.name);
			clif_disp_onlyself(vsd,temp,strlen(temp));
		}
	}

	//Always save BOTH: buyer and customer
	if (save_settings&2) {
		chrif_save(sd,0);
		chrif_save(vsd,0);
	}
	//check for @AUTOTRADE users [durf]
	if (vsd->state.autotrade)
	{
		//Close Vending (this was automatically done by the client, we have to do it manually for autovenders) [Skotlex]
		for(i = 0; i < vsd->vend_num && vsd->vending[i].amount < 1; i++);
		if (i == vsd->vend_num)
		{
			vending_closevending(vsd);
			map_quit(vsd);	//They have no reason to stay around anymore, do they?
		}
	}
}

/*==========================================
 * ˜I“XŠJÝ
 *------------------------------------------
 */
void vending_openvending(struct map_session_data *sd,int len,char *message,int flag,unsigned char *p)
{
	int i, j;
	int vending_skill_lvl;
	nullpo_retv(sd);

	if (map[sd->bl.m].flag.novending) {
		clif_displaymessage (sd->fd, msg_txt(276));
		return; //Can't vend in novending mapflag maps.
	}

	//check shopname len
	if(message[0] == '\0')
		return;

	vending_skill_lvl = pc_checkskill(sd, MC_VENDING);
	if(!vending_skill_lvl || !pc_iscarton(sd)) {	// cart skill and cart check [Valaris]
		clif_skill_fail(sd,MC_VENDING,0,0);
		return;
	}

	if (flag) {
		// check number of items in shop
		if (len < 85 + 8 || len > 85 + 8 * MAX_VENDING || len > 85 + 8 * (2 + vending_skill_lvl)) {
			clif_skill_fail(sd, MC_VENDING, 0, 0);
			return;
		}
		for(i = 0, j = 0; (85 + 8 * j < len) && (i < MAX_VENDING); i++, j++) {
			sd->vending[i].index = *(short*)(p+8*j)-2;
			if (sd->vending[i].index < 0 || sd->vending[i].index >= MAX_CART ||
				!itemdb_cantrade(&sd->status.cart[sd->vending[i].index], pc_isGM(sd), pc_isGM(sd)))
			{
				i--; //Preserve the vending index, skip to the next item.
				continue;
			}
			sd->vending[i].amount = *(short*)(p+2+8*j);
			sd->vending[i].value = *(int*)(p+4+8*j);
			if(sd->vending[i].value > battle_config.vending_max_value)
				sd->vending[i].value=battle_config.vending_max_value;
			else if(sd->vending[i].value < 1)
				sd->vending[i].value = 1000000;	// auto set to 1 million [celest]
			// ƒJ[ƒg“à‚ÌƒAƒCƒeƒ€”‚Æ”Ì”„‚·‚éƒAƒCƒeƒ€”‚É‘Šˆá‚ª‚ ‚Á‚½‚ç’†Ž~
			if(pc_cartitem_amount(sd, sd->vending[i].index, sd->vending[i].amount) < 0) { // fixes by Valaris and fritz
				clif_skill_fail(sd, MC_VENDING, 0, 0);
				return;
			}
		}
		if (i != j)
		{	//Some items were not vended. [Skotlex]
			clif_displaymessage (sd->fd, msg_txt(266));
		}
		sd->vender_id = sd->bl.id;
		sd->vend_num = i;
		memcpy(sd->message,message, MESSAGE_SIZE);
		sd->message[MESSAGE_SIZE-1] = '\0';
		if (clif_openvending(sd,sd->vender_id,sd->vending) > 0){
			pc_stop_walking(sd,1);
			clif_showvendingboard(&sd->bl,message,0);
			if(use_irc && irc_announce_shop_flag)
				irc_announce_shop(sd,1);
		} else
			sd->vender_id = 0;
	}
}

