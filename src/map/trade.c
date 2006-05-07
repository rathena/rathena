// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <string.h>

#include "../common/nullpo.h"
#include "clif.h"
#include "itemdb.h"
#include "map.h"
#include "trade.h"
#include "pc.h"
#include "npc.h"
#include "battle.h"
#include "chrif.h"
#include "storage.h"
#include "intif.h"
#include "atcommand.h"
#include "log.h"


/*==========================================
 * 取引要請を相手に送る
 *------------------------------------------
 */
void trade_traderequest(struct map_session_data *sd, int target_id) {
	struct map_session_data *target_sd;
	int level;

	nullpo_retv(sd);

	if (map[sd->bl.m].flag.notrade) {
		clif_displaymessage (sd->fd, msg_txt(272));
		return; //Can't trade in notrade mapflag maps.
	}

	if ((target_sd = map_id2sd(target_id)) != NULL) {
		if (!battle_config.invite_request_check) {
			if (target_sd->guild_invite > 0 || target_sd->party_invite > 0) {
				clif_tradestart(sd, 2); // 相手はPT要請中かGuild要請中
				return;
			}
		}
		level = pc_isGM(sd);
		if ( pc_can_give_items(level) || pc_can_give_items(pc_isGM(target_sd)) ) //check if both GMs are allowed to trade
		{
			clif_displaymessage(sd->fd, msg_txt(246));
			trade_tradecancel(sd); // GM is not allowed to trade
		} else if ((target_sd->trade_partner != 0) || (sd->trade_partner != 0)) {
			trade_tradecancel(sd); // person is in another trade
		} else {
			//Fixed. Only real GMs can request trade from far away! [Lupus] 
			if (level < lowest_gm_level && (sd->bl.m != target_sd->bl.m ||
			    (sd->bl.x - target_sd->bl.x <= -5 || sd->bl.x - target_sd->bl.x >= 5) ||
			    (sd->bl.y - target_sd->bl.y <= -5 || sd->bl.y - target_sd->bl.y >= 5))) {
				clif_tradestart(sd, 0); // too far
			} else if (sd != target_sd) {
				target_sd->trade_partner = sd->status.account_id;
				sd->trade_partner = target_sd->status.account_id;
				clif_traderequest(target_sd, sd->status.name);
			}
		}
	} else {
		clif_tradestart(sd, 1); // character does not exist
	}
}

/*==========================================
 * 取引要請
 *------------------------------------------
 */
void trade_tradeack(struct map_session_data *sd, int type) {
	struct map_session_data *target_sd;
	nullpo_retv(sd);

	if ((target_sd = map_id2sd(sd->trade_partner)) != NULL) {
		clif_tradestart(target_sd, type);
		clif_tradestart(sd, type);
		if (type == 4) { // Cancel
			sd->state.deal_locked = 0;
			sd->trade_partner = 0;
			target_sd->state.deal_locked = 0;
			target_sd->trade_partner = 0;
		}

		if (type == 3) { //Initiate trade
			sd->state.trading = 1;
			target_sd->state.trading = 1;
			memset(&sd->deal, 0, sizeof(sd->deal));
			memset(&target_sd->deal, 0, sizeof(target_sd->deal));
		}
		
		if (sd->npc_id != 0)
			npc_event_dequeue(sd);
		if (target_sd->npc_id != 0)
			npc_event_dequeue(target_sd);

		//close STORAGE window if it's open. It protects from spooffing packets [Lupus]
		if (sd->state.storage_flag == 1)
			storage_storageclose(sd);
		else if (sd->state.storage_flag == 2)
			storage_guild_storageclose(sd);
	}
}

/*==========================================
 * Check here hacker for duplicate item in trade
 * normal client refuse to have 2 same types of item (except equipment) in same trade window
 * normal client authorise only no equiped item and only from inventory
 *------------------------------------------
 */
int impossible_trade_check(struct map_session_data *sd) {
	struct item inventory[MAX_INVENTORY];
	char message_to_gm[200];
	int i, index;

	nullpo_retr(1, sd);
	
    if(sd->deal.zeny > sd->status.zeny)
	{
		pc_setglobalreg(sd,"ZENY_HACKER",1);
		return -1;
	}

	// get inventory of player
	memcpy(&inventory, &sd->status.inventory, sizeof(struct item) * MAX_INVENTORY);

	// remove this part: arrows can be trade and equiped
	// re-added! [celest]
	// remove equiped items (they can not be trade)
	for (i = 0; i < MAX_INVENTORY; i++)
		if (inventory[i].nameid > 0 && inventory[i].equip && !(inventory[i].equip & 0x8000))
			memset(&inventory[i], 0, sizeof(struct item));

	// check items in player inventory
	for(i = 0; i < 10; i++)
		if (sd->deal.item[i].amount < 0) { // negativ? -> hack
//			printf("Negativ amount in trade, by hack!\n"); // normal client send cancel when we type negativ amount
			return -1;
		} else if (sd->deal.item[i].amount > 0) {
			index = sd->deal.item[i].index;
			inventory[index].amount -= sd->deal.item[i].amount; // remove item from inventory
//			printf("%d items left\n", inventory[index].amount);
			if (inventory[index].amount < 0) { // if more than the player have -> hack
//				printf("A player try to trade more items that he has: hack!\n");
				sprintf(message_to_gm, msg_txt(538), sd->status.name, sd->status.account_id); // Hack on trade: character '%s' (account: %d) try to trade more items that he has.
				intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, message_to_gm);
				sprintf(message_to_gm, msg_txt(539), sd->status.inventory[index].amount, sd->status.inventory[index].nameid, sd->status.inventory[index].amount - inventory[index].amount); // This player has %d of a kind of item (id: %d), and try to trade %d of them.
				intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, message_to_gm);
				// if we block people
				if (battle_config.ban_hack_trade < 0) {
					chrif_char_ask_name(-1, sd->status.name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
					clif_setwaitclose(sd->fd); // forced to disconnect because of the hack
					// message about the ban
					sprintf(message_to_gm, msg_txt(540), battle_config.ban_spoof_namer); //  This player has been definitivly blocked.
				// if we ban people
				} else if (battle_config.ban_hack_trade > 0) {
					chrif_char_ask_name(-1, sd->status.name, 2, 0, 0, 0, 0, battle_config.ban_hack_trade, 0); // type: 2 - ban (year, month, day, hour, minute, second)
					clif_setwaitclose(sd->fd); // forced to disconnect because of the hack
					// message about the ban
					sprintf(message_to_gm, msg_txt(507), battle_config.ban_spoof_namer); //  This player has been banned for %d minute(s).
				} else {
					// message about the ban
					sprintf(message_to_gm, msg_txt(508)); //  This player hasn't been banned (Ban option is disabled).
				}
				intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, message_to_gm);
				return 1;
			}
		}

	return 0;
}

/*==========================================
 * Check here if we can add item in inventory (against full inventory)
 *------------------------------------------
 */
int trade_check(struct map_session_data *sd) {
	struct item inventory[MAX_INVENTORY];
	struct item inventory2[MAX_INVENTORY];
	struct item_data *data;
	struct map_session_data *target_sd;
	int trade_i, i, amount;

	target_sd = map_id2sd(sd->trade_partner);

	// get inventory of player
	memcpy(&inventory, &sd->status.inventory, sizeof(struct item) * MAX_INVENTORY);
	memcpy(&inventory2, &target_sd->status.inventory, sizeof(struct item) * MAX_INVENTORY);

	// check free slot in both inventory
	for(trade_i = 0; trade_i < 10; trade_i++) {
		amount = sd->deal.item[trade_i].amount;
		if (amount > 0) {
			int n = sd->deal.item[trade_i].index;
			// check quantity
			if (amount > inventory[n].amount)
				amount = inventory[n].amount;
			if (amount > 0) {
				data = itemdb_search(inventory[n].nameid);
				i = MAX_INVENTORY;
				// check for non-equipement item
				if (!itemdb_isequip2(data)) {
					for(i = 0; i < MAX_INVENTORY; i++)
						if (inventory2[i].nameid == inventory[n].nameid &&
							inventory2[i].card[0] == inventory[n].card[0] && inventory2[i].card[1] == inventory[n].card[1] &&
							inventory2[i].card[2] == inventory[n].card[2] && inventory2[i].card[3] == inventory[n].card[3]) {
							if (inventory2[i].amount + amount > MAX_AMOUNT)
								return 0;
							inventory2[i].amount += amount;
							inventory[n].amount -= amount;
							if (inventory[n].amount <= 0)
								memset(&inventory[n], 0, sizeof(struct item));
							break;
						}
				}
				// check for equipement
				if (i == MAX_INVENTORY) {
					for(i = 0; i < MAX_INVENTORY; i++) {
						if (inventory2[i].nameid == 0) {
							memcpy(&inventory2[i], &inventory[n], sizeof(struct item));
							inventory2[i].amount = amount;
							inventory[n].amount -= amount;
							if (inventory[n].amount <= 0)
								memset(&inventory[n], 0, sizeof(struct item));
							break;
						}
					}
					if (i == MAX_INVENTORY)
						return 0;
				}
			}
		}
		amount = target_sd->deal.item[trade_i].amount;
		if (amount > 0) {
			int n = target_sd->deal.item[trade_i].index;
			// check quantity
			if (amount > inventory2[n].amount)
				amount = inventory2[n].amount;
			if (amount > 0) {
				// search if it's possible to add item (for full inventory)
				data = itemdb_search(inventory2[n].nameid);
				i = MAX_INVENTORY;
				if (!itemdb_isequip2(data)) {
					for(i = 0; i < MAX_INVENTORY; i++)
						if (inventory[i].nameid == inventory2[n].nameid &&
							inventory[i].card[0] == inventory2[n].card[0] && inventory[i].card[1] == inventory2[n].card[1] &&
							inventory[i].card[2] == inventory2[n].card[2] && inventory[i].card[3] == inventory2[n].card[3]) {
							if (inventory[i].amount + amount > MAX_AMOUNT)
								return 0;
							inventory[i].amount += amount;
							inventory2[n].amount -= amount;
							if (inventory2[n].amount <= 0)
								memset(&inventory2[n], 0, sizeof(struct item));
							break;
						}
				}
				if (i == MAX_INVENTORY) {
					for(i = 0; i < MAX_INVENTORY; i++) {
						if (inventory[i].nameid == 0) {
							memcpy(&inventory[i], &inventory2[n], sizeof(struct item));
							inventory[i].amount = amount;
							inventory2[n].amount -= amount;
							if (inventory2[n].amount <= 0)
								memset(&inventory2[n], 0, sizeof(struct item));
							break;
						}
					}
					if (i == MAX_INVENTORY)
						return 0;
				}
			}
		}
	}

	return 1;
}

/*==========================================
 * Adds an item/qty to the trade window [rewrite by Skotlex] 
 *------------------------------------------
 */
void trade_tradeadditem(struct map_session_data *sd, int index, int amount) {
	struct map_session_data *target_sd;
	int trade_i, trade_weight, nameid;

	nullpo_retv(sd);
	if (!sd->state.trading || (target_sd = map_id2sd(sd->trade_partner)) == NULL || sd->state.deal_locked > 0)
		return; //Can't add stuff.

	if (index == 0)
	{	//Adding Zeny
		if (amount >= 0 && amount <= MAX_ZENY && amount <= sd->status.zeny && // check amount
			(target_sd->status.zeny + amount) <= MAX_ZENY) // fix positiv overflow
		{	//Check Ok
			sd->deal.zeny = amount;
			clif_tradeadditem(sd, target_sd, 0, amount);
		} else //Cancel Transaction
			trade_tradecancel(sd);
		return;
	}
	//Add an Item
	index = index -2; //Why the actual index used is -2?
	//Item checks...
	if (index < 0 || index > MAX_INVENTORY)
		return;
	if (amount < 0 || amount > sd->status.inventory[index].amount)
		return;

	nameid = sd->inventory_data[index]->nameid;

	if (!itemdb_cantrade(nameid, pc_isGM(sd), pc_isGM(target_sd)) &&	//Can't trade
		(pc_get_partner(sd) != target_sd || !itemdb_canpartnertrade(nameid, pc_isGM(sd), pc_isGM(target_sd))))	//Can't partner-trade
	{
		clif_displaymessage (sd->fd, msg_txt(260));
		return;
	}

	for(trade_i = 0; trade_i < 10; trade_i++)
	{	//Locate a trade position
		if (sd->deal.item[trade_i].index == index ||
			sd->deal.item[trade_i].amount == 0)
			break;
	}
	if (trade_i >= 10)	//No space left
		return;

	trade_weight = sd->inventory_data[index]->weight * amount;
	if (target_sd->weight + sd->deal.weight + trade_weight > target_sd->max_weight)
	{	//fail to add item -- the player was over weighted.
		clif_tradeitemok(sd, index, 1);
		return;
	}

	if (sd->deal.item[trade_i].index == index)
	{	//The same item as before is being readjusted.
		if (sd->deal.item[trade_i].amount + amount > sd->status.inventory[index].amount)
		{	//packet deal exploit check
			amount = sd->status.inventory[index].amount - sd->deal.item[trade_i].amount;
			trade_weight = sd->inventory_data[index]->weight * amount;
		}
		sd->deal.item[trade_i].amount += amount;
	} else {	//New deal item
		sd->deal.item[trade_i].index = index;
		sd->deal.item[trade_i].amount = amount;
	}
	sd->deal.weight += trade_weight;

	if (impossible_trade_check(sd))
	{ // check exploit (trade more items that you have)
		trade_tradecancel(sd);
		return;
	}

	clif_tradeitemok(sd, index+2, 0); // Return the index as it was received
	clif_tradeadditem(sd, target_sd, index+2, amount); //index fix
}

/*==========================================
 * アイテム追加完了(ok押し)
 *------------------------------------------
 */
void trade_tradeok(struct map_session_data *sd) {
	struct map_session_data *target_sd;
	int trade_i;

	nullpo_retv(sd);

	// check items
	for(trade_i = 0; trade_i < 10; trade_i++) {
		if ((((sd->deal.item[trade_i].index) >= 0) &&
		    (sd->deal.item[trade_i].amount > sd->status.inventory[sd->deal.item[trade_i].index].amount)) ||
		    (sd->deal.item[trade_i].amount < 0)) {
			trade_tradecancel(sd);
			return;
		}
	}

	// check exploit (trade more items that you have)
	if (impossible_trade_check(sd)) {
		trade_tradecancel(sd);
		return;
	}

	// check zeny
	if (sd->deal.zeny < 0 || sd->deal.zeny > MAX_ZENY || sd->deal.zeny > sd->status.zeny) { // check amount
		trade_tradecancel(sd);
		return;
	}

	if ((target_sd = map_id2sd(sd->trade_partner)) != NULL) {
		sd->state.deal_locked = 1;
		clif_tradeitemok(sd, 0, 0);
		clif_tradedeal_lock(sd, 0);
		clif_tradedeal_lock(target_sd, 1);
	}
}

/*==========================================
 * 取引キャンセル
 *------------------------------------------
 */
void trade_tradecancel(struct map_session_data *sd) {
	struct map_session_data *target_sd;
	int trade_i;

	nullpo_retv(sd);

	if ((target_sd = map_id2sd(sd->trade_partner)) != NULL) {
		for(trade_i = 0; trade_i < 10; trade_i++) { // give items back (only virtual)
			if (sd->deal.item[trade_i].amount != 0) {
				clif_additem(sd, sd->deal.item[trade_i].index, sd->deal.item[trade_i].amount, 0);
				sd->deal.item[trade_i].index = 0;
				sd->deal.item[trade_i].amount = 0;
			}
			if (target_sd->deal.item[trade_i].amount != 0) {
				clif_additem(target_sd, target_sd->deal.item[trade_i].index, target_sd->deal.item[trade_i].amount, 0);
				target_sd->deal.item[trade_i].index = 0;
				target_sd->deal.item[trade_i].amount = 0;
			}
		}
		if (sd->deal.zeny) {
			clif_updatestatus(sd, SP_ZENY);
			sd->deal.zeny = 0;
		}
		if (target_sd->deal.zeny) {
			clif_updatestatus(target_sd, SP_ZENY);
			target_sd->deal.zeny = 0;
		}
		sd->state.deal_locked = 0;
		sd->trade_partner = 0;
		sd->state.trading = 0;
		target_sd->state.deal_locked = 0;
		target_sd->trade_partner = 0;
		target_sd->state.trading = 0;
		clif_tradecancelled(sd);
		clif_tradecancelled(target_sd);
	}
}

/*==========================================
 * 取引許諾(trade押し)
 *------------------------------------------
 */
void trade_tradecommit(struct map_session_data *sd) {
	struct map_session_data *target_sd;
	int trade_i;
	int flag;

	nullpo_retv(sd);

	if (sd->state.trading && (target_sd = map_id2sd(sd->trade_partner)) != NULL) {
		if ((sd->state.deal_locked >= 1) && (target_sd->state.deal_locked >= 1)) { // both have pressed 'ok'
			if (sd->state.deal_locked < 2) { // set locked to 2
				sd->state.deal_locked = 2;
			}
			if (target_sd->state.deal_locked == 2) { // the other one pressed 'trade' too
				// check exploit (trade more items that you have)
				if (impossible_trade_check(sd)) {
					trade_tradecancel(sd);
					return;
				}
				// check exploit (trade more items that you have)
				if (impossible_trade_check(target_sd)) {
					trade_tradecancel(target_sd);
					return;
				}
				// check zenys value against hackers
				if (sd->deal.zeny >= 0 && sd->deal.zeny <= MAX_ZENY && sd->deal.zeny <= sd->status.zeny && // check amount
				    (target_sd->status.zeny + sd->deal.zeny) <= MAX_ZENY && // fix positiv overflow
				    target_sd->deal.zeny >= 0 && target_sd->deal.zeny <= MAX_ZENY && target_sd->deal.zeny <= target_sd->status.zeny && // check amount
				    (sd->status.zeny + target_sd->deal.zeny) <= MAX_ZENY) { // fix positiv overflow

					// check for full inventory (can not add traded items)
					if (!trade_check(sd)) { // check the both players
						trade_tradecancel(sd);
						return;
					}

					// trade is accepted
					for(trade_i = 0; trade_i < 10; trade_i++) {
						if (sd->deal.item[trade_i].amount != 0) {
							int n = sd->deal.item[trade_i].index;

							if (sd->status.inventory[n].amount < sd->deal.item[trade_i].amount)
								sd->deal.item[trade_i].amount = sd->status.inventory[n].amount;

							flag = pc_additem(target_sd, &sd->status.inventory[n], sd->deal.item[trade_i].amount);
							if (flag == 0) {
								//Logs (T)rade [Lupus]
								if(log_config.pick > 0 )
									log_pick(sd, "T", 0, sd->status.inventory[n].nameid, -(sd->deal.item[trade_i].amount), &sd->status.inventory[n]);
									log_pick(target_sd, "T", 0, sd->status.inventory[n].nameid, sd->deal.item[trade_i].amount, &sd->status.inventory[n]);
								//Logs
								pc_delitem(sd, n, sd->deal.item[trade_i].amount, 1);
							} else {
								clif_additem(sd, n, sd->deal.item[trade_i].amount, 0);
							}
							sd->deal.item[trade_i].index = 0;
							sd->deal.item[trade_i].amount = 0;
                                                        
						}
						if (target_sd->deal.item[trade_i].amount != 0) {
							int n = target_sd->deal.item[trade_i].index;

							if (target_sd->status.inventory[n].amount < target_sd->deal.item[trade_i].amount)
								target_sd->deal.item[trade_i].amount = target_sd->status.inventory[n].amount;

							flag = pc_additem(sd, &target_sd->status.inventory[n], target_sd->deal.item[trade_i].amount);
							if (flag == 0) {
								//Logs (T)rade [Lupus]
								if(log_config.pick > 0 )
									log_pick(target_sd, "T", 0, target_sd->status.inventory[n].nameid, -(target_sd->deal.item[trade_i].amount), &target_sd->status.inventory[n]);
									log_pick(sd, "T", 0, target_sd->status.inventory[n].nameid, target_sd->deal.item[trade_i].amount, &target_sd->status.inventory[n]);
								//Logs
								pc_delitem(target_sd, n, target_sd->deal.item[trade_i].amount, 1);
							} else {
								clif_additem(target_sd, n, target_sd->deal.item[trade_i].amount, 0);
							}
							target_sd->deal.item[trade_i].index = 0;
							target_sd->deal.item[trade_i].amount = 0;
						}
					}
					if (sd->deal.zeny) {
						//Logs Zeny (T)rade [Lupus]
						if(log_config.zeny > 0 )
							log_zeny(target_sd, "T", sd, sd->deal.zeny);
						//Logs
						sd->status.zeny -= sd->deal.zeny;
						target_sd->status.zeny += sd->deal.zeny;
					}
					if (target_sd->deal.zeny) {
						//Logs Zeny (T)rade [Lupus]
						if(log_config.zeny > 0 )
							log_zeny(sd, "T", target_sd, target_sd->deal.zeny);
						//Logs
						target_sd->status.zeny -= target_sd->deal.zeny;
						sd->status.zeny += target_sd->deal.zeny;
					}
					if (sd->deal.zeny || target_sd->deal.zeny) {
						clif_updatestatus(sd, SP_ZENY);
						sd->deal.zeny = 0;
						clif_updatestatus(target_sd, SP_ZENY);
						target_sd->deal.zeny = 0;
					}
					sd->state.deal_locked = 0;
					sd->trade_partner = 0;
					sd->state.trading = 0;
					target_sd->state.deal_locked = 0;
					target_sd->trade_partner = 0;
					target_sd->state.trading = 0;
					clif_tradecompleted(sd, 0);
					clif_tradecompleted(target_sd, 0);
					// save both player to avoid crash: they always have no advantage/disadvantage between the 2 players
					chrif_save(sd,0); // do pc_makesavestatus and save storage too
					chrif_save(target_sd,0); // do pc_makesavestatus and save storage too
				// zeny value was modified!!!! hacker with packet modified
				} else {
					trade_tradecancel(sd);
				}
			}
		}
	}
}
