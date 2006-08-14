// Homunculus and future Mercenary system code go here [Celest]
// implemented by [orn]
struct homunculus_db {
	int class_ ;
	char name[NAME_LENGTH];
	int basemaxHP ;
	int basemaxSP ;
	int baseSTR ;
	int baseAGI ;
	int baseVIT ;
	int baseINT ;
	int baseDEX ;
	int baseLUK ;
	int foodID ;
	int baseIntimacy ;
	short baseHungry ;
	long hungryDelay ;
	int gminHP ;
	int gmaxHP ;
	int gminSP ;
	int gmaxSP ;
	int gminSTR ;
	int gmaxSTR ;
	int gminAGI ;
	int gmaxAGI ;
	int gminVIT ;
	int gmaxVIT ;
	int gminINT ;
	int gmaxINT ;
	int gminDEX ;
	int gmaxDEX ;
	int gminLUK ;
	int gmaxLUK ;
	int evo_class ;
	int baseASPD ;
	//short size ;
	//short race ;
	//short element ;
	unsigned char element, race, size; // albator
	int accessID ;
};
extern struct homunculus_db homuncumlus_db[MAX_HOMUNCULUS_CLASS];
enum { HOMUNCULUS_CLASS, HOMUNCULUS_FOOD };
enum {
	SP_ACK 	= 0x00,
	SP_INTIMATE 	= 0x100,
	SP_HUNGRY 		= 0x200
};
// merc_is_hom_alive(struct homun_data *)
#define merc_is_hom_active(x) (x && x->master && x->master->homunculus.vaporize != 1 && x->battle_status.hp != 0)
int do_init_merc(void);
int merc_hom_recv_data(int account_id, struct s_homunculus *sh, int flag); //albator
void merc_load_sub(struct homun_data *hd, struct map_session_data *sd);
void merc_load_exptables(void);
char *merc_hom_skill_get_name(int id);
void merc_damage(struct homun_data *hd,struct block_list *src,int hp,int sp);
int merc_hom_dead(struct homun_data *hd, struct block_list *src);
void merc_hom_skillup(struct homun_data *hd,int skillnum);
int merc_hom_calc_skilltree(struct map_session_data *sd) ;
int merc_hom_checkskill(struct map_session_data *sd,int skill_id) ;
int merc_hom_gainexp(struct homun_data *hd,int exp) ;
int merc_hom_levelup(struct homun_data *hd) ;
int merc_hom_evolution(struct homun_data *hd) ;
void merc_hom_heal(struct homun_data *hd,int hp,int sp);
int merc_hom_vaporize(struct map_session_data *sd, int flag);
int merc_hom_revive(struct map_session_data *sd, int per);
void merc_save(struct homun_data *hd);
int merc_call_homunculus(struct map_session_data *sd);
int merc_create_homunculus(struct map_session_data *sd, int class_);
int search_homunculusDB_index(int key,int type);
int merc_menu(struct map_session_data *sd,int menunum);
int merc_hom_food(struct map_session_data *sd, struct homun_data *hd);
int merc_hom_hungry_timer_delete(struct homun_data *hd);
int merc_natural_heal_timer_delete(struct homun_data *hd);
#define merc_checkoverhp(hd) (hd->battle_status.hp == hd->battle_status.max_hp)
#define merc_checkoversp(hd) (hd->battle_status.sp == hd->battle_status.max_sp)
#define merc_stop_walking(hd, type) { if((hd)->ud.walktimer != -1) unit_stop_walking(&(hd)->bl, type); }
#define merc_stop_attack(hd) { if((hd)->ud.attacktimer != -1) unit_stop_attack(&(hd)->bl); hd->ud.target = 0; }
int read_homunculusdb(void);
int merc_hom_increase_intimacy(struct homun_data * hd, unsigned int value);
int merc_hom_decrease_intimacy(struct homun_data * hd, unsigned int value);
int merc_skill_tree_get_max(int id, int b_class);
int merc_hom_data_init(struct map_session_data *sd);
