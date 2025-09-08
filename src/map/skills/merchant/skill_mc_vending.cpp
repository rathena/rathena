#include "skill_mc_vending.hpp"

SkillMC_VENDING::SkillMC_VENDING() : SkillImpl(MC_VENDING) {
}

int32 SkillMC_VENDING::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) const {
	if (sd) {
		// Prevent vending of GMs with unnecessary Level to trade/drop. [Skotlex]
		if (!pc_can_give_items(sd))
			clif_skill_fail(*sd, MC_VENDING);
		else {
			sd->state.prevend = 1;
			sd->state.workinprogress = WIP_DISABLE_ALL;
			sd->vend_skill_lv = skill_lv;
			ARR_FIND(0, MAX_CART, i, sd->cart.u.items_cart[i].nameid && sd->cart.u.items_cart[i].id == 0);
			if (i < MAX_CART) {
				// Save the cart before opening the vending UI
				sd->state.pending_vending_ui = true;
				intif_storage_save(sd, &sd->cart);
			} else {
				// Instantly open the vending UI
				sd->state.pending_vending_ui = false;
				clif_openvendingreq(*sd, 2 + skill_lv);
			}
		}
	}
	return 1; // Success
}
