// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mail.hpp"

#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"

#include "atcommand.hpp"
#include "battle.hpp"
#include "clif.hpp"
#include "date.hpp" // date_get_dayofyear
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "pc.hpp"
#include "pet.hpp"

void mail_clear(struct map_session_data *sd)
{
	int i;

	for( i = 0; i < MAIL_MAX_ITEM; i++ ){
		sd->mail.item[i].nameid = 0;
		sd->mail.item[i].index = 0;
		sd->mail.item[i].amount = 0;
	}
	sd->mail.zeny = 0;

	return;
}

int mail_removeitem(struct map_session_data *sd, short flag, int idx, int amount)
{
	int i;

	nullpo_ret(sd);

	idx -= 2;

	if( idx < 0 || idx >= MAX_INVENTORY )
			return false;
	if( amount <= 0 || amount > sd->inventory.u.items_inventory[idx].amount )
			return false;

	ARR_FIND(0, MAIL_MAX_ITEM, i, sd->mail.item[i].index == idx && sd->mail.item[i].nameid > 0);

	if( i == MAIL_MAX_ITEM ){
		return false;
	}

	if( flag ){
		if( battle_config.mail_attachment_price > 0 ){
			if( pc_payzeny( sd, battle_config.mail_attachment_price, LOG_TYPE_MAIL, NULL ) ){
				return false;
			}
		}

#if PACKETVER < 20150513
		// With client update packet
		pc_delitem(sd, idx, amount, 1, 0, LOG_TYPE_MAIL);
#else
		// RODEX refreshes the client inventory from the ACK packet
		pc_delitem(sd, idx, amount, 0, 0, LOG_TYPE_MAIL);
#endif
	}else{
		sd->mail.item[i].amount -= amount;

		// Item was removed completely
		if( sd->mail.item[i].amount <= 0 ){
			// Move the rest of the array forward
			for( ; i < MAIL_MAX_ITEM - 1; i++ ){
				if ( sd->mail.item[i + 1].nameid == 0 ){
					break;
				}

				sd->mail.item[i].index = sd->mail.item[i+1].index;
				sd->mail.item[i].nameid = sd->mail.item[i+1].nameid;
				sd->mail.item[i].amount = sd->mail.item[i+1].amount;
			}

			// Zero the rest
			for( ; i < MAIL_MAX_ITEM; i++ ){
				sd->mail.item[i].index = 0;
				sd->mail.item[i].nameid = 0;
				sd->mail.item[i].amount = 0;
			}
		}

#if PACKETVER < 20150513
		clif_additem(sd, idx, amount, 0);
#else
		clif_mail_removeitem(sd, true, idx + 2, amount);
#endif
	}

	return 1;
}

bool mail_removezeny( struct map_session_data *sd, bool flag ){
	nullpo_retr( false, sd );

	if( sd->mail.zeny > 0 ){
		//Zeny send
		if( flag ){
			if( pc_payzeny( sd, sd->mail.zeny + sd->mail.zeny * battle_config.mail_zeny_fee / 100, LOG_TYPE_MAIL, NULL ) ){
				return false;
			}
		}else{
			// Update is called by pc_payzeny, so only call it in the else condition
			clif_updatestatus(sd, SP_ZENY);
		}
	}

	sd->mail.zeny = 0;

	return true;
}

/**
* Attempt to set item or zeny to a mail
* @param sd : player attaching the content
* @param idx 0 - Zeny; >= 2 - Inventory item
* @param amount : amout of zeny or number of item
* @return see enum mail_attach_result in mail.hpp
*/
enum mail_attach_result mail_setitem(struct map_session_data *sd, short idx, uint32 amount) {
	if( pc_istrading(sd) )
		return MAIL_ATTACH_ERROR;

	if( idx == 0 ) { // Zeny Transfer
		if( !pc_can_give_items(sd) )
			return MAIL_ATTACH_UNTRADEABLE;

#if PACKETVER < 20150513
		if( amount > sd->status.zeny )
			amount = sd->status.zeny; // TODO: confirm this behavior for old mail system
#else
		if( ( amount + battle_config.mail_zeny_fee / 100 * amount ) > sd->status.zeny )
			return MAIL_ATTACH_ERROR;
#endif

		sd->mail.zeny = amount;
		// clif_updatestatus(sd, SP_ZENY);
		return MAIL_ATTACH_SUCCESS;
	} else { // Item Transfer
		int i;
#if PACKETVER >= 20150513
		int j, total = 0;
#endif

		idx -= 2;

		if( idx < 0 || idx >= MAX_INVENTORY || sd->inventory_data[idx] == nullptr )
			return MAIL_ATTACH_ERROR;

		if (itemdb_ishatched_egg(&sd->inventory.u.items_inventory[idx]))
			return MAIL_ATTACH_ERROR;

		if( sd->inventory.u.items_inventory[idx].equipSwitch ){
			return MAIL_ATTACH_EQUIPSWITCH;
		}

#if PACKETVER < 20150513
		i = 0;
		// Remove existing item
		mail_removeitem(sd, 0, sd->mail.item[i].index + 2, sd->mail.item[i].amount);
#else
		ARR_FIND(0, MAIL_MAX_ITEM, i, sd->mail.item[i].index == idx && sd->mail.item[i].nameid > 0 );
		
		// The same item had already been added to the mail
		if( i < MAIL_MAX_ITEM ){
			// Check if it is stackable
			if( !itemdb_isstackable(sd->mail.item[i].nameid) ){
				return MAIL_ATTACH_ERROR;
			}

			// Check if it exceeds the total amount
			if( ( amount + sd->mail.item[i].amount ) > sd->inventory.u.items_inventory[idx].amount ){
				return MAIL_ATTACH_ERROR;
			}

			// Check if it exceeds the total weight
			if( battle_config.mail_attachment_weight ){
				// Sum up all items to get the current total weight
				for( j = 0; j < MAIL_MAX_ITEM; j++ ){
					total += sd->mail.item[j].amount * ( sd->inventory_data[sd->mail.item[j].index]->weight / 10 );
				}

				// Add the newly added weight to the current total
				total += amount * sd->inventory_data[idx]->weight / 10;

				if( total > battle_config.mail_attachment_weight ){
					return MAIL_ATTACH_WEIGHT;
				}
			}

			sd->mail.item[i].amount += amount;

			return MAIL_ATTACH_SUCCESS;
		}else{
			ARR_FIND(0, MAIL_MAX_ITEM, i, sd->mail.item[i].nameid == 0);

			if( i == MAIL_MAX_ITEM ){
				return MAIL_ATTACH_SPACE;
			}

			// Check if it exceeds the total weight
			if( battle_config.mail_attachment_weight ){
				// Only need to sum up all entries until the new entry
				for( j = 0; j < i; j++ ){
					total += sd->mail.item[j].amount * ( sd->inventory_data[sd->mail.item[j].index]->weight / 10 );
				}

				// Add the newly added weight to the current total
				total += amount * sd->inventory_data[idx]->weight / 10;

				if( total > battle_config.mail_attachment_weight ){
					return MAIL_ATTACH_WEIGHT;
				}
			}
		}
#endif

		if( amount > sd->inventory.u.items_inventory[idx].amount )
			return MAIL_ATTACH_ERROR;
		if( !pc_can_give_items(sd) || sd->inventory.u.items_inventory[idx].expire_time
			|| !itemdb_available(sd->inventory.u.items_inventory[idx].nameid)
			|| !itemdb_canmail(&sd->inventory.u.items_inventory[idx],pc_get_group_level(sd))
			|| (sd->inventory.u.items_inventory[idx].bound && !pc_can_give_bounded_items(sd)) )
			return MAIL_ATTACH_UNTRADEABLE;

		sd->mail.item[i].index = idx;
		sd->mail.item[i].nameid = sd->inventory.u.items_inventory[idx].nameid;
		sd->mail.item[i].amount = amount;
		return MAIL_ATTACH_SUCCESS;
	}
}

bool mail_setattachment(struct map_session_data *sd, struct mail_message *msg)
{
	int i, amount;

	nullpo_retr(false,sd);
	nullpo_retr(false,msg);

	for( i = 0, amount = 0; i < MAIL_MAX_ITEM; i++ ){
		int index = sd->mail.item[i].index;

		if( sd->mail.item[i].nameid == 0 || sd->mail.item[i].amount == 0 ){
			memset(&msg->item[i], 0x00, sizeof(struct item));
			continue;
		}

		amount++;

		if( sd->inventory.u.items_inventory[index].nameid != sd->mail.item[i].nameid )
			return false;

		if( sd->inventory.u.items_inventory[index].amount < sd->mail.item[i].amount )
			return false;

		if( sd->weight > sd->max_weight ) // TODO: Why check something weird like this here?
			return false;

		memcpy(&msg->item[i], &sd->inventory.u.items_inventory[index], sizeof(struct item));
		msg->item[i].amount = sd->mail.item[i].amount;
	}

	if( sd->mail.zeny < 0 || ( sd->mail.zeny + sd->mail.zeny * battle_config.mail_zeny_fee / 100 + amount * battle_config.mail_attachment_price ) > sd->status.zeny )
		return false;

	msg->zeny = sd->mail.zeny;

	// Removes the attachment from sender
	for( i = 0; i < MAIL_MAX_ITEM; i++ ){
		if( sd->mail.item[i].nameid == 0 || sd->mail.item[i].amount == 0 ){
			// Exit the loop on the first empty entry
			break;
		}

		mail_removeitem(sd,1,sd->mail.item[i].index + 2,sd->mail.item[i].amount);
	}
	mail_removezeny(sd,true);

	return true;
}

void mail_getattachment(struct map_session_data* sd, struct mail_message* msg, int zeny, struct item* item){
	int i;
	bool item_received = false;

	for( i = 0; i < MAIL_MAX_ITEM; i++ ){
		if( item[i].nameid > 0 && item[i].amount > 0 ){
			struct item_data* id = itemdb_search( item[i].nameid );

			// Item does not exist (anymore?)
			if( id == nullptr ){
				continue;
			}

			// Reduce the pending weight
			sd->mail.pending_weight -= ( id->weight * item[i].amount );

			// Check if it is a pet egg
			std::shared_ptr<s_pet_db> pet = pet_db_search( item[i].nameid, PET_EGG );

			// If it is a pet egg and the card data does not contain a pet id or other special ids are set
			if( pet != nullptr && item[i].card[0] == 0 ){
				// Create a new pet
				if( pet_create_egg( sd, item[i].nameid ) ){
					sd->mail.pending_slots--;
					item_received = true;
				}else{
					// Do not send receive packet so that the mail is still displayed with item attachment
					item_received = false;
					// Additionally stop the processing
					break;
				}
			}else{
				char check = pc_checkadditem( sd, item[i].nameid, item[i].amount );

				// Add the item normally
				if( check != CHKADDITEM_OVERAMOUNT && pc_additem( sd, &item[i], item[i].amount, LOG_TYPE_MAIL ) == ADDITEM_SUCCESS ){
					item_received = true;

					// Only reduce slots if it really required a new slot
					if( check == CHKADDITEM_NEW ){
						sd->mail.pending_slots -= id->inventorySlotNeeded( item[i].amount );
					}
				}else{
					// Do not send receive packet so that the mail is still displayed with item attachment
					item_received = false;
					// Additionally stop the processing
					break;
				}
			}

			// Make sure no requests are possible anymore
			item[i].amount = 0;
		}	
	}

	if( item_received ){
		clif_mail_getattachment( sd, msg, 0, MAIL_ATT_ITEM );
	}

	// Zeny receive
	if( zeny > 0 ){
		// Reduce the pending zeny
		sd->mail.pending_zeny -= zeny;

		// Add the zeny
		pc_getzeny(sd, zeny,LOG_TYPE_MAIL, NULL);
		clif_mail_getattachment( sd, msg, 0, MAIL_ATT_ZENY );
	}
}

int mail_openmail(struct map_session_data *sd)
{
	nullpo_ret(sd);

	if( sd->state.storage_flag || sd->state.vending || sd->state.buyingstore || sd->state.trading )
		return 0;

	clif_Mail_window(sd->fd, 0);

	return 1;
}

void mail_deliveryfail(struct map_session_data *sd, struct mail_message *msg){
	int i, zeny = 0;

	nullpo_retv(sd);
	nullpo_retv(msg);

	for( i = 0; i < MAIL_MAX_ITEM; i++ ){
		if( msg->item[i].amount > 0 ){
			// Item receive (due to failure)
			pc_additem(sd, &msg->item[i], msg->item[i].amount, LOG_TYPE_MAIL);
			zeny += battle_config.mail_attachment_price;
		}
	}

	if( msg->zeny > 0 ){
		pc_getzeny(sd,msg->zeny + msg->zeny*battle_config.mail_zeny_fee/100 + zeny,LOG_TYPE_MAIL, NULL); //Zeny receive (due to failure)
	}

	clif_Mail_send(sd, WRITE_MAIL_FAILED);
}

// This function only check if the mail operations are valid
bool mail_invalid_operation(struct map_session_data *sd)
{
#if PACKETVER < 20150513
	if( !map_getmapflag(sd->bl.m, MF_TOWN) && !pc_can_use_command(sd, "mail", COMMAND_ATCOMMAND) )
	{
		ShowWarning("clif_parse_Mail: char '%s' trying to do invalid mail operations.\n", sd->status.name);
		return true;
	}
#else
	if( map_getmapflag( sd->bl.m, MF_NORODEX ) ){
		clif_displaymessage( sd->fd, msg_txt( sd, 796 ) ); // You cannot use RODEX on this map.
		return true;
	}
#endif

	return false;
}

/**
* Attempt to send mail
* @param sd Sender
* @param dest_name Destination name
* @param title Mail title
* @param body_msg Mail message
* @param body_len Message's length
*/
void mail_send(struct map_session_data *sd, const char *dest_name, const char *title, const char *body_msg, int body_len) {
	struct mail_message msg;

	nullpo_retv(sd);

	if( sd->state.trading )
		return;

	if( DIFF_TICK(sd->cansendmail_tick, gettick()) > 0 ) {
		clif_displaymessage(sd->fd,msg_txt(sd,675)); //"Cannot send mails too fast!!."
		clif_Mail_send(sd, WRITE_MAIL_FAILED); // fail
		return;
	}

	if( battle_config.mail_daily_count ){
		mail_refresh_remaining_amount(sd);

		// After calling mail_refresh_remaining_amount the status should always be there
		if( sd->sc.data[SC_DAILYSENDMAILCNT] == NULL || sd->sc.data[SC_DAILYSENDMAILCNT]->val2 >= battle_config.mail_daily_count ){
			clif_Mail_send(sd, WRITE_MAIL_FAILED_CNT);
			return;
		}else{
			sc_start2( &sd->bl, &sd->bl, SC_DAILYSENDMAILCNT, 100, date_get_dayofyear(), sd->sc.data[SC_DAILYSENDMAILCNT]->val2 + 1, INFINITE_TICK );
		}
	}

	if( body_len > MAIL_BODY_LENGTH )
		body_len = MAIL_BODY_LENGTH;

	if( !mail_setattachment(sd, &msg) ) { // Invalid Append condition
		int i;

		clif_Mail_send(sd, WRITE_MAIL_FAILED); // fail
		for( i = 0; i < MAIL_MAX_ITEM; i++ ){
			mail_removeitem(sd,0,sd->mail.item[i].index + 2, sd->mail.item[i].amount);
		}
		mail_removezeny(sd,false);
		return;
	}

	msg.id = 0; // id will be assigned by charserver
	msg.send_id = sd->status.char_id;
	msg.dest_id = 0; // will attempt to resolve name
	safestrncpy(msg.send_name, sd->status.name, NAME_LENGTH);
	safestrncpy(msg.dest_name, (char*)dest_name, NAME_LENGTH);
	safestrncpy(msg.title, (char*)title, MAIL_TITLE_LENGTH);
	msg.type = MAIL_INBOX_NORMAL;

	if (msg.title[0] == '\0') {
		return; // Message has no length and somehow client verification was skipped.
	}

	if (body_len)
		safestrncpy(msg.body, (char*)body_msg, min(body_len + 1, MAIL_BODY_LENGTH));
	else
		memset(msg.body, 0x00, MAIL_BODY_LENGTH);

	msg.timestamp = time(NULL);
	if( !intif_Mail_send(sd->status.account_id, &msg) )
		mail_deliveryfail(sd, &msg);

	sd->cansendmail_tick = gettick() + battle_config.mail_delay; // Flood Protection
}

void mail_refresh_remaining_amount( struct map_session_data* sd ){
	int doy = date_get_dayofyear();

	nullpo_retv(sd);

	// If it was not yet started or it was started on another day
	if( sd->sc.data[SC_DAILYSENDMAILCNT] == NULL || sd->sc.data[SC_DAILYSENDMAILCNT]->val1 != doy ){
		sc_start2( &sd->bl, &sd->bl, SC_DAILYSENDMAILCNT, 100, doy, 0, INFINITE_TICK );
	}
}
