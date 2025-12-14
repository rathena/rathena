#!/usr/bin/env python3
"""Verify mob spawns for replacement items"""
import os
import re
from collections import defaultdict

# Read all spawn files
spawn_counts = defaultdict(int)
spawn_dir = "npc/pre-re/mobs"

for root, dirs, files in os.walk(spawn_dir):
    for f in files:
        if f.endswith('.txt'):
            filepath = os.path.join(root, f)
            with open(filepath, 'r') as file:
                for line in file:
                    # Match: mapname,x,y monster MobName MobID,count
                    match = re.search(r'monster\s+(\w+(?:\s+\w+)?)\s+(\d+),(\d+)', line)
                    if match:
                        mob_name = match.group(1)
                        mob_id = match.group(2)
                        count = int(match.group(3))
                        spawn_counts[mob_id] += count

# Read MOB_DROP_DETAILS.csv
mob_drops = defaultdict(list)
with open('Verified_Material_Loots/MOB_DROP_DETAILS.csv', 'r') as f:
    next(f)  # skip header
    for line in f:
        parts = line.strip().split(',')
        if len(parts) >= 5:
            item_id = parts[0]
            item_name = parts[1]
            mob_id = parts[3]
            mob_name = parts[4]
            mob_drops[item_id].append({
                'item_name': item_name,
                'mob_id': mob_id,
                'mob_name': mob_name,
                'spawns': spawn_counts.get(mob_id, 0)
            })

# Check replacement items
replacement_items = [
    '7317', '951', '906', '7319', '7345', '7507', '972', '926', '958',
    '1033', '1037', '7188', '7326', '7561', '924', '954', '1096',
    '7100', '7201', '7312', '7347', '7567'
]

print("=== REPLACEMENT ITEMS SPAWN VERIFICATION ===\n")

valid_items = []
invalid_items = []

for item_id in replacement_items:
    drops = mob_drops.get(item_id, [])
    total_spawns = sum(d['spawns'] for d in drops)
    item_name = drops[0]['item_name'] if drops else 'Unknown'

    print(f"[{item_id}] {item_name}:")
    for d in drops:
        status = "✓" if d['spawns'] >= 10 else "✗"
        print(f"  {status} {d['mob_name']}({d['mob_id']}): {d['spawns']} spawns")

    if total_spawns >= 50:
        valid_items.append((item_id, item_name, total_spawns))
        print(f"  TOTAL: {total_spawns} spawns - VALID\n")
    else:
        invalid_items.append((item_id, item_name, total_spawns))
        print(f"  TOTAL: {total_spawns} spawns - LOW!\n")

print("\n=== SUMMARY ===")
print(f"Valid items (50+ total spawns): {len(valid_items)}")
for item in valid_items:
    print(f"  ✓ {item[0]} {item[1]}: {item[2]} spawns")

print(f"\nInvalid items (< 50 spawns): {len(invalid_items)}")
for item in invalid_items:
    print(f"  ✗ {item[0]} {item[1]}: {item[2]} spawns")

# Find better alternatives
print("\n\n=== FINDING BETTER ALTERNATIVES ===")
print("Items with 100+ total spawns from high-spawn mobs:\n")

# Get all ETC items with good spawns
all_items = []
for item_id, drops in mob_drops.items():
    if item_id.startswith('7') or (item_id.isdigit() and 900 <= int(item_id) <= 1100):
        total = sum(d['spawns'] for d in drops)
        if total >= 100:
            item_name = drops[0]['item_name']
            all_items.append((item_id, item_name, total, drops))

all_items.sort(key=lambda x: -x[2])
for item_id, name, total, drops in all_items[:30]:
    top_mobs = sorted(drops, key=lambda x: -x['spawns'])[:3]
    mob_str = ', '.join([f"{d['mob_name']}({d['spawns']})" for d in top_mobs])
    print(f"{item_id} {name}: {total} spawns - {mob_str}")
