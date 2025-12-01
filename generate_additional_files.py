#!/usr/bin/env python3
"""
Generate additional files from verified materials:
1. NON_TRADABLE_ITEMS.txt - Items with trade restrictions
2. BOSS_MVP_DROPS.txt - Materials from Boss/MVP mobs
3. REGULAR_MOB_DROPS.txt - Materials from regular mobs only
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

def parse_mob_db_for_bosses():
    """Parse mob_db to identify Boss/MVP mobs and their drops"""
    print("Parsing mob_db.yml for boss classification...")

    mob_info = {}  # mob_id -> {name, is_mvp, is_boss, drops}
    item_mob_sources = defaultdict(list)  # item_ref -> [{mob_id, mob_name, is_mvp, is_boss, rate}]

    with open(MOB_DB, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f.read())

    if data and 'Body' in data:
        for mob in data['Body']:
            mob_id = mob.get('Id')
            if mob_id is None:
                continue

            mob_name = mob.get('Name', mob.get('AegisName', ''))
            mob_aegis = mob.get('AegisName', '')
            mob_level = mob.get('Level', 0)

            # Check boss classification
            mob_class = mob.get('Class', '')
            is_boss = (mob_class == 'Boss')

            # Check MVP status - in Modes
            modes = mob.get('Modes', {})
            is_mvp = modes.get('Mvp', False) if isinstance(modes, dict) else False

            # Also check MvpExp as indicator
            if mob.get('MvpExp', 0) > 0:
                is_mvp = True

            mob_info[mob_id] = {
                'id': mob_id,
                'name': mob_name,
                'aegis': mob_aegis,
                'level': mob_level,
                'is_mvp': is_mvp,
                'is_boss': is_boss
            }

            # Process regular drops
            for drop in mob.get('Drops', []):
                if isinstance(drop, dict):
                    item_ref = drop.get('Item')
                    rate = drop.get('Rate', 0)
                    if item_ref and rate > 0:
                        item_mob_sources[item_ref].append({
                            'mob_id': mob_id,
                            'mob_name': mob_name,
                            'mob_aegis': mob_aegis,
                            'mob_level': mob_level,
                            'is_mvp': is_mvp,
                            'is_boss': is_boss,
                            'drop_rate': rate,
                            'drop_type': 'normal'
                        })

            # Process MVP drops
            for drop in mob.get('MvpDrops', []):
                if isinstance(drop, dict):
                    item_ref = drop.get('Item')
                    rate = drop.get('Rate', 0)
                    if item_ref and rate > 0:
                        item_mob_sources[item_ref].append({
                            'mob_id': mob_id,
                            'mob_name': mob_name,
                            'mob_aegis': mob_aegis,
                            'mob_level': mob_level,
                            'is_mvp': True,
                            'is_boss': True,
                            'drop_rate': rate,
                            'drop_type': 'mvp_drop'
                        })

    print(f"Found {len([m for m in mob_info.values() if m['is_mvp']])} MVPs")
    print(f"Found {len([m for m in mob_info.values() if m['is_boss'] and not m['is_mvp']])} Mini-Bosses")

    return mob_info, item_mob_sources

def main():
    print("=" * 70)
    print("Generating Additional Classification Files")
    print("=" * 70)
    print(f"Started: {datetime.now()}\n")

    # Load verified safe materials
    safe_items = parse_safe_materials()
    print(f"Loaded {len(safe_items)} verified items\n")

    # Get trade restrictions
    trade_info = parse_item_db_for_trade()

    # Get boss/MVP classification
    mob_info, item_mob_sources = parse_mob_db_for_bosses()

    # Classify items
    non_tradable = []
    boss_drops = []  # Items that drop from ANY boss/mvp
    regular_only_drops = []  # Items that ONLY drop from regular mobs

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
        sources = []
        for ref in [aegis, str(item_id)]:
            if ref in item_mob_sources:
                sources.extend(item_mob_sources[ref])

        # Remove duplicates
        seen = set()
        unique_sources = []
        for s in sources:
            key = (s['mob_id'], s['drop_type'])
            if key not in seen:
                seen.add(key)
                unique_sources.append(s)

        # Classify based on sources
        has_boss_source = any(s['is_boss'] or s['is_mvp'] for s in unique_sources)
        has_regular_source = any(not s['is_boss'] and not s['is_mvp'] for s in unique_sources)

        boss_sources = [s for s in unique_sources if s['is_boss'] or s['is_mvp']]
        regular_sources = [s for s in unique_sources if not s['is_boss'] and not s['is_mvp']]

        if has_boss_source:
            boss_drops.append({
                'item': item,
                'boss_sources': boss_sources,
                'regular_sources': regular_sources
            })

        if has_regular_source and not has_boss_source:
            regular_only_drops.append({
                'item': item,
                'sources': regular_sources
            })

    # Items that ONLY drop from regular mobs (no boss sources at all)
    regular_exclusive = [e for e in regular_only_drops]  # These are already exclusive

    # Items that have BOTH boss and regular sources (mixed)
    mixed_drops = []
    for entry in boss_drops:
        if entry['regular_sources']:
            mixed_drops.append({
                'item': entry['item'],
                'boss_sources': entry['boss_sources'],
                'regular_sources': entry['regular_sources']
            })

    print(f"\n{'='*60}")
    print("CLASSIFICATION RESULTS")
    print(f"{'='*60}")
    print(f"Non-tradable items: {len(non_tradable)}")
    print(f"Items with Boss/MVP drops: {len(boss_drops)}")
    print(f"Items ONLY from regular mobs: {len(regular_exclusive)}")
    print(f"Items from BOTH (mixed): {len(mixed_drops)}")
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

    # Write BOSS_MVP_DROPS.txt
    with open(OUTPUT_DIR / "BOSS_MVP_DROPS.txt", 'w') as f:
        f.write("# ============================================================\n")
        f.write("# VERIFIED MATERIALS FROM BOSS/MVP MONSTERS\n")
        f.write("# Items that drop from MVP or Mini-Boss monsters\n")
        f.write("# ============================================================\n")
        f.write(f"# Generated: {datetime.now()}\n")
        f.write(f"# Total items: {len(boss_drops)}\n")
        f.write("#\n")
        f.write("# Boss Types:\n")
        f.write("#   MVP = Major boss with MVP rewards\n")
        f.write("#   Mini-Boss = Boss-type but not MVP\n")
        f.write("#\n")
        f.write("# Format: ItemID,AegisName,ItemName,BossCount,BossList\n")
        f.write("# ============================================================\n\n")

        for entry in sorted(boss_drops, key=lambda x: x['item']['id']):
            item = entry['item']
            boss_sources = entry['boss_sources']
            boss_list = []
            for s in boss_sources[:5]:  # Limit to 5 bosses
                boss_type = "MVP" if s['is_mvp'] else "Mini-Boss"
                boss_list.append(f"{s['mob_name']}({boss_type})")
            bosses_str = "; ".join(boss_list)
            if len(boss_sources) > 5:
                bosses_str += f"; +{len(boss_sources)-5} more"
            f.write(f"{item['id']},{item['aegis_name']},{item['name']},{len(boss_sources)},{bosses_str}\n")

    # Write REGULAR_MOB_DROPS.txt (EXCLUSIVE - no boss drops)
    with open(OUTPUT_DIR / "REGULAR_MOB_DROPS.txt", 'w') as f:
        f.write("# ============================================================\n")
        f.write("# VERIFIED MATERIALS FROM REGULAR MONSTERS ONLY\n")
        f.write("# Items that ONLY drop from regular mobs (NO boss drops)\n")
        f.write("# ============================================================\n")
        f.write(f"# Generated: {datetime.now()}\n")
        f.write(f"# Total items: {len(regular_exclusive)}\n")
        f.write("#\n")
        f.write("# These items do NOT drop from any MVP or Mini-Boss\n")
        f.write("# Safe for use without affecting boss hunting rewards\n")
        f.write("#\n")
        f.write("# Format: ItemID,AegisName,ItemName,MobCount\n")
        f.write("# ============================================================\n\n")

        for entry in sorted(regular_exclusive, key=lambda x: x['item']['id']):
            item = entry['item']
            f.write(f"{item['id']},{item['aegis_name']},{item['name']},{len(entry['sources'])}\n")

    # Write detailed BOSS_DROP_DETAILS.csv
    with open(OUTPUT_DIR / "BOSS_DROP_DETAILS.csv", 'w') as f:
        f.write("ItemID,ItemName,AegisName,MobID,MobName,BossType,DropRate,MobLevel,DropType\n")
        for entry in sorted(boss_drops, key=lambda x: x['item']['id']):
            item = entry['item']
            for s in entry['boss_sources']:
                boss_type = "MVP" if s['is_mvp'] else "Mini-Boss"
                f.write(f"{item['id']},{item['name'].replace(',', ' ')},{item['aegis_name']},")
                f.write(f"{s['mob_id']},{s['mob_name'].replace(',', ' ')},{boss_type},")
                f.write(f"{s['drop_rate']},{s['mob_level']},{s['drop_type']}\n")

    print(f"Output files written to: {OUTPUT_DIR}")
    print("\nFiles created:")
    print("  - NON_TRADABLE_ITEMS.txt")
    print("  - BOSS_MVP_DROPS.txt")
    print("  - REGULAR_MOB_DROPS.txt")
    print("  - BOSS_DROP_DETAILS.csv")

if __name__ == "__main__":
    main()
