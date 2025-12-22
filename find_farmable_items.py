#!/usr/bin/env python3
"""Find truly farmable items from pre-renewal with proper spawn verification"""
import yaml
import os
import re
from collections import defaultdict

print("=== FINDING TRULY FARMABLE ETC ITEMS ===\n")

# Step 1: Load spawned mob IDs
print("Step 1: Loading spawned mob IDs from npc/pre-re/mobs/...")
spawned_mobs = set()
spawn_counts = defaultdict(int)

for root, dirs, files in os.walk("npc/pre-re/mobs"):
    for f in files:
        if f.endswith('.txt'):
            with open(os.path.join(root, f), 'r') as file:
                for line in file:
                    # Match: monster MobName MobID,count
                    match = re.search(r'monster\s+\S+\s+(\d+),(\d+)', line)
                    if match:
                        mob_id = int(match.group(1))
                        count = int(match.group(2))
                        spawned_mobs.add(mob_id)
                        spawn_counts[mob_id] += count

print(f"  Found {len(spawned_mobs)} unique mobs with spawns")

# Step 2: Load mob drops from pre-re mob_db.yml
print("\nStep 2: Loading mob drops from db/pre-re/mob_db.yml...")
mob_drops = []

with open("db/pre-re/mob_db.yml", 'r') as f:
    data = yaml.safe_load(f)

for mob in data.get('Body', []):
    mob_id = mob.get('Id')
    mob_name = mob.get('Name', 'Unknown')
    aegis_name = mob.get('AegisName', '')

    # Skip event mobs
    if aegis_name.startswith('E_') or aegis_name.startswith('EVENT_') or aegis_name.startswith('G_'):
        continue

    # Skip if mob doesn't spawn
    if mob_id not in spawned_mobs:
        continue

    # Get drops
    drops = mob.get('Drops', [])
    for drop in drops:
        item_name = drop.get('Item', '')
        drop_rate = drop.get('Rate', 0)
        mob_drops.append({
            'mob_id': mob_id,
            'mob_name': mob_name,
            'aegis_name': aegis_name,
            'item_aegis': item_name,
            'drop_rate': drop_rate,
            'spawn_count': spawn_counts.get(mob_id, 0)
        })

print(f"  Found {len(mob_drops)} drop entries from spawned mobs")

# Step 3: Load item IDs from pre-re item_db_etc.yml
print("\nStep 3: Loading ETC items from db/pre-re/item_db_etc.yml...")
item_ids = {}

with open("db/pre-re/item_db_etc.yml", 'r') as f:
    data = yaml.safe_load(f)

for item in data.get('Body', []):
    item_id = item.get('Id')
    aegis_name = item.get('AegisName', '')
    name = item.get('Name', aegis_name)

    # Only ETC items in range 900-1100 and 7000-7999
    if (900 <= item_id <= 1100) or (7000 <= item_id <= 7999):
        item_ids[aegis_name] = {'id': item_id, 'name': name}

print(f"  Found {len(item_ids)} ETC items in target range")

# Step 4: Check NPC scripts for items given away
print("\nStep 4: Scanning NPC scripts for getitem/shops...")
npc_given_items = set()

npc_dirs = [
    "npc/merchants",
    "npc/pre-re/merchants",
    "npc/pre-re/quests",
    "npc/quests",
    "npc/events",
    "npc/jobs",
    "npc/custom"
]

for npc_dir in npc_dirs:
    if os.path.exists(npc_dir):
        for root, dirs, files in os.walk(npc_dir):
            for f in files:
                if f.endswith('.txt'):
                    try:
                        with open(os.path.join(root, f), 'r', errors='ignore') as file:
                            content = file.read()
                            # Find getitem patterns
                            for match in re.finditer(r'getitem[2]?\s+(\w+|\d+)', content):
                                item = match.group(1)
                                if item.isdigit():
                                    npc_given_items.add(int(item))
                                else:
                                    if item in item_ids:
                                        npc_given_items.add(item_ids[item]['id'])
                            # Find shop patterns with item IDs
                            for match in re.finditer(r'shop\s+.*?(\d{3,4})\s*:', content):
                                npc_given_items.add(int(match.group(1)))
                    except:
                        pass

print(f"  Found {len(npc_given_items)} items given by NPCs")

# Step 5: Build farmable items list
print("\nStep 5: Building farmable items list...")
print("  Criteria: 20%+ drop rate, spawned mobs, not NPC-given\n")

farmable_items = defaultdict(list)

for drop in mob_drops:
    item_aegis = drop['item_aegis']
    if item_aegis not in item_ids:
        continue

    item_data = item_ids[item_aegis]
    item_id = item_data['id']

    # Skip if given by NPC
    if item_id in npc_given_items:
        continue

    # Only 20%+ drop rate (2000/10000)
    if drop['drop_rate'] < 2000:
        continue

    farmable_items[item_id].append({
        'item_name': item_data['name'],
        'item_aegis': item_aegis,
        'mob_id': drop['mob_id'],
        'mob_name': drop['mob_name'],
        'drop_rate': drop['drop_rate'],
        'spawn_count': drop['spawn_count']
    })

# Sort by total spawn count
sorted_items = []
for item_id, drops in farmable_items.items():
    total_spawns = sum(d['spawn_count'] for d in drops)
    best_drop_rate = max(d['drop_rate'] for d in drops)
    sorted_items.append({
        'item_id': item_id,
        'item_name': drops[0]['item_name'],
        'mob_count': len(drops),
        'total_spawns': total_spawns,
        'best_drop_rate': best_drop_rate,
        'drops': drops
    })

sorted_items.sort(key=lambda x: (-x['total_spawns'], -x['best_drop_rate']))

print("=" * 80)
print("VERIFIED FARMABLE ETC ITEMS (20%+ drop rate, spawned mobs, no NPC)")
print("=" * 80)
print(f"{'ID':<6} {'Item Name':<30} {'Mobs':<5} {'Spawns':<8} {'Best Drop':<10} Top Mob Sources")
print("-" * 80)

for item in sorted_items[:50]:
    top_mobs = sorted(item['drops'], key=lambda x: -x['spawn_count'])[:3]
    mob_str = ', '.join([f"{d['mob_name']}({d['spawn_count']})" for d in top_mobs])
    drop_pct = f"{item['best_drop_rate']/100:.0f}%"
    print(f"{item['item_id']:<6} {item['item_name'][:29]:<30} {item['mob_count']:<5} {item['total_spawns']:<8} {drop_pct:<10} {mob_str[:40]}")

print(f"\nTotal farmable items found: {len(sorted_items)}")

# Save to file
with open("Verified_Material_Loots/TRULY_FARMABLE.txt", "w") as f:
    f.write("# TRULY FARMABLE ETC ITEMS\n")
    f.write("# Criteria:\n")
    f.write("#   - Drop rate >= 20% (2000/10000)\n")
    f.write("#   - Mob actually spawns in npc/pre-re/mobs/\n")
    f.write("#   - Item NOT given by any NPC\n")
    f.write("#   - Item from db/pre-re/item_db_etc.yml\n")
    f.write(f"# Total: {len(sorted_items)} items\n")
    f.write("#\n")
    f.write("# Format: ItemID,ItemName,MobCount,TotalSpawns,BestDropRate\n")
    f.write("# ============================================================\n\n")

    for item in sorted_items:
        f.write(f"{item['item_id']},{item['item_name']},{item['mob_count']},{item['total_spawns']},{item['best_drop_rate']}\n")

print("\nSaved to: Verified_Material_Loots/TRULY_FARMABLE.txt")
