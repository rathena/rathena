#!/usr/bin/env python3
# [RO Sandbox] Regenerate db/import item and monster data from the
# RO_Items_PreRenewal Google Sheet (the source of truth).
#
# Run from the repo root:
#   python3 tools/sandbox/sheet2yaml.py
#   python3 tools/sandbox/sheet2yaml.py --client "/path/to/client folder"
#
# Writes:
#   db/import/item_db.yml  custom items (ID 30000+) and official items whose
#                          price or weight differs from rAthena
#   db/import/mob_db.yml   monsters whose drops differ from rAthena
# With --client, also writes the client's SystemEN/itemInfo_C.lua (ROTP's
# custom item file), borrowing each item's icon from its "Client Look (ID)".
#
# Needs the sheet shared as "Anyone with the link can view".
# Standard library only, so nothing to install.

import argparse
import csv
import io
import os
import re
import sys
import textwrap
import urllib.request
from collections import Counter

SHEET_ID = "1jWZez0npq46j8lH682okNJrxiZRB0Izs_GIMvZlyHss"
TAB_GIDS = {"Items": 1345594810, "Monster Drops": 2101865557}

ITEM_DB_SOURCES = [
    "db/pre-re/item_db_equip.yml",
    "db/pre-re/item_db_usable.yml",
    "db/pre-re/item_db_etc.yml",
]
MOB_DB_SOURCE = "db/pre-re/mob_db.yml"
ITEM_DB_OUT = "db/import/item_db.yml"
MOB_DB_OUT = "db/import/mob_db.yml"

CUSTOM_ID_START = 30000
MAX_MOB_DROP = 10  # src/map/mob.hpp
MAX_WEAPON_LEVEL = 4  # src/common/mmo.hpp, pre-renewal

# Sheet values that differ from rAthena's names.
TYPE_MAP = {"Food": "Usable"}  # food effects come in Phase 7
SUBTYPE_MAP = {"Two-Handed Axe": "2hAxe"}
LOCATION_MAP = {
    "Right Hand": "Right_Hand",
    "Both Hands": "Both_Hand",
    "Accessory": "Both_Accessory",
}

# Client (ROTP translation) files, relative to the client folder.
CLIENT_ITEMINFO = "SystemEN/LuaFiles514/itemInfo.lua"
CLIENT_CUSTOM = "SystemEN/itemInfo_C.lua"
# How the item type is shown in the description, like official items.
CLIENT_TYPE = {"Etc": "Collectible", "Usable": "Usable", "Healing": "Restorative",
               "Food": "Food"}
CLIENT_SUBTYPE = {"1hSpear": "One-Handed Spear", "2hAxe": "Two-Handed Axe",
                  "2hSword": "Two-Handed Sword"}

errors = []


def fetch_tab(name):
    url = (f"https://docs.google.com/spreadsheets/d/{SHEET_ID}"
           f"/export?format=csv&gid={TAB_GIDS[name]}")
    with urllib.request.urlopen(url) as resp:
        text = resp.read().decode("utf-8")
    if text.lstrip().startswith("<"):
        sys.exit(f"Got a web page instead of CSV for {name}. "
                 "Is the sheet shared as 'Anyone with the link'?")
    return list(csv.reader(io.StringIO(text)))


def table(rows, first_header):
    """Rows below the header row, as dicts keyed by header name."""
    for i, row in enumerate(rows):
        if row and row[0].strip() == first_header:
            header = [h.strip() for h in row]
            return [dict(zip(header, r + [""] * len(header)))
                    for r in rows[i + 1:] if r and r[0].strip()]
    sys.exit(f"Header '{first_header}' not found")


def number(text):
    """'7,764' -> 7764, '7.' -> 7.0, '' -> None"""
    text = text.replace(",", "").strip()
    return float(text) if text else None


# ---------- rAthena data (simple line parser for its YAML layout) ----------

def read_rathena_items():
    items = {}
    for path in ITEM_DB_SOURCES:
        cur = None
        for line in open(path, encoding="utf-8"):
            m = re.match(r"  - Id: (\d+)", line)
            if m:
                cur = items[int(m.group(1))] = {}
                continue
            m = re.match(r"    (AegisName|Buy|Sell|Weight): (.+)", line)
            if cur is not None and m:
                cur[m.group(1)] = m.group(2).strip()
    return items


def read_rathena_mobs():
    mobs = {}
    cur = section = drop = None
    for line in open(MOB_DB_SOURCE, encoding="utf-8"):
        m = re.match(r"  - Id: (\d+)", line)
        if m:
            cur = mobs[int(m.group(1))] = []
            section = None
            continue
        if cur is None:
            continue
        m = re.match(r"    (\w+):", line)
        if m:
            section = m.group(1) if m.group(1) in ("Drops", "MvpDrops") else None
            continue
        if not section:
            continue
        m = re.match(r"      - Item: (.+)", line)
        if m:
            drop = {"Item": m.group(1).strip(), "Mvp": section == "MvpDrops",
                    "Rate": 0, "Steal": False}
            cur.append(drop)
            continue
        m = re.match(r"        (Rate|StealProtected): (.+)", line)
        if m and drop:
            if m.group(1) == "Rate":
                drop["Rate"] = int(m.group(2))
            else:
                drop["Steal"] = m.group(2).strip() == "true"
    return mobs


# ---------- items ----------

def custom_item(row):
    item_id = int(row["Item ID"])
    notes = row["Notes"]
    item_type = TYPE_MAP.get(row["Type"], row["Type"])
    lines = [f"  - Id: {item_id}",
             f"    AegisName: {row['Aegis Name']}",
             f"    Name: {yaml_str(row['Name'])}",
             f"    Type: {item_type}"]
    if item_type == "Weapon":
        lines.append(f"    SubType: {SUBTYPE_MAP.get(row['Subtype'], row['Subtype'])}")
    lines.append(f"    Buy: {int(number(row['Buy Price (z)']))}")
    lines.append(f"    Sell: {int(number(row['Sell Price (z)']))}")
    lines.append(f"    Weight: {round(number(row['Weight']) * 10)}")

    if item_type == "Weapon":
        atk = re.search(r"\bATK (\d+)", notes)
        wlv = re.search(r"\bTier (\d+)", notes)  # the sheet's Tier = weapon level
        if not atk:
            errors.append(f"{item_id} {row['Name']}: no 'ATK n' in Notes")
        if not wlv:
            errors.append(f"{item_id} {row['Name']}: no 'Tier n' in Notes")
        elif int(wlv.group(1)) > MAX_WEAPON_LEVEL:
            errors.append(f"{item_id} {row['Name']}: Tier {wlv.group(1)} is above "
                          f"pre-renewal's maximum weapon level {MAX_WEAPON_LEVEL}")
        if atk:
            lines.append(f"    Attack: {atk.group(1)}")
    if row["Slots"].strip():
        lines.append(f"    Slots: {int(number(row['Slots']))}")
    if row["Equip Slot"].strip():
        slot = row["Equip Slot"].strip()
        if slot not in LOCATION_MAP:
            errors.append(f"{item_id} {row['Name']}: unknown Equip Slot '{slot}'")
        else:
            lines += ["    Locations:", f"      {LOCATION_MAP[slot]}: true"]
    if item_type == "Weapon":
        if wlv:
            lines.append(f"    WeaponLevel: {wlv.group(1)}")
        lines.append("    Refineable: true")
    if item_type == "Armor":
        lines.append("    ArmorLevel: 1")  # the only armor level pre-renewal has
    if row["Equip Lv"].strip():
        lines.append(f"    EquipLevelMin: {int(number(row['Equip Lv']))}")
    return lines


def changed_official_item(row, rathena):
    """Only the fields that differ from rAthena, or None."""
    buy, sell = number(row["Buy Price (z)"]), number(row["Sell Price (z)"])
    weight = round((number(row["Weight"]) or 0) * 10)
    ra_buy = int(rathena["Buy"]) if "Buy" in rathena else None
    ra_sell = int(rathena["Sell"]) if "Sell" in rathena else None
    if ra_buy is None and ra_sell is not None:
        ra_buy = ra_sell * 2
    if ra_sell is None:
        ra_sell = (ra_buy or 0) // 2
    ra_buy = ra_buy or 0

    lines = []
    if buy is not None and (int(buy) != ra_buy or int(sell or 0) != ra_sell):
        lines += [f"    Buy: {int(buy)}", f"    Sell: {int(sell or 0)}"]
    if weight != int(rathena.get("Weight", 0)):
        lines.append(f"    Weight: {weight}")
    if not lines:
        return None
    return [f"  - Id: {row['Item ID']}", f"    AegisName: {row['Aegis Name']}"] + lines


# ---------- monsters ----------

def drops_from_sheet(rows):
    mobs = {}
    for r in rows:
        mobs.setdefault(int(r["Monster ID"]), []).append({
            "ItemId": int(r["Item ID"]),
            "Rate": round(number(r["Drop Rate (%)"]) * 100),  # rAthena: 1 = 0.01%
            "Mvp": r["MVP Drop"].strip() == "Yes",
            "Steal": r["Steal Protected"].strip() == "Yes",
        })
    return mobs


def drop_key(d):
    return (d["Item"], d["Rate"], d["Mvp"], d["Steal"])


def mob_entry(mob_id, drops):
    lines = [f"  - Id: {mob_id}"]
    for section, mvp in (("MvpDrops", True), ("Drops", False)):
        part = [d for d in drops if d["Mvp"] == mvp]
        if not part:
            continue
        lines.append(f"    {section}:")
        for i, d in enumerate(part):
            lines += [f"      - Item: {d['Item']}", f"        Rate: {d['Rate']}",
                      f"        Index: {i}"]
            if d["Steal"]:
                lines.append("        StealProtected: true")
    return lines


# ---------- client item info ----------

def read_client_looks(client_dir, look_ids):
    """Icon name and view (ClassNum) of each look-alike item, read from the
    client's own item list. Read as latin-1 so the Korean (CP949) icon names
    are copied byte for byte."""
    path = os.path.join(client_dir, CLIENT_ITEMINFO)
    with open(path, encoding="latin-1") as f:
        text = f.read()
    looks = {}
    for look_id in look_ids:
        start = text.find(f"[{look_id}] = {{")
        if start < 0:
            continue
        block = text[start:text.find("costume", start)]
        res = re.search(r'identifiedResourceName = "([^"]*)"', block)
        cls = re.search(r"ClassNum = (\d+)", block)
        if res:
            looks[look_id] = (res.group(1), int(cls.group(1)) if cls else 0)
    return looks


def lua_str(text):
    return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'


def client_entry(row, look):
    resource, class_num = look
    item_type = row["Type"]
    # Drop what the stat lines below already show.
    notes = row["Notes"].strip()
    runic = notes.startswith("Custom runic weapon.")
    notes = re.sub(r"^Custom (item|food|recipe page|runic weapon)\. ", "", notes)
    notes = re.sub(r"^Tier \d+\. ", "", notes)
    notes = re.sub(r"^ATK \d+[.,] ", "", notes)
    if runic:
        notes = "Runic weapon. " + notes
    desc = textwrap.wrap(notes, 55) + ["_______________________"]

    def stat(label, value):
        desc.append(f"^0000CC{label}:^000000 {value}")

    if item_type == "Weapon":
        sub = SUBTYPE_MAP.get(row["Subtype"], row["Subtype"])
        stat("Type", CLIENT_SUBTYPE.get(sub, sub))
        atk = re.search(r"\bATK (\d+)", row["Notes"])
        if atk:
            stat("Attack", atk.group(1))
    elif item_type == "Armor":
        stat("Type", row["Equip Slot"].strip() or "Armor")
    else:
        stat("Type", CLIENT_TYPE.get(item_type, item_type))
    stat("Weight", f"{number(row['Weight']):g}")
    tier = re.search(r"\bTier (\d+)", row["Notes"])
    if item_type == "Weapon" and tier:
        stat("Weapon Level", tier.group(1))
    if row["Equip Lv"].strip():
        stat("Requirement", f"Base Level {int(number(row['Equip Lv']))}")

    name = lua_str(row["Name"])
    res = lua_str(resource)
    lines_desc = ",\n".join("\t\t\t" + lua_str(d) for d in desc)
    slots = int(number(row["Slots"])) if row["Slots"].strip() else 0
    return (f"\t[{row['Item ID']}] = {{\n"
            f"\t\tunidentifiedDisplayName = {name},\n"
            f"\t\tunidentifiedResourceName = {res},\n"
            f"\t\tunidentifiedDescriptionName = {{ \"\" }},\n"
            f"\t\tidentifiedDisplayName = {name},\n"
            f"\t\tidentifiedResourceName = {res},\n"
            f"\t\tidentifiedDescriptionName = {{\n{lines_desc}\n\t\t}},\n"
            f"\t\tslotCount = {slots},\n"
            f"\t\tClassNum = {class_num},\n"
            f"\t\tcostume = false\n"
            f"\t}},")


def write_client(client_dir, entries):
    path = os.path.join(client_dir, CLIENT_CUSTOM)
    out = ["-- [RO Sandbox] Generated by tools/sandbox/sheet2yaml.py from the",
           "-- RO_Items_PreRenewal sheet. Don't edit by hand: change the sheet",
           "-- (the Client Look column sets each icon) and run the script again.",
           "",
           "-- Table for Custom Items",
           "tbl_custom = {"] + entries + [
           "}",
           "",
           "-- Table for Official Overrides",
           "tbl_override = {",
           "}",
           ""]
    with open(path, "w", encoding="latin-1", newline="\r\n") as f:
        f.write("\n".join(out))
    return path


# ---------- output ----------

def yaml_str(text):
    return f'"{text}"' if re.search(r"[:#'\"]", text) else text


def write_import(path, header_type, version, entries):
    with open(path, encoding="utf-8") as f:
        keep = f.read()
    # Keep rAthena's comment block, without our note from an earlier run.
    keep = keep.split("\nHeader:")[0].split("\n# [RO Sandbox]")[0].rstrip("\n")
    out = [keep, "", "# [RO Sandbox] Generated by tools/sandbox/sheet2yaml.py "
           "from the RO_Items_PreRenewal sheet.", "# Don't edit by hand: "
           "change the sheet and run the script again.", "",
           "Header:", f"  Type: {header_type}", f"  Version: {version}", ""]
    if entries:
        out.append("Body:")
        for e in entries:
            out += e
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(out) + "\n")


def main():
    parser = argparse.ArgumentParser(description="Regenerate import files from the sheet.")
    parser.add_argument("--client", help="client folder; also writes its SystemEN/itemInfo_C.lua")
    args = parser.parse_args()

    items_rows = table(fetch_tab("Items"), "Item ID")
    drop_rows = table(fetch_tab("Monster Drops"), "Monster ID")
    ra_items = read_rathena_items()
    ra_mobs = read_rathena_mobs()
    ra_aegis = {v["AegisName"].lower(): k for k, v in ra_items.items() if "AegisName" in v}

    item_entries, n_custom, n_changed = [], 0, 0
    aegis_by_id, custom_rows = {}, []
    for row in items_rows:
        if not row["Item ID"].strip().isdigit():
            continue
        item_id = int(row["Item ID"])
        aegis_by_id[item_id] = row["Aegis Name"].strip()
        if item_id >= CUSTOM_ID_START:
            if aegis_by_id[item_id].lower() in ra_aegis:
                errors.append(f"{item_id} {row['Name']}: Aegis name "
                              f"'{aegis_by_id[item_id]}' is already used by item "
                              f"{ra_aegis[aegis_by_id[item_id].lower()]}")
            item_entries.append((item_id, custom_item(row)))
            custom_rows.append(row)
            n_custom += 1
        elif item_id in ra_items:
            entry = changed_official_item(row, ra_items[item_id])
            if entry:
                item_entries.append((item_id, entry))
                n_changed += 1
        else:
            errors.append(f"Item {item_id} is below {CUSTOM_ID_START} but not in rAthena")

    mob_entries = []
    for mob_id, drops in sorted(drops_from_sheet(drop_rows).items()):
        if mob_id not in ra_mobs:
            errors.append(f"Monster {mob_id} is not in rAthena's mob_db")
            continue
        for d in drops:
            d["Item"] = aegis_by_id.get(d["ItemId"])
            if not d["Item"]:
                errors.append(f"Monster {mob_id}: item {d['ItemId']} is not on the Items tab")
        if any(not d["Item"] for d in drops):
            continue
        sheet = Counter(map(drop_key, drops))
        rathena = Counter(map(drop_key, ra_mobs[mob_id]))
        if sheet == rathena:
            continue
        for mvp in (False, True):
            n_sheet = sum(1 for d in drops if d["Mvp"] == mvp)
            n_ra = sum(1 for d in ra_mobs[mob_id] if d["Mvp"] == mvp)
            if n_sheet > MAX_MOB_DROP:
                errors.append(f"Monster {mob_id}: {n_sheet} drops, rAthena allows {MAX_MOB_DROP}")
            if n_sheet < n_ra:
                errors.append(f"Monster {mob_id}: sheet has fewer drops than rAthena; "
                              "an import file can't remove drops")
        mob_entries.append(mob_entry(mob_id, drops))

    client_entries = []
    if args.client:
        look_of = {}
        for row in custom_rows:
            look = row.get("Client Look (ID)", "").strip()
            if not look.isdigit():
                errors.append(f"{row['Item ID']} {row['Name']}: no Client Look (ID)")
            else:
                look_of[row["Item ID"]] = int(look)
        looks = read_client_looks(args.client, set(look_of.values()))
        for row in sorted(custom_rows, key=lambda r: int(r["Item ID"])):
            look = look_of.get(row["Item ID"])
            if look is None:
                continue
            if look not in looks:
                errors.append(f"{row['Item ID']} {row['Name']}: Client Look {look} "
                              "is not in the client's item list")
                continue
            client_entries.append(client_entry(row, looks[look]))

    if errors:
        print("Not written. Fix these on the sheet first:")
        for e in errors:
            print("  -", e)
        sys.exit(1)

    write_import(ITEM_DB_OUT, "ITEM_DB", 3, [e for _, e in sorted(item_entries)])
    write_import(MOB_DB_OUT, "MOB_DB", 5, mob_entries)
    print(f"{ITEM_DB_OUT}: {n_custom} custom items, {n_changed} changed official items")
    print(f"{MOB_DB_OUT}: {len(mob_entries)} monsters with changed drops")
    if args.client:
        path = write_client(args.client, client_entries)
        print(f"{path}: {len(client_entries)} custom items")


if __name__ == "__main__":
    main()
