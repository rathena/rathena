// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "char.h"
#include "inter.h"
#include "int_storage.h"
#include "int_guild.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char guild_txt[1024] = "save/guild.txt";
char castle_txt[1024] = "save/castle.txt";

#ifndef TXT_SQL_CONVERT
static DBMap* guild_db; // int guild_id -> struct guild*
static DBMap* castle_db; // int castle_id -> struct guild_castle*

static int guild_newid = 10000;

static unsigned int guild_exp[100];

int mapif_guild_broken(int guild_id, int flag);
static bool guild_check_empty(struct guild *g);
int guild_calcinfo(struct guild *g);
int mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len);
int mapif_guild_info(int fd, struct guild *g);
int guild_break_sub(DBKey key, void *data, va_list ap);

/// serializes the guild data structure into the provided string
int inter_guild_tostr(char* str, struct guild* g)
{
	int i, c, len;

	// save guild base info
	len = sprintf(str, "%d\t%s\t%s\t%d,%d,%u,%d,%d\t%s#\t%s#\t",
		g->guild_id, g->name, g->master, g->guild_lv, g->max_member, g->exp, g->skill_point, 0, g->mes1, g->mes2);

	// save guild member info
	for(i = 0; i < g->max_member; i++)
	{
		struct guild_member *m = &g->member[i];
		len += sprintf(str + len, "%d,%d,%d,%d,%d,%d,%d,%u,%d,%d\t%s\t",
		               m->account_id, m->char_id,
		               m->hair, m->hair_color, m->gender,
		               m->class_, m->lv, m->exp, m->exp_payper, m->position,
		               ((m->account_id > 0) ? m->name : "-"));
	}

	// save guild position info
	for(i = 0; i < MAX_GUILDPOSITION; i++) {
		struct guild_position *p = &g->position[i];
		len += sprintf(str + len, "%d,%d\t%s#\t", p->mode, p->exp_mode, p->name);
	}

	// save guild emblem
	len += sprintf(str + len, "%d,%d,", g->emblem_len, g->emblem_id);
	for(i = 0; i < g->emblem_len; i++) {
		len += sprintf(str + len, "%02x", (unsigned char)(g->emblem_data[i]));
	}
	len += sprintf(str + len, "$\t");

	// save guild alliance info
	c = 0;
	for(i = 0; i < MAX_GUILDALLIANCE; i++)
		if (g->alliance[i].guild_id > 0)
			c++;
	len += sprintf(str + len, "%d\t", c);
	for(i = 0; i < MAX_GUILDALLIANCE; i++) {
		struct guild_alliance *a = &g->alliance[i];
		if (a->guild_id > 0)
			len += sprintf(str + len, "%d,%d\t%s\t", a->guild_id, a->opposition, a->name);
	}

	// save guild expulsion info
	c = 0;
	for(i = 0; i < MAX_GUILDEXPULSION; i++)
		if (g->expulsion[i].account_id > 0)
			c++;
	len += sprintf(str + len, "%d\t", c);
	for(i = 0; i < MAX_GUILDEXPULSION; i++) {
		struct guild_expulsion *e = &g->expulsion[i];
		if (e->account_id > 0)
			len += sprintf(str + len, "%d,%d,%d,%d\t%s\t%s\t%s#\t",
			               e->account_id, 0, 0, 0, e->name, "#", e->mes );
	}

	// save guild skill info
	for(i = 0; i < MAX_GUILDSKILL; i++) {
		len += sprintf(str + len, "%d,%d ", g->skill[i].id, g->skill[i].lv);
	}
	len += sprintf(str + len, "\t");

	return 0;
}
#endif //TXT_SQL_CONVERT

/// parses the guild data string into a guild data structure
int inter_guild_fromstr(char* str, struct guild* g)
{
	int i, c;
	char *pstr;

	memset(g, 0, sizeof(struct guild));

	{// load guild base info
		int guildid;
		char name[256]; // only 24 used
		char master[256]; // only 24 used
		int guildlv;
		int max_member;
		unsigned int exp;
		int skpoint;
		char mes1[256]; // only 60 used
		char mes2[256]; // only 120 used
		int len;

		if( sscanf(str, "%d\t%[^\t]\t%[^\t]\t%d,%d,%u,%d,%*d\t%[^\t]\t%[^\t]\t%n",
				   &guildid, name, master, &guildlv, &max_member, &exp, &skpoint, mes1, mes2, &len) < 9 )
			return 1;

		// remove '#'
		mes1[strlen(mes1)-1] = '\0';
		mes2[strlen(mes2)-1] = '\0';

		g->guild_id = guildid;
		g->guild_lv = guildlv;
		g->max_member = max_member;
		g->exp = exp;
		g->skill_point = skpoint;
		safestrncpy(g->name, name, sizeof(g->name));
		safestrncpy(g->master, master, sizeof(g->master));
		safestrncpy(g->mes1, mes1, sizeof(g->mes1));
		safestrncpy(g->mes2, mes2, sizeof(g->mes2));

		str+= len;
	}

	{// load guild member info
		int accountid;
		int charid;
		int hair, hair_color, gender;
		int class_, lv;
		unsigned int exp;
		int exp_payper;
		int position;
		char name[256]; // only 24 used
		int len;
		int i;

		for( i = 0; i < g->max_member; i++ )
		{
			struct guild_member* m = &g->member[i];
			if (sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%u,%d,%d\t%[^\t]\t%n",
					   &accountid, &charid, &hair, &hair_color, &gender,
					   &class_, &lv, &exp, &exp_payper, &position,
					   name, &len) < 11)
				return 1;

			m->account_id = accountid;
			m->char_id = charid;
			m->hair = hair;
			m->hair_color = hair_color;
			m->gender = gender;
			m->class_ = class_;
			m->lv = lv;
			m->exp = exp;
			m->exp_payper = exp_payper;
			m->position = position;
			safestrncpy(m->name, name, NAME_LENGTH);

			str+= len;
		}
	}

	{// load guild position info
		int mode, exp_mode;
		char name[256]; // only 24 used
		int len;
		int i = 0;
		int j;

		while (sscanf(str, "%d,%d%n", &mode, &exp_mode, &j) == 2 && str[j] == '\t')
		{
			struct guild_position *p = &g->position[i];
			if (sscanf(str, "%d,%d\t%[^\t]\t%n", &mode, &exp_mode, name, &len) < 3)
				return 1;

			p->mode = mode;
			p->exp_mode = exp_mode;
			name[strlen(name)-1] = 0;
			safestrncpy(p->name, name, NAME_LENGTH);

			i++;
			str+= len;
		}
	}

	{// load guild emblem
		int emblemlen;
		int emblemid;
		char emblem[4096];
		int len;

		emblemid = 0;
		if( sscanf(str, "%d,%d,%[^\t]\t%n", &emblemlen, &emblemid, emblem, &len) < 3 )
		if( sscanf(str, "%d,%[^\t]\t%n", &emblemlen, emblem, &len) < 2 ) //! pre-svn format
			return 1;

		g->emblem_len = emblemlen;
		g->emblem_id = emblemid;
		for(i = 0, pstr = emblem; i < g->emblem_len; i++, pstr += 2) {
			int c1 = pstr[0], c2 = pstr[1], x1 = 0, x2 = 0;
			if (c1 >= '0' && c1 <= '9') x1 = c1 - '0';
			if (c1 >= 'a' && c1 <= 'f') x1 = c1 - 'a' + 10;
			if (c1 >= 'A' && c1 <= 'F') x1 = c1 - 'A' + 10;
			if (c2 >= '0' && c2 <= '9') x2 = c2 - '0';
			if (c2 >= 'a' && c2 <= 'f') x2 = c2 - 'a' + 10;
			if (c2 >= 'A' && c2 <= 'F') x2 = c2 - 'A' + 10;
			g->emblem_data[i] = (x1<<4) | x2;
		}

		str+= len;
	}

	{// load guild alliance info
		int guildid;
		int opposition;
		char name[256]; // only 24 used
		int len;

		if (sscanf(str, "%d\t%n", &c, &len) < 1)
			return 1;
		str+= len;

		for(i = 0; i < c; i++)
		{
			struct guild_alliance* a = &g->alliance[i];
			if (sscanf(str, "%d,%d\t%[^\t]\t%n", &guildid, &opposition, name, &len) < 3)
				return 1;

			a->guild_id = guildid;
			a->opposition = opposition;
			safestrncpy(a->name, name, NAME_LENGTH);

			str+= len;
		}
	}

	{// load guild expulsion info
		int accountid;
		char name[256]; // only 24 used
		char message[256]; // only 40 used
		int len;
		int i;

		if (sscanf(str, "%d\t%n", &c, &len) < 1)
			return 1;
		str+= len;

		for(i = 0; i < c; i++)
		{
			struct guild_expulsion *e = &g->expulsion[i];
			if (sscanf(str, "%d,%*d,%*d,%*d\t%[^\t]\t%*[^\t]\t%[^\t]\t%n", &accountid, name, message, &len) < 3)
				return 1;

			e->account_id = accountid;
			safestrncpy(e->name, name, sizeof(e->name));
			message[strlen(message)-1] = 0; // remove '#'
			safestrncpy(e->mes, message, sizeof(e->mes));

			str+= len;
		}
	}

	{// load guild skill info
		int skillid;
		int skilllv;
		int len;
		int i;

		for(i = 0; i < MAX_GUILDSKILL; i++)
		{
			if (sscanf(str, "%d,%d %n", &skillid, &skilllv, &len) < 2)
				break;
			g->skill[i].id = skillid;
			g->skill[i].lv = skilllv;

			str+= len;
		}
		str = strchr(str, '\t');
	}

	return 0;
}

#ifndef TXT_SQL_CONVERT
// ギルド城データの文字列への変換
int inter_guildcastle_tostr(char *str, struct guild_castle *gc)
{
	int len;

	len = sprintf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	              gc->castle_id, gc->guild_id, gc->economy, gc->defense, gc->triggerE,
	              gc->triggerD, gc->nextTime, gc->payTime, gc->createTime, gc->visibleC,
	              gc->guardian[0].visible, gc->guardian[1].visible, gc->guardian[2].visible, gc->guardian[3].visible,
	              gc->guardian[4].visible, gc->guardian[5].visible, gc->guardian[6].visible, gc->guardian[7].visible);

	return 0;
}
#endif ///TXT_SQL_CONVERT

// ギルド城データの文字列からの変換
int inter_guildcastle_fromstr(char *str, struct guild_castle *gc)
{
	int castleid, guildid, economy, defense, triggerE, triggerD, nextTime, payTime, createTime, visibleC;
	int guardian[8];
	int dummy;

	memset(gc, 0, sizeof(struct guild_castle));
	// structure of guild castle with the guardian hp included
	if( sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		&castleid, &guildid, &economy, &defense, &triggerE, &triggerD, &nextTime, &payTime, &createTime, &visibleC,
		&guardian[0], &guardian[1], &guardian[2], &guardian[3], &guardian[4], &guardian[5], &guardian[6], &guardian[7],
		&dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy) != 26 )
	// structure of guild castle without the hps (current one)
	if( sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		&castleid, &guildid, &economy, &defense, &triggerE, &triggerD, &nextTime, &payTime, &createTime, &visibleC,
		&guardian[0], &guardian[1], &guardian[2], &guardian[3], &guardian[4], &guardian[5], &guardian[6], &guardian[7]) != 18 )
		return 1;

	gc->castle_id = castleid;
	gc->guild_id = guildid;
	gc->economy = economy;
	gc->defense = defense;
	gc->triggerE = triggerE;
	gc->triggerD = triggerD;
	gc->nextTime = nextTime;
	gc->payTime = payTime;
	gc->createTime = createTime;
	gc->visibleC = visibleC;
	gc->guardian[0].visible = guardian[0];
	gc->guardian[1].visible = guardian[1];
	gc->guardian[2].visible = guardian[2];
	gc->guardian[3].visible = guardian[3];
	gc->guardian[4].visible = guardian[4];
	gc->guardian[5].visible = guardian[5];
	gc->guardian[6].visible = guardian[6];
	gc->guardian[7].visible = guardian[7];

	return 0;
}

#ifndef TXT_SQL_CONVERT
// ギルド関連データベース読み込み
int inter_guild_readdb(void)
{
	int i;
	FILE *fp;
	char line[1024];
	char path[1024];

	sprintf(path, "%s%s", db_path, "/exp_guild.txt");
	fp = fopen(path, "r");
	if (fp == NULL) {
		ShowError("can't read db/exp_guild.txt\n");
		return 1;
	}
	i = 0;
	while(fgets(line, sizeof(line), fp) && i < 100)
	{
		if (line[0] == '/' && line[1] == '/')
			continue;
		guild_exp[i] = (unsigned int)atof(line);
		i++;
	}
	fclose(fp);

	return 0;
}

// ギルドデータの読み込み
int inter_guild_init()
{
	char line[16384];
	struct guild *g;
	struct guild_castle *gc;
	FILE *fp;
	int i, j, c = 0;

	inter_guild_readdb();

	guild_db = idb_alloc(DB_OPT_RELEASE_DATA);
	castle_db = idb_alloc(DB_OPT_RELEASE_DATA);

	if ((fp = fopen(guild_txt,"r")) == NULL)
		return 1;
	while(fgets(line, sizeof(line), fp))
	{
		j = 0;
		if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && guild_newid <= i) {
			guild_newid = i;
			continue;
		}

		g = (struct guild *) aCalloc(sizeof(struct guild), 1);
		if(g == NULL){
			ShowFatalError("int_guild: out of memory!\n");
			exit(EXIT_FAILURE);
		}
//		memset(g, 0, sizeof(struct guild)); not needed...
		if (inter_guild_fromstr(line, g) == 0 && g->guild_id > 0) {
			if (g->guild_id >= guild_newid)
				guild_newid = g->guild_id + 1;
			idb_put(guild_db, g->guild_id, g);
			guild_check_empty(g);
			guild_calcinfo(g);
		} else {
			ShowError("int_guild: broken data [%s] line %d\n", guild_txt, c);
			aFree(g);
		}
		c++;
	}
	fclose(fp);

	c = 0;//カウンタ初期化

	if ((fp = fopen(castle_txt, "r")) == NULL) {
		return 1;
	}

	while(fgets(line, sizeof(line), fp))
	{
		gc = (struct guild_castle *) aCalloc(sizeof(struct guild_castle), 1);
		if(gc == NULL){
			ShowFatalError("int_guild: out of memory!\n");
			exit(EXIT_FAILURE);
		}
//		memset(gc, 0, sizeof(struct guild_castle)); No need...
		if (inter_guildcastle_fromstr(line, gc) == 0) {
			idb_put(castle_db, gc->castle_id, gc);
		} else {
			ShowError("int_guild: broken data [%s] line %d\n", castle_txt, c);
			aFree(gc);
		}
		c++;
	}

	fclose(fp);

	if (!c) {
		ShowStatus(" %s - making Default Data...\n", castle_txt);
		//デフォルトデータを作成
		for(i = 0; i < MAX_GUILDCASTLE; i++) {
			gc = (struct guild_castle *) aCalloc(sizeof(struct guild_castle), 1);
			if (gc == NULL) {
				ShowFatalError("int_guild: out of memory!\n");
				exit(EXIT_FAILURE);
			}
			gc->castle_id = i;
			idb_put(castle_db, gc->castle_id, gc);
		}
		ShowStatus(" %s - making done\n",castle_txt);
		return 0;
	}

	return 0;
}

void inter_guild_final()
{
	castle_db->destroy(castle_db, NULL);
	guild_db->destroy(guild_db, NULL);
	return;
}

struct guild *inter_guild_search(int guild_id)
{
	return (struct guild*)idb_get(guild_db, guild_id);
}

// ギルドデータのセーブ
int inter_guild_save()
{
	FILE *fp;
	int lock;
	DBIterator* iter;
	struct guild* g;
	struct guild_castle* gc;

	// save guild data
	if ((fp = lock_fopen(guild_txt, &lock)) == NULL) {
		ShowError("int_guild: can't write [%s] !!! data is lost !!!\n", guild_txt);
		return 1;
	}

	iter = guild_db->iterator(guild_db);
	for( g = (struct guild*)iter->first(iter,NULL); iter->exists(iter); g = (struct guild*)iter->next(iter,NULL) )
	{
		char line[16384];
		inter_guild_tostr(line, g);
		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

//	fprintf(fp, "%d\t%%newid%%\n", guild_newid);
	lock_fclose(fp, guild_txt, &lock);

	// save castle data
	if ((fp = lock_fopen(castle_txt,&lock)) == NULL) {
		ShowError("int_guild: can't write [%s] !!! data is lost !!!\n", castle_txt);
		return 1;
	}

	iter = castle_db->iterator(castle_db);
	for( gc = (struct guild_castle*)iter->first(iter,NULL); iter->exists(iter); gc = (struct guild_castle*)iter->next(iter,NULL) )
	{
		char line[16384];
		inter_guildcastle_tostr(line, gc);
		fprintf(fp, "%s\n", line);
	}
	iter->destroy(iter);

	lock_fclose(fp, castle_txt, &lock);

	return 0;
}

// ギルド名検索
struct guild* search_guildname(char *str)
{
	DBIterator* iter;
	struct guild* g;

	iter = guild_db->iterator(guild_db);
	for( g = (struct guild*)iter->first(iter,NULL); iter->exists(iter); g = (struct guild*)iter->next(iter,NULL) )
	{
		if (strcmpi(g->name, str) == 0)
			break;
	}
	iter->destroy(iter);

	return g;
}

// ギルドが空かどうかチェック
static bool guild_check_empty(struct guild *g)
{
	int i;
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id > 0 );
	if( i < g->max_member)
		return false; // not empty

	// 誰もいないので解散
	guild_db->foreach(guild_db, guild_break_sub, g->guild_id);
	inter_guild_storage_delete(g->guild_id);
	mapif_guild_broken(g->guild_id, 0);
	idb_remove(guild_db, g->guild_id);
	return true;
}

unsigned int guild_nextexp(int level)
{
	if (level == 0)
		return 1;
	if (level > 0 && level < 100)
		return guild_exp[level-1];

	return 0;
}

// ギルドスキルがあるか確認
int guild_checkskill(struct guild *g, int id)
{
	int idx = id - GD_SKILLBASE;


	if(idx < 0 || idx >= MAX_GUILDSKILL)

		return 0;

	return g->skill[idx].lv;
}

// ギルドの情報の再計算
int guild_calcinfo(struct guild *g)
{
	int i, c;
	unsigned int nextexp;
	struct guild before = *g;

	// スキルIDの設定
	for(i = 0; i < MAX_GUILDSKILL; i++)
		g->skill[i].id=i+GD_SKILLBASE;

	// ギルドレベル
	if (g->guild_lv <= 0)
		g->guild_lv = 1;
	nextexp = guild_nextexp(g->guild_lv);
	if (nextexp > 0) {
		while(g->exp >= nextexp && nextexp > 0) {	//fixed guild exp overflow [Kevin]
			g->exp -= nextexp;
			g->guild_lv++;
			g->skill_point++;
			nextexp = guild_nextexp(g->guild_lv);
		}
	}

	// ギルドの次の経験値
	g->next_exp = guild_nextexp(g->guild_lv);

	// メンバ上限（ギルド拡張適用）
	g->max_member = 16 + guild_checkskill(g, GD_EXTENSION) * 6; //Guild Extention skill - currently adds 6 to max per skill lv.
	if(g->max_member > MAX_GUILD)
	{	
		ShowError("Guild %d:%s has capacity for too many guild members (%d), max supported is %d\n", g->guild_id, g->name, g->max_member, MAX_GUILD);
		g->max_member = MAX_GUILD;
	}

	// 平均レベルとオンライン人数
	g->average_lv = 0;
	g->connect_member = 0;
	c = 0;
	for(i = 0; i < g->max_member; i++) {
		if (g->member[i].account_id > 0) {
			g->average_lv += g->member[i].lv;
			c++;
			if (g->member[i].online > 0)
				g->connect_member++;
		}
	}
	if(c) g->average_lv /= c;

	// 全データを送る必要がありそう
	if (g->max_member != before.max_member ||
		g->guild_lv != before.guild_lv ||
		g->skill_point != before.skill_point) {
		mapif_guild_info(-1, g);
		return 1;
	}

	return 0;
}

//-------------------------------------------------------------------
// map serverへの通信

// ギルド作成可否
int mapif_guild_created(int fd, int account_id, struct guild *g)
{
	WFIFOHEAD(fd, 10);
	WFIFOW(fd,0) = 0x3830;
	WFIFOL(fd,2) = account_id;
	if (g != NULL) {
		WFIFOL(fd,6) = g->guild_id;
		ShowInfo("Created Guild (%d %s)\n", g->guild_id, g->name);
	}else{
		WFIFOL(fd,6) = 0;
	}
	WFIFOSET(fd,10);
	return 0;
}

// ギルド情報見つからず
int mapif_guild_noinfo(int fd, int guild_id)
{
	WFIFOHEAD(fd, 8);
	WFIFOW(fd,0) = 0x3831;
	WFIFOW(fd,2) = 8;
	WFIFOL(fd,4) = guild_id;
	WFIFOSET(fd,8);
	ShowNotice("int_guild: info not found %d\n", guild_id);

	return 0;
}

// ギルド情報まとめ送り
int mapif_guild_info(int fd, struct guild *g)
{
	unsigned char buf[8+sizeof(struct guild)];

	WBUFW(buf,0) = 0x3831;
	memcpy(buf + 4, g, sizeof(struct guild));
	WBUFW(buf,2) = 4 + sizeof(struct guild);
	if (fd < 0)
		mapif_sendall(buf, WBUFW(buf,2));
	else
		mapif_send(fd, buf, WBUFW(buf,2));

	return 0;
}

// メンバ追加可否
int mapif_guild_memberadded(int fd, int guild_id, int account_id, int char_id, int flag)
{
	WFIFOHEAD(fd, 15);
	WFIFOW(fd,0) = 0x3832;
	WFIFOL(fd,2) = guild_id;
	WFIFOL(fd,6) = account_id;
	WFIFOL(fd,10) = char_id;
	WFIFOB(fd,14) = flag;
	WFIFOSET(fd, 15);

	return 0;
}

// 脱退/追放通知
int mapif_guild_leaved(int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes)
{
	unsigned char buf[79];

	WBUFW(buf, 0) = 0x3834;
	WBUFL(buf, 2) = guild_id;
	WBUFL(buf, 6) = account_id;
	WBUFL(buf,10) = char_id;
	WBUFB(buf,14) = flag;
	memcpy(WBUFP(buf,15), mes, 40);
	memcpy(WBUFP(buf,55), name, NAME_LENGTH);
	mapif_sendall(buf, 55+NAME_LENGTH);
//	mapif_sendall(buf, 79);
	ShowInfo("Character left guild (Guild %d, %d - %s: %s)\n", guild_id, account_id, name, mes);

	return 0;
}

// オンライン状態とLv更新通知
int mapif_guild_memberinfoshort(struct guild *g, int idx)
{
	unsigned char buf[19];

	WBUFW(buf, 0) = 0x3835;
	WBUFL(buf, 2) = g->guild_id;
	WBUFL(buf, 6) = g->member[idx].account_id;
	WBUFL(buf,10) = g->member[idx].char_id;
	WBUFB(buf,14) = (unsigned char)g->member[idx].online;
	WBUFW(buf,15) = g->member[idx].lv;
	WBUFW(buf,17) = g->member[idx].class_;
	mapif_sendall(buf, 19);
	return 0;
}

// 解散通知
int mapif_guild_broken(int guild_id, int flag)
{
	unsigned char buf[7];

	WBUFW(buf,0) = 0x3836;
	WBUFL(buf,2) = guild_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf, 7);
	ShowInfo("Guild Break (%d)\n", guild_id);

	return 0;
}

// ギルド内発言
int mapif_guild_message(int guild_id, int account_id, char *mes, int len, int sfd)
{
	unsigned char buf[2048];

	WBUFW(buf,0) = 0x3837;
	WBUFW(buf,2) = len + 12;
	WBUFL(buf,4) = guild_id;
	WBUFL(buf,8) = account_id;
	memcpy(WBUFP(buf,12), mes, len);
	mapif_sendallwos(sfd, buf, len + 12);

	return 0;
}

// ギルド基本情報変更通知
int mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len)
{
	unsigned char buf[2048];

	WBUFW(buf,0) = 0x3839;
	WBUFW(buf,2) = len+10;
	WBUFL(buf,4) = guild_id;
	WBUFW(buf,8) = type;
	memcpy(WBUFP(buf,10),data,len);
	mapif_sendall(buf,len+10);
	return 0;
}

// ギルドメンバ情報変更通知
int mapif_guild_memberinfochanged(int guild_id, int account_id, int char_id, int type, const void *data, int len)
{
	unsigned char buf[4096];

	WBUFW(buf, 0) = 0x383a;
	WBUFW(buf, 2) = len + 18;
	WBUFL(buf, 4) = guild_id;
	WBUFL(buf, 8) = account_id;
	WBUFL(buf,12) = char_id;
	WBUFW(buf,16) = type;
	memcpy(WBUFP(buf,18), data, len);
	mapif_sendall(buf,len+18);

	return 0;
}

// ギルドスキルアップ通知
int mapif_guild_skillupack(int guild_id, int skill_num, int account_id)
{
	unsigned char buf[14];

	WBUFW(buf, 0) = 0x383c;
	WBUFL(buf, 2) = guild_id;
	WBUFL(buf, 6) = skill_num;
	WBUFL(buf,10) = account_id;
	mapif_sendall(buf, 14);

	return 0;
}

// ギルド同盟/敵対通知
int mapif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2)
{
	unsigned char buf[67];

	WBUFW(buf, 0) = 0x383d;
	WBUFL(buf, 2) = guild_id1;
	WBUFL(buf, 6) = guild_id2;
	WBUFL(buf,10) = account_id1;
	WBUFL(buf,14) = account_id2;
	WBUFB(buf,18) = flag;
	memcpy(WBUFP(buf,19), name1, NAME_LENGTH);
	memcpy(WBUFP(buf,19+NAME_LENGTH), name2, NAME_LENGTH);
	mapif_sendall(buf,19+2*NAME_LENGTH);
/*
	memcpy(WBUFP(buf,43), name2, NAME_LENGTH);
	mapif_sendall(buf, 67);
*/
	return 0;
}

// ギルド役職変更通知
int mapif_guild_position(struct guild *g, int idx)
{
	unsigned char buf[2048];

	WBUFW(buf,0) = 0x383b;
	WBUFW(buf,2) = sizeof(struct guild_position) + 12;
	WBUFL(buf,4) = g->guild_id;
	WBUFL(buf,8) = idx;
	memcpy(WBUFP(buf,12), &g->position[idx], sizeof(struct guild_position));
	mapif_sendall(buf, WBUFW(buf,2));

	return 0;
}

// ギルド告知変更通知
int mapif_guild_notice(struct guild *g)
{
	unsigned char buf[186];

	WBUFW(buf,0) = 0x383e;
	WBUFL(buf,2) = g->guild_id;
	memcpy(WBUFP(buf,6), g->mes1, 60);
	memcpy(WBUFP(buf,66), g->mes2, 120);
	mapif_sendall(buf, 186);

	return 0;
}

// ギルドエンブレム変更通知
int mapif_guild_emblem(struct guild *g)
{
	unsigned char buf[2048];

	WBUFW(buf,0) = 0x383f;
	WBUFW(buf,2) = g->emblem_len + 12;
	WBUFL(buf,4) = g->guild_id;
	WBUFL(buf,8) = g->emblem_id;
	memcpy(WBUFP(buf,12), g->emblem_data, g->emblem_len);
	mapif_sendall(buf, WBUFW(buf,2));

	return 0;
}

int mapif_guild_master_changed(struct guild *g, int aid, int cid)
{
	unsigned char buf[14];
	WBUFW(buf,0)=0x3843;
	WBUFL(buf,2)=g->guild_id;
	WBUFL(buf,6)=aid;
	WBUFL(buf,10)=cid;
	mapif_sendall(buf,14);
	return 0;
}

int mapif_guild_castle_dataload(int castle_id, int index, int value)
{
	unsigned char buf[9];

	WBUFW(buf,0) = 0x3840;
	WBUFW(buf,2) = castle_id;
	WBUFB(buf,4) = index;
	WBUFL(buf,5) = value;
	mapif_sendall(buf,9);

	return 0;
}

int mapif_guild_castle_datasave(int castle_id, int index, int value)
{
	unsigned char buf[9];

	WBUFW(buf,0) = 0x3841;
	WBUFW(buf,2) = castle_id;
	WBUFB(buf,4) = index;
	WBUFL(buf,5) = value;
	mapif_sendall(buf,9);

	return 0;
}

int mapif_guild_castle_alldataload(int fd)
{
	DBIterator* iter;
	struct guild_castle* gc;
	int len = 4;

	WFIFOHEAD(fd, 4 + MAX_GUILDCASTLE*sizeof(struct guild_castle));
	WFIFOW(fd,0) = 0x3842;
	iter = castle_db->iterator(castle_db);
	for( gc = (struct guild_castle*)iter->first(iter,NULL); iter->exists(iter); gc = (struct guild_castle*)iter->next(iter,NULL) )
	{
		memcpy(WFIFOP(fd,len), gc, sizeof(struct guild_castle));
		len += sizeof(struct guild_castle);
	}
	iter->destroy(iter);
	WFIFOW(fd,2) = len;
	WFIFOSET(fd, len);

	return 0;
}

//-------------------------------------------------------------------
// map serverからの通信

// ギルド作成要求
int mapif_parse_CreateGuild(int fd, int account_id, char *name, struct guild_member *master)
{
	struct guild *g;
	int i;

	for(i = 0; i < NAME_LENGTH && name[i]; i++) {
		if (!(name[i] & 0xe0) || name[i] == 0x7f) {
			ShowInfo("Create Guild: illegal guild name [%s]\n", name);
			mapif_guild_created(fd, account_id, NULL);
			return 0;
		}
	}

	if ((g = search_guildname(name)) != NULL) {
		ShowInfo("Create Guild: same name guild exists [%s]\n", name);
		mapif_guild_created(fd, account_id, NULL);
		return 0;
	}

	// Check Authorised letters/symbols in the name of the character
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) == NULL) {
				mapif_guild_created(fd,account_id,NULL);
				return 0;
			}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
			if (strchr(char_name_letters, name[i]) != NULL) {
				mapif_guild_created(fd,account_id,NULL);
				return 0;
			}
	}

	g = (struct guild *) aCalloc(sizeof(struct guild), 1);
	if (g == NULL) {
		ShowFatalError("int_guild: CreateGuild: out of memory !\n");
		mapif_guild_created(fd, account_id, NULL);
		exit(EXIT_FAILURE);
	}
//	memset(g, 0, sizeof(struct guild)); Meh...
	g->guild_id = guild_newid++;
	memcpy(g->name, name, NAME_LENGTH);
	memcpy(g->master, master->name, NAME_LENGTH);
	memcpy(&g->member[0], master, sizeof(struct guild_member));

	g->position[0].mode = 0x11;
	strcpy(g->position[                  0].name, "GuildMaster");
	strcpy(g->position[MAX_GUILDPOSITION-1].name, "Newbie");
	for(i = 1; i < MAX_GUILDPOSITION-1; i++)
		sprintf(g->position[i].name, "Position %d", i + 1);

	// ここでギルド情報計算が必要と思われる
	g->max_member = 16;
	g->average_lv = master->lv;
	g->connect_member = 1;
	for(i = 0; i < MAX_GUILDSKILL; i++)
		g->skill[i].id=i + GD_SKILLBASE;

	idb_put(guild_db, g->guild_id, g);

	mapif_guild_created(fd, account_id, g);
	mapif_guild_info(fd, g);

	if(log_inter)
		inter_log("guild %s (id=%d) created by master %s (id=%d)\n",
			name, g->guild_id, master->name, master->account_id);

	return 0;
}

// ギルド情報要求
int mapif_parse_GuildInfo(int fd, int guild_id)
{
	struct guild *g;

	g = (struct guild*)idb_get(guild_db, guild_id);
	if (g != NULL){
		guild_calcinfo(g);
		mapif_guild_info(fd, g);
	} else
		mapif_guild_noinfo(fd, guild_id);

	return 0;
}

// ギルドメンバ追加要求
int mapif_parse_GuildAddMember(int fd, int guild_id, struct guild_member *m)
{
	struct guild *g;
	int i;

	g = (struct guild*)idb_get(guild_db, guild_id);
	if (g == NULL) {
		mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 1);
		return 0;
	}

	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == 0 );
	if( i < g->max_member )
	{
		memcpy(&g->member[i], m, sizeof(struct guild_member));
		mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 0);
		guild_calcinfo(g);
		mapif_guild_info(-1, g);
	}
	else
		mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 1);

	return 0;
}

// Delete member from guild
int mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes)
{
	int i, j;

	struct guild* g = (struct guild*)idb_get(guild_db, guild_id);
	if( g == NULL )
	{
		//TODO
		return 0;
	}

	// Find the member
	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == account_id && g->member[i].char_id == char_id );
	if( i == g->max_member )
	{
		//TODO
		return 0;
	}

	if( flag )
	{	// 追放の場合追放リストに入れる
		ARR_FIND( 0, MAX_GUILDEXPULSION, j, g->expulsion[j].account_id == 0 );
		if (j == MAX_GUILDEXPULSION)
		{	// 一杯なので古いのを消す
			for(j = 0; j < MAX_GUILDEXPULSION - 1; j++)
				g->expulsion[j] = g->expulsion[j+1];
			j = MAX_GUILDEXPULSION - 1;
		}
		// Save the expulsion entry
		g->expulsion[j].account_id = account_id;
		safestrncpy(g->expulsion[j].name, g->member[i].name, NAME_LENGTH);
		safestrncpy(g->expulsion[j].mes, mes, 40);
	}

	mapif_guild_leaved(guild_id, account_id, char_id, flag, g->member[i].name, mes);

	memset(&g->member[i], 0, sizeof(struct guild_member));

	if (guild_check_empty(g) == 0)
		mapif_guild_info(-1,g);// まだ人がいるのでデータ送信

	return 0;
}

// オンライン/Lv更新
int mapif_parse_GuildChangeMemberInfoShort(int fd, int guild_id, int account_id, int char_id, int online, int lv, int class_)
{
	struct guild *g;
	int i, sum, c;

	g = (struct guild*)idb_get(guild_db, guild_id);
	if (g == NULL)
		return 0;

	ARR_FIND( 0, g->max_member, i, g->member[i].account_id == account_id && g->member[i].char_id == char_id );
	if( i < g->max_member )
	{
			g->member[i].online = online;
			g->member[i].lv = lv;
			g->member[i].class_ = class_;
			mapif_guild_memberinfoshort(g, i);
	}

	g->average_lv = 0;
	g->connect_member = 0;
	c = 0; // member count
	sum = 0; // total sum of base levels

	for(i = 0; i < g->max_member; i++)
	{
		if( g->member[i].account_id > 0 )
		{
			sum += g->member[i].lv;
			c++;
		}
		if( g->member[i].online )
			g->connect_member++;
	}

	if( c ) // this check should always succeed...
		g->average_lv = sum / c;

	//FIXME: how about sending a mapif_guild_info() update to the mapserver? [ultramage] 

	return 0;
}

// ギルド解散処理用（同盟/敵対を解除）
int guild_break_sub(DBKey key, void *data, va_list ap)
{
	struct guild *g = (struct guild *)data;
	int guild_id = va_arg(ap, int);
	int i;

	for(i = 0; i < MAX_GUILDALLIANCE; i++) {
		if (g->alliance[i].guild_id == guild_id)
			g->alliance[i].guild_id = 0;
	}
	return 0;
}

// ギルド解散要求
int mapif_parse_BreakGuild(int fd, int guild_id)
{
	struct guild *g;

	g = (struct guild*)idb_get(guild_db, guild_id);
	if(g == NULL)
		return 0;

	guild_db->foreach(guild_db, guild_break_sub, guild_id);
	inter_guild_storage_delete(guild_id);
	mapif_guild_broken(guild_id, 0);

	if(log_inter)
		inter_log("guild %s (id=%d) broken\n", g->name, guild_id);

	idb_remove(guild_db, guild_id);
	return 0;
}

// ギルドメッセージ送信
int mapif_parse_GuildMessage(int fd, int guild_id, int account_id, char *mes, int len)
{
	return mapif_guild_message(guild_id, account_id, mes, len, fd);
}

// ギルド基本データ変更要求
int mapif_parse_GuildBasicInfoChange(int fd, int guild_id, int type, const char *data, int len)
{
	struct guild *g;
	short dw = *((short *)data);

	g = (struct guild*)idb_get(guild_db, guild_id);
	if (g == NULL)
		return 0;

	switch(type) {
	case GBI_GUILDLV:
		if (dw > 0 && g->guild_lv + dw <= 50) {
			g->guild_lv+=dw;
			g->skill_point+=dw;
		} else if (dw < 0 && g->guild_lv + dw >= 1)
			g->guild_lv += dw;
		mapif_guild_info(-1, g);
		return 0;
	default:
		ShowError("int_guild: GuildBasicInfoChange: Unknown type %d\n", type);
		break;
	}
	mapif_guild_basicinfochanged(guild_id, type, data, len);

	return 0;
}

// ギルドメンバデータ変更要求
int mapif_parse_GuildMemberInfoChange(int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len)
{
	int i;
	struct guild *g;

	g = (struct guild*)idb_get(guild_db, guild_id);
	if(g == NULL)
		return 0;

	for(i = 0; i < g->max_member; i++)
		if (g->member[i].account_id == account_id && g->member[i].char_id == char_id)
			break;
	if (i == g->max_member) {
		ShowWarning("int_guild: GuildMemberChange: Not found %d,%d in %d[%s]\n", account_id, char_id, guild_id, g->name);
		return 0;
	}
	switch(type) {
	case GMI_POSITION:	// 役職
		g->member[i].position = *((int *)data);
		break;
	case GMI_EXP:	// EXP
	{
		unsigned int exp, old_exp=g->member[i].exp;
		g->member[i].exp=*((unsigned int *)data);
		if (g->member[i].exp > old_exp)
		{
			exp = g->member[i].exp - old_exp;
			if (guild_exp_rate != 100)
				exp = exp*guild_exp_rate/100;
			if (exp > UINT_MAX - g->exp)
				g->exp = UINT_MAX;
			else
				g->exp+=exp;
			guild_calcinfo(g);
			mapif_guild_basicinfochanged(guild_id,GBI_EXP,&g->exp,4);
		}
		break;
	}
	case GMI_HAIR:
	{
		g->member[i].hair=*((int *)data);
		mapif_guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
		break;
	}
	case GMI_HAIR_COLOR:
	{
		g->member[i].hair_color=*((int *)data);
		mapif_guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
		break;
	}
	case GMI_GENDER:
	{
		g->member[i].gender=*((int *)data);
		mapif_guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
		break;
	}
	case GMI_CLASS:
	{
		g->member[i].class_=*((int *)data);
		mapif_guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
		break;
	}
	case GMI_LEVEL:
	{
		g->member[i].lv=*((int *)data);
		mapif_guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
		break;
	}

	default:
		ShowError("int_guild: GuildMemberInfoChange: Unknown type %d\n", type);
		break;
	}
	mapif_guild_memberinfochanged(guild_id, account_id, char_id, type, data, len);

	return 0;
}

int inter_guild_sex_changed(int guild_id,int account_id,int char_id, int gender)
{
	return mapif_parse_GuildMemberInfoChange(0, guild_id, account_id, char_id, GMI_GENDER, (const char*)&gender, sizeof(gender));
}

// ギルド役職名変更要求
int mapif_parse_GuildPosition(int fd, int guild_id, int idx, struct guild_position *p)
{
	struct guild *g = (struct guild*)idb_get(guild_db, guild_id);

	if (g == NULL || idx < 0 || idx >= MAX_GUILDPOSITION) {
		return 0;
	}
	memcpy(&g->position[idx], p, sizeof(struct guild_position));
	mapif_guild_position(g, idx);
	ShowInfo("int_guild: position [%d] changed\n", idx);

	return 0;
}

// ギルドスキルアップ要求
int mapif_parse_GuildSkillUp(int fd, int guild_id, int skill_num, int account_id, int max)
{
	struct guild *g = (struct guild*)idb_get(guild_db, guild_id);
	int idx = skill_num - GD_SKILLBASE;

	if (g == NULL || idx < 0 || idx >= MAX_GUILDSKILL)
		return 0;

	if (g->skill_point > 0 && g->skill[idx].id > 0 && g->skill[idx].lv < max) {
		g->skill[idx].lv++;
		g->skill_point--;
		if (guild_calcinfo(g) == 0)
			mapif_guild_info(-1, g);
		mapif_guild_skillupack(guild_id, skill_num, account_id);
	}

	return 0;
}

//Manual deletion of an alliance when partnering guild does not exists. [Skotlex]
static int mapif_parse_GuildDeleteAlliance(struct guild *g, int guild_id, int account_id1, int account_id2, int flag)
{
	int i;
	char name[NAME_LENGTH];
	for(i=0;i<MAX_GUILDALLIANCE;i++)
		if(g->alliance[i].guild_id == guild_id)
		{
			strcpy(name, g->alliance[i].name);
			g->alliance[i].guild_id=0;
			break;
		}
	if (i == MAX_GUILDALLIANCE)
		return -1;
	
	mapif_guild_alliance(g->guild_id,guild_id,account_id1,account_id2,flag,g->name,name);
	return 0;
}

// ギルド同盟要求
int mapif_parse_GuildAlliance(int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag)
{
	struct guild *g[2];
	int j, i;

	g[0] = (struct guild*)idb_get(guild_db, guild_id1);
	g[1] = (struct guild*)idb_get(guild_db, guild_id2);

	if(g[0] && g[1]==NULL && (flag&0x8)) //Requested to remove an alliance with a not found guild.
		return mapif_parse_GuildDeleteAlliance(g[0], guild_id2,
			account_id1, account_id2, flag); //Try to do a manual removal of said guild.

	if (g[0] == NULL || g[1] == NULL)
		return 0;

	if (!(flag & 0x8)) {
		for(i = 0; i < 2 - (flag & 1); i++) {
			for(j = 0; j < MAX_GUILDALLIANCE; j++)
				if (g[i]->alliance[j].guild_id == 0) {
					g[i]->alliance[j].guild_id = g[1-i]->guild_id;
					memcpy(g[i]->alliance[j].name, g[1-i]->name, NAME_LENGTH);
					g[i]->alliance[j].opposition = flag & 1;
					break;
				}
		}
	} else {	// 関係解消
		for(i = 0; i < 2 - (flag & 1); i++) {
			for(j = 0; j < MAX_GUILDALLIANCE; j++)
				if (g[i]->alliance[j].guild_id == g[1-i]->guild_id && g[i]->alliance[j].opposition == (flag & 1)) {
					g[i]->alliance[j].guild_id = 0;
					break;
				}
		}
	}
	mapif_guild_alliance(guild_id1, guild_id2, account_id1, account_id2, flag, g[0]->name, g[1]->name);

	return 0;
}

// ギルド告知変更要求
int mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2)
{
	struct guild *g;

	g = (struct guild*)idb_get(guild_db, guild_id);
	if (g == NULL)
		return 0;
	memcpy(g->mes1, mes1, 60);
	memcpy(g->mes2, mes2, 120);

	return mapif_guild_notice(g);
}

// ギルドエンブレム変更要求
int mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data)
{
	struct guild *g;

	g = (struct guild*)idb_get(guild_db, guild_id);
	if (g == NULL)
		return 0;
	memcpy(g->emblem_data, data, len);
	g->emblem_len = len;
	g->emblem_id++;

	return mapif_guild_emblem(g);
}

int mapif_parse_GuildCastleDataLoad(int fd, int castle_id, int index)
{
	struct guild_castle *gc = (struct guild_castle*)idb_get(castle_db, castle_id);

	if (gc == NULL) {
		return mapif_guild_castle_dataload(castle_id, 0, 0);
	}
	switch(index) {
	case 1: return mapif_guild_castle_dataload(gc->castle_id, index, gc->guild_id);
	case 2: return mapif_guild_castle_dataload(gc->castle_id, index, gc->economy);
	case 3: return mapif_guild_castle_dataload(gc->castle_id, index, gc->defense);
	case 4: return mapif_guild_castle_dataload(gc->castle_id, index, gc->triggerE);
	case 5: return mapif_guild_castle_dataload(gc->castle_id, index, gc->triggerD);
	case 6: return mapif_guild_castle_dataload(gc->castle_id, index, gc->nextTime);
	case 7: return mapif_guild_castle_dataload(gc->castle_id, index, gc->payTime);
	case 8: return mapif_guild_castle_dataload(gc->castle_id, index, gc->createTime);
	case 9: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleC);
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		return mapif_guild_castle_dataload(gc->castle_id, index, gc->guardian[index-10].visible);
	default:
		ShowError("mapif_parse_GuildCastleDataLoad ERROR!! (Not found index=%d)\n", index);
		return 0;
	}

	return 0;
}

int mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value)
{
	struct guild_castle *gc = (struct guild_castle*)idb_get(castle_db, castle_id);

	if (gc == NULL)
		return mapif_guild_castle_datasave(castle_id, index, value);

	switch(index) {
	case 1:
		if (gc->guild_id != value) {
			int gid = (value) ? value : gc->guild_id;
			struct guild *g = (struct guild*)idb_get(guild_db, gid);
			if(log_inter)
				inter_log("guild %s (id=%d) %s castle id=%d\n",
					(g) ? g->name : "??", gid, (value) ? "occupy" : "abandon", castle_id);
		}
		gc->guild_id = value;
		if(gc->guild_id == 0) {
			//Delete guardians.
			memset(&gc->guardian, 0, sizeof(gc->guardian));
		}
		break;
	case 2: gc->economy = value; break;
	case 3: gc->defense = value; break;
	case 4: gc->triggerE = value; break;
	case 5: gc->triggerD = value; break;
	case 6: gc->nextTime = value; break;
	case 7: gc->payTime = value; break;
	case 8: gc->createTime = value; break;
	case 9: gc->visibleC = value; break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		gc->guardian[index-10].visible = value; break;
	default:
		ShowError("mapif_parse_GuildCastleDataSave ERROR!! (Not found index=%d)\n", index);
		return 0;
	}

	return mapif_guild_castle_datasave(gc->castle_id, index, value);
}

int mapif_parse_GuildMasterChange(int fd, int guild_id, const char* name, int len)
{
	struct guild *g = (struct guild*)idb_get(guild_db, guild_id);
	struct guild_member gm;
	int pos;

	if(g==NULL || g->guild_id<=0 || len > NAME_LENGTH)
		return 0;
	
	for (pos = 0; pos < g->max_member && strncmp(g->member[pos].name, name, len); pos++);

	if (pos == g->max_member)
		return 0; //Character not found??
	
	memcpy(&gm, &g->member[pos], sizeof (struct guild_member));
	memcpy(&g->member[pos], &g->member[0], sizeof(struct guild_member));
	memcpy(&g->member[0], &gm, sizeof(struct guild_member));

	g->member[pos].position = g->member[0].position;
	g->member[0].position = 0; //Position 0: guild Master.
	safestrncpy(g->master, name, NAME_LENGTH);

	ShowInfo("int_guild: Guildmaster Changed to %s (Guild %d - %s)\n",name, guild_id, g->name);
	return mapif_guild_master_changed(g, g->member[0].account_id, g->member[0].char_id);
}

// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_guild_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)) {
	case 0x3030: mapif_parse_CreateGuild(fd, RFIFOL(fd,4), (char*)RFIFOP(fd,8), (struct guild_member *)RFIFOP(fd,32)); break;
	case 0x3031: mapif_parse_GuildInfo(fd, RFIFOL(fd,2)); break;
	case 0x3032: mapif_parse_GuildAddMember(fd, RFIFOL(fd,4), (struct guild_member *)RFIFOP(fd,8)); break;
	case 0x3033: mapif_parse_GuildMasterChange(fd,RFIFOL(fd,4),(const char*)RFIFOP(fd,8),RFIFOW(fd,2)-8); break;
	case 0x3034: mapif_parse_GuildLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), (const char*)RFIFOP(fd,15)); break;
	case 0x3035: mapif_parse_GuildChangeMemberInfoShort(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), RFIFOW(fd,15), RFIFOW(fd,17)); break;
	case 0x3036: mapif_parse_BreakGuild(fd, RFIFOL(fd,2)); break;
	case 0x3037: mapif_parse_GuildMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), (char*)RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3039: mapif_parse_GuildBasicInfoChange(fd, RFIFOL(fd,4), RFIFOW(fd,8), (const char*)RFIFOP(fd,10), RFIFOW(fd,2)-10); break;
	case 0x303A: mapif_parse_GuildMemberInfoChange(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOL(fd,12), RFIFOW(fd,16), (const char*)RFIFOP(fd,18), RFIFOW(fd,2)-18); break;
	case 0x303B: mapif_parse_GuildPosition(fd, RFIFOL(fd,4), RFIFOL(fd,8), (struct guild_position *)RFIFOP(fd,12)); break;
	case 0x303C: mapif_parse_GuildSkillUp(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14)); break;
	case 0x303D: mapif_parse_GuildAlliance(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), RFIFOB(fd,18)); break;
	case 0x303E: mapif_parse_GuildNotice(fd, RFIFOL(fd,2), (const char*)RFIFOP(fd,6), (const char*)RFIFOP(fd,66)); break;
	case 0x303F: mapif_parse_GuildEmblem(fd, RFIFOW(fd,2)-12, RFIFOL(fd,4), RFIFOL(fd,8), (const char*)RFIFOP(fd,12)); break;
	case 0x3040: mapif_parse_GuildCastleDataLoad(fd, RFIFOW(fd,2), RFIFOB(fd,4)); break;
	case 0x3041: mapif_parse_GuildCastleDataSave(fd, RFIFOW(fd,2), RFIFOB(fd,4), RFIFOL(fd,5)); break;

	default:
		return 0;
	}

	return 1;
}

// processes a mapserver connection event
int inter_guild_mapif_init(int fd)
{
	return mapif_guild_castle_alldataload(fd);
}

// サーバーから脱退要求（キャラ削除用）
int inter_guild_leave(int guild_id, int account_id, int char_id)
{
	return mapif_parse_GuildLeave(-1, guild_id, account_id, char_id, 0, "** Character Deleted **");
}
#endif //TXT_SQL_CONVERT
