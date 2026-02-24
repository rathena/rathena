// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "homunculus_stylechange.hpp"

SkillStyleChange::SkillStyleChange() : SkillImpl(MH_STYLE_CHANGE) {
}

void SkillStyleChange::castendNoDamageId(block_list* src, block_list* target, uint16 skill_lv, t_tick tick, int32& flag) const {
	homun_data* hd = BL_CAST(BL_HOM, src);

	if (hd) {
		struct status_change_entry* sce;
		if ((sce = hd->sc.getSCE(SC_STYLE_CHANGE)) != nullptr) { //in preparation for other bl usage
			if (sce->val1 == MH_MD_FIGHTING) {
				sce->val1 = MH_MD_GRAPPLING;
			} else {
				sce->val1 = MH_MD_FIGHTING;
			}
			//if(hd->master && hd->sc.getSCE(SC_STYLE_CHANGE)) { // Aegis does not show any message when switching fighting style
			//	char output[128];
			//	safesnprintf(output,sizeof(output),msg_txt(sd,378),(sce->val1==MH_MD_FIGHTING?"fighthing":"grappling"));
			//	clif_messagecolor(hd->master, color_table[COLOR_RED], output, false, SELF);
			//}
		} else {
			sc_start(hd, hd, SC_STYLE_CHANGE, 100, MH_MD_FIGHTING, INFINITE_TICK);
		}
	}
}
