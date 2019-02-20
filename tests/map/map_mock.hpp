#ifndef MAP_MOCK_HPP
#define MAP_MOCK_HPP

#include "gmock/gmock.h"

class map_mock : public Map_Interface {
 public:
  MOCK_METHOD1(setusers,
      void(int));
  MOCK_METHOD0(getusers,
      int(void));
  MOCK_METHOD0(usercount,
      int(void));
  MOCK_METHOD1(freeblock,
      int(struct block_list *bl));
  MOCK_METHOD0(freeblock_lock,
      int(void));
  MOCK_METHOD0(freeblock_unlock,
      int(void));
  MOCK_METHOD1(addblock,
      int(struct block_list* bl));
  MOCK_METHOD1(delblock,
      int(struct block_list* bl));
  MOCK_METHOD4(moveblock,
      int(struct block_list *, int, int, t_tick));
  MOCK_METHOD5(foreachinrange,
      int(int test, struct block_list* center, int16 range, int type, std::string message));
  MOCK_METHOD5(foreachinallrange,
      int(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, std::string message));
  MOCK_METHOD5(foreachinshootrange,
      int(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, std::string message));
  MOCK_METHOD8(foreachinarea,
      int(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, std::string message));
  MOCK_METHOD8(foreachinallarea,
      int(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, std::string message));
  MOCK_METHOD8(foreachinshootarea,
      int(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, std::string message));
  MOCK_METHOD6(forcountinrange,
      int(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int count, int type, std::string message));
  MOCK_METHOD9(forcountinarea,
      int(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, std::string message));
  MOCK_METHOD7(foreachinmovearea,
      int(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, std::string message));
  MOCK_METHOD6(foreachincell,
      int(int (*func)(struct block_list*,va_list), int16 m, int16 x, int16 y, int type, std::string message));
  //MOCK_METHOD10(foreachinpath,
  //    int(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, ...));
  //MOCK_METHOD12(foreachindir,
  //    int(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int offset, int type, ...));
  MOCK_METHOD4(foreachinmap,
      int(int (*func)(struct block_list*,va_list), int16 m, int type, std::string message));
  MOCK_METHOD5(count_oncell,
      int(int16 m,int16 x,int16 y,int type,int flag));
  MOCK_METHOD6(find_skill_unit_oncell,
      struct skill_unit*(struct block_list *,int16 x,int16 y,uint16 skill_id,struct skill_unit *, int flag));
  MOCK_METHOD0(get_new_object_id,
      int(void));
  MOCK_METHOD7(search_freecell,
      int(struct block_list *src, int16 m, int16 *x, int16 *y, int16 rx, int16 ry, int flag));
  MOCK_METHOD5(closest_freecell,
      bool(int16 m, int16 *x, int16 *y, int type, int flag));
  MOCK_METHOD1(quit,
      int(struct map_session_data *));
  MOCK_METHOD2(addnpc,
      bool(int16 m,struct npc_data *));
  MOCK_METHOD1(clearflooritem,
      void(struct block_list* bl));
  //MOCK_METHOD11(addflooritem_, int (struct item *item, int amount, int16 m, int16 x, int16 y, int first_charid, int second_charid, int third_charid, int flags, unsigned short mob_id, bool canShowEffect));
  MOCK_METHOD1(addflooritem, int(int id));
  MOCK_METHOD2(addinstancemap,
      int(const char *name, unsigned short instance_id));
  MOCK_METHOD1(delinstancemap,
      int(int m));
  MOCK_METHOD0(data_copyall,
      void(void));
  MOCK_METHOD2(data_copy,
      void(struct map_data *dst_map, struct map_data *src_map));
  MOCK_METHOD2(addnickdb,
      void(int charid, const char* nick));
  MOCK_METHOD2(delnickdb,
      void(int charid, const char* nick));
  MOCK_METHOD2(reqnickdb,
      void(struct map_session_data* sd,int charid));
  MOCK_METHOD1(charid2nick,
      const char*(int charid));
  MOCK_METHOD1(charid2sd,
      struct map_session_data*(int charid));
  MOCK_METHOD1(id2sd,
      struct map_session_data*(int id));
  MOCK_METHOD1(id2md,
      struct mob_data*(int id));
  MOCK_METHOD1(id2nd,
      struct npc_data*(int id));
  MOCK_METHOD1(id2hd,
      struct homun_data*(int id));
  MOCK_METHOD1(id2mc,
      struct mercenary_data*(int id));
  MOCK_METHOD1(id2pd,
      struct pet_data*(int id));
  MOCK_METHOD1(id2ed,
      struct elemental_data*(int id));
  MOCK_METHOD1(id2cd,
      struct chat_data*(int id));
  MOCK_METHOD1(id2bl,
      struct block_list*(int id));
  MOCK_METHOD1(blid_exists,
      bool(int id));
  MOCK_METHOD1(mapid2mapname,
      const char*(int m));
  MOCK_METHOD1(mapindex2mapid,
      int16(unsigned short mapindex));
  MOCK_METHOD1(mapname2mapid,
      int16(const char* name));
  MOCK_METHOD3(mapname2ipport,
      int(unsigned short name, uint32* ip, uint16* port));
  MOCK_METHOD3(setipport,
      int(unsigned short map, uint32 ip, uint16 port));
  MOCK_METHOD3(eraseipport,
      int(unsigned short map, uint32 ip, uint16 port));
  MOCK_METHOD0(eraseallipport,
      int(void));
  MOCK_METHOD1(addiddb,
      void(struct block_list *));
  MOCK_METHOD1(deliddb,
      void(struct block_list *bl));
  MOCK_METHOD2(foreachpc,
      void(int (*func)(struct map_session_data* sd, va_list args), std::string message));
  MOCK_METHOD2(foreachmob,
      void(int (*func)(struct mob_data* md, va_list args), std::string message));
  MOCK_METHOD2(foreachnpc,
      void(int (*func)(struct npc_data* nd, va_list args), std::string message));
  MOCK_METHOD2(foreachregen,
      void(int (*func)(struct block_list* bl, va_list args), std::string message));
  MOCK_METHOD2(foreachiddb,
      void(int (*func)(struct block_list* bl, va_list args), std::string message));
  MOCK_METHOD2(nick2sd,
      struct map_session_data*(const char* nick, bool allow_partial));
  MOCK_METHOD1(getmob_boss,
      struct mob_data*(int16 m));
  MOCK_METHOD1(id2boss,
      struct mob_data*(int id));
  MOCK_METHOD1(reloadnpc,
      void(bool clear));
  MOCK_METHOD2(add_questinfo,
      struct questinfo*(int m, struct questinfo *qi));
  MOCK_METHOD2(remove_questinfo,
      bool(int m, struct npc_data *nd));
  MOCK_METHOD3(has_questinfo,
      struct questinfo*(int m, struct npc_data *nd, int quest_id));
  MOCK_METHOD2(check_dir,
      int(int s_dir,int t_dir));
  MOCK_METHOD3(calc_dir,
      uint8(struct block_list *src,int16 x,int16 y));
  MOCK_METHOD5(calc_dir_xy,
      uint8(int16 srcx, int16 srcy, int16 x, int16 y, uint8 srcdir));
  MOCK_METHOD3(random_dir,
      int(struct block_list *bl, int16 *x, int16 *y));
  MOCK_METHOD1(delmap,
      int(char* mapname));
  MOCK_METHOD0(flags_init,
      void(void));
  MOCK_METHOD1(iwall_exist,
      bool(const char* wall_name));
  MOCK_METHOD7(iwall_set,
      bool(int16 m, int16 x, int16 y, int size, int8 dir, bool shootable, const char* wall_name));
  MOCK_METHOD1(iwall_get,
      void(struct map_session_data *sd));
  MOCK_METHOD1(iwall_remove,
      bool(const char *wall_name));
  MOCK_METHOD2(addmobtolist,
      int(unsigned short m, struct spawn_data *spawn));
  MOCK_METHOD1(spawnmobs,
      void(int16 m));
  MOCK_METHOD1(removemobs,
      void(int16 m));
  MOCK_METHOD1(addmap2db,
      void(struct map_data *m));
  MOCK_METHOD1(removemapdb,
      void(struct map_data *m));
  MOCK_METHOD4(skill_damage_add,
      void(struct map_data *m, uint16 skill_id, int rate[SKILLDMG_MAX], uint16 caster));
  MOCK_METHOD3(skill_duration_add,
      void(struct map_data *mapd, uint16 skill_id, uint16 per));
  MOCK_METHOD1(getmapflag_by_name,
      enum e_mapflag(char* name));
  MOCK_METHOD2(getmapflag_name,
      bool(enum e_mapflag mapflag, char* output));
  MOCK_METHOD3(getmapflag_sub,
      int(int16 m, enum e_mapflag mapflag, union u_mapflag_args *args));
  MOCK_METHOD4(setmapflag_sub,
      bool(int16 m, enum e_mapflag mapflag, bool status, union u_mapflag_args *args));
  
int addflooritem(struct item *item, int amount, int16 m, int16 x, int16 y, int first_charid, int second_charid, int third_charid, int flags, unsigned short mob_id, bool canShowEffect = false) {
	return 0;
}

int foreachinrange( int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...)
{
	std::string non_variadic("Test");
	foreachinrange( 0, center, range, type, non_variadic );
	return 0;
}

int foreachinallarea(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...)
{
	return 0;
}

int foreachinpath(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int16 range, int length, int type, ...)
{
	return 0;
}


int foreachinallrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...)
{
	return 0;
}

int foreachinshootrange (int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int type, ...)
{
	return 0;
}

int foreachinarea(int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, ...)
{
	return 0;
}

int foreachinshootarea (int(*func)(struct block_list*, va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int type, ...)
{
	return 0;
}

int forcountinrange(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int count, int type, ...)
{
	return 0;
}

int forcountinarea(int (*func)(struct block_list*,va_list), int16 m, int16 x0, int16 y0, int16 x1, int16 y1, int count, int type, ...)
{
	return 0;
}

int foreachinmovearea(int (*func)(struct block_list*,va_list), struct block_list* center, int16 range, int16 dx, int16 dy, int type, ...)
{
	return 0;
}

int foreachincell(int (*func)(struct block_list*,va_list), int16 m, int16 x, int16 y, int type, ...)
{
	return 0;
}

int foreachinmap(int (*func)(struct block_list*,va_list), int16 m, int type, ...) 
{
	return 0;
}

void foreachmob(int (*func)(struct mob_data* md, va_list args), ...)
{
}

void foreachnpc(int (*func)(struct npc_data* nd, va_list args), ...)
{
}

void foreachregen(int (*func)(struct block_list* bl, va_list args), ...)
{
}

void foreachiddb(int (*func)(struct block_list* bl, va_list args), ...)
{
}

void foreachpc(int (*func)(struct map_session_data* sd, va_list args), ...)
{
}
};
#endif /* MAP_MOCK_HPP */

