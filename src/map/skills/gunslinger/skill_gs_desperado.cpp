#include "skill_gs_desperado.hpp"

SkillGS_DESPERADO::SkillGS_DESPERADO() : WeaponSkillImpl(GS_DESPERADO) {
}

void SkillGS_DESPERADO::calculateSkillRatio(const Damage* wd, const block_list* src, const block_list* target, uint16 skill_lv, int32& base_skillratio) const {
	const struct status_change *sc = status_get_sc(*src);
	
	base_skillratio += 50 * (skill_lv - 1);
	if (sc && sc->getSCE(SC_FALLEN_ANGEL))
		base_skillratio *= 2;
}
