#!/usr/bin/env python3
"""
Generate material classification files WITH event mob filtering.
Separates:
- FARMABLE boss drops (regular boss mobs that spawn normally)
- EVENT-ONLY drops (event/guild mobs that don't spawn normally)
"""

import yaml
from pathlib import Path
from datetime import datetime
from collections import defaultdict

BASE_DIR = Path("/home/user/rathena2")
ITEM_DB = BASE_DIR / "db/pre-re/item_db_etc.yml"
MOB_DB = BASE_DIR / "db/pre-re/mob_db.yml"
SAFE_MATERIALS = BASE_DIR / "Verified_Material_Loots/SAFE_MATERIALS.txt"
OUTPUT_DIR = BASE_DIR / "Verified_Material_Loots"

# Prefixes that indicate event/guild/special mobs (don't spawn normally)
EVENT_PREFIXES = ['EVENT_', 'G_', 'E_']

# Patterns for WoE/Agit mobs (Treasure Chests, etc.)
WOE_PATTERNS = ['TREASURE_BOX', 'TREASURE_CHEST']

def is_event_mob(aegis_name):
    """Check if mob is an event/guild mob based on AegisName"""
    for prefix in EVENT_PREFIXES:
        if aegis_name.startswith(prefix):
            return True
    return False

def is_woe_mob(aegis_name):
    """Check if mob is a WoE/Agit mob (Treasure Chest, etc.)"""
    aegis_upper = aegis_name.upper()
    for pattern in WOE_PATTERNS:
        if pattern in aegis_upper:
            return True
    return False

def is_non_farmable_mob(aegis_name):
    """Check if mob is non-farmable (event, guild, WoE, etc.)"""
    return is_event_mob(aegis_name) or is_woe_mob(aegis_name)

def parse_safe_materials():
    """Parse the SAFE_MATERIALS.txt file"""
    items = {}
    with open(SAFE_MATERIALS, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split(',')
            if len(parts) >= 4:
                item_id = int(parts[0])
                items[item_id] = {
                    'id': item_id,
                    'aegis_name': parts[1],
                    'name': parts[2],
                    'weight': parts[3]
                }
    return items

def parse_item_db_for_trade():
    """Parse item_db to get trade restrictions"""
    print("Parsing item_db_etc.yml for trade restrictions...")
    trade_info = {}

    with open(ITEM_DB, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f.read())

    if data and 'Body' in data:
        for item in data['Body']:
            item_id = item.get('Id')
            if item_id is None:
                continue

            trade = item.get('Trade', {})
            if trade:
                trade_info[item_id] = {
                    'no_drop': trade.get('NoDrop', False),
                    'no_trade': trade.get('NoTrade', False),
                    'no_sell': trade.get('NoSell', False),
                    'no_cart': trade.get('NoCart', False),
                    'no_storage': trade.get('NoStorage', False),
                    'no_guild_storage': trade.get('NoGuildStorage', False),
                    'no_mail': trade.get('NoMail', False),
                    'no_auction': trade.get('NoAuction', False)
                }

    return trade_info

def parse_mob_db_detailed():
    """Parse mob_db and extract boss information with event mob detection"""

    print("Parsing mob_db.yml for boss classification...")

    boss_mobs = {}
    regular_mobs = {}
    event_mobs = {}

    # Track drops by item reference, separated by mob type
    regular_boss_drops = defaultdict(list)  # Farmable boss drops
    event_boss_drops = defaultdict(list)    # Event-only boss drops
    regular_mob_drops = defaultdict(list)   # Regular mob drops

    with open(MOB_DB, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f.read())

    if not data or 'Body' not in data:
        print("ERROR: Could not parse mob_db.yml")
        return {}, {}, {}, {}, {}, {}

    for mob in data['Body']:
        mob_id = mob.get('Id')
        if mob_id is None:
            continue

        mob_name = mob.get('Name', mob.get('AegisName', f'Mob_{mob_id}'))
        mob_aegis = mob.get('AegisName', '')
        mob_level = mob.get('Level', 0)

        # Determine if boss
        mob_class = mob.get('Class', '')
        is_boss = (mob_class == 'Boss')

        # Check MVP status
        modes = mob.get('Modes', {})
        is_mvp = modes.get('Mvp', False) if isinstance(modes, dict) else False
        if mob.get('MvpExp', 0) > 0:
            is_mvp = True

        # Check if event/guild/WoE mob (non-farmable)
        is_event = is_non_farmable_mob(mob_aegis)

        mob_info = {
            'id': mob_id,
            'name': mob_name,
            'aegis': mob_aegis,
            'level': mob_level,
            'is_mvp': is_mvp,
            'is_boss': is_boss,
            'is_event': is_event
        }

        if is_boss:
            if is_event:
                event_mobs[mob_id] = mob_info
            else:
                boss_mobs[mob_id] = mob_info
        else:
            regular_mobs[mob_id] = mob_info

        # Process regular drops
        for drop in mob.get('Drops', []):
            if isinstance(drop, dict):
                item_ref = drop.get('Item')
                rate = drop.get('Rate', 0)
                if item_ref and rate > 0:
                    drop_info = {
                        'mob_id': mob_id,
                        'mob_name': mob_name,
                        'mob_aegis': mob_aegis,
                        'mob_level': mob_level,
                        'is_mvp': is_mvp,
                        'is_boss': is_boss,
                        'is_event': is_event,
                        'drop_rate': rate,
                        'drop_type': 'normal'
                    }

                    if is_boss:
                        if is_event:
                            event_boss_drops[item_ref].append(drop_info)
                        else:
                            regular_boss_drops[item_ref].append(drop_info)
                    else:
                        regular_mob_drops[item_ref].append(drop_info)

        # Process MVP drops (always from bosses)
        for drop in mob.get('MvpDrops', []):
            if isinstance(drop, dict):
                item_ref = drop.get('Item')
                rate = drop.get('Rate', 0)
                if item_ref and rate > 0:
                    drop_info = {
                        'mob_id': mob_id,
                        'mob_name': mob_name,
                        'mob_aegis': mob_aegis,
                        'mob_level': mob_level,
                        'is_mvp': True,
                        'is_boss': True,
                        'is_event': is_event,
                        'drop_rate': rate,
                        'drop_type': 'mvp_drop'
                    }

                    if is_event:
                        event_boss_drops[item_ref].append(drop_info)
                    else:
                        regular_boss_drops[item_ref].append(drop_info)

    print(f"Regular boss mobs (farmable): {len(boss_mobs)}")
    print(f"Event/Guild boss mobs (NOT farmable): {len(event_mobs)}")
    print(f"Regular mobs: {len(regular_mobs)}")

    return boss_mobs, event_mobs, regular_mobs, regular_boss_drops, event_boss_drops, regular_mob_drops

def main():
    print("=" * 70)
    print("Generating Material Classification Files (WITH EVENT FILTERING)")
    print("=" * 70)
    print(f"Started: {datetime.now()}\n")

    # Load verified safe materials
    safe_items = parse_safe_materials()
    print(f"Loaded {len(safe_items)} verified items\n")

    # Get trade restrictions
    trade_info = parse_item_db_for_trade()

    # Get boss/event classification
    boss_mobs, event_mobs, regular_mobs, regular_boss_drops, event_boss_drops, regular_mob_drops = parse_mob_db_detailed()

    # Classify items
    non_tradable = []
    farmable_boss_drops = []      # Items from regular bosses (can be farmed)
    event_only_drops = []         # Items ONLY from event mobs (cannot be farmed normally)
    regular_only_drops = []       # Items ONLY from regular mobs

    for item_id, item in safe_items.items():
        aegis = item['aegis_name']

        # Check trade restrictions
        if item_id in trade_info:
            restrictions = trade_info[item_id]
            active_restrictions = [k for k, v in restrictions.items() if v]
            if active_restrictions:
                non_tradable.append({
                    'item': item,
                    'restrictions': active_restrictions
                })

        # Get mob sources for this item
        farmable_sources = []
        event_sources = []
        regular_sources = []

        for ref in [aegis, str(item_id)]:
            if ref in regular_boss_drops:
                farmable_sources.extend(regular_boss_drops[ref])
            if ref in event_boss_drops:
                event_sources.extend(event_boss_drops[ref])
            if ref in regular_mob_drops:
                regular_sources.extend(regular_mob_drops[ref])

        # Remove duplicates
        def dedupe(sources):
            seen = set()
            unique = []
            for s in sources:
                key = (s['mob_id'], s['drop_type'])
                if key not in seen:
                    seen.add(key)
                    unique.append(s)
            return unique

        farmable_sources = dedupe(farmable_sources)
        event_sources = dedupe(event_sources)
        regular_sources = dedupe(regular_sources)

        # Classify
        has_farmable_boss = len(farmable_sources) > 0
        has_event_only = len(event_sources) > 0 and not has_farmable_boss
        has_regular = len(regular_sources) > 0

        if has_farmable_boss:
            farmable_boss_drops.append({
                'item': item,
                'boss_sources': farmable_sources,
                'event_sources': event_sources,
                'regular_sources': regular_sources
            })
        elif has_event_only:
            event_only_drops.append({
                'item': item,
                'event_sources': event_sources,
                'regular_sources': regular_sources
            })

        if has_regular and not has_farmable_boss and not has_event_only:
            regular_only_drops.append({
                'item': item,
                'sources': regular_sources
            })

    print(f"\n{'='*60}")
    print("CLASSIFICATION RESULTS (WITH EVENT FILTERING)")
    print(f"{'='*60}")
    print(f"Non-tradable items: {len(non_tradable)}")
    print(f"Farmable Boss/MVP drops: {len(farmable_boss_drops)}")
    print(f"EVENT-ONLY drops (NOT farmable): {len(event_only_drops)}")
    print(f"Regular mob drops only: {len(regular_only_drops)}")
    print(f"{'='*60}\n")

    # Write NON_TRADABLE_ITEMS.txt
    with open(OUTPUT_DIR / "NON_TRADABLE_ITEMS.txt", 'w') as f:
        f.write("# ============================================================\n")
        f.write("# NON-TRADABLE VERIFIED MATERIALS\n")
        f.write("# These items have trade restrictions that may need updating\n")
        f.write("# ============================================================\n")
        f.write(f"# Generated: {datetime.now()}\n")
        f.write(f"# Total items: {len(non_tradable)}\n")
        f.write("#\n")
        f.write("# Format: ItemID,AegisName,ItemName,Restrictions\n")
        f.write("# ============================================================\n\n")

        for entry in sorted(non_tradable, key=lambda x: x['item']['id']):
            item = entry['item']
            restrictions = ', '.join(entry['restrictions'])
            f.write(f"{item['id']},{item['aegis_name']},{item['name']},{restrictions}\n")

    # Write BOSS_MVP_DROPS.txt (FARMABLE ONLY)
    with open(OUTPUT_DIR / "BOSS_MVP_DROPS.txt", 'w') as f:
        f.write("# ============================================================\n")
        f.write("# VERIFIED MATERIALS FROM BOSS/MVP MONSTERS (FARMABLE)\n")
        f.write("# Items that drop from regular Boss/MVP monsters\n")
        f.write("# ============================================================\n")
        f.write(f"# Generated: {datetime.now()}\n")
        f.write(f"# Total items: {len(farmable_boss_drops)}\n")
        f.write("#\n")
        f.write("# NOTE: Event-only drops have been EXCLUDED from this list\n")
        f.write("#       These items can actually be farmed in-game\n")
        f.write("#\n")
        f.write("# Format: ItemID,AegisName,ItemName,BossCount,BossList\n")
        f.write("# ============================================================\n\n")

        for entry in sorted(farmable_boss_drops, key=lambda x: x['item']['id']):
            item = entry['item']
            boss_sources = entry['boss_sources']
            boss_list = []
            for s in boss_sources[:5]:
                boss_type = "MVP" if s['is_mvp'] else "Mini-Boss"
                boss_list.append(f"{s['mob_name']}({boss_type})")
            bosses_str = "; ".join(boss_list)
            if len(boss_sources) > 5:
                bosses_str += f"; +{len(boss_sources)-5} more"
            f.write(f"{item['id']},{item['aegis_name']},{item['name']},{len(boss_sources)},{bosses_str}\n")

    # Write EVENT_ONLY_DROPS.txt (NEW FILE)
    with open(OUTPUT_DIR / "EVENT_ONLY_DROPS.txt", 'w') as f:
        f.write("# ============================================================\n")
        f.write("# EVENT-ONLY DROPS (NOT FARMABLE NORMALLY)\n")
        f.write("# Items that ONLY drop from Event/Guild/WoE mobs\n")
        f.write("# ============================================================\n")
        f.write(f"# Generated: {datetime.now()}\n")
        f.write(f"# Total items: {len(event_only_drops)}\n")
        f.write("#\n")
        f.write("# WARNING: These items CANNOT be farmed normally!\n")
        f.write("# They only drop from event/guild mobs that don't spawn in the\n")
        f.write("# regular game world. DO NOT use these in quests unless you\n")
        f.write("# have custom spawns for these mobs.\n")
        f.write("#\n")
        f.write("# Event mob prefixes: EVENT_, G_, E_\n")
        f.write("#\n")
        f.write("# Format: ItemID,AegisName,ItemName,EventMobCount,EventMobList\n")
        f.write("# ============================================================\n\n")

        for entry in sorted(event_only_drops, key=lambda x: x['item']['id']):
            item = entry['item']
            event_sources = entry['event_sources']
            mob_list = []
            for s in event_sources[:5]:
                mob_list.append(f"{s['mob_name']}({s['mob_aegis']})")
            mobs_str = "; ".join(mob_list)
            if len(event_sources) > 5:
                mobs_str += f"; +{len(event_sources)-5} more"
            f.write(f"{item['id']},{item['aegis_name']},{item['name']},{len(event_sources)},{mobs_str}\n")

    # Write REGULAR_MOB_DROPS.txt (EXCLUSIVE - no boss drops)
    with open(OUTPUT_DIR / "REGULAR_MOB_DROPS.txt", 'w') as f:
        f.write("# ============================================================\n")
        f.write("# VERIFIED MATERIALS FROM REGULAR MONSTERS ONLY\n")
        f.write("# Items that ONLY drop from regular mobs (NO boss drops)\n")
        f.write("# ============================================================\n")
        f.write(f"# Generated: {datetime.now()}\n")
        f.write(f"# Total items: {len(regular_only_drops)}\n")
        f.write("#\n")
        f.write("# These items do NOT drop from any MVP, Mini-Boss, or Event mob\n")
        f.write("# Safe for use without affecting boss hunting rewards\n")
        f.write("#\n")
        f.write("# Format: ItemID,AegisName,ItemName,MobCount\n")
        f.write("# ============================================================\n\n")

        for entry in sorted(regular_only_drops, key=lambda x: x['item']['id']):
            item = entry['item']
            f.write(f"{item['id']},{item['aegis_name']},{item['name']},{len(entry['sources'])}\n")

    # Write BOSS_DROP_DETAILS.csv (farmable only)
    with open(OUTPUT_DIR / "BOSS_DROP_DETAILS.csv", 'w') as f:
        f.write("ItemID,ItemName,AegisName,MobID,MobName,MobAegis,BossType,DropRate,MobLevel,DropType,IsEvent\n")
        for entry in sorted(farmable_boss_drops, key=lambda x: x['item']['id']):
            item = entry['item']
            for s in entry['boss_sources']:
                boss_type = "MVP" if s['is_mvp'] else "Mini-Boss"
                f.write(f"{item['id']},{item['name'].replace(',', ' ')},{item['aegis_name']},")
                f.write(f"{s['mob_id']},{s['mob_name'].replace(',', ' ')},{s['mob_aegis']},{boss_type},")
                f.write(f"{s['drop_rate']},{s['mob_level']},{s['drop_type']},No\n")

    print(f"Output files written to: {OUTPUT_DIR}")
    print("\nFiles created/updated:")
    print("  - NON_TRADABLE_ITEMS.txt")
    print("  - BOSS_MVP_DROPS.txt (farmable only, event mobs excluded)")
    print("  - EVENT_ONLY_DROPS.txt (NEW - items that can't be farmed)")
    print("  - REGULAR_MOB_DROPS.txt")
    print("  - BOSS_DROP_DETAILS.csv")

    # Summary
    print(f"\n{'='*60}")
    print("SUMMARY")
    print(f"{'='*60}")
    total_safe = len(farmable_boss_drops) + len(regular_only_drops)
    total_unsafe = len(event_only_drops)
    print(f"SAFE TO USE (farmable): {total_safe} items")
    print(f"  - From regular bosses: {len(farmable_boss_drops)}")
    print(f"  - From regular mobs: {len(regular_only_drops)}")
    print(f"")
    print(f"DO NOT USE (event-only): {total_unsafe} items")
    print(f"  - These items can't be farmed normally!")
    print(f"{'='*60}")

if __name__ == "__main__":
    main()
