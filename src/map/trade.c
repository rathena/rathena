#include <stdio.h>
#include <string.h>

#include "clif.h"
#include "itemdb.h"
#include "map.h"
#include "trade.h"
#include "pc.h"
#include "npc.h"
#include "battle.h"
#include "nullpo.h"
#include "log.h"
#include "chrif.h"

/*==========================================
 * 取引要請を相手に送る
 *------------------------------------------
 */
void trade_traderequest(struct map_session_data *sd,int target_id)
{
	struct map_session_data *target_sd;

	nullpo_retv(sd);

	if((target_sd = map_id2sd(target_id)) != NULL){
		if(!battle_config.invite_request_check) {
			if(target_sd->guild_invite>0 || target_sd->party_invite>0){
				clif_tradestart(sd,2);	// 相手はPT要請中かGuild要請中
				return;
			}
		}
		if((target_sd->trade_partner !=0) || (sd->trade_partner !=0)) {
			trade_tradecancel(sd); //person is in another trade
		}
		else{
			if((pc_isGM(sd) < 60) && (sd->bl.m != target_sd->bl.m
			 || (sd->bl.x - target_sd->bl.x <= -5 || sd->bl.x - target_sd->bl.x >= 5)
			 || (sd->bl.y - target_sd->bl.y <= -5 || sd->bl.y - target_sd->bl.y >= 5))) {
				clif_tradestart(sd,0); //too far
			}
			else if(sd!=target_sd) {
				target_sd->trade_partner = sd->status.account_id;
				sd->trade_partner = target_sd->status.account_id;
				clif_traderequest(target_sd,sd->status.name);
			}
		}
	}
	else{
		clif_tradestart(sd,1); //character does not exist
	}
}

/*==========================================
 * 取引要請
 *------------------------------------------
 */
void trade_tradeack(struct map_session_data *sd,int type)
{
	struct map_session_data *target_sd;

	nullpo_retv(sd);

	if((target_sd = map_id2sd(sd->trade_partner)) != NULL){
		clif_tradestart(target_sd,type);
		clif_tradestart(sd,type);
		if(type == 4){ // Cancel
			sd->deal_locked =0;
			sd->trade_partner=0;
			target_sd->deal_locked=0;
			target_sd->trade_partner=0;
		}
		if(sd->npc_id != 0)
			npc_event_dequeue(sd);
		if(target_sd->npc_id != 0)
			npc_event_dequeue(target_sd);
	}
}

/*==========================================
 * アイテム追加
 *------------------------------------------
 */
void trade_tradeadditem(struct map_session_data *sd, int index, int amount)
{
	struct map_session_data *target_sd;
	int trade_i;
	int trade_weight = 0;
	int c;

	nullpo_retv(sd);

	if (((target_sd = map_id2sd(sd->trade_partner)) != NULL) && (sd->deal_locked < 1)){
		if (index < 2 || index >= MAX_INVENTORY + 2){
			if (index == 0) {
				if (amount > 0 && amount <= MAX_ZENY && amount <= sd->status.zeny && // check amount
				    (target_sd->status.zeny + amount) <= MAX_ZENY) { // fix positiv overflow
					sd->deal_zeny = amount;
					clif_tradeadditem(sd, target_sd, 0, amount);
				} else {
					if (amount != 0) {
						trade_tradecancel(sd);
						return;
					}
				}
			}
		} else if (amount > 0 && amount <= sd->status.inventory[index-2].amount) {
			for(trade_i = 0; trade_i < 10; trade_i++) {
				if (sd->deal_item_amount[trade_i] == 0) {
					trade_weight += sd->inventory_data[index-2]->weight * amount;
					if (target_sd->weight + trade_weight > target_sd->max_weight){
						clif_tradeitemok(sd, index, 1); // fail to add item -- the player was over weighted.
						amount = 0; // [MouseJstr]
					} else {
						for(c = 0; c == trade_i - 1; c++) { // re-deal exploit protection [Valaris]
							if (sd->deal_item_index[c] == index) {
								trade_tradecancel(sd);
								return;
							}
						}
						sd->deal_item_index[trade_i] = index;
						sd->deal_item_amount[trade_i] += amount;
						clif_tradeitemok(sd, index, 0); // success to add item
						clif_tradeadditem(sd, target_sd, index, amount);
					}
					break;
				} else {
					trade_weight += sd->inventory_data[sd->deal_item_index[trade_i]-2]->weight * sd->deal_item_amount[trade_i];
				}
			}
		}
	}
}

/*==========================================
 * アイテム追加完了(ok押し)
 *------------------------------------------
 */
void trade_tradeok(struct map_session_data *sd)
{
	struct map_session_data *target_sd;
	int trade_i;

	nullpo_retv(sd);

	// check items
	for(trade_i=0;trade_i<10;trade_i++) {
		int idx = sd->deal_item_index[trade_i]-2;
		if(((idx >= 0) && (sd->deal_item_amount[trade_i]>sd->status.inventory[idx].amount)) || sd->deal_item_amount[trade_i]<0) {
			trade_tradecancel(sd);
			return;
		}
	
	}

	// check zeny
	if (sd->deal_zeny < 0 || sd->deal_zeny > MAX_ZENY || sd->deal_zeny > sd->status.zeny) { // check amount
		trade_tradecancel(sd);
		return;
	}
	
	if((target_sd = map_id2sd(sd->trade_partner)) != NULL){
		sd->deal_locked=1;
		clif_tradeitemok(sd,0,0);
		clif_tradedeal_lock(sd,0);
		clif_tradedeal_lock(target_sd,1);
	}
}

/*==========================================
 * 取引キャンセル
 *------------------------------------------
 */
void trade_tradecancel(struct map_session_data *sd)
{
	struct map_session_data *target_sd;
	int trade_i;

	nullpo_retv(sd);

	if((target_sd = map_id2sd(sd->trade_partner)) != NULL){
		for(trade_i=0; trade_i<10;trade_i++) { //give items back (only virtual)
			if(sd->deal_item_amount[trade_i] != 0) {
				clif_additem(sd,sd->deal_item_index[trade_i]-2,sd->deal_item_amount[trade_i],0);
				sd->deal_item_index[trade_i] =0;
				sd->deal_item_amount[trade_i]=0;
			}
			if(target_sd->deal_item_amount[trade_i] != 0) {
				clif_additem(target_sd,target_sd->deal_item_index[trade_i]-2,target_sd->deal_item_amount[trade_i],0);
				target_sd->deal_item_index[trade_i] =0;
				target_sd->deal_item_amount[trade_i]=0;
			}
		}
		if(sd->deal_zeny) {
			clif_updatestatus(sd,SP_ZENY);
			sd->deal_zeny=0;
		}
		if(target_sd->deal_zeny) {
			clif_updatestatus(target_sd,SP_ZENY);
			target_sd->deal_zeny=0;
		}
		sd->deal_locked =0;
		sd->trade_partner=0;
		target_sd->deal_locked=0;
		target_sd->trade_partner=0;
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

	if ((target_sd = map_id2sd(sd->trade_partner)) != NULL) {
		if ((sd->deal_locked >= 1) && (target_sd->deal_locked >= 1)) { // both have pressed 'ok'
			if (sd->deal_locked < 2) {sd->deal_locked = 2;} // set locked to 2
			if (target_sd->deal_locked == 2) { // the other one pressed 'trade' too
				// check zenys value against hackers
				if (sd->deal_zeny >= 0 && sd->deal_zeny <= MAX_ZENY && sd->deal_zeny <= sd->status.zeny && // check amount
				    (target_sd->status.zeny + sd->deal_zeny) <= MAX_ZENY && // fix positiv overflow
				    target_sd->deal_zeny >= 0 && target_sd->deal_zeny <= MAX_ZENY && target_sd->deal_zeny <= target_sd->status.zeny && // check amount
				    (sd->status.zeny + target_sd->deal_zeny) <= MAX_ZENY) { // fix positiv overflow

					// trade is accepted
					for(trade_i = 0; trade_i < 10; trade_i++) {
						if (sd->deal_item_amount[trade_i] != 0) {
							int n = sd->deal_item_index[trade_i] - 2;

							if (sd->status.inventory[n].amount < sd->deal_item_amount[trade_i])
								sd->deal_item_amount[trade_i] = sd->status.inventory[n].amount;

							if(log_config.trade > 0)
								log_trade(sd,target_sd,n,sd->deal_item_amount[trade_i]);

							flag = pc_additem(target_sd, &sd->status.inventory[n], sd->deal_item_amount[trade_i]);
							if (flag == 0)
								pc_delitem(sd, n, sd->deal_item_amount[trade_i], 1);
							else
								clif_additem(sd, n, sd->deal_item_amount[trade_i], 0);
							sd->deal_item_index[trade_i] = 0;
							sd->deal_item_amount[trade_i] = 0;
						}
						if (target_sd->deal_item_amount[trade_i] != 0) {
							int n = target_sd->deal_item_index[trade_i] - 2;

							if (target_sd->status.inventory[n].amount < target_sd->deal_item_amount[trade_i])
								target_sd->deal_item_amount[trade_i] = target_sd->status.inventory[n].amount;

							if(log_config.trade > 0)
								log_trade(target_sd,sd,n,target_sd->deal_item_amount[trade_i]);

							flag = pc_additem(sd, &target_sd->status.inventory[n], target_sd->deal_item_amount[trade_i]);
							if (flag == 0)
								pc_delitem(target_sd, n, target_sd->deal_item_amount[trade_i], 1);
							else
								clif_additem(target_sd, n, target_sd->deal_item_amount[trade_i], 0);
							target_sd->deal_item_index[trade_i] = 0;
							target_sd->deal_item_amount[trade_i] = 0;
						}
					}
					if (sd->deal_zeny) {
						if (log_config.trade > 0 && log_config.zeny > 0)
							log_zeny(sd, target_sd, sd->deal_zeny);
						sd->status.zeny -= sd->deal_zeny;
						target_sd->status.zeny += sd->deal_zeny;
					}
					if (target_sd->deal_zeny) {
						if (log_config.trade > 0 && log_config.zeny > 0)
							log_zeny(target_sd, sd, sd->deal_zeny);

						target_sd->status.zeny -= target_sd->deal_zeny;
						sd->status.zeny += target_sd->deal_zeny;
					}
					if (sd->deal_zeny || target_sd->deal_zeny) {
						clif_updatestatus(sd, SP_ZENY);
						sd->deal_zeny = 0;
						clif_updatestatus(target_sd, SP_ZENY);
						target_sd->deal_zeny = 0;
					}
					sd->deal_locked = 0;
					sd->trade_partner = 0;
					target_sd->deal_locked = 0;
					target_sd->trade_partner = 0;
					clif_tradecompleted(sd, 0);
					clif_tradecompleted(target_sd, 0);
					// save both player to avoid crash: they always have no advantage/disadvantage between the 2 players
					chrif_save(sd); // do pc_makesavestatus and save storage too
					chrif_save(target_sd); // do pc_makesavestatus and save storage too
				// zeny value was modified!!!! hacker with packet modified
				} else {
					trade_tradecancel(sd);
				}
			}
		}
	}
}
