#!/usr/bin/env python3
"""Export a client's System/Rune tables to the Rune Tablet databases.

Reads the compiled System/Rune lubs (via lub51.py) and writes
rune_tablet_db.yml and rune_tablet_decomposition.yml. With --client-out it
also writes English rune_desc.lub/runeset_desc.lub for the client. Set effects are parsed from the zh-TW (TWRoZ) set descriptions;
other languages export sets without Effects (not equippable until reviewed).

Usage:
    python tools/rune_tablet/export_db.py --rune C:\\Gravity\\RagnarokZero\\System\\Rune ^
        --encoding cp950 --db-out db/pre-re --client-out <game>\\System\\Rune
    python tools/rune_tablet/export_db.py --rune client/rune_tablet/System/Rune ^
        --encoding cp949 --check db/re/rune_tablet_db.yml

--check compares every non-effect field with an existing rune_tablet_db.yml.
See doc/rune_tablet.txt (Pre-renewal data).
"""
import argparse, glob, hashlib, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lub51  # noqa: E402

DATA_LUBS = ("runesystemid.lub", "rune_info.lub", "runeset_info.lub", "runeSystem_table.lub",
             "runeset_reward.lub", "itemDecom.lub", "rune_desc.lub", "runeset_desc.lub")


def ordered(table):
    """Numeric Lua table keys in ascending order."""
    return [table[k] for k in sorted(table, key=lambda k: int(k))]


def load(rune_dir):
    """Run the System/Rune data lubs; text (decompiled) lubs are skipped."""
    G = {}
    hashes = {}
    for name in DATA_LUBS:
        path = os.path.join(rune_dir, name)
        if not os.path.exists(path):
            continue
        data = open(path, "rb").read()
        hashes[name] = hashlib.sha256(data).hexdigest()
        if not data.startswith(b"\x1bLua"):
            print("skip text lub:", name, file=sys.stderr)
            continue
        G = lub51.run_file(data, G)
    return G, hashes


def materials(item_list, index):
    """Item cost list, summed per item and sorted by item ID (the patch's export order)."""
    total = {}
    for e in ordered(key(item_list, index)):
        total[key(e, 1)] = total.get(key(e, 1), 0) + key(e, 2)
    return sorted(total.items())


def key(d, k):
    """Lua tables decoded by lub51 use int keys; JSON round trips use str keys."""
    return d[k] if k in d else d[str(k)]


# zh-TW effect texts -> (bonus, parameter, value multiplier, needs percent)
STATS = {
    "STR": ("bStr", None, 1, False), "AGI": ("bAgi", None, 1, False), "VIT": ("bVit", None, 1, False),
    "INT": ("bInt", None, 1, False), "DEX": ("bDex", None, 1, False), "LUK": ("bLuk", None, 1, False),
    "FLEE": ("bFlee", None, 1, False), "HIT": ("bHit", None, 1, False),
    "DEF": ("bDef", None, 1, False), "MDEF": ("bMdef", None, 1, False),
    "MHP": ("bMaxHPrate", None, 1, True), "MSP": ("bMaxSPrate", None, 1, True),
    "冰凍抗性": ("bResEff", "Eff_Freeze", 100, True), "暈眩抗性": ("bResEff", "Eff_Stun", 100, True),
    "石化抗性": ("bResEff", "Eff_Stone", 100, True), "睡眠抗性": ("bResEff", "Eff_Sleep", 100, True),
    "攻擊速度增加": ("bAspdRate", None, 1, True),
}
ENGLISH = {
    "bStr": "STR", "bAgi": "AGI", "bVit": "VIT", "bInt": "INT", "bDex": "DEX", "bLuk": "LUK",
    "bFlee": "FLEE", "bHit": "HIT", "bDef": "DEF", "bMdef": "MDEF",
    "bMaxHPrate": "MaxHP", "bMaxSPrate": "MaxSP", "bAspdRate": "Attack speed",
    ("bResEff", "Eff_Freeze"): "Freeze resistance", ("bResEff", "Eff_Stun"): "Stun resistance",
    ("bResEff", "Eff_Stone"): "Stone resistance", ("bResEff", "Eff_Sleep"): "Sleep resistance",
}
RE_BASE = re.compile(r"^(?P<stat>[^+\-\d(（]+?)\s*(?:[(（][^)）]*[)）])?\s*(?P<sign>[+-]?)\s*(?P<val>\d+)\s*(?P<pct>%?)$")
RE_STEP = re.compile(r"^強化\s*\+(?P<lvl>\d+)\s*時[，,]\s*(?P<stat>.+?)\s*再\s*(?P<sign>[+-]?)\s*(?P<val>\d+)\s*(?P<pct>%?)$")


def parse_effects(fragments, lines, where):
    effects = []
    for line in lines:
        text = line.strip()
        m = RE_STEP.match(text)
        level = int(m["lvl"]) if m else 0
        m = m or RE_BASE.match(text)
        if not m:
            raise ValueError("%s: unparsed effect line %r" % (where, text))
        stat = re.sub(r"\s*[(（][^)）]*[)）]\s*", "", m["stat"]).strip()
        if stat not in STATS:
            raise ValueError("%s: unknown stat %r in %r" % (where, stat, text))
        bonus, param, mult, pct = STATS[stat]
        if bool(m["pct"]) != pct:
            raise ValueError("%s: unexpected unit in %r" % (where, text))
        value = int(m["val"]) * mult * (-1 if m["sign"] == "-" else 1)
        effects.append({"Fragments": fragments, "Bonus": bonus, "Base": value, "PerStep": 0,
                        "LevelStep": 1, "MinLevel": level, "Parameter": param})
    return effects


def english_line(e):
    name = ENGLISH[(e["Bonus"], e["Parameter"])] if e["Parameter"] else ENGLISH[e["Bonus"]]
    value = e["Base"] // 100 if e["Bonus"] == "bResEff" else e["Base"]
    unit = "%" if e["Bonus"] in ("bResEff", "bMaxHPrate", "bMaxSPrate", "bAspdRate") else ""
    text = "%s %+d%s" % (name, value, unit)
    if e["MinLevel"]:
        return "At +%d upgrade, %s more." % (e["MinLevel"], text)
    if e["Bonus"] == "bAspdRate":
        return "%s (after-attack delay -%d%%)." % (text, value)
    return text + "."


def build(G, encoding_has_effects):
    tags = G["RUNESETIDTBL"], G["RUNEIDTBL"]
    item_list = G["RuneTable_itemList"]
    grade, grade_fail = G.get("GradeTable", {}), G.get("GradeTable_Fail", {})
    rewards = G.get("RuneSettbl_Reward", {})
    set_desc = G.get("RuneSettbl_desc", {})
    records = []
    for tag in sorted(G["Runetbl_info"], key=int):
        for rid in sorted(G["Runetbl_info"][tag], key=int):
            e = G["Runetbl_info"][tag][rid]
            records.append({"Id": int(rid), "Type": "Fragment", "Category": int(tag), "Name": e["Rune_Res"],
                            "Materials": materials(item_list, e["Rune_ActiveList"])})
    for tag in sorted(G["RuneSettbl_info"], key=int):
        for sid in sorted(G["RuneSettbl_info"][tag], key=int):
            e = G["RuneSettbl_info"][tag][sid]
            rec = {"Id": int(sid), "Type": "Set", "Category": int(tag), "Name": e["RuneSetRes"],
                   "Materials": materials(item_list, e["RuneSetActiveList"])}
            if e.get("RuneSet_UpGradeList"):
                pct = key(grade, e["RuneSet_UpGrade_Percentage_table"])
                fail = key(grade_fail, e["RuneSet_UpGrade_Percentage_table_Fail"])
                rec["Upgrades"] = [{"Level": int(lv), "Chance": key(pct, lv), "FailureIncrement": key(fail, lv),
                                    "Materials": materials(item_list, idx)}
                                   for lv, idx in sorted(e["RuneSet_UpGradeList"].items(), key=lambda x: int(x[0]))]
            slots = key(key(rewards, tag), sid) if tag in rewards and sid in key(rewards, tag) else {}
            if slots:
                rec["Rewards"] = [(int(s), it) for s, it in sorted(slots.items(), key=lambda x: int(x[0])) if it]
            rec["Fragments"] = [f for f in ordered(e["RuneSet_SlotList"]) if f]
            if encoding_has_effects and tag in set_desc and sid in set_desc[tag]:
                rec["Effects"] = []
                for frag, lines in sorted(set_desc[tag][sid]["RuneSetDescription"].items(), key=lambda x: int(x[0])):
                    rec["Effects"] += parse_effects(int(frag), ordered(lines), "set %s" % sid)
            records.append(rec)
    return records


def yaml_records(records):
    out = []
    for r in records:
        out += ["  - Id: %d" % r["Id"], "    Type: %s" % r["Type"], "    Category: %d" % r["Category"],
                "    Name: %s" % r["Name"], "    Materials:"]
        for it, amount in r["Materials"]:
            out += ["      - Item: %d" % it, "        Amount: %d" % amount]
        if "Upgrades" in r:
            out.append("    Upgrades:")
            for u in r["Upgrades"]:
                out += ["      - Level: %d" % u["Level"], "        Chance: %d" % u["Chance"],
                        "        FailureIncrement: %d" % u["FailureIncrement"], "        Materials:"]
                for it, amount in u["Materials"]:
                    out += ["          - Item: %d" % it, "            Amount: %d" % amount]
        if r.get("Rewards"):
            out.append("    Rewards:")
            for slot, it in r["Rewards"]:
                out += ["      - Slot: %d" % slot, "        Item: %d" % it]
        if r["Type"] == "Set":
            out.append("    Fragments:")
            out += ["      - Id: %d" % f for f in r["Fragments"]]
        if r.get("Effects"):
            out.append("    Effects:")
            for e in r["Effects"]:
                out += ["      - Fragments: %d" % e["Fragments"], "        Bonus: %s" % e["Bonus"],
                        "        Base: %d" % e["Base"], "        PerStep: %d" % e["PerStep"],
                        "        LevelStep: %d" % e["LevelStep"], "        MinLevel: %d" % e["MinLevel"]]
                if e["Parameter"]:
                    out.append('        Parameter: "%s"' % e["Parameter"])
    return out


def yaml_decomposition(G):
    num = G["itemDecomItemNum_tbl"]
    types = G["itemDecomType_tbl"]
    out = []
    for card in sorted(G["itemDecom_tbl"], key=int):
        modes = G["itemDecom_tbl"][card]
        out += ["  - Item: %d" % int(card), "    Modes:"]
        for mode in sorted(modes, key=int):
            out += ["      - Mode: %d" % int(mode), "        Consume: %d" % key(num, mode), "        Outputs:"]
            for o in ordered(key(types, modes[mode])):
                out += ["          - Item: %d" % key(o, 1), "            Minimum: %d" % key(o, 2),
                        "            Maximum: %d" % key(o, 3), "            Chance: %d" % key(o, 4)]
    return out


def card_names(item_db_globs):
    import yaml
    loader = getattr(yaml, "CSafeLoader", yaml.SafeLoader)
    names = {}
    for pattern in item_db_globs:
        for path in glob.glob(pattern):
            for e in (yaml.load(open(path, encoding="utf-8"), Loader=loader) or {}).get("Body") or []:
                if "Name" in e:
                    names[e["Id"]] = re.sub(r"\s*#.*$", "", str(e["Name"])).strip()
    return names


def lua_string(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def write_client(records, G, english_sets, names, out_dir):
    tag_name = {v: k for k, v in G["RUNETAGIDTBL"].items()}
    rune_name = {v: k for k, v in G["RUNEIDTBL"].items()}
    set_name = {v: k for k, v in G["RUNESETIDTBL"].items()}
    header = ["-- Rune Tablet client text (English), generated by tools/rune_tablet/export_db.py", ""]
    lines = header + ["Runetbl_desc = {"]
    tags = sorted({r["Category"] for r in records})
    for t in tags:
        frags = [r for r in records if r["Type"] == "Fragment" and r["Category"] == t]
        lines.append("\t[RUNETAGIDTBL.%s] = {" % tag_name[t])
        for i, r in enumerate(frags):
            card = r["Materials"][0][0]
            label = "%s Rune" % names.get(card, r["Name"][3:].replace("_", " "))
            lines.append("\t\t[RUNEIDTBL.%s] = { Rune_DisplayName = %s }%s" % (
                rune_name[r["Id"]], lua_string(label), "," if i < len(frags) - 1 else ""))
        lines.append("\t}" + ("," if t != tags[-1] else ""))
    lines.append("}")
    write_text(os.path.join(out_dir, "rune_desc.lub"), lines)

    lines = header + ["Runesystemtbl_tag = {"]
    lines += ["\t[RUNETAGIDTBL.%s] = %s%s" % (tag_name[t], lua_string(tag_name[t]), "," if t != tags[-1] else "") for t in tags]
    lines += ["}", "RuneSettbl_desc = {"]
    for t in tags:
        sets = [r for r in records if r["Type"] == "Set" and r["Category"] == t]
        lines.append("\t[RUNETAGIDTBL.%s] = {" % tag_name[t])
        for i, r in enumerate(sets):
            by_count = {}
            for e in r.get("Effects", []):
                by_count.setdefault(e["Fragments"], []).append(english_line(e))
            desc = ", ".join("[%d] = { %s }" % (c, ", ".join(lua_string(l) for l in ls)) for c, ls in sorted(by_count.items()))
            lines += ["\t\t[RUNESETIDTBL.%s] = {" % set_name[r["Id"]],
                      "\t\t\tRuneSetDisplayName = %s," % lua_string(english_sets.get(r["Id"], r["Name"])),
                      "\t\t\tRuneSetDescription = { %s }" % desc,
                      "\t\t}" + ("," if i < len(sets) - 1 else "")]
        lines.append("\t}" + ("," if t != tags[-1] else ""))
    lines.append("}")
    write_text(os.path.join(out_dir, "runeset_desc.lub"), lines)


def write_text(path, lines, newline="\r\n"):
    with open(path, "w", encoding="ascii", newline=newline) as f:
        f.write("\n".join(lines) + "\n")
    print("wrote", path)


def check(records, path):
    import yaml
    loader = getattr(yaml, "CSafeLoader", yaml.SafeLoader)
    ref = {e["Id"]: e for e in yaml.load(open(path, encoding="utf-8"), Loader=loader)["Body"]}
    problems = 0
    for r in records:
        e = ref.get(r["Id"])
        got = {"Type": r["Type"], "Category": r["Category"], "Name": r["Name"],
               "Materials": [{"Item": i, "Amount": a} for i, a in r["Materials"]]}
        want = {k: e.get(k) for k in got} if e else None
        if r["Type"] == "Set" and e:
            got["Fragments"] = [{"Id": f} for f in r["Fragments"]]
            want["Fragments"] = e.get("Fragments")
            got["Upgrades"] = [{"Level": u["Level"], "Chance": u["Chance"], "FailureIncrement": u["FailureIncrement"],
                                "Materials": [{"Item": i, "Amount": a} for i, a in u["Materials"]]}
                               for u in r.get("Upgrades", [])] or None
            want["Upgrades"] = e.get("Upgrades")
            got["Rewards"] = [{"Slot": s, "Item": i} for s, i in r.get("Rewards", [])] or None
            want["Rewards"] = e.get("Rewards")
        if got != want:
            problems += 1
            if problems <= 10:
                print("MISMATCH", r["Id"], {k: (got[k], want.get(k) if want else None) for k in got if not want or got[k] != want.get(k)})
    missing = set(ref) - {r["Id"] for r in records}
    print("checked %d records against %s: %d mismatches, %d only in reference" % (len(records), path, problems, len(missing)))
    return problems == 0 and not missing


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--rune", required=True, help="client System/Rune folder")
    ap.add_argument("--encoding", default="cp949", help="client text encoding (cp950 for TWRoZ)")
    ap.add_argument("--db-out", help="folder for rune_tablet_db.yml and rune_tablet_decomposition.yml")
    ap.add_argument("--client-out", help="folder for the English rune_desc.lub and runeset_desc.lub")
    ap.add_argument("--item-db", nargs="*", default=["db/pre-re/item_db*.yml", "db/import/item_db*.yml"],
                    help="item DB globs for English card names")
    ap.add_argument("--check", help="compare non-effect fields with an existing rune_tablet_db.yml")
    args = ap.parse_args()

    lub51.ENCODING = args.encoding
    G, hashes = load(args.rune)
    records = build(G, args.encoding == "cp950")
    if args.check:
        sys.exit(0 if check(records, args.check) else 1)
    if args.db_out:
        source = ["#   %s %s" % (n, h) for n, h in sorted(hashes.items())]
        write_db(os.path.join(args.db_out, "rune_tablet_db.yml"), "RUNE_TABLET_DB", source, yaml_records(records))
        if "itemDecom_tbl" in G:
            write_db(os.path.join(args.db_out, "rune_tablet_decomposition.yml"), "RUNE_TABLET_DECOMPOSITION_DB",
                     source, yaml_decomposition(G))
    if args.client_out:
        write_client(records, G, SETS_EN, card_names(args.item_db), args.client_out)


RULE = "#" * 75


def write_db(path, type_, source, body):
    """Keep the banner of the existing file and replace its generated part."""
    head = open(path, encoding="utf-8").read().split("\nHeader:")[0] if os.path.exists(path) else RULE
    head = head.split("\n# Generated by tools/rune_tablet/export_db.py")[0].rstrip("\n")
    lines = [head, "# Generated by tools/rune_tablet/export_db.py from System/Rune (sha256):"] + source + [
        RULE, "", "Header:", "  Type: %s" % type_, "  Version: 1", "", "Body:"] + body
    write_text(path, lines, "\n")


# English Rune Set names (iRO map names). Where the TWRoZ label disagrees with
# the resource name (Rt_T4_Iz_dun_lo, Rt_T4_Pay_dun_lo are labelled "upper"),
# the resource name wins.
SETS_EN = {
    1260068: "Poring Family", 1260069: "Upper Undersea Cave", 1260070: "Umbala", 1260071: "Comodo Beach",
    1260072: "Prontera Outskirts", 1260073: "Morroc Outskirts", 1260074: "Sunken Ship",
    1260075: "Geffen Outskirts", 1260076: "Payon Outskirts", 1260077: "Comodo Cave - Northeast",
    1260078: "Ant Hell", 1260079: "Goblin Village", 1260080: "Orc Village",
    1260081: "Prontera Culvert", 1260082: "Outer Morroc Fields", 1260083: "Middle Undersea Cave",
    1260084: "Outer Geffen Fields", 1260085: "Upper Payon Cave", 1260086: "Comodo Fields 1 & 3",
    1260087: "Pyramid", 1260088: "Southern Mt. Mjolnir", 1260089: "Pharos Lighthouse",
    1260090: "Lutie", 1260091: "Prontera Vicinity", 1260092: "Morroc Vicinity",
    1260093: "Lower Undersea Cave", 1260094: "Geffen Dungeon", 1260095: "Lower Payon Cave",
    1260096: "Comodo Cave - Northwest", 1260097: "Sphinx", 1260098: "Northern Mt. Mjolnir",
    1260099: "Mjolnir Dead Pit",
}


if __name__ == "__main__":
    main()
