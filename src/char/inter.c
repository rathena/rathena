// $Id: inter.c,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#include "mmo.h"
#include "char.h"
#include "socket.h"
#include "timer.h"
#include "db.h"
#include <string.h>
#include <stdlib.h>

#include "inter.h"
#include "int_party.h"
#include "int_guild.h"
#include "int_storage.h"
#include "int_pet.h"
#include "lock.h"

#define WISDATA_TTL (60*1000)	// Existence time of Wisp/page data (60 seconds)
                             	// that is the waiting time of answers of all map-servers
#define WISDELLIST_MAX 256   	// Number of elements of Wisp/page data deletion list

char inter_log_filename[1024] = "log/inter.log";

char accreg_txt[1024] = "save/accreg.txt";
static struct dbt *accreg_db = NULL;

struct accreg {
	int account_id, reg_num;
	struct global_reg reg[ACCOUNT_REG_NUM];
};

int party_share_level = 10;


// 送信パケット長リスト
int inter_send_packet_length[] = {
	-1,-1,27,-1, -1, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	-1, 7, 0, 0,  0, 0, 0, 0, -1,11, 0, 0,  0, 0,  0, 0,
	35,-1,11,15, 34,29, 7,-1,  0, 0, 0, 0,  0, 0,  0, 0,
	10,-1,15, 0, 79,19, 7,-1,  0,-1,-1,-1, 14,67,186,-1,
	 9, 9,-1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	11,-1, 7, 3,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
};
// 受信パケット長リスト
int inter_recv_packet_length[] = {
	-1,-1, 7,-1, -1, 6, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 6,-1, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0,
	74, 6,52,14, 10,29, 6,-1, 34, 0, 0, 0,  0, 0,  0, 0,
	-1, 6,-1, 0, 55,19, 6,-1, 14,-1,-1,-1, 14,19,186,-1,
	 5, 9, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,
};

struct WisData {
	int id, fd, count, len;
	unsigned long tick;
	unsigned char src[24], dst[24], msg[1024];
};
static struct dbt * wis_db = NULL;
static int wis_dellist[WISDELLIST_MAX], wis_delnum;


//--------------------------------------------------------

// アカウント変数を文字列へ変換
int inter_accreg_tostr(char *str, struct accreg *reg) {
	int j;
	char *p = str;

	p += sprintf(p, "%d\t", reg->account_id);
	for(j = 0; j < reg->reg_num; j++) {
		p += sprintf(p,"%s,%d ", reg->reg[j].str, reg->reg[j].value);
	}

	return 0;
}

// アカウント変数を文字列から変換
int inter_accreg_fromstr(const char *str, struct accreg *reg) {
	int j, v, n;
	char buf[128];
	const char *p = str;

	if (sscanf(p, "%d\t%n", &reg->account_id, &n ) != 1 || reg->account_id <= 0)
		return 1;

	for(j = 0, p += n; j < ACCOUNT_REG_NUM; j++, p += n) {
		if (sscanf(p, "%[^,],%d %n", buf, &v, &n) != 2)
			break;
		memcpy(reg->reg[j].str, buf, 32);
		reg->reg[j].value = v;
	}
	reg->reg_num = j;

	return 0;
}

// アカウント変数の読み込み
int inter_accreg_init() {
	char line[8192];
	FILE *fp;
	int c = 0;
	struct accreg *reg;

	accreg_db = numdb_init();

	if( (fp = fopen(accreg_txt, "r")) == NULL)
		return 1;
	while(fgets(line, sizeof(line)-1, fp)){
		line[sizeof(line)-1] = '\0';

		reg = (struct accreg*)aCalloc(sizeof(struct accreg), 1);
		if (reg == NULL) {
			printf("inter: accreg: out of memory!\n");
			exit(0);
		}
		if (inter_accreg_fromstr(line, reg) == 0 && reg->account_id > 0) {
			numdb_insert(accreg_db, reg->account_id, reg);
		} else {
			printf("inter: accreg: broken data [%s] line %d\n", accreg_txt, c);
			aFree(reg);
		}
		c++;
	}
	fclose(fp);
//	printf("inter: %s read done (%d)\n", accreg_txt, c);

	return 0;
}

// アカウント変数のセーブ用
int inter_accreg_save_sub(void *key, void *data, va_list ap) {
	char line[8192];
	FILE *fp;
	struct accreg *reg = (struct accreg *)data;

	if (reg->reg_num > 0) {
		inter_accreg_tostr(line,reg);
		fp = va_arg(ap, FILE *);
		fprintf(fp, "%s" RETCODE, line);
	}

	return 0;
}

// アカウント変数のセーブ
int inter_accreg_save() {
	FILE *fp;
	int lock;

	if ((fp = lock_fopen(accreg_txt,&lock)) == NULL) {
		printf("int_accreg: cant write [%s] !!! data is lost !!!\n", accreg_txt);
		return 1;
	}
	numdb_foreach(accreg_db, inter_accreg_save_sub,fp);
	lock_fclose(fp, accreg_txt, &lock);
//	printf("inter: %s saved.\n", accreg_txt);

	return 0;
}

//--------------------------------------------------------

/*==========================================
 * 設定ファイルを読み込む
 *------------------------------------------
 */
int inter_config_read(const char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName, "r");
	if (fp == NULL) {
		printf("file not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line) - 1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		line[sizeof(line)-1] = '\0';

		if (sscanf(line,"%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "storage_txt") == 0) {
			strncpy(storage_txt, w2, sizeof(storage_txt));
		} else if (strcmpi(w1, "party_txt") == 0) {
			strncpy(party_txt, w2, sizeof(party_txt));
		} else if (strcmpi(w1, "guild_txt") == 0) {
			strncpy(guild_txt, w2, sizeof(guild_txt));
		} else if (strcmpi(w1, "pet_txt") == 0) {
			strncpy(pet_txt, w2, sizeof(pet_txt));
		} else if (strcmpi(w1, "castle_txt") == 0) {
			strncpy(castle_txt, w2, sizeof(castle_txt));
		} else if (strcmpi(w1, "accreg_txt") == 0) {
			strncpy(accreg_txt, w2, sizeof(accreg_txt));
		} else if (strcmpi(w1, "guild_storage_txt") == 0) {
			strncpy(guild_storage_txt, w2, sizeof(guild_storage_txt));
		} else if (strcmpi(w1, "party_share_level") == 0) {
			party_share_level = atoi(w2);
			if (party_share_level < 0)
				party_share_level = 0;
		} else if (strcmpi(w1, "inter_log_filename") == 0) {
			strncpy(inter_log_filename, w2, sizeof(inter_log_filename));
		} else if (strcmpi(w1, "import") == 0) {
			inter_config_read(w2);
		} else if(strcmpi(w1,"log_inter")==0) {
			log_inter = atoi(w2);
		}
	}
	fclose(fp);

	return 0;
}

// ログ書き出し
int inter_log(char *fmt,...) {
	FILE *logfp;
	va_list ap;

	va_start(ap,fmt);
	logfp = fopen(inter_log_filename, "a");
	if (logfp) {
		vfprintf(logfp, fmt, ap);
		fclose(logfp);
	}
	va_end(ap);

	return 0;
}

// セーブ
int inter_save() {
	inter_party_save();
	inter_guild_save();
	inter_storage_save();
	inter_guild_storage_save();
	inter_pet_save();
	inter_accreg_save();

	return 0;
}

// 初期化
int inter_init(const char *file) {
	inter_config_read(file);

	wis_db = numdb_init();

	inter_party_init();
	inter_guild_init();
	inter_storage_init();
	inter_pet_init();
	inter_accreg_init();

	return 0;
}

// finalize
int accreg_db_final (void *k, void *data, va_list ap) {	
	struct accreg *p = (struct accreg *) data;
	if (p) aFree(p);
	return 0;
}
int wis_db_final (void *k, void *data, va_list ap) {
	struct WisData *p = (struct WisData *) data;
	if (p) aFree(p);
	return 0;
}
void inter_final() {
	numdb_final(accreg_db, accreg_db_final);
	numdb_final(wis_db, wis_db_final);

	inter_party_final();
	inter_guild_final();
	inter_storage_final();
	inter_pet_final();

	return;
}

// マップサーバー接続
int inter_mapif_init(int fd) {
	inter_guild_mapif_init(fd);

	return 0;
}

//--------------------------------------------------------
// sended packets to map-server

// GMメッセージ送信
int mapif_GMmessage(unsigned char *mes, int len, int sfd) {
	unsigned char buf[2048];

	WBUFW(buf,0) = 0x3800;
	WBUFW(buf,2) = len;
	memcpy(WBUFP(buf,4), mes, len - 4);
	mapif_sendallwos(sfd, buf, len);
//	printf("inter server: GM:%d %s\n", len, mes);

	return 0;
}

// Wisp/page transmission to all map-server
int mapif_wis_message(struct WisData *wd) {
	unsigned char buf[2048];

	WBUFW(buf, 0) = 0x3801;
	WBUFW(buf, 2) = 56 + wd->len;
	WBUFL(buf, 4) = wd->id;
	memcpy(WBUFP(buf, 8), wd->src, 24);
	memcpy(WBUFP(buf,32), wd->dst, 24);
	memcpy(WBUFP(buf,56), wd->msg, wd->len);
	wd->count = mapif_sendall(buf, WBUFW(buf,2));

	return 0;
}

// Wisp/page transmission result to map-server
int mapif_wis_end(struct WisData *wd, int flag) {
	unsigned char buf[27];

	WBUFW(buf, 0) = 0x3802;
	memcpy(WBUFP(buf, 2), wd->src, 24);
	WBUFB(buf,26) = flag; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
	mapif_send(wd->fd, buf, 27);
//	printf("inter server wis_end: flag: %d\n", flag);

	return 0;
}

// アカウント変数送信
int mapif_account_reg(int fd, unsigned char *src) {
	unsigned char buf[2048];

	memcpy(WBUFP(buf,0),src,WBUFW(src,2));
	WBUFW(buf, 0) = 0x3804;
	mapif_sendallwos(fd, buf, WBUFW(buf,2));

	return 0;
}

// アカウント変数要求返信
int mapif_account_reg_reply(int fd,int account_id) {
	struct accreg *reg = (struct accreg*)numdb_search(accreg_db,account_id);

	WFIFOW(fd,0) = 0x3804;
	WFIFOL(fd,4) = account_id;
	if (reg == NULL) {
		WFIFOW(fd,2) = 8;
	} else {
		int j, p;
		for(j = 0, p = 8; j < reg->reg_num; j++, p += 36) {
			memcpy(WFIFOP(fd,p), reg->reg[j].str, 32);
			WFIFOL(fd,p+32) = reg->reg[j].value;
		}
		WFIFOW(fd,2) = p;
	}
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

//--------------------------------------------------------

// Existence check of WISP data
int check_ttl_wisdata_sub(void *key, void *data, va_list ap) {
	unsigned long tick;
	struct WisData *wd = (struct WisData *)data;
	tick = va_arg(ap, unsigned long);

	if (DIFF_TICK(tick, wd->tick) > WISDATA_TTL && wis_delnum < WISDELLIST_MAX)
		wis_dellist[wis_delnum++] = wd->id;

	return 0;
}

int check_ttl_wisdata() {
	unsigned long tick = gettick();
	int i;

	do {
		wis_delnum = 0;
		numdb_foreach(wis_db, check_ttl_wisdata_sub, tick);
		for(i = 0; i < wis_delnum; i++) {
			struct WisData *wd = (struct WisData*)numdb_search(wis_db, wis_dellist[i]);
			printf("inter: wis data id=%d time out : from %s to %s\n", wd->id, wd->src, wd->dst);
			// removed. not send information after a timeout. Just no answer for the player
			//mapif_wis_end(wd, 1); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			numdb_erase(wis_db, wd->id);
			aFree(wd);
		}
	} while(wis_delnum >= WISDELLIST_MAX);

	return 0;
}

//--------------------------------------------------------
// received packets from map-server

// GMメッセージ送信
int mapif_parse_GMmessage(int fd) {
	mapif_GMmessage(RFIFOP(fd,4), RFIFOW(fd,2), fd);

	return 0;
}

// Wisp/page request to send
int mapif_parse_WisRequest(int fd) {
	struct WisData* wd;
	static int wisid = 0;
	int index;

	if (RFIFOW(fd,2)-52 >= sizeof(wd->msg)) {
		printf("inter: Wis message size too long.\n");
		return 0;
	} else if (RFIFOW(fd,2)-52 <= 0) { // normaly, impossible, but who knows...
		printf("inter: Wis message doesn't exist.\n");
		return 0;
	}

	// search if character exists before to ask all map-servers
	if ((index = search_character_index((char*)RFIFOP(fd,28))) == -1) {
		unsigned char buf[27];
		WBUFW(buf, 0) = 0x3802;
		memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), 24);
		WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		mapif_send(fd, buf, 27);
	// Character exists. So, ask all map-servers
	} else {
		// to be sure of the correct name, rewrite it
		memset(RFIFOP(fd,28), 0, 24);
		strncpy((char*)RFIFOP(fd,28), search_character_name(index), 24);
		// if source is destination, don't ask other servers.
		if (strcmp((char*)RFIFOP(fd,4),(char*)RFIFOP(fd,28)) == 0) {
			unsigned char buf[27];
			WBUFW(buf, 0) = 0x3802;
			memcpy(WBUFP(buf, 2), RFIFOP(fd, 4), 24);
			WBUFB(buf,26) = 1; // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			mapif_send(fd, buf, 27);
		} else {

			wd = (struct WisData *)aCalloc(sizeof(struct WisData), 1);
			if (wd == NULL){
				printf("inter: WisRequest: out of memory !\n");
				return 0;
			}

			// Whether the failure of previous wisp/page transmission (timeout)
			check_ttl_wisdata();

			wd->id = ++wisid;
			wd->fd = fd;
			wd->len= RFIFOW(fd,2)-52;
			memcpy(wd->src, RFIFOP(fd, 4), 24);
			memcpy(wd->dst, RFIFOP(fd,28), 24);
			memcpy(wd->msg, RFIFOP(fd,52), wd->len);
			wd->tick = gettick();
			numdb_insert(wis_db, wd->id, wd);
			mapif_wis_message(wd);
		}
	}

	return 0;
}

// Wisp/page transmission result
int mapif_parse_WisReply(int fd) {
	int id = RFIFOL(fd,2), flag = RFIFOB(fd,6);
	struct WisData *wd = (struct WisData*)numdb_search(wis_db, id);

	if (wd == NULL)
		return 0;	// This wisp was probably suppress before, because it was timeout of because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif_wis_end(wd, flag); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		numdb_erase(wis_db, id);
		aFree(wd);
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd) {
	unsigned char buf[2048]; // 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B

	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
	WBUFW(buf, 0) = 0x3803;
	mapif_sendall(buf, RFIFOW(fd,2));

	return 0;
}

// アカウント変数保存要求
int mapif_parse_AccReg(int fd) {
	int j, p;
	struct accreg *reg = (struct accreg*)numdb_search(accreg_db, RFIFOL(fd,4));

	if (reg == NULL) {
		if ((reg = (struct accreg*)aCalloc(sizeof(struct accreg), 1)) == NULL) {
			printf("inter: accreg: out of memory !\n");
			exit(0);
		}
		reg->account_id = RFIFOL(fd,4);
		numdb_insert(accreg_db, RFIFOL(fd,4), reg);
	}

	for(j = 0, p = 8; j < ACCOUNT_REG_NUM && p < RFIFOW(fd,2); j++, p += 36) {
		memcpy(reg->reg[j].str, RFIFOP(fd,p), 32);
		reg->reg[j].value = RFIFOL(fd, p + 32);
	}
	reg->reg_num = j;

	mapif_account_reg(fd, RFIFOP(fd,0));	// 他のMAPサーバーに送信

	return 0;
}

// アカウント変数送信要求
int mapif_parse_AccRegRequest(int fd) {
//	printf("mapif: accreg request\n");
	return mapif_account_reg_reply(fd, RFIFOL(fd,2));
}

//--------------------------------------------------------

// map server からの通信（１パケットのみ解析すること）
// エラーなら0(false)、処理できたなら1、
// パケット長が足りなければ2をかえさなければならない
int inter_parse_frommap(int fd) {
	int cmd = RFIFOW(fd,0);
	int len = 0;

	// inter鯖管轄かを調べる
	if (cmd < 0x3000 || cmd >= 0x3000 + (sizeof(inter_recv_packet_length) / sizeof(inter_recv_packet_length[0])))
		return 0;

	// パケット長を調べる
	if ((len = inter_check_length(fd, inter_recv_packet_length[cmd - 0x3000])) == 0)
		return 2;

	switch(cmd) {
	case 0x3000: mapif_parse_GMmessage(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	case 0x3003: mapif_parse_WisToGM(fd); break;
	case 0x3004: mapif_parse_AccReg(fd); break;
	case 0x3005: mapif_parse_AccRegRequest(fd); break;
	default:
		if (inter_party_parse_frommap(fd))
			break;
		if (inter_guild_parse_frommap(fd))
			break;
		if (inter_storage_parse_frommap(fd))
			break;
		if (inter_pet_parse_frommap(fd))
			break;
		return 0;
	}
	RFIFOSKIP(fd, len);

	return 1;
}

// RFIFOのパケット長確認
// 必要パケット長があればパケット長、まだ足りなければ0
int inter_check_length(int fd, int length) {
	if (length == -1) {	// 可変パケット長
		if (RFIFOREST(fd) < 4)	// パケット長が未着
			return 0;
		length = RFIFOW(fd,2);
	}

	if (RFIFOREST(fd) < length)	// パケットが未着
		return 0;

	return length;
}

