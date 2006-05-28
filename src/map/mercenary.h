// Homunculus and future Mercenary system code go here [Celest]

int do_init_merc (void);
void merc_load_exptables(void);
char *merc_skill_get_name(int id);
void merc_die(struct map_session_data *sd);
int merc_damage(struct block_list *src,struct homun_data *hd,int damage,int type);
void merc_calc_status(struct homun_data *hd);
void merc_skillup(struct map_session_data *sd,short skillnum);
void merc_calc_stats(struct homun_data *hd);
int merc_gainexp(struct homun_data *hd,int exp);
int merc_heal(struct homun_data *hd,int hp,int sp);
void merc_save(struct map_session_data *sd);
void merc_res(struct map_session_data *sd,int skilllv);
void merc_load(struct map_session_data *sd);
int merc_create_homunculus(struct map_session_data *sd,int id,int m,int x,int y);
