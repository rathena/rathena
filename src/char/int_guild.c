// $Id: int_guild.c,v 1.2 2004/09/25 19:36:53 Akitasha Exp $
#include "inter.h"
#include "int_guild.h"
#include "int_storage.h"
#include "mmo.h"
#include "char.h"
#include "socket.h"
#include "db.h"
#include "lock.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char guild_txt[1024] = "save/guild.txt";
char castle_txt[1024] = "save/castle.txt";

static struct dbt *guild_db;
static struct dbt *castle_db;

static int guild_newid = 10000;

static int guild_exp[100];

int mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes);
int mapif_guild_broken(int guild_id, int flag);
int guild_check_empty(struct guild *g);
int guild_calcinfo(struct guild *g);
int mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len);
int mapif_guild_info(int fd, struct guild *g);
int guild_break_sub(void *key, void *data, va_list ap);

// ギルドデータの文字列への変換
int inter_guild_tostr(char *str, struct guild *g) {
	int i, c, len;

	// 基本データ
	len = sprintf(str, "%d\t%s\t%s\t%d,%d,%d,%d,%d\t%s#\t%s#\t",
	              g->guild_id, g->name, g->master,
	              g->guild_lv, g->max_member, g->exp, g->skill_point, g->castle_id,
	              g->mes1, g->mes2);
	// メンバー
	for(i = 0; i < g->max_member; i++) {
		struct guild_member *m = &g->member[i];
		len += sprintf(str + len, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\t%s\t",
		               m->account_id, m->char_id,
		               m->hair, m->hair_color, m->gender,
		               m->class, m->lv, m->exp, m->exp_payper, m->position,
		               ((m->account_id > 0) ? m->name : "-"));
	}
	// 役職
	for(i = 0; i < MAX_GUILDPOSITION; i++) {
		struct guild_position *p = &g->position[i];
		len += sprintf(str + len, "%d,%d\t%s#\t", p->mode, p->exp_mode, p->name);
	}
	// エンブレム
	len += sprintf(str + len, "%d,%d,", g->emblem_len, g->emblem_id);
	for(i = 0; i < g->emblem_len; i++) {
		len += sprintf(str + len, "%02x", (unsigned char)(g->emblem_data[i]));
	}
	len += sprintf(str + len, "$\t");
	// 同盟リスト
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
	// 追放リスト
	c = 0;
	for(i = 0; i < MAX_GUILDEXPLUSION; i++)
		if (g->explusion[i].account_id > 0)
			c++;
	len += sprintf(str + len, "%d\t", c);
	for(i = 0; i < MAX_GUILDEXPLUSION; i++) {
		struct guild_explusion *e = &g->explusion[i];
		if (e->account_id > 0)
			len += sprintf(str + len, "%d,%d,%d,%d\t%s\t%s\t%s#\t",
			               e->account_id, e->rsv1, e->rsv2, e->rsv3,
			               e->name, e->acc, e->mes );
	}
	// ギルドスキル
	for(i = 0; i < MAX_GUILDSKILL; i++) {
		len += sprintf(str + len, "%d,%d ", g->skill[i].id, g->skill[i].lv);
	}
	len += sprintf(str + len, "\t");

	return 0;
}

// ギルドデータの文字列からの変換
int inter_guild_fromstr(char *str, struct guild *g) {
	int i, j, c;
	int tmp_int[16];
	char tmp_str[4][256];
	char tmp_str2[4096];
	char *pstr;

	// 基本データ
	memset(g, 0, sizeof(struct guild));
	if (sscanf(str, "%d\t%[^\t]\t%[^\t]\t%d,%d,%d,%d,%d\t%[^\t]\t%[^\t]\t", &tmp_int[0],
	           tmp_str[0], tmp_str[1],
	           &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5],
	           tmp_str[2], tmp_str[3]) < 8)
		return 1;

	g->guild_id = tmp_int[0];
	g->guild_lv = tmp_int[1];
	g->max_member = tmp_int[2];
	g->exp = tmp_int[3];
	g->skill_point = tmp_int[4];
	g->castle_id = tmp_int[5];
	memcpy(g->name, tmp_str[0], 24);
	memcpy(g->master, tmp_str[1], 24);
	memcpy(g->mes1, tmp_str[2], 60);
	memcpy(g->mes2, tmp_str[3], 120);
	g->mes1[strlen(g->mes1)-1] = 0;
	g->mes2[strlen(g->mes2)-1] = 0;

	for(j = 0; j < 6 && str != NULL; j++)	// 位置スキップ
		str = strchr(str + 1, '\t');
//	printf("GuildBaseInfo OK\n");

	// メンバー
	for(i = 0; i < g->max_member; i++) {
		struct guild_member *m = &g->member[i];
		if (sscanf(str + 1, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\t%[^\t]\t",
		           &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4],
		           &tmp_int[5], &tmp_int[6], &tmp_int[7], &tmp_int[8], &tmp_int[9],
		           tmp_str[0]) < 11)
			return 1;
		m->account_id = tmp_int[0];
		m->char_id = tmp_int[1];
		m->hair = tmp_int[2];
		m->hair_color = tmp_int[3];
		m->gender = tmp_int[4];
		m->class = tmp_int[5];
		m->lv = tmp_int[6];
		m->exp = tmp_int[7];
		m->exp_payper = tmp_int[8];
		m->position = tmp_int[9];
		memcpy(m->name, tmp_str[0], 24);

		for(j = 0; j < 2 && str != NULL; j++)	// 位置スキップ
			str = strchr(str + 1, '\t');
	}
//	printf("GuildMemberInfo OK\n");
	// 役職
	i = 0;
	while (sscanf(str+1, "%d,%d%n", &tmp_int[0], &tmp_int[1], &j) == 2 && str[1+j] == '\t') {
		struct guild_position *p = &g->position[i];
		if (sscanf(str+1, "%d,%d\t%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str[0]) < 3)
			return 1;
		p->mode = tmp_int[0];
		p->exp_mode = tmp_int[1];
		tmp_str[0][strlen(tmp_str[0])-1] = 0;
		memcpy(p->name, tmp_str[0], 24);

		for(j = 0; j < 2 && str != NULL; j++)	// 位置スキップ
			str = strchr(str+1, '\t');
		i++;
	}
//	printf("GuildPositionInfo OK\n");
	// エンブレム
	tmp_int[1] = 0;
	if (sscanf(str + 1, "%d,%d,%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str2)< 3 &&
		sscanf(str + 1, "%d,%[^\t]\t", &tmp_int[0], tmp_str2) < 2)
		return 1;
	g->emblem_len = tmp_int[0];
	g->emblem_id = tmp_int[1];
	for(i = 0, pstr = tmp_str2; i < g->emblem_len; i++, pstr += 2) {
		int c1 = pstr[0], c2 = pstr[1], x1 = 0, x2 = 0;
		if (c1 >= '0' && c1 <= '9') x1 = c1 - '0';
		if (c1 >= 'a' && c1 <= 'f') x1 = c1 - 'a' + 10;
		if (c1 >= 'A' && c1 <= 'F') x1 = c1 - 'A' + 10;
		if (c2 >= '0' && c2 <= '9') x2 = c2 - '0';
		if (c2 >= 'a' && c2 <= 'f') x2 = c2 - 'a' + 10;
		if (c2 >= 'A' && c2 <= 'F') x2 = c2 - 'A' + 10;
		g->emblem_data[i] = (x1<<4) | x2;
	}
//	printf("GuildEmblemInfo OK\n");
	str=strchr(str + 1, '\t');	// 位置スキップ

	// 同盟リスト
	if (sscanf(str + 1, "%d\t", &c) < 1)
		return 1;
	str = strchr(str + 1, '\t');	// 位置スキップ
	for(i = 0; i < c; i++) {
		struct guild_alliance *a = &g->alliance[i];
		if (sscanf(str + 1, "%d,%d\t%[^\t]\t", &tmp_int[0], &tmp_int[1], tmp_str[0]) < 3)
			return 1;
		a->guild_id = tmp_int[0];
		a->opposition = tmp_int[1];
		memcpy(a->name, tmp_str[0], 24);

		for(j = 0; j < 2 && str != NULL; j++)	// 位置スキップ
			str = strchr(str + 1, '\t');
	}
//	printf("GuildAllianceInfo OK\n");
	// 追放リスト
	if (sscanf(str+1, "%d\t", &c) < 1)
		return 1;
	str = strchr(str + 1, '\t');	// 位置スキップ
	for(i = 0; i < c; i++) {
		struct guild_explusion *e = &g->explusion[i];
		if (sscanf(str + 1, "%d,%d,%d,%d\t%[^\t]\t%[^\t]\t%[^\t]\t",
		           &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		           tmp_str[0], tmp_str[1], tmp_str[2]) < 6)
			return 1;
		e->account_id = tmp_int[0];
		e->rsv1 = tmp_int[1];
		e->rsv2 = tmp_int[2];
		e->rsv3 = tmp_int[3];
		memcpy(e->name, tmp_str[0], 24);
		memcpy(e->acc, tmp_str[1], 24);
		tmp_str[2][strlen(tmp_str[2])-1] = 0;
		memcpy(e->mes, tmp_str[2], 40);

		for(j = 0; j < 4 && str != NULL; j++)	// 位置スキップ
			str = strchr(str + 1, '\t');
	}
//	printf("GuildExplusionInfo OK\n");
	// ギルドスキル
	for(i = 0; i < MAX_GUILDSKILL; i++) {
		if (sscanf(str+1,"%d,%d ", &tmp_int[0], &tmp_int[1]) < 2)
			break;
		g->skill[i].id = tmp_int[0];
		g->skill[i].lv = tmp_int[1];
		str = strchr(str + 1, ' ');
	}
	str = strchr(str + 1, '\t');
//	printf("GuildSkillInfo OK\n");

	return 0;
}

// ギルド城データの文字列への変換
int inter_guildcastle_tostr(char *str, struct guild_castle *gc) {
	int len;

	len = sprintf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",	// added Guardian HP [Valaris]
	              gc->castle_id, gc->guild_id, gc->economy, gc->defense, gc->triggerE,
	              gc->triggerD, gc->nextTime, gc->payTime, gc->createTime, gc->visibleC,
	              gc->visibleG0, gc->visibleG1, gc->visibleG2, gc->visibleG3, gc->visibleG4,
	              gc->visibleG5, gc->visibleG6, gc->visibleG7, gc->Ghp0, gc->Ghp1, gc->Ghp2,
	              gc->Ghp3, gc->Ghp4, gc->Ghp5, gc->Ghp6, gc->Ghp7);

	return 0;
}

// ギルド城データの文字列からの変換
int inter_guildcastle_fromstr(char *str, struct guild_castle *gc) {
	int tmp_int[26];

	memset(gc, 0, sizeof(struct guild_castle));
	// new structure of guild castle
	if (sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	           &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
	           &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12], &tmp_int[13],
	           &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18], &tmp_int[19], &tmp_int[20],
	           &tmp_int[21], &tmp_int[22], &tmp_int[23], &tmp_int[24], &tmp_int[25]) == 26) {
		gc->castle_id = tmp_int[0];
		gc->guild_id = tmp_int[1];
		gc->economy = tmp_int[2];
		gc->defense = tmp_int[3];
		gc->triggerE = tmp_int[4];
		gc->triggerD = tmp_int[5];
		gc->nextTime = tmp_int[6];
		gc->payTime = tmp_int[7];
		gc->createTime = tmp_int[8];
		gc->visibleC = tmp_int[9];
		gc->visibleG0 = tmp_int[10];
		gc->visibleG1 = tmp_int[11];
		gc->visibleG2 = tmp_int[12];
		gc->visibleG3 = tmp_int[13];
		gc->visibleG4 = tmp_int[14];
		gc->visibleG5 = tmp_int[15];
		gc->visibleG6 = tmp_int[16];
		gc->visibleG7 = tmp_int[17];
		gc->Ghp0 = tmp_int[18];
		gc->Ghp1 = tmp_int[19];
		gc->Ghp2 = tmp_int[20];
		gc->Ghp3 = tmp_int[21];
		gc->Ghp4 = tmp_int[22];
		gc->Ghp5 = tmp_int[23];
		gc->Ghp6 = tmp_int[24];
		gc->Ghp7 = tmp_int[25];	// end additions [Valaris]
	// old structure of guild castle
	} else if (sscanf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
	                  &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3], &tmp_int[4], &tmp_int[5], &tmp_int[6],
	                  &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12], &tmp_int[13],
	                  &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17]) == 18) {
		gc->castle_id = tmp_int[0];
		gc->guild_id = tmp_int[1];
		gc->economy = tmp_int[2];
		gc->defense = tmp_int[3];
		gc->triggerE = tmp_int[4];
		gc->triggerD = tmp_int[5];
		gc->nextTime = tmp_int[6];
		gc->payTime = tmp_int[7];
		gc->createTime = tmp_int[8];
		gc->visibleC = tmp_int[9];
		gc->visibleG0 = tmp_int[10];
		gc->visibleG1 = tmp_int[11];
		gc->visibleG2 = tmp_int[12];
		gc->visibleG3 = tmp_int[13];
		gc->visibleG4 = tmp_int[14];
		gc->visibleG5 = tmp_int[15];
		gc->visibleG6 = tmp_int[16];
		gc->visibleG7 = tmp_int[17];
		if (gc->visibleG0 == 1)
			gc->Ghp0 = 15670 + 2000 * gc->defense;
		else
			gc->Ghp0 = 0;
		if (gc->visibleG1 == 1)
			gc->Ghp1 = 15670 + 2000 * gc->defense;
		else
			gc->Ghp1 = 0;
		if (gc->visibleG2 == 1)
			gc->Ghp2 = 15670 + 2000 * gc->defense;
		else
			gc->Ghp2 = 0;
		if (gc->visibleG3 == 1)
			gc->Ghp3 = 30214 + 2000 * gc->defense;
		else
			gc->Ghp3 = 0;
		if (gc->visibleG4 == 1)
			gc->Ghp4 = 30214 + 2000 * gc->defense;
		else
			gc->Ghp4 = 0;
		if (gc->visibleG5 == 1)
			gc->Ghp5 = 28634 + 2000 * gc->defense;
		else
			gc->Ghp5 = 0;
		if (gc->visibleG6 == 1)
			gc->Ghp6 = 28634 + 2000 * gc->defense;
		else
			gc->Ghp6 = 0;
		if (gc->visibleG7 == 1)
			gc->Ghp7 = 28634 + 2000 * gc->defense;
		else
			gc->Ghp7 = 0;
	} else {
		return 1;
	}

	return 0;
}

// ギルド関連データベース読み込み
int inter_guild_readdb() {
	int i;
	FILE *fp;
	char line[1024];
	char path[1024];

	sprintf(path, "%s%s", db_path, "/exp_guild.txt");
	fp = fopen(path, "r");
	if (fp == NULL) {
		printf("can't read db/exp_guild.txt\n");
		return 1;
	}
	i = 0;
	while(fgets(line, sizeof(line)-1, fp) && i < 100){
		if (line[0] == '/' && line[1] == '/')
			continue;
		guild_exp[i] = atoi(line);
		i++;
	}
	fclose(fp);

	return 0;
}

// ギルドデータの読み込み
int inter_guild_init() {
	char line[16384];
	struct guild *g;
	struct guild_castle *gc;
	FILE *fp;
	int i, j, c = 0;

	inter_guild_readdb();

	guild_db = numdb_init();
	castle_db = numdb_init();

	if ((fp = fopen(guild_txt,"r")) == NULL)
		return 1;
	while(fgets(line, sizeof(line)-1, fp)) {
		j = 0;
		if (sscanf(line, "%d\t%%newid%%\n%n", &i, &j) == 1 && j > 0 && guild_newid <= i) {
			guild_newid = i;
			continue;
		}

		g = calloc(sizeof(struct guild), 1);
		if(g == NULL){
			printf("int_guild: out of memory!\n");
			exit(0);
		}
		memset(g, 0, sizeof(struct guild));
		if (inter_guild_fromstr(line, g) == 0 && g->guild_id > 0) {
			if (g->guild_id >= guild_newid)
				guild_newid = g->guild_id + 1;
			numdb_insert(guild_db, g->guild_id, g);
			guild_check_empty(g);
			guild_calcinfo(g);
		} else {
			printf("int_guild: broken data [%s] line %d\n", guild_txt, c);
			free(g);
		}
		c++;
	}
	fclose(fp);
//	printf("int_guild: %s read done (%d guilds)\n", guild_txt, c);

	c = 0;//カウンタ初期化

	if ((fp = fopen(castle_txt, "r")) == NULL) {
		return 1;
	}

	while(fgets(line, sizeof(line)-1, fp)) {
		gc = calloc(sizeof(struct guild_castle), 1);
		if(gc == NULL){
			printf("int_guild: out of memory!\n");
			exit(0);
		}
		memset(gc, 0, sizeof(struct guild_castle));
		if (inter_guildcastle_fromstr(line, gc) == 0) {
			numdb_insert(castle_db, gc->castle_id, gc);
		} else {
			printf("int_guild: broken data [%s] line %d\n", castle_txt, c);
			free(gc);
		}
		c++;
	}

	if (!c) {
		printf(" %s - making Default Data...\n", castle_txt);
		//デフォルトデータを作成
		for(i = 0; i < MAX_GUILDCASTLE; i++) {
			gc = calloc(sizeof(struct guild_castle), 1);
			if (gc == NULL) {
				printf("int_guild: out of memory!\n");
				exit(0);
			}
			memset(gc, 0, sizeof(struct guild_castle));
			gc->castle_id = i;
			gc->guild_id = 0;
			gc->economy = 0;
			gc->defense = 0;
			gc->triggerE = 0;
			gc->triggerD = 0;
			gc->nextTime = 0;
			gc->payTime = 0;
			gc->createTime = 0;
			gc->visibleC = 0;
			gc->visibleG0 = 0;
			gc->visibleG1 = 0;
			gc->visibleG2 = 0;
			gc->visibleG3 = 0;
			gc->visibleG4 = 0;
			gc->visibleG5 = 0;
			gc->visibleG6 = 0;
			gc->visibleG7 = 0;
			gc->Ghp0 = 0;	// guardian HP [Valaris]
			gc->Ghp1 = 0;
			gc->Ghp2 = 0;
			gc->Ghp3 = 0;
			gc->Ghp4 = 0;
			gc->Ghp5 = 0;
			gc->Ghp6 = 0;
			gc->Ghp7 = 0;	// end additions [Valaris]
			numdb_insert(castle_db, gc->castle_id, gc);
		}
		printf(" %s - making done\n",castle_txt);
		return 0;
	}

	fclose(fp);

	return 0;
}

struct guild *inter_guild_search(int guild_id) {
	struct guild *g;

	g=numdb_search(guild_db, guild_id);

	return g;
}

// ギルドデータのセーブ用
int inter_guild_save_sub(void *key,void *data,va_list ap) {
	char line[16384];
	FILE *fp;

	inter_guild_tostr(line,(struct guild *)data);
	fp=va_arg(ap,FILE *);
	fprintf(fp,"%s" RETCODE,line);

	return 0;
}

// ギルド城データのセーブ用
int inter_castle_save_sub(void *key, void *data, va_list ap) {
	char line[16384];
	FILE *fp;

	inter_guildcastle_tostr(line, (struct guild_castle *)data);
	fp = va_arg(ap, FILE *);
	fprintf(fp, "%s" RETCODE, line);

	return 0;
}

// ギルドデータのセーブ
int inter_guild_save() {
	FILE *fp;
	int lock;

	if ((fp = lock_fopen(guild_txt, &lock)) == NULL) {
		printf("int_guild: cant write [%s] !!! data is lost !!!\n", guild_txt);
		return 1;
	}
	numdb_foreach(guild_db, inter_guild_save_sub, fp);
//	fprintf(fp, "%d\t%%newid%%\n", guild_newid);
	lock_fclose(fp, guild_txt, &lock);
//	printf("int_guild: %s saved.\n", guild_txt);

	if ((fp = lock_fopen(castle_txt,&lock)) == NULL) {
		printf("int_guild: cant write [%s] !!! data is lost !!!\n", castle_txt);
		return 1;
	}
	numdb_foreach(castle_db, inter_castle_save_sub, fp);
	lock_fclose(fp, castle_txt, &lock);

	return 0;
}

// ギルド名検索用
int search_guildname_sub(void *key, void *data, va_list ap) {
	struct guild *g = (struct guild *)data, **dst;
	char *str;

	str = va_arg(ap, char *);
	dst = va_arg(ap, struct guild **);
	if (strcmpi(g->name, str) == 0)
		*dst = g;
	return 0;
}

// ギルド名検索
struct guild* search_guildname(char *str) {
	struct guild *g = NULL;
	numdb_foreach(guild_db, search_guildname_sub, str, &g);
	return g;
}

// ギルドが空かどうかチェック
int guild_check_empty(struct guild *g) {
	int i;

	for(i = 0; i < g->max_member; i++) {
		if (g->member[i].account_id > 0) {
			return 0;
		}
	}
		// 誰もいないので解散
	numdb_foreach(guild_db, guild_break_sub, g->guild_id);
	numdb_erase(guild_db, g->guild_id);
	inter_guild_storage_delete(g->guild_id);
	mapif_guild_broken(g->guild_id, 0);
	free(g);

	return 1;
}

// キャラの競合がないかチェック用
int guild_check_conflict_sub(void *key, void *data, va_list ap) {
	struct guild *g = (struct guild *)data;
	int guild_id, account_id, char_id, i;

	guild_id = va_arg(ap, int);
	account_id = va_arg(ap, int);
	char_id = va_arg(ap, int);

	if (g->guild_id == guild_id)	// 本来の所属なので問題なし
		return 0;

	for(i = 0; i < MAX_GUILD; i++) {
		if (g->member[i].account_id == account_id && g->member[i].char_id == char_id) {
			// 別のギルドに偽の所属データがあるので脱退
			printf("int_guild: guild conflict! %d,%d %d!=%d\n", account_id, char_id, guild_id, g->guild_id);
			mapif_parse_GuildLeave(-1, g->guild_id, account_id, char_id, 0, "**データ競合**");
		}
	}

	return 0;
}
// キャラの競合がないかチェック
int guild_check_conflict(int guild_id, int account_id, int char_id) {
	numdb_foreach(guild_db, guild_check_conflict_sub, guild_id, account_id, char_id);

	return 0;
}

int guild_nextexp(int level) {
	if (level < 100)
		return guild_exp[level-1];

	return 0;
}

// ギルドスキルがあるか確認
int guild_checkskill(struct guild *g, int id) {
	int idx = id - GD_SKILLBASE;


	if(idx < 0 || idx >= MAX_GUILDSKILL)

		return 0;

	return g->skill[idx].lv;
}

// ギルドの情報の再計算
int guild_calcinfo(struct guild *g) {
	int i, c, nextexp;
	struct guild before = *g;

	// スキルIDの設定
	for(i = 0; i < MAX_GUILDSKILL; i++)
		g->skill[i].id=i+GD_SKILLBASE;

	// ギルドレベル
	if (g->guild_lv <= 0)
		g->guild_lv = 1;
	nextexp = guild_nextexp(g->guild_lv);
	if (nextexp > 0) {
		while(g->exp >= nextexp) {	// レベルアップ処理
			g->exp -= nextexp;
			g->guild_lv++;
			g->skill_point++;
			nextexp = guild_nextexp(g->guild_lv);
		}
	}

	// ギルドの次の経験値
	g->next_exp = guild_nextexp(g->guild_lv);

	// メンバ上限（ギルド拡張適用）
	g->max_member = 16 + guild_checkskill(g, GD_EXTENSION) * 2;

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
int mapif_guild_created(int fd, int account_id, struct guild *g) {
	WFIFOW(fd,0) = 0x3830;
	WFIFOL(fd,2) = account_id;
	if (g != NULL) {
		WFIFOL(fd,6) = g->guild_id;
		printf("int_guild: created! %d %s\n", g->guild_id, g->name);
	}else{
		WFIFOL(fd,6) = 0;
	}
	WFIFOSET(fd,10);
	return 0;
}

// ギルド情報見つからず
int mapif_guild_noinfo(int fd, int guild_id) {
	WFIFOW(fd,0) = 0x3831;
	WFIFOW(fd,2) = 8;
	WFIFOL(fd,4) = guild_id;
	WFIFOSET(fd,8);
	printf("int_guild: info not found %d\n", guild_id);

	return 0;
}

// ギルド情報まとめ送り
int mapif_guild_info(int fd, struct guild *g) {
	unsigned char buf[4 + sizeof(struct guild)];

	WBUFW(buf,0) = 0x3831;
	memcpy(buf + 4, g, sizeof(struct guild));
	WBUFW(buf,2) = 4 + sizeof(struct guild);
//	printf("int_guild: sizeof(guild)=%d\n", sizeof(struct guild));
	if (fd < 0)
		mapif_sendall(buf, WBUFW(buf,2));
	else
		mapif_send(fd, buf, WBUFW(buf,2));
//	printf("int_guild: info %d %s\n", p->guild_id, p->name);

	return 0;
}

// メンバ追加可否
int mapif_guild_memberadded(int fd, int guild_id, int account_id, int char_id, int flag) {
	WFIFOW(fd,0) = 0x3832;
	WFIFOL(fd,2) = guild_id;
	WFIFOL(fd,6) = account_id;
	WFIFOL(fd,10) = char_id;
	WFIFOB(fd,14) = flag;
	WFIFOSET(fd, 15);

	return 0;
}

// 脱退/追放通知
int mapif_guild_leaved(int guild_id, int account_id, int char_id, int flag, const char *name, const char *mes) {
	unsigned char buf[79];

	WBUFW(buf, 0) = 0x3834;
	WBUFL(buf, 2) = guild_id;
	WBUFL(buf, 6) = account_id;
	WBUFL(buf,10) = char_id;
	WBUFB(buf,14) = flag;
	memcpy(WBUFP(buf,15), mes, 40);
	memcpy(WBUFP(buf,55), name, 24);
	mapif_sendall(buf, 79);
	printf("int_guild: guild leaved %d %d %s %s\n", guild_id, account_id, name, mes);

	return 0;
}

// オンライン状態とLv更新通知
int mapif_guild_memberinfoshort(struct guild *g, int idx) {
	unsigned char buf[19];

	WBUFW(buf, 0) = 0x3835;
	WBUFL(buf, 2) = g->guild_id;
	WBUFL(buf, 6) = g->member[idx].account_id;
	WBUFL(buf,10) = g->member[idx].char_id;
	WBUFB(buf,14) = g->member[idx].online;
	WBUFW(buf,15) = g->member[idx].lv;
	WBUFW(buf,17) = g->member[idx].class;
	mapif_sendall(buf, 19);
	return 0;
}

// 解散通知
int mapif_guild_broken(int guild_id, int flag) {
	unsigned char buf[7];

	WBUFW(buf,0) = 0x3836;
	WBUFL(buf,2) = guild_id;
	WBUFB(buf,6) = flag;
	mapif_sendall(buf, 7);
	printf("int_guild: broken %d\n", guild_id);

	return 0;
}

// ギルド内発言
int mapif_guild_message(int guild_id, int account_id, char *mes, int len) {
	unsigned char buf[len+12];

	WBUFW(buf,0) = 0x3837;
	WBUFW(buf,2) = len + 12;
	WBUFL(buf,4) = guild_id;
	WBUFL(buf,8) = account_id;
	memcpy(WBUFP(buf,12), mes, len);
	mapif_sendall(buf, len + 12);

	return 0;
}

// ギルド基本情報変更通知
int mapif_guild_basicinfochanged(int guild_id, int type, const void *data, int len) {
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
int mapif_guild_memberinfochanged(int guild_id, int account_id, int char_id, int type, const void *data, int len) {
	unsigned char buf[len + 18];

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
int mapif_guild_skillupack(int guild_id, int skill_num, int account_id) {
	unsigned char buf[14];

	WBUFW(buf, 0) = 0x383c;
	WBUFL(buf, 2) = guild_id;
	WBUFL(buf, 6) = skill_num;
	WBUFL(buf,10) = account_id;
	mapif_sendall(buf, 14);

	return 0;
}

// ギルド同盟/敵対通知
int mapif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag, const char *name1, const char *name2) {
	unsigned char buf[67];

	WBUFW(buf, 0) = 0x383d;
	WBUFL(buf, 2) = guild_id1;
	WBUFL(buf, 6) = guild_id2;
	WBUFL(buf,10) = account_id1;
	WBUFL(buf,14) = account_id2;
	WBUFB(buf,18) = flag;
	memcpy(WBUFP(buf,19), name1, 24);
	memcpy(WBUFP(buf,43), name2, 24);
	mapif_sendall(buf, 67);

	return 0;
}

// ギルド役職変更通知
int mapif_guild_position(struct guild *g, int idx) {
	unsigned char buf[sizeof(struct guild_position) + 12];

	WBUFW(buf,0) = 0x383b;
	WBUFW(buf,2) = sizeof(struct guild_position) + 12;
	WBUFL(buf,4) = g->guild_id;
	WBUFL(buf,8) = idx;
	memcpy(WBUFP(buf,12), &g->position[idx], sizeof(struct guild_position));
	mapif_sendall(buf, WBUFW(buf,2));

	return 0;
}

// ギルド告知変更通知
int mapif_guild_notice(struct guild *g) {
	unsigned char buf[186];

	WBUFW(buf,0) = 0x383e;
	WBUFL(buf,2) = g->guild_id;
	memcpy(WBUFP(buf,6), g->mes1, 60);
	memcpy(WBUFP(buf,66), g->mes2, 120);
	mapif_sendall(buf, 186);

	return 0;
}

// ギルドエンブレム変更通知
int mapif_guild_emblem(struct guild *g) {
	unsigned char buf[2048];

	WBUFW(buf,0) = 0x383f;
	WBUFW(buf,2) = g->emblem_len + 12;
	WBUFL(buf,4) = g->guild_id;
	WBUFL(buf,8) = g->emblem_id;
	memcpy(WBUFP(buf,12), g->emblem_data, g->emblem_len);
	mapif_sendall(buf, WBUFW(buf,2));

	return 0;
}

int mapif_guild_castle_dataload(int castle_id, int index, int value) {
	unsigned char buf[9];

	WBUFW(buf,0) = 0x3840;
	WBUFW(buf,2) = castle_id;
	WBUFB(buf,4) = index;
	WBUFL(buf,5) = value;
	mapif_sendall(buf,9);

	return 0;
}

int mapif_guild_castle_datasave(int castle_id, int index, int value) {
	unsigned char buf[9];

	WBUFW(buf,0) = 0x3841;
	WBUFW(buf,2) = castle_id;
	WBUFB(buf,4) = index;
	WBUFL(buf,5) = value;
	mapif_sendall(buf,9);

	return 0;
}

int mapif_guild_castle_alldataload_sub(void *key, void *data, va_list ap) {
	int fd = va_arg(ap, int);
	int *p = va_arg(ap, int*);

	memcpy(WFIFOP(fd,*p), (struct guild_castle*)data, sizeof(struct guild_castle));
	(*p) += sizeof(struct guild_castle);

	return 0;
}

int mapif_guild_castle_alldataload(int fd) {
	int len = 4;

	WFIFOW(fd,0) = 0x3842;
	numdb_foreach(castle_db, mapif_guild_castle_alldataload_sub, fd, &len);
	WFIFOW(fd,2) = len;
	WFIFOSET(fd, len);

	return 0;
}

//-------------------------------------------------------------------
// map serverからの通信

// ギルド作成要求
int mapif_parse_CreateGuild(int fd, int account_id, char *name, struct guild_member *master) {
	struct guild *g;
	int i;

	for(i = 0; i < 24 && name[i]; i++) {
		if (!(name[i] & 0xe0) || name[i] == 0x7f) {
			printf("int_guild: illeagal guild name [%s]\n", name);
			mapif_guild_created(fd, account_id, NULL);
			return 0;
		}
	}

	if ((g = search_guildname(name)) != NULL) {
		printf("int_guild: same name guild exists [%s]\n", name);
		mapif_guild_created(fd, account_id, NULL);
		return 0;
	}
	g = calloc(sizeof(struct guild), 1);
	if (g == NULL) {
		printf("int_guild: CreateGuild: out of memory !\n");
		mapif_guild_created(fd, account_id, NULL);
		exit(0);
	}
	memset(g, 0, sizeof(struct guild));
	g->guild_id = guild_newid++;
	memcpy(g->name, name, 24);
	memcpy(g->master, master->name, 24);
	memcpy(&g->member[0], master, sizeof(struct guild_member));

	g->position[0].mode = 0x11;
	strcpy(g->position[                  0].name, "GuildMaster");
	strcpy(g->position[MAX_GUILDPOSITION-1].name, "Newbie");
	for(i = 1; i < MAX_GUILDPOSITION-1; i++)
		sprintf(g->position[i].name, "Position %d", i + 1);

	// ここでギルド情報計算が必要と思われる
	g->max_member = 16;
	g->average_lv = master->lv;
	for(i = 0; i < MAX_GUILDSKILL; i++)
		g->skill[i].id=i + GD_SKILLBASE;

	numdb_insert(guild_db, g->guild_id, g);

	mapif_guild_created(fd, account_id, g);
	mapif_guild_info(fd, g);

	if(log_inter)
		inter_log("guild %s (id=%d) created by master %s (id=%d)" RETCODE,
			name, g->guild_id, master->name, master->account_id);

	return 0;
}

// ギルド情報要求
int mapif_parse_GuildInfo(int fd, int guild_id) {
	struct guild *g;

	g = numdb_search(guild_db, guild_id);
	if (g != NULL){
		guild_calcinfo(g);
		mapif_guild_info(fd, g);
	} else
		mapif_guild_noinfo(fd, guild_id);

	return 0;
}

// ギルドメンバ追加要求
int mapif_parse_GuildAddMember(int fd, int guild_id, struct guild_member *m) {
	struct guild *g;
	int i;

	g = numdb_search(guild_db, guild_id);
	if (g == NULL) {
		mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 1);
		return 0;
	}

	for(i = 0; i < g->max_member; i++) {
		if (g->member[i].account_id == 0) {
			memcpy(&g->member[i], m, sizeof(struct guild_member));
			mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 0);
			guild_calcinfo(g);
			mapif_guild_info(-1, g);

			return 0;
		}
	}
	mapif_guild_memberadded(fd, guild_id, m->account_id, m->char_id, 1);

	return 0;
}

// ギルド脱退/追放要求
int mapif_parse_GuildLeave(int fd, int guild_id, int account_id, int char_id, int flag, const char *mes) {
	struct guild *g = NULL;
	int i, j;

	g = numdb_search(guild_db, guild_id);
	if (g != NULL) {
		for(i = 0; i < MAX_GUILD; i++) {
			if (g->member[i].account_id == account_id && g->member[i].char_id == char_id) {
//				printf("%d %d\n", i, (int)(&g->member[i]));
//				printf("%d %s\n", i, g->member[i].name);

				if (flag) {	// 追放の場合追放リストに入れる
					for(j = 0; j < MAX_GUILDEXPLUSION; j++) {
						if (g->explusion[j].account_id == 0)
							break;
					}
					if (j == MAX_GUILDEXPLUSION) {	// 一杯なので古いのを消す
						for(j = 0; j < MAX_GUILDEXPLUSION - 1; j++)
							g->explusion[j] = g->explusion[j+1];
						j = MAX_GUILDEXPLUSION - 1;
					}
					g->explusion[j].account_id = account_id;
					memcpy(g->explusion[j].acc, "dummy", 24);
					memcpy(g->explusion[j].name, g->member[i].name, 24);
					memcpy(g->explusion[j].mes, mes, 40);
				}

				mapif_guild_leaved(guild_id, account_id, char_id, flag, g->member[i].name, mes);
//				printf("%d %d\n", i, (int)(&g->member[i]));
//				printf("%d %s\n", i, (&g->member[i])->name);
				memset(&g->member[i], 0, sizeof(struct guild_member));

				if (guild_check_empty(g) == 0)
					mapif_guild_info(-1,g);// まだ人がいるのでデータ送信

				return 0;
			}
		}
	}
	return 0;
}

// オンライン/Lv更新
int mapif_parse_GuildChangeMemberInfoShort(int fd, int guild_id, int account_id, int char_id, int online, int lv, int class) {
	struct guild *g;
	int i, alv, c;

	g = numdb_search(guild_db, guild_id);
	if (g == NULL)
		return 0;

	g->connect_member = 0;

	alv = 0;
	c = 0;
	for(i = 0; i < MAX_GUILD; i++) {
		if (g->member[i].account_id == account_id && g->member[i].char_id == char_id) {
			g->member[i].online = online;
			g->member[i].lv = lv;
			g->member[i].class = class;
			mapif_guild_memberinfoshort(g, i);
		}
		if (g->member[i].account_id > 0) {
			alv += g->member[i].lv;
			c++;
		}
		if (g->member[i].online)
			g->connect_member++;
	}
	// 平均レベル
	g->average_lv = alv / c;

	return 0;
}

// ギルド解散処理用（同盟/敵対を解除）
int guild_break_sub(void *key, void *data, va_list ap) {
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
int mapif_parse_BreakGuild(int fd, int guild_id) {
	struct guild *g;

	g = numdb_search(guild_db, guild_id);
	if(g == NULL)
		return 0;

	numdb_foreach(guild_db, guild_break_sub, guild_id);
	numdb_erase(guild_db, guild_id);
	inter_guild_storage_delete(guild_id);
	mapif_guild_broken(guild_id, 0);

	if(log_inter)
		inter_log("guild %s (id=%d) broken" RETCODE, g->name, guild_id);
	free(g);

	return 0;
}

// ギルドメッセージ送信
int mapif_parse_GuildMessage(int fd, int guild_id, int account_id, char *mes, int len) {
	return mapif_guild_message(guild_id, account_id, mes, len);
}

// ギルド基本データ変更要求
int mapif_parse_GuildBasicInfoChange(int fd, int guild_id, int type, const char *data, int len) {
	struct guild *g;
	short dw = *((short *)data);

	g = numdb_search(guild_db, guild_id);
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
		printf("int_guild: GuildBasicInfoChange: Unknown type %d\n", type);
		break;
	}
	mapif_guild_basicinfochanged(guild_id, type, data, len);

	return 0;
}

// ギルドメンバデータ変更要求
int mapif_parse_GuildMemberInfoChange(int fd, int guild_id, int account_id, int char_id, int type, const char *data, int len) {
	int i;
	struct guild *g;

	g = numdb_search(guild_db, guild_id);
	if(g == NULL)
		return 0;

	for(i = 0; i < g->max_member; i++)
		if (g->member[i].account_id == account_id && g->member[i].char_id == char_id)
			break;
		if (i == g->max_member) {
			printf("int_guild: GuildMemberChange: Not found %d,%d in %d[%s]\n", account_id, char_id, guild_id, g->name);
			return 0;
		}
		switch(type) {
		case GMI_POSITION:	// 役職
			g->member[i].position = *((int *)data);
			break;
		case GMI_EXP:	// EXP
		  {
			int exp, oldexp = g->member[i].exp;
			exp = g->member[i].exp = *((unsigned int *)data);
			g->exp += (exp - oldexp);
			guild_calcinfo(g);	// Lvアップ判断
			mapif_guild_basicinfochanged(guild_id, GBI_EXP, &g->exp, 4);
		  }
			break;
	default:
		printf("int_guild: GuildMemberInfoChange: Unknown type %d\n", type);
		break;
	}
	mapif_guild_memberinfochanged(guild_id, account_id, char_id, type, data, len);

	return 0;
}

// ギルド役職名変更要求
int mapif_parse_GuildPosition(int fd, int guild_id, int idx, struct guild_position *p) {
	struct guild *g = numdb_search(guild_db, guild_id);

	if (g == NULL || idx < 0 || idx >= MAX_GUILDPOSITION) {
		return 0;
	}
	memcpy(&g->position[idx], p, sizeof(struct guild_position));
	mapif_guild_position(g, idx);
	printf("int_guild: position changed %d\n", idx);

	return 0;
}

// ギルドスキルアップ要求
int mapif_parse_GuildSkillUp(int fd, int guild_id, int skill_num, int account_id) {
	struct guild *g = numdb_search(guild_db, guild_id);
	int idx = skill_num - GD_SKILLBASE;

	if (g == NULL || idx < 0 || idx >= MAX_GUILDSKILL)
		return 0;

	if (g->skill_point > 0 && g->skill[idx].id > 0 && g->skill[idx].lv < 10) {
		g->skill[idx].lv++;
		g->skill_point--;
		if (guild_calcinfo(g) == 0)
			mapif_guild_info(-1, g);
		mapif_guild_skillupack(guild_id, skill_num, account_id);
		printf("int_guild: skill %d up\n", skill_num);
	}

	return 0;
}

// ギルド同盟要求
int mapif_parse_GuildAlliance(int fd, int guild_id1, int guild_id2, int account_id1, int account_id2, int flag) {
	struct guild *g[2];
	int j, i;

	g[0] = numdb_search(guild_db, guild_id1);
	g[1] = numdb_search(guild_db, guild_id2);
	if (g[0] == NULL || g[1] == NULL)
		return 0;

	if (!(flag & 0x8)) {
		for(i = 0; i < 2 - (flag & 1); i++) {
			for(j = 0; j < MAX_GUILDALLIANCE; j++)
				if (g[i]->alliance[j].guild_id == 0) {
					g[i]->alliance[j].guild_id = g[1-i]->guild_id;
					memcpy(g[i]->alliance[j].name, g[1-i]->name, 24);
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
int mapif_parse_GuildNotice(int fd, int guild_id, const char *mes1, const char *mes2) {
	struct guild *g;

	g = numdb_search(guild_db, guild_id);
	if (g == NULL)
		return 0;
	memcpy(g->mes1, mes1, 60);
	memcpy(g->mes2, mes2, 120);

	return mapif_guild_notice(g);
}

// ギルドエンブレム変更要求
int mapif_parse_GuildEmblem(int fd, int len, int guild_id, int dummy, const char *data) {
	struct guild *g;

	g = numdb_search(guild_db, guild_id);
	if (g == NULL)
		return 0;
	memcpy(g->emblem_data, data, len);
	g->emblem_len = len;
	g->emblem_id++;

	return mapif_guild_emblem(g);
}

int mapif_parse_GuildCastleDataLoad(int fd, int castle_id, int index) {
	struct guild_castle *gc = numdb_search(castle_db, castle_id);

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
	case 10: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleG0);
	case 11: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleG1);
	case 12: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleG2);
	case 13: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleG3);
	case 14: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleG4);
	case 15: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleG5);
	case 16: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleG6);
	case 17: return mapif_guild_castle_dataload(gc->castle_id, index, gc->visibleG7);
	case 18: return mapif_guild_castle_dataload(gc->castle_id, index, gc->Ghp0);	// guardian HP [Valaris]
	case 19: return mapif_guild_castle_dataload(gc->castle_id, index, gc->Ghp1);
	case 20: return mapif_guild_castle_dataload(gc->castle_id, index, gc->Ghp2);
	case 21: return mapif_guild_castle_dataload(gc->castle_id, index, gc->Ghp3);
	case 22: return mapif_guild_castle_dataload(gc->castle_id, index, gc->Ghp4);
	case 23: return mapif_guild_castle_dataload(gc->castle_id, index, gc->Ghp5);
	case 24: return mapif_guild_castle_dataload(gc->castle_id, index, gc->Ghp6);
	case 25: return mapif_guild_castle_dataload(gc->castle_id, index, gc->Ghp7);	// end additions [Valaris]

	default:
		printf("mapif_parse_GuildCastleDataLoad ERROR!! (Not found index=%d)\n", index);
		return 0;
	}

	return 0;
}

int mapif_parse_GuildCastleDataSave(int fd, int castle_id, int index, int value) {
	struct guild_castle *gc=numdb_search(castle_db, castle_id);

	if (gc == NULL) {
		return mapif_guild_castle_datasave(castle_id, index, value);
	}
	switch(index) {
	case 1:
		if (gc->guild_id != value) {
			int gid = (value) ? value : gc->guild_id;
			struct guild *g = numdb_search(guild_db, gid);
			if(log_inter)
				inter_log("guild %s (id=%d) %s castle id=%d" RETCODE,
					(g) ? g->name : "??", gid, (value) ? "occupy" : "abandon", index);
		}
		gc->guild_id = value;
		break;
	case 2: gc->economy = value; break;
	case 3: gc->defense = value; break;
	case 4: gc->triggerE = value; break;
	case 5: gc->triggerD = value; break;
	case 6: gc->nextTime = value; break;
	case 7: gc->payTime = value; break;
	case 8: gc->createTime = value; break;
	case 9: gc->visibleC = value; break;
	case 10: gc->visibleG0 = value; break;
	case 11: gc->visibleG1 = value; break;
	case 12: gc->visibleG2 = value; break;
	case 13: gc->visibleG3 = value; break;
	case 14: gc->visibleG4 = value; break;
	case 15: gc->visibleG5 = value; break;
	case 16: gc->visibleG6 = value; break;
	case 17: gc->visibleG7 = value; break;
	case 18: gc->Ghp0 = value; break;	// guardian HP [Valaris]
	case 19: gc->Ghp1 = value; break;
	case 20: gc->Ghp2 = value; break;
	case 21: gc->Ghp3 = value; break;
	case 22: gc->Ghp4 = value; break;
	case 23: gc->Ghp5 = value; break;
	case 24: gc->Ghp6 = value; break;
	case 25: gc->Ghp7 = value; break;	// end additions [Valaris]
	default:
		printf("mapif_parse_GuildCastleDataSave ERROR!! (Not found index=%d)\n", index);
		return 0;
	}

	return mapif_guild_castle_datasave(gc->castle_id, index, value);
}

// ギルドチェック要求
int mapif_parse_GuildCheck(int fd, int guild_id, int account_id, int char_id) {
	return guild_check_conflict(guild_id, account_id, char_id);
}

// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_guild_parse_frommap(int fd) {
	switch(RFIFOW(fd,0)) {
	case 0x3030: mapif_parse_CreateGuild(fd, RFIFOL(fd,4), RFIFOP(fd,8), (struct guild_member *)RFIFOP(fd,32)); break;
	case 0x3031: mapif_parse_GuildInfo(fd, RFIFOL(fd,2)); break;
	case 0x3032: mapif_parse_GuildAddMember(fd, RFIFOL(fd,4), (struct guild_member *)RFIFOP(fd,8)); break;
	case 0x3034: mapif_parse_GuildLeave(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), RFIFOP(fd,15)); break;
	case 0x3035: mapif_parse_GuildChangeMemberInfoShort(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOB(fd,14), RFIFOW(fd,15), RFIFOW(fd,17)); break;
	case 0x3036: mapif_parse_BreakGuild(fd, RFIFOL(fd,2)); break;
	case 0x3037: mapif_parse_GuildMessage(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12), RFIFOW(fd,2)-12); break;
	case 0x3038: mapif_parse_GuildCheck(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	case 0x3039: mapif_parse_GuildBasicInfoChange(fd, RFIFOL(fd,4), RFIFOW(fd,8), RFIFOP(fd,10), RFIFOW(fd,2)-10); break;
	case 0x303A: mapif_parse_GuildMemberInfoChange(fd, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOL(fd,12), RFIFOW(fd,16), RFIFOP(fd,18), RFIFOW(fd,2)-18); break;
	case 0x303B: mapif_parse_GuildPosition(fd, RFIFOL(fd,4), RFIFOL(fd,8), (struct guild_position *)RFIFOP(fd,12)); break;
	case 0x303C: mapif_parse_GuildSkillUp(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10)); break;
	case 0x303D: mapif_parse_GuildAlliance(fd, RFIFOL(fd,2), RFIFOL(fd,6), RFIFOL(fd,10), RFIFOL(fd,14), RFIFOB(fd,18)); break;
	case 0x303E: mapif_parse_GuildNotice(fd, RFIFOL(fd,2), RFIFOP(fd,6), RFIFOP(fd,66)); break;
	case 0x303F: mapif_parse_GuildEmblem(fd, RFIFOW(fd,2)-12, RFIFOL(fd,4), RFIFOL(fd,8), RFIFOP(fd,12)); break;
	case 0x3040: mapif_parse_GuildCastleDataLoad(fd, RFIFOW(fd,2), RFIFOB(fd,4)); break;
	case 0x3041: mapif_parse_GuildCastleDataSave(fd, RFIFOW(fd,2), RFIFOB(fd,4), RFIFOL(fd,5)); break;

	default:
		return 0;
	}

	return 1;
}

// マップサーバーの接続時処理
int inter_guild_mapif_init(int fd) {
	return mapif_guild_castle_alldataload(fd);
}

// サーバーから脱退要求（キャラ削除用）
int inter_guild_leave(int guild_id, int account_id, int char_id) {
	return mapif_parse_GuildLeave(-1, guild_id, account_id, char_id, 0, "**サーバー命令**");
}
