//
// original code from athena
// SQL conversion by hack
//

#include "char.h"
#include "strlib.h"
#include "int_storage.h"
#include "inter.h"
#include "int_guild.h"
#include "int_storage.h"
#include "mmo.h"
#include "socket.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static struct guild *guild_pt;
static struct guild *guild_pt2;
static struct guild_castle * guildcastle_pt;
static int guild_newid=10000;

static int guild_exp[100];

int mapif_parse_GuildLeave(int fd,int guild_id,int account_id,int char_id,int flag,const char *mes);
int mapif_guild_broken(int guild_id,int flag);
int guild_check_empty(struct guild *g);
int guild_calcinfo(struct guild *g);
int mapif_guild_basicinfochanged(int guild_id,int type,const void *data,int len);
int mapif_guild_info(int fd,struct guild *g);
int guild_break_sub(void *key,void *data,va_list ap);


// Save guild into sql
int inter_guild_tosql(struct guild *g,int flag)
{
	// 1 `guild` (`guild_id`, `name`,`master`,`guild_lv`,`connect_member`,`max_member`,`average_lv`,`exp`,`next_exp`,`skill_point`,`castle_id`,`mes1`,`mes2`,`emblem_len`,`emblem_id`,`emblem_data`)
	// 2 `guild_member` (`guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`rsv1`,`rsv2`,`name`)
	// 4 `guild_position` (`guild_id`,`position`,`name`,`mode`,`exp_mode`)
	// 8 `guild_alliance` (`guild_id`,`opposition`,`alliance_id`,`name`)
	// 16 `guild_expulsion` (`guild_id`,`name`,`mes`,`acc`,`account_id`,`rsv1`,`rsv2`,`rsv3`) 
	// 32 `guild_skill` (`guild_id`,`id`,`lv`)
	
	char t_name[100],t_master[24],t_mes1[60],t_mes2[120],t_member[24],t_position[24],t_alliance[24];  // temporay storage for str convertion;
	char t_ename[24],t_emes[40];
	char emblem_data[4096];
	int i=0;
	int guild_exist=0,guild_member=0,guild_online_member=0;
	
	if (g->guild_id<=0) return -1;
	
	printf("(\033[1;35m%d\033[0m)  Request save guild - ",g->guild_id);
	
	jstrescapecpy(t_name, g->name);
	
	//printf("- Check if guild %d exists\n",g->guild_id);
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `guild_id`='%d'",guild_db,g->guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild`)- %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);
		guild_exist =  atoi (sql_row[0]);
		//printf("- Check if guild %d exists : %s\n",g->guild_id,((guild_exist==0)?"No":"Yes"));
	}
	mysql_free_result(sql_res) ; //resource free
	
	if (guild_exist >0){
		// Check members in party
		sprintf(tmp_sql,"SELECT count(*) FROM `%s` WHERE `guild_id`='%d'",guild_member_db, g->guild_id);
		if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
			return -1;
		}
		sql_res = mysql_store_result(&mysql_handle) ;
		if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
			sql_row = mysql_fetch_row(sql_res);
			
			guild_member =  atoi (sql_row[0]);
		//	printf("- Check members in guild %d : %d \n",g->guild_id,guild_member);

		}
		mysql_free_result(sql_res) ; //resource free
		
		// Delete old guild from sql
		if (flag&1||guild_member==0){
		//	printf("- Delete guild %d from guild\n",g->guild_id);
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_db, g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (delete `guild`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
		if (flag&2||guild_member==0){
		//	printf("- Delete guild %d from guild_member\n",g->guild_id);
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_member_db, g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (delete `guild_member`)- %s\n", mysql_error(&mysql_handle) );
			}
			sprintf(tmp_sql, "UPDATE `%s` SET `guild_id`='0' WHERE `guild_id`='%d'",char_db, g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
		if (flag&32||guild_member==0){
		//	printf("- Delete guild %d from guild_skill\n",g->guild_id);
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_skill_db, g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (delete `guild_skill`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
		if (flag&4||guild_member==0){
		//	printf("- Delete guild %d from guild_position\n",g->guild_id);
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_position_db, g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (delete `guild_position`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
		if (flag&16||guild_member==0){
		//	printf("- Delete guild %d from guild_expulsion\n",g->guild_id);
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_expulsion_db, g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (delete `guild_expulsion`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
		if (flag&8||guild_member==0){
		//	printf("- Delete guild %d from guild_alliance\n",g->guild_id);
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d' OR `alliance_id`='%d'",guild_alliance_db, g->guild_id,g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (delete `guild_alliance`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
		if (flag&2||guild_member==0){
		//	printf("- Delete guild %d from char\n",g->guild_id);
			sprintf(tmp_sql, "UPDATE `%s` SET `guild_id`='0' WHERE `guild_id`='%d'",char_db, g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (delete `guild_alliance`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
		if (guild_member==0){
		//	printf("- Delete guild %d from guild_castle\n",g->guild_id);
			sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_castle_db, g->guild_id);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (delete `guild_castle`)- %s\n", mysql_error(&mysql_handle) );
			}			
		}
	}

	guild_online_member = 0;
	i=0;
	while (i<g->max_member) {
		if (g->member[i].account_id>0) guild_online_member++;
		i++;
	}
	
	// No member in guild , no need to create it in sql
	if (guild_member <= 0 && guild_online_member <=0) {
		inter_guild_storage_delete(g->guild_id);
		printf("No member in guild %d , break it! \n",g->guild_id);
		return -2;
	}
	
	// Insert new guild to sqlserver
	if (flag&1||guild_member==0){
		int len=0;
		//printf("- Insert guild %d to guild\n",g->guild_id);
		for(i=0;i<g->emblem_len;i++){
			len+=sprintf(emblem_data+len,"%02x",(unsigned char)(g->emblem_data[i]));
			//printf("%02x",(unsigned char)(g->emblem_data[i]));
		}	
                emblem_data[len] = '\0';
		//printf("- emblem_len = %d \n",g->emblem_len);
		sprintf(tmp_sql,"INSERT INTO `%s` "
			"(`guild_id`, `name`,`master`,`guild_lv`,`connect_member`,`max_member`,`average_lv`,`exp`,`next_exp`,`skill_point`,`castle_id`,`mes1`,`mes2`,`emblem_len`,`emblem_id`,`emblem_data`) "
			"VALUES ('%d', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%s', '%s', '%d', '%d', '%s')",
			guild_db, g->guild_id,t_name,jstrescapecpy(t_master,g->master),
			g->guild_lv,g->connect_member,g->max_member,g->average_lv,g->exp,g->next_exp,g->skill_point,g->castle_id,
			jstrescapecpy(t_mes1,g->mes1),jstrescapecpy(t_mes2,g->mes2),g->emblem_len,g->emblem_id,emblem_data);
		//printf(" %s\n",tmp_sql);
		if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error (insert `guild`)- %s\n", mysql_error(&mysql_handle) );
		}
	}
	
	if (flag&2||guild_member==0){
		//printf("- Insert guild %d to guild_member\n",g->guild_id);
		for(i=0;i<g->max_member;i++){
			if (g->member[i].account_id>0){
				struct guild_member *m = &g->member[i];
				sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",guild_member_db, m->char_id);
				if(mysql_query(&mysql_handle, tmp_sql) ) {
					printf("DB server Error (delete `guild_member`)- %s\n", mysql_error(&mysql_handle) );
				}
				sprintf(tmp_sql,"INSERT INTO `%s` "
					"(`guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`rsv1`,`rsv2`,`name`) "
					"VALUES ('%d','%d','%d','%d','%d', '%d','%d','%d','%d','%d','%d','%d','%d','%d','%s')",
					guild_member_db, g->guild_id,
					m->account_id,m->char_id,
					m->hair,m->hair_color,m->gender,
					m->class,m->lv,m->exp,m->exp_payper,m->online,m->position,
					0,0,
					jstrescapecpy(t_member,m->name));
				//printf(" %s\n",tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql) ) {
					printf("DB server Error (insert `guild_member`)- %s\n", mysql_error(&mysql_handle) );
				}
				sprintf(tmp_sql, "UPDATE `%s` SET `guild_id`='%d' WHERE `account_id`='%d' AND `char_id`='%d'",char_db, g->guild_id,m->account_id,m->char_id);
				if(mysql_query(&mysql_handle, tmp_sql) ) {
					printf("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
				}
			}
		}
	}
	
	if (flag&4||guild_member==0){
		//printf("- Insert guild %d to guild_position\n",g->guild_id);
		for(i=0;i<MAX_GUILDPOSITION;i++){
			struct guild_position *p = &g->position[i];
			sprintf(tmp_sql,"INSERT INTO `%s` (`guild_id`,`position`,`name`,`mode`,`exp_mode`) VALUES ('%d','%d', '%s','%d','%d')",
				guild_position_db, g->guild_id, i, jstrescapecpy(t_position,p->name),p->mode,p->exp_mode);
			//printf(" %s\n",tmp_sql);
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				printf("DB server Error (insert `guild_position`)- %s\n", mysql_error(&mysql_handle) );
			}
		}
	}

	if (flag&8||guild_member==0){
		//printf("- Insert guild %d to guild_alliance\n",g->guild_id);
		for(i=0;i<MAX_GUILDALLIANCE;i++){
			struct guild_alliance *a=&g->alliance[i];
			if(a->guild_id>0){
				sprintf(tmp_sql,"INSERT INTO `%s` (`guild_id`,`opposition`,`alliance_id`,`name`) "
					"VALUES ('%d','%d','%d','%s')",
					guild_alliance_db, g->guild_id,a->opposition,a->guild_id,jstrescapecpy(t_alliance,a->name));
				//printf(" %s\n",tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql) ) {
					printf("DB server Error (insert `guild_alliance`)- %s\n", mysql_error(&mysql_handle) );
				}
				sprintf(tmp_sql,"INSERT INTO `%s` (`guild_id`,`opposition`,`alliance_id`,`name`) "
					"VALUES ('%d','%d','%d','%s')",
					guild_alliance_db, a->guild_id,a->opposition,g->guild_id,t_name);
				//printf(" %s\n",tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql) ) {
					printf("DB server Error (insert `guild_alliance`)- %s\n", mysql_error(&mysql_handle) );
				}		
			}
		}
	}

	if (flag&16||guild_member==0){
		//printf("- Insert guild %d to guild_expulsion\n",g->guild_id);
		for(i=0;i<MAX_GUILDEXPLUSION;i++){
			struct guild_explusion *e=&g->explusion[i];
			if(e->account_id>0){
				sprintf(tmp_sql,"INSERT INTO `%s` (`guild_id`,`name`,`mes`,`acc`,`account_id`,`rsv1`,`rsv2`,`rsv3`) "
					"VALUES ('%d','%s','%s','%s','%d','%d','%d','%d')",
					guild_expulsion_db, g->guild_id,
					jstrescapecpy(t_ename,e->name),jstrescapecpy(t_emes,e->mes),e->acc,e->account_id,e->rsv1,e->rsv2,e->rsv3 );
				//printf(" %s\n",tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql) ) {
					printf("DB server Error (insert `guild_expulsion`)- %s\n", mysql_error(&mysql_handle) );
				}
			}
		}
	}

	if (flag&32||guild_member==0){
		//printf("- Insert guild %d to guild_skill\n",g->guild_id);
		for(i=0;i<MAX_GUILDSKILL;i++){
			if (g->skill[i].id>0){
				sprintf(tmp_sql,"INSERT INTO `%s` (`guild_id`,`id`,`lv`) VALUES ('%d','%d','%d')",
					guild_skill_db, g->guild_id,g->skill[i].id,g->skill[i].lv);
				//printf("%s\n",tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql) ) {
					printf("DB server Error (insert `guild_skill`)- %s\n", mysql_error(&mysql_handle) );
				}
			}
		}
	}
	
	printf("Save guild done\n");
	return 0;
}

// Read guild from sql
int inter_guild_fromsql(int guild_id,struct guild *g)
{
	int i;
	char emblem_data[4096];
	char *pstr;
	
	if (g==NULL) return 0;
	memset(g,0,sizeof(struct guild));
	if (guild_id==0) return 0;
	
//	printf("Retrieve guild information from sql ......\n");
//	printf("- Read guild %d from sql \n",guild_id);

	sprintf(tmp_sql,"SELECT `guild_id`, `name`,`master`,`guild_lv`,`connect_member`,`max_member`,`average_lv`,`exp`,`next_exp`,`skill_point`,`castle_id`,`mes1`,`mes2`,`emblem_len`,`emblem_id`,`emblem_data` "
		"FROM `%s` WHERE `guild_id`='%d'",guild_db, guild_id);
	//printf("  %s\n",tmp_sql);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (select `guild`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);
		if (sql_row==NULL) {
			mysql_free_result(sql_res);
			return 0;
		}
		
		g->guild_id=atoi(sql_row[0]);
		strncpy(g->name,sql_row[1],24);
		strncpy(g->master,sql_row[2],24);
		g->guild_lv=atoi(sql_row[3]);
		g->connect_member=atoi(sql_row[4]);
		g->max_member=atoi(sql_row[5]);
		g->average_lv=atoi(sql_row[6]);
		g->exp=atoi(sql_row[7]);
		g->next_exp=atoi(sql_row[8]);
		g->skill_point=atoi(sql_row[9]);
		g->castle_id=atoi(sql_row[10]);
		strncpy(g->mes1,sql_row[11],60);
		strncpy(g->mes2,sql_row[12],120);
		g->emblem_len=atoi(sql_row[13]);
		g->emblem_id=atoi(sql_row[14]);
		strncpy(emblem_data,sql_row[15],4096);
		for(i=0,pstr=emblem_data;i<g->emblem_len;i++,pstr+=2){
			int c1=pstr[0],c2=pstr[1],x1=0,x2=0;
			if(c1>='0' && c1<='9')x1=c1-'0';
			if(c1>='a' && c1<='f')x1=c1-'a'+10;
			if(c1>='A' && c1<='F')x1=c1-'A'+10;
			if(c2>='0' && c2<='9')x2=c2-'0';
			if(c2>='a' && c2<='f')x2=c2-'a'+10;
			if(c2>='A' && c2<='F')x2=c2-'A'+10;
			g->emblem_data[i]=(x1<<4)|x2;
		}	
	}
	mysql_free_result(sql_res);

	//printf("- Read guild_member %d from sql \n",guild_id);
	sprintf(tmp_sql,"SELECT `guild_id`,`account_id`,`char_id`,`hair`,`hair_color`,`gender`,`class`,`lv`,`exp`,`exp_payper`,`online`,`position`,`rsv1`,`rsv2`,`name` "
		"FROM `%s` WHERE `guild_id`='%d'  ORDER BY `position`", guild_member_db, guild_id);
	//printf("  %s\n",tmp_sql);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (select `guild_member`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		int i;
		for(i=0;((sql_row = mysql_fetch_row(sql_res))&&i<g->max_member);i++){
			struct guild_member *m = &g->member[i];
			m->account_id=atoi(sql_row[1]);
			m->char_id=atoi(sql_row[2]);
			m->hair=atoi(sql_row[3]);
			m->hair_color=atoi(sql_row[4]);
			m->gender=atoi(sql_row[5]);
			m->class=atoi(sql_row[6]);
			m->lv=atoi(sql_row[7]);
			m->exp=atoi(sql_row[8]);
			m->exp_payper=atoi(sql_row[9]);
			m->online=atoi(sql_row[10]);
			m->position=atoi(sql_row[11]);
			strncpy(m->name,sql_row[14],24);
		}
	}
	mysql_free_result(sql_res);
	
	//printf("- Read guild_position %d from sql \n",guild_id);
	sprintf(tmp_sql,"SELECT `guild_id`,`position`,`name`,`mode`,`exp_mode` FROM `%s` WHERE `guild_id`='%d'",guild_position_db, guild_id);
	//printf("  %s\n",tmp_sql);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (select `guild_position`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		int i;
		for(i=0;((sql_row = mysql_fetch_row(sql_res))&&i<MAX_GUILDPOSITION);i++){
			int position = atoi(sql_row[1]);	
			struct guild_position *p = &g->position[position];
			strncpy(p->name,sql_row[2],24);	
			p->mode=atoi(sql_row[3]);
			p->exp_mode=atoi(sql_row[4]);	
		}
	}
	mysql_free_result(sql_res);	

	//printf("- Read guild_alliance %d from sql \n",guild_id);
	sprintf(tmp_sql,"SELECT `guild_id`,`opposition`,`alliance_id`,`name` FROM `%s` WHERE `guild_id`='%d'",guild_alliance_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (select `guild_alliance`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		int i;
		for(i=0;((sql_row = mysql_fetch_row(sql_res))&&i<MAX_GUILDALLIANCE);i++){
			struct guild_alliance *a = &g->alliance[i];
			a->opposition=atoi(sql_row[1]);
			a->guild_id=atoi(sql_row[2]);
			strncpy(a->name,sql_row[3],24);
		}
	}
	mysql_free_result(sql_res);
	
	//printf("- Read guild_expulsion %d from sql \n",guild_id);
	sprintf(tmp_sql,"SELECT `guild_id`,`name`,`mes`,`acc`,`account_id`,`rsv1`,`rsv2`,`rsv3` FROM `%s` WHERE `guild_id`='%d'",guild_expulsion_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (select `guild_expulsion`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		int i;
		for(i=0;((sql_row = mysql_fetch_row(sql_res))&&i<MAX_GUILDEXPLUSION);i++){
			struct guild_explusion *e = &g->explusion[i];

			strncpy(e->name,sql_row[1],24);
			strncpy(e->mes,sql_row[2],40);
			strncpy(e->acc,sql_row[3],24);
			e->account_id=atoi(sql_row[4]);
			e->rsv1=atoi(sql_row[5]);
			e->rsv2=atoi(sql_row[6]);
			e->rsv3=atoi(sql_row[7]);
			
		}
	}
	mysql_free_result(sql_res);
	
	//printf("- Read guild_skill %d from sql \n",guild_id);
	sprintf(tmp_sql,"SELECT `guild_id`,`id`,`lv` FROM `%s` WHERE `guild_id`='%d' ORDER BY `id`",guild_skill_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (select `guild_skill`)- %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		int i;
		for(i=0;((sql_row = mysql_fetch_row(sql_res))&&i<MAX_GUILDSKILL);i++){
			g->skill[i].id=atoi(sql_row[1]);
			g->skill[i].lv=atoi(sql_row[2]);
		}
	}
	mysql_free_result(sql_res);
							
//	printf("Successfully retrieve guild information from sql!\n");

	return 0;
	
}

// Save guild_castle to sql
int inter_guildcastle_tosql(struct guild_castle *gc)
{
	// `guild_castle` (`castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`, `visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`)
	
	if (gc==NULL) return 0;
	//printf("Save to guild_castle\n");
	sprintf(tmp_sql,"DELETE FROM `%s` WHERE `castle_id`='%d'",guild_castle_db, gc->castle_id);
	//printf(" %s\n",tmp_sql);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	
	sprintf(tmp_sql,"INSERT INTO `%s` "
		"(`castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`,"
		"`visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`,"
		"`Ghp0`, `Ghp1`, `Ghp2`, `Ghp3`, `Ghp4`, `Ghp5`, `Ghp6`, `Ghp7`)"
		"VALUES ('%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d')", 
		guild_castle_db, gc->castle_id, gc->guild_id,  gc->economy, gc->defense, gc->triggerE, gc->triggerD, gc->nextTime, gc->payTime, 
		gc->createTime, gc->visibleC, gc->visibleG0, gc->visibleG1, gc->visibleG2, gc->visibleG3, gc->visibleG4, gc->visibleG5,
		gc->visibleG6, gc->visibleG7, gc->Ghp0, gc->Ghp1, gc->Ghp2, gc->Ghp3, gc->Ghp4, gc->Ghp5, gc->Ghp6, gc->Ghp7);
	//printf(" %s\n",tmp_sql);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		return 0;
	}

	sprintf(tmp_sql,"UPDATE `%s` SET `castle_id`='-1' WHERE `castle_id`='%d'",guild_db, gc->castle_id);
	//printf(" %s\n",tmp_sql);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
		
	sprintf(tmp_sql,"UPDATE `%s` SET `castle_id`='%d' WHERE `guild_id`='%d'",guild_db, gc->castle_id,gc->guild_id);
	//printf(" %s\n",tmp_sql);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	
	return 0;
}
// Read guild_castle from sql
int inter_guildcastle_fromsql(int castle_id,struct guild_castle *gc)
{
	
	if (gc==NULL) return 0;
	//printf("Read from guild_castle\n");
	memset(gc,0,sizeof(struct guild_castle));
	gc->castle_id=castle_id;
	if (castle_id==-1) return 0;
	sprintf(tmp_sql,"SELECT `castle_id`, `guild_id`, `economy`, `defense`, `triggerE`, `triggerD`, `nextTime`, `payTime`, `createTime`, "
		"`visibleC`, `visibleG0`, `visibleG1`, `visibleG2`, `visibleG3`, `visibleG4`, `visibleG5`, `visibleG6`, `visibleG7`,"
		"`Ghp0`, `Ghp1`, `Ghp2`, `Ghp3`, `Ghp4`, `Ghp5`, `Ghp6`, `Ghp7`"
        	" FROM `%s` WHERE `castle_id`='%d'",guild_castle_db, castle_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);
		if (sql_row==NULL){
			mysql_free_result(sql_res);
			return 0;
		}
		
		gc->guild_id =  atoi (sql_row[1]);
		gc->economy = atoi (sql_row[2]);
		gc->defense = atoi (sql_row[3]);
		gc->triggerE = atoi (sql_row[4]);
		gc->triggerD = atoi (sql_row[5]);
		gc->nextTime = atoi (sql_row[6]);
		gc->payTime = atoi (sql_row[7]);
		gc->createTime = atoi (sql_row[8]);
		gc->visibleC = atoi (sql_row[9]);
		gc->visibleG0 = atoi (sql_row[10]);
		gc->visibleG1 = atoi (sql_row[11]);
		gc->visibleG2 = atoi (sql_row[12]);
		gc->visibleG3 = atoi (sql_row[13]);
		gc->visibleG4 = atoi (sql_row[14]);
		gc->visibleG5 = atoi (sql_row[15]);
		gc->visibleG6 = atoi (sql_row[16]);
		gc->visibleG7 = atoi (sql_row[17]);
		gc->Ghp0 = atoi (sql_row[18]);
		gc->Ghp1 = atoi (sql_row[19]);
		gc->Ghp2 = atoi (sql_row[20]);
		gc->Ghp3 = atoi (sql_row[21]);
		gc->Ghp4 = atoi (sql_row[22]);
		gc->Ghp5 = atoi (sql_row[23]);
		gc->Ghp6 = atoi (sql_row[24]);
		gc->Ghp7 = atoi (sql_row[25]);
		
		//printf("Read Castle %d of guild %d from sql \n",castle_id,gc->guild_id);

	}
	mysql_free_result(sql_res) ; //resource free
	return 0;
}

// Read exp_guild.txt
int inter_guild_readdb()
{
	int i;
	FILE *fp;
	char line[1024];
	for (i=0;i<100;i++) guild_exp[i]=0;
	
	fp=fopen("db/exp_guild.txt","r");
	if(fp==NULL){
		printf("can't read db/exp_guild.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line,256,fp) && i<100){
		if(line[0]=='/' && line[1]=='/')
			continue;
		guild_exp[i]=atoi(line);
		i++;
	}
	fclose(fp);

	return 0;
}


// Initialize guild sql
int inter_guild_sql_init()
{
	int i;
	
	printf("interserver guild memory initialize.... (%d byte)\n",sizeof(struct guild));
	guild_pt = calloc(sizeof(struct guild), 1);
	guild_pt2= calloc(sizeof(struct guild), 1);
	guildcastle_pt=calloc(sizeof(struct guild_castle), 1);
	
	inter_guild_readdb(); // Read exp
	
	sprintf(tmp_sql,"UPDATE `%s` SET `online`='0'",guild_member_db);	
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
		exit(0);
	}
	
	sprintf (tmp_sql , "SELECT count(*) FROM `%s`",guild_db);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		exit(0);
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	sql_row = mysql_fetch_row(sql_res);
	printf("total guild data -> '%s'.......\n",sql_row[0]);
	i = atoi (sql_row[0]);
	mysql_free_result(sql_res);

	if (i > 0) {
		//set party_newid
		sprintf (tmp_sql , "SELECT max(`guild_id`) FROM `%s`",guild_db);
		if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
			exit(0);
		}
		
		sql_res = mysql_store_result(&mysql_handle) ;
		sql_row = mysql_fetch_row(sql_res);
		guild_newid = atoi(sql_row[0])+1;
		mysql_free_result(sql_res);
	}
	
	printf("set guild_newid: %d.......\n",guild_newid);

	return 0;
}


// Get guild by its name
struct guild* search_guildname(char *str)
{
	struct guild *g=guild_pt;
	char t_name[24];
	int guild_id=0;
	printf("search_guildname\n");
	sprintf (tmp_sql , "SELECT `guild_id` FROM `%s` WHERE `name`='%s'",guild_db, jstrescapecpy(t_name,str));
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);
		guild_id = atoi (sql_row[0]);
	}
	mysql_free_result(sql_res);
	inter_guild_fromsql(guild_id,g);
	return g;
}

// Check if guild is empty
int guild_check_empty(struct guild *g)
{
	int i;
	for(i=0;i<g->max_member;i++){
		if(g->member[i].account_id>0){
			return 0;
		}
	}
	
	// 誰もいないので解散
	mapif_guild_broken(g->guild_id,0);
	inter_guild_storage_delete(g->guild_id);
	inter_guild_tosql(g,255);
	memset(g,0,sizeof(struct guild));
	return 1;
}

int guild_nextexp(int level)
{
	if(level < 100 && level >0) // Change by hack
		return guild_exp[level-1];

	return 0;
}

// ギルドスキルがあるか確認
int guild_checkskill(struct guild *g,int id) {

	int idx = id - GD_SKILLBASE;


	if(idx < 0 || idx >= MAX_GUILDSKILL)

		return 0;

	return g->skill[idx].lv;

}

// ギルドの情報の再計算
int guild_calcinfo(struct guild *g)
{
	int i,c,nextexp;
	struct guild before=*g;

	// スキルIDの設定
	for(i=0;i<MAX_GUILDSKILL;i++)
		g->skill[i].id=i+GD_SKILLBASE;

	// ギルドレベル
	if(g->guild_lv<=0) g->guild_lv=1;
	nextexp = guild_nextexp(g->guild_lv);
	if(nextexp > 0) {
		while(g->exp >= nextexp && nextexp>0){	// Change by hack
			g->exp-=nextexp;
			g->guild_lv++;
			g->skill_point++;
			nextexp = guild_nextexp(g->guild_lv);
		}
	}
	
	// ギルドの次の経験値
	g->next_exp = guild_nextexp(g->guild_lv);

	// メンバ上限（ギルド拡張適用）
	g->max_member = 16 + guild_checkskill(g, GD_EXTENSION) * 2; // Updated max_members [PoW]

	// 平均レベルとオンライン人数
	g->average_lv=0;
	g->connect_member=0;
	for(i=c=0;i<g->max_member;i++){
		if(g->member[i].account_id>0){
			g->average_lv+=g->member[i].lv;
			c++;
			
			if(g->member[i].online>0)
				g->connect_member++;
		}
	}
	if(c) g->average_lv/=c;
	
	// 全データを送る必要がありそう
	if(	g->max_member!=before.max_member	||
		g->guild_lv!=before.guild_lv		||
		g->skill_point!=before.skill_point	){
		mapif_guild_info(-1,g);
		return 1;
	}
		
	return 0;
}

//-------------------------------------------------------------------
// map serverへの通信

// ギルド作成可否
int mapif_guild_created(int fd,int account_id,struct guild *g)
{
	WFIFOW(fd,0)=0x3830;
	WFIFOL(fd,2)=account_id;
	if(g!=NULL){
		WFIFOL(fd,6)=g->guild_id;
		printf("int_guild: created! %d %s\n",g->guild_id,g->name);
	}else{
		WFIFOL(fd,6)=0;
	}
	WFIFOSET(fd,10);
	return 0;
}
// ギルド情報見つからず
int mapif_guild_noinfo(int fd,int guild_id)
{
	WFIFOW(fd,0)=0x3831;
	WFIFOW(fd,2)=8;
	WFIFOL(fd,4)=guild_id;
	WFIFOSET(fd,8);
	printf("int_guild: info not found %d\n",guild_id);
	return 0;
}
// ギルド情報まとめ送り
int mapif_guild_info(int fd,struct guild *g)
{
	unsigned char buf[16384];
	WBUFW(buf,0)=0x3831;
	memcpy(buf+4,g,sizeof(struct guild));
	WBUFW(buf,2)=4+sizeof(struct guild);
//	printf("int_guild: sizeof(guild)=%d\n",sizeof(struct guild));
	if(fd<0)
		mapif_sendall(buf,WBUFW(buf,2));
	else
		mapif_send(fd,buf,WBUFW(buf,2));
//	printf("int_guild: info %d %s\n",p->guild_id,p->name);
	return 0;
}

// メンバ追加可否
int mapif_guild_memberadded(int fd,int guild_id,int account_id,int char_id,int flag)
{
	WFIFOW(fd,0)=0x3832;
	WFIFOL(fd,2)=guild_id;
	WFIFOL(fd,6)=account_id;
	WFIFOL(fd,10)=char_id;
	WFIFOB(fd,14)=flag;
	WFIFOSET(fd,15);
	return 0;
}
// 脱退/追放通知
int mapif_guild_leaved(int guild_id,int account_id,int char_id,int flag,
	const char *name,const char *mes)
{
	unsigned char buf[128];
	WBUFW(buf, 0)=0x3834;
	WBUFL(buf, 2)=guild_id;
	WBUFL(buf, 6)=account_id;
	WBUFL(buf,10)=char_id;
	WBUFB(buf,14)=flag;
	memcpy(WBUFP(buf,15),mes,40);
	memcpy(WBUFP(buf,55),name,24);
	mapif_sendall(buf,79);
	printf("int_guild: guild leaved %d %d %s %s\n",guild_id,account_id,name,mes);
	return 0;
}

// オンライン状態とLv更新通知
int mapif_guild_memberinfoshort(struct guild *g,int idx)
{
	unsigned char buf[32];
	WBUFW(buf, 0)=0x3835;
	WBUFL(buf, 2)=g->guild_id;
	WBUFL(buf, 6)=g->member[idx].account_id;
	WBUFL(buf,10)=g->member[idx].char_id;
	WBUFB(buf,14)=g->member[idx].online;
	WBUFW(buf,15)=g->member[idx].lv;
	WBUFW(buf,17)=g->member[idx].class;
	mapif_sendall(buf,19);
	return 0;
}

// 解散通知
int mapif_guild_broken(int guild_id,int flag)
{
	unsigned char buf[16];
	WBUFW(buf,0)=0x3836;
	WBUFL(buf,2)=guild_id;
	WBUFB(buf,6)=flag;
	mapif_sendall(buf,7);
	printf("int_guild: broken %d\n",guild_id);
	return 0;
}

// ギルド内発言
int mapif_guild_message(int guild_id,int account_id,char *mes,int len)
{
	unsigned char buf[512];
	WBUFW(buf,0)=0x3837;
	WBUFW(buf,2)=len+12;
	WBUFL(buf,4)=guild_id;
	WBUFL(buf,8)=account_id;
	memcpy(WBUFP(buf,12),mes,len);
	mapif_sendall(buf,len+12);
	return 0;
}

// ギルド基本情報変更通知
int mapif_guild_basicinfochanged(int guild_id,int type,const void *data,int len)
{
	unsigned char buf[2048];
	WBUFW(buf, 0)=0x3839;
	WBUFW(buf, 2)=len+10;
	WBUFL(buf, 4)=guild_id;
	WBUFW(buf, 8)=type;
	memcpy(WBUFP(buf,10),data,len);
	mapif_sendall(buf,len+10);
	return 0;
}
// ギルドメンバ情報変更通知
int mapif_guild_memberinfochanged(int guild_id,int account_id,int char_id,
	int type,const void *data,int len)
{
	unsigned char buf[2048];
	WBUFW(buf, 0)=0x383a;
	WBUFW(buf, 2)=len+18;
	WBUFL(buf, 4)=guild_id;
	WBUFL(buf, 8)=account_id;
	WBUFL(buf,12)=char_id;
	WBUFW(buf,16)=type;
	memcpy(WBUFP(buf,18),data,len);
	mapif_sendall(buf,len+18);
	return 0;
}
// ギルドスキルアップ通知
int mapif_guild_skillupack(int guild_id,int skill_num,int account_id)
{
	unsigned char buf[16];
	WBUFW(buf, 0)=0x383c;
	WBUFL(buf, 2)=guild_id;
	WBUFL(buf, 6)=skill_num;
	WBUFL(buf,10)=account_id;
	mapif_sendall(buf,14);
	return 0;
}
// ギルド同盟/敵対通知
int mapif_guild_alliance(int guild_id1,int guild_id2,int account_id1,int account_id2,
	int flag,const char *name1,const char *name2)
{
	unsigned char buf[128];
	WBUFW(buf, 0)=0x383d;
	WBUFL(buf, 2)=guild_id1;
	WBUFL(buf, 6)=guild_id2;
	WBUFL(buf,10)=account_id1;
	WBUFL(buf,14)=account_id2;
	WBUFB(buf,18)=flag;
	memcpy(WBUFP(buf,19),name1,24);
	memcpy(WBUFP(buf,43),name2,24);
	mapif_sendall(buf,67);
	return 0;
}

// ギルド役職変更通知
int mapif_guild_position(struct guild *g,int idx)
{
	unsigned char buf[128];
	WBUFW(buf,0)=0x383b;
	WBUFW(buf,2)=sizeof(struct guild_position)+12;
	WBUFL(buf,4)=g->guild_id;
	WBUFL(buf,8)=idx;
	memcpy(WBUFP(buf,12),&g->position[idx],sizeof(struct guild_position));
	mapif_sendall(buf,WBUFW(buf,2));
	return 0;
}

// ギルド告知変更通知
int mapif_guild_notice(struct guild *g)
{
	unsigned char buf[256];
	WBUFW(buf,0)=0x383e;
	WBUFL(buf,2)=g->guild_id;
	memcpy(WBUFP(buf,6),g->mes1,60);
	memcpy(WBUFP(buf,66),g->mes2,120);
	mapif_sendall(buf,186);
	return 0;
}
// ギルドエンブレム変更通知
int mapif_guild_emblem(struct guild *g)
{
	unsigned char buf[2048];
	WBUFW(buf,0)=0x383f;
	WBUFW(buf,2)=g->emblem_len+12;
	WBUFL(buf,4)=g->guild_id;
	WBUFL(buf,8)=g->emblem_id;
	memcpy(WBUFP(buf,12),g->emblem_data,g->emblem_len);
	mapif_sendall(buf,WBUFW(buf,2));
	return 0;
}

int mapif_guild_castle_dataload(int castle_id,int index,int value)      // <Agit>
{
	unsigned char buf[16];
	WBUFW(buf, 0)=0x3840;
	WBUFW(buf, 2)=castle_id;
	WBUFB(buf, 4)=index;
	WBUFL(buf, 5)=value;
	mapif_sendall(buf,9);
	return 0;
}

int mapif_guild_castle_datasave(int castle_id,int index,int value)      // <Agit>
{
	unsigned char buf[16];
	WBUFW(buf, 0)=0x3841;
	WBUFW(buf, 2)=castle_id;
	WBUFB(buf, 4)=index;
	WBUFL(buf, 5)=value;
	mapif_sendall(buf,9);
	return 0;
}

int mapif_guild_castle_alldataload(int fd) {
	struct guild_castle* gc = guildcastle_pt;
	int i, len = 4;

	WFIFOW(fd,0) = 0x3842;
	sprintf(tmp_sql, "SELECT * FROM `%s` ORDER BY `castle_id`", guild_castle_db);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res != NULL && mysql_num_rows(sql_res) > 0) {
		for(i = 0; ((sql_row = mysql_fetch_row(sql_res)) && i < MAX_GUILDCASTLE); i++) {
			memset(gc, 0, sizeof(struct guild_castle));
			gc->castle_id = atoi(sql_row[0]);
			gc->guild_id =  atoi(sql_row[1]);
			gc->economy = atoi(sql_row[2]);
			gc->defense = atoi(sql_row[3]);
			gc->triggerE = atoi(sql_row[4]);
			gc->triggerD = atoi(sql_row[5]);
			gc->nextTime = atoi(sql_row[6]);
			gc->payTime = atoi(sql_row[7]);
			gc->createTime = atoi(sql_row[8]);
			gc->visibleC = atoi(sql_row[9]);
			gc->visibleG0 = atoi(sql_row[10]);
			gc->visibleG1 = atoi(sql_row[11]);
			gc->visibleG2 = atoi(sql_row[12]);
			gc->visibleG3 = atoi(sql_row[13]);
			gc->visibleG4 = atoi(sql_row[14]);
			gc->visibleG5 = atoi(sql_row[15]);
			gc->visibleG6 = atoi(sql_row[16]);
			gc->visibleG7 = atoi(sql_row[17]);
			gc->Ghp0 = atoi(sql_row[18]);
			gc->Ghp1 = atoi(sql_row[19]);
			gc->Ghp2 = atoi(sql_row[20]);
			gc->Ghp3 = atoi(sql_row[21]);
			gc->Ghp4 = atoi(sql_row[22]);
			gc->Ghp5 = atoi(sql_row[23]);
			gc->Ghp6 = atoi(sql_row[24]);
			gc->Ghp7 = atoi(sql_row[25]);
			memcpy(WFIFOP(fd,len), gc, sizeof(struct guild_castle));
			len += sizeof(struct guild_castle);
		}
	}
	mysql_free_result(sql_res);
	WFIFOW(fd,2) = len;
	WFIFOSET(fd,len);

	return 0;
}


//-------------------------------------------------------------------
// map serverからの通信


// ギルド作成要求
int mapif_parse_CreateGuild(int fd,int account_id,char *name,struct guild_member *master)
{
	struct guild *g;
	int i;
	
	printf("CreateGuild\n");
	g=search_guildname(name);
	if(g!=NULL&&g->guild_id>0){
		printf("int_guild: same name guild exists [%s]\n",name);
		mapif_guild_created(fd,account_id,NULL);
		return 0;
	}
	g=guild_pt;
	memset(g,0,sizeof(struct guild));
	g->guild_id=guild_newid++;
	memcpy(g->name,name,24);
	memcpy(g->master,master->name,24);
	memcpy(&g->member[0],master,sizeof(struct guild_member));
	
	g->position[0].mode=0x11;
	strcpy(g->position[                  0].name,"GuildMaster");
	strcpy(g->position[MAX_GUILDPOSITION-1].name,"Newbie");
	for(i=1;i<MAX_GUILDPOSITION-1;i++)
		sprintf(g->position[i].name,"Position %d",i+1);
	
	// Initialize guild property
	g->max_member=16;
	g->average_lv=master->lv;
	g->castle_id=-1;
	for(i=0;i<MAX_GUILDSKILL;i++)
		g->skill[i].id=i + GD_SKILLBASE;
	
	// Save to sql
	printf("Create initialize OK!\n");
	i=inter_guild_tosql(g,255);
	
	if (i<0) {
		mapif_guild_created(fd,account_id,NULL);
		return 0;
	}
	
	// Report to client
	mapif_guild_created(fd,account_id,g);
	mapif_guild_info(fd,g);
	
	if(log_inter)
		inter_log("guild %s (id=%d) created by master %s (id=%d)" RETCODE,
			name, g->guild_id, master->name, master->account_id );
	
	
	return 0;
}
// Return guild info to client
int mapif_parse_GuildInfo(int fd,int guild_id)
{
	struct guild *g;
	g=guild_pt;
	inter_guild_fromsql(guild_id,g);
	if(g!=NULL&&g->guild_id>0){
		guild_calcinfo(g);
		mapif_guild_info(fd,g);
		//inter_guild_tosql(g,1); // Change guild
	}else
		mapif_guild_noinfo(fd,guild_id);
	return 0;
}
// Add member to guild
int mapif_parse_GuildAddMember(int fd,int guild_id,struct guild_member *m)
{
	struct guild *g=guild_pt;
	int i;

	inter_guild_fromsql(guild_id,g);
	
	if(g==NULL||g->guild_id<=0){
		mapif_guild_memberadded(fd,guild_id,m->account_id,m->char_id,1);
		return 0;
	}
	
	for(i=0;i<g->max_member;i++){
		if(g->member[i].account_id==0){
			
			memcpy(&g->member[i],m,sizeof(struct guild_member));
			mapif_guild_memberadded(fd,guild_id,m->account_id,m->char_id,0);
			guild_calcinfo(g);
			mapif_guild_info(-1,g);
			inter_guild_tosql(g,3); // Change guild & guild_member
			return 0;
		}
	}
	mapif_guild_memberadded(fd,guild_id,m->account_id,m->char_id,1);
	//inter_guild_tosql(g,3); // Change guild & guild_member
	return 0;
}
// Delete member from guild
int mapif_parse_GuildLeave(int fd,int guild_id,int account_id,int char_id,int flag,const char *mes)
{
	struct guild *g=guild_pt;

	inter_guild_fromsql(guild_id,g);
	
	if(g!=NULL&&g->guild_id>0){
		int i;
		for(i=0;i<g->max_member;i++){
			if( g->member[i].account_id==account_id &&
				g->member[i].char_id==char_id){
				printf("%d %d\n",i, (int)(&g->member[i]));
				printf("%d %s\n",i, g->member[i].name);
				
				if(flag){	// 追放の場合追放リストに入れる
					int j;
					for(j=0;j<MAX_GUILDEXPLUSION;j++){
						if(g->explusion[j].account_id==0)
							break;
					}
					if(j==MAX_GUILDEXPLUSION){	// 一杯なので古いのを消す
						for(j=0;j<MAX_GUILDEXPLUSION-1;j++)
							g->explusion[j]=g->explusion[j+1];
						j=MAX_GUILDEXPLUSION-1;
					}
					g->explusion[j].account_id=account_id;
					memcpy(g->explusion[j].acc,"dummy",24);
					memcpy(g->explusion[j].name,g->member[i].name,24);
					memcpy(g->explusion[j].mes,mes,40);
				}
				
				mapif_guild_leaved(guild_id,account_id,char_id,flag,g->member[i].name,mes);
				printf("%d %d\n",i, (int)(&g->member[i]));
				printf("%d %s\n",i, (&g->member[i])->name);
				memset(&g->member[i],0,sizeof(struct guild_member));
				
				if( guild_check_empty(g)==0 )
					mapif_guild_info(-1,g);// まだ人がいるのでデータ送信
				/*
				else
					inter_guild_save();	// 解散したので一応セーブ
				return 0;*/
			}
		}
		guild_calcinfo(g);
		inter_guild_tosql(g,19); // Change guild & guild_member & guild_expulsion
	}else{
		sprintf(tmp_sql, "UPDATE `%s` SET `guild_id`='0' WHERE `account_id`='%d' AND `char_id`='%d'",char_db, account_id,char_id);
		if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle) );
		}
		/* mapif_guild_leaved(guild_id,account_id,char_id,flag,g->member[i].name,mes);	*/
	}
	
	
	return 0;
}
// Change member info
int mapif_parse_GuildChangeMemberInfoShort(int fd,int guild_id,
	int account_id,int char_id,int online,int lv,int class)
{
	// Could speed up by manipulating only guild_member
	struct guild * g=guild_pt;
	int i,alv,c;




	
	if(g==NULL||g->guild_id<=0)
		return 0;

	inter_guild_fromsql(guild_id,g);
	
	g->connect_member=0;
	
	for(i=0,alv=0,c=0;i<g->max_member;i++){
		if(	g->member[i].account_id==account_id &&
			g->member[i].char_id==char_id){
			
			g->member[i].online=online;
			g->member[i].lv=lv;
			g->member[i].class=class;
			mapif_guild_memberinfoshort(g,i);
		}
		if( g->member[i].account_id>0 ){
			alv+=g->member[i].lv;
			c++;
		}
		if( g->member[i].online )
			g->connect_member++;
	}
	// 平均レベル
	g->average_lv=alv/c;
	
	inter_guild_tosql(g,3); // Change guild & guild_member
	
	return 0;
}

// BreakGuild
int mapif_parse_BreakGuild(int fd,int guild_id)
{
	struct guild *g=guild_pt;
	if(g==NULL)
		return 0;
	inter_guild_fromsql(guild_id,g);



	
	// Delete guild from sql
	//printf("- Delete guild %d from guild\n",guild_id);
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild`)- %s\n", mysql_error(&mysql_handle) );
	}
	//printf("- Delete guild %d from guild_member\n",guild_id);
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_member_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild_member`)- %s\n", mysql_error(&mysql_handle) );
	}
	//printf("- Delete guild %d from guild_skill\n",guild_id);
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_skill_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild_skill`)- %s\n", mysql_error(&mysql_handle) );
	}
	//printf("- Delete guild %d from guild_position\n",guild_id);
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_position_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild_position`)- %s\n", mysql_error(&mysql_handle) );
	}
	//printf("- Delete guild %d from guild_expulsion\n",guild_id);
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_expulsion_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild_expulsion`)- %s\n", mysql_error(&mysql_handle) );
	}
	//printf("- Delete guild %d from guild_alliance\n",guild_id);
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d' OR `alliance_id`='%d'",guild_alliance_db, guild_id,guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild_position`)- %s\n", mysql_error(&mysql_handle) );
	}
	
	//printf("- Delete guild %d from guild_castle\n",guild_id);
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_castle_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild_position`)- %s\n", mysql_error(&mysql_handle) );
	}
	
	//printf("- Update guild %d of char\n",guild_id);
	sprintf(tmp_sql, "UPDATE `%s` SET `guild_id`='0' WHERE `guild_id`='%d'",char_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild_position`)- %s\n", mysql_error(&mysql_handle) );
	}
		
	inter_guild_storage_delete(guild_id);
	mapif_guild_broken(guild_id,0);
	
	if(log_inter)
		inter_log("guild %s (id=%d) broken" RETCODE,g->name,guild_id);
	
	return 0;
}

// ギルドメッセージ送信
int mapif_parse_GuildMessage(int fd,int guild_id,int account_id,char *mes,int len)
{
	return mapif_guild_message(guild_id,account_id,mes,len);
}
// ギルド基本データ変更要求
int mapif_parse_GuildBasicInfoChange(int fd,int guild_id,
	int type,const char *data,int len)
{
	struct guild * g=guild_pt;
//	int dd=*((int *)data);
	short dw=*((short *)data);




	
	if(g==NULL||g->guild_id<=0)
		return 0;
	inter_guild_fromsql(guild_id,g);
	switch(type){
	case GBI_GUILDLV: {
		printf("GBI_GUILDLV\n");
			if(dw>0 && g->guild_lv+dw<=50){
				g->guild_lv+=dw;
				g->skill_point+=dw;
			}else if(dw<0 && g->guild_lv+dw>=1)
				g->guild_lv+=dw;
			mapif_guild_info(-1,g);
			inter_guild_tosql(g,1);
		} return 0;
	default:
		printf("int_guild: GuildBasicInfoChange: Unknown type %d\n",type);
		break;
	}
	mapif_guild_basicinfochanged(guild_id,type,data,len);
	//inter_guild_tosql(g,1); // Change guild
	return 0;
}

// ギルドメンバデータ変更要求
int mapif_parse_GuildMemberInfoChange(int fd,int guild_id,int account_id,int char_id,
	int type,const char *data,int len)
{
	// Could make some improvement in speed, because only change guild_member
	int i;
	struct guild * g=guild_pt;

	inter_guild_fromsql(guild_id,g);
	//printf("GuildMemberInfoChange %s \n",(type==GMI_EXP)?"GMI_EXP":"OTHER");
	
	if(g==NULL){
		return 0;
	}
	for(i=0;i<g->max_member;i++)
		if(	g->member[i].account_id==account_id &&
			g->member[i].char_id==char_id )
				break;
	if(i==g->max_member){
		printf("int_guild: GuildMemberChange: Not found %d,%d in %d[%s]\n",
			account_id,char_id,guild_id,g->name);
		return 0;
	}
	switch(type){
	case GMI_POSITION:	// 役職
		g->member[i].position=*((int *)data);
		break;
	case GMI_EXP:	{	// EXP
			int exp,oldexp=g->member[i].exp;
			exp=g->member[i].exp=*((unsigned int *)data);
			g->exp+=(exp-oldexp);
			guild_calcinfo(g);	// Lvアップ判断
			mapif_guild_basicinfochanged(guild_id,GBI_EXP,&g->exp,4);
		}break;
	default:
		printf("int_guild: GuildMemberInfoChange: Unknown type %d\n",type);
		break;
	}
	mapif_guild_memberinfochanged(guild_id,account_id,char_id,type,data,len);
	inter_guild_tosql(g,3); // Change guild & guild_member
	return 0;
}

// ギルド役職名変更要求
int mapif_parse_GuildPosition(int fd,int guild_id,int idx,struct guild_position *p)
{
	// Could make some improvement in speed, because only change guild_position
	struct guild * g=guild_pt;

	inter_guild_fromsql(guild_id,g);	

	if(g==NULL || idx<0 || idx>=MAX_GUILDPOSITION){
		return 0;
	}
	memcpy(&g->position[idx],p,sizeof(struct guild_position));
	mapif_guild_position(g,idx);
	printf("int_guild: position changed %d\n",idx);
	inter_guild_tosql(g,4); // Change guild_position
	return 0;
}
// ギルドスキルアップ要求
int mapif_parse_GuildSkillUp(int fd,int guild_id,int skill_num,int account_id)
{
	// Could make some improvement in speed, because only change guild_position
	struct guild *g=guild_pt;
	int idx = skill_num - GD_SKILLBASE;

	inter_guild_fromsql(guild_id,g);

	if(g == NULL || idx < 0 || idx >= MAX_GUILDSKILL)
		return 0;
	//printf("GuildSkillUp\n");
	
	if(	g->skill_point>0 && g->skill[idx].id>0 &&
		g->skill[idx].lv<10 ){
		g->skill[idx].lv++;
		g->skill_point--;
		if(guild_calcinfo(g)==0)
			mapif_guild_info(-1,g);
		mapif_guild_skillupack(guild_id,skill_num,account_id);
		printf("int_guild: skill %d up\n",skill_num);
		inter_guild_tosql(g,33); // Change guild & guild_skill
	}

	return 0;
}
// ギルド同盟要求
int mapif_parse_GuildAlliance(int fd,int guild_id1,int guild_id2,
	int account_id1,int account_id2,int flag)
{
	// Could speed up
	struct guild *g[2];
	int j,i;
	g[0]=guild_pt;
	g[1]=guild_pt2;
	inter_guild_fromsql(guild_id1,g[0]);
	inter_guild_fromsql(guild_id2,g[1]);
	
	if(g[0]==NULL || g[1]==NULL || g[0]->guild_id ==0 || g[1]->guild_id==0)
		return 0;
		
	if(!(flag&0x8)){
		for(i=0;i<2-(flag&1);i++){
			for(j=0;j<MAX_GUILDALLIANCE;j++)
				if(g[i]->alliance[j].guild_id==0){
					g[i]->alliance[j].guild_id=g[1-i]->guild_id;
					memcpy(g[i]->alliance[j].name,g[1-i]->name,24);
					g[i]->alliance[j].opposition=flag&1;
					break;
				}
		}
	}else{	// 関係解消
		for(i=0;i<2-(flag&1);i++){
			for(j=0;j<MAX_GUILDALLIANCE;j++)
				if(	g[i]->alliance[j].guild_id==g[1-i]->guild_id &&
					g[i]->alliance[j].opposition==(flag&1)){
					g[i]->alliance[j].guild_id=0;
					break;
				}
		}
	}
	mapif_guild_alliance(guild_id1,guild_id2,account_id1,account_id2,flag,
		g[0]->name,g[1]->name);
	inter_guild_tosql(g[0],8); // Change guild_alliance
	inter_guild_tosql(g[1],8); // Change guild_alliance
	return 0;
}
// ギルド告知変更要求
int mapif_parse_GuildNotice(int fd,int guild_id,const char *mes1,const char *mes2)
{
	struct guild *g=guild_pt;

	inter_guild_fromsql(guild_id,g);
	
	if(g==NULL||g->guild_id<=0)
		return 0;
	memcpy(g->mes1,mes1,60);
	memcpy(g->mes2,mes2,120);
	inter_guild_tosql(g,1); // Change mes of guild
	return mapif_guild_notice(g);
}
// ギルドエンブレム変更要求
int mapif_parse_GuildEmblem(int fd,int len,int guild_id,int dummy,const char *data)
{
	struct guild * g=guild_pt;

	inter_guild_fromsql(guild_id,g);
	
	if(g==NULL||g->guild_id<=0)
		return 0;
	memcpy(g->emblem_data,data,len);
	g->emblem_len=len;
	g->emblem_id++;
	inter_guild_tosql(g,1); // Change guild
	return mapif_guild_emblem(g);
}

int mapif_parse_GuildCastleDataLoad(int fd,int castle_id,int index)     // <Agit>
{
	struct guild_castle *gc=guildcastle_pt;
	inter_guildcastle_fromsql(castle_id, gc);
	if(gc==NULL||gc->castle_id==-1){
		return mapif_guild_castle_dataload(castle_id,0,0);
	}
	switch(index){
	case 1: return mapif_guild_castle_dataload(gc->castle_id,index,gc->guild_id); break;
	case 2: return mapif_guild_castle_dataload(gc->castle_id,index,gc->economy); break;
	case 3: return mapif_guild_castle_dataload(gc->castle_id,index,gc->defense); break;
	case 4: return mapif_guild_castle_dataload(gc->castle_id,index,gc->triggerE); break;
	case 5: return mapif_guild_castle_dataload(gc->castle_id,index,gc->triggerD); break;
	case 6: return mapif_guild_castle_dataload(gc->castle_id,index,gc->nextTime); break;
	case 7: return mapif_guild_castle_dataload(gc->castle_id,index,gc->payTime); break;
	case 8: return mapif_guild_castle_dataload(gc->castle_id,index,gc->createTime); break;
	case 9: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleC); break;
	case 10: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleG0); break;
	case 11: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleG1); break;
	case 12: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleG2); break;
	case 13: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleG3); break;
	case 14: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleG4); break;
	case 15: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleG5); break;
	case 16: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleG6); break;
	case 17: return mapif_guild_castle_dataload(gc->castle_id,index,gc->visibleG7); break;
	case 18: return mapif_guild_castle_dataload(gc->castle_id,index,gc->Ghp0); break;	// guardian HP [Valaris]
	case 19: return mapif_guild_castle_dataload(gc->castle_id,index,gc->Ghp1); break;
	case 20: return mapif_guild_castle_dataload(gc->castle_id,index,gc->Ghp2); break;
	case 21: return mapif_guild_castle_dataload(gc->castle_id,index,gc->Ghp3); break;
	case 22: return mapif_guild_castle_dataload(gc->castle_id,index,gc->Ghp4); break;
	case 23: return mapif_guild_castle_dataload(gc->castle_id,index,gc->Ghp5); break;
	case 24: return mapif_guild_castle_dataload(gc->castle_id,index,gc->Ghp6); break;
	case 25: return mapif_guild_castle_dataload(gc->castle_id,index,gc->Ghp7); break;	// end additions [Valaris]
	default:
		printf("mapif_parse_GuildCastleDataLoad ERROR!! (Not found index=%d)\n", index);
		return 0;
	}
}

int mapif_parse_GuildCastleDataSave(int fd,int castle_id,int index,int value)   // <Agit>
{
	struct guild_castle *gc=guildcastle_pt;
	inter_guildcastle_fromsql(castle_id, gc);
	if(gc==NULL||gc->castle_id==-1){
		return mapif_guild_castle_datasave(castle_id,index,value);
	}
	switch(index){
	case 1:
		if( gc->guild_id!=value ){
			int gid=(value)?value:gc->guild_id;
			struct guild *g=guild_pt;
			inter_guild_fromsql(gid, g);
			if(log_inter)
				inter_log("guild %s (id=%d) %s castle id=%d" RETCODE,
					(g)?g->name:"??" ,gid, (value)?"occupy":"abandon", index);
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
	inter_guildcastle_tosql(gc);
	return mapif_guild_castle_datasave(gc->castle_id,index,value);
}

// ギルドチェック要求
int mapif_parse_GuildCheck(int fd,int guild_id,int account_id,int char_id)
{
	// What does this mean? Check if belong to another guild?
	return 0;
}

// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_guild_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0)){
	case 0x3030: mapif_parse_CreateGuild(fd,RFIFOL(fd,4),RFIFOP(fd,8),(struct guild_member *)RFIFOP(fd,32)); break;
	case 0x3031: mapif_parse_GuildInfo(fd,RFIFOL(fd,2)); break;
	case 0x3032: mapif_parse_GuildAddMember(fd,RFIFOL(fd,4),(struct guild_member *)RFIFOP(fd,8)); break;
	case 0x3034: mapif_parse_GuildLeave(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),RFIFOP(fd,15)); break;
	case 0x3035: mapif_parse_GuildChangeMemberInfoShort(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOB(fd,14),RFIFOW(fd,15),RFIFOW(fd,17)); break;
	case 0x3036: mapif_parse_BreakGuild(fd,RFIFOL(fd,2)); break;
	case 0x3037: mapif_parse_GuildMessage(fd,RFIFOL(fd,4),RFIFOL(fd,8),RFIFOP(fd,12),RFIFOW(fd,2)-12); break;
	case 0x3038: mapif_parse_GuildCheck(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10)); break;
	case 0x3039: mapif_parse_GuildBasicInfoChange(fd,RFIFOL(fd,4),RFIFOW(fd,8),RFIFOP(fd,10),RFIFOW(fd,2)-10); break;
	case 0x303A: mapif_parse_GuildMemberInfoChange(fd,RFIFOL(fd,4),RFIFOL(fd,8),RFIFOL(fd,12),RFIFOW(fd,16),RFIFOP(fd,18),RFIFOW(fd,2)-18); break;
	case 0x303B: mapif_parse_GuildPosition(fd,RFIFOL(fd,4),RFIFOL(fd,8),(struct guild_position *)RFIFOP(fd,12)); break;
	case 0x303C: mapif_parse_GuildSkillUp(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10)); break;
	case 0x303D: mapif_parse_GuildAlliance(fd,RFIFOL(fd,2),RFIFOL(fd,6),RFIFOL(fd,10),RFIFOL(fd,14),RFIFOB(fd,18)); break;
	case 0x303E: mapif_parse_GuildNotice(fd,RFIFOL(fd,2),RFIFOP(fd,6),RFIFOP(fd,66)); break;
	case 0x303F: mapif_parse_GuildEmblem(fd,RFIFOW(fd,2)-12,RFIFOL(fd,4),RFIFOL(fd,8),RFIFOP(fd,12)); break;
	case 0x3040: mapif_parse_GuildCastleDataLoad(fd,RFIFOW(fd,2),RFIFOB(fd,4)); break;
	case 0x3041: mapif_parse_GuildCastleDataSave(fd,RFIFOW(fd,2),RFIFOB(fd,4),RFIFOL(fd,5)); break;

	default:
		return 0;
	}
	return 1;
}

int inter_guild_mapif_init(int fd)
{
	return mapif_guild_castle_alldataload(fd);
}

// サーバーから脱退要求（キャラ削除用）
int inter_guild_leave(int guild_id,int account_id,int char_id)
{
	return mapif_parse_GuildLeave(-1,guild_id,account_id,char_id,0,"**サーバー命令**");
}
