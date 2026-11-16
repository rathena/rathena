# Auto Attack - skills not in any category

Player skills (in skill_tree) from `db/re/skill_db.yml` that are not passive but show in neither Skill Attack nor Skill Buff:
they deal no damage (`NoDamage`), are not cast on enemies (TargetType is not `Attack`), and are either Ground skills or Self/Support skills without a `Status`.

Fill in the **Category** column (e.g. Buff-interval, Heal, Spirit sphere, Skip).

Total: 174

| Job | ID | Skill | Name | TargetType | Category |
|---|---|---|---|---|---|
| Mage | 12 | MG_SAFETYWALL | Safety Wall | Ground | |
| Acolyte | 25 | AL_PNEUMA | Pneuma | Ground | |
| Acolyte | 26 | AL_TELEPORT | Teleport | Self | |
| Acolyte | 27 | AL_WARP | Warp Portal | Ground | |
| Acolyte | 28 | AL_HEAL | Heal | Support | |
| Acolyte | 31 | AL_HOLYWATER | Aqua Benedicta | Self | |
| Acolyte | 35 | AL_CURE | Cure | Support | |
| Merchant | 40 | MC_IDENTIFY | Item Appraisal | Self | |
| Merchant | 41 | MC_VENDING | Vending | Self | |
| Thief | 53 | TF_DETOXIFY | Detoxify | Support | |
| Priest | 54 | ALL_RESURRECTION | Resurrection | Support | |
| Priest | 69 | PR_BENEDICTIO | B.S. Sacramenti | Ground | |
| Priest | 70 | PR_SANCTUARY | Sanctuary | Ground | |
| Wizard | 87 | WZ_ICEWALL | Ice Wall | Ground | |
| Wizard | 92 | WZ_QUAGMIRE | Quagmire | Ground | |
| Blacksmith | 108 | BS_REPAIRWEAPON | Weapon Repair | Support | |
| Blacksmith | 110 | BS_HAMMERFALL | Hammer Fall | Ground | |
| Hunter | 115 | HT_SKIDTRAP | Skid Trap | Ground | |
| Hunter | 117 | HT_ANKLESNARE | Ankle Snare | Ground | |
| Hunter | 118 | HT_SHOCKWAVE | Shockwave Trap | Ground | |
| Hunter | 119 | HT_SANDMAN | Sandman | Ground | |
| Hunter | 120 | HT_FLASHER | Flasher | Ground | |
| Hunter | 124 | HT_REMOVETRAP | Remove Trap | Trap | |
| Hunter | 125 | HT_TALKIEBOX | Talkie Box | Ground | |
| Hunter | 130 | HT_DETECTING | Detect | Ground | |
| Hunter | 131 | HT_SPRINGTRAP | Spring Trap | Trap | |
| Assassin | 140 | AS_VENOMDUST | Venom Dust | Ground | |
| Novice | 142 | NV_FIRSTAID | First Aid | Self | |
| Archer | 147 | AC_MAKINGARROW | Arrow Crafting | Self | |
| Thief | 150 | TF_BACKSLIDING | Back Slide | Self | |
| Thief | 151 | TF_PICKSTONE | Find Stone | Self | |
| Merchant | 154 | MC_CHANGECART | Change Cart | Self | |
| Rogue | 220 | RG_GRAFFITI | Scribble | Ground | |
| Rogue | 221 | RG_FLAGGRAFFITI | Piece | Ground | |
| Rogue | 222 | RG_CLEANER | Remover | Ground | |
| Alchemist | 228 | AM_PHARMACY | Prepare Potion | Self | |
| Alchemist | 229 | AM_DEMONSTRATION | Bomb | Ground | |
| Alchemist | 231 | AM_POTIONPITCHER | Aid Potion | Support | |
| Alchemist | 232 | AM_CANNIBALIZE | Summon Flora | Ground | |
| Alchemist | 233 | AM_SPHEREMINE | Summon Marine Sphere | Ground | |
| Alchemist | 244 | AM_REST | Vaporize | Self | |
| Alchemist | 247 | AM_RESURRECTHOMUN | Homunculus Resurrection | Self | |
| Monk | 261 | MO_CALLSPIRITS | Summon Spirit Sphere | Self | |
| Monk | 262 | MO_ABSORBSPIRITS | Absorb Spirit Sphere | Support | |
| Monk | 264 | MO_BODYRELOCATION | Snap | Ground | |
| Sage | 275 | SA_CASTCANCEL | Cast Cancel | Self | |
| Sage | 285 | SA_VOLCANO | Volcano | Ground | |
| Sage | 286 | SA_DELUGE | Deluge | Ground | |
| Sage | 287 | SA_VIOLENTGALE | Whirlwind | Ground | |
| Sage | 288 | SA_LANDPROTECTOR | Magnetic Earth | Ground | |
| Sage | 290 | SA_ABRACADABRA | Hocus-pocus | Self | |
| Bard | 317 | BA_DISSONANCE | Unchained Serenade | Self | |
| Dancer | 325 | DC_UGLYDANCE | Hip Shaker | Self | |
| Professor | 373 | PF_HPCONVERSION | Indulge | Self | |
| Clown | 395 | CG_MOONLIT | Sheltering Bliss | Self | |
| Champion | 401 | CH_SOULCOLLECT | Zen | Self | |
| Professor | 404 | PF_FOGWALL | Blinding Mist | Ground | |
| Assassin_Cross | 407 | ASC_CDP | Create Deadly Poison | Self | |
| Baby | 409 | WE_CALLPARENT | Call Parent | Self | |
| Novice | 410 | WE_CALLBABY | Call Baby | Self | |
| Taekwon | 426 | TK_HIGHJUMP | Taekwon Jump | Self | |
| Star_Gladiator | 427 | SG_FEEL | Feeling the Sun Moon and Stars | Self | |
| Alchemist | 446 | AM_BERSERKPITCHER | Aid Berserk Potion | Support | |
| Whitesmith | 477 | WS_WEAPONREFINE | Upgrade Weapon | Self | |
| Creator | 478 | CR_SLIMPITCHER | Aid Condensed Potion | Ground | |
| Creator | 479 | CR_FULLPROTECTION | Full Protection | Support | |
| High_Wizard | 483 | HW_GANBANTEIN | Ganbantein | Ground | |
| Taekwon | 493 | TK_MISSION | Taekwon Mission | Self | |
| Alchemist | 496 | AM_TWILIGHT1 | Twilight Alchemy 1 | Self | |
| Alchemist | 497 | AM_TWILIGHT2 | Twilight Alchemy 2 | Self | |
| Alchemist | 498 | AM_TWILIGHT3 | Twilight Alchemy 3 | Self | |
| Gunslinger | 500 | GS_GLITTERING | Flip the Coin | Self | |
| Ninja | 529 | NJ_SHADOWJUMP | Shadow Leap | Ground | |
| Ninja | 538 | NJ_SUITON | Hidden Water | Ground | |
| Sage | 1007 | SA_CREATECON | Create Elemental Converter | Self | |
| Blacksmith | 1013 | BS_GREED | Greed | Self | |
| Priest | 1014 | PR_REDEMPTIO | Redemptio | Self | |
| Monk | 1015 | MO_KITRANSLATION | Ki Translation | Support | |
| Warlock | 2222 | WL_SUMMONFB | Summon Fire Ball | Self | |
| Warlock | 2223 | WL_SUMMONBL | Summon Lightning Ball | Self | |
| Warlock | 2224 | WL_SUMMONWB | Summon Water Ball | Self | |
| Warlock | 2229 | WL_SUMMONSTONE | Summon Stone | Self | |
| Guillotine_Cross | 2025 | GC_CREATENEWPOISON | Create New Poison | Self | |
| Guillotine_Cross | 2026 | GC_ANTIDOTE | Antidote | Support | |
| Guillotine_Cross | 2032 | GC_POISONSMOKE | Poison Smoke | Ground | |
| Arch_Bishop | 2039 | AB_ANCILLA | Ancilla | Self | |
| Arch_Bishop | 2043 | AB_CHEAL | Coluceo Heal | Self | |
| Arch_Bishop | 2044 | AB_EPICLESIS | Epiclesis | Ground | |
| Arch_Bishop | 2051 | AB_HIGHNESSHEAL | Highness Heal | Support | |
| Arch_Bishop | 2057 | AB_SILENTIUM | Silentium | Self | |
| Ranger | 2237 | RA_DETONATOR | Detonator | Ground | |
| Ranger | 2238 | RA_ELECTRICSHOCKER | Electric Shocker | Ground | |
| Ranger | 2249 | RA_MAGENTATRAP | Magenta Trap | Ground | |
| Ranger | 2250 | RA_COBALTTRAP | Cobalt Trap | Ground | |
| Ranger | 2251 | RA_MAIZETRAP | Maize Trap | Ground | |
| Ranger | 2252 | RA_VERDURETRAP | Verdure Trap | Ground | |
| Mechanic | 2264 | NC_F_SIDESLIDE | Front-Side Slide | Self | |
| Mechanic | 2265 | NC_B_SIDESLIDE | Back-Side Slide | Self | |
| Mechanic | 2269 | NC_EMERGENCYCOOL | Emergency Cool | Self | |
| Mechanic | 2275 | NC_REPAIR | Repair | Support | |
| Mechanic | 2281 | NC_SILVERSNIPER | FAW - Silver Sniper | Ground | |
| Mechanic | 2282 | NC_MAGICDECOY | FAW - Magic Decoy | Ground | |
| Shadow_Chaser | 2299 | SC_MANHOLE | Man Hole | Ground | |
| Shadow_Chaser | 2300 | SC_DIMENSIONDOOR | Dimension Door | Ground | |
| Shadow_Chaser | 2301 | SC_CHAOSPANIC | Chaos Panic | Ground | |
| Shadow_Chaser | 2302 | SC_MAELSTROM | Maelstrom | Ground | |
| Shadow_Chaser | 2303 | SC_BLOODYLUST | Bloody Lust | Ground | |
| Royal_Guard | 2309 | LG_TRAMPLE | Trample | Self | |
| Sura | 2330 | SR_TIGERCANNON | Tiger Cannon | Self | |
| Sura | 2340 | SR_ASSIMILATEPOWER | Assimilate Power | Self | |
| Sura | 2341 | SR_POWERVELOCITY | Power Velocity | Support | |
| Sura | 2345 | SR_GENTLETOUCH_CURE | Gentle Touch - Cure | Support | |
| Minstrel | 2418 | WM_SEVERE_RAINSTORM | Severe Rainstorm | Ground | |
| Minstrel | 2419 | WM_POEMOFNETHERWORLD | Poem of The Netherworld | Ground | |
| Minstrel | 2421 | WM_DEADHILLHERE | Valley of Death | Support | |
| Sorcerer | 2452 | SO_WARMER | Warmer | Ground | |
| Sorcerer | 2453 | SO_VACUUM_EXTREME | Vacuum Extreme | Ground | |
| Sorcerer | 2455 | SO_ARRULLO | Arrullo | Ground | |
| Sorcerer | 2456 | SO_EL_CONTROL | Spirit Control | Self | |
| Sorcerer | 2457 | SO_SUMMON_AGNI | Summon Fire Spirit Agni | Self | |
| Sorcerer | 2458 | SO_SUMMON_AQUA | Summon Water Spirit Aqua | Self | |
| Sorcerer | 2459 | SO_SUMMON_VENTUS | Summon Wind Spirit Ventus | Self | |
| Sorcerer | 2460 | SO_SUMMON_TERA | Summon Earth Spirit Tera | Self | |
| Sorcerer | 2462 | SO_EL_ANALYSIS | Four Spirit Analysis | Self | |
| Sorcerer | 2464 | SO_EL_CURE | Spirit Recovery | Self | |
| Sorcerer | 2465 | SO_FIRE_INSIGNIA | Fire Insignia | Ground | |
| Sorcerer | 2466 | SO_WATER_INSIGNIA | Water Insignia | Ground | |
| Sorcerer | 2467 | SO_WIND_INSIGNIA | Wind Insignia | Ground | |
| Sorcerer | 2468 | SO_EARTH_INSIGNIA | Earth Insignia | Ground | |
| Genetic | 2483 | GN_CRAZYWEED | Crazy Weed | Ground | |
| Genetic | 2494 | GN_CHANGEMATERIAL | Change Material | Self | |
| Genetic | 2495 | GN_MIX_COOKING | Mix Cooking | Self | |
| Genetic | 2497 | GN_S_PHARMACY | Special Pharmacy | Self | |
| Merchant | 2535 | ALL_BUYING_STORE | Open Buying Store | Self | |
| Merchant | 2544 | MC_CARTDECORATE | Decorate Cart | Self | |
| Rebellion | 2552 | RL_RICHS_COIN | Rich's Coin | Self | |
| Rebellion | 2555 | RL_B_TRAP | Bind Trap | Ground | |
| Rebellion | 2556 | RL_FLICKER | Flicker | Self | |
| Rebellion | 2564 | RL_FALLEN_ANGEL | Fallen Angel | Ground | |
| Star_Emperor | 2588 | SJ_DOCUMENT | Document of Sun Moon and Star | Self | |
| Soul_Reaper | 2608 | SP_SOULREVOLVE | Soul Revolution | Support | |
| Soul_Reaper | 2612 | SP_KAUTE | Kaute | Support | |
| Kagerou | 3012 | KO_ZANZOU | Illusion - Shadow | Self | |
| Kagerou | 3015 | KO_KAHU_ENTEN | Fire Charm | Self | |
| Kagerou | 3016 | KO_HYOUHU_HUBUKI | Ice Charm | Self | |
| Kagerou | 3017 | KO_KAZEHU_SEIRAN | Wind Charm | Self | |
| Kagerou | 3018 | KO_DOHU_KOUKAI | Earth Charm | Self | |
| Kagerou | 3021 | KO_GENWAKU | Illusion - Bewitch | Support | |
| Sorcerer | 5008 | SO_ELEMENTAL_SHIELD | Elemental Shield | Self | |
| Summoner | 5023 | SU_LOPE | Lope | Ground | |
| Summoner | 5038 | SU_TUNABELLY | Tuna Belly | Support | |
| Summoner | 5045 | SU_POWEROFFLOCK | Power of Flock | Self | |
| Summoner | 5048 | SU_NYANGGRASS | Nyang Grass | Ground | |
| Clown | 5068 | CG_SPECIALSINGER | Skilled Special Singer | Self | |
| Arch_Bishop | 5073 | AB_CONVENIO | Convenio | Self | |
| Cardinal | 5268 | CD_REPARATIO | Reparatio | Support | |
| Cardinal | 5280 | CD_DILECTIO_HEAL | Dilectio Heal | Support | |
| Meister | 5297 | MT_M_MACHINE | Manufacture Machine | Self | |
| Windhawk | 5327 | WH_HAWK_M | Hawk Mastery | Self | |
| Biolo | 5336 | BO_BIONIC_PHARMACY | Bionic Pharmacy | Self | |
| Biolo | 5338 | BO_THE_WHOLE_PROTECTION | The Whole Protection | Self | |
| Troubadour | 5350 | TR_RETROSPECTION | Retrospection | Self | |
| Troubadour | 5359 | TR_ROKI_CAPRICCIO | Roki Capriccio | Self | |
| Troubadour | 5363 | TR_NIPELHEIM_REQUIEM | Nipelheim Requiem | Self | |
| Elemental_Master | 5368 | EM_INCREASING_ACTIVITY | Increasing Activity | Support | |
| Spirit_Handler | 5439 | SH_KI_SUL_WATER_SPRAYING | Kisul Water Spraying | Self | |
| Spirit_Handler | 5444 | SH_COLORS_OF_HYUN_ROK | Colors of Hyunrok | Self | |
| Night_Watch | 5404 | NW_GRENADE_FRAGMENT | Grenade Fragment | Self | |
| Soul_Ascetic | 5421 | SOA_SOUL_GATHERING | Soul Gathering | Self | |
| Soul_Ascetic | 5422 | SOA_TOTEM_OF_TUTELARY | Totem Of Tutelary | Ground | |
| Shinkiro | 5478 | SS_AKUMUKESU | Nightmare Erasion | Self | |
| Shinkiro | 5479 | SS_SHINKIROU | Mirage | Ground | |
| Shinkiro | 5499 | SS_FOUR_CHARM | Four Colors Charm | Self | |
| Arch_Mage | 6516 | AG_ENERGY_CONVERSION | Energy Conversion | Self | |
