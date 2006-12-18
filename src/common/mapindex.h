#ifndef _MAX_INDEX_H
#define _MAX_INDEX_H
//File in charge of assigning a numberic ID to each map in existance for space saving when passing map info between servers.
extern char mapindex_cfgfile[80];

//whether to enable auto-adding of maps during run. Not so secure as the map indexes will vary!
// disabled - since mapindex.h wasn't included in mapindex.c it never got enabled anyway... [FlavioJS]
//#define MAPINDEX_AUTOADD

//Some definitions for the mayor city maps.
#define MAP_PRONTERA "prontera.gat"
#define MAP_GEFFEN "geffen.gat"
#define MAP_MORROC "morocc.gat"
#define MAP_ALBERTA "alberta.gat"
#define MAP_PAYON "payon.gat"
#define MAP_IZLUDE "izlude.gat"
#define MAP_ALDEBARAN "aldebaran.gat"
#define MAP_LUTIE "xmas.gat"
#define MAP_COMODO "comodo.gat"
#define MAP_YUNO "yuno.gat"
#define MAP_AMATSU "amatsu.gat"
#define MAP_GONRYUN "gonryun.gat"
#define MAP_UMBALA "umbala.gat"
#define MAP_NIFLHEIM "niflheim.gat"
#define MAP_LOUYANG "louyang.gat"
#define MAP_JAWAII "jawaii.gat"
#define MAP_AYOTHAYA "ayothaya.gat"
#define MAP_EINBROCH "einbroch.gat"
#define MAP_LIGHTHALZEN "lighthalzen.gat"
#define MAP_EINBECH "einbech.gat"
#define MAP_HUGEL "hugel.gat"
#define MAP_JAIL "sec_pri.gat"
unsigned short mapindex_name2id(char*);
const char* mapindex_id2name(unsigned short);
void mapindex_init(void);
void mapindex_final(void);

#endif
