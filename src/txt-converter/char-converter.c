// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/mapindex.h"
#include "../common/utils.h"

#include "../char/char.h"
#include "../char/int_storage.h"
#include "../char/int_pet.h"
#include "../char/int_party.h"
#include "../char/int_guild.h"
#include "../char/inter.h"

#include "../char_sql/char.h"
#include "../char_sql/int_storage.h"
#include "../char_sql/int_pet.h"
#include "../char_sql/int_party.h"
#include "../char_sql/int_guild.h"
#include "../char_sql/inter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_CONF_NAME "conf/char_athena.conf"
#define SQL_CONF_NAME "conf/inter_athena.conf"
#define INTER_CONF_NAME "conf/inter_athena.conf"
//--------------------------------------------------------

int convert_init(void)
{
	char line[65536];
	int ret;
	int set,tmp_int[2], lineno, count;
	char input;
	FILE *fp;

	ShowWarning("Make sure you backup your databases before continuing!\n");
	ShowMessage("\n");

	ShowNotice("Do you wish to convert your Character Database to SQL? (y/n) : ");
	input = getchar();
	if(input == 'y' || input == 'Y')
	{
		struct character_data char_dat;
		struct accreg reg;

		ShowStatus("Converting Character Database...\n");
		if( (fp = fopen(char_txt, "r")) == NULL )
		{
			ShowError("Unable to open file [%s]!\n", char_txt);
			return 0;
		}
		lineno = count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			lineno++;
			memset(&char_dat, 0, sizeof(struct character_data));
			ret=mmo_char_fromstr(line, &char_dat.status, char_dat.global, &char_dat.global_num);
			if(ret > 0) {
				count++;
				parse_friend_txt(&char_dat.status); //Retrieve friends.
				mmo_char_tosql(char_dat.status.char_id , &char_dat.status);

				memset(&reg, 0, sizeof(reg));
				reg.account_id = char_dat.status.account_id;
				reg.char_id = char_dat.status.char_id;
				reg.reg_num = char_dat.global_num;
				memcpy(&reg.reg, &char_dat.global, reg.reg_num*sizeof(struct global_reg));
				inter_accreg_tosql(reg.account_id, reg.char_id, &reg, 3); //Type 3: Character regs
			} else {
				ShowError("Error %d converting character line [%s] (at %s:%d).\n", ret, line, char_txt, lineno);
			}
		}
		ShowStatus("Converted %d characters.\n", count);
		fclose(fp);
		ShowStatus("Converting Account variables Database...\n");
		if( (fp = fopen(accreg_txt, "r")) == NULL )
		{
			ShowError("Unable to open file %s!", accreg_txt);
			return 1;
		}
		lineno = count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			lineno++;
			memset (&reg, 0, sizeof(struct accreg));
			if(inter_accreg_fromstr(line, &reg) == 0 && reg.account_id > 0) {
				count++;
				inter_accreg_tosql(reg.account_id, 0, &reg, 2); //Type 2: Account regs
			} else {
				ShowError("accreg reading: broken data [%s] at %s:%d\n", line, accreg_txt, lineno);
			}
		}
		ShowStatus("Converted %d account registries.\n", count);
		fclose(fp);
	}
	
	while(getchar() != '\n');
	ShowMessage("\n");
	ShowNotice("Do you wish to convert your Storage Database to SQL? (y/n) : ");
	input = getchar();
	if(input == 'y' || input == 'Y')
	{
		struct storage storage_;
		ShowMessage("\n");
		ShowStatus("Converting Storage Database...\n");
		if( (fp = fopen(storage_txt,"r")) == NULL )
		{
			ShowError("can't read : %s\n", storage_txt);
			return 0;
		}
		lineno = count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			lineno++;
			set=sscanf(line,"%d,%d",&tmp_int[0],&tmp_int[1]);
			if(set==2) {
				memset(&storage_, 0, sizeof(struct storage));
				storage_.account_id=tmp_int[0];
				if (storage_fromstr(line,&storage_) == 0) {
					count++;
					storage_tosql(storage_.account_id,&storage_); //to sql. (dump)
				} else {
					ShowError("Error parsing storage line [%s] (at %s:%d)\n", line, storage_txt, lineno);
				}
			}
		}
		ShowStatus("Converted %d storages.\n", count);
		fclose(fp);
	}

	//FIXME: CONVERT STATUS DATA HERE!!!

	while(getchar() != '\n');
	ShowMessage("\n");
	ShowNotice("Do you wish to convert your Pet Database to SQL? (y/n) : ");
	input=getchar();
	if(input == 'y' || input == 'Y')
	{
		struct s_pet p;
		ShowMessage("\n");
		ShowStatus("Converting Pet Database...\n");
		if( (fp = fopen(pet_txt, "r")) == NULL )
		{
			ShowError("Unable to open file %s!", pet_txt);
			return 1;
		}
		lineno = count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			lineno++;
			memset (&p, 0, sizeof(struct s_pet));
			if(inter_pet_fromstr(line, &p)==0 && p.pet_id>0) {
				count++;
				inter_pet_tosql(p.pet_id,&p);
			} else {
				ShowError("pet reading: broken data [%s] at %s:%d\n", line, pet_txt, lineno);
			}
		}
		ShowStatus("Converted %d pets.\n", count);
		fclose(fp);
	}

	//FIXME: CONVERT HOMUNCULUS DATA AND SKILLS HERE!!!

	while(getchar() != '\n');
	ShowMessage("\n");
	ShowNotice("Do you wish to convert your Party Database to SQL? (y/n) : ");
	input=getchar();
	if(input == 'y' || input == 'Y')
	{
		struct party p;
		ShowMessage("\n");
		ShowStatus("Converting Party Database...\n");
		if( (fp = fopen(party_txt, "r")) == NULL )
		{
			ShowError("Unable to open file %s!", party_txt);
			return 1;
		}
		lineno = count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			lineno++;
			memset (&p, 0, sizeof(struct party));
			if(inter_party_fromstr(line, &p) == 0 &&
				p.party_id > 0 &&
				inter_party_tosql(&p, PS_CREATE, 0))
				count++;
			else{
				ShowError("party reading: broken data [%s] at %s:%d\n", line, pet_txt, lineno);
			}
		}
		ShowStatus("Converted %d parties.\n", count);
		fclose(fp);
	}

	while(getchar() != '\n');
	ShowMessage("\n");
	ShowNotice("Do you wish to convert your Guilds and Castles Database to SQL? (y/n) : ");
	input=getchar();
	if(input == 'y' || input == 'Y')
	{
		struct guild g;
		struct guild_castle gc;
		ShowMessage("\n");
		ShowStatus("Converting Guild Database...\n");
		if( (fp = fopen(guild_txt, "r")) == NULL )
		{
			ShowError("Unable to open file %s!", guild_txt);
			return 1;
		}
		lineno = count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			lineno++;
			memset (&g, 0, sizeof(struct guild));
			if (inter_guild_fromstr(line, &g) == 0 &&
				g.guild_id > 0 &&
				inter_guild_tosql(&g,GS_MASK))
				count++;
			else
				ShowError("guild reading: broken data [%s] at %s:%d\n", line, guild_txt, lineno);
		}
		ShowStatus("Converted %d guilds.\n", count);
		fclose(fp);
		ShowStatus("Converting Guild Castles Database...\n");
		if( (fp = fopen(castle_txt, "r")) == NULL )
		{
			ShowError("Unable to open file %s!", castle_txt);
			return 1;
		}
		lineno = count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			lineno++;
			memset(&gc, 0, sizeof(struct guild_castle));
			if (inter_guildcastle_fromstr(line, &gc) == 0) {
				inter_guildcastle_tosql(&gc);
				count++;
			}
			else
				ShowError("guild castle reading: broken data [%s] at %s:%d\n", line, castle_txt, lineno);
		}
		ShowStatus("Converted %d guild castles.\n", count);
		fclose(fp);
	}

	while(getchar() != '\n');
	ShowMessage("\n");
	ShowNotice("Do you wish to convert your Guild Storage Database to SQL? (y/n) : ");
	input=getchar();
	if(input == 'y' || input == 'Y')
	{
		struct guild_storage storage_;
		ShowMessage("\n");
		ShowStatus("Converting Guild Storage Database...\n");
		if( (fp = fopen(guild_storage_txt, "r")) == NULL )
		{
			ShowError("can't read : %s\n", guild_storage_txt);
			return 0;
		}
		lineno = count = 0;
		while(fgets(line, sizeof(line), fp))
		{
			lineno++;
			memset(&storage_, 0, sizeof(struct guild_storage));
			if (sscanf(line,"%d",&storage_.guild_id) == 1 &&
				storage_.guild_id > 0 &&
				guild_storage_fromstr(line,&storage_) == 0
			) {
				count++;
				guild_storage_tosql(storage_.guild_id, &storage_);
			} else
				ShowError("Error parsing guild storage line [%s] (at %s:%d)\n", line, guild_storage_txt, lineno);
		}
		ShowStatus("Converted %d guild storages.\n", count);
		fclose(fp);
	}

	//FIXME: CONVERT MAPREG HERE! (after enforcing MAPREGSQL for sql)

	return 0;
}

int do_init(int argc, char** argv)
{
	char_config_read( (argc > 1) ? argv[1] : CHAR_CONF_NAME);
	mapindex_init();
	sql_config_read( (argc > 2) ? argv[2] : SQL_CONF_NAME);
	inter_init_txt( (argc > 3) ? argv[3] : INTER_CONF_NAME);
	inter_init_sql( (argc > 3) ? argv[3] : INTER_CONF_NAME);
	convert_init();
	ShowStatus("Everything's been converted!\n");
	mapindex_final();
	return 0;
}

void do_final(void) {}
