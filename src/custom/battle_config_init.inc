// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom battle config initializations
 * Place the definition of custom battle configuration initializations in this file.
 *
 * Do not forget to add a comma at the end of the line, if you want to add another config
 *
 * Format:
 *	 { "name", &battle_config.<variable name>, <default value>, <minimum value>, <maximum value> },
 **/

//@eduardo
	{ "more_exp_rate",         &battle_config.more_exp_rate,         0, 		0, 		1, 				},
	{ "mvp_raid_mode",         &battle_config.mvp_raid_mode,         0, 		0, 		1, 				},
	{ "mvp_raid_mode_m",         &battle_config.mvp_raid_mode_m,         1, 		1, 		1000, 			},
	{ "more_exp_rate_f",         &battle_config.more_exp_rate_f,         1, 		1, 		10000, 			},
	{ "cur_mstat",         &battle_config.cur_mstat,         10, 		10, 		100, 			},
	{ "cur_attprob",         &battle_config.cur_attprob,         1, 		1, 		100, 			},
	{ "sclonepartyexpdist",         &battle_config.sclonepartyexpdist,         0, 		0, 		1, 				},
	{ "slaveclonepet",         &battle_config.slaveclonepet,         0, 		0, 		1, 				},
	{ "scskillbyhotkeys",         &battle_config.scskillbyhotkeys,         0, 		0, 		1, 				},
	{ "hyperactivesc",         &battle_config.hyperactivesc,         1, 		0, 		1, 				},
	{ "sceasymode",         &battle_config.sceasymode,         1, 		0, 		1, 				},
	{ "nv_a",         &battle_config.nv_a,         100, 	0, 		100, 				},
	{ "al_a",         &battle_config.al_a,         50, 		0, 		100, 				},
	{ "sm_a",         &battle_config.sm_a,         80, 		0, 		100, 				},
	{ "ac_a",         &battle_config.ac_a,         70, 		0, 		100, 				},
	{ "mg_a",         &battle_config.mg_a,         10, 		0, 		100, 				},
	{ "mc_a",         &battle_config.mc_a,         90, 		0, 		100, 				},
	{ "tf_a",         &battle_config.tf_a,         90, 		0, 		100, 				},
	{ "tk_a",         &battle_config.tk_a,         85, 		0, 		100, 				},
	{ "gs_a",         &battle_config.gs_a,         85, 		0, 		100, 				},
	{ "nj_a",         &battle_config.nj_a,         70, 		0, 		100, 				},
	{ "su_a",         &battle_config.su_a,         70, 		0, 		100, 				},
	{ "snv_a",        &battle_config.snv_a,        65, 		0, 		100, 				},
	{ "pr_a",         &battle_config.pr_a,         30, 		0, 		100, 				},
	{ "kn_a",         &battle_config.kn_a,         80, 		0, 		100, 				},
	{ "ht_a",         &battle_config.ht_a,         80, 		0, 		100, 				},
	{ "bs_a",         &battle_config.bs_a,         80, 		0, 		100, 				},
	{ "wz_a",         &battle_config.wz_a,         5, 		0, 		100, 				},
	{ "as_a",         &battle_config.as_a,         90, 		0, 		100, 				},
	{ "sj_a",         &battle_config.sj_a,         90, 		0, 		100, 				},
	{ "rl_a",         &battle_config.rl_a,         90, 		0, 		100, 				},
	{ "kg_a",         &battle_config.kg_a,         75, 		0, 		100, 				},
	{ "cr_a",         &battle_config.cr_a,         60, 		0, 		100, 				},
	{ "sa_a",         &battle_config.sa_a,         75, 		0, 		100, 				},
	{ "ba_a",         &battle_config.ba_a,         40, 		0, 		100, 				},
	{ "mo_a",         &battle_config.mo_a,         85, 		0, 		100, 				},
	{ "am_a",         &battle_config.am_a,         60, 		0, 		100, 				},
	{ "rg_a",         &battle_config.rg_a,         85, 		0, 		100, 				},
	{ "sl_a",         &battle_config.sl_a,         10, 		0, 		100, 				},

