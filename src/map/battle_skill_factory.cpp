#include "battle_skill_factory.hpp"
#include "battle_skill_categories.hpp"
#include "battle_skills.hpp"
#include "skill.hpp"

BattleSkillFactory::BattleSkillFactory()
{
    register_all_skills();
}

BattleSkillFactory &BattleSkillFactory::instance()
{
    static BattleSkillFactory instance;
    return instance;
}

std::shared_ptr<BattleSkill> BattleSkillFactory::get_skill(uint16 skill_id) const
{
    std::lock_guard<std::mutex> lock(cache_mutex);
    auto it = skill_cache.find(skill_id);
    if (it != skill_cache.end())
    {
        return it->second;
    }

    return nullptr;
}

bool BattleSkillFactory::has_skill(uint16 skill_id) const
{
    std::lock_guard<std::mutex> lock(creators_mutex);
    return skill_creators.find(skill_id) != skill_creators.end();
}

void BattleSkillFactory::clear_cache()
{
    std::lock_guard<std::mutex> lock(cache_mutex);
    skill_cache.clear();
}

void BattleSkillFactory::register_all_skills()
{
    register_weapon_skills();
    register_magic_skills();
    register_misc_skills();
    register_passive_skills();
}

void BattleSkillFactory::register_weapon_skills()
{
    register_skills<BashSkill>({SM_BASH, MS_BASH});
    // // Bash skills (SM_BASH and MS_BASH use same implementation)
    // REGISTER_WEAPON_SKILL(*this, BashSkill, SM_BASH, MS_BASH);

    // // Pierce skills
    // REGISTER_WEAPON_SKILL(*this, PierceSkill, KN_PIERCE, ML_PIERCE);

    // // Mammonite
    // register_skill<MammoniteSkill>(MC_MAMMONITE);

    // // Magnum Break
    // REGISTER_WEAPON_SKILL(*this, MagnumSkill, SM_MAGNUM, MS_MAGNUM);

    // // Double Attack
    // REGISTER_WEAPON_SKILL(*this, DoubleAttackSkill, AC_DOUBLE, MA_DOUBLE, TF_DOUBLE);

    // // Arrow Shower
    // REGISTER_WEAPON_SKILL(*this, ArrowShowerSkill, AC_SHOWER, MA_SHOWER);

    // // Charge Arrow
    // REGISTER_WEAPON_SKILL(*this, ChargeArrowSkill, AC_CHARGEARROW, MA_CHARGEARROW);

    // // Sonic Blow
    // register_skill<SonicBlowSkill>(AS_SONICBLOW);

    // // Backstab
    // register_skill<BackstabSkill>(RG_BACKSTAP);

    // // Brandish Spear
    // REGISTER_WEAPON_SKILL(*this, BrandishSpearSkill, KN_BRANDISHSPEAR, ML_BRANDISH);

    // // Bowling Bash
    // REGISTER_WEAPON_SKILL(*this, BowlingBashSkill, KN_BOWLINGBASH, MS_BOWLINGBASH);

    // // Spiral Pierce
    // REGISTER_WEAPON_SKILL(*this, SpiralPierceSkill, LK_SPIRALPIERCE, ML_SPIRALPIERCE);

    // // Meteor Assault
    // register_skill<MeteorAssaultSkill>(ASC_METEORASSAULT);

    // // Cart Revolution
    // register_skill<CartRevolutionSkill>(MC_CARTREVOLUTION);

    // // Cart Termination
    // register_skill<CartTerminationSkill>(WS_CARTTERMINATION);

    // // Taekwon skills
    // REGISTER_WEAPON_SKILL(*this, TaekwonKickSkill, TK_DOWNKICK, TK_STORMKICK, TK_TURNKICK, TK_COUNTER);
    // register_skill<JumpKickSkill>(TK_JUMPKICK);

    // // Gunslinger skills
    // register_skill<TripleActionSkill>(GS_TRIPLEACTION);
    // register_skill<BullseyeSkill>(GS_BULLSEYE);
    // register_skill<TrackingSkill>(GS_TRACKING);
    // register_skill<PiercingShotSkill>(GS_PIERCINGSHOT);
    // register_skill<RapidShowerSkill>(GS_RAPIDSHOWER);
    // register_skill<DesperadoSkill>(GS_DESPERADO);
    // register_skill<DustSkill>(GS_DUST);
    // register_skill<FullBusterSkill>(GS_FULLBUSTER);
    // register_skill<SpreadAttackSkill>(GS_SPREADATTACK);

    // // Ninja weapon skills
    // register_skill<HuumaSkill>(NJ_HUUMA);
    // register_skill<TatamiGaeshiSkill>(NJ_TATAMIGAESHI);
    // register_skill<KasumikiriSkill>(NJ_KASUMIKIRI);
    // register_skill<KirikageSkill>(NJ_KIRIKAGE);
}

// Register magic skills
void BattleSkillFactory::register_magic_skills()
{
    // // Bolt spells
    // register_skill<FireboltSkill>(MG_FIREBOLT);
    // register_skill<ColdboltSkill>(MG_COLDBOLT);
    // register_skill<LightningboltSkill>(MG_LIGHTNINGBOLT);

    // // Area spells
    // register_skill<FireballSkill>(MG_FIREBALL);
    // register_skill<FirewallSkill>(MG_FIREWALL);
    // register_skill<ThunderstormSkill>(MG_THUNDERSTORM);
    // register_skill<FrostdiverSkill>(MG_FROSTDIVER);
    // register_skill<StormgustSkill>(WZ_STORMGUST);

    // // Napalm spells
    // register_skill<NapalmBeatSkill>(MG_NAPALMBEAT);
    // register_skill<NapalmVulcanSkill>(HW_NAPALMVULCAN);

    // // Soul Strike
    // register_skill<SoulStrikeSkill>(MG_SOULSTRIKE);

    // // High Wizard skills
    // register_skill<JupitelThunderSkill>(WZ_JUPITEL);
    // register_skill<LordOfVermilionSkill>(WZ_VERMILION);
    // register_skill<MeteorStormSkill>(WZ_METEOR);

    // // Sage skills
    // register_skill<FirepillarSkill>(WZ_FIREPILLAR);
    // register_skill<SightrasherSkill>(WZ_SIGHTRASHER);
    // register_skill<WaterballSkill>(WZ_WATERBALL);
    // register_skill<EarthspikeSkill>(WZ_EARTHSPIKE);
    // register_skill<HeavensDriveSkill>(WZ_HEAVENDRIVE);
}

// Register misc skills
void BattleSkillFactory::register_misc_skills()
{
    // // Ninja misc skills
    // register_skill<ZenyNageSkill>(NJ_ZENYNAGE);

    // // Trap skills
    // register_skill<FiringTrapSkill>(RA_FIRINGTRAP);
    // register_skill<IceboundTrapSkill>(RA_ICEBOUNDTRAP);
    // register_skill<ClusterbombSkill>(RA_CLUSTERBOMB);

    // // Monk misc skills
    // register_skill<ExtremityFistSkill>(MO_EXTREMITYFIST);
}

// Register passive skills
void BattleSkillFactory::register_passive_skills()
{
    // // Masteries and passive bonuses
    // register_skill<WeaponMasterySkill>(SM_SWORD);
    // register_skill<WeaponMasterySkill>(SM_TWOHAND);
    // register_skill<WeaponMasterySkill>(KN_SPEARMASTERY);
    // register_skill<WeaponMasterySkill>(AM_AXEMASTERY);
    // register_skill<WeaponMasterySkill>(PR_MACEMASTERY);
    // register_skill<WeaponMasterySkill>(MO_IRONHAND);
    // register_skill<WeaponMasterySkill>(AS_KATAR);
    // register_skill<WeaponMasterySkill>(BA_MUSICALLESSON);
    // register_skill<WeaponMasterySkill>(DC_DANCINGLESSON);
    // register_skill<WeaponMasterySkill>(SA_ADVANCEDBOOK);
}
