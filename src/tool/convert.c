// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RETCODE	"\r\n"

#define MAX_INVENTORY 100
#define MAX_CART 100
#define MAX_SKILL 350
#define GLOBAL_REG_NUM 16

struct item {
	int id;
	short nameid;
	short amount;
	short equip;
	char identify;
	char refine;
	char attribute;
	short card[4];
};
struct point{
	char map[16];
	short x,y;
};
struct skill {
	unsigned short id,lv,flag;
};
struct global_reg {
	char str[16];
	int value;
};

struct mmo_charstatus {
	int char_id;
	int account_id;
	int base_exp,job_exp,zeny;

	short class;
	short status_point,skill_point;
	short hp,max_hp,sp,max_sp;
	short option,karma,manner;
	short hair,hair_color,clothes_color;
	int party_id,guild_id,pet_id;

	short weapon,shield;
	short head_top,head_mid,head_bottom;

	char name[24];
	unsigned char base_level,job_level;
	unsigned char str,agi,vit,int_,dex,luk,char_num,sex;

	struct point last_point,save_point,memo_point[3];
	struct item inventory[MAX_INVENTORY],cart[MAX_CART];
	struct skill skill[MAX_SKILL];
	int global_reg_num;
	struct global_reg global_reg[GLOBAL_REG_NUM];
};

int mmo_char_tostr(char *str,struct mmo_charstatus *p)
{
  int i;
  sprintf(str,"%d\t%d,%d\t%s\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
	  "\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
	  "\t%s,%d,%d\t%s,%d,%d",
	  p->char_id,p->account_id,p->char_num,p->name, //
	  p->class,p->base_level,p->job_level,
	  p->base_exp,p->job_exp,p->zeny,
	  p->hp,p->max_hp,p->sp,p->max_sp,
	  p->str,p->agi,p->vit,p->int_,p->dex,p->luk,
	  p->status_point,p->skill_point,
	  p->option,p->karma,p->manner,	//
	  p->party_id,p->guild_id,p->pet_id,
	  p->hair,p->hair_color,p->clothes_color,
	  p->weapon,p->shield,p->head_top,p->head_mid,p->head_bottom,
	  p->last_point.map,p->last_point.x,p->last_point.y, //
	  p->save_point.map,p->save_point.x,p->save_point.y
	  );
  strcat(str,"\t");
  for(i=0;i<3;i++)
    if(p->memo_point[i].map[0]){
      sprintf(str+strlen(str),"%s,%d,%d",p->memo_point[i].map,p->memo_point[i].x,p->memo_point[i].y);
    }      
  strcat(str,"\t");
  for(i=0;i<MAX_INVENTORY;i++)
    if(p->inventory[i].nameid){
      sprintf(str+strlen(str),"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
	      p->inventory[i].id,p->inventory[i].nameid,p->inventory[i].amount,p->inventory[i].equip,
	      p->inventory[i].identify,p->inventory[i].refine,p->inventory[i].attribute,
	      p->inventory[i].card[0],p->inventory[i].card[1],p->inventory[i].card[2],p->inventory[i].card[3]);
    }      
  strcat(str,"\t");
  for(i=0;i<MAX_CART;i++)
    if(p->cart[i].nameid){
      sprintf(str+strlen(str),"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d ",
	      p->cart[i].id,p->cart[i].nameid,p->cart[i].amount,p->cart[i].equip,
	      p->cart[i].identify,p->cart[i].refine,p->cart[i].attribute,
	      p->cart[i].card[0],p->cart[i].card[1],p->cart[i].card[2],p->cart[i].card[3]);
    }      
  strcat(str,"\t");
  for(i=0;i<MAX_SKILL;i++)
    if(p->skill[i].id){
      sprintf(str+strlen(str),"%d,%d ",p->skill[i].id,p->skill[i].lv);
    }      
  strcat(str,"\t");
  for(i=0;i<p->global_reg_num;i++)
    sprintf(str+strlen(str),"%s,%d ",p->global_reg[i].str,p->global_reg[i].value);
  strcat(str,"\t");
  return 0;
}

int mmo_char_fromstr(char *str,struct mmo_charstatus *p)
{
  int tmp_int[256];
  int set,next,len,i;

  set=sscanf(str,"%d\t%d,%d\t%[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
	   "\t%d,%d,%d\t%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
	   "\t%[^,],%d,%d\t%[^,],%d,%d%n",
	   &tmp_int[0],&tmp_int[1],&tmp_int[2],p->name, //
	   &tmp_int[3],&tmp_int[4],&tmp_int[5],
	   &tmp_int[6],&tmp_int[7],&tmp_int[8],
	   &tmp_int[9],&tmp_int[10],&tmp_int[11],&tmp_int[12],
	   &tmp_int[13],&tmp_int[14],&tmp_int[15],&tmp_int[16],&tmp_int[17],&tmp_int[18],
	   &tmp_int[19],&tmp_int[20],
	   &tmp_int[21],&tmp_int[22],&tmp_int[23], //
	   &tmp_int[24],&tmp_int[25],
	   &tmp_int[26],&tmp_int[27],&tmp_int[28],
	   &tmp_int[29],&tmp_int[30],&tmp_int[31],&tmp_int[32],&tmp_int[33],
	   p->last_point.map,&tmp_int[34],&tmp_int[35], //
	   p->save_point.map,&tmp_int[36],&tmp_int[37],&next
	 );
  p->char_id=tmp_int[0];
  p->account_id=tmp_int[1];
  p->char_num=tmp_int[2];
  p->class=tmp_int[3];
  p->base_level=tmp_int[4];
  p->job_level=tmp_int[5];
  p->base_exp=tmp_int[6];
  p->job_exp=tmp_int[7];
  p->zeny=tmp_int[8];
  p->hp=tmp_int[9];
  p->max_hp=tmp_int[10];
  p->sp=tmp_int[11];
  p->max_sp=tmp_int[12];
  p->str=tmp_int[13];
  p->agi=tmp_int[14];
  p->vit=tmp_int[15];
  p->int_=tmp_int[16];
  p->dex=tmp_int[17];
  p->luk=tmp_int[18];
  p->status_point=tmp_int[19];
  p->skill_point=tmp_int[20];
  p->option=tmp_int[21];
  p->karma=tmp_int[22];
  p->manner=tmp_int[23];
  p->party_id=tmp_int[24];
  p->guild_id=tmp_int[25];
  p->pet_id=0;
  p->hair=tmp_int[26];
  p->hair_color=tmp_int[27];
  p->clothes_color=tmp_int[28];
  p->weapon=tmp_int[29];
  p->shield=tmp_int[30];
  p->head_top=tmp_int[31];
  p->head_mid=tmp_int[32];
  p->head_bottom=tmp_int[33];
  p->last_point.x=tmp_int[34];
  p->last_point.y=tmp_int[35];
  p->save_point.x=tmp_int[36];
  p->save_point.y=tmp_int[37];
  if(set!=41)
    return 0;
  if(str[next]=='\n' || str[next]=='\r')
    return 1;	// 新規データ
  next++;
  for(i=0;str[next] && str[next]!='\t';i++){
    set=sscanf(str+next,"%[^,],%d,%d%n",p->memo_point[i].map,&tmp_int[0],&tmp_int[1],&len);
    if(set!=3) 
      return 0;
    p->memo_point[i].x=tmp_int[0];
    p->memo_point[i].y=tmp_int[1];
    next+=len;
    if(str[next]==' ')
      next++;
  }
  next++;
  for(i=0;str[next] && str[next]!='\t';i++){
    set=sscanf(str+next,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
	       &tmp_int[0],&tmp_int[1],&tmp_int[2],&tmp_int[3],
	       &tmp_int[4],&tmp_int[5],&tmp_int[6],
	       &tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10],&len);
    if(set!=11)
      return 0;
    p->inventory[i].id=tmp_int[0];
    p->inventory[i].nameid=tmp_int[1];
    p->inventory[i].amount=tmp_int[2];
    p->inventory[i].equip=tmp_int[3];
    p->inventory[i].identify=tmp_int[4];
    p->inventory[i].refine=tmp_int[5];
    p->inventory[i].attribute=tmp_int[6];
    p->inventory[i].card[0]=tmp_int[7];
    p->inventory[i].card[1]=tmp_int[8];
    p->inventory[i].card[2]=tmp_int[9];
    p->inventory[i].card[3]=tmp_int[10];
    next+=len;
    if(str[next]==' ')
      next++;
  }
  next++;
  for(i=0;str[next] && str[next]!='\t';i++){
    set=sscanf(str+next,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
	       &tmp_int[0],&tmp_int[1],&tmp_int[2],&tmp_int[3],
	       &tmp_int[4],&tmp_int[5],&tmp_int[6],
	       &tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10],&len);
    if(set!=11)
      return 0;
    p->cart[i].id=tmp_int[0];
    p->cart[i].nameid=tmp_int[1];
    p->cart[i].amount=tmp_int[2];
    p->cart[i].equip=tmp_int[3];
    p->cart[i].identify=tmp_int[4];
    p->cart[i].refine=tmp_int[5];
    p->cart[i].attribute=tmp_int[6];
    p->cart[i].card[0]=tmp_int[7];
    p->cart[i].card[1]=tmp_int[8];
    p->cart[i].card[2]=tmp_int[9];
    p->cart[i].card[3]=tmp_int[10];
    next+=len;
    if(str[next]==' ')
      next++;
  }
  next++;
  for(i=0;str[next] && str[next]!='\t';i++){
    set=sscanf(str+next,"%d,%d%n",
	       &tmp_int[0],&tmp_int[1],&len);
    if(set!=2)
      return 0;
    p->skill[tmp_int[0]].id=tmp_int[0];
    p->skill[tmp_int[0]].lv=tmp_int[1];
    next+=len;
    if(str[next]==' ')
      next++;
  }
  next++;
  for(i=0;str[next] && str[next]!='\t' && str[next]!='\n' && str[next]!='\r';i++){ //global_reg実装以前のathena.txt互換のため一応'\n'チェック
    set=sscanf(str+next,"%[^,],%d%n",
	       p->global_reg[i].str,&p->global_reg[i].value,&len);
    if(set!=2)
      return 0;
    next+=len;
    if(str[next]==' ')
      next++;
  }
  p->global_reg_num=i;
  return 1;
}

int mmo_char_convert(char *fname1,char *fname2)
{
  char line[65536];
  int ret;
	struct mmo_charstatus char_dat;
  FILE *ifp,*ofp;

	ifp=fopen(fname1,"r");
	ofp=fopen(fname2,"w");
  if(ifp==NULL) {
  	printf("file not found %s\n",fname1);
    return 0;
  }
  if(ofp==NULL) {
  	printf("file open error %s\n",fname2);
    return 0;
  }
  while(fgets(line,65535,ifp)){
    memset(&char_dat,0,sizeof(struct mmo_charstatus));
    ret=mmo_char_fromstr(line,&char_dat);
    if(ret){
	    mmo_char_tostr(line,&char_dat);
  	  fprintf(ofp,"%s" RETCODE,line);
    }
  }
  fcloseall();
  return 0;
}

int main(int argc,char *argv[])
{
	if(argc < 3) {
		printf("Usage: convert <input filename> <output filename>\n");
		exit(0);
	}
	mmo_char_convert(argv[1],argv[2]);

	return 0;
}
