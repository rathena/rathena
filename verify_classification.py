#!/usr/bin/env python3
"""
Ultra-deep verification of material classification.
Cross-check all data directly against source YAML files.
"""

import yaml
from pathlib import Path
from collections import defaultdict

BASE_DIR = Path("/home/user/rathena2")
MOB_DB = BASE_DIR / "db/pre-re/mob_db.yml"
ITEM_DB = BASE_DIR / "db/pre-re/item_db_etc.yml"
OUTPUT_DIR = BASE_DIR / "Verified_Material_Loots"

def load_safe_items():
    """Load the verified safe items from SAFE_MATERIALS.txt"""
    items = {}
    with open(OUTPUT_DIR / "SAFE_MATERIALS.txt", 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split(',')
            if len(parts) >= 4:
                item_id = int(parts[0])
                items[item_id] = {
                    'id': item_id,
                    'aegis': parts[1],
                    'name': parts[2]
                }
    return items

def load_regular_only():
    """Load items from REGULAR_MOB_DROPS.txt"""
    items = set()
    with open(OUTPUT_DIR / "REGULAR_MOB_DROPS.txt", 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split(',')
            if parts:
                items.add(int(parts[0]))
    return items

def load_boss_drops():
    """Load items from BOSS_MVP_DROPS.txt"""
    items = set()
    with open(OUTPUT_DIR / "BOSS_MVP_DROPS.txt", 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split(',')
            if parts:
                items.add(int(parts[0]))
    return items

def parse_item_db():
    """Parse item_db to get AegisName -> ID mapping"""
    aegis_to_id = {}
    id_to_aegis = {}

    with open(ITEM_DB, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f.read())

    if data and 'Body' in data:
        for item in data['Body']:
            item_id = item.get('Id')
            aegis = item.get('AegisName')
            if item_id and aegis:
                aegis_to_id[aegis] = item_id
                id_to_aegis[item_id] = aegis

    return aegis_to_id, id_to_aegis

def parse_mob_db_detailed():
    """Parse mob_db and extract ALL boss information with their drops"""

    print("=" * 70)
    print("PARSING MOB DATABASE")
    print("=" * 70)

    boss_mobs = {}  # mob_id -> {name, is_mvp, drops}
    regular_mobs = {}

    # Track all drops by item reference
    boss_item_drops = defaultdict(list)  # item_ref -> [(mob_id, mob_name, is_mvp)]
    regular_item_drops = defaultdict(list)

    with open(MOB_DB, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f.read())

    if not data or 'Body' not in data:
        print("ERROR: Could not parse mob_db.yml")
        return {}, {}, {}, {}

    for mob in data['Body']:
        mob_id = mob.get('Id')
        if mob_id is None:
            continue

        mob_name = mob.get('Name', mob.get('AegisName', f'Mob_{mob_id}'))
        mob_aegis = mob.get('AegisName', '')

        # Determine if boss
        mob_class = mob.get('Class', '')
        is_boss = (mob_class == 'Boss')

        # Check MVP status
        modes = mob.get('Modes', {})
        is_mvp = modes.get('Mvp', False) if isinstance(modes, dict) else False

        # MvpExp also indicates MVP
        if mob.get('MvpExp', 0) > 0:
            is_mvp = True

        # Collect drops
        drops = []
        for drop in mob.get('Drops', []):
            if isinstance(drop, dict):
                item_ref = drop.get('Item')
                rate = drop.get('Rate', 0)
                if item_ref and rate > 0:
                    drops.append({'item': item_ref, 'rate': rate, 'type': 'normal'})

                    if is_boss:
                        boss_item_drops[item_ref].append((mob_id, mob_name, is_mvp, 'normal'))
                    else:
                        regular_item_drops[item_ref].append((mob_id, mob_name, 'normal'))

        # MVP drops
        for drop in mob.get('MvpDrops', []):
            if isinstance(drop, dict):
                item_ref = drop.get('Item')
                rate = drop.get('Rate', 0)
                if item_ref and rate > 0:
                    drops.append({'item': item_ref, 'rate': rate, 'type': 'mvp'})
                    boss_item_drops[item_ref].append((mob_id, mob_name, True, 'mvp'))

        if is_boss:
            boss_mobs[mob_id] = {
                'id': mob_id,
                'name': mob_name,
                'aegis': mob_aegis,
                'is_mvp': is_mvp,
                'drops': drops
            }
        else:
            regular_mobs[mob_id] = {
                'id': mob_id,
                'name': mob_name,
                'aegis': mob_aegis,
                'drops': drops
            }

    print(f"Total mobs parsed: {len(boss_mobs) + len(regular_mobs)}")
    print(f"Boss mobs (Class: Boss): {len(boss_mobs)}")
    print(f"  - MVPs: {len([m for m in boss_mobs.values() if m['is_mvp']])}")
    print(f"  - Mini-Bosses: {len([m for m in boss_mobs.values() if not m['is_mvp']])}")
    print(f"Regular mobs: {len(regular_mobs)}")
    print(f"Unique items dropped by bosses: {len(boss_item_drops)}")
    print(f"Unique items dropped by regular mobs: {len(regular_item_drops)}")

    return boss_mobs, regular_mobs, boss_item_drops, regular_item_drops

def verify_classification():
    """Main verification function"""

    print("\n" + "=" * 70)
    print("ULTRA-DEEP VERIFICATION OF MATERIAL CLASSIFICATION")
    print("=" * 70 + "\n")

    # Load all data
    safe_items = load_safe_items()
    regular_only = load_regular_only()
    boss_drops_list = load_boss_drops()
    aegis_to_id, id_to_aegis = parse_item_db()

    print(f"Verified safe items: {len(safe_items)}")
    print(f"Items in REGULAR_MOB_DROPS.txt: {len(regular_only)}")
    print(f"Items in BOSS_MVP_DROPS.txt: {len(boss_drops_list)}")
    print(f"Sum: {len(regular_only) + len(boss_drops_list)}")
    print()

    # Parse mob database
    boss_mobs, regular_mobs, boss_item_drops, regular_item_drops = parse_mob_db_detailed()

    # VERIFICATION 1: Check items in REGULAR_MOB_DROPS don't drop from any boss
    print("\n" + "=" * 70)
    print("VERIFICATION 1: Regular-only items should NOT drop from bosses")
    print("=" * 70)

    errors_regular = []
    for item_id in regular_only:
        aegis = id_to_aegis.get(item_id, '')

        # Check both ID and aegis name against boss drops
        found_in_boss = []
        for ref in [aegis, str(item_id)]:
            if ref in boss_item_drops:
                for mob_id, mob_name, is_mvp, drop_type in boss_item_drops[ref]:
                    boss_type = "MVP" if is_mvp else "Mini-Boss"
                    found_in_boss.append(f"{mob_name}({boss_type})")

        if found_in_boss:
            errors_regular.append({
                'id': item_id,
                'aegis': aegis,
                'name': safe_items.get(item_id, {}).get('name', ''),
                'bosses': found_in_boss
            })

    if errors_regular:
        print(f"\n❌ ERRORS FOUND: {len(errors_regular)} items in REGULAR_MOB_DROPS.txt also drop from bosses!")
        for err in errors_regular:
            print(f"  Item {err['id']} ({err['aegis']}) - {err['name']}")
            print(f"    Drops from: {', '.join(err['bosses'][:5])}")
            if len(err['bosses']) > 5:
                print(f"    ... and {len(err['bosses'])-5} more bosses")
    else:
        print("\n✓ All items in REGULAR_MOB_DROPS.txt verified - none drop from bosses")

    # VERIFICATION 2: Check items in BOSS_MVP_DROPS actually drop from bosses
    print("\n" + "=" * 70)
    print("VERIFICATION 2: Boss items should actually drop from bosses")
    print("=" * 70)

    errors_boss = []
    for item_id in boss_drops_list:
        aegis = id_to_aegis.get(item_id, '')

        # Check if this item drops from any boss
        found_in_boss = False
        for ref in [aegis, str(item_id)]:
            if ref in boss_item_drops:
                found_in_boss = True
                break

        if not found_in_boss:
            errors_boss.append({
                'id': item_id,
                'aegis': aegis,
                'name': safe_items.get(item_id, {}).get('name', '')
            })

    if errors_boss:
        print(f"\n❌ ERRORS FOUND: {len(errors_boss)} items in BOSS_MVP_DROPS.txt don't drop from any boss!")
        for err in errors_boss:
            print(f"  Item {err['id']} ({err['aegis']}) - {err['name']}")
    else:
        print("\n✓ All items in BOSS_MVP_DROPS.txt verified - all drop from bosses")

    # VERIFICATION 3: Check for overlap (should be none)
    print("\n" + "=" * 70)
    print("VERIFICATION 3: No overlap between lists")
    print("=" * 70)

    overlap = regular_only.intersection(boss_drops_list)
    if overlap:
        print(f"\n❌ ERROR: {len(overlap)} items appear in BOTH lists!")
        for item_id in overlap:
            aegis = id_to_aegis.get(item_id, '')
            print(f"  Item {item_id} ({aegis})")
    else:
        print("\n✓ No overlap between REGULAR_MOB_DROPS and BOSS_MVP_DROPS")

    # VERIFICATION 4: Check all safe items are accounted for
    print("\n" + "=" * 70)
    print("VERIFICATION 4: All safe items accounted for")
    print("=" * 70)

    all_classified = regular_only.union(boss_drops_list)
    missing = set(safe_items.keys()) - all_classified
    extra = all_classified - set(safe_items.keys())

    if missing:
        print(f"\n❌ ERROR: {len(missing)} safe items not in either list!")
        for item_id in missing:
            aegis = id_to_aegis.get(item_id, '')
            print(f"  Item {item_id} ({aegis})")
    else:
        print("\n✓ All safe items accounted for")

    if extra:
        print(f"\n❌ ERROR: {len(extra)} items in classification files but not in safe items!")
        for item_id in extra:
            print(f"  Item {item_id}")

    # VERIFICATION 5: Sample spot-checks
    print("\n" + "=" * 70)
    print("VERIFICATION 5: Sample spot-checks")
    print("=" * 70)

    # Check some specific items manually
    spot_checks = [
        (920, "Wolf_Claw", "Should be BOSS (Vagabond Wolf is Mini-Boss)"),
        (967, "Turtle_Shell", "Should be BOSS (Turtle General is MVP) but also regular"),
        (7054, "Brigan", "Should be BOSS (drops from many bosses)"),
        (904, "Scorpion's_Tail", "Should be REGULAR only"),
        (7317, "Screw", "Should be REGULAR only"),
    ]

    for item_id, aegis, expected in spot_checks:
        in_regular = item_id in regular_only
        in_boss = item_id in boss_drops_list

        boss_sources = []
        regular_sources = []

        for ref in [aegis, str(item_id)]:
            if ref in boss_item_drops:
                for mob_id, mob_name, is_mvp, drop_type in boss_item_drops[ref]:
                    boss_type = "MVP" if is_mvp else "Mini-Boss"
                    boss_sources.append(f"{mob_name}({boss_type})")
            if ref in regular_item_drops:
                for mob_id, mob_name, drop_type in regular_item_drops[ref]:
                    regular_sources.append(mob_name)

        print(f"\nItem {item_id} ({aegis}):")
        print(f"  Expected: {expected}")
        print(f"  In REGULAR list: {in_regular}")
        print(f"  In BOSS list: {in_boss}")
        print(f"  Boss sources: {len(boss_sources)} - {', '.join(boss_sources[:3])}")
        print(f"  Regular sources: {len(regular_sources)} - {', '.join(regular_sources[:3])}")

        if boss_sources and in_regular:
            print(f"  ❌ ERROR: Has boss sources but is in REGULAR list!")
        elif not boss_sources and in_boss:
            print(f"  ❌ ERROR: No boss sources but is in BOSS list!")
        else:
            print(f"  ✓ Classification correct")

    # Return error counts
    total_errors = len(errors_regular) + len(errors_boss) + len(overlap) + len(missing) + len(extra)

    print("\n" + "=" * 70)
    print("VERIFICATION SUMMARY")
    print("=" * 70)
    print(f"Regular items incorrectly having boss drops: {len(errors_regular)}")
    print(f"Boss items not dropping from any boss: {len(errors_boss)}")
    print(f"Items in both lists: {len(overlap)}")
    print(f"Missing from classification: {len(missing)}")
    print(f"Extra items in classification: {len(extra)}")
    print(f"\nTOTAL ERRORS: {total_errors}")

    if total_errors == 0:
        print("\n✓ ✓ ✓ ALL VERIFICATIONS PASSED ✓ ✓ ✓")
    else:
        print(f"\n❌ ❌ ❌ {total_errors} ERRORS NEED FIXING ❌ ❌ ❌")

    return errors_regular, errors_boss

if __name__ == "__main__":
    errors_regular, errors_boss = verify_classification()
