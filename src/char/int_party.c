// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/showmsg.h"
#include "char.h"
#include "inter.h"
#include "int_party.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char party_txt[1024] = "save/party.txt";
#ifndef TXT_SQL_CONVERT
struct party_data {
	struct party party;
	unsigned int min_lv, max_lv;
	int family; //Is this party a family? if so, this holds the child id.
	unsigned char size; //Total size of party.
};

static DBMap* party_db; // int party_id -> struct party_data*
static int party_newid = 100;

int mapif_party_broken(int party_id, int flag);
int party_check_empty(struct party *p);
int mapif_parse_PartyLeave(int fd, int party_id, int account_id, int char_id);
int party_check_exp_share(struct party_data *p);
int mapif_party_optionchanged(int fd,struct party *p, int account_id, int flag);

//Updates party's level range and unsets even share if broken.
static int int_party_check_lv(struct party_data *p) {
	int i;
	unsigned int lv;
	p->min_lv = UINT_MAX;
	p->max_lv = 0;
	for(i=0;i<MAX_PARTY;i++){
		if(!p->party.member[i].online)
			continue;

		lv=p->party.member[i].lv;
		if (lv < p->min_lv) p->min_lv = lv;
		if (lv > p->max_lv) p->max_lv = lv;
	}

	if (p->party.exp && !party_check_exp_share(p)) {
		p->party.exp = 0;
		mapif_party_optionchanged(0, &p->party, 0, 0);
		return 0;
	}
	return 1;
}

//Calculates the state of a party.
static void int_party_calc_state(struct party_data *p)
{
	int i;
	unsigned int lv;
	p->min_lv = UINT_MAX;
	p->max_lv = 0;
	p->party.count =
	p->size =
	p->family = 0;

	//Check party size.
	for(i=0;i<MAX_PARTY;i++){
		if (!p->party.member[i].lv) continue;
		p->size++;
		if(p->party.member[i].online)
			p->party.count++;
	}
	if(p->size == 3) {
		//Check Family State.
		p->family = char_family(
			p->party.member[0].char_id,
			p->party.member[1].char_id,
			p->party.member[2].char_id
		);
	}
	//max/min levels.
	for(i=0;i<MAX_PARTY;i++){
		lv=p->party.member[i].lv;
		if (!lv) continue;
		if(p->party.member[i].online &&
			//On families, the kid is not counted towards exp share rules.
			p->party.member[i].char_id != p->family)
		{
			if( lv < p->min_lv ) p->min_lv=lv;
			if( p->max_lv < lv ) p->max_lv=lv;
		}
	}

	if (p->party.exp && !party_check_exp_share(p)) {
		p->party.exp = 0; //Set off even share.
		mapif_party_optionchanged(0, &p->party, 0, 0);
	}
	return;
}

// パ?ティデ?タの文字列への?換
int inter_party_tostr(char *str, struct party *p) {
	int i, len;

	len = sprintf(str, "%d\t%s\t%d,%d\t", p->party_id, p->name, p->exp, p->item);
	for(i = 0; i < MAX_PARTY; i++) {
		struct party_member *m = &p->member[i];
		len += sprintf(str + len, "%d,%d,%d\t", m->account_id, m->char_id, m->leader);
	}

	return 0;
}
#endif //TXT_SQL_CONVERT
// パ?ティデ?タの文字列からの?換
int inter_party_fromstr(char *str, struct party *p) {
	int i, j;
	int tmp_int[16];
	char tmp_str[256];
#ifndef TXT_SQL_CONVERT
	struct mmo_charstatus* status;
#endif
	
	memset(p, 0, sizeof(struct party));

	if (sscanf(str, "%d\t%255[^\t]\t%d,%d\t", &tmp_int[0], tmp_str, &tmp_int[1], &tmp_int[2]) != 4)
		return 1;

	p->party_id = tmp_int[0];
	memcpy(p->name, tmp_str, NAME_LENGTH);
	p->exp = tmp_int[1]?1:0;
	p->item = tmp_int[2];

	for(j = 0; j < 3 && str != NULL; j++)
		str = strchr(str + 1, '\t');

	for(i = 0; i < MAX_PARTY; i++) {
		struct party_member *m = &p->member[i];
		if (str == NULL)
			return 1;

		if (sscanf(str + 1, "%d,%d,%d\t", &tmp_int[0], &tmp_int[1], &tmp_int[2]) != 3)
			return 1;

		m->account_id = tmp_int[0];
		m->char_id = tmp_int[1]; 
		m->leader = tmp_int[2]?1:0;

		str = strchr(str + 1, '\t');
#ifndef TXT_SQL_CONVERT
		if (!m->account_id) continue;
		//Lookup player for rest of data.
		status = search_character(m->account_id, m->char_id);
		if (!status) continue;
		
		memcpy(m->name, status->name, NAME_LENGTH);
		m->class_ = status->class_;
		m->map = status->last_point.map;
		m->lv = status->base_level;
#endif //TXT_SQL_CONVERT
	}

	return 0;
}
#ifndef TXT_SQL_CONVERT
// パ?ティデ?タのロ?ド
int inter_party_init() {
	char line[8192];
	struct party_data *p;
	FILE *fp;
	int c = 0;
	int i, j;

	party_db = idb_alloc(DB_OPT_RELEASE_DATA);

	if ((fp = fopen(party_txt, "r")) == NULL)
		return 1;

	while(fgets(line, sizeof(line), fp))
	{
		j = 0;
		if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && party_newid <= i) {
			party_newid = i;
			continue;
		}

		p = (struct party_data*)aCalloc(sizeof(struct party_data), 1);
		if (p == NULL){
			ShowFatalError("int_party: out of memory!\n");
			exit(EXIT_FAILURE);
		}
		memset(p, 0, sizeof(struct party_data));
		if (inter_party_fromstr(line, &p->party) == 0 && p->party.party_id > 0) {
			int_party_calc_state(p);
			if (p->party.party_id >= party_newid)
				party_newid = p->party.party_id + 1;
			idb_put(party_db, p->party.party_id, p);
			party_check_empty(&p->party);
		} else {
			ShowError("int_party: broken data [%s] line %d\n", party_txt, c + 1);
			aFree(p);
		}
		c++;
	}
	fclose(fp);

	return 0;
}

void inter_party_final()
{
	party_db->destroy(party_db, NULL);
	return;
}

// パ?ティ?デ?タのセ?ブ用
int inter_party_save_sub(DBKey key, void *data, va_list ap) {
	char line[8192];
	FILE *fp;

	inter_party_tostr(line, &((struct party_data*)data)->party);
	fp = va_arg(ap, FILE *);
	fprintf(fp, "%s\n", line);

	return 0;
}

// パ?ティ?デ?タのセ?ブ
int inter_party_save() {
	FILE *fp;
	int lock;

	if ((fp = lock_fopen(party_txt, &lock)) == NULL) {
		ShowError("int_party: can't write [%s] !!! data is lost !!!\n", party_txt);
		return 1;
	}
	party_db->foreach(party_db, inter_party_save_sub, fp);
	lock_fclose(fp,party_txt, &lock);
	return 0;
}

// Search for the party according to its name
struct party_data* search_partyname(char *str)
{
	struct DBIterator* iter;
	struct party_data* p;
	struct party_data* result = NULL;

	iter = party_db->iterator(party_db);
	for( p = (struct party_data*)iter->first(iter,NULL); iter->exists(iter); p = (struct party_data*)iter->next(iter,NULL) )
	{
		if( strncmpi(p->party.name, str, NAME_LENGTH) == 0 )
		{
			result = p;
			break;
		}
	}
	iter->destroy(iter);

	return result;
}

// Returns whether this party can keep having exp share or not.
int party_check_exp_share(struct party_data *p) {
	return (p->party.count < 2|| p->max_lv - p->min_lv <= party_share_level);
}

// パ?ティが空かどうかチェック
int party_check_empty(struct party *p) {
	int i;

	for(i = 0; i < MAX_PARTY; i++) {
		if (p->member[i].account_id > 0) {
			return 0;
		}
	}
	mapif_party_broken(p->party_id, 0);
	idb_remove(party_db, p->party_id);

	return 1;
}

//-------------------------------------------------------------------
// map serverへの通信

// パ?ティ作成可否
int mapif_party_created(int fd,int account_id, int char_id, struct party *p)
{
	WFIFOHEAD(fd, 39);
	WFIFOW(fd,0) = 0x3820;
	WFIFOL(fd,2) = account_id;
	WFIFOL(fd,6) = char_id;
	if (p != NULL) {
		WFIFOB(fd,10) = 0;
		WFIFOL(fd,11) = p->party_id;
		memcpy(WFIFOP(fd,15), p->name, NAME_LENGTH);
		ShowInfo("Created party (%d - %s)\n", p->party_id, p->name);
	} else {
		WFIFOB(fd,10) = 1;
		WFIFOL(fd,11) = 0;
		memset(WFIFOP(fd,15), 0, NAME_LENGTH);
	}
	WFIFOSET(fd,39);
	return 0;
}

// パ?ティ情報見つからず
int mapif_party_noinfo(int fd, int party_id) {
	WFIFOHEAD(fd, 8);
	WFIFOW(fd,0) = 0x3821;
	WFIFOW(fd,2) = 8;
	WFIFOL(fd,4) = party_id;
	WFIFOSET(fd,8);
	ShowWarning("int_party: info not found %d\n", party_id);

	return 0;
}

// パ?ティ情報まとめ送り
int mapif_party_info(int fd, struct party *p) {
	unsigned char buf[2048];

	WBUFW(buf,0) = 0x3821;
	memcpy(buf + 4, p, sizeof(struct party));
	WBUFW(buf,2) = 4 + sizeof(struct party);
	if (fd < 0)
		mapif_sendall(buf, WBUFW(buf,2));
	else
		mapif_send(fd, buf, WBUFW(buf,2));
	return 0;
}

// パ?ティメンバ追加可否
int mapif_party_memberadded(int fd, int party_id, int account_id, int char_id, int flag) {
	WFIFOHEAD(fd, 15);
	WFIFOW(fd,0) = 0x3822;
	WFIFOL(fd,2) = party_id;
	WFIFOL(fd,6) = account_id;
	WFIFOL(fd,10) = char_id;
	WFIFOB(fd,14) = flag;
	WFIFOSET(fd,15);

	return 0;
}

// パ?ティ設定?更通知
int mapif_party_optionchanged(int fd,struct party *p, int account_id, int flag) {
	unsigned char buf[15];

	WBUFW(buf,0) = 0x3823;
	WBUFL(buf,2) = p->party_id;
	WBUFL(buf,6) = account_id;
	WBUFW(buf,10) = p->exp;
	WBUFW(buf,12) = p->item;
	WBUFB(buf,14) = flag;
	if (flag == 0)
		mapif_sendall(buf, 15);
	else
		mapif_send(fd, buf, 15);
	return 0;
}

// パ?ティ?退通知
int mapif_party_withdraw(int party_id,int account_id, int char_id) {
	unsigned char buf[16];

	WBUFW(buf,0) = 0x3824;
	WBUFL(buf,2) = party_id;
	WBUFL(buf,6) = account_id;
	WBUFL(buf,10) = char_id;
	mapif_sendall(buf, 14);
	return 0;
}

// パ?ティマップ更新通知
int mapif_party_membermoved(struct party *p, int idx) {
	unsigned char buf[20];

	WBUFW(buf,0) = 0x3825;
	WBUFL(buf,2) = p->party_id;
	WBUFL(buf,6) = p->member[idx].account_id;
	WBUFL(buf,10) = p->member[idx].char_id;
	WBUFW(buf,14) = p->member[idx].map;
	WBUFB(buf,16) = p->member[idx].online;
	WBUFW(buf,17) = p->member[idx].lv;
	mapif_sendall(buf, 19);
	return 0;
}

// パ?ティ解散通知
int mapif_party_broken(int party_id, int flag) {
	unsigned char buf[7];
	WBUFW(buf,0) = 0x3826;
	WBUFL(buf,2) = party_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf, 7);
	ShowInfo("Party broken (%d)\n", party_id);

	return 0;
}

// パ?ティ??言
int mapif_party_message(int party_id, int account_id, char *mes, int len, int sfd) {
	unsigned char buf[2048];

	WBUFW(buf,0) = 0x3827;
	WBUFW(buf,2) = len + 12;
	WBUFL(buf,4) = party_id;
	WBUFL(buf,8) = account_id;
	memcpy(WBUFP(buf,12), mes, len);
	mapif_sendallwos(sfd, buf,len + 12);

	return 0;
}

//-------------------------------------------------------------------
// map serverからの通信


// パ?ティ
int mapif_parse_CreateParty(int fd, char *name, int item, int item2, struct party_member *leader)
{
	struct party_data *p;
	int i;

	//FIXME: this should be removed once the savefiles can handle all symbols
	for(i = 0; i < NAME_LENGTH && name[i]; i++) {
		if (!(name[i] & 0xe0) || name[i] == 0x7f) {
			ShowInfo("int_party: illegal party name [%s]\n", name);
			mapif_party_created(fd, leader->account_id, leader->char_id, NULL);
			return 0;
		}
	}

	if ((p = search_partyname(name)) != NULL) {
		ShowInfo("int_party: same name party exists [%s]\n", name);
		mapif_party_created(fd, leader->account_id, leader->char_id, NULL);
		return 0;
	}

	// Check Authorised letters/symbols in the name of the character
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) == NULL) {
				mapif_party_created(fd, leader->account_id, leader->char_id, NULL);
				return 0;
			}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) != NULL) {
				mapif_party_created(fd, leader->account_id, leader->char_id, NULL);
				return 0;
			}
	}

	p = (struct party_data *) aCalloc(sizeof(struct party_data), 1);
	if (p == NULL) {
		ShowFatalError("int_party: out of memory !\n");
		mapif_party_created(fd,leader->account_id,leader->char_id,NULL);
		return 0;
	}
	p->party.party_id = party_newid++;
	memcpy(p->party.name, name, NAME_LENGTH);
	p->party.exp = 0;
	p->party.item=(item?1:0)|(item2?2:0);
	memcpy(&p->party.member[0], leader, sizeof(struct party_member));
	p->party.member[0].leader = 1;
	int_party_calc_state(p);
	idb_put(party_db, p->party.party_id, p);

	mapif_party_created(fd, leader->account_id, leader->char_id, &p->party);
	mapif_party_info(fd, &p->party);

	return 0;
}

// パ?ティ情報要求
int mapif_parse_PartyInfo(int fd, int party_id)
{
	struct party_data *p;

	p = (struct party_data*)idb_get(party_db, party_id);
	if (p != NULL)
		mapif_party_info(fd, &p->party);
	else {
		mapif_party_noinfo(fd, party_id);
	}

	return 0;
}

// パーティ追加要求	
int mapif_parse_PartyAddMember(int fd, int party_id, struct party_member *member)
{
	struct party_data *p;
	int i;

	p = (struct party_data*)idb_get(party_db, party_id);
	if( p == NULL || p->size == MAX_PARTY ) {
		mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 1);
		return 0;
	}

	ARR_FIND( 0, MAX_PARTY, i, p->party.member[i].account_id == 0 );
	if( i == MAX_PARTY )
	{// Party full
		mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 1);
		return 0;
	}

	memcpy(&p->party.member[i], member, sizeof(struct party_member));
	p->party.member[i].leader = 0;
	if (p->party.member[i].online) p->party.count++;
	p->size++;
	if (p->size == 3) //Check family state.
		int_party_calc_state(p);
	else //Check even share range.
	if (member->lv < p->min_lv || member->lv > p->max_lv || p->family) {
		if (p->family) p->family = 0; //Family state broken.
		int_party_check_lv(p);
	}

	mapif_party_memberadded(fd, party_id, member->account_id, member->char_id, 0);
	mapif_party_info(-1, &p->party);

	return 0;
}

// パ?ティ?設定?更要求
int mapif_parse_PartyChangeOption(int fd, int party_id, int account_id, int exp, int item)
{
	struct party_data *p;
	int flag = 0;

	p = (struct party_data*)idb_get(party_db, party_id);
	if (p == NULL)
		return 0;

	p->party.exp = exp;
	if (exp>0 && !party_check_exp_share(p)) {
		flag |= 0x01;
		p->party.exp = 0;
	}
	p->party.item = item&0x3;
	mapif_party_optionchanged(fd, &p->party, account_id, flag);
	return 0;
}

// パ?ティ?退要求
int mapif_parse_PartyLeave(int fd, int party_id, int account_id, int char_id)
{
	struct party_data *p;
	int i,lv;

	p = (struct party_data*)idb_get(party_db, party_id);
	if (!p) return 0;

	for(i = 0; i < MAX_PARTY; i++) {
		if(p->party.member[i].account_id == account_id &&
			p->party.member[i].char_id == char_id)
		{
			mapif_party_withdraw(party_id, account_id, char_id);
			lv = p->party.member[i].lv;
			if(p->party.member[i].online) p->party.count--;
			memset(&p->party.member[i], 0, sizeof(struct party_member));
			p->size--;
			if (lv == p->min_lv || lv == p->max_lv || p->family)
			{
				if(p->family) p->family = 0; //Family state broken.
				int_party_check_lv(p);
			}
			if (party_check_empty(&p->party) == 0)
				mapif_party_info(-1, &p->party);
			return 0;
		}
	}
	return 0;
}

int mapif_parse_PartyChangeMap(int fd, int party_id, int account_id, int char_id, unsigned short map, int online, unsigned int lv)
{
	struct party_data *p;
	int i;

	p = (struct party_data*)idb_get(party_db, party_id);
	if (p == NULL)
		return 0;

	for(i = 0; i < MAX_PARTY && 
		(p->party.member[i].account_id != account_id ||
		p->party.member[i].char_id != char_id); i++);

	if (i == MAX_PARTY) return 0;

	if (p->party.member[i].online != online)
	{
		p->party.member[i].online = online;
		if (online)
			p->party.count++;
		else
			p->party.count--;
		// Even share check situations: Family state (always breaks)
		// character logging on/off is max/min level (update level range) 
		// or character logging on/off has a different level (update level range using new level)
		if (p->family ||
			(p->party.member[i].lv <= p->min_lv || p->party.member[i].lv >= p->max_lv) ||
			(p->party.member[i].lv != lv && (lv <= p->min_lv || lv >= p->max_lv))
			)
		{
			p->party.member[i].lv = lv;
			int_party_check_lv(p);
		}
		//Send online/offline update.
		mapif_party_membermoved(&p->party, i);
	}
	if (p->party.member[i].lv != lv) {
		if(p->party.member[i].lv == p->min_lv ||
			p->party.member[i].lv == p->max_lv)
		{
			p->party.member[i].lv = lv;
			int_party_check_lv(p);
		} else
			p->party.member[i].lv = lv;
		//There is no need to send level update to map servers
		//since they do nothing with it.
	}
	if (p->party.member[i].map != map) {
		p->party.member[i].map = map;
		mapif_party_membermoved(&p->party, i);
	}
	return 0;
}

// パ?ティ解散要求
int mapif_parse_BreakParty(int fd, int party_id) {

	idb_remove(party_db, party_id);
	mapif_party_broken(fd, party_id);

	return 0;
}

// パ?ティメッセ?ジ送信
int mapif_parse_PartyMessage(int fd, int party_id, int account_id, char *mes, int len)
{
	return mapif_party_message(party_id, account_id, mes, len, fd);
}

int mapif_parse_PartyLeaderChange(int fd,int party_id,int account_id,int char_id)
{
	struct party_data *p;
	int i;

	p = (struct party_data*)idb_get(party_db, party_id);
	if (p == NULL)
		return 0;

	for (i = 0; i < MAX_PARTY; i++)
	{
		if(p->party.member[i].leader) 
			p->party.member[i].leader = 0;
		if(p->party.member[i].account_id == account_id &&
			p->party.member[i].char_id == char_id)
			p->party.member[i].leader = 1;
	}
	return 1;
}

// map server からの通信
// ?１パケットのみ解析すること
// ?パケット長デ?タはinter.cにセットしておくこと
// ?パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ?エラ?なら0(false)、そうでないなら1(true)をかえさなければならない
int inter_party_parse_frommap(int fd) {
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)) {
	case 0x3020: mapif_parse_CreateParty(fd, (char*)RFIFOP(fd,4), RFIFOB(fd,28), RFIFOB(fd,29), (struct party_member*)RFIFOP(fd,30)); break;
	case 0x3021: mapif_parse_PartyInfo(fd, RFIFOL(fd,2)); break;
	case 0x3022: mapif_parse_PartyAddMember(fd, RFIFOL(fd,4), (struct party_member*)RFIFOP(fd,8)); break;
	case 0x3023: mapif_parse_PartyChangeOption(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOW(fd,10), RFIFOW(fd,12)); break;
	case 0x3024: mapif_parse_PartyLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	case 0x3025: mapif_parse_PartyChangeMap(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOW(fd,14), RFIFOB(fd,16), RFIFOW(fd,17)); break;
	case 0x3026: mapif_parse_BreakParty(fd, RFIFOL(fd,2)); break;
	case 0x3027: mapif_parse_PartyMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), (char*)RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3029: mapif_parse_PartyLeaderChange(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	default:
		return 0;
	}

	return 1;
}

// サ?バ?から?退要求（キャラ削除用）
int inter_party_leave(int party_id, int account_id, int char_id) {
	return mapif_parse_PartyLeave(-1, party_id, account_id, char_id);
}
#endif //TXT_SQL_CONVERT
