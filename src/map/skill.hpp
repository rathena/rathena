// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef SKILL_HPP
#define SKILL_HPP

#include <array>
#include <bitset>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/db.hpp>
#include <common/mmo.hpp> // MAX_SKILL, struct square
#include <common/timer.hpp>

#include "map.hpp" // struct block_list

enum damage_lv : uint8;
enum sc_type : int16;
enum send_target : uint8;
enum e_damage_type : uint8;
enum e_battle_flag : uint16;
enum e_battle_check_target : uint32;
class map_session_data;
struct homun_data;
struct skill_unit;
struct s_skill_unit_group;
struct status_change_entry;
class status_change;

#define MAX_SKILL_PRODUCE_DB	300 /// Max Produce DB
#define MAX_PRODUCE_RESOURCE	12 /// Max Produce requirements
#define MAX_SKILL_LEVEL 13 /// Max Skill Level (for skill_db storage)
#define MAX_MOBSKILL_LEVEL 100	/// Max monster skill level (on skill usage)
#define MAX_SKILL_CRIMSON_MARKER 3 /// Max Crimson Marker targets (RL_C_MARKER)
#define SKILL_NAME_LENGTH 40 /// Max Skill Name length
#define SKILL_DESC_LENGTH 40 /// Max Skill Desc length

/// Used with tracking the hitcount of Earthquake for skills that can avoid the first attack
#define NPC_EARTHQUAKE_FLAG 0x800

/// To control alternative skill scalings
#define SKILL_ALTDMG_FLAG 0x10
/// Make skill ignore requirement consumption [Muh]
#define SKILL_NOCONSUME_REQ 0x20
/// Make skill consume ammo, but not the unit [Muh]
#define UNIT_NOCONSUME_AMMO 0x40

/// Constants to identify a skill's nk value (damage properties)
/// The NK value applies only to non INF_GROUND_SKILL skills
/// when determining skill castend function to invoke.
enum e_skill_nk : uint8 {
	NK_NODAMAGE = 0,
	NK_SPLASH,
	NK_SPLASHSPLIT,
	NK_IGNOREATKCARD,
	NK_IGNOREELEMENT,
	NK_IGNOREDEFENSE,
	NK_IGNOREFLEE,
	NK_IGNOREDEFCARD,
	NK_CRITICAL,
	NK_IGNORELONGCARD,
	NK_SIMPLEDEFENSE,
	NK_MAX,
};

/// Constants to identify the skill's inf value.
enum e_skill_inf : uint16 {
	INF_PASSIVE_SKILL = 0x00, // Used just for skill_db parsing
	INF_ATTACK_SKILL  = 0x01,
	INF_GROUND_SKILL  = 0x02,
	INF_SELF_SKILL    = 0x04, // Skills casted on self where target is automatically chosen
	// 0x08 not assigned
	INF_SUPPORT_SKILL = 0x10,
	INF_TRAP_SKILL    = 0x20,
};

/// Constants to identify the skill's inf2 value.
enum e_skill_inf2 : uint8 {
	INF2_ISQUEST = 0,
	INF2_ISNPC, //NPC skills are those that players can't have in their skill tree.
	INF2_ISWEDDING,
	INF2_ISSPIRIT,
	INF2_ISGUILD,
	INF2_ISSONG,
	INF2_ISENSEMBLE,
	INF2_ISTRAP,
	INF2_TARGETSELF, //Refers to ground placed skills that will target the caster as well (like Grandcross)
	INF2_NOTARGETSELF,
	INF2_PARTYONLY,
	INF2_GUILDONLY,
	INF2_NOTARGETENEMY,
	INF2_ISAUTOSHADOWSPELL, // Skill that available for SC_AUTOSHADOWSPELL
	INF2_ISCHORUS, // Chorus skill
	INF2_IGNOREBGREDUCTION, // Skill that ignore bg reduction
	INF2_IGNOREGVGREDUCTION, // Skill that ignore gvg reduction
	INF2_DISABLENEARNPC, // disable to cast skill if near with NPC [Cydh]
	INF2_TARGETTRAP, // can hit trap-type skill (INF2_ISTRAP) [Cydh]
	INF2_IGNORELANDPROTECTOR, // Skill that can ignore Land Protector
	INF2_ALLOWWHENHIDDEN, // Skill that can be use in hiding
	INF2_ALLOWWHENPERFORMING, // Skill that can be use while in dancing state
	INF2_TARGETEMPERIUM, // Skill that could hit emperium
	INF2_IGNOREKAGEHUMI, // Skill blocked by kagehumi
	INF2_ALTERRANGEVULTURE, // Skill range affected by AC_VULTURE
	INF2_ALTERRANGESNAKEEYE, // Skill range affected by GS_SNAKEEYE
	INF2_ALTERRANGESHADOWJUMP, // Skill range affected by NJ_SHADOWJUMP
	INF2_ALTERRANGERADIUS, // Skill range affected by WL_RADIUS
	INF2_ALTERRANGERESEARCHTRAP, // Skill range affected by RA_RESEARCHTRAP
	INF2_IGNOREHOVERING, // Skill that does not affect user that has SC_HOVERING active
	INF2_ALLOWONWARG, // Skill that can be use while riding warg
	INF2_ALLOWONMADO, // Skill that can be used while on Madogear
	INF2_TARGETMANHOLE, // Skill that can be used to target while under SC__MANHOLE effect
	INF2_TARGETHIDDEN, // Skill that affects hidden targets
	INF2_INCREASEDANCEWITHWUGDAMAGE, // Skill that is affected by SC_DANCEWITHWUG
	INF2_IGNOREWUGBITE, // Skill blocked by RA_WUGBITE
	INF2_IGNOREAUTOGUARD , // Skill is not blocked by SC_AUTOGUARD (physical-skill only)
	INF2_IGNORECICADA, // Skill is not blocked by SC_UTSUSEMI or SC_BUNSINJYUTSU (physical-skill only)
	INF2_SHOWSCALE, // Skill shows AoE area while casting
	INF2_IGNOREGTB, // Skill ignores effect of GTB
	INF2_TOGGLEABLE, // Skill can be toggled on and off (won't consume HP/SP when toggled off)
	INF2_MAX,
};

/// Constants for skill requirements
enum e_skill_require : uint16 {
	SKILL_REQ_HPCOST = 0x1,
	SKILL_REQ_SPCOST = 0x2,
	SKILL_REQ_HPRATECOST = 0x4,
	SKILL_REQ_SPRATECOST = 0x8,
	SKILL_REQ_MAXHPTRIGGER = 0x10,
	SKILL_REQ_ZENYCOST = 0x20,
	SKILL_REQ_WEAPON = 0x40,
	SKILL_REQ_AMMO = 0x80,
	SKILL_REQ_STATE = 0x100,
	SKILL_REQ_STATUS = 0x200,
	SKILL_REQ_SPIRITSPHERECOST = 0x400,
	SKILL_REQ_ITEMCOST = 0x800,
	SKILL_REQ_EQUIPMENT = 0x1000,
	SKILL_REQ_APCOST = 0x2000,
	SKILL_REQ_APRATECOST = 0x4000,
};

/// Constants for skill cast near NPC.
enum e_skill_nonear_npc : uint8 {
	SKILL_NONEAR_WARPPORTAL = 0x1,
	SKILL_NONEAR_SHOP = 0x2,
	SKILL_NONEAR_NPC = 0x4,
	SKILL_NONEAR_TOMB = 0x8,
};

/// Constants for skill cast adjustments.
enum e_skill_cast_flags : uint8 {
	SKILL_CAST_IGNOREDEX = 0x1,
	SKILL_CAST_IGNORESTATUS = 0x2,
	SKILL_CAST_IGNOREITEMBONUS = 0x4,
};

/// Constants for skill copyable options.
enum e_skill_copyable_option : uint8 {
	SKILL_COPY_PLAGIARISM = 0x1,
	SKILL_COPY_REPRODUCE = 0x2,
};

/// Constants for skill unit flags.
enum e_skill_unit_flag : uint8 {
	UF_NONE = 0,
	UF_NOENEMY,	// If 'defunit_not_enemy' is set, the target is changed to 'friend'
	UF_NOREITERATION,	// Spell cannot be stacked
	UF_NOFOOTSET,	// Spell cannot be cast near/on targets
	UF_NOOVERLAP,	// Spell effects do not overlap
	UF_PATHCHECK,	// Only cells with a shootable path will be placed
	UF_NOPC,	// May not target players
	UF_NOMOB,	// May not target mobs
	UF_SKILL,	// May target skills
	UF_DANCE,	// Dance
	UF_ENSEMBLE,	// Duet
	UF_SONG,	// Song
	UF_DUALMODE,	// Spells should trigger both ontimer and onplace/onout/onleft effects.
	UF_NOKNOCKBACK,	// Skill unit cannot be knocked back
	UF_RANGEDSINGLEUNIT,	// hack for ranged layout, only display center
	UF_CRAZYWEEDIMMUNE,	// Immune to Crazy Weed removal
	UF_REMOVEDBYFIRERAIN,	// removed by Fire Rain
	UF_KNOCKBACKGROUP,	// knockback skill unit with its group instead of single unit
	UF_HIDDENTRAP,	// Hidden trap [Cydh]
	UF_MAX,
};

/// Time that's added to canact delay on castbegin and substracted on castend
/// This is to prevent hackers from sending a skill packet after cast but before a timer triggers castend
const t_tick SECURITY_CASTTIME = 100;

/// Flags passed to skill_attack/skill_area_sub
enum e_skill_display {
	SD_LEVEL     = 0x1000, // skill_attack will send -1 instead of skill level (affects display of some skills)
	SD_ANIMATION = 0x2000, // skill_attack will use '5' instead of the skill's 'type' (this makes skills show an animation). Also being used in skill_attack for splash skill (NK_SPLASH) to check status_check_skilluse
	SD_SPLASH    = 0x4000, // skill_area_sub will count targets in skill_area_temp[2]
	SD_PREAMBLE  = 0x8000, // skill_area_sub will transmit a 'magic' damage packet (-30000 dmg) for the first target selected
};

#define MAX_SKILL_ITEM_REQUIRE	10 /// Maximum required items
#define MAX_SKILL_STATUS_REQUIRE 3 /// Maximum required statuses
#define MAX_SKILL_EQUIP_REQUIRE 10 /// Maximum required equipped item

/// Single skill requirement. !TODO: Cleanup the variable types
struct s_skill_condition {
	int32 hp;								///< HP cost
	int32 mhp;								///< Max HP to trigger
	int32 sp;								/// SP cost
	int32 ap;								/// AP cost
	int32 hp_rate;							/// HP cost (%)
	int32 sp_rate;							/// SP cost (%)
	int32 ap_rate;							/// AP cost (%)
	int32 zeny;								/// Zeny cost
	int32 weapon;							/// Weapon type. Combined bitmask of enum weapon_type (1<<weapon)
	int32 ammo;								/// Ammo type. Combine bitmask of enum ammo_type (1<<ammo)
	int32 ammo_qty;							/// Amount of ammo
	int32 state;							/// State/condition. @see enum e_require_state
	int32 spiritball;						/// Spiritball cost
	t_itemid itemid[MAX_SKILL_ITEM_REQUIRE];	/// Required item
	int32 amount[MAX_SKILL_ITEM_REQUIRE];	/// Amount of item
	std::vector<t_itemid> eqItem;				/// List of equipped item
	std::vector<sc_type> status;			/// List of Status required (SC)
};

/// Skill requirement structure.
struct s_skill_require {
	int32 hp[MAX_SKILL_LEVEL];				///< HP cost
	int32 mhp[MAX_SKILL_LEVEL];				///< Max HP to trigger
	int32 sp[MAX_SKILL_LEVEL];				/// SP cost
	int32 ap[MAX_SKILL_LEVEL];				/// AP cost
	int32 hp_rate[MAX_SKILL_LEVEL];			/// HP cost (%)
	int32 sp_rate[MAX_SKILL_LEVEL];			/// SP cost (%)
	int32 ap_rate[MAX_SKILL_LEVEL];			/// AP cost (%)
	int32 zeny[MAX_SKILL_LEVEL];			/// Zeny cost
	int32 weapon;							/// Weapon type. Combined bitmask of enum weapon_type (1<<weapon)
	int32 ammo;								/// Ammo type. Combine bitmask of enum ammo_type (1<<ammo)
	int32 ammo_qty[MAX_SKILL_LEVEL];		/// Amount of ammo
	int32 state;							/// State/condition. @see enum e_require_state
	int32 spiritball[MAX_SKILL_LEVEL];		/// Spiritball cost
	t_itemid itemid[MAX_SKILL_ITEM_REQUIRE];	/// Required item
	int32 amount[MAX_SKILL_ITEM_REQUIRE];	/// Amount of item
	std::vector<t_itemid> eqItem;				/// List of equipped item
	std::vector<sc_type> status;			/// List of Status required (SC)
	bool itemid_level_dependent;			/// If the ItemCost is skill level dependent or not.
};

/// Skill Copyable structure.
struct s_skill_copyable { // [Cydh]
	uint8 option;
	uint16 req_opt;
};

/// Database skills
struct s_skill_db {
	uint16 nameid;								///< Skill ID
	char name[SKILL_NAME_LENGTH];				///< AEGIS_Name
	char desc[SKILL_DESC_LENGTH];				///< English Name
	int32 range[MAX_SKILL_LEVEL];				///< Range
	e_damage_type hit;							///< Hit type
	uint16 inf;									///< Inf: 0- passive, 1- enemy, 2- place, 4- self, 16- friend, 32- trap
	e_element element[MAX_SKILL_LEVEL];			///< Element
	std::bitset<NK_MAX> nk;						///< Damage properties
	int32 splash[MAX_SKILL_LEVEL];				///< Splash effect
	uint16 max;									///< Max level
	int32 num[MAX_SKILL_LEVEL];					///< Number of hit
	bool castcancel;							///< Cancel cast when being hit
	uint16 cast_def_rate;						///< Def rate during cast a skill
	e_battle_flag skill_type;					///< Skill type
	int32 blewcount[MAX_SKILL_LEVEL];			///< Blew count
	std::bitset<INF2_MAX> inf2;					///< Skill flags @see enum e_skill_inf2
	int32 maxcount[MAX_SKILL_LEVEL];			///< Max number skill can be casted in same map

	uint8 castnodex;							///< 1 - Not affected by dex, 2 - Not affected by SC, 4 - Not affected by item
	uint8 delaynodex;							///< 1 - Not affected by dex, 2 - Not affected by SC, 4 - Not affected by item

	// skill_nocast_db.txt
	uint32 nocast;								///< Skill cannot be casted at this zone

	int32 giveap[MAX_SKILL_LEVEL];				///< AP Given On Use

	uint16 unit_id;								///< Unit ID. @see enum e_skill_unit_id
	uint16 unit_id2;							///< Alternate unit ID. @see enum e_skill_unit_id
	int32 unit_layout_type[MAX_SKILL_LEVEL];	///< Layout type. -1 is special layout, others are square with lenght*width: (val*2+1)^2
	int32 unit_range[MAX_SKILL_LEVEL];			///< Unit cell effect range
	int16 unit_interval;						///< Interval
	int32 unit_target;							///< Unit target.
	std::bitset<UF_MAX> unit_flag;				///< Unit flags.

	int32 cast[MAX_SKILL_LEVEL];				///< Variable casttime
	int32 delay[MAX_SKILL_LEVEL];				///< Global delay (delay before reusing all skills)
	int32 walkdelay[MAX_SKILL_LEVEL];			///< Delay to walk after casting
	int32 upkeep_time[MAX_SKILL_LEVEL];			///< Duration
	int32 upkeep_time2[MAX_SKILL_LEVEL];		///< Duration2
	int32 cooldown[MAX_SKILL_LEVEL];			///< Cooldown (delay before reusing same skill)
#ifdef RENEWAL_CAST
	int32 fixed_cast[MAX_SKILL_LEVEL];			///< If -1 means 20% than 'cast'
#endif

	struct s_skill_require require;				///< Skill requirement

	uint16 unit_nonearnpc_range;				///< Additional range for UF_NONEARNPC or INF2_DISABLENEARNPC [Cydh]
	uint16 unit_nonearnpc_type;					///< Type of NPC [Cydh]

	struct s_skill_damage damage;
	struct s_skill_copyable copyable;

	int32 abra_probability[MAX_SKILL_LEVEL];
	uint16 improvisedsong_rate;
	sc_type sc;									///< Default SC for skill
};

class SkillDatabase : public TypesafeCachedYamlDatabase <uint16, s_skill_db> {
private:
	/// Skill ID to Index lookup: skill_index = skill_get_index(skill_id) - [FWI] 20160423 the whole index thing should be removed.
	uint16 skilldb_id2idx[(UINT16_MAX + 1)];
	/// Skill count, also as last index
	uint16 skill_num;

	template<typename T, size_t S> bool parseNode(const std::string& nodeName, const std::string& subNodeName, const ryml::NodeRef& node, T(&arr)[S]);

public:
	SkillDatabase() : TypesafeCachedYamlDatabase("SKILL_DB", 4) {
		this->clear();
	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	void clear() override;
	void loadingFinished() override;

	// Additional
	uint16 get_index( uint16 skill_id, bool silent, const char* func, const char* file, int32 line );
};

extern SkillDatabase skill_db;

#define MAX_SQUARE_LAYOUT		7	// 15*15 unit placement maximum
#define MAX_SKILL_UNIT_LAYOUT	(48+MAX_SQUARE_LAYOUT)	// 47 special ones + the square ones
#define MAX_SKILL_UNIT_LAYOUT2	17
#define MAX_SKILL_UNIT_COUNT	((MAX_SQUARE_LAYOUT*2+1)*(MAX_SQUARE_LAYOUT*2+1))
struct s_skill_unit_layout {
	int32 count;
	int32 dx[MAX_SKILL_UNIT_COUNT];
	int32 dy[MAX_SKILL_UNIT_COUNT];
};

struct s_skill_nounit_layout {
	int32 count;
	int32 dx[MAX_SKILL_UNIT_COUNT];
	int32 dy[MAX_SKILL_UNIT_COUNT];
};

#define MAX_SKILLTIMERSKILL 50
struct skill_timerskill {
	int32 timer;
	int32 src_id;
	int32 target_id;
	int32 map;
	int16 x,y;
	uint16 skill_id,skill_lv;
	int32 type; // a BF_ type (NOTE: some places use this as general-purpose storage...)
	int32 flag;
};

/// Skill unit
struct skill_unit : public block_list {
	std::shared_ptr<s_skill_unit_group> group; /// Skill group reference
	t_tick limit;
	int32 val1, val2;
	int16 range;
	bool alive;
	bool hidden;
};

/// Skill unit group
struct s_skill_unit_group {
	int32 src_id; /// Caster ID/RID, if player is account_id
	int32 party_id; /// Party ID
	int32 guild_id; /// Guild ID
	int32 bg_id; /// Battleground ID
	int32 map; /// Map
	int32 target_flag; /// Holds BCT_* flag for battle_check_target
	int32 bl_flag; /// Holds BL_* flag for map_foreachin* functions
	t_tick tick; /// Tick when skill unit initialized
	t_tick limit; /// Life time
	int32 interval; /// Timer interval
	uint16 skill_id, /// Skill ID
		skill_lv; /// Skill level
	int32 val1, val2, val3; /// Values
	char *valstr; /// String value, used for HT_TALKIEBOX & RG_GRAFFITI
	int32 unit_id; /// Unit ID (for client effect)
	int32 group_id; /// Skill Group ID
	int32 link_group_id; /// Linked group that should be deleted if this one is deleted
	int32 unit_count, /// Number of unit at this group
		alive_count; /// Number of alive unit
	t_itemid item_id; /// Store item used.
	struct skill_unit *unit; /// Skill Unit
	struct {
		unsigned ammo_consume : 1; // Need to consume ammo
		unsigned song_dance : 2; //0x1 Song/Dance, 0x2 Ensemble
		unsigned guildaura : 1; // Guild Aura
	} state;

	~s_skill_unit_group() {
		if (this->unit)
			map_freeblock(this->unit); // schedules deallocation of whole array (HACK)
	}
};

#define MAX_SKILLUNITGROUPTICKSET 25
struct skill_unit_group_tickset {
	t_tick tick;
	int32 id;
};

/// Ring of Nibelungen bonuses
enum e_nibelungen_status : uint8 {
	RINGNBL_ASPDRATE = 1,		///< ASPD + 20%
	RINGNBL_ATKRATE,		///< Physical damage + 20%
	RINGNBL_MATKRATE,		///< MATK + 20%
	RINGNBL_HPRATE,			///< Maximum HP + 30%
	RINGNBL_SPRATE,			///< Maximum SP + 30%
	RINGNBL_ALLSTAT,		///< All stats + 15
	RINGNBL_HIT,			///< HIT + 50
	RINGNBL_FLEE,			///< FLEE + 50
	RINGNBL_SPCONSUM,		///< SP consumption - 30%
	RINGNBL_HPREGEN,		///< HP recovery + 100%
	RINGNBL_SPREGEN,		///< SP recovery + 100%
	RINGNBL_MAX,
};

/// Enum for skill_blown
enum e_skill_blown	{
	BLOWN_NONE					= 0x00,
	BLOWN_DONT_SEND_PACKET		= 0x01, // Position update packets must not be sent to the client
	BLOWN_IGNORE_NO_KNOCKBACK	= 0x02, // Ignores players' special_state.no_knockback
	// These flags return 'count' instead of 0 if target is cannot be knocked back
	BLOWN_NO_KNOCKBACK_MAP		= 0x04, // On a WoE/BG map
	BLOWN_MD_KNOCKBACK_IMMUNE	= 0x08, // If target is MD_KNOCKBACK_IMMUNE
	BLOWN_TARGET_NO_KNOCKBACK	= 0x10, // If target has 'special_state.no_knockback'
	BLOWN_TARGET_BASILICA		= 0x20, // If target is in Basilica area
};

// Enum for skill_dance_overlap flag parameter
enum e_dance_overlap : int32 {
	OVERLAP_REMOVE = 0, // Skill unit is about to be removed, remove overlap marker from overlapping units on the cell if applicable
	OVERLAP_SET, // Skill unit was placed, add overlap marker to overlapping units on the cell
	OVERLAP_COUNT, // Don't change overlap marker, just count units overlapping with skill unit (excluding itself)
};

/// Create Database item
struct s_skill_produce_db {
	t_itemid nameid; /// Product ID
	uint16 req_skill; /// Required Skill
	unsigned char req_skill_lv, /// Required Skill Level
		itemlv; /// Item Level
	t_itemid mat_id[MAX_PRODUCE_RESOURCE]; /// Materials needed
	uint16 mat_amount[MAX_PRODUCE_RESOURCE]; /// Amount of each materials
};
extern struct s_skill_produce_db skill_produce_db[MAX_SKILL_PRODUCE_DB];

/// Creating database arrow
struct s_skill_arrow_db {
	t_itemid nameid; /// Material ID
	std::unordered_map<t_itemid, uint16> created; /// Arrow created
};

class SkillArrowDatabase : public TypesafeYamlDatabase<t_itemid, s_skill_arrow_db> {
public:
	SkillArrowDatabase() : TypesafeYamlDatabase("CREATE_ARROW_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
};

extern SkillArrowDatabase skill_arrow_db;

/// Abracadabra database
struct s_skill_abra_db {
	uint16 skill_id; /// Skill ID
	std::array<uint16, MAX_SKILL_LEVEL> per; /// Probability summoned
};

class AbraDatabase : public TypesafeYamlDatabase<uint16, s_skill_abra_db> {
public:
	AbraDatabase() : TypesafeYamlDatabase("ABRA_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
};

void do_init_skill(void);
void do_final_skill(void);

/// Cast type
enum e_cast_type { CAST_GROUND, CAST_DAMAGE, CAST_NODAMAGE };
/// Returns the cast type of the skill: ground cast, castend damage, castend no damage
e_cast_type skill_get_casttype(uint16 skill_id); //[Skotlex]
const char*	skill_get_name( uint16 skill_id ); 	// [Skotlex]
const char*	skill_get_desc( uint16 skill_id ); 	// [Skotlex]
int32 skill_tree_get_max( uint16 skill_id, int32 b_class );	// Celest

// Accessor to the skills database
#define skill_get_index(skill_id) skill_db.get_index((skill_id), false, __FUNCTION__, __FILE__, __LINE__) /// Get skill index from skill_id (common usage on source)
int32 skill_get_type( uint16 skill_id );
e_damage_type skill_get_hit( uint16 skill_id );
int32 skill_get_inf( uint16 skill_id );
int32 skill_get_ele( uint16 skill_id , uint16 skill_lv );
int32 skill_get_max( uint16 skill_id );
int32 skill_get_range( uint16 skill_id , uint16 skill_lv );
int32 skill_get_range2(struct block_list *bl, uint16 skill_id, uint16 skill_lv, bool isServer);
int32 skill_get_splash( uint16 skill_id , uint16 skill_lv );
int32 skill_get_num( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_cast( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_delay( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_walkdelay( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_time( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_time2( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_castnodex( uint16 skill_id );
int32 skill_get_castdef( uint16 skill_id );
int32 skill_get_nocast( uint16 skill_id );
int32 skill_get_unit_id( uint16 skill_id );
int32 skill_get_unit_id2( uint16 skill_id );
int32 skill_get_castcancel( uint16 skill_id );
int32 skill_get_maxcount( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_blewcount( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_cooldown( uint16 skill_id, uint16 skill_lv );
int32 skill_get_giveap( uint16 skill_id, uint16 skill_lv );
int32 skill_get_unit_target( uint16 skill_id );
#define skill_get_nk(skill_id, nk) skill_get_nk_(skill_id, { nk })
bool skill_get_nk_(uint16 skill_id, std::vector<e_skill_nk> nk);
#define skill_get_inf2(skill_id, inf2) skill_get_inf2_(skill_id, { inf2 })
bool skill_get_inf2_(uint16 skill_id, std::vector<e_skill_inf2> inf2);
#define skill_get_unit_flag(skill_id, unit) skill_get_unit_flag_(skill_id, { unit })
bool skill_get_unit_flag_(uint16 skill_id, std::vector<e_skill_unit_flag> unit);
int32 skill_get_unit_range(uint16 skill_id, uint16 skill_lv);
// Accessor for skill requirements
int32 skill_get_hp( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_mhp( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_sp( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_ap( uint16 skill_id, uint16 skill_lv );
int32 skill_get_hp_rate( uint16 skill_id, uint16 skill_lv );
int32 skill_get_sp_rate( uint16 skill_id, uint16 skill_lv );
int32 skill_get_ap_rate( uint16 skill_id, uint16 skill_lv );
int32 skill_get_zeny( uint16 skill_id ,uint16 skill_lv );
int32 skill_get_weapontype( uint16 skill_id );
int32 skill_get_ammotype( uint16 skill_id );
int32 skill_get_ammo_qty( uint16 skill_id, uint16 skill_lv );
int32 skill_get_state(uint16 skill_id);
size_t skill_get_status_count( uint16 skill_id );
int32 skill_get_spiritball( uint16 skill_id, uint16 skill_lv );
uint16 skill_dummy2skill_id(uint16 skill_id);

uint16 skill_name2id(const char* name);

int32 skill_isammotype(map_session_data *sd, uint16 skill_id);
TIMER_FUNC(skill_castend_id);
TIMER_FUNC(skill_castend_pos);
TIMER_FUNC( skill_keep_using );
int32 skill_castend_map( map_session_data *sd,uint16 skill_id, const char *map);

int32 skill_cleartimerskill(struct block_list *src);
int32 skill_addtimerskill(struct block_list *src,t_tick tick,int32 target,int32 x,int32 y,uint16 skill_id,uint16 skill_lv,int32 type,int32 flag);

// Results? Added
int32 skill_additional_effect( struct block_list* src, struct block_list *bl,uint16 skill_id,uint16 skill_lv,int32 attack_type,enum damage_lv dmg_lv,t_tick tick);
int32 skill_counter_additional_effect( struct block_list* src, struct block_list *bl,uint16 skill_id,uint16 skill_lv,int32 attack_type,t_tick tick);
int16 skill_blown(struct block_list* src, struct block_list* target, char count, int8 dir, enum e_skill_blown flag);
int32 skill_break_equip(struct block_list *src,struct block_list *bl, uint16 where, int32 rate, int32 flag);
int32 skill_strip_equip(struct block_list *src,struct block_list *bl, uint16 where, int32 rate, int32 lv, int32 time);
// Skills unit
std::shared_ptr<s_skill_unit_group> skill_id2group(int32 group_id);
std::shared_ptr<s_skill_unit_group> skill_unitsetting(struct block_list* src, uint16 skill_id, uint16 skill_lv, int16 x, int16 y, int32 flag);
skill_unit* skill_initunit(std::shared_ptr<s_skill_unit_group> group, int32 idx, int32 x, int32 y, int32 val1, int32 val2, bool hidden, int32 range, t_tick limit);
int32 skill_delunit(struct skill_unit *unit);
std::shared_ptr<s_skill_unit_group> skill_initunitgroup(struct block_list* src, int32 count, uint16 skill_id, uint16 skill_lv, int32 unit_id, t_tick limit, int32 interval);
int32 skill_delunitgroup_(std::shared_ptr<s_skill_unit_group> group, const char* file, int32 line, const char* func);
#define skill_delunitgroup(group) skill_delunitgroup_(group,__FILE__,__LINE__,__func__)
void skill_clear_unitgroup(struct block_list *src);
int32 skill_clear_group(block_list *bl, uint8 flag);
void ext_skill_unit_onplace(struct skill_unit *unit, struct block_list *bl, t_tick tick);
int64 skill_unit_ondamaged(struct skill_unit *unit,int64 damage);

// Skill unit visibility [Cydh]
void skill_getareachar_skillunit_visibilty(struct skill_unit *su, enum send_target target);
void skill_getareachar_skillunit_visibilty_single(struct skill_unit *su, struct block_list *bl);

int32 skill_castfix(struct block_list *bl, uint16 skill_id, uint16 skill_lv);
int32 skill_castfix_sc(struct block_list *bl, double time, uint8 flag);
#ifdef RENEWAL_CAST
int32 skill_vfcastfix(struct block_list *bl, double time, uint16 skill_id, uint16 skill_lv);
#endif
int32 skill_delayfix(struct block_list *bl, uint16 skill_id, uint16 skill_lv);
void skill_toggle_magicpower(struct block_list *bl, uint16 skill_id);
//Check sc of bl [Muh]
int32 skill_check_bl_sc(struct block_list *target, va_list ap);

// Skill conditions check and remove [Inkfish]
bool skill_check_condition_castbegin( map_session_data& sd, uint16 skill_id, uint16 skill_lv );
bool skill_check_condition_castend( map_session_data& sd, uint16 skill_id, uint16 skill_lv );
int32 skill_check_condition_char_sub (struct block_list *bl, va_list ap);
void skill_consume_requirement(map_session_data *sd, uint16 skill_id, uint16 skill_lv, int16 type);
struct s_skill_condition skill_get_requirement(map_session_data *sd, uint16 skill_id, uint16 skill_lv);
bool skill_disable_check(status_change &sc, uint16 skill_id);
bool skill_pos_maxcount_check(struct block_list *src, int16 x, int16 y, uint16 skill_id, uint16 skill_lv, enum bl_type type, bool display_failure);

int32 skill_check_pc_partner(map_session_data *sd, uint16 skill_id, uint16 *skill_lv, int32 range, int32 cast_flag);
int32 skill_unit_move(struct block_list *bl,t_tick tick,int32 flag);
void skill_unit_move_unit_group( std::shared_ptr<s_skill_unit_group> group, int16 m,int16 dx,int16 dy);
void skill_unit_move_unit(struct block_list *bl, int32 dx, int32 dy);

int32 skill_sit(map_session_data *sd, bool sitting);
void skill_repairweapon( map_session_data& sd, int32 idx );
void skill_identify(map_session_data *sd,int32 idx);
void skill_weaponrefine( map_session_data& sd, int32 idx ); // [Celest]
int32 skill_autospell(map_session_data *md,uint16 skill_id);

int32 skill_calc_heal(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, bool heal);

bool skill_check_cloaking(struct block_list *bl, struct status_change_entry *sce);
int8 skill_isCopyable(map_session_data *sd, uint16 skill_id);

// Abnormal status
bool skill_isNotOk( uint16 skill_id, map_session_data& sd );
bool skill_isNotOk_hom(struct homun_data *hd, uint16 skill_id, uint16 skill_lv);
bool skill_isNotOk_mercenary( uint16 skill_id, s_mercenary_data& md);

bool skill_isNotOk_npcRange(struct block_list *src, uint16 skill_id, uint16 skill_lv, int32 pos_x, int32 pos_y);

// Item creation
int16 skill_can_produce_mix( map_session_data *sd, t_itemid nameid, int32 trigger, int32 qty);
bool skill_produce_mix( map_session_data *sd, uint16 skill_id, t_itemid nameid, int32 slot1, int32 slot2, int32 slot3, int32 qty, int16 produce_idx );

bool skill_arrow_create( map_session_data *sd, t_itemid nameid);

// skills for the mob
int32 skill_castend_nodamage_id( struct block_list *src, struct block_list *bl,uint16 skill_id,uint16 skill_lv,t_tick tick,int32 flag );
int32 skill_castend_damage_id( struct block_list* src, struct block_list *bl,uint16 skill_id,uint16 skill_lv,t_tick tick,int32 flag );
int32 skill_castend_pos2( struct block_list *src, int32 x,int32 y,uint16 skill_id,uint16 skill_lv,t_tick tick,int32 flag);

bool skill_blockpc_start(map_session_data &sd, uint16 skill_id, t_tick tick);
void skill_blockpc_clear(map_session_data &sd);
TIMER_FUNC(skill_blockpc_end);
bool skill_blockhomun_start(homun_data &hd, uint16 skill_id, t_tick tick);
void skill_blockhomun_clear(homun_data &hd);
TIMER_FUNC(skill_blockhomun_end);
bool skill_blockmerc_start(s_mercenary_data &mc, uint16 skill_id, t_tick tick);
void skill_blockmerc_clear(s_mercenary_data &mc);
TIMER_FUNC(skill_blockmerc_end);

// (Epoque:) To-do: replace this macro with some sort of skill tree check (rather than hard-coded skill names)
#define skill_ischangesex(id) ( \
	((id) >= BD_ADAPTATION     && (id) <= DC_SERVICEFORYOU) || ((id) >= CG_ARROWVULCAN && (id) <= CG_MARIONETTE) || \
	((id) >= CG_LONGINGFREEDOM && (id) <= CG_TAROTCARD)     || ((id) >= WA_SWING_DANCE && (id) <= WM_UNLIMITED_HUMMING_VOICE))

// Skill action, (return dmg,heal)
int64 skill_attack( int32 attack_type, struct block_list* src, struct block_list *dsrc,struct block_list *bl,uint16 skill_id,uint16 skill_lv,t_tick tick,int32 flag );

void skill_reload(void);

/// List of State Requirements
enum e_require_state : uint8 {
	ST_NONE,
	ST_HIDDEN,
	ST_RIDING,
	ST_FALCON,
	ST_CART,
	ST_SHIELD,
	ST_RECOVER_WEIGHT_RATE,
	ST_MOVE_ENABLE,
	ST_WATER,
	ST_RIDINGDRAGON,
	ST_WUG,
	ST_RIDINGWUG,
	ST_MADO,
	ST_ELEMENTALSPIRIT,
	ST_ELEMENTALSPIRIT2,
	ST_PECO,
	ST_SUNSTANCE,
	ST_MOONSTANCE,
	ST_STARSTANCE,
	ST_UNIVERSESTANCE
};

/// List of Skills
enum e_skill {
	NV_BASIC = 1,

	SM_SWORD,
	SM_TWOHAND,
	SM_RECOVERY,
	SM_BASH,
	SM_PROVOKE,
	SM_MAGNUM,
	SM_ENDURE,

	MG_SRECOVERY,
	MG_SIGHT,
	MG_NAPALMBEAT,
	MG_SAFETYWALL,
	MG_SOULSTRIKE,
	MG_COLDBOLT,
	MG_FROSTDIVER,
	MG_STONECURSE,
	MG_FIREBALL,
	MG_FIREWALL,
	MG_FIREBOLT,
	MG_LIGHTNINGBOLT,
	MG_THUNDERSTORM,

	AL_DP,
	AL_DEMONBANE,
	AL_RUWACH,
	AL_PNEUMA,
	AL_TELEPORT,
	AL_WARP,
	AL_HEAL,
	AL_INCAGI,
	AL_DECAGI,
	AL_HOLYWATER,
	AL_CRUCIS,
	AL_ANGELUS,
	AL_BLESSING,
	AL_CURE,

	MC_INCCARRY,
	MC_DISCOUNT,
	MC_OVERCHARGE,
	MC_PUSHCART,
	MC_IDENTIFY,
	MC_VENDING,
	MC_MAMMONITE,

	AC_OWL,
	AC_VULTURE,
	AC_CONCENTRATION,
	AC_DOUBLE,
	AC_SHOWER,

	TF_DOUBLE,
	TF_MISS,
	TF_STEAL,
	TF_HIDING,
	TF_POISON,
	TF_DETOXIFY,

	ALL_RESURRECTION,

	KN_SPEARMASTERY,
	KN_PIERCE,
	KN_BRANDISHSPEAR,
	KN_SPEARSTAB,
	KN_SPEARBOOMERANG,
	KN_TWOHANDQUICKEN,
	KN_AUTOCOUNTER,
	KN_BOWLINGBASH,
	KN_RIDING,
	KN_CAVALIERMASTERY,

	PR_MACEMASTERY,
	PR_IMPOSITIO,
	PR_SUFFRAGIUM,
	PR_ASPERSIO,
	PR_BENEDICTIO,
	PR_SANCTUARY,
	PR_SLOWPOISON,
	PR_STRECOVERY,
	PR_KYRIE,
	PR_MAGNIFICAT,
	PR_GLORIA,
	PR_LEXDIVINA,
	PR_TURNUNDEAD,
	PR_LEXAETERNA,
	PR_MAGNUS,

	WZ_FIREPILLAR,
	WZ_SIGHTRASHER,
	WZ_FIREIVY,
	WZ_METEOR,
	WZ_JUPITEL,
	WZ_VERMILION,
	WZ_WATERBALL,
	WZ_ICEWALL,
	WZ_FROSTNOVA,
	WZ_STORMGUST,
	WZ_EARTHSPIKE,
	WZ_HEAVENDRIVE,
	WZ_QUAGMIRE,
	WZ_ESTIMATION,

	BS_IRON,
	BS_STEEL,
	BS_ENCHANTEDSTONE,
	BS_ORIDEOCON,
	BS_DAGGER,
	BS_SWORD,
	BS_TWOHANDSWORD,
	BS_AXE,
	BS_MACE,
	BS_KNUCKLE,
	BS_SPEAR,
	BS_HILTBINDING,
	BS_FINDINGORE,
	BS_WEAPONRESEARCH,
	BS_REPAIRWEAPON,
	BS_SKINTEMPER,
	BS_HAMMERFALL,
	BS_ADRENALINE,
	BS_WEAPONPERFECT,
	BS_OVERTHRUST,
	BS_MAXIMIZE,

	HT_SKIDTRAP,
	HT_LANDMINE,
	HT_ANKLESNARE,
	HT_SHOCKWAVE,
	HT_SANDMAN,
	HT_FLASHER,
	HT_FREEZINGTRAP,
	HT_BLASTMINE,
	HT_CLAYMORETRAP,
	HT_REMOVETRAP,
	HT_TALKIEBOX,
	HT_BEASTBANE,
	HT_FALCON,
	HT_STEELCROW,
	HT_BLITZBEAT,
	HT_DETECTING,
	HT_SPRINGTRAP,

	AS_RIGHT,
	AS_LEFT,
	AS_KATAR,
	AS_CLOAKING,
	AS_SONICBLOW,
	AS_GRIMTOOTH,
	AS_ENCHANTPOISON,
	AS_POISONREACT,
	AS_VENOMDUST,
	AS_SPLASHER,

	NV_FIRSTAID,
	NV_TRICKDEAD,
	SM_MOVINGRECOVERY,
	SM_FATALBLOW,
	SM_AUTOBERSERK,
	AC_MAKINGARROW,
	AC_CHARGEARROW,
	TF_SPRINKLESAND,
	TF_BACKSLIDING,
	TF_PICKSTONE,
	TF_THROWSTONE,
	MC_CARTREVOLUTION,
	MC_CHANGECART,
	MC_LOUD,
	AL_HOLYLIGHT,
	MG_ENERGYCOAT,

	NPC_PIERCINGATT,
	NPC_MENTALBREAKER,
	NPC_RANGEATTACK,
	NPC_ATTRICHANGE,
	NPC_CHANGEWATER,
	NPC_CHANGEGROUND,
	NPC_CHANGEFIRE,
	NPC_CHANGEWIND,
	NPC_CHANGEPOISON,
	NPC_CHANGEHOLY,
	NPC_CHANGEDARKNESS,
	NPC_CHANGETELEKINESIS,
	NPC_CRITICALSLASH,
	NPC_COMBOATTACK,
	NPC_GUIDEDATTACK,
	NPC_SELFDESTRUCTION,
	NPC_SPLASHATTACK,
	NPC_SUICIDE,
	NPC_POISON,
	NPC_BLINDATTACK,
	NPC_SILENCEATTACK,
	NPC_STUNATTACK,
	NPC_PETRIFYATTACK,
	NPC_CURSEATTACK,
	NPC_SLEEPATTACK,
	NPC_RANDOMATTACK,
	NPC_WATERATTACK,
	NPC_GROUNDATTACK,
	NPC_FIREATTACK,
	NPC_WINDATTACK,
	NPC_POISONATTACK,
	NPC_HOLYATTACK,
	NPC_DARKNESSATTACK,
	NPC_TELEKINESISATTACK,
	NPC_MAGICALATTACK,
	NPC_METAMORPHOSIS,
	NPC_PROVOCATION,
	NPC_SMOKING,
	NPC_SUMMONSLAVE,
	NPC_EMOTION,
	NPC_TRANSFORMATION,
	NPC_BLOODDRAIN,
	NPC_ENERGYDRAIN,
	NPC_KEEPING,
	NPC_DARKBREATH,
	NPC_DARKBLESSING,
	NPC_BARRIER,
	NPC_DEFENDER,
	NPC_LICK,
	NPC_HALLUCINATION,
	NPC_REBIRTH,
	NPC_SUMMONMONSTER,

	RG_SNATCHER,
	RG_STEALCOIN,
	RG_BACKSTAP,
	RG_TUNNELDRIVE,
	RG_RAID,
	RG_STRIPWEAPON,
	RG_STRIPSHIELD,
	RG_STRIPARMOR,
	RG_STRIPHELM,
	RG_INTIMIDATE,
	RG_GRAFFITI,
	RG_FLAGGRAFFITI,
	RG_CLEANER,
	RG_GANGSTER,
	RG_COMPULSION,
	RG_PLAGIARISM,

	AM_AXEMASTERY,
	AM_LEARNINGPOTION,
	AM_PHARMACY,
	AM_DEMONSTRATION,
	AM_ACIDTERROR,
	AM_POTIONPITCHER,
	AM_CANNIBALIZE,
	AM_SPHEREMINE,
	AM_CP_WEAPON,
	AM_CP_SHIELD,
	AM_CP_ARMOR,
	AM_CP_HELM,
	AM_BIOETHICS,
	AM_BIOTECHNOLOGY,
	AM_CREATECREATURE,
	AM_CULTIVATION,
	AM_FLAMECONTROL,
	AM_CALLHOMUN,
	AM_REST,
	AM_DRILLMASTER,
	AM_HEALHOMUN,
	AM_RESURRECTHOMUN,

	CR_TRUST,
	CR_AUTOGUARD,
	CR_SHIELDCHARGE,
	CR_SHIELDBOOMERANG,
	CR_REFLECTSHIELD,
	CR_HOLYCROSS,
	CR_GRANDCROSS,
	CR_DEVOTION,
	CR_PROVIDENCE,
	CR_DEFENDER,
	CR_SPEARQUICKEN,

	MO_IRONHAND,
	MO_SPIRITSRECOVERY,
	MO_CALLSPIRITS,
	MO_ABSORBSPIRITS,
	MO_TRIPLEATTACK,
	MO_BODYRELOCATION,
	MO_DODGE,
	MO_INVESTIGATE,
	MO_FINGEROFFENSIVE,
	MO_STEELBODY,
	MO_BLADESTOP,
	MO_EXPLOSIONSPIRITS,
	MO_EXTREMITYFIST,
	MO_CHAINCOMBO,
	MO_COMBOFINISH,

	SA_ADVANCEDBOOK,
	SA_CASTCANCEL,
	SA_MAGICROD,
	SA_SPELLBREAKER,
	SA_FREECAST,
	SA_AUTOSPELL,
	SA_FLAMELAUNCHER,
	SA_FROSTWEAPON,
	SA_LIGHTNINGLOADER,
	SA_SEISMICWEAPON,
	SA_DRAGONOLOGY,
	SA_VOLCANO,
	SA_DELUGE,
	SA_VIOLENTGALE,
	SA_LANDPROTECTOR,
	SA_DISPELL,
	SA_ABRACADABRA,
	SA_MONOCELL,
	SA_CLASSCHANGE,
	SA_SUMMONMONSTER,
	SA_REVERSEORCISH,
	SA_DEATH,
	SA_FORTUNE,
	SA_TAMINGMONSTER,
	SA_QUESTION,
	SA_GRAVITY,
	SA_LEVELUP,
	SA_INSTANTDEATH,
	SA_FULLRECOVERY,
	SA_COMA,

	BD_ADAPTATION,
	BD_ENCORE,
	BD_LULLABY,
	BD_RICHMANKIM,
	BD_ETERNALCHAOS,
	BD_DRUMBATTLEFIELD,
	BD_RINGNIBELUNGEN,
	BD_ROKISWEIL,
	BD_INTOABYSS,
	BD_SIEGFRIED,
	BD_RAGNAROK,

	BA_MUSICALLESSON,
	BA_MUSICALSTRIKE,
	BA_DISSONANCE,
	BA_FROSTJOKER,
	BA_WHISTLE,
	BA_ASSASSINCROSS,
	BA_POEMBRAGI,
	BA_APPLEIDUN,

	DC_DANCINGLESSON,
	DC_THROWARROW,
	DC_UGLYDANCE,
	DC_SCREAM,
	DC_HUMMING,
	DC_DONTFORGETME,
	DC_FORTUNEKISS,
	DC_SERVICEFORYOU,

	NPC_RANDOMMOVE,
	NPC_SPEEDUP,
	NPC_REVENGE,

	WE_MALE,
	WE_FEMALE,
	WE_CALLPARTNER,

	ITM_TOMAHAWK,

	NPC_DARKCROSS,
	NPC_GRANDDARKNESS,
	NPC_DARKSTRIKE,
	NPC_DARKTHUNDER,
	NPC_STOP,
	NPC_WEAPONBRAKER,
	NPC_ARMORBRAKE,
	NPC_HELMBRAKE,
	NPC_SHIELDBRAKE,
	NPC_UNDEADATTACK,
	NPC_CHANGEUNDEAD,
	NPC_POWERUP,
	NPC_AGIUP,
	NPC_SIEGEMODE,
	NPC_CALLSLAVE,
	NPC_INVISIBLE,
	NPC_RUN,

	LK_AURABLADE,
	LK_PARRYING,
	LK_CONCENTRATION,
	LK_TENSIONRELAX,
	LK_BERSERK,
	LK_FURY,
	HP_ASSUMPTIO,
	HP_BASILICA,
	HP_MEDITATIO,
	HW_SOULDRAIN,
	HW_MAGICCRASHER,
	HW_MAGICPOWER,
	PA_PRESSURE,
	PA_SACRIFICE,
	PA_GOSPEL,
	CH_PALMSTRIKE,
	CH_TIGERFIST,
	CH_CHAINCRUSH,
	PF_HPCONVERSION,
	PF_SOULCHANGE,
	PF_SOULBURN,
	ASC_KATAR,
	ASC_HALLUCINATION,
	ASC_EDP,
	ASC_BREAKER,
	SN_SIGHT,
	SN_FALCONASSAULT,
	SN_SHARPSHOOTING,
	SN_WINDWALK,
	WS_MELTDOWN,
	WS_CREATECOIN,
	WS_CREATENUGGET,
	WS_CARTBOOST,
	WS_SYSTEMCREATE,
	ST_CHASEWALK,
	ST_REJECTSWORD,
	ST_STEALBACKPACK,
	CR_ALCHEMY,
	CR_SYNTHESISPOTION,
	CG_ARROWVULCAN,
	CG_MOONLIT,
	CG_MARIONETTE,
	LK_SPIRALPIERCE,
	LK_HEADCRUSH,
	LK_JOINTBEAT,
	HW_NAPALMVULCAN,
	CH_SOULCOLLECT,
	PF_MINDBREAKER,
	PF_MEMORIZE,
	PF_FOGWALL,
	PF_SPIDERWEB,
	ASC_METEORASSAULT,
	ASC_CDP,
	WE_BABY,
	WE_CALLPARENT,
	WE_CALLBABY,

	TK_RUN,
	TK_READYSTORM,
	TK_STORMKICK,
	TK_READYDOWN,
	TK_DOWNKICK,
	TK_READYTURN,
	TK_TURNKICK,
	TK_READYCOUNTER,
	TK_COUNTER,
	TK_DODGE,
	TK_JUMPKICK,
	TK_HPTIME,
	TK_SPTIME,
	TK_POWER,
	TK_SEVENWIND,
	TK_HIGHJUMP,

	SG_FEEL,
	SG_SUN_WARM,
	SG_MOON_WARM,
	SG_STAR_WARM,
	SG_SUN_COMFORT,
	SG_MOON_COMFORT,
	SG_STAR_COMFORT,
	SG_HATE,
	SG_SUN_ANGER,
	SG_MOON_ANGER,
	SG_STAR_ANGER,
	SG_SUN_BLESS,
	SG_MOON_BLESS,
	SG_STAR_BLESS,
	SG_DEVIL,
	SG_FRIEND,
	SG_KNOWLEDGE,
	SG_FUSION,

	SL_ALCHEMIST,
	AM_BERSERKPITCHER,
	SL_MONK,
	SL_STAR,
	SL_SAGE,
	SL_CRUSADER,
	SL_SUPERNOVICE,
	SL_KNIGHT,
	SL_WIZARD,
	SL_PRIEST,
	SL_BARDDANCER,
	SL_ROGUE,
	SL_ASSASIN,
	SL_BLACKSMITH,
	BS_ADRENALINE2,
	SL_HUNTER,
	SL_SOULLINKER,
	SL_KAIZEL,
	SL_KAAHI,
	SL_KAUPE,
	SL_KAITE,
	SL_KAINA,
	SL_STIN,
	SL_STUN,
	SL_SMA,
	SL_SWOO,
	SL_SKE,
	SL_SKA,

	SM_SELFPROVOKE,
	NPC_EMOTION_ON,
	ST_PRESERVE,
	ST_FULLSTRIP,
	WS_WEAPONREFINE,
	CR_SLIMPITCHER,
	CR_FULLPROTECTION,
	PA_SHIELDCHAIN,
	HP_MANARECHARGE,
	PF_DOUBLECASTING,
	HW_GANBANTEIN,
	HW_GRAVITATION,
	WS_CARTTERMINATION,
	WS_OVERTHRUSTMAX,
	CG_LONGINGFREEDOM,
	CG_HERMODE,
	CG_TAROTCARD,
	CR_ACIDDEMONSTRATION,
	CR_CULTIVATION, // Removed on kRO (renewal)
	ITEM_ENCHANTARMS,
	TK_MISSION,
	SL_HIGH,
	KN_ONEHAND,
	AM_TWILIGHT1,
	AM_TWILIGHT2,
	AM_TWILIGHT3,
	HT_POWER,

	GS_GLITTERING,
	GS_FLING,
	GS_TRIPLEACTION,
	GS_BULLSEYE,
	GS_MADNESSCANCEL,
	GS_ADJUSTMENT,
	GS_INCREASING,
	GS_MAGICALBULLET,
	GS_CRACKER,
	GS_SINGLEACTION,
	GS_SNAKEEYE,
	GS_CHAINACTION,
	GS_TRACKING,
	GS_DISARM,
	GS_PIERCINGSHOT,
	GS_RAPIDSHOWER,
	GS_DESPERADO,
	GS_GATLINGFEVER,
	GS_DUST,
	GS_FULLBUSTER,
	GS_SPREADATTACK,
	GS_GROUNDDRIFT,

	NJ_TOBIDOUGU,
	NJ_SYURIKEN,
	NJ_KUNAI,
	NJ_HUUMA,
	NJ_ZENYNAGE,
	NJ_TATAMIGAESHI,
	NJ_KASUMIKIRI,
	NJ_SHADOWJUMP,
	NJ_KIRIKAGE,
	NJ_UTSUSEMI,
	NJ_BUNSINJYUTSU,
	NJ_NINPOU,
	NJ_KOUENKA,
	NJ_KAENSIN,
	NJ_BAKUENRYU,
	NJ_HYOUSENSOU,
	NJ_SUITON,
	NJ_HYOUSYOURAKU,
	NJ_HUUJIN,
	NJ_RAIGEKISAI,
	NJ_KAMAITACHI,
	NJ_NEN,
	NJ_ISSEN,

	MB_FIGHTING,
	MB_NEUTRAL,
	MB_TAIMING_PUTI,
	MB_WHITEPOTION,
	MB_MENTAL,
	MB_CARDPITCHER,
	MB_PETPITCHER,
	MB_BODYSTUDY,
	MB_BODYALTER,
	MB_PETMEMORY,
	MB_M_TELEPORT,
	MB_B_GAIN,
	MB_M_GAIN,
	MB_MISSION,
	MB_MUNAKKNOWLEDGE,
	MB_MUNAKBALL,
	MB_SCROLL,
	MB_B_GATHERING,
	MB_M_GATHERING,
	MB_B_EXCLUDE,
	MB_B_DRIFT,
	MB_B_WALLRUSH,
	MB_M_WALLRUSH,
	MB_B_WALLSHIFT,
	MB_M_WALLCRASH,
	MB_M_REINCARNATION,
	MB_B_EQUIP,

	SL_DEATHKNIGHT,
	SL_COLLECTOR,
	SL_NINJA,
	SL_GUNNER,
	AM_TWILIGHT4,
	DA_RESET,
	DE_BERSERKAIZER,
	DA_DARKPOWER,

	DE_PASSIVE,
	DE_PATTACK,
	DE_PSPEED,
	DE_PDEFENSE,
	DE_PCRITICAL,
	DE_PHP,
	DE_PSP,
	DE_RESET,
	DE_RANKING,
	DE_PTRIPLE,
	DE_ENERGY,
	DE_NIGHTMARE,
	DE_SLASH,
	DE_COIL,
	DE_WAVE,
	DE_REBIRTH,
	DE_AURA,
	DE_FREEZER,
	DE_CHANGEATTACK,
	DE_PUNISH,
	DE_POISON,
	DE_INSTANT,
	DE_WARNING,
	DE_RANKEDKNIFE,
	DE_RANKEDGRADIUS,
	DE_GAUGE,
	DE_GTIME,
	DE_GPAIN,
	DE_GSKILL,
	DE_GKILL,
	DE_ACCEL,
	DE_BLOCKDOUBLE,
	DE_BLOCKMELEE,
	DE_BLOCKFAR,
	DE_FRONTATTACK,
	DE_DANGERATTACK,
	DE_TWINATTACK,
	DE_WINDATTACK,
	DE_WATERATTACK,

	DA_ENERGY,
	DA_CLOUD,
	DA_FIRSTSLOT,
	DA_HEADDEF,
	DA_SPACE,
	DA_TRANSFORM,
	DA_EXPLOSION,
	DA_REWARD,
	DA_CRUSH,
	DA_ITEMREBUILD,
	DA_ILLUSION,
	DA_NUETRALIZE,
	DA_RUNNER,
	DA_TRANSFER,
	DA_WALL,
	DA_ZENY,
	DA_REVENGE,
	DA_EARPLUG,
	DA_CONTRACT,
	DA_BLACK,
	DA_DREAM,
	DA_MAGICCART,
	DA_COPY,
	DA_CRYSTAL,
	DA_EXP,
	DA_CARTSWING,
	DA_REBUILD,
	DA_JOBCHANGE,
	DA_EDARKNESS,
	DA_EGUARDIAN,
	DA_TIMEOUT,
	ALL_TIMEIN,
	DA_ZENYRANK,
	DA_ACCESSORYMIX,

	NPC_EARTHQUAKE,
	NPC_FIREBREATH,
	NPC_ICEBREATH,
	NPC_THUNDERBREATH,
	NPC_ACIDBREATH,
	NPC_DARKNESSBREATH,
	NPC_DRAGONFEAR,
	NPC_BLEEDING,
	NPC_PULSESTRIKE,
	NPC_HELLJUDGEMENT,
	NPC_WIDESILENCE,
	NPC_WIDEFREEZE,
	NPC_WIDEBLEEDING,
	NPC_WIDESTONE,
	NPC_WIDECONFUSE,
	NPC_WIDESLEEP,
	NPC_WIDESIGHT,
	NPC_EVILLAND,
	NPC_MAGICMIRROR,
	NPC_SLOWCAST,
	NPC_CRITICALWOUND,
	NPC_EXPULSION,
	NPC_STONESKIN,
	NPC_ANTIMAGIC,
	NPC_WIDECURSE,
	NPC_WIDESTUN,
	NPC_VAMPIRE_GIFT,
	NPC_WIDESOULDRAIN,

	ALL_INCCARRY,
	NPC_TALK,
	NPC_HELLPOWER,
	NPC_WIDEHELLDIGNITY,
	NPC_INVINCIBLE,
	NPC_INVINCIBLEOFF,
	NPC_ALLHEAL,
	GM_SANDMAN,
	CASH_BLESSING,
	CASH_INCAGI,
	CASH_ASSUMPTIO,
	ALL_CATCRY,
	ALL_PARTYFLEE,
	ALL_ANGEL_PROTECT,
	ALL_DREAM_SUMMERNIGHT,
	NPC_CHANGEUNDEAD2,
	ALL_REVERSEORCISH,
	ALL_WEWISH,
	ALL_SONKRAN,
	NPC_WIDEHEALTHFEAR,
	NPC_WIDEBODYBURNNING,
	NPC_WIDEFROSTMISTY,
	NPC_WIDECOLD,
	NPC_WIDE_DEEP_SLEEP,
	NPC_WIDESIREN,
	NPC_VENOMFOG,
	NPC_MILLENNIUMSHIELD,
	NPC_COMET,
	NPC_ICEMINE,
	NPC_ICEEXPLO,
	NPC_FLAMECROSS,
	NPC_PULSESTRIKE2,
	NPC_DANCINGBLADE,
	NPC_DANCINGBLADE_ATK,
	NPC_DARKPIERCING,
	NPC_MAXPAIN,
	NPC_MAXPAIN_ATK,
	NPC_DEATHSUMMON,
	NPC_HELLBURNING,
	NPC_JACKFROST,
	NPC_WIDEWEB,
	NPC_WIDESUCK,
	NPC_STORMGUST2,
	NPC_FIRESTORM,
	NPC_REVERBERATION,
	NPC_REVERBERATION_ATK,
	NPC_LEX_AETERNA,
	NPC_ARROWSTORM,
	NPC_CHEAL,
	NPC_SR_CURSEDCIRCLE,
	NPC_DRAGONBREATH,
	NPC_FATALMENACE,
	NPC_MAGMA_ERUPTION,
	NPC_MAGMA_ERUPTION_DOTDAMAGE,
	NPC_MANDRAGORA,
	NPC_PSYCHIC_WAVE,
	NPC_RAYOFGENESIS,
	NPC_VENOMIMPRESS,
	NPC_CLOUD_KILL,
	NPC_IGNITIONBREAK,
	NPC_PHANTOMTHRUST,
	NPC_POISON_BUSTER,
	NPC_HALLUCINATIONWALK,
	NPC_ELECTRICWALK,
	NPC_FIREWALK,
	NPC_WIDEDISPEL,
	NPC_LEASH,
	NPC_WIDELEASH,
	NPC_WIDECRITICALWOUND,
	NPC_EARTHQUAKE_K,
	NPC_ALL_STAT_DOWN,
	NPC_GRADUAL_GRAVITY,
	NPC_DAMAGE_HEAL,
	NPC_IMMUNE_PROPERTY,
	NPC_MOVE_COORDINATE,
	NPC_WIDEBLEEDING2,
	NPC_WIDESILENCE2,
	NPC_WIDESTUN2,
	NPC_WIDESTONE2,
	NPC_WIDESLEEP2,
	NPC_WIDECURSE2,
	NPC_WIDECONFUSE2,
	NPC_WIDEFREEZE2,
	NPC_BLEEDING2,
	NPC_ICEBREATH2,
	NPC_ACIDBREATH2,
	NPC_EVILLAND2,
	NPC_HELLJUDGEMENT2,
	NPC_RAINOFMETEOR,
	NPC_GROUNDDRIVE,
	NPC_RELIEVE_ON,
	NPC_RELIEVE_OFF,
	NPC_LOCKON_LASER,
	NPC_LOCKON_LASER_ATK,
	NPC_SEEDTRAP,
	NPC_DEADLYCURSE,
	NPC_RANDOMBREAK,
	NPC_STRIP_SHADOW,
	NPC_DEADLYCURSE2,
	NPC_CANE_OF_EVIL_EYE,
	NPC_CURSE_OF_RED_CUBE,
	NPC_CURSE_OF_BLUE_CUBE,
	NPC_KILLING_AURA,
	ALL_EVENT_20TH_ANNIVERSARY,
	NPC_TARGET_MARKER,
	NPC_AIMED_SHOWER,
	NPC_BLAZING_ERUPTION,
	NPC_BLOCK_SEAL,
	NPC_BLOCK_EXPLOSION,
	NPC_FROST_FIELD,
	NPC_LIGHTNING_JUDGEMENT,
	NPC_GROGGY_ON,
	NPC_RESET_EFST, //793

	KN_CHARGEATK = 1001,
	CR_SHRINK,
	AS_SONICACCEL,
	AS_VENOMKNIFE,
	RG_CLOSECONFINE,
	WZ_SIGHTBLASTER,
	SA_CREATECON,
	SA_ELEMENTWATER,
	HT_PHANTASMIC,
	BA_PANGVOICE,
	DC_WINKCHARM,
	BS_UNFAIRLYTRICK,
	BS_GREED,
	PR_REDEMPTIO,
	MO_KITRANSLATION,
	MO_BALKYOUNG,
	SA_ELEMENTGROUND,
	SA_ELEMENTFIRE,
	SA_ELEMENTWIND,

	RK_ENCHANTBLADE = 2001,
	RK_SONICWAVE,
	RK_DEATHBOUND,
	RK_HUNDREDSPEAR,
	RK_WINDCUTTER,
	RK_IGNITIONBREAK,
	RK_DRAGONTRAINING,
	RK_DRAGONBREATH,
	RK_DRAGONHOWLING,
	RK_RUNEMASTERY,
	RK_MILLENNIUMSHIELD,
	RK_CRUSHSTRIKE,
	RK_REFRESH,
	RK_GIANTGROWTH,
	RK_STONEHARDSKIN,
	RK_VITALITYACTIVATION,
	RK_STORMBLAST,
	RK_FIGHTINGSPIRIT,
	RK_ABUNDANCE,
	RK_PHANTOMTHRUST,

	GC_VENOMIMPRESS,
	GC_CROSSIMPACT,
	GC_DARKILLUSION,
	GC_RESEARCHNEWPOISON,
	GC_CREATENEWPOISON,
	GC_ANTIDOTE,
	GC_POISONINGWEAPON,
	GC_WEAPONBLOCKING,
	GC_COUNTERSLASH,
	GC_WEAPONCRUSH,
	GC_VENOMPRESSURE,
	GC_POISONSMOKE,
	GC_CLOAKINGEXCEED,
	GC_PHANTOMMENACE,
	GC_HALLUCINATIONWALK,
	GC_ROLLINGCUTTER,
	GC_CROSSRIPPERSLASHER,

	AB_JUDEX,
	AB_ANCILLA,
	AB_ADORAMUS,
	AB_CLEMENTIA,
	AB_CANTO,
	AB_CHEAL,
	AB_EPICLESIS,
	AB_PRAEFATIO,
	AB_ORATIO,
	AB_LAUDAAGNUS,
	AB_LAUDARAMUS,
	AB_EUCHARISTICA, // Removed on kRO
	AB_RENOVATIO,
	AB_HIGHNESSHEAL,
	AB_CLEARANCE,
	AB_EXPIATIO,
	AB_DUPLELIGHT,
	AB_DUPLELIGHT_MELEE,
	AB_DUPLELIGHT_MAGIC,
	AB_SILENTIUM,

	WL_WHITEIMPRISON = 2201,
	WL_SOULEXPANSION,
	WL_FROSTMISTY,
	WL_JACKFROST,
	WL_MARSHOFABYSS,
	WL_RECOGNIZEDSPELL,
	WL_SIENNAEXECRATE,
	WL_RADIUS,
	WL_STASIS,
	WL_DRAINLIFE,
	WL_CRIMSONROCK,
	WL_HELLINFERNO,
	WL_COMET,
	WL_CHAINLIGHTNING,
	WL_CHAINLIGHTNING_ATK,
	WL_EARTHSTRAIN,
	WL_TETRAVORTEX,
	WL_TETRAVORTEX_FIRE,
	WL_TETRAVORTEX_WATER,
	WL_TETRAVORTEX_WIND,
	WL_TETRAVORTEX_GROUND,
	WL_SUMMONFB,
	WL_SUMMONBL,
	WL_SUMMONWB,
	WL_SUMMON_ATK_FIRE,
	WL_SUMMON_ATK_WIND,
	WL_SUMMON_ATK_WATER,
	WL_SUMMON_ATK_GROUND,
	WL_SUMMONSTONE,
	WL_RELEASE,
	WL_READING_SB,
	WL_FREEZE_SP,

	RA_ARROWSTORM,
	RA_FEARBREEZE,
	RA_RANGERMAIN,
	RA_AIMEDBOLT,
	RA_DETONATOR,
	RA_ELECTRICSHOCKER,
	RA_CLUSTERBOMB,
	RA_WUGMASTERY,
	RA_WUGRIDER,
	RA_WUGDASH,
	RA_WUGSTRIKE,
	RA_WUGBITE,
	RA_TOOTHOFWUG,
	RA_SENSITIVEKEEN,
	RA_CAMOUFLAGE,
	RA_RESEARCHTRAP,
	RA_MAGENTATRAP,
	RA_COBALTTRAP,
	RA_MAIZETRAP,
	RA_VERDURETRAP,
	RA_FIRINGTRAP,
	RA_ICEBOUNDTRAP,

	NC_MADOLICENCE,
	NC_BOOSTKNUCKLE,
	NC_PILEBUNKER,
	NC_VULCANARM,
	NC_FLAMELAUNCHER,
	NC_COLDSLOWER,
	NC_ARMSCANNON,
	NC_ACCELERATION,
	NC_HOVERING,
	NC_F_SIDESLIDE,
	NC_B_SIDESLIDE,
	NC_MAINFRAME,
	NC_SELFDESTRUCTION,
	NC_SHAPESHIFT,
	NC_EMERGENCYCOOL,
	NC_INFRAREDSCAN,
	NC_ANALYZE,
	NC_MAGNETICFIELD,
	NC_NEUTRALBARRIER,
	NC_STEALTHFIELD,
	NC_REPAIR,
	NC_TRAININGAXE,
	NC_RESEARCHFE,
	NC_AXEBOOMERANG,
	NC_POWERSWING,
	NC_AXETORNADO,
	NC_SILVERSNIPER,
	NC_MAGICDECOY,
	NC_DISJOINT,

	SC_FATALMENACE,
	SC_REPRODUCE,
	SC_AUTOSHADOWSPELL,
	SC_SHADOWFORM,
	SC_TRIANGLESHOT,
	SC_BODYPAINT,
	SC_INVISIBILITY,
	SC_DEADLYINFECT,
	SC_ENERVATION,
	SC_GROOMY,
	SC_IGNORANCE,
	SC_LAZINESS,
	SC_UNLUCKY,
	SC_WEAKNESS,
	SC_STRIPACCESSARY,
	SC_MANHOLE,
	SC_DIMENSIONDOOR,
	SC_CHAOSPANIC,
	SC_MAELSTROM,
	SC_BLOODYLUST,
	SC_FEINTBOMB,

	LG_CANNONSPEAR = 2307,
	LG_BANISHINGPOINT,
	LG_TRAMPLE,
	LG_SHIELDPRESS,
	LG_REFLECTDAMAGE,
	LG_PINPOINTATTACK,
	LG_FORCEOFVANGUARD,
	LG_RAGEBURST,
	LG_SHIELDSPELL,
	LG_EXEEDBREAK,
	LG_OVERBRAND,
	LG_PRESTIGE,
	LG_BANDING,
	LG_MOONSLASHER,
	LG_RAYOFGENESIS,
	LG_PIETY,
	LG_EARTHDRIVE,
	LG_HESPERUSLIT,
	LG_INSPIRATION,

	SR_DRAGONCOMBO,
	SR_SKYNETBLOW,
	SR_EARTHSHAKER,
	SR_FALLENEMPIRE,
	SR_TIGERCANNON,
	SR_HELLGATE,
	SR_RAMPAGEBLASTER,
	SR_CRESCENTELBOW,
	SR_CURSEDCIRCLE,
	SR_LIGHTNINGWALK,
	SR_KNUCKLEARROW,
	SR_WINDMILL,
	SR_RAISINGDRAGON,
	SR_GENTLETOUCH,
	SR_ASSIMILATEPOWER,
	SR_POWERVELOCITY,
	SR_CRESCENTELBOW_AUTOSPELL,
	SR_GATEOFHELL,
	SR_GENTLETOUCH_QUIET,
	SR_GENTLETOUCH_CURE,
	SR_GENTLETOUCH_ENERGYGAIN,
	SR_GENTLETOUCH_CHANGE,
	SR_GENTLETOUCH_REVITALIZE,

	WA_SWING_DANCE = 2350,
	WA_SYMPHONY_OF_LOVER,
	WA_MOONLIT_SERENADE,

	MI_RUSH_WINDMILL = 2381,
	MI_ECHOSONG,
	MI_HARMONIZE,

	WM_LESSON = 2412,
	WM_METALICSOUND,
	WM_REVERBERATION,
	WM_REVERBERATION_MELEE, // Removed on kRO
	WM_REVERBERATION_MAGIC, // Removed on kRO
	WM_DOMINION_IMPULSE, // Removed on kRO
	WM_SEVERE_RAINSTORM,
	WM_POEMOFNETHERWORLD,
	WM_VOICEOFSIREN,
	WM_DEADHILLHERE,
	WM_LULLABY_DEEPSLEEP,
	WM_SIRCLEOFNATURE,
	WM_RANDOMIZESPELL,
	WM_GLOOMYDAY,
	WM_GREAT_ECHO,
	WM_SONG_OF_MANA,
	WM_DANCE_WITH_WUG,
	WM_SOUND_OF_DESTRUCTION,
	WM_SATURDAY_NIGHT_FEVER,
	WM_LERADS_DEW,
	WM_MELODYOFSINK,
	WM_BEYOND_OF_WARCRY,
	WM_UNLIMITED_HUMMING_VOICE,

	SO_FIREWALK = 2443,
	SO_ELECTRICWALK,
	SO_SPELLFIST,
	SO_EARTHGRAVE,
	SO_DIAMONDDUST,
	SO_POISON_BUSTER,
	SO_PSYCHIC_WAVE,
	SO_CLOUD_KILL,
	SO_STRIKING,
	SO_WARMER,
	SO_VACUUM_EXTREME,
	SO_VARETYR_SPEAR,
	SO_ARRULLO,
	SO_EL_CONTROL,
	SO_SUMMON_AGNI,
	SO_SUMMON_AQUA,
	SO_SUMMON_VENTUS,
	SO_SUMMON_TERA,
	SO_EL_ACTION,
	SO_EL_ANALYSIS,
	SO_EL_SYMPATHY,
	SO_EL_CURE,
	SO_FIRE_INSIGNIA,
	SO_WATER_INSIGNIA,
	SO_WIND_INSIGNIA,
	SO_EARTH_INSIGNIA,

	GN_TRAINING_SWORD = 2474,
	GN_REMODELING_CART,
	GN_CART_TORNADO,
	GN_CARTCANNON,
	GN_CARTBOOST,
	GN_THORNS_TRAP,
	GN_BLOOD_SUCKER,
	GN_SPORE_EXPLOSION,
	GN_WALLOFTHORN,
	GN_CRAZYWEED,
	GN_CRAZYWEED_ATK,
	GN_DEMONIC_FIRE,
	GN_FIRE_EXPANSION,
	GN_FIRE_EXPANSION_SMOKE_POWDER,
	GN_FIRE_EXPANSION_TEAR_GAS,
	GN_FIRE_EXPANSION_ACID,
	GN_HELLS_PLANT,
	GN_HELLS_PLANT_ATK,
	GN_MANDRAGORA,
	GN_SLINGITEM, // Removed on kRO
	GN_CHANGEMATERIAL,
	GN_MIX_COOKING,
	GN_MAKEBOMB, // Removed on kRO
	GN_S_PHARMACY,
	GN_SLINGITEM_RANGEMELEEATK, // Removed on kRO

	AB_SECRAMENT = 2515,
	WM_SEVERE_RAINSTORM_MELEE,
	SR_HOWLINGOFLION,
	SR_RIDEINLIGHTNING,
	LG_OVERBRAND_BRANDISH,
	LG_OVERBRAND_PLUSATK,

	ALL_ODINS_RECALL = 2533,
	RETURN_TO_ELDICASTES,
	ALL_BUYING_STORE,
	ALL_GUARDIAN_RECALL,
	ALL_ODINS_POWER,
	ALL_BEER_BOTTLE_CAP,
	NPC_ASSASSINCROSS,
	NPC_DISSONANCE,
	NPC_UGLYDANCE,
	ALL_TETANY,
	ALL_RAY_OF_PROTECTION,
	MC_CARTDECORATE,
	GM_ITEM_ATKMAX,
	GM_ITEM_ATKMIN,
	GM_ITEM_MATKMAX,
	GM_ITEM_MATKMIN,

	RL_GLITTERING_GREED = 2551,
	RL_RICHS_COIN,
	RL_MASS_SPIRAL,
	RL_BANISHING_BUSTER,
	RL_B_TRAP,
	RL_FLICKER,
	RL_S_STORM,
	RL_E_CHAIN,
	RL_QD_SHOT,
	RL_C_MARKER,
	RL_FIREDANCE,
	RL_H_MINE,
	RL_P_ALTER,
	RL_FALLEN_ANGEL,
	RL_R_TRIP,
	RL_D_TAIL,
	RL_FIRE_RAIN,
	RL_HEAT_BARREL,
	RL_AM_BLAST,
	RL_SLUGSHOT,
	RL_HAMMER_OF_GOD,
	RL_R_TRIP_PLUSATK,
	RL_B_FLICKER_ATK,
//	RL_GLITTERING_GREED_ATK,
	SJ_LIGHTOFMOON,
	SJ_LUNARSTANCE,
	SJ_FULLMOONKICK,
	SJ_LIGHTOFSTAR,
	SJ_STARSTANCE,
	SJ_NEWMOONKICK,
	SJ_FLASHKICK,
	SJ_STAREMPEROR,
	SJ_NOVAEXPLOSING,
	SJ_UNIVERSESTANCE,
	SJ_FALLINGSTAR,
	SJ_GRAVITYCONTROL,
	SJ_BOOKOFDIMENSION,
	SJ_BOOKOFCREATINGSTAR,
	SJ_DOCUMENT,
	SJ_PURIFY,
	SJ_LIGHTOFSUN,
	SJ_SUNSTANCE,
	SJ_SOLARBURST,
	SJ_PROMINENCEKICK,
	SJ_FALLINGSTAR_ATK,
	SJ_FALLINGSTAR_ATK2,
	SP_SOULGOLEM,
	SP_SOULSHADOW,
	SP_SOULFALCON,
	SP_SOULFAIRY,
	SP_CURSEEXPLOSION,
	SP_SOULCURSE,
	SP_SPA,
	SP_SHA,
	SP_SWHOO,
	SP_SOULUNITY,
	SP_SOULDIVISION,
	SP_SOULREAPER,
	SP_SOULREVOLVE,
	SP_SOULCOLLECT,
	SP_SOULEXPLOSION,
	SP_SOULENERGY,
	SP_KAUTE,

	KO_YAMIKUMO = 3001,
	KO_RIGHT,
	KO_LEFT,
	KO_JYUMONJIKIRI,
	KO_SETSUDAN,
	KO_BAKURETSU,
	KO_HAPPOKUNAI,
	KO_MUCHANAGE,
	KO_HUUMARANKA,
	KO_MAKIBISHI,
	KO_MEIKYOUSISUI,
	KO_ZANZOU,
	KO_KYOUGAKU,
	KO_JYUSATSU,
	KO_KAHU_ENTEN,
	KO_HYOUHU_HUBUKI,
	KO_KAZEHU_SEIRAN,
	KO_DOHU_KOUKAI,
	KO_KAIHOU,
	KO_ZENKAI,
	KO_GENWAKU,
	KO_IZAYOI,
	KG_KAGEHUMI,
	KG_KYOMU,
	KG_KAGEMUSYA,
	OB_ZANGETSU,
	OB_OBOROGENSOU,
	OB_OBOROGENSOU_TRANSITION_ATK,
	OB_AKAITSUKI,

	ECL_SNOWFLIP = 3031,
	ECL_PEONYMAMY,
	ECL_SADAGUI,
	ECL_SEQUOIADUST,
	ECLAGE_RECALL,
	BA_POEMBRAGI2,
	DC_FORTUNEKISS2,
	// ITEM_OPTION_SPLASH_ATTACK,
	GM_FORCE_TRANSFER = 3039,
	GM_WIDE_RESURRECTION,
	ALL_NIFLHEIM_RECALL,
	ALL_PRONTERA_RECALL,
	ALL_GLASTHEIM_RECALL,
	ALL_THANATOS_RECALL,
	ALL_LIGHTHALZEN_RECALL,

	GC_DARKCROW = 5001,
	RA_UNLIMIT,
	GN_ILLUSIONDOPING,
	RK_DRAGONBREATH_WATER,
	RK_LUXANIMA,
	NC_MAGMA_ERUPTION,
	WM_FRIGG_SONG,
	SO_ELEMENTAL_SHIELD,
	SR_FLASHCOMBO,
	SC_ESCAPE,
	AB_OFFERTORIUM,
	WL_TELEKINESIS_INTENSE,
	LG_KINGS_GRACE,
	ALL_FULL_THROTTLE,
	NC_MAGMA_ERUPTION_DOTDAMAGE,

	SU_BASIC_SKILL = 5018,
	SU_BITE,
	SU_HIDE,
	SU_SCRATCH,
	SU_STOOP,
	SU_LOPE,
	SU_SPRITEMABLE,
	SU_POWEROFLAND,
	SU_SV_STEMSPEAR,
	SU_CN_POWDERING,
	SU_CN_METEOR,
	SU_SV_ROOTTWIST,
	SU_SV_ROOTTWIST_ATK,
	SU_POWEROFLIFE,
	SU_SCAROFTAROU,
	SU_PICKYPECK,
	SU_PICKYPECK_DOUBLE_ATK,
	SU_ARCLOUSEDASH,
	SU_LUNATICCARROTBEAT,
	SU_POWEROFSEA,
	SU_TUNABELLY,
	SU_TUNAPARTY,
	SU_BUNCHOFSHRIMP,
	SU_FRESHSHRIMP,
	SU_CN_METEOR2,
	SU_LUNATICCARROTBEAT2,
	SU_SOULATTACK,
	SU_POWEROFFLOCK,
	SU_SVG_SPIRIT,
	SU_HISS,
	SU_NYANGGRASS,
	SU_GROOMING,
	SU_PURRING,
	SU_SHRIMPARTY,
	SU_SPIRITOFLIFE,
	SU_MEOWMEOW,
	SU_SPIRITOFLAND,
	SU_CHATTERING,
	SU_SPIRITOFSEA,

	WE_CALLALLFAMILY = 5063,
	WE_ONEFOREVER,
	WE_CHEERUP,

	ALL_EQSWITCH = 5067,

	CG_SPECIALSINGER,

	AB_VITUPERATUM = 5072,
	AB_CONVENIO,
	ALL_LIGHTNING_STORM,
	NV_BREAKTHROUGH,
	NV_HELPANGEL,
	NV_TRANSCENDENCE,
	WL_READING_SB_READING,

	DK_SERVANTWEAPON = 5201,
	DK_SERVANTWEAPON_ATK,
	DK_SERVANT_W_SIGN,
	DK_SERVANT_W_PHANTOM,
	DK_SERVANT_W_DEMOL,
	DK_CHARGINGPIERCE,
	DK_TWOHANDDEF,
	DK_HACKANDSLASHER,
	DK_HACKANDSLASHER_ATK,
	DK_DRAGONIC_AURA,
	DK_MADNESS_CRUSHER,
	DK_VIGOR,
	DK_STORMSLASH,

	AG_DEADLY_PROJECTION,
	AG_DESTRUCTIVE_HURRICANE,
	AG_RAIN_OF_CRYSTAL,
	AG_MYSTERY_ILLUSION,
	AG_VIOLENT_QUAKE,
	AG_VIOLENT_QUAKE_ATK,
	AG_SOUL_VC_STRIKE,
	AG_STRANTUM_TREMOR,
	AG_ALL_BLOOM,
	AG_ALL_BLOOM_ATK,
	AG_ALL_BLOOM_ATK2,
	AG_CRYSTAL_IMPACT,
	AG_CRYSTAL_IMPACT_ATK,
	AG_TORNADO_STORM,
	AG_TWOHANDSTAFF,
	AG_FLORAL_FLARE_ROAD,
	AG_ASTRAL_STRIKE,
	AG_ASTRAL_STRIKE_ATK,
	AG_CLIMAX,
	AG_ROCK_DOWN,
	AG_STORM_CANNON,
	AG_CRIMSON_ARROW,
	AG_CRIMSON_ARROW_ATK,
	AG_FROZEN_SLASH,

	IQ_POWERFUL_FAITH,
	IQ_FIRM_FAITH,
	IQ_WILL_OF_FAITH,
	IQ_OLEUM_SANCTUM,
	IQ_SINCERE_FAITH,
	IQ_MASSIVE_F_BLASTER,
	IQ_EXPOSION_BLASTER,
	IQ_FIRST_BRAND,
	IQ_FIRST_FAITH_POWER,
	IQ_JUDGE,
	IQ_SECOND_FLAME,
	IQ_SECOND_FAITH,
	IQ_SECOND_JUDGEMENT,
	IQ_THIRD_PUNISH,
	IQ_THIRD_FLAME_BOMB,
	IQ_THIRD_CONSECRATION,
	IQ_THIRD_EXOR_FLAME,

	IG_GUARD_STANCE,
	IG_GUARDIAN_SHIELD,
	IG_REBOUND_SHIELD,
	IG_SHIELD_MASTERY,
	IG_SPEAR_SWORD_M,
	IG_ATTACK_STANCE,
	IG_ULTIMATE_SACRIFICE,
	IG_HOLY_SHIELD,
	IG_GRAND_JUDGEMENT,
	IG_JUDGEMENT_CROSS,
	IG_SHIELD_SHOOTING,
	IG_OVERSLASH,
	IG_CROSS_RAIN,

	CD_REPARATIO,
	CD_MEDIALE_VOTUM,
	CD_MACE_BOOK_M,
	CD_ARGUTUS_VITA,
	CD_ARGUTUS_TELUM,
	CD_ARBITRIUM,
	CD_ARBITRIUM_ATK,
	CD_PRESENS_ACIES,
	CD_FIDUS_ANIMUS,
	CD_EFFLIGO,
	CD_COMPETENTIA,
	CD_PNEUMATICUS_PROCELLA,
	CD_DILECTIO_HEAL,
	CD_RELIGIO,
	CD_BENEDICTUM,
	CD_PETITIO,
	CD_FRAMEN,

	SHC_SHADOW_EXCEED,
	SHC_DANCING_KNIFE,
	SHC_SAVAGE_IMPACT,
	SHC_SHADOW_SENSE,
	SHC_ETERNAL_SLASH,
	SHC_POTENT_VENOM,
	SHC_SHADOW_STAB,
	SHC_IMPACT_CRATER,
	SHC_ENCHANTING_SHADOW,
	SHC_FATAL_SHADOW_CROW,

	MT_AXE_STOMP,
	MT_RUSH_QUAKE,
	MT_M_MACHINE,
	MT_A_MACHINE,
	MT_D_MACHINE,
	MT_TWOAXEDEF,
	MT_ABR_M,
	MT_SUMMON_ABR_BATTLE_WARIOR,
	MT_SUMMON_ABR_DUAL_CANNON,
	MT_SUMMON_ABR_MOTHER_NET,
	MT_SUMMON_ABR_INFINITY,

	AG_DESTRUCTIVE_HURRICANE_CLIMAX,
	BO_ACIDIFIED_ZONE_WATER_ATK,
	BO_ACIDIFIED_ZONE_GROUND_ATK,
	BO_ACIDIFIED_ZONE_WIND_ATK,
	BO_ACIDIFIED_ZONE_FIRE_ATK,

	ABC_DAGGER_AND_BOW_M,
	ABC_MAGIC_SWORD_M,
	ABC_STRIP_SHADOW,
	ABC_ABYSS_DAGGER,
	ABC_UNLUCKY_RUSH,
	ABC_CHAIN_REACTION_SHOT,
	ABC_FROM_THE_ABYSS,
	ABC_ABYSS_SLAYER,
	ABC_ABYSS_STRIKE,
	ABC_DEFT_STAB,
	ABC_ABYSS_SQUARE,
	ABC_FRENZY_SHOT,

	WH_ADVANCED_TRAP,
	WH_WIND_SIGN,
	WH_NATUREFRIENDLY,
	WH_HAWKRUSH,
	WH_HAWK_M,
	WH_CALAMITYGALE,
	WH_HAWKBOOMERANG,
	WH_GALESTORM,
	WH_DEEPBLINDTRAP,
	WH_SOLIDTRAP,
	WH_SWIFTTRAP,
	WH_CRESCIVE_BOLT,
	WH_FLAMETRAP,

	BO_BIONIC_PHARMACY,
	BO_BIONICS_M,
	BO_THE_WHOLE_PROTECTION,
	BO_ADVANCE_PROTECTION,
	BO_ACIDIFIED_ZONE_WATER,
	BO_ACIDIFIED_ZONE_GROUND,
	BO_ACIDIFIED_ZONE_WIND,
	BO_ACIDIFIED_ZONE_FIRE,
	BO_WOODENWARRIOR,
	BO_WOODEN_FAIRY,
	BO_CREEPER,
	BO_RESEARCHREPORT,
	BO_HELLTREE,

	TR_STAGE_MANNER,
	TR_RETROSPECTION,
	TR_MYSTIC_SYMPHONY,
	TR_KVASIR_SONATA,
	TR_ROSEBLOSSOM,
	TR_ROSEBLOSSOM_ATK,
	TR_RHYTHMSHOOTING,
	TR_METALIC_FURY,
	TR_SOUNDBLEND,
	TR_GEF_NOCTURN,
	TR_ROKI_CAPRICCIO,
	TR_AIN_RHAPSODY,
	TR_MUSICAL_INTERLUDE,
	TR_JAWAII_SERENADE,
	TR_NIPELHEIM_REQUIEM,
	TR_PRON_MARCH,

	EM_MAGIC_BOOK_M,
	EM_SPELL_ENCHANTING,
	EM_ACTIVITY_BURN,
	EM_INCREASING_ACTIVITY,
	EM_DIAMOND_STORM,
	EM_LIGHTNING_LAND,
	EM_VENOM_SWAMP,
	EM_CONFLAGRATION,
	EM_TERRA_DRIVE,
	EM_ELEMENTAL_SPIRIT_M,
	EM_SUMMON_ELEMENTAL_ARDOR,
	EM_SUMMON_ELEMENTAL_DILUVIO,
	EM_SUMMON_ELEMENTAL_PROCELLA,
	EM_SUMMON_ELEMENTAL_TERREMOTUS,
	EM_SUMMON_ELEMENTAL_SERPENS,
	EM_ELEMENTAL_BUSTER,
	EM_ELEMENTAL_VEIL,

	ABC_CHAIN_REACTION_SHOT_ATK,
	ABC_FROM_THE_ABYSS_ATK,
	BO_WOODEN_THROWROCK,
	BO_WOODEN_ATTACK,
	BO_HELL_HOWLING,
	BO_HELL_DUSTY,
	BO_FAIRY_DUSTY,
	EM_ELEMENTAL_BUSTER_FIRE,
	EM_ELEMENTAL_BUSTER_WATER,
	EM_ELEMENTAL_BUSTER_WIND,
	EM_ELEMENTAL_BUSTER_GROUND,
	EM_ELEMENTAL_BUSTER_POISON,

	NW_P_F_I = 5401,
	NW_GRENADE_MASTERY,
	NW_INTENSIVE_AIM,
	NW_GRENADE_FRAGMENT,
	NW_THE_VIGILANTE_AT_NIGHT,
	NW_ONLY_ONE_BULLET,
	NW_SPIRAL_SHOOTING,
	NW_MAGAZINE_FOR_ONE,
	NW_WILD_FIRE,
	NW_BASIC_GRENADE,
	NW_HASTY_FIRE_IN_THE_HOLE,
	NW_GRENADES_DROPPING,
	NW_AUTO_FIRING_LAUNCHER,
	NW_HIDDEN_CARD,
	NW_MISSION_BOMBARD,

	SOA_TALISMAN_MASTERY,
	SOA_SOUL_MASTERY,
	SOA_TALISMAN_OF_PROTECTION,
	SOA_TALISMAN_OF_WARRIOR,
	SOA_TALISMAN_OF_MAGICIAN,
	SOA_SOUL_GATHERING,
	SOA_TOTEM_OF_TUTELARY,
	SOA_TALISMAN_OF_FIVE_ELEMENTS,
	SOA_TALISMAN_OF_SOUL_STEALING,
	SOA_EXORCISM_OF_MALICIOUS_SOUL,
	SOA_TALISMAN_OF_BLUE_DRAGON,
	SOA_TALISMAN_OF_WHITE_TIGER,
	SOA_TALISMAN_OF_RED_PHOENIX,
	SOA_TALISMAN_OF_BLACK_TORTOISE,
	SOA_TALISMAN_OF_FOUR_BEARING_GOD,
	SOA_CIRCLE_OF_DIRECTIONS_AND_ELEMENTALS,
	SOA_SOUL_OF_HEAVEN_AND_EARTH,

	SH_MYSTICAL_CREATURE_MASTERY,
	SH_COMMUNE_WITH_CHUL_HO,
	SH_CHUL_HO_SONIC_CLAW,
	SH_HOWLING_OF_CHUL_HO,
	SH_HOGOGONG_STRIKE,
	SH_COMMUNE_WITH_KI_SUL,
	SH_KI_SUL_WATER_SPRAYING,
	SH_MARINE_FESTIVAL_OF_KI_SUL,
	SH_SANDY_FESTIVAL_OF_KI_SUL,
	SH_KI_SUL_RAMPAGE,
	SH_COMMUNE_WITH_HYUN_ROK,
	SH_COLORS_OF_HYUN_ROK,
	SH_HYUN_ROKS_BREEZE,
	SH_HYUN_ROK_CANNON,
	SH_TEMPORARY_COMMUNION,
	SH_BLESSING_OF_MYSTICAL_CREATURES,

	HN_SELFSTUDY_TATICS,
	HN_SELFSTUDY_SOCERY,
	HN_DOUBLEBOWLINGBASH,
	HN_MEGA_SONIC_BLOW,
	HN_SHIELD_CHAIN_RUSH,
	HN_SPIRAL_PIERCE_MAX,
	HN_METEOR_STORM_BUSTER,
	HN_JUPITEL_THUNDER_STORM,
	HN_JACK_FROST_NOVA,
	HN_HELLS_DRIVE,
	HN_GROUND_GRAVITATION,
	HN_NAPALM_VULCAN_STRIKE,
	HN_BREAKINGLIMIT,
	HN_RULEBREAK,

	SKE_SKY_MASTERY,
	SKE_WAR_BOOK_MASTERY,
	SKE_RISING_SUN,
	SKE_NOON_BLAST,
	SKE_SUNSET_BLAST,
	SKE_RISING_MOON,
	SKE_MIDNIGHT_KICK,
	SKE_DAWN_BREAK,
	SKE_TWINKLING_GALAXY,
	SKE_STAR_BURST,
	SKE_STAR_CANNON,
	SKE_ALL_IN_THE_SKY,
	SKE_ENCHANTING_SKY,

	SS_TOKEDASU,
	SS_SHIMIRU,
	SS_AKUMUKESU,
	SS_SHINKIROU,
	SS_KAGEGARI,
	SS_KAGENOMAI,
	SS_KAGEGISSEN,
	SS_FUUMASHOUAKU,
	SS_FUUMAKOUCHIKU,
	SS_KUNAIWAIKYOKU,
	SS_KUNAIKAITEN,
	SS_KUNAIKUSSETSU,
	SS_SEKIENHOU,
	SS_REIKETSUHOU,
	SS_RAIDENPOU,
	SS_KINRYUUHOU,
	SS_ANTENPOU,
	SS_KAGEAKUMU,
	SS_HITOUAKUMU,
	SS_ANKOKURYUUAKUMU,

	NW_THE_VIGILANTE_AT_NIGHT_GUN_GATLING,
	NW_THE_VIGILANTE_AT_NIGHT_GUN_SHOTGUN,

	SS_FOUR_CHARM = 5499,
	NW_WILD_SHOT,
	NW_MIDNIGHT_FALLEN,
	SKE_SKY_SUN,
	SKE_SKY_MOON,
	SKE_STAR_LIGHT_KICK,
	HN_OVERCOMING_CRISIS,
	SH_CHUL_HO_BATTERING,
	SH_HYUN_ROK_SPIRIT_POWER,	// 5507

	DK_DRAGONIC_BREATH = 6001,
	MT_SPARK_BLASTER,
	MT_TRIPLE_LASER,
	MT_MIGHTY_SMASH,
	BO_EXPLOSIVE_POWDER,
	BO_MAYHEMIC_THORNS,

	DK_DRAGONIC_PIERCE = 6502,
	IG_RADIANT_SPEAR,
	IG_IMPERIAL_CROSS,
	IG_IMPERIAL_PRESSURE,
	MT_RUSH_STRIKE,
	MT_POWERFUL_SWING,
	MT_ENERGY_CANNONADE,
	BO_MYSTERY_POWDER,
	BO_DUST_EXPLOSION,
	SHC_CROSS_SLASH,
	ABC_HIT_AND_SLIDING,
	ABC_CHASING_BREAK,
	ABC_CHASING_SHOT,
	ABC_ABYSS_FLAME,
	AG_ENERGY_CONVERSION,
	EM_PSYCHIC_STREAM,
	CD_DIVINUS_FLOS,
	IQ_BLAZING_FLAME_BLAST,
	WH_WILD_WALK,
	TR_RHYTHMICAL_WAVE,
	ABC_ABYSS_FLAME_ATK,	// 6522

	HLIF_HEAL = 8001,
	HLIF_AVOID,
	HLIF_BRAIN,
	HLIF_CHANGE,
	HAMI_CASTLE,
	HAMI_DEFENCE,
	HAMI_SKIN,
	HAMI_BLOODLUST,
	HFLI_MOON,
	HFLI_FLEET,
	HFLI_SPEED,
	HFLI_SBR44,
	HVAN_CAPRICE,
	HVAN_CHAOTIC,
	HVAN_INSTRUCT,
	HVAN_EXPLOSION,
	MUTATION_BASEJOB,
	MH_SUMMON_LEGION,
	MH_NEEDLE_OF_PARALYZE,
	MH_POISON_MIST,
	MH_PAIN_KILLER,
	MH_LIGHT_OF_REGENE,
	MH_OVERED_BOOST,
	MH_ERASER_CUTTER,
	MH_XENO_SLASHER,
	MH_SILENT_BREEZE,
	MH_STYLE_CHANGE,
	MH_SONIC_CRAW,
	MH_SILVERVEIN_RUSH,
	MH_MIDNIGHT_FRENZY,
	MH_STAHL_HORN,
	MH_GOLDENE_FERSE,
	MH_STEINWAND,
	MH_HEILIGE_STANGE,
	MH_ANGRIFFS_MODUS,
	MH_TINDER_BREAKER,
	MH_CBC,
	MH_EQC,
	MH_MAGMA_FLOW,
	MH_GRANITIC_ARMOR,
	MH_LAVA_SLIDE,
	MH_PYROCLASTIC,
	MH_VOLCANIC_ASH,
	MH_BLAST_FORGE,
	MH_TEMPERING,
	MH_CLASSY_FLUTTER,
	MH_TWISTER_CUTTER,
	MH_ABSOLUTE_ZEPHYR,
	MH_BRUSHUP_CLAW,
	MH_BLAZING_AND_FURIOUS,
	MH_THE_ONE_FIGHTER_RISES,
	MH_POLISHING_NEEDLE,
	MH_TOXIN_OF_MANDARA,
	MH_NEEDLE_STINGER,
	MH_LICHT_GEHORN,
	MH_GLANZEN_SPIES,
	MH_HEILIGE_PFERD,
	MH_GOLDENE_TONE,
	MH_BLAZING_LAVA,

	MS_BASH = 8201,
	MS_MAGNUM,
	MS_BOWLINGBASH,
	MS_PARRYING,
	MS_REFLECTSHIELD,
	MS_BERSERK,
	MA_DOUBLE,
	MA_SHOWER,
	MA_SKIDTRAP,
	MA_LANDMINE,
	MA_SANDMAN,
	MA_FREEZINGTRAP,
	MA_REMOVETRAP,
	MA_CHARGEARROW,
	MA_SHARPSHOOTING,
	ML_PIERCE,
	ML_BRANDISH,
	ML_SPIRALPIERCE,
	ML_DEFENDER,
	ML_AUTOGUARD,
	ML_DEVOTION,
	MER_MAGNIFICAT,
	MER_QUICKEN,
	MER_SIGHT,
	MER_CRASH,
	MER_REGAIN,
	MER_TENDER,
	MER_BENEDICTION,
	MER_RECUPERATE,
	MER_MENTALCURE,
	MER_COMPRESS,
	MER_PROVOKE,
	MER_AUTOBERSERK,
	MER_DECAGI,
	MER_SCAPEGOAT,
	MER_LEXDIVINA,
	MER_ESTIMATION,
	MER_KYRIE,
	MER_BLESSING,
	MER_INCAGI,
	MER_INVINCIBLEOFF2,

	EL_CIRCLE_OF_FIRE = 8401,
	EL_FIRE_CLOAK,
	EL_FIRE_MANTLE,
	EL_WATER_SCREEN,
	EL_WATER_DROP,
	EL_WATER_BARRIER,
	EL_WIND_STEP,
	EL_WIND_CURTAIN,
	EL_ZEPHYR,
	EL_SOLID_SKIN,
	EL_STONE_SHIELD,
	EL_POWER_OF_GAIA,
	EL_PYROTECHNIC,
	EL_HEATER,
	EL_TROPIC,
	EL_AQUAPLAY,
	EL_COOLER,
	EL_CHILLY_AIR,
	EL_GUST,
	EL_BLAST,
	EL_WILD_STORM,
	EL_PETROLOGY,
	EL_CURSED_SOIL,
	EL_UPHEAVAL,
	EL_FIRE_ARROW,
	EL_FIRE_BOMB,
	EL_FIRE_BOMB_ATK,
	EL_FIRE_WAVE,
	EL_FIRE_WAVE_ATK,
	EL_ICE_NEEDLE,
	EL_WATER_SCREW,
	EL_WATER_SCREW_ATK,
	EL_TIDAL_WEAPON,
	EL_WIND_SLASH,
	EL_HURRICANE,
	EL_HURRICANE_ATK,
	EL_TYPOON_MIS,
	EL_TYPOON_MIS_ATK,
	EL_STONE_HAMMER,
	EL_ROCK_CRUSHER,
	EL_ROCK_CRUSHER_ATK,
	EL_STONE_RAIN,
	EM_EL_FLAMETECHNIC,
	EM_EL_FLAMEARMOR,
	EM_EL_FLAMEROCK,
	EM_EL_COLD_FORCE,
	EM_EL_CRYSTAL_ARMOR,
	EM_EL_AGE_OF_ICE,
	EM_EL_GRACE_BREEZE,
	EM_EL_EYES_OF_STORM,
	EM_EL_STORM_WIND,
	EM_EL_EARTH_CARE,
	EM_EL_STRONG_PROTECTION,
	EM_EL_AVALANCHE,
	EM_EL_DEEP_POISONING,
	EM_EL_POISON_SHIELD,
	EM_EL_DEADLY_POISON,

	ABR_BATTLE_BUSTER = 8601,
	ABR_DUAL_CANNON_FIRE,
	ABR_NET_REPAIR,
	ABR_NET_SUPPORT,
	ABR_INFINITY_BUSTER,
};

/// The client view ids for land skills.
enum e_skill_unit_id : uint16 {
	UNT_SAFETYWALL = 0x7e,
	UNT_FIREWALL,
	UNT_WARP_WAITING,
	UNT_WARP_ACTIVE,
	UNT_BENEDICTIO, //TODO
	UNT_SANCTUARY,
	UNT_MAGNUS,
	UNT_PNEUMA,
	UNT_DUMMYSKILL, //These show no effect on the client
	UNT_FIREPILLAR_WAITING,
	UNT_FIREPILLAR_ACTIVE,
	UNT_HIDDEN_TRAP, //TODO
	UNT_TRAP, //TODO
	UNT_HIDDEN_WARP_NPC, //TODO
	UNT_USED_TRAPS,
	UNT_ICEWALL,
	UNT_QUAGMIRE,
	UNT_BLASTMINE,
	UNT_SKIDTRAP,
	UNT_ANKLESNARE,
	UNT_VENOMDUST,
	UNT_LANDMINE,
	UNT_SHOCKWAVE,
	UNT_SANDMAN,
	UNT_FLASHER,
	UNT_FREEZINGTRAP,
	UNT_CLAYMORETRAP,
	UNT_TALKIEBOX,
	UNT_VOLCANO,
	UNT_DELUGE,
	UNT_VIOLENTGALE,
	UNT_LANDPROTECTOR,
	UNT_LULLABY,
	UNT_RICHMANKIM,
	UNT_ETERNALCHAOS,
	UNT_DRUMBATTLEFIELD,
	UNT_RINGNIBELUNGEN,
	UNT_ROKISWEIL,
	UNT_INTOABYSS,
	UNT_SIEGFRIED,
	UNT_DISSONANCE,
	UNT_WHISTLE,
	UNT_ASSASSINCROSS,
	UNT_POEMBRAGI,
	UNT_APPLEIDUN,
	UNT_UGLYDANCE,
	UNT_HUMMING,
	UNT_DONTFORGETME,
	UNT_FORTUNEKISS,
	UNT_SERVICEFORYOU,
	UNT_GRAFFITI,
	UNT_DEMONSTRATION,
	UNT_CALLFAMILY,
	UNT_GOSPEL,
	UNT_BASILICA,
	UNT_MOONLIT,
	UNT_FOGWALL,
	UNT_SPIDERWEB,
	UNT_GRAVITATION,
	UNT_HERMODE,
	UNT_KAENSIN, //TODO
	UNT_SUITON,
	UNT_TATAMIGAESHI,
	UNT_KAEN,
	UNT_GROUNDDRIFT_WIND,
	UNT_GROUNDDRIFT_DARK,
	UNT_GROUNDDRIFT_POISON,
	UNT_GROUNDDRIFT_WATER,
	UNT_GROUNDDRIFT_FIRE,
	UNT_DEATHWAVE, //TODO
	UNT_WATERATTACK, //TODO
	UNT_WINDATTACK, //TODO
	UNT_EARTHQUAKE,
	UNT_EVILLAND,
	UNT_DARK_RUNNER, //TODO
	UNT_DARK_TRANSFER, //TODO
	UNT_EPICLESIS,
	UNT_EARTHSTRAIN,
	UNT_MANHOLE,
	UNT_DIMENSIONDOOR,
	UNT_CHAOSPANIC,
	UNT_MAELSTROM,
	UNT_BLOODYLUST,
	UNT_FEINTBOMB,
	UNT_MAGENTATRAP,
	UNT_COBALTTRAP,
	UNT_MAIZETRAP,
	UNT_VERDURETRAP,
	UNT_FIRINGTRAP,
	UNT_ICEBOUNDTRAP,
	UNT_ELECTRICSHOCKER,
	UNT_CLUSTERBOMB,
	UNT_REVERBERATION,
	UNT_SEVERE_RAINSTORM,
	UNT_FIREWALK,
	UNT_ELECTRICWALK,
	UNT_NETHERWORLD,
	UNT_PSYCHIC_WAVE,
	UNT_CLOUD_KILL,
	UNT_POISONSMOKE,
	UNT_NEUTRALBARRIER,
	UNT_STEALTHFIELD,
	UNT_WARMER,
	UNT_THORNS_TRAP,
	UNT_WALLOFTHORN,
	UNT_DEMONIC_FIRE,
	UNT_FIRE_EXPANSION_SMOKE_POWDER,
	UNT_FIRE_EXPANSION_TEAR_GAS,
	UNT_HELLS_PLANT, // No longer a unit skill
	UNT_VACUUM_EXTREME,
	UNT_BANDING,
	UNT_FIRE_MANTLE,
	UNT_WATER_BARRIER,
	UNT_ZEPHYR,
	UNT_POWER_OF_GAIA,
	UNT_FIRE_INSIGNIA,
	UNT_WATER_INSIGNIA,
	UNT_WIND_INSIGNIA,
	UNT_EARTH_INSIGNIA,
	UNT_POISON_MIST,
	UNT_LAVA_SLIDE,
	UNT_VOLCANIC_ASH,
	UNT_ZENKAI_WATER,
	UNT_ZENKAI_LAND,
	UNT_ZENKAI_FIRE,
	UNT_ZENKAI_WIND,
	UNT_MAKIBISHI,
	UNT_VENOMFOG,
	UNT_ICEMINE,
	UNT_FLAMECROSS,
	UNT_HELLBURNING,
	UNT_MAGMA_ERUPTION,
	UNT_KINGS_GRACE,

	UNT_GLITTERING_GREED,
	UNT_B_TRAP,
	UNT_FIRE_RAIN,

	UNT_CATNIPPOWDER,
	UNT_NYANGGRASS,

	UNT_CREATINGSTAR,// Should be GROUNDDRIFT_NEUTRAL
	UNT_DUMMY_0,// CREATINGSTAR

	UNT_RAIN_OF_CRYSTAL,
	UNT_MYSTERY_ILLUSION,

	UNT_STRANTUM_TREMOR = 269,
	UNT_VIOLENT_QUAKE,
	UNT_ALL_BLOOM,
	UNT_TORNADO_STORM,
	UNT_FLORAL_FLARE_ROAD,
	UNT_ASTRAL_STRIKE,
	UNT_CROSS_RAIN,
	UNT_PNEUMATICUS_PROCELLA,
	UNT_ABYSS_SQUARE,
	UNT_ACIDIFIED_ZONE_WATER,
	UNT_ACIDIFIED_ZONE_GROUND,
	UNT_ACIDIFIED_ZONE_WIND,
	UNT_ACIDIFIED_ZONE_FIRE,
	UNT_LIGHTNING_LAND,
	UNT_VENOM_SWAMP,
	UNT_CONFLAGRATION,
	UNT_CANE_OF_EVIL_EYE,
	UNT_TWINKLING_GALAXY,
	UNT_STAR_CANNON,
	UNT_GRENADES_DROPPING,

	UNT_FUUMASHOUAKU = 290, // Huuma Shuriken - Grasp
	UNT_MISSION_BOMBARD,
	UNT_TOTEM_OF_TUTELARY,
	UNT_HYUN_ROKS_BREEZE,
	UNT_SHINKIROU, // Mirage
	UNT_JACK_FROST_NOVA,
	UNT_GROUND_GRAVITATION,
	UNT_KUNAIKAITEN, // Shows Nothing
	UNT_KUNAIWAIKYOKU, // Kunai - Distortion

	UNT_STAR_BURST = 2409,

	// Skill units outside the normal unit range.
	UNT_DEEPBLINDTRAP = 20852,
	UNT_SOLIDTRAP,
	UNT_SWIFTTRAP,
	UNT_FLAMETRAP,

	/**
	 * Guild Auras
	 **/
	UNT_GD_LEADERSHIP = 0xc1,
	UNT_GD_GLORYWOUNDS = 0xc2,
	UNT_GD_SOULCOLD = 0xc3,
	UNT_GD_HAWKEYES = 0xc4,

	UNT_MAX = 0x190
};

/**
 * Skill Unit Save
 **/
void skill_usave_add(map_session_data * sd, uint16 skill_id, uint16 skill_lv);
void skill_usave_trigger(map_session_data *sd);

/**
 * Warlock
 **/
enum e_wl_spheres {
	WLS_FIRE = 0x44,
	WLS_WIND,
	WLS_WATER,
	WLS_STONE,
};

struct s_skill_spellbook_db {
	uint16 skill_id, points;
	t_itemid nameid;
};

class ReadingSpellbookDatabase : public TypesafeYamlDatabase<uint16, s_skill_spellbook_db> {
public:
	ReadingSpellbookDatabase() : TypesafeYamlDatabase("READING_SPELLBOOK_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;

	// Additional
	std::shared_ptr<s_skill_spellbook_db> findBook(t_itemid nameid);
};

extern ReadingSpellbookDatabase reading_spellbook_db;

void skill_spellbook(map_session_data &sd, t_itemid nameid);

int32 skill_block_check(struct block_list *bl, enum sc_type type, uint16 skill_id);

struct s_skill_magicmushroom_db {
	uint16 skill_id;
};

class MagicMushroomDatabase : public TypesafeYamlDatabase<uint16, s_skill_magicmushroom_db> {
public:
	MagicMushroomDatabase() : TypesafeYamlDatabase("MAGIC_MUSHROOM_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
};

extern MagicMushroomDatabase magic_mushroom_db;

int32 skill_maelstrom_suction(struct block_list *bl, va_list ap);
bool skill_check_shadowform(struct block_list *bl, int64 damage, int32 hit);

/**
 * Ranger
 **/
int32 skill_detonator(struct block_list *bl, va_list ap);
bool skill_check_camouflage(struct block_list *bl, struct status_change_entry *sce);

/**
 * Mechanic
 **/
void skill_magicdecoy( map_session_data& sd, t_itemid nameid );

/**
 * Guiltoine Cross
 **/
void skill_poisoningweapon( map_session_data& sd, t_itemid nameid );

/**
 * Auto Shadow Spell (Shadow Chaser)
 **/
void skill_select_menu( map_session_data& sd, uint16 skill_id );

int32 skill_elementalanalysis( map_session_data& sd, int32 n, uint16 skill_lv, uint16 *item_list ); // Sorcerer Four Elemental Analisys.
int32 skill_changematerial(map_session_data *sd, int32 n, uint16 *item_list);	// Genetic Change Material.
int32 skill_get_elemental_type(uint16 skill_id, uint16 skill_lv);

int32 skill_banding_count(map_session_data *sd);

int32 skill_is_combo(uint16 skill_id);
void skill_combo_toggle_inf(struct block_list* bl, uint16 skill_id, int32 inf);
void skill_combo(struct block_list* src,struct block_list *dsrc, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick);

enum sc_type skill_get_sc(int16 skill_id);
void skill_reveal_trap_inarea(struct block_list *src, int32 range, int32 x, int32 y);
int32 skill_get_time3(struct map_data *mapdata, uint16 skill_id, uint16 skill_lv);

/// Variable name of copied skill by Plagiarism
#define SKILL_VAR_PLAGIARISM "CLONE_SKILL"
/// Variable name of copied skill level by Plagiarism
#define SKILL_VAR_PLAGIARISM_LV "CLONE_SKILL_LV"

/// Variable name of copied skill by Reproduce
#define SKILL_VAR_REPRODUCE "REPRODUCE_SKILL"
/// Variable name of copied skill level by Reproduce
#define SKILL_VAR_REPRODUCE_LV "REPRODUCE_SKILL_LV"

#define SKILL_CHK_HOMUN(skill_id) ( (skill_id) >= HM_SKILLBASE && (skill_id) < HM_SKILLBASE+MAX_HOMUNSKILL )
#define SKILL_CHK_MERC(skill_id)  ( (skill_id) >= MC_SKILLBASE && (skill_id) < MC_SKILLBASE+MAX_MERCSKILL )
#define SKILL_CHK_ELEM(skill_id)  ( (skill_id) >= EL_SKILLBASE && (skill_id) < EL_SKILLBASE+MAX_ELEMENTALSKILL )
#define SKILL_CHK_ABR(skill_id)   ( (skill_id) >= ABR_SKILLBASE && (skill_id) < ABR_SKILLBASE+MAX_ABRSKILL )
#define SKILL_CHK_GUILD(skill_id) ( (skill_id) >= GD_SKILLBASE && (skill_id) < GD_SKILLBASE+MAX_GUILDSKILL )

#endif /* SKILL_HPP */
