// $Id: storage.c,v 1.3 2004/09/25 02:05:22 MouseJstr Exp $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "itemdb.h"
#include "clif.h"
#include "intif.h"
#include "pc.h"
#include "storage.h"
#include "guild.h"
#include "nullpo.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

static struct dbt *storage_db;
static struct dbt *guild_storage_db;

/*==========================================
 * 倉庫内アイテムソート
 *------------------------------------------
 */
int storage_comp_item(const void *_i1, const void *_i2){
struct item *i1=(struct item *)_i1;
struct item *i2=(struct item *)_i2;

	if (i1->nameid == i2->nameid) {
		return 0;
	} else if (!(i1->nameid) || !(i1->amount)){
		return 1;
	} else if (!(i2->nameid) || !(i2->amount)){
		return -1;
	} else {
		return i1->nameid - i2->nameid;
	}
}

 
void sortage_sortitem(struct storage* stor){
	nullpo_retv(stor);

	qsort(stor->storage, MAX_STORAGE, sizeof(struct item), storage_comp_item);
}

void sortage_gsortitem(struct guild_storage* gstor){
	nullpo_retv(gstor);

	qsort(gstor->storage, MAX_GUILD_STORAGE, sizeof(struct item), storage_comp_item);
}

/*==========================================
 * 初期化とか
 *------------------------------------------
 */
int do_init_storage(void) // map.c::do_init()から呼ばれる
{
	storage_db=numdb_init();
	guild_storage_db=numdb_init();
	return 1;
}

void do_final_storage(void) // map.c::do_final()から呼ばれる
{
}

struct storage *account2storage(int account_id)
{
	struct storage *stor;
	stor=numdb_search(storage_db,account_id);
	if(stor == NULL) {
		stor = calloc(sizeof(struct storage), 1);
		if(stor == NULL){
			printf("storage: out of memory!\n");
			exit(0);
		}
		memset(stor,0,sizeof(struct storage));
		stor->account_id=account_id;
		numdb_insert(storage_db,stor->account_id,stor);
	}
	return stor;
}

// Just to ask storage, without creation
struct storage *account2storage2(int account_id) {
	return numdb_search(storage_db, account_id);
}

int storage_delete(int account_id)
{
	struct storage *stor = numdb_search(storage_db,account_id);
	if(stor) {
		numdb_erase(storage_db,account_id);
		free(stor);
	}
	return 0;
}

/*==========================================
 * カプラ倉庫を開く
 *------------------------------------------
 */
int storage_storageopen(struct map_session_data *sd)
{
	struct storage *stor;

	nullpo_retr(0, sd);

	if((stor = numdb_search(storage_db,sd->status.account_id)) != NULL) {
		stor->storage_status = 1;
		sd->state.storage_flag = 0;
		clif_storageitemlist(sd,stor);
		clif_storageequiplist(sd,stor);
		clif_updatestorageamount(sd,stor);
		return 0;
	} else
		intif_request_storage(sd->status.account_id);

	return 1;
}

int storage_storageopen2(struct map_session_data *sd, struct map_session_data *pl_sd)
{
	struct storage *stor;
	if(sd == NULL || pl_sd == NULL)
	{
		printf("storage_storageopen nullpo\n");
		return 0;
	}

	if((stor = numdb_search(storage_db,pl_sd->status.account_id)) != NULL)
	{
		clif_storageitemlist(sd,stor);
		clif_storageequiplist(sd,stor);
		clif_updatestorageamount(sd,stor);
		return 1;
	}
	return 0;
}

/*==========================================
 * カプラ倉庫へアイテム追加
 *------------------------------------------
 */
int storage_additem(struct map_session_data *sd,struct storage *stor,struct item *item_data,int amount)
{
	struct item_data *data;
	int i;

	nullpo_retr(1, sd);
	nullpo_retr(1, stor);
	nullpo_retr(1, item_data);

	if(item_data->nameid <= 0 || amount <= 0)
		return 1;
	nullpo_retr(1, data = itemdb_search(item_data->nameid));

	i=MAX_STORAGE;
	if(!itemdb_isequip2(data)){
		// 装備品ではないので、既所有品なら個数のみ変化させる
		for(i=0;i<MAX_STORAGE;i++){
			if(stor->storage[i].nameid == item_data->nameid &&
				stor->storage[i].card[0] == item_data->card[0] && stor->storage[i].card[1] == item_data->card[1] &&
				stor->storage[i].card[2] == item_data->card[2] && stor->storage[i].card[3] == item_data->card[3]){
				if(stor->storage[i].amount+amount > MAX_AMOUNT)
					return 1;
				stor->storage[i].amount+=amount;
				clif_storageitemadded(sd,stor,i,amount);
				break;
			}
		}
	}
	if(i>=MAX_STORAGE){
		// 装備品か未所有品だったので空き欄へ追加
		for(i=0;i<MAX_STORAGE;i++){
			if(stor->storage[i].nameid==0){
				memcpy(&stor->storage[i],item_data,sizeof(stor->storage[0]));
				stor->storage[i].amount=amount;
				stor->storage_amount++;
				clif_storageitemadded(sd,stor,i,amount);
				clif_updatestorageamount(sd,stor);
				break;
			}
		}
		if(i>=MAX_STORAGE)
			return 1;
	}
	return 0;
}
/*==========================================
 * カプラ倉庫アイテムを減らす
 *------------------------------------------
 */
int storage_delitem(struct map_session_data *sd,struct storage *stor,int n,int amount)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, stor);

	if(stor->storage[n].nameid==0 || stor->storage[n].amount<amount)
		return 1;

	stor->storage[n].amount-=amount;
	if(stor->storage[n].amount==0){
		memset(&stor->storage[n],0,sizeof(stor->storage[0]));
		stor->storage_amount--;
		clif_updatestorageamount(sd,stor);
	}
	clif_storageitemremoved(sd,n,amount);

	return 0;
}
/*==========================================
 * カプラ倉庫へ入れる
 *------------------------------------------
 */
int storage_storageadd(struct map_session_data *sd,int index,int amount)
{
	struct storage *stor;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor=account2storage(sd->status.account_id));

	if( (stor->storage_amount <= MAX_STORAGE) && (stor->storage_status == 1) ) { // storage not full & storage open
		if(index>=0 && index<MAX_INVENTORY) { // valid index
			if( (amount <= sd->status.inventory[index].amount) && (amount > 0) ) { //valid amount
				if(storage_additem(sd,stor,&sd->status.inventory[index],amount)==0)
				// remove item from inventory
					pc_delitem(sd,index,amount,0);
			} // valid amount
		}// valid index
	}// storage not full & storage open

	return 0;
}

/*==========================================
 * カプラ倉庫から出す
 *------------------------------------------
 */
int storage_storageget(struct map_session_data *sd,int index,int amount)
{
	struct storage *stor;
	int flag;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor=account2storage(sd->status.account_id));

	if(stor->storage_status == 1) { //  storage open
		if(index>=0 && index<MAX_STORAGE) { // valid index
			if( (amount <= stor->storage[index].amount) && (amount > 0) ) { //valid amount
				if((flag = pc_additem(sd,&stor->storage[index],amount)) == 0)
					storage_delitem(sd,stor,index,amount);
				else
					clif_additem(sd,0,0,flag);
			} // valid amount
		}// valid index
	}// storage open

	return 0;
}
/*==========================================
 * カプラ倉庫へカートから入れる
 *------------------------------------------
 */
int storage_storageaddfromcart(struct map_session_data *sd,int index,int amount)
{
	struct storage *stor;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor=account2storage(sd->status.account_id));

	if( (stor->storage_amount <= MAX_STORAGE) && (stor->storage_status == 1) ) { // storage not full & storage open
		if(index>=0 && index<MAX_INVENTORY) { // valid index
			if( (amount <= sd->status.cart[index].amount) && (amount > 0) ) { //valid amount
				if(storage_additem(sd,stor,&sd->status.cart[index],amount)==0)
					pc_cart_delitem(sd,index,amount,0);
			} // valid amount
		}// valid index
	}// storage not full & storage open

	return 0;
}

/*==========================================
 * カプラ倉庫からカートへ出す
 *------------------------------------------
 */
int storage_storagegettocart(struct map_session_data *sd,int index,int amount)
{
	struct storage *stor;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor=account2storage(sd->status.account_id));

	if(stor->storage_status == 1) { //  storage open
		if(index>=0 && index<MAX_STORAGE) { // valid index
			if( (amount <= stor->storage[index].amount) && (amount > 0) ) { //valid amount
				if(pc_cart_additem(sd,&stor->storage[index],amount)==0){
					storage_delitem(sd,stor,index,amount);
				}
			} // valid amount
		}// valid index
	}// storage open

	return 0;
}


/*==========================================
 * カプラ倉庫を閉じる
 *------------------------------------------
 */
int storage_storageclose(struct map_session_data *sd)
{
	struct storage *stor;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor=account2storage(sd->status.account_id));

	stor->storage_status=0;
	sd->state.storage_flag = 0;
	clif_storageclose(sd);

	sortage_sortitem(stor);
	return 0;
}

/*==========================================
 * ログアウト時開いているカプラ倉庫の保存
 *------------------------------------------
 */
int storage_storage_quit(struct map_session_data *sd)
{
	struct storage *stor;

	nullpo_retr(0, sd);

	stor = numdb_search(storage_db,sd->status.account_id);
	if(stor) stor->storage_status = 0;

	return 0;
}

int storage_storage_save(struct map_session_data *sd)
{
	struct storage *stor;

	nullpo_retr(0, sd);

	stor=numdb_search(storage_db,sd->status.account_id);
	if(stor) intif_send_storage(stor);

	return 0;
}

struct guild_storage *guild2storage(int guild_id)
{
	struct guild_storage *gs = NULL;
	if(guild_search(guild_id) != NULL) {
		gs=numdb_search(guild_storage_db,guild_id);
		if(gs == NULL) {
			gs = calloc(sizeof(struct guild_storage), 1);
			if(gs==NULL){
				printf("storage: out of memory!\n");
				exit(0);
			}
			gs->guild_id=guild_id;
			numdb_insert(guild_storage_db,gs->guild_id,gs);
		}
	}
	return gs;
}

int guild_storage_delete(int guild_id)
{
	struct guild_storage *gstor = numdb_search(guild_storage_db,guild_id);
	if(gstor) {
		numdb_erase(guild_storage_db,guild_id);
		free(gstor);
	}
	return 0;
}

int storage_guild_storageopen(struct map_session_data *sd)
{
	struct guild_storage *gstor;

	nullpo_retr(0, sd);

	if(sd->status.guild_id <= 0)
		return 2;
	if((gstor = numdb_search(guild_storage_db,sd->status.guild_id)) != NULL) {
		if(gstor->storage_status)
			return 1;
		gstor->storage_status = 1;
		sd->state.storage_flag = 1;
		clif_guildstorageitemlist(sd,gstor);
		clif_guildstorageequiplist(sd,gstor);
		clif_updateguildstorageamount(sd,gstor);
		return 0;
	}
	else {
		gstor = guild2storage(sd->status.guild_id);
		gstor->storage_status = 1;
		intif_request_guild_storage(sd->status.account_id,sd->status.guild_id);
	}

	return 0;
}

int guild_storage_additem(struct map_session_data *sd,struct guild_storage *stor,struct item *item_data,int amount)
{
	struct item_data *data;
	int i;

	nullpo_retr(1, sd);
	nullpo_retr(1, stor);
	nullpo_retr(1, item_data);
	nullpo_retr(1, data = itemdb_search(item_data->nameid));

	if(item_data->nameid <= 0 || amount <= 0)
		return 1;

	i=MAX_GUILD_STORAGE;
	if(!itemdb_isequip2(data)){
		// 装備品ではないので、既所有品なら個数のみ変化させる
		for(i=0;i<MAX_GUILD_STORAGE;i++){
			if(stor->storage[i].nameid == item_data->nameid &&
				stor->storage[i].card[0] == item_data->card[0] && stor->storage[i].card[1] == item_data->card[1] &&
				stor->storage[i].card[2] == item_data->card[2] && stor->storage[i].card[3] == item_data->card[3]){
				if(stor->storage[i].amount+amount > MAX_AMOUNT)
					return 1;
				stor->storage[i].amount+=amount;
				clif_guildstorageitemadded(sd,stor,i,amount);
				break;
			}
		}
	}
	if(i>=MAX_GUILD_STORAGE){
		// 装備品か未所有品だったので空き欄へ追加
		for(i=0;i<MAX_GUILD_STORAGE;i++){
			if(stor->storage[i].nameid==0){
				memcpy(&stor->storage[i],item_data,sizeof(stor->storage[0]));
				stor->storage[i].amount=amount;
				stor->storage_amount++;
				clif_guildstorageitemadded(sd,stor,i,amount);
				clif_updateguildstorageamount(sd,stor);
				break;
			}
		}
		if(i>=MAX_GUILD_STORAGE)
			return 1;
	}
	return 0;
}

int guild_storage_delitem(struct map_session_data *sd,struct guild_storage *stor,int n,int amount)
{
	nullpo_retr(1, sd);
	nullpo_retr(1, stor);

	if(stor->storage[n].nameid==0 || stor->storage[n].amount<amount)
		return 1;

	stor->storage[n].amount-=amount;
	if(stor->storage[n].amount==0){
		memset(&stor->storage[n],0,sizeof(stor->storage[0]));
		stor->storage_amount--;
		clif_updateguildstorageamount(sd,stor);
	}
	clif_storageitemremoved(sd,n,amount);

	return 0;
}

int storage_guild_storageadd(struct map_session_data *sd,int index,int amount)
{
	struct guild_storage *stor;

	nullpo_retr(0, sd);

	if((stor=guild2storage(sd->status.guild_id)) != NULL) {
		if( (stor->storage_amount <= MAX_GUILD_STORAGE) && (stor->storage_status == 1) ) { // storage not full & storage open
			if(index>=0 && index<MAX_INVENTORY) { // valid index
				if( (amount <= sd->status.inventory[index].amount) && (amount > 0) ) { //valid amount
					if(guild_storage_additem(sd,stor,&sd->status.inventory[index],amount)==0)
					// remove item from inventory
						pc_delitem(sd,index,amount,0);
				} // valid amount
			}// valid index
		}// storage not full & storage open
	}

	return 0;
}

int storage_guild_storageget(struct map_session_data *sd,int index,int amount)
{
	struct guild_storage *stor;
	int flag;

	nullpo_retr(0, sd);

	if((stor=guild2storage(sd->status.guild_id)) != NULL) {
		if(stor->storage_status == 1) { //  storage open
			if(index>=0 && index<MAX_GUILD_STORAGE) { // valid index
				if( (amount <= stor->storage[index].amount) && (amount > 0) ) { //valid amount
					if((flag = pc_additem(sd,&stor->storage[index],amount)) == 0)
						guild_storage_delitem(sd,stor,index,amount);
					else
						clif_additem(sd,0,0,flag);
				} // valid amount
			}// valid index
		}// storage open
	}

	return 0;
}

int storage_guild_storageaddfromcart(struct map_session_data *sd,int index,int amount)
{
	struct guild_storage *stor;

	nullpo_retr(0, sd);

	if((stor=guild2storage(sd->status.guild_id)) != NULL) {
		if( (stor->storage_amount <= MAX_GUILD_STORAGE) && (stor->storage_status == 1) ) { // storage not full & storage open
			if(index>=0 && index<MAX_INVENTORY) { // valid index
				if( (amount <= sd->status.cart[index].amount) && (amount > 0) ) { //valid amount
					if(guild_storage_additem(sd,stor,&sd->status.cart[index],amount)==0)
						pc_cart_delitem(sd,index,amount,0);
				} // valid amount
			}// valid index
		}// storage not full & storage open
	}

	return 0;
}

int storage_guild_storagegettocart(struct map_session_data *sd,int index,int amount)
{
	struct guild_storage *stor;

	nullpo_retr(0, sd);

	if((stor=guild2storage(sd->status.guild_id)) != NULL) {
		if(stor->storage_status == 1) { //  storage open
			if(index>=0 && index<MAX_GUILD_STORAGE) { // valid index
				if( (amount <= stor->storage[index].amount) && (amount > 0) ) { //valid amount
					if(pc_cart_additem(sd,&stor->storage[index],amount)==0){
						guild_storage_delitem(sd,stor,index,amount);
					}
				} // valid amount
			}// valid index
		}// storage open
	}

	return 0;
}

int storage_guild_storageclose(struct map_session_data *sd)
{
	struct guild_storage *stor;

	nullpo_retr(0, sd);

	if((stor=guild2storage(sd->status.guild_id)) != NULL) {
		intif_send_guild_storage(sd->status.account_id,stor);
		stor->storage_status = 0;
		sd->state.storage_flag = 0;
		sortage_gsortitem(stor);
	}
	clif_storageclose(sd);

	return 0;
}

int storage_guild_storage_quit(struct map_session_data *sd,int flag)
{
	struct guild_storage *stor;

	nullpo_retr(0, sd);

	stor = numdb_search(guild_storage_db,sd->status.guild_id);
	if(stor) {
		if(!flag)
			intif_send_guild_storage(sd->status.account_id,stor);
		stor->storage_status = 0;
		sd->state.storage_flag = 0;
	}

	return 0;
}
