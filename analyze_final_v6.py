#!/usr/bin/env python3
"""
FINAL CORRECTED Mob Drop Materials Analyzer v6.0
Fixed quest name pattern to work with underscores in AegisNames
"""

import os
import re
import yaml
from collections import defaultdict
from pathlib import Path
from datetime import datetime

BASE_DIR = Path("/home/user/rathena2")
ITEM_DB = BASE_DIR / "db/pre-re/item_db_etc.yml"
MOB_DB = BASE_DIR / "db/pre-re/mob_db.yml"
OUTPUT_DIR = BASE_DIR / "Verified_Material_Loots"

ITEM_RANGES = [(900, 1100), (7000, 7999)]

# Quest name patterns - use lookahead/lookbehind that work with underscores
# Match keyword as standalone word OR after/before underscores
QUEST_KEYWORDS = ['key', 'token', 'letter', 'permit', 'certificate',
                  'pass', 'ticket', 'voucher', 'evidence', 'proof', 'badge']

EXCLUDE_PATHS = ['doc/', 'test/', 'sample/', '.git/', '3rdparty/', 'src/', 'tools/']

def should_skip_file(filepath):
    fp_lower = filepath.lower()
    for exclude in EXCLUDE_PATHS:
        if exclude.lower() in fp_lower:
            return True
    return False

def check_quest_name(item):
    """Check if item name contains quest-related keywords"""
    name = item.get('name', '').lower()
    aegis = item.get('aegis_name', '').lower()

    for keyword in QUEST_KEYWORDS:
        # For Name field - use word boundary
        if re.search(r'\b' + keyword + r'\b', name):
            return True, f"Name contains '{keyword}'"

        # For AegisName - also check with underscores
        # Match: _keyword_, _keyword, keyword_, or standalone keyword
        aegis_pattern = r'(?:^|_)' + keyword + r'(?:_|$)'
        if re.search(aegis_pattern, aegis) or re.search(r'\b' + keyword + r'\b', aegis):
            return True, f"AegisName contains '{keyword}'"

    return False, None

def parse_item_db():
    print("Parsing item_db_etc.yml...")
    items = {}
    with open(ITEM_DB, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f.read())

    if data and 'Body' in data:
        for item in data['Body']:
            item_id = item.get('Id')
            if item_id is None:
                continue
            if not any(start <= item_id <= end for start, end in ITEM_RANGES):
                continue

            trade = item.get('Trade', {})
            items[item_id] = {
                'id': item_id,
                'aegis_name': item.get('AegisName', ''),
                'name': item.get('Name', ''),
                'weight': item.get('Weight', 0),
                'no_drop': trade.get('NoDrop', False) if trade else False,
                'no_trade': trade.get('NoTrade', False) if trade else False
            }
    print(f"Found {len(items)} items")
    return items

def parse_mob_db():
    print("Parsing mob_db.yml...")
    mobs = {}
    item_drops = defaultdict(list)

    with open(MOB_DB, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f.read())

    if data and 'Body' in data:
        for mob in data['Body']:
            mob_id = mob.get('Id')
            if mob_id is None:
                continue

            mob_info = {
                'id': mob_id,
                'name': mob.get('Name', mob.get('AegisName', '')),
                'aegis_name': mob.get('AegisName', ''),
                'level': mob.get('Level', 0)
            }
            mobs[mob_id] = mob_info
            mobs[mob_info['aegis_name']] = mob_info

            for drop in mob.get('Drops', []):
                if isinstance(drop, dict):
                    item_ref = drop.get('Item')
                    rate = drop.get('Rate', 0)
                    if item_ref and rate > 0:
                        item_drops[item_ref].append({
                            'mob_id': mob_id,
                            'mob_name': mob_info['name'],
                            'mob_aegis': mob_info['aegis_name'],
                            'drop_rate': rate,
                            'mob_level': mob_info['level']
                        })

    print(f"Found {len([m for m in mobs if isinstance(m, int)])} mobs")
    return mobs, item_drops

def scan_for_giving_patterns(items):
    """Scan for patterns where items are GIVEN to players"""
    print("\nScanning for GIVING patterns...")

    giving_patterns = {
        'getitem': re.compile(r'\bgetitem[23]?\s*[\(\s]*([\w_]+|\d+)', re.IGNORECASE),
        'makeitem': re.compile(r'\bmakeitem[23]?\s*[\(\s]*([\w_]+|\d+)', re.IGNORECASE),
        'rentitem': re.compile(r'\brentitem[23]?\s*[\(\s]*([\w_]+|\d+)', re.IGNORECASE),
        'getnameditem': re.compile(r'\bgetnameditem\s*[\(\s]*([\w_]+|\d+)', re.IGNORECASE),
        'sellitem': re.compile(r'\bsellitem\s*[\(\s]*([\w_]+|\d+)', re.IGNORECASE),
    }

    findings = defaultdict(list)
    files_scanned = []

    npc_dir = BASE_DIR / "npc"

    for root, dirs, files in os.walk(npc_dir):
        dirs[:] = [d for d in dirs if d.lower() != 'test']

        for filename in files:
            if not filename.endswith('.txt'):
                continue

            filepath = os.path.join(root, filename)
            rel_path = os.path.relpath(filepath, BASE_DIR)

            if should_skip_file(rel_path):
                continue

            files_scanned.append(rel_path)

            try:
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                    lines = content.split('\n')

                    in_shop = False

                    for line_num, line in enumerate(lines, 1):
                        stripped = line.strip()

                        if stripped.startswith('//') or stripped.startswith('#'):
                            continue

                        if re.search(r'\b(shop|cashshop|pointshop)\b', stripped, re.IGNORECASE):
                            in_shop = True

                        for pattern_name, pattern in giving_patterns.items():
                            matches = pattern.findall(line)
                            for match in matches:
                                item_ref = str(match).strip()
                                if item_ref and item_ref not in ['', '0', 'var', 'getarg']:
                                    findings[item_ref].append({
                                        'pattern': pattern_name,
                                        'file': rel_path,
                                        'line_num': line_num,
                                        'line': stripped[:100]
                                    })

                        if in_shop and ':' in stripped:
                            shop_items = re.findall(r'(?:^|,)\s*(\d+)\s*:\s*-?\d+', stripped)
                            for item_id in shop_items:
                                if item_id and int(item_id) > 500:
                                    findings[item_id].append({
                                        'pattern': 'shop_item',
                                        'file': rel_path,
                                        'line_num': line_num,
                                        'line': stripped[:100]
                                    })

                        if 'setarray' in stripped.lower():
                            var_match = re.search(r'setarray\s+([.\@\w]+)', stripped, re.IGNORECASE)
                            if var_match:
                                var_name = var_match.group(1).replace('[0]', '').replace('[', '').replace(']', '')
                                if re.search(rf'getitem.*{re.escape(var_name)}', content, re.IGNORECASE):
                                    nums = re.findall(r'\b(\d{3,4})\b', stripped)
                                    for num in nums:
                                        if 900 <= int(num) <= 1100 or 7000 <= int(num) <= 7999:
                                            findings[num].append({
                                                'pattern': 'setarray_getitem',
                                                'file': rel_path,
                                                'line_num': line_num,
                                                'line': stripped[:100]
                                            })

            except Exception as e:
                print(f"Error: {filepath}: {e}")

    print(f"Scanned {len(files_scanned)} files")
    return findings, files_scanned

def main():
    print("=" * 70)
    print("FINAL CORRECTED Mob Drop Materials Analyzer v6.0")
    print("Fixed quest name patterns for underscore handling")
    print("=" * 70)
    print(f"Started: {datetime.now()}")
    print()

    OUTPUT_DIR.mkdir(exist_ok=True)

    items = parse_item_db()
    mobs, item_drops = parse_mob_db()

    findings, files_scanned = scan_for_giving_patterns(items)

    safe_items = []
    excluded_items = []

    print("\nAnalyzing items...")

    for item_id, item in sorted(items.items()):
        exclusion_reasons = []
        aegis_name = item['aegis_name']

        # 1. Quest name check (FIXED for underscores)
        is_quest, pattern = check_quest_name(item)
        if is_quest:
            exclusion_reasons.append(f"Quest-named: {pattern}")

        # 2. Check mob drops
        item_drop_info = []
        for ref in [aegis_name, str(item_id)]:
            if ref in item_drops:
                for drop in item_drops[ref]:
                    if drop not in item_drop_info:
                        if drop['mob_id'] in mobs or drop['mob_aegis'] in mobs:
                            item_drop_info.append(drop)

        if not item_drop_info:
            exclusion_reasons.append("No mob drops in mob_db.yml")

        # 3. Check if item is GIVEN by NPCs
        for ref in [aegis_name, str(item_id)]:
            if ref in findings:
                seen_files = set()
                for finding in findings[ref]:
                    f = finding['file']
                    if f not in seen_files:
                        seen_files.add(f)
                        exclusion_reasons.append(f"GIVEN: {finding['pattern']} in {f}:{finding['line_num']}")
                        if len(exclusion_reasons) > 5:
                            break

        if exclusion_reasons:
            excluded_items.append({'item': item, 'reasons': exclusion_reasons})
        else:
            safe_items.append({'item': item, 'drops': item_drop_info})

    print(f"\n{'='*60}")
    print("FINAL RESULTS")
    print(f"{'='*60}")
    print(f"Total items scanned: {len(items)}")
    print(f"Items PASSED: {len(safe_items)}")
    print(f"Items EXCLUDED: {len(excluded_items)}")
    print(f"Pass Rate: {len(safe_items)/len(items)*100:.1f}%")
    print(f"{'='*60}\n")

    # Verify: Check if 7486 is excluded
    excluded_ids = [e['item']['id'] for e in excluded_items]
    if 7486 in excluded_ids:
        print("✓ Item 7486 (3rd_Floor_Pass) correctly EXCLUDED")
    else:
        print("✗ WARNING: Item 7486 (3rd_Floor_Pass) NOT excluded!")

    write_outputs(safe_items, excluded_items, items, files_scanned)

    return safe_items, excluded_items

def write_outputs(safe_items, excluded_items, items, files_scanned):
    # SAFE_MATERIALS.txt
    with open(OUTPUT_DIR / "SAFE_MATERIALS.txt", 'w') as f:
        f.write("# ============================================================\n")
        f.write("# VERIFIED PURE MOB DROP MATERIALS DATABASE\n")
        f.write("# Pre-Renewal rAthena\n")
        f.write("# ============================================================\n")
        f.write(f"# Generated: {datetime.now()}\n")
        f.write(f"# Total verified items: {len(safe_items)}\n")
        f.write("#\n")
        f.write("# INCLUSION CRITERIA:\n")
        f.write("#   - Has mob drops in db/pre-re/mob_db.yml\n")
        f.write("#   - NOT given by any NPC (getitem, shops, rewards)\n")
        f.write("#   - No quest-like naming (key, token, pass, ticket, etc.)\n")
        f.write("#\n")
        f.write("# NOTE: Items with delitem/countitem are INCLUDED\n")
        f.write("#       (used as quest requirements but still from mobs)\n")
        f.write("# NOTE: Trade restricted items INCLUDED (can be modified)\n")
        f.write("#\n")
        f.write("# Format: ItemID,AegisName,ItemName,Weight\n")
        f.write("# ============================================================\n\n")
        for entry in sorted(safe_items, key=lambda x: x['item']['id']):
            item = entry['item']
            f.write(f"{item['id']},{item['aegis_name']},{item['name']},{item['weight']}\n")

    # MOB_DROP_DETAILS.csv
    with open(OUTPUT_DIR / "MOB_DROP_DETAILS.csv", 'w') as f:
        f.write("ItemID,ItemName,AegisName,MobID,MobName,DropRate,MobLevel\n")
        for entry in sorted(safe_items, key=lambda x: x['item']['id']):
            item = entry['item']
            for drop in entry['drops']:
                f.write(f"{item['id']},{item['name'].replace(',', ' ')},{item['aegis_name']},{drop['mob_id']},{drop['mob_name'].replace(',', ' ')},{drop['drop_rate']},{drop['mob_level']}\n")

    # EXCLUDED_ITEMS.txt
    with open(OUTPUT_DIR / "EXCLUDED_ITEMS.txt", 'w') as f:
        f.write("# EXCLUDED ITEMS\n")
        f.write(f"# Total: {len(excluded_items)}\n\n")
        for entry in sorted(excluded_items, key=lambda x: x['item']['id']):
            item = entry['item']
            reasons = "; ".join(entry['reasons'][:3])
            f.write(f"{item['id']},{item['name']},{reasons}\n")

    # QUICK_REFERENCE.txt
    with open(OUTPUT_DIR / "QUICK_REFERENCE.txt", 'w') as f:
        f.write(f"# Quick Reference - {len(safe_items)} verified items\n\n")
        for entry in sorted(safe_items, key=lambda x: x['item']['id']):
            item = entry['item']
            f.write(f"{item['id']},{item['name']}\n")

    # VERIFICATION_REPORT.md
    with open(OUTPUT_DIR / "VERIFICATION_REPORT.md", 'w') as f:
        f.write("# Verification Report: Pure Mob Drop Materials\n\n")
        f.write(f"**Generated:** {datetime.now()}\n\n")

        f.write("## Summary\n\n")
        f.write(f"| Metric | Count |\n|---|---|\n")
        f.write(f"| Total scanned | {len(items)} |\n")
        f.write(f"| **PASSED** | **{len(safe_items)}** |\n")
        f.write(f"| Excluded | {len(excluded_items)} |\n")
        f.write(f"| Pass Rate | {len(safe_items)/len(items)*100:.1f}% |\n\n")

        f.write("## Exclusion Rules\n\n")
        f.write("Items are **EXCLUDED** if:\n")
        f.write("1. No mob drops in mob_db.yml\n")
        f.write("2. Given by NPCs (getitem, shops, rewards)\n")
        f.write("3. Quest-like name contains: key, token, letter, permit, certificate, pass, ticket, voucher, evidence, proof, badge\n\n")

        f.write("Items are **INCLUDED** even if:\n")
        f.write("- Used in `delitem`/`countitem` (quest requirements)\n")
        f.write("- Trade restricted (NoDrop/NoTrade)\n\n")

        f.write("## Verified Items\n\n")
        f.write("| ID | AegisName | Name | Weight | Mobs |\n|---|---|---|---|---|\n")
        for entry in sorted(safe_items, key=lambda x: x['item']['id']):
            item = entry['item']
            f.write(f"| {item['id']} | {item['aegis_name']} | {item['name']} | {item['weight']} | {len(entry['drops'])} |\n")

    print(f"Output: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
