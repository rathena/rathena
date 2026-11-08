#!/usr/bin/env python3
# [RO Sandbox] Regenerate db/import skill trees and skill data from the
# RO_Skill_Trees Google Sheet (the source of truth). Phase 3: jobs that
# already exist in rAthena, and only data rAthena already reads.
#
# Run from the repo root:
#   python3 tools/sandbox/skills2yaml.py
#
# Writes:
#   db/import/skill_tree.yml  new max levels, new prerequisites, moved and
#                             removed skills, per job
#   db/import/skill_db.yml    raised max levels, cast time, after-cast delay
#                             and cooldown that differ from rAthena
#
# Rules:
#   - Rows whose rAthena Code isn't an rAthena skill are new skills (Phase 8)
#     and are skipped. Their names still count when other rows name them.
#   - If a skill's New Prerequisites name a skill that doesn't exist yet, the
#     skill keeps its classic prerequisites until it does.
#   - New Max Lv 0, or a Move To another job, takes the skill off that job.
#     If the job inherits the skill from a parent job (Novice, Mage...), its
#     own entry copies the parent's instead, so inheritance doesn't drop it.
#   - Classic skills a tab doesn't list stay until their replacement exists.
#   - Timings: "-" is 0, blank is no change, "a → b" goes linearly from
#     level 1 to the top level. Costs (MP/SP) wait for Phase 6.
#
# Needs the sheet shared as "Anyone with the link can view".
# Standard library only, so nothing to install.

import csv
import io
import re
import sys
import urllib.request

SHEET_ID = "1hZjgxrIMtovbU2v_QkTTRp4lYQNNunaTGAuN0HEa9RE"
# Sheet tab -> (gid, rAthena job name), in tree order.
TABS = {
    "Novice": (594664322, "Novice"),
    "Swordsman": (810271896, "Swordman"),
    "Marksman": (1293747392, "Archer"),
    "Acolyte": (213601925, "Acolyte"),
    "Mage": (281602313, "Mage"),
    "Thief": (1523786416, "Thief"),
    "Merchant": (1007296472, "Merchant"),
    "Knight": (2001, "Knight"),
    "Crusader": (2002, "Crusader"),
    "Wizard": (2003, "Wizard"),
    "Sage": (2004, "Sage"),
    "Hunter": (2005, "Hunter"),
    "Bard": (2006, "Bard"),
    "Dancer": (2007, "Dancer"),
    "Priest": (2009, "Priest"),
    "Monk": (2010, "Monk"),
    "Blacksmith": (2011, "Blacksmith"),
    "Alchemist": (2012, "Alchemist"),
    "Assassin": (2013, "Assassin"),
    "Rogue": (2014, "Rogue"),
}

SKILL_DB_SOURCE = "db/pre-re/skill_db.yml"
SKILL_TREE_SOURCE = "db/pre-re/skill_tree.yml"
SKILL_DB_OUT = "db/import/skill_db.yml"
SKILL_TREE_OUT = "db/import/skill_tree.yml"

MAX_SKILL_LEVEL = 13  # src/map/skill.hpp
TIMINGS = [("Cast Time (s)", "CastTime"),
           ("After-Cast Delay (s)", "AfterCastActDelay"),
           ("Cooldown (s)", "Cooldown")]

errors = []


def fetch_tab(name):
    url = (f"https://docs.google.com/spreadsheets/d/{SHEET_ID}"
           f"/export?format=csv&gid={TABS[name][0]}")
    with urllib.request.urlopen(url) as resp:
        text = resp.read().decode("utf-8")
    if text.lstrip().startswith("<"):
        sys.exit(f"Got a web page instead of CSV for {name}. "
                 "Is the sheet shared as 'Anyone with the link'?")
    return list(csv.reader(io.StringIO(text)))


def table(rows, tab):
    """Data rows below the header, as (sheet row number, dict by header name)."""
    for i, row in enumerate(rows):
        if "rAthena Code" in [c.strip() for c in row]:
            header = [h.strip() for h in row]
            return [(i + 1 + n + 1, dict(zip(header, r + [""] * len(header))))
                    for n, r in enumerate(rows[i + 1:])]
    sys.exit(f"{tab}: header row with 'rAthena Code' not found")


def norm(name):
    return " ".join(name.replace("’", "'").lower().split())


# ---------- rAthena data (simple line parser for its YAML layout) ----------

def read_rathena_skills():
    """Name -> {"Id", "MaxLevel", "CastTime": {level: ms} or ms, ...}"""
    skills, cur, field = {}, None, None
    with open(SKILL_DB_SOURCE, encoding="utf-8") as f:
        for line in f:
            m = re.match(r"  - Id: (\d+)", line)
            if m:
                cur, field = {"Id": int(m.group(1))}, None
                continue
            if cur is None:
                continue
            m = re.match(r"    (\w+):\s*(.*?)\s*$", line)
            if m:
                key, val = m.groups()
                field = None
                if key == "Name":
                    skills[val] = cur
                elif key == "MaxLevel":
                    cur["MaxLevel"] = int(val)
                elif key in ("CastTime", "AfterCastActDelay", "Cooldown"):
                    if val:
                        cur[key] = int(val)
                    else:
                        cur[key], field = {}, key
                continue
            if field:
                m = re.match(r"      - Level: (\d+)", line)
                if m:
                    level = int(m.group(1))
                    continue
                m = re.match(r"        Time: (-?\d+)", line)
                if m:
                    cur[field][level] = int(m.group(1))
    return skills


def read_rathena_trees():
    """Job -> {"inherit": [jobs], "tree": {skill: {"max": n, "req": {skill: lv}}}}"""
    trees, job, entry, section = {}, None, None, None
    with open(SKILL_TREE_SOURCE, encoding="utf-8") as f:
        for line in f:
            m = re.match(r"  - Job: (\S+)", line)
            if m:
                job = {"inherit": [], "tree": {}}
                trees[m.group(1)] = job
                entry = section = None
                continue
            if job is None:
                continue
            m = re.match(r"    (Inherit|Tree):", line)
            if m:
                section = m.group(1)
                continue
            if section == "Inherit":
                m = re.match(r"      (\S+): true", line)
                if m:
                    job["inherit"].append(m.group(1))
                continue
            m = re.match(r"      - Name: (\S+)", line)
            if m:
                entry = {"max": 0, "req": {}}
                job["tree"][m.group(1)] = entry
                continue
            m = re.match(r"        MaxLevel: (\d+)", line)
            if m and entry is not None:
                entry["max"] = int(m.group(1))
                continue
            m = re.match(r"          - Name: (\S+)", line)
            if m and entry is not None:
                req = m.group(1)
                continue
            m = re.match(r"            Level: (\d+)", line)
            if m and entry is not None:
                entry["req"][req] = int(m.group(1))
    return trees


def per_level(value):
    """rAthena's per-level array, extended the way SkillDatabase::parseNode does."""
    if value is None:
        return [0] * MAX_SKILL_LEVEL
    if isinstance(value, int):
        return [value] * MAX_SKILL_LEVEL
    arr = [0] * MAX_SKILL_LEVEL
    for level, v in value.items():
        arr[level - 1] = v
    top = max(value)
    for step in range(1, top // 2 + 1):
        diff = arr[top - 1] - arr[top - step - 1]
        if any(arr[j] - arr[j - step] != diff for j in range(top - 1, step - 1, -1)):
            continue
        for i in range(top, MAX_SKILL_LEVEL):
            arr[i] = arr[i - step] + diff
            if arr[i] < 1 and arr[i - 1] >= 0:
                arr[i], diff, step = 1, 0, 1
        return arr
    return arr[:top] + [arr[top - 1]] * (MAX_SKILL_LEVEL - top)


# ---------- sheet values ----------

def seconds_to_ms(text):
    return int(round(float(text) * 1000))


def parse_timing(text):
    """'' -> None (no change), '-' -> 0, '1.5' -> 1500, '2 → 0.65' -> (2000, 650)"""
    t = text.strip()
    if t == "":
        return None
    if t == "-":
        return 0
    if re.fullmatch(r"\d+(\.\d+)?", t):
        return seconds_to_ms(t)
    m = re.fullmatch(r"(\d+(?:\.\d+)?)\s*(?:→|->)\s*(\d+(?:\.\d+)?)", t)
    if m:
        return (seconds_to_ms(m.group(1)), seconds_to_ms(m.group(2)))
    raise ValueError(t)


def timing_levels(value, top):
    if isinstance(value, int):
        return [value] * top
    first, last = value
    if top == 1:
        return [first]
    return [round(first + (last - first) * (lv - 1) / (top - 1)) for lv in range(1, top + 1)]


def parse_prereqs(text, names, where):
    """'Bash 5, Magnum Break 3' -> [(code, 5), ...]; None on a problem (logged)."""
    t = text.strip()
    if t == "":
        errors.append(f"{where}: New Prerequisites is empty (write None if it has none)")
        return None
    if norm(t) == "none":
        return []
    out = []
    for part in t.split(","):
        part = part.strip()
        m = re.fullmatch(r"(.*\S)\s+(\d+)", part)
        if not m:
            errors.append(f"{where}: prerequisite '{part}' has no level")
            return None
        name, level = m.group(1), int(m.group(2))
        code = names.get(norm(name))
        if code is None and re.fullmatch(r"[A-Z]+_[A-Z0-9_]+", name):
            code = name
        if code is None:
            errors.append(f"{where}: prerequisite '{name}' is not a skill on any "
                          "tab (missing comma or typo?)")
            return None
        out.append((code, level))
    return out


# ---------- building ----------

def main():
    ra_skills = read_rathena_skills()
    ra_trees = read_rathena_trees()
    tab_of_job = {job: tab for tab, (_, job) in TABS.items()}
    for tab, (_, job) in TABS.items():
        if job not in ra_trees:
            sys.exit(f"Job {job} (tab {tab}) is not in {SKILL_TREE_SOURCE}")

    # Read every tab, and every skill name -> code (new skills included).
    tabs, names = {}, {}
    for tab in TABS:
        rows = []
        for rownum, row in table(fetch_tab(tab), tab):
            code, name = row["rAthena Code"].strip(), row["Skill"].strip()
            if not code:
                continue
            rows.append((rownum, name, code, row))
            if norm(name) in names and names[norm(name)] != code:
                errors.append(f"{tab} row {rownum}: '{name}' is {code} here but "
                              f"{names[norm(name)]} on another tab")
            names[norm(name)] = code
        tabs[tab] = rows
    listed = {tab: {code for _, _, code, _ in rows} for tab, rows in tabs.items()}

    # What each job should have, from its tab and from Move To on other tabs.
    desired = {job: {} for _, job in TABS.values()}
    leaves = {job: set() for _, job in TABS.values()}
    timings = {}   # code -> {field: (value, where)}
    top_level = {}  # code -> highest New Max Lv
    n_new = 0
    for tab, rows in tabs.items():
        job = TABS[tab][1]
        for rownum, name, code, row in rows:
            where = f"{tab} row {rownum} ({name})"
            if code not in ra_skills:
                n_new += 1
                continue
            new_max = row["New Max Lv"].strip()
            if not new_max.isdigit():
                errors.append(f"{where}: New Max Lv is '{new_max}', needs a number")
                continue
            new_max = int(new_max)
            if new_max > MAX_SKILL_LEVEL:
                errors.append(f"{where}: New Max Lv {new_max} is above rAthena's "
                              f"limit of {MAX_SKILL_LEVEL}")
                continue

            move = row["Move To"].strip()
            if new_max == 0:
                leaves[job].add(code)
                continue
            dest = job
            if move and norm(move) != norm(tab):
                match = [t for t in TABS if norm(t) == norm(move)]
                if not match:
                    errors.append(f"{where}: Move To '{move}' is not a Phase 3 job tab")
                    continue
                leaves[job].add(code)
                if code in listed[match[0]]:
                    continue  # the destination tab's own row describes it
                dest = TABS[match[0]][1]
            prereqs = parse_prereqs(row["New Prerequisites"], names, where)
            if prereqs is None:
                continue
            if code in desired[dest]:
                errors.append(f"{where}: {code} is listed twice for {dest}")
                continue
            desired[dest][code] = {"max": new_max, "req": prereqs, "where": where}
            top_level[code] = max(top_level.get(code, 0), new_max)

            # Timings come only from rows that define the skill somewhere.
            for column, field in TIMINGS:
                try:
                    value = parse_timing(row[column])
                except ValueError:
                    errors.append(f"{where}: {column} '{row[column].strip()}' should be "
                                  "a number of seconds, '-', or 'a → b'")
                    continue
                if value is None:
                    continue
                seen = timings.setdefault(code, {})
                if field in seen and seen[field][0] != value:
                    errors.append(f"{where}: {column} '{row[column].strip()}' differs "
                                  f"from {seen[field][1]}")
                seen.setdefault(field, (value, where))

    # Required levels must fit the required skill.
    for job, skills in desired.items():
        for code, d in skills.items():
            for req, level in d["req"]:
                if req in ra_skills:
                    cap = max(top_level.get(req, 0), ra_skills[req]["MaxLevel"])
                    if level > cap:
                        errors.append(f"{d['where']}: needs {req} {level}, "
                                      f"but its max level is {cap}")

    if errors:
        print("Not written. Fix these on the sheet first:")
        for e in errors:
            print("  -", e)
        sys.exit(1)

    def classic_req(job, code):
        for j in [job] + ra_trees[job]["inherit"]:
            if code in ra_trees[j]["tree"]:
                return dict(ra_trees[j]["tree"][code]["req"])
        for t in ra_trees.values():
            if code in t["tree"]:
                return dict(t["tree"][code]["req"])
        return {}

    # Final own tree per job, following the loader's inheritance rules.
    finals, waits, mirrored = {}, [], []

    def final_tree(job):
        if job in finals:
            return finals[job]
        own = ra_trees[job]["tree"]
        final = {code: {"max": e["max"], "req": dict(e["req"])} for code, e in own.items()}
        for code, d in desired.get(job, {}).items():
            missing = [r for r, _ in d["req"] if r not in ra_skills]
            if missing:
                req = classic_req(job, code)
                waits.append(f"{job} {code}: waits for "
                             + ", ".join(sorted(set(missing))))
            else:
                req = dict(d["req"])
            final[code] = {"max": d["max"], "req": req}

        def from_parent(code):
            got = None
            for parent in ra_trees[job]["inherit"]:
                entry = final_tree(parent).get(code)
                if entry is not None:
                    got = entry  # the last parent wins, as in the loader
            return got if got and got["max"] > 0 else None

        for code in list(final):
            in_desired = code in desired.get(job, {})
            if code in leaves.get(job, set()) and not in_desired:
                parent = from_parent(code)
                if parent:
                    final[code] = {"max": parent["max"], "req": dict(parent["req"])}
                    mirrored.append(f"{job} {code}")
                else:
                    final[code] = {"max": 0, "req": {}}
            elif code in own and not in_desired:
                # Own classic copy would hide a parent's changed one.
                parent = from_parent(code)
                changed_upstream = any(code in desired.get(p, {})
                                       for p in ra_trees[job]["inherit"])
                if parent and changed_upstream:
                    final[code] = {"max": parent["max"], "req": dict(parent["req"])}
                    mirrored.append(f"{job} {code}")
        finals[job] = final
        return final

    tree_entries, counts = [], {}
    for tab, (_, job) in TABS.items():
        own = ra_trees[job]["tree"]
        final = final_tree(job)
        lines, n = [], {"changed": 0, "added": 0, "removed": 0}
        for code, e in final.items():
            old = own.get(code)
            if old and old["max"] == e["max"] and old["req"] == e["req"]:
                continue
            lines.append(f"      - Name: {code}")
            lines.append(f"        MaxLevel: {e['max']}")
            if e["max"] == 0:
                n["removed"] += 1
                continue
            n["added" if old is None else "changed"] += 1
            reqs = list(e["req"].items())
            if old:
                reqs += [(r, 0) for r in old["req"] if r not in e["req"]]
            if reqs:
                lines.append("        Requires:")
                for r, lv in reqs:
                    lines.append(f"          - Name: {r}")
                    lines.append(f"            Level: {lv}")
        counts[job] = n
        if lines:
            tree_entries.append([f"  - Job: {job}", "    Tree:"] + lines)

    # skill_db: raised max levels and changed timings.
    db_entries, raised, timing_changes = [], [], []
    for code in sorted(set(top_level) | set(timings), key=lambda c: ra_skills[c]["Id"]):
        ra = ra_skills[code]
        top = max(top_level.get(code, 0), ra["MaxLevel"])
        lines = []
        if top_level.get(code, 0) > ra["MaxLevel"]:
            lines.append(f"    MaxLevel: {top}")
            raised.append(f"{code} {ra['MaxLevel']} -> {top}")
        for field, (value, _) in sorted(timings.get(code, {}).items()):
            old = per_level(ra.get(field))[:top]
            new = timing_levels(value, top)
            if isinstance(value, tuple):
                same = old[0] == new[0] and old[-1] == new[-1]
            else:
                same = old == new
            if same:
                continue
            show = lambda a: f"{a[0]}" if len(set(a)) == 1 else f"{a[0]}..{a[-1]}"
            timing_changes.append(f"{code} {field}: {show(old)} -> {show(new)} ms")
            if len(set(new)) == 1:
                lines.append(f"    {field}: {new[0]}")
            else:
                lines.append(f"    {field}:")
                for lv, ms in enumerate(new, 1):
                    lines += [f"      - Level: {lv}", f"        Time: {ms}"]
        if lines:
            db_entries.append([f"  - Id: {ra['Id']}", f"    Name: {code}"] + lines)

    write_import(SKILL_TREE_OUT, "SKILL_TREE_DB", 1, tree_entries)
    write_import(SKILL_DB_OUT, "SKILL_DB", 4, db_entries)

    print(f"{SKILL_TREE_OUT}: {len(tree_entries)} jobs")
    for job, n in counts.items():
        if any(n.values()):
            print(f"  {job}: {n['changed']} changed, {n['added']} added, "
                  f"{n['removed']} removed")
    print(f"{SKILL_DB_OUT}: {len(db_entries)} skills")
    print(f"Skipped {n_new} rows for new skills (Phase 8).")
    for title, items in (("Kept classic prerequisites", waits),
                         ("Copied from a parent job", mirrored),
                         ("Max level raised (check source formulas)", raised),
                         ("Timing changes", timing_changes)):
        if items:
            print(f"{title}:")
            for item in items:
                print("  -", item)


def write_import(path, header_type, version, entries):
    with open(path, encoding="utf-8") as f:
        keep = f.read()
    # Keep rAthena's comment block, without our note from an earlier run.
    keep = keep.split("\nHeader:")[0].split("\n# [RO Sandbox]")[0].rstrip("\n")
    out = [keep, "", "# [RO Sandbox] Generated by tools/sandbox/skills2yaml.py "
           "from the RO_Skill_Trees sheet.", "# Don't edit by hand: "
           "change the sheet and run the script again.", "",
           "Header:", f"  Type: {header_type}", f"  Version: {version}", ""]
    if entries:
        out.append("Body:")
        for e in entries:
            out += e
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
