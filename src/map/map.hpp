// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAP_HPP
#define MAP_HPP

#include "map_interface.hpp"

class Map_Obj : public Map_Interface {

	public:
		// users
		void setusers(int);
		int getusers(void);
		int usercount(void);

		// blocklist lock
		int freeblock(struct block_list *bl);
		int freeblock_lock(void);
		int freeblock_unlock(void);
		// blocklist manipulation
		int addblock(struct block_list* bl);
		int delblock(struct block_list* bl);
		int moveblock(struct block_list *, int, int, t_tick);
		int foreachinrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...);
		int foreachinallrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...);
		int foreachinshootrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...);
		int foreachinarea(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...);
		int foreachinallarea(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...);
		int foreachinshootarea(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...);
		int forcountinrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int count, int type, ...);
		int forcountinarea(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, ...);
		int foreachinmovearea(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, ...);
		int foreachincell(int (*func)(struct block_list*,va_list), int16 m, int16 x, int16 y, int type, ...);
		int foreachinpath(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, ...);
		int foreachindir(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int offset, int type, ...);
		int foreachinmap(int (*func)(struct block_list*,va_list), int16 m, int type, ...);
		//blocklist nb in one cell
		int count_oncell(int16 m,int16 x,int16 y,int type,int flag);
		struct skill_unit *find_skill_unit_oncell(struct block_list *,int16 x,int16 y,uint16 skill_id,struct skill_unit *, int flag);
		// search and creation
		int get_new_object_id(void);
		int search_freecell(struct block_list *src, int16 m, int16 *x, int16 *y, int16 rx, int16 ry, int flag);
		bool closest_freecell(int16 m, int16 *x, int16 *y, int type, int flag);
		//
		int quit(struct map_session_data *);
		// npc
		bool addnpc(int16 m,struct npc_data *);

		// map item
		//TIMER_FUNC(map_clearflooritem_timer);
		//TIMER_FUNC(map_removemobs_timer);
		void clearflooritem(struct block_list* bl);
		int addflooritem(struct item *item, int amount, int16 m, int16 x, int16 y, int first_charid, int second_charid, int third_charid, int flags, unsigned short mob_id, bool canShowEffect = false);

		// instances
		int addinstancemap(const char *name, unsigned short instance_id);
		int delinstancemap(int m);
		void data_copyall(void);
		void data_copy(struct map_data *dst_map, struct map_data *src_map);

		// player to map session
		void addnickdb(int charid, const char* nick);
		void delnickdb(int charid, const char* nick);
		void reqnickdb(struct map_session_data* sd,int charid);
		const char* charid2nick(int charid);
		struct map_session_data* charid2sd(int charid);

		struct map_session_data * id2sd(int id);
		struct mob_data * id2md(int id);
		struct npc_data * id2nd(int id);
		struct homun_data* id2hd(int id);
		struct mercenary_data* id2mc(int id);
		struct pet_data* id2pd(int id);
		struct elemental_data* id2ed(int id);
		struct chat_data* id2cd(int id);
		struct block_list * id2bl(int id);
		bool blid_exists( int id );

		const char* mapid2mapname(int m);
		int16 mapindex2mapid(unsigned short mapindex);
		int16 mapname2mapid(const char* name);
		int mapname2ipport(unsigned short name, uint32* ip, uint16* port);
		int setipport(unsigned short map, uint32 ip, uint16 port);
		int eraseipport(unsigned short map, uint32 ip, uint16 port);
		int eraseallipport(void);
		void addiddb(struct block_list *);
		void deliddb(struct block_list *bl);
		void foreachpc(int (*func)(struct map_session_data* sd, va_list args), ...);
		void foreachmob(int (*func)(struct mob_data* md, va_list args), ...);
		void foreachnpc(int (*func)(struct npc_data* nd, va_list args), ...);
		void foreachregen(int (*func)(struct block_list* bl, va_list args), ...);
		void foreachiddb(int (*func)(struct block_list* bl, va_list args), ...);
		struct map_session_data * nick2sd(const char* nick, bool allow_partial);
		struct mob_data * getmob_boss(int16 m);
		struct mob_data * id2boss(int id);

		// reload config file looking only for npcs
		void reloadnpc(bool clear);

		struct questinfo *add_questinfo(int m, struct questinfo *qi);
		bool remove_questinfo(int m, struct npc_data *nd);
		struct questinfo *has_questinfo(int m, struct npc_data *nd, int quest_id);

		int check_dir(int s_dir,int t_dir);
		uint8 calc_dir(struct block_list *src,int16 x,int16 y);
		uint8 calc_dir_xy(int16 srcx, int16 srcy, int16 x, int16 y, uint8 srcdir);
		int random_dir(struct block_list *bl, int16 *x, int16 *y); // [Skotlex]

		//int cleanup_sub(struct block_list *bl, va_list ap);

		int delmap(char* mapname);
		void flags_init(void);

		bool iwall_exist(const char* wall_name);
		bool iwall_set(int16 m, int16 x, int16 y, int size, int8 dir, bool shootable, const char* wall_name);
		void iwall_get(struct map_session_data *sd);
		bool iwall_remove(const char *wall_name);

		int addmobtolist(unsigned short m, struct spawn_data *spawn);	// [Wizputer]
		void spawnmobs(int16 m); // [Wizputer]
		void removemobs(int16 m); // [Wizputer]
		void addmap2db(struct map_data *m);
		void removemapdb(struct map_data *m);

		void skill_damage_add(struct map_data *m, uint16 skill_id, int rate[SKILLDMG_MAX], uint16 caster);
		void skill_duration_add(struct map_data *mapd, uint16 skill_id, uint16 per);

		enum e_mapflag getmapflag_by_name(char* name);
		bool getmapflag_name(enum e_mapflag mapflag, char* output);
		int getmapflag_sub(int16 m, enum e_mapflag mapflag, union u_mapflag_args *args);
		bool setmapflag_sub(int16 m, enum e_mapflag mapflag, bool status, union u_mapflag_args *args);

		//void do_shutdown(void);
		Map_Obj();
		virtual ~Map_Obj();

};
#endif /* MAP_HPP */
