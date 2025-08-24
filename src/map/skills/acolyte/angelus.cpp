#include "angelus.hpp"

#include "../../status.hpp"
#include "../../clif.hpp"
#include "../../party.hpp"
#include "../../skill.hpp"

SkillAL_ANGELUS::SkillAL_ANGELUS() : SkillImpl(AL_ANGELUS)
{
}

void SkillAL_ANGELUS::castendNoDamageId(struct block_list *src, struct block_list *bl, uint16 skill_lv, t_tick tick, int32 flag) const
{
	struct map_session_data *sd = nullptr;
	struct status_change *tsc = nullptr;

	e_skill skill_id = getSkillId();
	enum sc_type type = skill_get_sc(skill_id);

	if (sd == nullptr || sd->status.party_id == 0 || (flag & 1))
	{

		// Animations don't play when outside visible range
		if (check_distance_bl(src, bl, AREA_SIZE))
			clif_skill_nodamage(bl, *bl, skill_id, skill_lv);

		if (skill_id == SOA_SOUL_OF_HEAVEN_AND_EARTH)
		{
			status_percent_heal(bl, 0, 100);

			if (src != bl && sc != nullptr && sc->getSCE(SC_TOTEM_OF_TUTELARY) != nullptr)
			{
				status_heal(bl, 0, 0, 3 * skill_lv, 0);
			}
		}

		sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
	}
	else if (sd)
		party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag | BCT_PARTY | 1, skill_castend_nodamage_id);
}