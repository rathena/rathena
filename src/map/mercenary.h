// Homunculus and future Mercenary system code go here [Celest]

int do_init_merc (void);
void merc_load_exptables(void);
char *merc_skill_get_name(int id);
void merc_damage(struct homun_data *hd,struct block_list *src,int hp,int sp);
int merc_dead(struct homun_data *hd, struct block_list *src);
void merc_skillup(struct map_session_data *sd,short skillnum);
int merc_gainexp(struct homun_data *hd,int exp);
void merc_heal(struct homun_data *hd,int hp,int sp);
void merc_save(struct homun_data *hd);
void merc_load(struct map_session_data *sd);
int merc_create_homunculus(struct map_session_data *sd,int id,int m,int x,int y);
