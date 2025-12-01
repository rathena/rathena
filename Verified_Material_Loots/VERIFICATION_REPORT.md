# Verification Report: Pure Mob Drop Materials Database

**Generated:** 2025-12-01 02:55:29

---

## Summary Statistics

| Metric | Count |
|--------|-------|
| Total items scanned from item_db_etc.yml | 1115 |
| Items PASSED verification | 129 |
| Items EXCLUDED | 986 |
| Pass Rate | 11.6% |

## Exclusion Breakdown

| Exclusion Category | Count |
|-------------------|-------|
| Quest Requirements (delitem/countitem) | 1746 |
| No Mob Drops in mob_db.yml | 652 |
| Trade Restricted | 445 |
| Job Quest Items | 274 |
| Quest Rewards (getitem) | 254 |
| Quest-Related Name | 118 |
| Event Scripts | 89 |
| NPC Shop Items | 64 |
| Other Script References | 64 |
| Custom Script References | 20 |
| Instance Rewards | 15 |
| Guild/WoE Items | 12 |
| Battleground Items | 4 |

## Item ID Ranges Analyzed

- **900 - 1100**: 196 items
- **7000 - 7999**: 919 items

## Files Searched

**Total NPC script files scanned:** 1127

### By Category

- **Bg Scripts**: 15 files
- **Custom Scripts**: 33 files
- **Event Scripts**: 45 files
- **Guild Scripts**: 56 files
- **Instance Scripts**: 52 files
- **Job Scripts**: 70 files
- **Merchant Scripts**: 91 files
- **Other Npc**: 550 files
- **Quest Scripts**: 215 files

## Methodology

### Data Sources

1. **Primary Item Source**: `db/pre-re/item_db_etc.yml`
2. **Mob Drop Source**: `db/pre-re/mob_db.yml`
3. **Exclusion Source**: All `.txt` files in `npc/` directory

### Exclusion Criteria

Items were excluded if ANY of the following conditions were met:

1. **Trade Restricted**: NoDrop=true or NoTrade=true
2. **No Mob Drops**: Not in any mob's drop list in mob_db.yml
3. **Quest-Named**: Name contains: key, token, letter, permit, certificate, pass, ticket, voucher, evidence, proof, badge
4. **NPC Shops**: Referenced in sellitem or shop scripts
5. **Quest Rewards**: getitem in quest scripts
6. **Quest Requirements**: delitem/countitem in quest scripts
7. **Event/Instance/Job/Guild**: Any reference in these script types

### Excluded from Scanning

- Documentation files (`doc/`)
- Test files (`test/`)
- Sample files (`sample/`)
- Source code (`src/`)
- Third-party libraries (`3rdparty/`)

## Confidence Assessment

All items in SAFE_MATERIALS.txt have **HIGH CONFIDENCE**:

- ✅ Defined in item_db_etc.yml
- ✅ Has valid mob drops in mob_db.yml
- ✅ No trade restrictions
- ✅ No quest-related naming
- ✅ Zero references in any NPC scripts
- ✅ Safe for custom content

## Verified Safe Items

| ID | AegisName | Name | Weight | Drop Sources |
|-----|-----------|------|--------|-------------|
| 902 | Tree_Root | Tree Root | 10 | 2 mob(s) |
| 903 | Reptile_Tongue | Reptile Tongue | 10 | 2 mob(s) |
| 922 | Orcish_Cuspid | Orc's Fang | 10 | 1 mob(s) |
| 928 | Insect_Feeler | Insect Feeler | 10 | 6 mob(s) |
| 967 | Turtle_Shell | Turtle Shell | 10 | 6 mob(s) |
| 1014 | Jaws_Of_Ant | Ant Jaw | 10 | 2 mob(s) |
| 1031 | Limb_Of_Mantis | Mantis Scythe | 10 | 2 mob(s) |
| 1034 | Cobold_Hair | Blue Hair | 10 | 5 mob(s) |
| 1039 | Petite_DiablOfs_Wing | Little Evil Wing | 10 | 6 mob(s) |
| 1044 | Tooth_Of_ | Zenorc's Fang | 10 | 1 mob(s) |
| 1045 | Sacred_Masque | Cultish Masque | 10 | 2 mob(s) |
| 1056 | Grit | Grit | 10 | 3 mob(s) |
| 1063 | Sharpened_Cuspid | Fang | 10 | 1 mob(s) |
| 1098 | Manacles | Manacles | 10 | 2 mob(s) |
| 7006 | Wing_Of_Red_Bat | Wing of Red Bat | 10 | 3 mob(s) |
| 7007 | Claw_Of_Rat | Claw of Rat | 10 | 1 mob(s) |
| 7008 | Stiff_Horn | Stiff Horn | 10 | 1 mob(s) |
| 7009 | Glitter_Shell | Glitter Shell | 10 | 1 mob(s) |
| 7010 | Tail_Of_Steel_Scorpion | Tail of Steel Scorpion | 10 | 1 mob(s) |
| 7011 | Claw_Of_Monkey | Claw of Monkey | 10 | 2 mob(s) |
| 7016 | Spoon_Stub | Spoon Stub | 20 | 3 mob(s) |
| 7067 | Stone_Piece | Stone Fragment | 10 | 3 mob(s) |
| 7070 | Broken_Shell | Broken Shell | 10 | 5 mob(s) |
| 7071 | Tatters_Clothes | Tattered Clothes | 10 | 3 mob(s) |
| 7072 | Rust_Suriken | Old Shuriken | 10 | 1 mob(s) |
| 7095 | Broken_Steel_Piece | Metal Fragment | 10 | 3 mob(s) |
| 7107 | Gaoat's_Skin | Antelope Skin | 10 | 1 mob(s) |
| 7108 | Boroken_Shiled_Piece | Piece of Shield | 10 | 1 mob(s) |
| 7109 | Shine_Spear_Blade | Shining Spear Blade | 10 | 1 mob(s) |
| 7113 | Broken_Pharaoh_Symbol | Broken Pharaoh Emblem | 10 | 1 mob(s) |
| 7116 | Harpy's_Claw | Harpy Talon | 10 | 2 mob(s) |
| 7124 | Sand_Lump | Sand Clump | 10 | 1 mob(s) |
| 7125 | Scropion's_Nipper | Scorpion Claw | 10 | 1 mob(s) |
| 7154 | Poison_Powder | Poisonous Powder | 10 | 3 mob(s) |
| 7159 | Tengu's_Nose | Tengu Nose | 10 | 1 mob(s) |
| 7162 | Cloud_Piece | Cloud Crumb | 10 | 1 mob(s) |
| 7163 | Sharp_Feeler | Hard Feeler | 10 | 1 mob(s) |
| 7168 | Great_Wing | Giant Butterfly Wing | 10 | 1 mob(s) |
| 7169 | Taegeuk_Plate | Ba Gua | 10 | 1 mob(s) |
| 7171 | Leopard_Skin | Leopard Skin | 10 | 2 mob(s) |
| 7186 | Thin_Stem | Thin Trunk | 10 | 1 mob(s) |
| 7192 | Blade_Of_Pinwheel | Vane | 10 | 1 mob(s) |
| 7195 | Air_Rifle | Slingshot | 10 | 1 mob(s) |
| 7202 | Beetle_Nipper | Pincher of Beetle | 10 | 1 mob(s) |
| 7203 | Solid_Twig | Strong Branch | 10 | 2 mob(s) |
| 7208 | Rusty_Cleaver | Rusty Kitchen Knife | 10 | 1 mob(s) |
| 7209 | Dullahan's_Helm | Helm of Dullahan | 10 | 1 mob(s) |
| 7210 | Dullahan_Armor | Armor Piece of Dullahan | 10 | 1 mob(s) |
| 7211 | Rojerta_Piece | Fragment of Rossata Stone | 10 | 1 mob(s) |
| 7214 | Bat_Cage | Bat Cage | 10 | 1 mob(s) |
| 7215 | Broken_Needle | Broken Needle | 10 | 1 mob(s) |
| 7218 | Rotten_Rope | Decomposed Rope | 10 | 1 mob(s) |
| 7219 | Striped_Socks | Striped Sock | 10 | 1 mob(s) |
| 7221 | Tangled_Chain | Tangled Chains | 10 | 1 mob(s) |
| 7223 | Distorted_Portrait | Contorted Self-Portrait | 10 | 1 mob(s) |
| 7225 | Pumpkin_Bucket | Pumpkin Lantern | 10 | 3 mob(s) |
| 7226 | Pill | Pellet | 10 | 2 mob(s) |
| 7264 | Dried_Sand | Dry Sand | 10 | 1 mob(s) |
| 7265 | Dragon_Horn | Dragon Horn | 10 | 1 mob(s) |
| 7266 | Dragon_Fang | Denture from Dragon Mask | 10 | 1 mob(s) |
| 7268 | Little_Blacky_Ghost | Little Ghost Doll | 10 | 2 mob(s) |
| 7269 | Bib | Pinafore | 10 | 2 mob(s) |
| 7272 | Meat_Dumpling_Doll | Rice Ball Doll | 10 | 1 mob(s) |
| 7299 | Bamboo_Basket | Straw Basket | 10 | 2 mob(s) |
| 7300 | Gemstone | Gemstone | 10 | 1 mob(s) |
| 7303 | Bag_Of_Rice | Straw Rice Bag | 800 | 1 mob(s) |
| 7316 | Long_Limb | Insect Leg | 10 | 1 mob(s) |
| 7318 | Old_Pick | Old Pick | 10 | 1 mob(s) |
| 7320 | Air_Pollutant | Dust Pollutant | 10 | 2 mob(s) |
| 7322 | Poisonous_Gas | Toxic Gas | 10 | 2 mob(s) |
| 7323 | Battered_Kettle | Battered Kettle | 10 | 1 mob(s) |
| 7327 | Headlamp | Flashlight | 10 | 2 mob(s) |
| 7435 | Golden_Bracelet | Golden Ornament | 10 | 1 mob(s) |
| 7436 | Piece_Of_Memory_Green | Fragment of Agony | 10 | 1 mob(s) |
| 7437 | Piece_Of_Memory_Purple | Fragment of Misery | 10 | 1 mob(s) |
| 7438 | Piece_Of_Memory_Blue | Fragment of Hatred | 10 | 1 mob(s) |
| 7439 | Piece_Of_Memory_Red | Fragment of Despair | 10 | 1 mob(s) |
| 7440 | Red_Feather | Red Feather | 10 | 2 mob(s) |
| 7441 | Blue_Feather | Blue Feather | 10 | 2 mob(s) |
| 7442 | Cursed_Seal | Cursed Seal | 10 | 5 mob(s) |
| 7443 | Tri_Headed_Dragon_Head | Three-Headed Dragon's Head | 10 | 1 mob(s) |
| 7450 | Piece_Of_Bone_Armor | Skeletal Armor Piece | 10 | 1 mob(s) |
| 7477 | Cookbook06 | Level 6 Cookbook | 10 | 3 mob(s) |
| 7478 | Cookbook07 | Level 7 Cookbook | 10 | 1 mob(s) |
| 7479 | Cookbook08 | Level 8 Cookbook | 10 | 2 mob(s) |
| 7480 | Cookbook09 | Level 9 Cookbook | 10 | 2 mob(s) |
| 7481 | Cookbook10 | Level 10 Cookbook | 10 | 1 mob(s) |
| 7510 | Valhalla_Flower | Valhalla's Flower | 10 | 2 mob(s) |
| 7512 | Burnt_Parts | Burnt Part | 10 | 3 mob(s) |
| 7513 | Pocket_Watch | Pocket Watch | 10 | 1 mob(s) |
| 7565 | Sticky_Poison | Sticky Poison | 10 | 2 mob(s) |
| 7566 | Will_Of_Darkness_ | Will of Red Darkness | 10 | 1 mob(s) |
| 7567 | Suspicious_Hat | Suspicious Hat | 10 | 4 mob(s) |
| 7568 | White_Mask | White Mask | 10 | 4 mob(s) |
| 7578 | Anti_Spell_Bead | Countermagic Crystal | 10 | 1 mob(s) |
| 7582 | Gem_Of_Ruin | Jewel of Destruction | 10 | 1 mob(s) |
| 7607 | Evil_Dragon_Head | Neck of Demon Dragon | 10 | 1 mob(s) |
| 7613 | Small_Rice_Dough | Small Rice Cake Dough | 0 | 8 mob(s) |
| 7701 | Dragon_Spirit | Soul | 10 | 1 mob(s) |
| 7703 | Piece_Of_Cogwheel | Piece of Cogwheel | 10 | 1 mob(s) |
| 7742 | Kaong | Kaong | 10 | 1 mob(s) |
| 7743 | Gulaman | Gulaman | 10 | 1 mob(s) |
| 7744 | Leche_Flan | Leche Flan | 10 | 1 mob(s) |
| 7745 | Ube_Jam | Ube Jam | 10 | 1 mob(s) |
| 7746 | Sago | Sago | 10 | 1 mob(s) |
| 7747 | Langka | Langka | 10 | 1 mob(s) |
| 7748 | Sweet_Bean | Sweet Beans | 10 | 1 mob(s) |
| 7749 | Sweet_Banana | Sweet Bananas | 10 | 1 mob(s) |
| 7750 | Macapuno | Macapuno | 10 | 1 mob(s) |
| 7751 | Old_White_Cloth | Old White Cloth | 10 | 2 mob(s) |
| 7752 | Clattering_Skull | Clattering Skull | 10 | 3 mob(s) |
| 7753 | Broken_Farming_Utensil | Broken Farming Utensil | 10 | 1 mob(s) |
| 7754 | Broken_Crown | Broken Crown | 10 | 2 mob(s) |
| 7781 | Heart_Box | Engrave Treasure Box | 10 | 1 mob(s) |
| 7830 | Goddess_Tear | Goddess Tear | 500 | 1 mob(s) |
| 7832 | Brynhild_Armor_Piece | Brynhild Armor Piece | 500 | 1 mob(s) |
| 7833 | Hero_Remains | Hero's Remains | 500 | 1 mob(s) |
| 7834 | Andvari_Ring | Andvari's Ring | 500 | 1 mob(s) |
| 7835 | Dusk_Glow | Dusk Glow | 500 | 1 mob(s) |
| 7836 | Dawn_Essence | Dawn Essence | 500 | 1 mob(s) |
| 7837 | Cold_Moonlight | Cold Moonlight | 500 | 1 mob(s) |
| 7838 | Hazy_Starlight | Hazy Starlight | 500 | 1 mob(s) |
| 7849 | Soul_Crystal | Soul Crystal | 10 | 1 mob(s) |
| 7850 | Wooden_Block_ | Wooden Block | 100 | 9 mob(s) |
| 7858 | Dragonball_Yellow_ | Dragonball Yellow | 10 | 1 mob(s) |
| 7860 | Peeps | Peeps | 50 | 1 mob(s) |
| 7861 | Jelly_Bean | Jelly Bean | 50 | 1 mob(s) |
| 7862 | Marshmallow | Marshmallow | 50 | 1 mob(s) |
| 7888 | Bag_Of_Nuts | Bag of Nuts | 30 | 1 mob(s) |

## Sample Excluded Items (First 30)

| ID | Name | Primary Reason |
|-----|------|---------------|
| 901 | Daenggie | NPC Shop: countitem in npc/pre-re/merchants/hai... |
| 904 | Scorpion Tail | Job quest: countitem in npc/jobs/2-2/rogue.txt |
| 905 | Stem | Quest requirement: countitem in npc/re/quests/q... |
| 906 | Pointed Scale | Quest requirement: countitem in npc/quests/skil... |
| 907 | Resin | Job quest: countitem in npc/jobs/novice/superno... |
| 908 | Spawn | Quest requirement: countitem in npc/re/quests/n... |
| 909 | Jellopy | Job quest: countitem in npc/re/jobs/novice/acad... |
| 910 | Garlet | Quest requirement: countitem in npc/re/quests/e... |
| 911 | Scell | Quest requirement: countitem in npc/quests/skil... |
| 912 | Zargon | Job quest: countitem in npc/jobs/1-1e/gunslinge... |
| 913 | Tooth of Bat | Job quest: countitem in npc/jobs/2-2/rogue.txt |
| 914 | Fluff | Job quest: countitem in npc/re/jobs/novice/acad... |
| 915 | Chrysalis | Quest requirement: countitem in npc/pre-re/ques... |
| 916 | Feather of Birds | Quest requirement: countitem in npc/re/quests/n... |
| 917 | Talon | Job quest: countitem in npc/jobs/2-2/rogue.txt |
| 918 | Sticky Webfoot | Custom script: countitem in npc/re/custom/lasag... |
| 919 | Animal Skin | Quest requirement: countitem in npc/re/quests/e... |
| 920 | Wolf Claw | Quest requirement: countitem in npc/quests/part... |
| 921 | Mushroom Spore | Custom script: countitem in npc/re/custom/lasag... |
| 923 | Evil Horn | Quest requirement: countitem in npc/custom/ques... |
| 924 | Powder of Butterfly | Quest requirement: countitem in npc/pre-re/ques... |
| 925 | Bill of Birds | Quest requirement: countitem in npc/pre-re/ques... |
| 926 | Snake Scale | Quest requirement: countitem in npc/quests/ques... |
| 929 | Immortal Heart | Job quest: countitem in npc/jobs/2-2e/SoulLinke... |
| 930 | Rotten Bandage | Quest requirement: countitem in npc/re/quests/q... |
| 931 | Orcish Voucher | Quest-named: Name contains 'voucher' |
| 932 | Skel-Bone | Quest requirement: countitem in npc/re/quests/e... |
| 934 | Memento | Quest requirement: countitem in npc/custom/ques... |
| 935 | Shell | Job quest: countitem in npc/jobs/1-1e/gunslinge... |
| 936 | Scale Shell | Quest requirement: countitem in npc/re/quests/n... |

---

*Generated by Mob Drop Materials Analyzer v3.0*
