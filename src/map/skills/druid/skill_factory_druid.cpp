// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill_factory_druid.hpp"

#include <cstdarg>

#include <common/random.hpp>

#include "../../battle.hpp"
#include "../../clif.hpp"
#include "../../map.hpp"
#include "../../party.hpp"
#include "../../path.hpp"
#include "../../pc.hpp"
#include "../../status.hpp"
#include "../../unit.hpp"
#include "../status_skill_impl.hpp"

#include "aroundflower.hpp"
#include "chopchop.hpp"
#include "cruelbite.hpp"
#include "cuttingwind.hpp"
#include "earthdrill.hpp"
#include "earthflower.hpp"
#include "feathersprinkle.hpp"
#include "flickingtornado.hpp"
#include "hunger.hpp"
#include "icecloud.hpp"
#include "icepillar.hpp"
#include "icetotem.hpp"
#include "lowflight.hpp"
#include "nastyslash.hpp"
#include "nomercyclaw.hpp"
#include "sharpengust.hpp"
#include "shootingfeather.hpp"
#include "thunderingfocus.hpp"
#include "transformationbeast.hpp"
#include "transformationraptor.hpp"
#include "truthofearth.hpp"
#include "truthofice.hpp"
#include "truthofwind.hpp"
#include "windbomb.hpp"

namespace {
	constexpr int32 kClawChainDuration = 5000;
	constexpr int32 kZephyrChargeDuration = 10000;
	constexpr int32 kThunderingChargeDuration = 10000;
	constexpr int32 kGrowthDuration = 10000;
	constexpr int32 kTerraWaveStepMs = 150;

	int32 get_madness_stage(const status_change *sc) {
		if (!sc) {
			return 0;
		}

		if (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE3)) {
			return 3;
		}
		if (sc->hasSCE(SC_INSANE2)) {
			return 2;
		}
		if (sc->hasSCE(SC_INSANE)) {
			return 1;
		}

		return 0;
	}

	bool is_thundering_charge_skill(e_skill skill_id) {
		switch (skill_id) {
			case KR_THUNDERING_FOCUS:
			case KR_THUNDERING_ORB:
			case KR_THUNDERING_CALL:
			case AT_ROARING_PIERCER:
			case AT_ROARING_CHARGE:
				return true;
			default:
				return false;
		}
	}

	e_skill resolve_thundering_charge_skill(const status_change *sc, e_skill skill_id) {
		if (!sc || !sc->hasSCE(SC_THUNDERING_ROD_MAX)) {
			return skill_id;
		}

		switch (skill_id) {
			case KR_THUNDERING_FOCUS:
				return KR_THUNDERING_FOCUS_S;
			case KR_THUNDERING_ORB:
				return KR_THUNDERING_ORB_S;
			case KR_THUNDERING_CALL:
				return KR_THUNDERING_CALL_S;
			case AT_ROARING_PIERCER:
				return AT_ROARING_PIERCER_S;
			case AT_ROARING_CHARGE:
				return AT_ROARING_CHARGE_S;
			default:
				return skill_id;
		}
	}

	e_skill resolve_quill_spear_skill(const status_change *sc, e_skill skill_id) {
		if (skill_id != AT_QUILL_SPEAR) {
			return skill_id;
		}

		if (sc && sc->hasSCE(SC_APEX_PHASE)) {
			return AT_QUILL_SPEAR_S;
		}

		return skill_id;
	}

	void try_gain_madness(block_list *src) {
		status_change *sc = status_get_sc(src);
		status_change_entry *pulse = sc ? sc->getSCE(SC_PULSE_OF_MADNESS) : nullptr;

		if (!pulse) {
			return;
		}

		int32 chance = 20 + 10 * (pulse->val1 - 1);
		if (!rnd_chance(chance, 100)) {
			return;
		}

		t_tick duration = skill_get_time2(AT_PULSE_OF_MADNESS, pulse->val1);

		if (sc->hasSCE(SC_INSANE3)) {
			sc_start(src, src, SC_INSANE3, 100, 1, duration);
			return;
		}

		if (sc->hasSCE(SC_INSANE2)) {
			status_change_end(src, SC_INSANE2);
			sc_start(src, src, SC_INSANE3, 100, 1, duration);
			return;
		}

		if (sc->hasSCE(SC_INSANE)) {
			status_change_end(src, SC_INSANE);
			sc_start(src, src, SC_INSANE2, 100, 1, duration);
			return;
		}

		sc_start(src, src, SC_INSANE, 100, 1, duration);
	}

	void add_thundering_charge(block_list *src, int32 count) {
		status_change *sc = status_get_sc(src);
		if (!sc || sc->hasSCE(SC_THUNDERING_ROD_MAX) || count <= 0) {
			return;
		}

		status_change_entry *charge = sc->getSCE(SC_THUNDERING_ROD);
		int32 current = charge ? charge->val3 : 0;
		int32 next = current + count;

		if (next > 5) {
			status_change_end(src, SC_THUNDERING_ROD);
			status_change_start(src, src, SC_THUNDERING_ROD_MAX, 10000, 1, 0, 0, 0, kThunderingChargeDuration, SCSTART_NOAVOID);
			return;
		}
		status_change_start(src, src, SC_THUNDERING_ROD, 10000, 0, 0, next, 0, kThunderingChargeDuration, SCSTART_NOAVOID);
	}

	void try_gain_thundering_charge(block_list *src, const status_change *sc, e_skill skill_id, int32 gain) {
		if (!is_thundering_charge_skill(skill_id)) {
			return;
		}

		if (sc && sc->hasSCE(SC_THUNDERING_ROD_MAX)) {
			status_change_end(src, SC_THUNDERING_ROD_MAX);
		} else {
			add_thundering_charge(src, gain);
		}
	}

	void add_growth_stacks(block_list *src, t_tick tick, int32 amount) {
		map_session_data *sd = BL_CAST(BL_PC, src);
		status_change *sc = status_get_sc(src);

		if (!sd || !sc || amount <= 0) {
			return;
		}

		int32 bud_lv = pc_checkskill(sd, KR_EARTH_BUD);
		if (bud_lv <= 0) {
			return;
		}

		status_change_entry *grow = sc->getSCE(SC_GROUND_GROW);
		int32 current = grow ? grow->val3 : 0;

		if (current >= 12) {
			status_change_end(src, SC_GROUND_GROW);
			skill_castend_nodamage_id(src, src, KR_GROUND_BLOOM, bud_lv, tick, 0);

			int64 heal = static_cast<int64>(status_get_max_hp(src)) * (bud_lv * 3) / 100;
			if (heal > 0) {
				status_heal(src, heal, 0, 0);
			}
			return;
		}

		int32 next = current + amount;
		if (next > 12) {
			next = 12;
		}

		status_change_start(src, src, SC_GROUND_GROW, 10000, 0, 0, next, 0, kGrowthDuration, SCSTART_NOAVOID);
	}

	void try_gain_growth_stacks(block_list *src, t_tick tick, e_skill skill_id) {
		int32 amount = 0;
		switch (skill_id) {
			case KR_EARTH_DRILL:
			case KR_EARTH_STAMP:
				amount = 1;
				break;
			case AT_TERRA_HARVEST:
			case AT_TERRA_WAVE:
				amount = 2;
				break;
			case AT_SOLID_STOMP:
				amount = 4;
				break;
			default:
				return;
		}

		add_growth_stacks(src, tick, amount);
	}

	bool get_glacier_center(const status_change *sc, int32 &x, int32 &y, int32 &map_id) {
		if (!sc) {
			return false;
		}

		const status_change_entry *sce = sc->getSCE(SC_GLACIER_SHEILD);
		if (!sce) {
			return false;
		}

		x = sce->val2;
		y = sce->val3;
		map_id = sce->val4;
		return true;
	}

	bool get_glacier_center_on_map(const block_list *src, const status_change *sc, int32 &gx, int32 &gy) {
		int32 map_id = 0;

		if (!get_glacier_center(sc, gx, gy, map_id)) {
			return false;
		}

		return src->m == map_id;
	}

	void clear_glacier_monolith(block_list *src) {
		unit_data *ud = unit_bl2ud(src);

		if (!ud) {
			return;
		}

		for (size_t i = 0; i < ud->skillunits.size();) {
			std::shared_ptr<s_skill_unit_group> sg = ud->skillunits[i];

			if (sg && (sg->skill_id == AT_GLACIER_MONOLITH || sg->unit_id == UNT_GLACIAL_MONOLITH)) {
				skill_delunitgroup(sg);
				continue;
			}

			++i;
		}
	}

	int32 apply_splash_outer_sub(block_list *bl, va_list ap) {
		block_list *src = va_arg(ap, block_list *);
		uint16 skill_id = static_cast<uint16>(va_arg(ap, int));
		uint16 skill_lv = static_cast<uint16>(va_arg(ap, int));
		t_tick tick = va_arg(ap, t_tick);
		int32 flag = va_arg(ap, int32);
		int32 x = va_arg(ap, int32);
		int32 y = va_arg(ap, int32);
		int32 min_distance = va_arg(ap, int32);
		int32 exclude_id = va_arg(ap, int32);

		if (bl->id == exclude_id) {
			return 0;
		}

		if (distance_xy(x, y, bl->x, bl->y) <= min_distance) {
			return 0;
		}

		if (battle_check_target(src, bl, BCT_ENEMY) <= 0) {
			return 0;
		}

		return static_cast<int32>(skill_attack(skill_get_type(static_cast<e_skill>(skill_id)), src, src, bl, skill_id, skill_lv, tick, flag | SD_ANIMATION));
	}

	int32 apply_gravity_hole_hit(block_list *src, block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int32 flag) {
		if (battle_check_target(src, bl, BCT_ENEMY) <= 0) {
			return 0;
		}

		int32 sflag = flag | SD_ANIMATION;
		if (skill_attack(skill_get_type(static_cast<e_skill>(skill_id)), src, src, bl, skill_id, skill_lv, tick, sflag) > 0) {
			int32 dist = distance_xy(src->x, src->y, bl->x, bl->y) - 1;
			const sc_type type = skill_get_sc(skill_id);
			const t_tick duration = skill_get_time(skill_id, skill_lv);

			if (type != SC_NONE && duration > 0) {
				sc_start(src, bl, type, 100, 1, duration);
			}
			if (dist > 0) {
				int8 dir = map_calc_dir(src, bl->x, bl->y);
				skill_blown(src, bl, static_cast<char>(dist), dir, BLOWN_NONE);
			}
			return 1;
		}

		return 0;
	}

int32 apply_gravity_hole_sub(block_list *bl, va_list ap) {
	block_list *src = va_arg(ap, block_list *);
	uint16 skill_id = static_cast<uint16>(va_arg(ap, int));
	uint16 skill_lv = static_cast<uint16>(va_arg(ap, int));
	t_tick tick = va_arg(ap, t_tick);
	int32 flag = va_arg(ap, int32);
	int32 max_targets = va_arg(ap, int32);

		if (skill_area_temp[2] >= max_targets) {
			return 0;
		}

		if (apply_gravity_hole_hit(src, bl, skill_id, skill_lv, tick, flag) > 0) {
			skill_area_temp[2]++;
			return 1;
		}

	return 0;
}

int32 apply_zephyr_link_sub(block_list *bl, va_list ap) {
	block_list *src = va_arg(ap, block_list *);
	int32 party_id = va_arg(ap, int32);
	t_tick tick = va_arg(ap, t_tick);
	map_session_data *sd = BL_CAST(BL_PC, bl);

	if (!sd || sd->status.party_id != party_id) {
		return 0;
	}

	return skill_castend_nodamage_id(src, bl, AT_ZEPHYR_LINK, 1, tick, 0);
}

class SkillDruidImpl : public SkillImplRecursiveDamageSplash {
public:
	explicit SkillDruidImpl(e_skill skill_id) : SkillImplRecursiveDamageSplash(skill_id) {}

		void castendDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const override {
			status_change *sc = status_get_sc(src);

			if (!(flag & 1)) {
				clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
			}

			switch (getSkillId()) {
				case AT_GRAVITY_HOLE: {
					if ((flag & 1) || target != src) {
						apply_gravity_hole_hit(src, target, getSkillId(), skill_lv, tick, flag);
						return;
					}

					const int32 splash = skill_get_splash(getSkillId(), skill_lv);
					const int32 max_targets = 5 + skill_lv;

					skill_area_temp[0] = 0;
					skill_area_temp[1] = 0;
					skill_area_temp[2] = 0;
					map_foreachinrange(apply_gravity_hole_sub, src, splash, BL_CHAR, src, getSkillId(), skill_lv, tick, flag, max_targets);
					return;
				}
				case AT_QUILL_SPEAR:
				case AT_QUILL_SPEAR_S: {
					if (flag & 1) {
						SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
					} else {
						const bool apex = sc && sc->hasSCE(SC_APEX_PHASE);
						const int16 half_width = (apex || getSkillId() == AT_QUILL_SPEAR_S) ? 2 : 1;
						const int16 half_length = 3;
						const int32 length = half_length * 2 + 1;
						const uint8 dir = map_calc_dir(src, target->x, target->y);
						const int16 dir_x = dirx[dir];
						const int16 dir_y = diry[dir];
						const int16 start_x = target->x - dir_x * half_length;
						const int16 start_y = target->y - dir_y * half_length;
						const int16 end_x = target->x;
						const int16 end_y = target->y;

						skill_area_temp[0] = 0;
						skill_area_temp[1] = target->id;
						skill_area_temp[2] = 0;
						map_foreachindir(skill_area_sub, src->m, start_x, start_y, end_x, end_y, half_width, length, 0, splash_target(src),
								 src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | SD_SPLASH | 1, skill_castend_damage_id);
					}
					return;
				}
				case AT_ROARING_PIERCER:
				case AT_ROARING_PIERCER_S: {
					if (flag & 1) {
						SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
					} else {
						const int16 half_width = 2;
						const int16 half_length = 4;
						const int32 length = half_length * 2 + 1;
						int32 offset = distance(target->x - src->x, target->y - src->y) - half_length;

						if (offset < 0) {
							offset = 0;
						}

						skill_area_temp[0] = 0;
						skill_area_temp[1] = target->id;
						skill_area_temp[2] = 0;
						map_foreachindir(skill_area_sub, src->m, src->x, src->y, target->x, target->y, half_width, length, offset, splash_target(src),
								 src, getSkillId(), skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | SD_SPLASH | 1, skill_castend_damage_id);
					}
					if (!(flag & 1) && getSkillId() == AT_ROARING_PIERCER) {
						try_gain_thundering_charge(src, sc, getSkillId(), 1);
					}
					return;
				}
				case AT_PRIMAL_CLAW: {
					if (!(flag & 1)) {
						if (!unit_movepos(src, target->x, target->y, 2, true)) {
							map_session_data *sd = BL_CAST(BL_PC, src);
							if (sd) {
								clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
							}
							return;
						}
						clif_blown(src);
					}

					SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

					if (!(flag & 1)) {
						status_change_end(src, SC_FERAL_CLAW);
						sc_start(src, src, SC_PRIMAL_CLAW, 100, skill_lv, kClawChainDuration);

						const int32 madness_stage = get_madness_stage(sc);
						if (madness_stage >= 2) {
							int32 base_radius = skill_get_splash(getSkillId(), skill_lv);
							int32 ring_radius = base_radius + 1;
							if (ring_radius > base_radius) {
								map_foreachinrange(apply_splash_outer_sub, target, ring_radius, BL_CHAR, src, getSkillId(), skill_lv, tick, flag,
											   target->x, target->y, base_radius, target->id);
							}
						}

						try_gain_madness(src);
					}
					return;
				}
				case AT_FERAL_CLAW: {
					if (!(flag & 1)) {
						if (!sc || !sc->hasSCE(SC_PRIMAL_CLAW)) {
							map_session_data *sd = BL_CAST(BL_PC, src);
							if (sd) {
								clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
							}
							return;
						}
					}
					if (flag & 1) {
						SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
					} else {
						int32 splash_flag = flag | BCT_WOS;
						SkillImplRecursiveDamageSplash::castendDamageId(src, src, skill_lv, tick, splash_flag);
					}

					if (!(flag & 1)) {
						status_change_end(src, SC_PRIMAL_CLAW);
						sc_start(src, src, SC_FERAL_CLAW, 100, skill_lv, kClawChainDuration);

						const int32 madness_stage = get_madness_stage(sc);
						if (madness_stage >= 2) {
							const int32 base_radius = skill_get_splash(getSkillId(), skill_lv);
							const int32 ring_radius = base_radius + 1;
							if (ring_radius > base_radius) {
								map_foreachinrange(apply_splash_outer_sub, src, ring_radius, BL_CHAR, src, getSkillId(), skill_lv, tick, flag,
											   src->x, src->y, base_radius, target->id);
							}
						}

						try_gain_madness(src);
					}
					return;
				}
				case AT_ALPHA_CLAW: {
					if (!(flag & 1)) {
						if (!sc || !sc->hasSCE(SC_FERAL_CLAW)) {
							map_session_data *sd = BL_CAST(BL_PC, src);
							if (sd) {
								clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
							}
							return;
						}
					}
					if (flag & 1) {
						SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
					} else {
						int32 splash_flag = flag | BCT_WOS;
						SkillImplRecursiveDamageSplash::castendDamageId(src, src, skill_lv, tick, splash_flag);
					}

					if (!(flag & 1)) {
						status_change_end(src, SC_PRIMAL_CLAW);
						status_change_end(src, SC_FERAL_CLAW);

						const int32 madness_stage = get_madness_stage(sc);
						if (madness_stage >= 2) {
							const int32 base_radius = skill_get_splash(getSkillId(), skill_lv);
							const int32 ring_radius = base_radius + 1;
							if (ring_radius > base_radius) {
								map_foreachinrange(apply_splash_outer_sub, src, ring_radius, BL_CHAR, src, getSkillId(), skill_lv, tick, flag,
											   src->x, src->y, base_radius, target->id);
							}
						}

						try_gain_madness(src);
					}
					return;
				}
				case AT_SAVAGE_LUNGE: {
					if (!(flag & 1)) {
						if (!unit_movepos(src, target->x, target->y, 2, true)) {
							map_session_data *sd = BL_CAST(BL_PC, src);
							if (sd) {
								clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
							}
							return;
						}
						clif_blown(src);
					}

					int32 attack_flag = flag | SD_ANIMATION;
					skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, attack_flag);

					if (!(flag & 1)) {
						const int32 madness_stage = get_madness_stage(sc);
						if (madness_stage >= 2) {
							map_foreachinrange(apply_splash_outer_sub, target, 3, BL_CHAR, src, getSkillId(), skill_lv, tick, flag,
									   target->x, target->y, 0, target->id);
						}
						try_gain_madness(src);
					}
					return;
				}
				case AT_FRENZY_FANG: {
					skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
					if (!(flag & 1)) {
						try_gain_madness(src);
					}
					return;
				}
				case AT_PINION_SHOT: {
					int32 attack_flag = flag | SD_ANIMATION;
					skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, attack_flag);

					if (!(flag & 1)) {
						status_change_entry *charge = sc ? sc->getSCE(SC_ZEPHYR_CHARGE) : nullptr;
						int32 current = charge ? charge->val1 : 0;
						if (current >= 5) {
							map_session_data *sd = BL_CAST(BL_PC, src);
							if (sd) {
								int32 party_id = sd->status.party_id;
								const int16 link_range = skill_get_splash(AT_ZEPHYR_LINK, 1);
								if (party_id == 0) {
									skill_castend_nodamage_id(src, src, AT_ZEPHYR_LINK, 1, tick, 0);
								} else {
									map_foreachinrange(apply_zephyr_link_sub, src, link_range, BL_PC, src, party_id, tick);
								}
							}
							status_change_end(src, SC_ZEPHYR_CHARGE);
						} else {
							sc_start4(src, src, SC_ZEPHYR_CHARGE, 100, current + 1, current + 1, current + 1, 0, kZephyrChargeDuration);
						}
					}
					return;
				}
				case AT_SOLID_STOMP: {
					if (!(flag & 1)) {
						sc_type type = skill_get_sc(getSkillId());
						if (type != SC_NONE) {
							sc_start(src, src, type, 100, skill_lv, skill_get_time(getSkillId(), skill_lv));
						}
					}

					SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

					if (!(flag & 1)) {
						int64 heal = static_cast<int64>(status_get_max_hp(src)) * skill_lv / 100;
						if (heal > 0) {
							status_heal(src, heal, 0, 0);
						}
					}
					if (!(flag & 1)) {
						try_gain_growth_stacks(src, tick, getSkillId());
					}
					return;
				}
				case AT_GLACIER_STOMP: {
					if (!(flag & 1)) {
						int32 gx = 0;
						int32 gy = 0;
						if (!get_glacier_center_on_map(src, sc, gx, gy)) {
							map_session_data *sd = BL_CAST(BL_PC, src);
							if (sd) {
								clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
							}
							return;
						}
						if (!unit_movepos(src, gx, gy, 2, true)) {
							map_session_data *sd = BL_CAST(BL_PC, src);
							if (sd) {
								clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
							}
							return;
						}
						clif_fixpos(*src);
					}
					SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);

					if (!(flag & 1)) {
						int32 gx = 0;
						int32 gy = 0;
						if (get_glacier_center_on_map(src, sc, gx, gy)) {
							skill_castend_pos2(src, gx, gy, AT_GLACIER_NOVA, 1, tick, 0);
						}
					}
					return;
				}
				case AT_CHILLING_BLAST:
				case AT_GLACIER_SHARD: {
					SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
					if (!(flag & 1)) {
						int32 gx = 0;
						int32 gy = 0;
						if (get_glacier_center_on_map(src, sc, gx, gy)) {
							skill_castend_pos2(src, gx, gy, AT_GLACIER_NOVA, 1, tick, 0);
						}
					}
					return;
				}
				case AT_ROARING_CHARGE:
				case AT_ROARING_CHARGE_S: {
					SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
					if (!(flag & 1) && getSkillId() == AT_ROARING_CHARGE) {
						try_gain_thundering_charge(src, sc, getSkillId(), 1 + skill_lv);
					}
					return;
				}
				case AT_FURIOS_STORM:
				default:
					SkillImplRecursiveDamageSplash::castendDamageId(src, target, skill_lv, tick, flag);
					if (!(flag & 1)) {
						if (is_thundering_charge_skill(getSkillId())) {
							try_gain_thundering_charge(src, sc, getSkillId(), 1);
						}
						switch (getSkillId()) {
							case KR_EARTH_STAMP:
							case AT_TERRA_WAVE:
							case AT_TERRA_HARVEST:
								try_gain_growth_stacks(src, tick, getSkillId());
								break;
							default:
								break;
						}
					}
					return;
			}
		}

		void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const override {
			switch (getSkillId()) {
				case AT_ZEPHYR_LINK:
					if (!(flag & 1)) {
						clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
					}
					sc_start(src, target, SC_ZEPHYR_LINK, 100, 0, skill_get_time(getSkillId(), skill_lv));
					return;
				case KR_GROUND_BLOOM:
				case AT_FERAL_CLAW:
				case AT_ALPHA_CLAW:
				case AT_GLACIER_STOMP:
				case AT_CHILLING_BLAST:
				case AT_ROARING_CHARGE:
				case AT_ROARING_CHARGE_S:
				case AT_SOLID_STOMP:
				case AT_GRAVITY_HOLE:
				case AT_FURIOS_STORM:
					castendDamageId(src, target, skill_lv, tick, flag);
					return;
				default:
					if (!(flag & 1)) {
						clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
					}
					return;
			}
		}

		void castendPos2(block_list *src, int32 x, int32 y, uint16 skill_lv, t_tick tick, int32 &flag) const override {
			switch (getSkillId()) {
				case AT_TERRA_WAVE: {
					const int32 segment_count = 4;
					const int32 segment_step = 3;
					map_session_data *sd = BL_CAST(BL_PC, src);
					map_data *mapdata = map_getmapdata(src->m);
					bool blocked = false;

					if (mapdata != nullptr) {
						for (int32 dx = -1; dx <= 1 && !blocked; ++dx) {
							for (int32 dy = -1; dy <= 1; ++dy) {
								const int16 cx = x + dx;
								const int16 cy = y + dy;

								if (cx < 0 || cy < 0 || cx >= mapdata->xs || cy >= mapdata->ys) {
									blocked = true;
									break;
								}
								if (map_getcell(src->m, cx, cy, CELL_CHKNOPASS)) {
									blocked = true;
									break;
								}
							}
						}
					}

					if (blocked) {
						if (sd) {
							clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
						}
						return;
					}

					const uint8 dir = map_calc_dir(src, x, y);
					const int32 max_distance = segment_step * (segment_count - 1);
					const int32 wave_flag = flag & ~SD_ANIMATION;
					int16 cx = x;
					int16 cy = y;
					int32 placed = 0;

					for (int32 dist = 0; dist <= max_distance && placed < segment_count; ++dist) {
						if (dist > 0) {
							cx += dirx[dir];
							cy += diry[dir];
						}

						if (mapdata && (cx < 0 || cy < 0 || cx >= mapdata->xs || cy >= mapdata->ys)) {
							break;
						}
						if (map_getcell(src->m, cx, cy, CELL_CHKNOPASS)) {
							break;
						}
						if (dist % segment_step != 0) {
							continue;
						}

						const t_tick step_tick = tick + kTerraWaveStepMs * (placed + 1);
						skill_addtimerskill(src, step_tick, 0, cx, cy, getSkillId(), skill_lv, 0, wave_flag);
						placed++;
					}
					try_gain_growth_stacks(src, tick, getSkillId());
					return;
				}
				case AT_GLACIER_MONOLITH: {
					SkillImplRecursiveDamageSplash::castendPos2(src, x, y, skill_lv, tick, flag);
					clear_glacier_monolith(src);
					const t_tick buff_duration = skill_get_unit_interval(getSkillId());
					sc_start4(src, src, SC_GLACIER_SHEILD, 100, skill_lv, x, y, src->m, buff_duration);
					skill_unitsetting(src, getSkillId(), skill_lv, x, y, 0);
					break;
				}
				default:
					SkillImplRecursiveDamageSplash::castendPos2(src, x, y, skill_lv, tick, flag);
					break;
			}

			if (!(flag & 1)) {
				const status_change *sc = status_get_sc(src);
				try_gain_thundering_charge(src, sc, getSkillId(), 1);
				try_gain_growth_stacks(src, tick, getSkillId());
			}
		}

		void calculateSkillRatio(const Damage *, const block_list *src, const block_list *, uint16 skill_lv, int32 &base_skillratio, int32 mflag) const override {
			const status_change *sc = status_get_sc(src);
			const status_data *sstatus = status_get_status_data(*src);
			int32 skillratio = 0;
			const bool madness = sc && (sc->hasSCE(SC_ALPHA_PHASE) || sc->hasSCE(SC_INSANE) || sc->hasSCE(SC_INSANE2) || sc->hasSCE(SC_INSANE3));

			switch (getSkillId()) {
				case KR_DOUBLE_SLASH: {
					skillratio = 1200 + 80 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_ENRAGE_WOLF)) {
						skillratio += 400;
					}
					skillratio += sstatus->str; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_CLAW_WAVE: {
					skillratio = 880 + 70 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_ENRAGE_WOLF)) {
						skillratio += 320;
					}
					skillratio += sstatus->str; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_SHARPEN_HAIL: {
					skillratio = 640 + 40 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
						skillratio += 300;
					}
					skillratio += sstatus->dex; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_TYPHOON_WING: {
					skillratio = 600 + 80 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_ENRAGE_RAPTOR)) {
						skillratio += 300;
					}
					skillratio += sstatus->dex; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_ICE_SPLASH: {
					skillratio = 1140 + 70 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_ICE)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_THUNDERING_ORB: {
					skillratio = 1400 + 70 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_THUNDERING_ORB_S: {
					skillratio = 1750 + 100 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_THUNDERING_CALL: {
					skillratio = 5200 + 200 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_THUNDERING_CALL_S: {
					skillratio = 9500 + 500 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_EARTH_STAMP: {
					skillratio = 1000 + 70 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					return;
				}
				case KR_GROUND_BLOOM: {
					skillratio = 6000 + 2000 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					return;
				}

				case AT_PRIMAL_CLAW:
					skillratio = 1100 + 950 * (skill_lv - 1);
					if (madness) {
						skillratio += 800;
					}
					skillratio += sstatus->pow * 5; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					break;
				case AT_FERAL_CLAW:
					skillratio = 1600 + 1150 * (skill_lv - 1);
					if (madness) {
						skillratio += 800;
					}
					skillratio += sstatus->pow * 5; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					break;
				case AT_ALPHA_CLAW:
					skillratio = 2200 + 1400 * (skill_lv - 1);
					if (madness) {
						skillratio += 800;
					}
					skillratio += sstatus->pow * 5; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					break;
				case AT_SAVAGE_LUNGE:
					skillratio = (madness ? 9000 : 7500) + (madness ? 2000 : 1500) * (skill_lv - 1);
					skillratio += sstatus->pow * 5; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					break;
				case AT_FRENZY_FANG:
					skillratio = (madness ? 1750 : 1000) + 250 * (skill_lv - 1);
					skillratio += sstatus->pow * 5; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					break;
				case AT_PINION_SHOT:
					skillratio = 2450 * skill_lv;
					skillratio += sstatus->con * 5; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					break;
				case AT_QUILL_SPEAR:
				case AT_QUILL_SPEAR_S:
					skillratio = 2050 * skill_lv;
					skillratio += sstatus->con * 5; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					break;
				case AT_TEMPEST_FLAP:
					skillratio = 1250 * skill_lv;
					skillratio += sstatus->con * 5; // TODO - unknown scaling [munkrej]
					RE_LVL_DMOD(100);
					base_skillratio += -100 + skillratio;
					break;
				case AT_GLACIER_MONOLITH:
					skillratio = 7100 + 300 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_ICE)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_GLACIER_NOVA:
					skillratio = 15000;
					base_skillratio += -100 + skillratio;
					break;
				case AT_GLACIER_SHARD:
					skillratio = 5500 + 300 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_ICE)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_GLACIER_STOMP:
					skillratio = 6400 + 500 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_ICE)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_CHILLING_BLAST:
					skillratio = 8400 + 1500 * (skill_lv - 1);
					base_skillratio += -100 + skillratio;
					break;
				case AT_ROARING_PIERCER:
					skillratio = 11250 + 700 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_ROARING_PIERCER_S:
					skillratio = 11250 + 750 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_ROARING_CHARGE:
					skillratio = 8000 + 400 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_ROARING_CHARGE_S:
					skillratio = 11500 + 500 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_WIND)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_FURIOS_STORM:
					skillratio = 7800 + 400 * (skill_lv - 1);
					base_skillratio += -100 + skillratio;
					break;
				case AT_TERRA_WAVE:
					skillratio = 12000 + 300 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_TERRA_HARVEST:
					skillratio = 18000 + 500 * (skill_lv - 1);
					if (sc && sc->hasSCE(SC_TRUTH_OF_EARTH)) {
						skillratio += sstatus->int_; // TODO - unknown scaling [munkrej]
						RE_LVL_DMOD(100);
					}
					base_skillratio += -100 + skillratio;
					break;
				case AT_SOLID_STOMP:
					skillratio = 10400 + 800 * (skill_lv - 1);
					base_skillratio += -100 + skillratio;
					break;
				case AT_GRAVITY_HOLE:
					skillratio = 9800 + 800 * (skill_lv - 1);
					base_skillratio += -100 + skillratio;
					break;
				default:
					return;
			}
		}

		int64 splashDamage(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 flag) const override {
			switch (getSkillId()) {
				case KR_THUNDERING_ORB:
				case KR_THUNDERING_CALL:
				case AT_ROARING_PIERCER:
				case AT_ROARING_CHARGE: {
					e_skill actual_skill = getSkillId();
					const status_change *sc = status_get_sc(src);
					actual_skill = resolve_thundering_charge_skill(sc, actual_skill);

					return skill_attack(skill_get_type(actual_skill), src, src, target, actual_skill, skill_lv, tick, flag);
				}
				case AT_QUILL_SPEAR: {
					const status_change *sc = status_get_sc(src);
					e_skill actual_skill = resolve_quill_spear_skill(sc, getSkillId());
					return skill_attack(skill_get_type(actual_skill), src, src, target, actual_skill, skill_lv, tick, flag | SD_ANIMATION);
				}
				case AT_PRIMAL_CLAW:
				case AT_FERAL_CLAW:
				case AT_ALPHA_CLAW:
				case AT_TEMPEST_FLAP:
				case AT_QUILL_SPEAR_S:
					return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag | SD_ANIMATION);
				case KR_DOUBLE_SLASH:
				case KR_CLAW_WAVE:
				case KR_SHARPEN_HAIL:
				case KR_TYPHOON_WING:
				case KR_ICE_SPLASH:
				case KR_THUNDERING_ORB_S:
				case KR_THUNDERING_CALL_S:
				case KR_EARTH_STAMP:
				case KR_GROUND_BLOOM:
				case AT_ROARING_PIERCER_S:
				case AT_ROARING_CHARGE_S:
				case AT_GLACIER_MONOLITH:
				case AT_GLACIER_NOVA:
				case AT_GLACIER_SHARD:
				case AT_GLACIER_STOMP:
				case AT_CHILLING_BLAST:
				case AT_FURIOS_STORM:
				case AT_TERRA_WAVE:
				case AT_TERRA_HARVEST:
				case AT_SOLID_STOMP:
				case AT_GRAVITY_HOLE: {
					return skill_attack(skill_get_type(getSkillId()), src, src, target, getSkillId(), skill_lv, tick, flag);
				}
				default:
					break;
			}

			return SkillImplRecursiveDamageSplash::splashDamage(src, target, skill_lv, tick, flag);
		}

	};

	class SkillKarnosNatureProtectionImpl : public SkillImpl {
	public:
		SkillKarnosNatureProtectionImpl() : SkillImpl(KR_NATURE_PROTECTION) {}

		void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const override {
			int32 reduce = 0;
			int32 hp_rate = 5 * skill_lv;
			int32 sp_rate = 3 * skill_lv;

			switch (skill_lv) {
				case 1:
					reduce = 60;
					break;
				case 2:
					reduce = 70;
					break;
				case 3:
					reduce = 80;
					break;
				case 4:
					reduce = 90;
					break;
				default:
					reduce = 99;
					break;
			}

			clif_skill_nodamage(src, *target, getSkillId(), skill_lv);
			sc_start(src, target, SC_NATURE_PROTECTION, 100, reduce, skill_get_time(getSkillId(), skill_lv));

			int64 heal_hp = static_cast<int64>(status_get_max_hp(target)) * hp_rate / 100;
			int64 heal_sp = static_cast<int64>(status_get_max_sp(target)) * sp_rate / 100;
			if (heal_hp > 0 || heal_sp > 0) {
				status_heal(target, heal_hp, heal_sp, 0, 2);
			}
		}
	};

	class SkillAliteaAeroSyncImpl : public SkillImpl {
	public:
		SkillAliteaAeroSyncImpl() : SkillImpl(AT_AERO_SYNC) {}

		void castendNoDamageId(block_list *src, block_list *target, uint16 skill_lv, t_tick tick, int32 &flag) const override {
			applyAeroSync(src, target, skill_lv, tick, flag);
		}

	private:
		void applyAeroSync(block_list *src, block_list *target, uint16 skill_lv, t_tick, int32 flag) const {
			if (flag & 1) {
				return;
			}

			map_session_data *sd = BL_CAST(BL_PC, src);
			map_session_data *tsd = BL_CAST(BL_PC, target);
			status_change *sc = status_get_sc(src);

			if (!sd || !tsd || !sc) {
				return;
			}

				if (!sc->hasSCE(SC_WERERAPTOR) || !sc->hasSCE(SC_FLIP_FLAP) || sc->hasSCE(SC_FLIP_FLAP_TARGET)) {
					clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
					return;
				}

			if (sd->status.party_id == 0 || sd->status.party_id != tsd->status.party_id) {
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
				return;
			}

			const int32 range = skill_get_range2(src, getSkillId(), skill_lv, true);
			if (range > 0 && distance_xy(src->x, src->y, target->x, target->y) > range) {
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
				return;
			}

			if (!unit_movepos(src, target->x, target->y, 2, true)) {
				clif_skill_fail(*sd, getSkillId(), USESKILL_FAIL);
				return;
			}

			clif_blown(src);
			clif_skill_nodamage(src, *target, getSkillId(), skill_lv);

			const int32 flip_lv = pc_checkskill(sd, AT_FLIP_FLAP);
			if (flip_lv <= 0) {
				return;
			}

			const t_tick duration = skill_get_time(AT_FLIP_FLAP, flip_lv);
			sc_start(src, target, SC_FLIP_FLAP_TARGET, 100, flip_lv, duration);
		}
	};
} // namespace

std::unique_ptr<const SkillImpl> SkillFactoryDruid::create(const e_skill skill_id) const {
	switch (skill_id) {
		case DR_AROUND_FLOWER:
			return std::make_unique<SkillAroundFlower>();
		case DR_BLOOD_HOWLING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_CRUEL_BITE:
			return std::make_unique<SkillCruelBite>();
		case DR_CUTTING_WIND:
			return std::make_unique<SkillCuttingWind>();
		case DR_EARTH_FLOWER:
			return std::make_unique<SkillEarthFlower>();
		case DR_ENRAGE_WOLF:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_ENRAGE_RAPTOR:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_FLICKING_TONADO:
			return std::make_unique<SkillFlickingTornado>();
		case DR_HUNGER:
			return std::make_unique<SkillHunger>();
		case DR_ICE_CLOUD:
			return std::make_unique<SkillIceCloud>();
		case DR_ICE_TOTEM:
			return std::make_unique<SkillIceTotem>();
		case DR_LOW_FLIGHT:
			return std::make_unique<SkillLowFlight>();
		case DR_NATURE_SHIELD:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_NOMERCY_CLAW:
			return std::make_unique<SkillNoMercyClaw>();
		case DR_PREENING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case DR_SHOOTING_FEATHER:
			return std::make_unique<SkillShootingFeather>();
		case DR_TRUTH_OF_EARTH:
			return std::make_unique<SkillTruthOfEarth>();
		case DR_TRUTH_OF_ICE:
			return std::make_unique<SkillTruthOfIce>();
		case DR_TRUTH_OF_WIND:
			return std::make_unique<SkillTruthOfWind>();
		case DR_WIND_BOMB:
			return std::make_unique<SkillWindBomb>();
		case DR_WEREWOLF:
			return std::make_unique<SkillTransformationBeast>();
		case DR_WERERAPTOR:
			return std::make_unique<SkillTransformationRaptor>();

		case KR_CHOP_CHOP:
			return std::make_unique<SkillChopChop>();
		case KR_CLAW_WAVE:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_DOUBLE_SLASH:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_EARTH_DRILL:
			return std::make_unique<SkillEarthDrill>();
		case KR_EARTH_STAMP:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_FEATHER_SPRINKLE:
			return std::make_unique<SkillFeatherSprinkle>();
		case KR_GROUND_BLOOM:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_ICE_PILLAR:
			return std::make_unique<SkillIcePillar>();
		case KR_ICE_SPLASH:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_IRON_HOWLING:
			return std::make_unique<StatusSkillImpl>(skill_id);
		case KR_NASTY_SLASH:
			return std::make_unique<SkillNastySlash>();
		case KR_NATURE_PROTECTION:
			return std::make_unique<SkillKarnosNatureProtectionImpl>();
		case KR_SHARPEN_GUST:
			return std::make_unique<SkillSharpenGust>();
		case KR_SHARPEN_HAIL:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_THUNDERING_CALL:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_THUNDERING_CALL_S:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_THUNDERING_FOCUS:
			return std::make_unique<SkillThunderingFocus>();
		case KR_THUNDERING_FOCUS_S:
			return std::make_unique<SkillThunderingFocusS>();
		case KR_THUNDERING_ORB:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_THUNDERING_ORB_S:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_TYPHOON_WING:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case KR_WIND_VEIL:
			return std::make_unique<SkillKarnosNatureProtectionImpl>();

		case AT_AERO_SYNC:
			return std::make_unique<SkillAliteaAeroSyncImpl>();
		case AT_ALPHA_CLAW:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_ALPHA_PHASE:
			return std::make_unique<SkillKarnosNatureProtectionImpl>();
		case AT_APEX_PHASE:
			return std::make_unique<SkillKarnosNatureProtectionImpl>();
		case AT_CHILLING_BLAST:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_FERAL_CLAW:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_FLIP_FLAP:
			return std::make_unique<SkillKarnosNatureProtectionImpl>();
		case AT_FRENZY_FANG:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_FURIOS_STORM:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_GLACIER_MONOLITH:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_GLACIER_NOVA:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_GLACIER_SHARD:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_GLACIER_STOMP:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_GRAVITY_HOLE:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_NATURE_HARMONY:
			return std::make_unique<SkillKarnosNatureProtectionImpl>();
		case AT_PINION_SHOT:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_PRIMAL_CLAW:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_PULSE_OF_MADNESS:
			return std::make_unique<SkillKarnosNatureProtectionImpl>();
		case AT_QUILL_SPEAR:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_QUILL_SPEAR_S:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_ROARING_CHARGE:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_ROARING_CHARGE_S:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_ROARING_PIERCER:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_ROARING_PIERCER_S:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_SAVAGE_LUNGE:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_SOLID_STOMP:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_TEMPEST_FLAP:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_TERRA_HARVEST:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_TERRA_WAVE:
			return std::make_unique<SkillDruidImpl>(skill_id);
		case AT_ZEPHYR_LINK:
			return std::make_unique<SkillDruidImpl>(skill_id);

		default:
			return nullptr;
	}
}
