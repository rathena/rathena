// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "trade.hpp"

#include <cstdio>
#include <cstring>

#include <common/nullpo.hpp>
#include <common/socket.hpp>

#include "atcommand.hpp"
#include "battle.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "storage.hpp"

#define TRADE_DISTANCE 2 ///Max distance from traders to enable a trade to take place.

/**
 * Player initiates a trade request.
 * @param sd : player requesting the trade
 * @param target_sd : player requested
 */
void trade_traderequest(map_session_data *sd, map_session_data *target_sd)
{
	nullpo_retv(sd);

	if (map_getmapflag(sd->m, MF_NOTRADE)) {
		clif_displaymessage (sd->fd, msg_txt(sd,272));
		return; //Can't trade in notrade mapflag maps.
	}

	if (target_sd == nullptr || sd == target_sd) {
		clif_traderesponse(*sd, TRADE_ACK_CHARNOTEXIST);
		return;
	}

	if (target_sd->npc_id) { // Trade fails if you are using an NPC.
		clif_traderesponse(*sd, TRADE_ACK_FAILED);
		return;
	}

	if (!battle_config.invite_request_check) {
		if (target_sd->guild_invite > 0 || target_sd->party_invite > 0 || target_sd->adopt_invite) {
			clif_traderesponse(*sd, TRADE_ACK_FAILED);
			return;
		}
	}

	if ( sd->trade_partner.id != 0 ) { // If a character tries to trade to another one then cancel the previous one
		map_session_data *previous_sd = map_id2sd(sd->trade_partner.id);

		if( previous_sd != nullptr ){
			previous_sd->trade_partner = {0,0};
			clif_tradecancelled( *previous_sd );
		} // Once cancelled then continue to the new one.
		sd->trade_partner = {0,0};
		clif_tradecancelled( *sd );
	}

	if (target_sd->trade_partner.id != 0) {
		clif_traderesponse(*sd, TRADE_ACK_FAILED); // person is in another trade
		return;
	}

	if (!pc_can_give_items(sd) || !pc_can_give_items(target_sd)) { // check if both GMs are allowed to trade
		clif_displaymessage( sd->fd, msg_txt( sd, 246 ) ); // Your GM level doesn't authorize you to perform this action.
		clif_traderesponse(*sd, TRADE_ACK_FAILED); // GM is not allowed to trade
		return;
	}

	// Players can not request trade from far away, unless they are allowed to use @trade.
	if (!pc_can_use_command(sd, "trade", COMMAND_ATCOMMAND) &&
	    (sd->m != target_sd->m || !check_distance_bl(sd, target_sd, TRADE_DISTANCE))) {
		clif_traderesponse(*sd, TRADE_ACK_TOOFAR);
		return ;
	}

	target_sd->trade_partner.id = sd->status.account_id;
	target_sd->trade_partner.lv = sd->status.base_level;

	sd->trade_partner.id = target_sd->status.account_id;
	sd->trade_partner.lv = target_sd->status.base_level;

	clif_traderequest(*target_sd, sd->status.name);
}


/**
 * Reply to a trade-request.
 * @param sd : player receiving the trade request answer
 * @param type : answer code
 *  0: Char is too far
 *  1: Character does not exist
 *  2: Trade failed
 *  3: Accept
 *  4: Cancel
 * Weird enough, the client should only send 3/4
 * and the server is the one that can reply 0~2
 */
void trade_tradeack(map_session_data *sd, int32 type)
{
	map_session_data *tsd;

	nullpo_retv(sd);

	if (sd->state.trading || !sd->trade_partner.id)
		return; // Already trading or no partner set.

	if ((tsd = map_id2sd(sd->trade_partner.id)) == nullptr) {
		clif_traderesponse(*sd, TRADE_ACK_CHARNOTEXIST);
		sd->trade_partner = {0,0};
		return;
	}

	if (tsd->state.trading || tsd->trade_partner.id != sd->id) {
		clif_traderesponse(*sd, TRADE_ACK_FAILED);
		sd->trade_partner = {0,0};
		return; // Already trading or wrong partner.
	}

	if (type == 4) { // Cancel
		clif_traderesponse(*tsd, TRADE_ACK_CANCEL);
		clif_traderesponse(*sd, TRADE_ACK_CANCEL);
		sd->state.deal_locked = 0;
		sd->trade_partner = {0,0};
		tsd->state.deal_locked = 0;
		tsd->trade_partner = {0,0};
		return;
	}

	if (type != 3)
		return; //If client didn't send accept, it's a broken packet?

	// Players can not request trade from far away, unless they are allowed to use @trade.
	// Check here as well since the original character could had warped.
	if (!pc_can_use_command(sd, "trade", COMMAND_ATCOMMAND) &&
	    (sd->m != tsd->m || !check_distance_bl(sd, tsd, TRADE_DISTANCE))) {
		clif_traderesponse(*sd, TRADE_ACK_TOOFAR);
		sd->trade_partner = {0,0};
		tsd->trade_partner = {0,0};
		return;
	}

	// Check if you can start trade.
	if (sd->npc_id || sd->state.vending || sd->state.buyingstore || sd->state.storage_flag ||
		tsd->npc_id || tsd->state.vending || tsd->state.buyingstore || tsd->state.storage_flag) { // Fail
		clif_traderesponse(*sd, TRADE_ACK_FAILED);
		clif_traderesponse(*tsd, TRADE_ACK_FAILED);
		sd->state.deal_locked = 0;
		sd->trade_partner = {0,0};
		tsd->state.deal_locked = 0;
		tsd->trade_partner = {0,0};
		return;
	}

	// Initiate trade
	sd->state.trading = 1;
	tsd->state.trading = 1;
	memset(&sd->deal, 0, sizeof(sd->deal));
	memset(&tsd->deal, 0, sizeof(tsd->deal));
	clif_traderesponse(*tsd, static_cast<e_ack_trade_response>( type ));
	clif_traderesponse(*sd, static_cast<e_ack_trade_response>( type ));
}

/**
 * Check here hacker for duplicate item in trade
 * normal client refuse to have 2 same types of item (except equipment) in same trade window
 * normal client authorise only no equipped item and only from inventory
 * This function could end player connection if too much hack is detected
 * @param sd : player to check
 * @return -1:zeny hack, 0:all fine, 1:item hack
 */
int32 impossible_trade_check(map_session_data *sd)
{
	struct item inventory[MAX_INVENTORY];
	char message_to_gm[200];
	int32 i, index;

	nullpo_retr(1, sd);

	if(sd->deal.zeny > sd->status.zeny) {
		pc_setglobalreg(sd, add_str("ZENY_HACKER"), 1);
		return -1;
	}

	// get inventory of player
	memcpy(&inventory, &sd->inventory.u.items_inventory, sizeof(struct item) * MAX_INVENTORY);

	// remove this part: arrows can be trade and equipped
	// re-added! [celest]
	// remove equipped items (they can not be trade)
	for (i = 0; i < MAX_INVENTORY; i++)
		if (inventory[i].nameid > 0 && inventory[i].equip && !(inventory[i].equip & EQP_AMMO))
			memset(&inventory[i], 0, sizeof(struct item));

	// check items in player inventory
	for(i = 0; i < 10; i++) {
		if (!sd->deal.item[i].amount)
			continue;

		index = sd->deal.item[i].index;

		if (inventory[index].amount < sd->deal.item[i].amount) { // if more than the player have -> hack
			sprintf(message_to_gm, msg_txt(sd,538), sd->status.name, sd->status.account_id); // Hack on trade: character '%s' (account: %d) try to trade more items that he has.
			intif_wis_message_to_gm(wisp_server_name, PC_PERM_RECEIVE_HACK_INFO, message_to_gm);
			sprintf(message_to_gm, msg_txt(sd,539), inventory[index].amount, inventory[index].nameid, sd->deal.item[i].amount); // This player has %d of a kind of item (id: %u), and try to trade %d of them.
			intif_wis_message_to_gm(wisp_server_name, PC_PERM_RECEIVE_HACK_INFO, message_to_gm);
			// if we block people
			if (battle_config.ban_hack_trade < 0) {
				chrif_req_login_operation(-1, sd->status.name, CHRIF_OP_LOGIN_BLOCK, 0, 0, 0); // type: 1 - block
				set_eof(sd->fd); // forced to disconnect because of the hack
				// message about the ban
				strcpy(message_to_gm, msg_txt(sd,540)); //  This player has been definitively blocked.
			// if we ban people
			} else if (battle_config.ban_hack_trade > 0) {
				chrif_req_login_operation(-1, sd->status.name, CHRIF_OP_LOGIN_BAN, battle_config.ban_hack_trade*60, 0, 0); // type: 2 - ban (year, month, day, hour, minute, second)
				set_eof(sd->fd); // forced to disconnect because of the hack
				// message about the ban
				sprintf(message_to_gm, msg_txt(sd,507), battle_config.ban_hack_trade); //  This player has been banned for %d minute(s).
			} else
				// message about the ban
				strcpy(message_to_gm, msg_txt(sd,508)); //  This player hasn't been banned (Ban option is disabled).

			intif_wis_message_to_gm(wisp_server_name, PC_PERM_RECEIVE_HACK_INFO, message_to_gm);
			return 1;
		}

		inventory[index].amount -= sd->deal.item[i].amount; // remove item from inventory
	}
	return 0;
}

/**
 * Checks if trade is possible (against zeny limits, inventory limits, etc)
 * @param sd : player 1 trading
 * @param tsd : player 2 trading
 * @return 0:error, 1:success
 */
int32 trade_check(map_session_data *sd, map_session_data *tsd)
{
	struct item inventory[MAX_INVENTORY];
	struct item inventory2[MAX_INVENTORY];
	struct item_data *data;
	int32 trade_i, i, n;

	// check zeny value against hackers (Zeny was already checked on time of adding, but you never know when you lost some zeny since then.
	if(sd->deal.zeny > sd->status.zeny || (tsd->status.zeny > MAX_ZENY - sd->deal.zeny))
		return 0;
	if(tsd->deal.zeny > tsd->status.zeny || (sd->status.zeny > MAX_ZENY - tsd->deal.zeny))
		return 0;

	// get inventory of player
	memcpy(&inventory, &sd->inventory.u.items_inventory, sizeof(struct item) * MAX_INVENTORY);
	memcpy(&inventory2, &tsd->inventory.u.items_inventory, sizeof(struct item) * MAX_INVENTORY);

	// check free slot in both inventory
	for(trade_i = 0; trade_i < 10; trade_i++) {
		int16 amount;

		amount = sd->deal.item[trade_i].amount;

		if (amount) {
			n = sd->deal.item[trade_i].index;

			if (amount > inventory[n].amount)
				return 0; // Quantity Exploit?

			data = itemdb_search(inventory[n].nameid);
			i = MAX_INVENTORY;
			if (itemdb_isstackable2(data)) { // Stackable item.
				for(i = 0; i < MAX_INVENTORY; i++)
					if (inventory2[i].nameid == inventory[n].nameid &&
						inventory2[i].card[0] == inventory[n].card[0] && inventory2[i].card[1] == inventory[n].card[1] &&
						inventory2[i].card[2] == inventory[n].card[2] && inventory2[i].card[3] == inventory[n].card[3]) {
						if (inventory2[i].amount + amount > MAX_AMOUNT)
							return 0;

						inventory2[i].amount += amount;
						inventory[n].amount -= amount;
						break;
					}
			}

			if (i == MAX_INVENTORY) { // look for an empty slot.
				for(i = 0; i < MAX_INVENTORY && inventory2[i].nameid; i++);
				if (i == MAX_INVENTORY)
					return 0;

				memcpy(&inventory2[i], &inventory[n], sizeof(struct item));
				inventory2[i].amount = amount;
				inventory[n].amount -= amount;
			}
		}

		amount = tsd->deal.item[trade_i].amount;

		if (!amount)
			continue;

		n = tsd->deal.item[trade_i].index;

		if (amount > inventory2[n].amount)
			return 0;

		// search if it's possible to add item (for full inventory)
		data = itemdb_search(inventory2[n].nameid);
		i = MAX_INVENTORY;

		if (itemdb_isstackable2(data)) {
			for(i = 0; i < MAX_INVENTORY; i++)
				if (inventory[i].nameid == inventory2[n].nameid &&
					inventory[i].card[0] == inventory2[n].card[0] && inventory[i].card[1] == inventory2[n].card[1] &&
					inventory[i].card[2] == inventory2[n].card[2] && inventory[i].card[3] == inventory2[n].card[3]) {
					if (inventory[i].amount + amount > MAX_AMOUNT)
						return 0;

					inventory[i].amount += amount;
					inventory2[n].amount -= amount;
					break;
				}
		}

		if (i == MAX_INVENTORY) {
			for(i = 0; i < MAX_INVENTORY && inventory[i].nameid; i++);
			if (i == MAX_INVENTORY)
				return 0;

			memcpy(&inventory[i], &inventory2[n], sizeof(struct item));
			inventory[i].amount = amount;
			inventory2[n].amount -= amount;
		}
	}

	return 1;
}

/**
 * Adds an item/qty to the trade window
 * @param sd : Player requesting to add stuff to the trade
 * @param index : index of item in inventory
 * @param amount : amount of item to add from index
 */
void trade_tradeadditem(map_session_data *sd, int16 index, int16 amount)
{
	map_session_data *target_sd;
	struct item *item;
	int32 trade_i, trade_weight;
	int32 src_lv, dst_lv;

	nullpo_retv(sd);

	if( !sd->state.trading || sd->state.deal_locked > 0 )
		return; // Can't add stuff.

	if( (target_sd = map_id2sd(sd->trade_partner.id)) == nullptr ) {
		trade_tradecancel(sd);
		return;
	}

	if( !amount ) { // Why do this.. ~.~ just send an ack, the item won't display on the trade window.
		clif_tradeitemok(*sd, -2, EXITEM_ADD_SUCCEED); // We pass -2 which will becomes 0 in clif_tradeitemok (Official behavior)
		return;
	}

	// Item checks...
	if( index < 0 || index >= MAX_INVENTORY )
		return;
	if( amount < 0 || amount > sd->inventory.u.items_inventory[index].amount )
		return;

	item = &sd->inventory.u.items_inventory[index];
	src_lv = pc_get_group_level(sd);
	dst_lv = pc_get_group_level(target_sd);

	if( !itemdb_cantrade(item, src_lv, dst_lv) && // Can't trade
		(pc_get_partner(sd) != target_sd || !itemdb_canpartnertrade(item, src_lv, dst_lv)) ) { // Can't partner-trade
		clif_displaymessage (sd->fd, msg_txt(sd,260));
		clif_tradeitemok(*sd, index, EXITEM_ADD_FAILED_CLOSED);
		return;
	}

	if (itemdb_ishatched_egg(item))
		return;

	if( item->expire_time ) { // Rental System
		clif_displaymessage (sd->fd, msg_txt(sd,260));
		clif_tradeitemok(*sd, index, EXITEM_ADD_FAILED_CLOSED);
		return;
	}

	if( ((item->bound == BOUND_ACCOUNT || item->bound > BOUND_GUILD) || (item->bound == BOUND_GUILD && sd->status.guild_id != target_sd->status.guild_id)) && !pc_can_give_bounded_items(sd) ) { // Item Bound
		clif_displaymessage(sd->fd, msg_txt(sd,293));
		clif_tradeitemok(*sd, index, EXITEM_ADD_FAILED_CLOSED);
		return;
	}

	if( item->equipSwitch ){
		clif_msg( *sd, MSI_SWAP_EQUIPITEM_UNREGISTER_FIRST );
		return;
	}

	if (item->bound)
		sd->state.isBoundTrading |= (1<<item->bound);

	// Locate a trade position
	ARR_FIND( 0, 10, trade_i, sd->deal.item[trade_i].index == index || sd->deal.item[trade_i].amount == 0 );
	if( trade_i == 10 ) { // No space left
		// The client does not allow to add more than 10 items, and will show an error message.
		return;
	}

	char add_item = pc_checkadditem(target_sd, item->nameid, amount);
	// Fail to add the item if is stackable and adding the traded amount will exceed the maximum
	if (add_item == CHKADDITEM_OVERAMOUNT) {
		clif_tradeitemok(*sd, index, EXITEM_ADD_FAILED_EACHITEM_OVERCOUNT);
		return;
	}

	// Determines whether the item should be counted when checking for inventory space.
	// If the 'trade_count_stackable' config is enabled, the item will be counted separately even if the recipient already has it.
	bool count_stackable = (battle_config.trade_count_stackable == 1) || (add_item != CHKADDITEM_EXIST);

	// Fail to add the item if the inventory will be full
	if (count_stackable && pc_inventoryblank(target_sd) < sd->deal.inventory_space + 1) {
		clif_tradeitemok(*sd, index, EXITEM_ADD_FAILED_OVERCOUNT);
		return;
	}

	trade_weight = sd->inventory_data[index]->weight * amount;
	if( target_sd->weight + sd->deal.weight + trade_weight > target_sd->max_weight ) { // fail to add item -- the player was over weighted.
		clif_tradeitemok(*sd, index, EXITEM_ADD_FAILED_OVERWEIGHT);
		return;
	}

	if( sd->deal.item[trade_i].index == index ) { // The same item as before is being readjusted.
		if( sd->deal.item[trade_i].amount + amount > sd->inventory.u.items_inventory[index].amount ) { // packet deal exploit check
			amount = sd->inventory.u.items_inventory[index].amount - sd->deal.item[trade_i].amount;
			trade_weight = sd->inventory_data[index]->weight * amount;
		}

		sd->deal.item[trade_i].amount += amount;
	} else { // New deal item
		sd->deal.item[trade_i].index = index;
		sd->deal.item[trade_i].amount = amount;
	}

	sd->deal.weight += trade_weight;

	if (count_stackable)
		sd->deal.inventory_space++;

	clif_tradeitemok(*sd, index, EXITEM_ADD_SUCCEED); // Return the index as it was received
	clif_tradeadditem(sd, target_sd, index+2, amount);
}

/**
 * Adds the specified amount of zeny to the trade window
 * This function will check if the player have enough money to do so
 * And if the target player have enough space for that money
 * @param sd : Player who's adding zeny
 * @param amount : zeny amount
 */
void trade_tradeaddzeny(map_session_data* sd, int32 amount)
{
	map_session_data* target_sd;

	nullpo_retv(sd);

	if( !sd->state.trading || sd->state.deal_locked > 0 )
		return; //Can't add stuff.

	if( (target_sd = map_id2sd(sd->trade_partner.id)) == nullptr ) {
		trade_tradecancel(sd);
		return;
	}

	if( amount < 0 || amount > sd->status.zeny || amount > MAX_ZENY - target_sd->status.zeny ) { // invalid values, no appropriate packet for it => abort
		trade_tradecancel(sd);
		return;
	}

	sd->deal.zeny = amount;
	clif_tradeadditem(sd, target_sd, 0, amount);
}

/**
 * 'Ok' button on the trade window is pressed.
 * @param sd : Player that pressed the button
 */
void trade_tradeok(map_session_data *sd)
{
	map_session_data *target_sd;

	if(sd->state.deal_locked || !sd->state.trading)
		return;

	if ((target_sd = map_id2sd(sd->trade_partner.id)) == nullptr) {
		trade_tradecancel(sd);
		return;
	}

	sd->state.deal_locked = 1;
	clif_tradeitemok(*sd, -2, EXITEM_ADD_SUCCEED); // We pass -2 which will becomes 0 in clif_tradeitemok (Official behavior)
	clif_tradedeal_lock( *sd, false );
	clif_tradedeal_lock( *target_sd, true );
}

/**
 * 'Cancel' is pressed. (or trade was force-cancelled by the code)
 * @param sd : Player that pressed the button
 */
void trade_tradecancel(map_session_data *sd)
{
	map_session_data *target_sd;
	int32 trade_i;

	nullpo_retv(sd);

	target_sd = map_id2sd(sd->trade_partner.id);
	sd->state.isBoundTrading = 0;

	if(!sd->state.trading) { // Not trade accepted
		if( target_sd != nullptr ) {
			target_sd->trade_partner = {0,0};
			clif_tradecancelled( *target_sd );
		}
		sd->trade_partner = {0,0};
		clif_tradecancelled( *sd );
		return;
	}

	for(trade_i = 0; trade_i < 10; trade_i++) { // give items back (only virtual)
		if (!sd->deal.item[trade_i].amount)
			continue;

		clif_additem(sd, sd->deal.item[trade_i].index, sd->deal.item[trade_i].amount, 0);
		sd->deal.item[trade_i].index = 0;
		sd->deal.item[trade_i].amount = 0;
	}

	if (sd->deal.zeny) {
		clif_updatestatus(*sd, SP_ZENY);
		sd->deal.zeny = 0;
	}

	sd->state.deal_locked = 0;
	sd->state.trading = 0;
	sd->trade_partner = {0,0};
	clif_tradecancelled( *sd );

	if (!target_sd)
		return;

	for(trade_i = 0; trade_i < 10; trade_i++) { // give items back (only virtual)
		if (!target_sd->deal.item[trade_i].amount)
			continue;
		clif_additem(target_sd, target_sd->deal.item[trade_i].index, target_sd->deal.item[trade_i].amount, 0);
		target_sd->deal.item[trade_i].index = 0;
		target_sd->deal.item[trade_i].amount = 0;
	}

	if (target_sd->deal.zeny) {
		clif_updatestatus(*target_sd, SP_ZENY);
		target_sd->deal.zeny = 0;
	}

	target_sd->state.deal_locked = 0;
	target_sd->trade_partner = {0,0};
	target_sd->state.trading = 0;
	clif_tradecancelled( *target_sd );
}

/**
 * Execute the trade
 * lock sd and tsd trade data, execute the trade, clear, then save players
 * @param sd : Player that has click on trade button
 */
void trade_tradecommit(map_session_data *sd)
{
	map_session_data *tsd;
	int32 trade_i;

	nullpo_retv(sd);

	if (!sd->state.trading || !sd->state.deal_locked) //Locked should be 1 (pressed ok) before you can press trade.
		return;

	if ((tsd = map_id2sd(sd->trade_partner.id)) == nullptr) {
		trade_tradecancel(sd);
		return;
	}

	sd->state.deal_locked = 2;

	if (tsd->state.deal_locked < 2)
		return; //Not yet time for trading.

	// Now is a good time (to save on resources) to check that the trade can indeed be made and it's not exploitable.
	// check exploit (trade more items that you have)
	if (impossible_trade_check(sd)) {
		trade_tradecancel(sd);
		return;
	}

	// check exploit (trade more items that you have)
	if (impossible_trade_check(tsd)) {
		trade_tradecancel(tsd);
		return;
	}

	// check for full inventory (can not add traded items)
	if (!trade_check(sd,tsd)) { // check the both players
		trade_tradecancel(sd);
		return;
	}

	// trade is accepted and correct.
	for( trade_i = 0; trade_i < 10; trade_i++ ) {
		int32 n;
		unsigned char flag = 0;

		if (sd->deal.item[trade_i].amount) {
			n = sd->deal.item[trade_i].index;

			flag = pc_additem(tsd, &sd->inventory.u.items_inventory[n], sd->deal.item[trade_i].amount,LOG_TYPE_TRADE);
			if (flag == 0)
				pc_delitem(sd, n, sd->deal.item[trade_i].amount, 1, 6, LOG_TYPE_TRADE);
			else
				clif_additem(sd, n, sd->deal.item[trade_i].amount, 0);
			sd->deal.item[trade_i].index = 0;
			sd->deal.item[trade_i].amount = 0;
		}

		if (tsd->deal.item[trade_i].amount) {
			n = tsd->deal.item[trade_i].index;

			flag = pc_additem(sd, &tsd->inventory.u.items_inventory[n], tsd->deal.item[trade_i].amount,LOG_TYPE_TRADE);
			if (flag == 0)
				pc_delitem(tsd, n, tsd->deal.item[trade_i].amount, 1, 6, LOG_TYPE_TRADE);
			else
				clif_additem(tsd, n, tsd->deal.item[trade_i].amount, 0);
			tsd->deal.item[trade_i].index = 0;
			tsd->deal.item[trade_i].amount = 0;
		}
	}

	if( sd->deal.zeny ) {
		pc_payzeny(sd ,sd->deal.zeny, LOG_TYPE_TRADE, tsd->status.char_id);
		pc_getzeny(tsd,sd->deal.zeny,LOG_TYPE_TRADE, sd->status.char_id);
		sd->deal.zeny = 0;

	}

	if ( tsd->deal.zeny) {
		pc_payzeny(tsd,tsd->deal.zeny,LOG_TYPE_TRADE, sd->status.char_id);
		pc_getzeny(sd ,tsd->deal.zeny,LOG_TYPE_TRADE, tsd->status.char_id);
		tsd->deal.zeny = 0;
	}

	sd->state.deal_locked = 0;
	sd->trade_partner = {0,0};
	sd->state.trading = 0;
	sd->state.isBoundTrading = 0;

	tsd->state.deal_locked = 0;
	tsd->trade_partner = {0,0};
	tsd->state.trading = 0;
	tsd->state.isBoundTrading = 0;

	clif_tradecompleted( *sd );
	clif_tradecompleted( *tsd );

	// save both player to avoid crash: they always have no advantage/disadvantage between the 2 players
	if (save_settings&CHARSAVE_TRADE) {
		chrif_save(sd, CSAVE_INVENTORY|CSAVE_CART);
		chrif_save(tsd, CSAVE_INVENTORY|CSAVE_CART);
	}
}
