// $Id: int_party.c,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#include "inter.h"
#include "int_party.h"
#include "mmo.h"
#include "char.h"
#include "socket.h"
#include "db.h"
#include "lock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char party_txt[1024] = "save/party.txt";

static struct dbt *party_db;
static int party_newid = 100;

int mapif_party_broken(int party_id, int flag);
int party_check_empty(struct party *p);
int mapif_parse_PartyLeave(int fd, int party_id, int account_id);

// パーティデータの文字列への変換
int inter_party_tostr(char *str, struct party *p) {
	int i, len;

	len = sprintf(str, "%d\t%s\t%d,%d\t", p->party_id, p->name, p->exp, p->item);
	for(i = 0; i < MAX_PARTY; i++) {
		struct party_member *m = &p->member[i];
		len += sprintf(str + len, "%d,%d\t%s\t", m->account_id, m->leader, ((m->account_id > 0) ? m->name : "NoMember"));
	}

	return 0;
}

// パーティデータの文字列からの変換
int inter_party_fromstr(char *str, struct party *p) {
	int i, j;
	int tmp_int[16];
	char tmp_str[256];

	memset(p, 0, sizeof(struct party));

//	printf("sscanf party main info\n");
	if (sscanf(str, "%d\t%[^\t]\t%d,%d\t", &tmp_int[0], tmp_str, &tmp_int[1], &tmp_int[2]) != 4)
		return 1;

	p->party_id = tmp_int[0];
	strcpy(p->name, tmp_str);
	p->exp = tmp_int[1];
	p->item = tmp_int[2];
//	printf("%d [%s] %d %d\n", tmp_int[0], tmp_str[0], tmp_int[1], tmp_int[2]);

	for(j = 0; j < 3 && str != NULL; j++)
		str = strchr(str + 1, '\t');

	for(i = 0; i < MAX_PARTY; i++) {
		struct party_member *m = &p->member[i];
		if (str == NULL)
			return 1;
//		printf("sscanf party member info %d\n", i);

		if (sscanf(str + 1, "%d,%d\t%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str) != 3)
			return 1;

		m->account_id = tmp_int[0];
		m->leader = tmp_int[1];
		strncpy(m->name, tmp_str, sizeof(m->name));
//		printf(" %d %d [%s]\n", tmp_int[0], tmp_int[1], tmp_str);

		for(j = 0; j < 2 && str != NULL; j++)
			str = strchr(str + 1, '\t');
	}

	return 0;
}

// パーティデータのロード
int inter_party_init() {
	char line[8192];
	struct party *p;
	FILE *fp;
	int c = 0;
	int i, j;

	party_db = numdb_init();

	if ((fp = fopen(party_txt, "r")) == NULL)
		return 1;

	while(fgets(line, sizeof(line) - 1, fp)) {
		j = 0;
		if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && party_newid <= i) {
			party_newid = i;
			continue;
		}

		p = calloc(sizeof(struct party), 1);
		if (p == NULL){
			printf("int_party: out of memory!\n");
			exit(0);
		}
		memset(p, 0, sizeof(struct party));
		if (inter_party_fromstr(line, p) == 0 && p->party_id > 0) {
			if (p->party_id >= party_newid)
				party_newid = p->party_id + 1;
			numdb_insert(party_db, p->party_id, p);
			party_check_empty(p);
		} else {
			printf("int_party: broken data [%s] line %d\n", party_txt, c + 1);
			free(p);
		}
		c++;
	}
	fclose(fp);
//	printf("int_party: %s read done (%d parties)\n", party_txt, c);

	return 0;
}

// パーティーデータのセーブ用
int inter_party_save_sub(void *key, void *data, va_list ap) {
	char line[8192];
	FILE *fp;

	inter_party_tostr(line, (struct party *)data);
	fp = va_arg(ap, FILE *);
	fprintf(fp, "%s" RETCODE, line);

	return 0;
}

// パーティーデータのセーブ
int inter_party_save() {
	FILE *fp;
	int lock;

	if ((fp = lock_fopen(party_txt, &lock)) == NULL) {
		printf("int_party: cant write [%s] !!! data is lost !!!\n", party_txt);
		return 1;
	}
	numdb_foreach(party_db, inter_party_save_sub, fp);
//	fprintf(fp, "%d\t%%newid%%\n", party_newid);
	lock_fclose(fp,party_txt, &lock);
//	printf("int_party: %s saved.\n", party_txt);

	return 0;
}

// パーティ名検索用
int search_partyname_sub(void *key,void *data,va_list ap) {
	struct party *p = (struct party *)data,**dst;
	char *str;

	str = va_arg(ap, char *);
	dst = va_arg(ap, struct party **);
	if (strcmpi(p->name, str) == 0)
		*dst = p;

	return 0;
}

// パーティ名検索
struct party* search_partyname(char *str) {
	struct party *p = NULL;
	numdb_foreach(party_db, search_partyname_sub, str, &p);

	return p;
}

// EXP公平分配できるかチェック
int party_check_exp_share(struct party *p) {
	int i;
	int maxlv = 0, minlv = 0x7fffffff;

	for(i = 0; i < MAX_PARTY; i++) {
		int lv = p->member[i].lv;
		if (p->member[i].online) {
			if (lv < minlv)
				minlv = lv;
			if (maxlv < lv)
				maxlv = lv;
		}
	}

	return (maxlv == 0 || maxlv-minlv <= party_share_level);
}

// パーティが空かどうかチェック
int party_check_empty(struct party *p) {
	int i;

//	printf("party check empty %08X\n", (int)p);
	for(i = 0; i < MAX_PARTY; i++) {
//		printf("%d acc=%d\n", i, p->member[i].account_id);
		if (p->member[i].account_id > 0) {
			return 0;
		}
	}
		// 誰もいないので解散
	mapif_party_broken(p->party_id, 0);
	numdb_erase(party_db, p->party_id);
	free(p);

	return 1;
}

// キャラの競合がないかチェック用
int party_check_conflict_sub(void *key, void *data, va_list ap) {
	struct party *p = (struct party *)data;
	int party_id, account_id, i;
	char *nick;

	party_id=va_arg(ap, int);
	account_id=va_arg(ap, int);
	nick=va_arg(ap, char *);

	if (p->party_id == party_id)	// 本来の所属なので問題なし
		return 0;

	for(i = 0; i < MAX_PARTY; i++) {
		if (p->member[i].account_id == account_id && strcmp(p->member[i].name, nick) == 0) {
			// 別のパーティに偽の所属データがあるので脱退
			printf("int_party: party conflict! %d %d %d\n", account_id, party_id, p->party_id);
			mapif_parse_PartyLeave(-1, p->party_id, account_id);
		}
	}

	return 0;
}

// キャラの競合がないかチェック
int party_check_conflict(int party_id, int account_id, char *nick) {
	numdb_foreach(party_db, party_check_conflict_sub, party_id, account_id, nick);

	return 0;
}

//-------------------------------------------------------------------
// map serverへの通信

// パーティ作成可否
int mapif_party_created(int fd,int account_id, struct party *p) {
	WFIFOW(fd,0) = 0x3820;
	WFIFOL(fd,2) = account_id;
	if (p != NULL) {
		WFIFOB(fd,6) = 0;
		WFIFOL(fd,7) = p->party_id;
		memcpy(WFIFOP(fd,11), p->name, 24);
		printf("int_party: created! %d %s\n", p->party_id, p->name);
	} else {
		WFIFOB(fd,6) = 1;
		WFIFOL(fd,7) = 0;
		memcpy(WFIFOP(fd,11), "error", 24);
	}
	WFIFOSET(fd,35);

	return 0;
}

// パーティ情報見つからず
int mapif_party_noinfo(int fd, int party_id) {
	WFIFOW(fd,0) = 0x3821;
	WFIFOW(fd,2) = 8;
	WFIFOL(fd,4) = party_id;
	WFIFOSET(fd,8);
	printf("int_party: info not found %d\n", party_id);

	return 0;
}

// パーティ情報まとめ送り
int mapif_party_info(int fd, struct party *p) {
	unsigned char buf[4 + sizeof(struct party)];

	WBUFW(buf,0) = 0x3821;
	memcpy(buf + 4, p, sizeof(struct party));
	WBUFW(buf,2) = 4 + sizeof(struct party);
	if (fd < 0)
		mapif_sendall(buf, WBUFW(buf,2));
	else
		mapif_send(fd, buf, WBUFW(buf,2));
//	printf("int_party: info %d %s\n", p->party_id, p->name);

	return 0;
}

// パーティメンバ追加可否
int mapif_party_memberadded(int fd, int party_id, int account_id, int flag) {
	WFIFOW(fd,0) = 0x3822;
	WFIFOL(fd,2) = party_id;
	WFIFOL(fd,6) = account_id;
	WFIFOB(fd,10) = flag;
	WFIFOSET(fd,11);

	return 0;
}

// パーティ設定変更通知
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
	printf("int_party: option changed %d %d %d %d %d\n", p->party_id, account_id, p->exp, p->item, flag);

	return 0;
}

// パーティ脱退通知
int mapif_party_leaved(int party_id,int account_id, char *name) {
	unsigned char buf[34];

	WBUFW(buf,0) = 0x3824;
	WBUFL(buf,2) = party_id;
	WBUFL(buf,6) = account_id;
	memcpy(WBUFP(buf,10), name, 24);
	mapif_sendall(buf, 34);
	printf("int_party: party leaved %d %d %s\n", party_id, account_id, name);

	return 0;
}

// パーティマップ更新通知
int mapif_party_membermoved(struct party *p, int idx) {
	unsigned char buf[29];

	WBUFW(buf,0) = 0x3825;
	WBUFL(buf,2) = p->party_id;
	WBUFL(buf,6) = p->member[idx].account_id;
	memcpy(WBUFP(buf,10), p->member[idx].map, 16);
	WBUFB(buf,26) = p->member[idx].online;
	WBUFW(buf,27) = p->member[idx].lv;
	mapif_sendall(buf, 29);

	return 0;
}

// パーティ解散通知
int mapif_party_broken(int party_id, int flag) {
	unsigned char buf[7];
	WBUFW(buf,0) = 0x3826;
	WBUFL(buf,2) = party_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf, 7);
	printf("int_party: broken %d\n", party_id);

	return 0;
}

// パーティ内発言
int mapif_party_message(int party_id, int account_id, char *mes, int len) {
	unsigned char buf[len+12];

	WBUFW(buf,0) = 0x3827;
	WBUFW(buf,2) = len + 12;
	WBUFL(buf,4) = party_id;
	WBUFL(buf,8) = account_id;
	memcpy(WBUFP(buf,12), mes, len);
	mapif_sendall(buf,len + 12);

	return 0;
}

//-------------------------------------------------------------------
// map serverからの通信


// パーティ
int mapif_parse_CreateParty(int fd, int account_id, char *name, char *nick, char *map, int lv) {
	struct party *p;
	int i;

	for(i = 0; i < 24 && name[i]; i++) {
		if (!(name[i] & 0xe0) || name[i] == 0x7f) {
			printf("int_party: illegal party name [%s]\n", name);
			mapif_party_created(fd, account_id, NULL);
			return 0;
		}
	}

	if ((p = search_partyname(name)) != NULL) {
		printf("int_party: same name party exists [%s]\n", name);
		mapif_party_created(fd, account_id, NULL);
		return 0;
	}
	p = calloc(sizeof(struct party), 1);
	if (p == NULL) {
		printf("int_party: out of memory !\n");
		mapif_party_created(fd,account_id,NULL);
		return 0;
	}
	memset(p, 0, sizeof(struct party));
	p->party_id = party_newid++;
	memcpy(p->name, name, 24);
	p->exp = 0;
	p->item = 0;
	p->member[0].account_id = account_id;
	memcpy(p->member[0].name, nick, 24);
	memcpy(p->member[0].map, map, 16);
	p->member[0].leader = 1;
	p->member[0].online = 1;
	p->member[0].lv = lv;

	numdb_insert(party_db, p->party_id, p);

	mapif_party_created(fd, account_id, p);
	mapif_party_info(fd, p);

	return 0;
}

// パーティ情報要求
int mapif_parse_PartyInfo(int fd, int party_id) {
	struct party *p;

	p = numdb_search(party_db, party_id);
	if (p != NULL)
		mapif_party_info(fd, p);
	else
		mapif_party_noinfo(fd, party_id);

	return 0;
}

// パーティ追加要求
int mapif_parse_PartyAddMember(int fd, int party_id, int account_id, char *nick, char *map, int lv) {
	struct party *p;
	int i;

	p = numdb_search(party_db, party_id);
	if (p == NULL) {
		mapif_party_memberadded(fd, party_id, account_id, 1);
		return 0;
	}

	for(i = 0; i < MAX_PARTY; i++) {
		if (p->member[i].account_id == 0) {
			int flag = 0;

			p->member[i].account_id = account_id;
			memcpy(p->member[i].name, nick, 24);
			memcpy(p->member[i].map, map, 16);
			p->member[i].leader = 0;
			p->member[i].online = 1;
			p->member[i].lv = lv;
			mapif_party_memberadded(fd, party_id, account_id, 0);
			mapif_party_info(-1, p);

			if (p->exp > 0 && !party_check_exp_share(p)) {
				p->exp = 0;
				flag = 0x01;
			}
			if (flag)
				mapif_party_optionchanged(fd, p, 0, 0);
			return 0;
		}
	}
	mapif_party_memberadded(fd, party_id, account_id, 1);

	return 0;
}

// パーティー設定変更要求
int mapif_parse_PartyChangeOption(int fd, int party_id, int account_id, int exp, int item) {
	struct party *p;
	int flag = 0;

	p = numdb_search(party_db, party_id);
	if (p == NULL)
		return 0;

	p->exp = exp;
	if (exp>0 && !party_check_exp_share(p)) {
		flag |= 0x01;
		p->exp = 0;
	}

	p->item = item;

	mapif_party_optionchanged(fd, p, account_id, flag);
	return 0;
}

// パーティ脱退要求
int mapif_parse_PartyLeave(int fd, int party_id, int account_id) {
	struct party *p;
	int i;

	p = numdb_search(party_db, party_id);
	if (p != NULL) {
		for(i = 0; i < MAX_PARTY; i++) {
			if (p->member[i].account_id == account_id) {
				mapif_party_leaved(party_id, account_id, p->member[i].name);

				memset(&p->member[i], 0, sizeof(struct party_member));
				if (party_check_empty(p) == 0)
					mapif_party_info(-1, p);// まだ人がいるのでデータ送信
				return 0;
			}
		}
	}

	return 0;
}

// パーティマップ更新要求
int mapif_parse_PartyChangeMap(int fd, int party_id, int account_id, char *map, int online, int lv) {
	struct party *p;
	int i;

	p = numdb_search(party_db, party_id);
	if (p == NULL)
		return 0;

	for(i = 0; i < MAX_PARTY; i++) {
		if (p->member[i].account_id == account_id) {
			int flag = 0;

			memcpy(p->member[i].map, map, 16);
			p->member[i].online = online;
			p->member[i].lv = lv;
			mapif_party_membermoved(p, i);

			if (p->exp > 0 && !party_check_exp_share(p)) {
				p->exp = 0;
				flag = 1;
			}
			if (flag)
				mapif_party_optionchanged(fd, p, 0, 0);
			break;
		}
	}

	return 0;
}

// パーティ解散要求
int mapif_parse_BreakParty(int fd, int party_id) {
	struct party *p;

	p = numdb_search(party_db, party_id);
	if (p == NULL)
		return 0;

	numdb_erase(party_db, party_id);
	mapif_party_broken(fd, party_id);

	return 0;
}

// パーティメッセージ送信
int mapif_parse_PartyMessage(int fd, int party_id, int account_id, char *mes, int len) {
	return mapif_party_message(party_id, account_id, mes, len);
}
// パーティチェック要求
int mapif_parse_PartyCheck(int fd, int party_id, int account_id, char *nick) {
	return party_check_conflict(party_id, account_id, nick);
}

// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_party_parse_frommap(int fd) {
	switch(RFIFOW(fd,0)) {
	case 0x3020: mapif_parse_CreateParty(fd, RFIFOL(fd,2), RFIFOP(fd,6), RFIFOP(fd,30), RFIFOP(fd,54), RFIFOW(fd,70)); break;
	case 0x3021: mapif_parse_PartyInfo(fd, RFIFOL(fd,2)); break;
	case 0x3022: mapif_parse_PartyAddMember(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOP(fd,10), RFIFOP(fd,34), RFIFOW(fd,50)); break;
	case 0x3023: mapif_parse_PartyChangeOption(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOW(fd,10), RFIFOW(fd,12)); break;
	case 0x3024: mapif_parse_PartyLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6)); break;
	case 0x3025: mapif_parse_PartyChangeMap(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOP(fd,10), RFIFOB(fd,26), RFIFOW(fd,27)); break;
	case 0x3026: mapif_parse_BreakParty(fd, RFIFOL(fd,2)); break;
	case 0x3027: mapif_parse_PartyMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3028: mapif_parse_PartyCheck(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOP(fd,10)); break;
	default:
		return 0;
	}

	return 1;
}

// サーバーから脱退要求（キャラ削除用）
int inter_party_leave(int party_id, int account_id) {
	return mapif_parse_PartyLeave(-1, party_id, account_id);
}

