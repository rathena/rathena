#!/usr/bin/env python3
# [RO Sandbox] Regenerate db/import skill trees and skill data from the
# RO_Skill_Trees Google Sheet (the source of truth). Phase 3: jobs that
# already exist in rAthena, and only data rAthena already reads.
#
# Run from the repo root:
#   python3 tools/sandbox/skills2yaml.py
#   python3 tools/sandbox/skills2yaml.py --client "/path/to/client folder"
#
# Writes:
#   db/import/skill_tree.yml  new max levels, new prerequisites, moved and
#                             removed skills, per job
#   db/import/skill_db.yml    raised max levels, cast time, after-cast delay
#                             and cooldown that differ from rAthena
#   db/import/job_stats.yml   max job level where a tab's "Skill points
#                             available" differs from rAthena (Novice: 25)
# With --client, also writes the client's skillinfolist.lub, skilltreeview.lub
# and skilldescript.lub into its data folder (data\luafiles514\lua files\
# skillinfoz and skillinfozss), edited from the copies in its GRFs, so the
# skill window shows the same trees, prerequisites and max levels. Delete
# those files to go back to the GRF versions.
#
# Rules:
#   - Rows whose rAthena Code isn't an rAthena skill are new skills (Phase 6)
#     and are skipped. Their names still count when other rows name them.
#   - If a skill's New Prerequisites name a skill that doesn't exist yet, the
#     skill keeps its classic prerequisites until it does.
#   - New Max Lv 0, or a Move To another job, takes the skill off that job.
#     If the job inherits the skill from a parent job (Novice, Mage...), its
#     own entry copies the parent's instead, so inheritance doesn't drop it.
#   - Classic skills a tab doesn't list stay until their replacement exists.
#   - Timings: "-" is 0, blank is no change, "a → b" goes linearly from
#     level 1 to the top level. Costs (MP/SP) wait for Phase 7.
#
# Needs the sheet shared as "Anyone with the link can view".
# Standard library only, so nothing to install.

import argparse
import csv
import io
import os
import re
import sys
import urllib.request
from collections import Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import grf  # noqa: E402  (tools/sandbox/grf.py)

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
JOB_EXP_SOURCE = "db/pre-re/job_exp.yml"
JOB_STATS_OUT = "db/import/job_stats.yml"
FIRST_JOB_EXP = "Swordman"  # new job levels borrow the 1st jobs' EXP table

MAX_SKILL_LEVEL = 13  # src/map/skill.hpp

# Client skill data (ROenglishRE plain-text Lua), and its job IDs.
CLIENT_DIRS = ["skillinfoz", "skillinfozss"]
CLIENT_PATH = "data\\luafiles514\\lua files\\{dir}\\{file}"
CLIENT_JOBID = {
    "Novice": "JT_NOVICE", "Swordman": "JT_SWORDMAN", "Mage": "JT_MAGICIAN",
    "Archer": "JT_ARCHER", "Acolyte": "JT_ACOLYTE", "Merchant": "JT_MERCHANT",
    "Thief": "JT_THIEF", "Knight": "JT_KNIGHT", "Priest": "JT_PRIEST",
    "Wizard": "JT_WIZARD", "Blacksmith": "JT_BLACKSMITH", "Hunter": "JT_HUNTER",
    "Assassin": "JT_ASSASSIN", "Crusader": "JT_CRUSADER", "Monk": "JT_MONK",
    "Sage": "JT_SAGE", "Rogue": "JT_ROGUE", "Alchemist": "JT_ALCHEMIST",
    "Bard": "JT_BARD", "Dancer": "JT_DANCER",
}
TIMINGS = [("Cast Time (s)", "CastTime"),
           ("After-Cast Delay (s)", "AfterCastActDelay"),
           ("Cooldown (s)", "Cooldown"),
           ("Duration (s)", "Duration1")]  # optional column; blank keeps rAthena's value

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


def skill_points(rows, tab):
    """The tab's 'Skill points available' number, or None."""
    for row in rows:
        cells = [c.strip() for c in row]
        for i, cell in enumerate(cells):
            if cell.lower().startswith("skill points available"):
                for value in cells[i + 1:]:
                    if value.isdigit():
                        return int(value)
                errors.append(f"{tab}: 'Skill points available' has no number")
                return None
    return None


def read_job_exp():
    """Job -> {"max": max job level, "exp": {level: exp}}"""
    with open(JOB_EXP_SOURCE, encoding="utf-8") as f:
        body = f.read().split("\nBody:", 1)[1]
    out = {}
    for entry in re.split(r"\n  - Jobs:", body)[1:]:
        jobs = re.findall(r"\n      (\w+): true", entry)
        top = re.search(r"\n    MaxJobLevel: (\d+)", entry)
        if not top or "JobExp:" not in entry:
            continue
        exp = {int(lv): int(x) for lv, x in re.findall(
            r"- Level: (\d+)\n        Exp: (\d+)", entry.split("JobExp:", 1)[1])}
        for job in jobs:
            out[job] = {"max": int(top.group(1)), "exp": exp}
    return out


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
                elif key in ("CastTime", "AfterCastActDelay", "Cooldown", "Duration1"):
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
    parser = argparse.ArgumentParser(description="Regenerate skill import files from the sheet.")
    parser.add_argument("--client", help="client folder; also writes its skill window files")
    args = parser.parse_args()
    if args.client:
        client_sources = read_client_sources(args.client)

    ra_skills = read_rathena_skills()
    ra_trees = read_rathena_trees()
    tab_of_job = {job: tab for tab, (_, job) in TABS.items()}
    for tab, (_, job) in TABS.items():
        if job not in ra_trees:
            sys.exit(f"Job {job} (tab {tab}) is not in {SKILL_TREE_SOURCE}")

    # Read every tab, and every skill name -> code (new skills included).
    tabs, names, points = {}, {}, {}
    for tab in TABS:
        rows = []
        raw = fetch_tab(tab)
        points[tab] = skill_points(raw, tab)
        for rownum, row in table(raw, tab):
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
                    value = parse_timing(row.get(column, ""))
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

    # Max job level: a tab's "Skill points available" is the job's level cap
    # (rAthena gives Job Lv - 1 points; the extra point at Job Lv 1 needs
    # source). Levels the job's EXP table lacks borrow the 1st jobs' table.
    ra_job_exp = read_job_exp()
    job_entries, job_levels = [], []
    for tab, (_, job) in TABS.items():
        cap = points.get(tab)
        if cap is None or job not in ra_job_exp or cap == ra_job_exp[job]["max"]:
            continue
        old, borrow = ra_job_exp[job], ra_job_exp[FIRST_JOB_EXP]["exp"]
        exp = {lv: old["exp"][lv] if lv < old["max"] else borrow.get(lv)
               for lv in range(1, cap)}
        if None in exp.values():
            sys.exit(f"{tab}: no EXP values for job levels up to {cap}")
        lines = ["  - Jobs:", f"      {job}: true", f"    MaxJobLevel: {cap}", "    JobExp:"]
        for lv, x in exp.items():
            lines += [f"      - Level: {lv}", f"        Exp: {x}"]
        job_entries.append(lines)
        job_levels.append(f"{job} max job level {old['max']} -> {cap}")

    write_import(SKILL_TREE_OUT, "SKILL_TREE_DB", 1, tree_entries)
    write_import(SKILL_DB_OUT, "SKILL_DB", 4, db_entries)
    write_import(JOB_STATS_OUT, "JOB_STATS", 4, job_entries)

    print(f"{SKILL_TREE_OUT}: {len(tree_entries)} jobs")
    for job, n in counts.items():
        if any(n.values()):
            print(f"  {job}: {n['changed']} changed, {n['added']} added, "
                  f"{n['removed']} removed")
    print(f"{SKILL_DB_OUT}: {len(db_entries)} skills")
    print(f"{JOB_STATS_OUT}: {', '.join(job_levels) or 'no job level changes'}")
    print(f"Skipped {n_new} rows for new skills (Phase 6).")
    for title, items in (("Kept classic prerequisites", waits),
                         ("Copied from a parent job", mirrored),
                         ("Max level raised (check source formulas)", raised),
                         ("Timing changes", timing_changes)):
        if items:
            print(f"{title}:")
            for item in items:
                print("  -", item)

    if args.client:
        jobs = [job for _, job in TABS.values()]
        top = {code: lv for code, lv in top_level.items()
               if lv > ra_skills[code]["MaxLevel"]}
        write_client(args.client, client_sources, jobs, ra_skills, ra_trees,
                     finals, desired, leaves, top)


# ---------- client skill window files ----------

def read_client_sources(client_dir):
    """{(dir, file): text} from the client's GRFs, in DATA.INI order."""
    ini = os.path.join(client_dir, "DATA.INI")
    if not os.path.exists(ini):
        sys.exit(f"{ini} not found: is --client the client folder?")
    grfs = []
    with open(ini, encoding="latin-1") as f:
        for line in f:
            m = re.match(r"\s*(\d+)\s*=\s*(\S.*\.grf)\s*$", line, re.I)
            if m:
                grfs.append((int(m.group(1)), m.group(2)))
    wanted = {CLIENT_PATH.format(dir=d, file=f): (d, f) for d in CLIENT_DIRS
              for f in ("skillinfolist.lub", "skilltreeview.lub", "skilldescript.lub",
                        "skillid.lub", "jobinheritlist.lub")}
    sources = {}
    for _, name in sorted(grfs):
        path = os.path.join(client_dir, name)
        if not os.path.exists(path):
            continue
        for grf_path, data in grf.read_files(path, [w for w in wanted if wanted[w] not in sources]).items():
            sources[wanted[grf_path]] = data.decode("latin-1")
    if not sources:
        sys.exit("No skillinfolist.lub or skilltreeview.lub found in the client's GRFs")
    for key, text in sources.items():
        if text.lstrip().startswith("\x1bLua"):
            sys.exit(f"{key[0]}/{key[1]} is compiled Lua; only plain-text files can be edited")
    return sources


def strip_strings(line):
    return re.sub(r'"(?:[^"\\]|\\.)*"', '""', line)


def depth_change(line):
    s = strip_strings(line)
    return s.count("{") - s.count("}")


def find_block(lines, start_pattern):
    """(first, last) line index of a table whose first line matches, or None."""
    for i, line in enumerate(lines):
        if re.match(start_pattern, line):
            depth = 0
            for j in range(i, len(lines)):
                depth += depth_change(lines[j])
                if depth == 0:
                    return i, j
    return None


def block_fields(lines):
    """Split the inside of a table into fields: [(name or None, [lines])]."""
    fields, depth, cur = [], 0, None
    for line in lines:
        if depth == 0:
            m = re.match(r"\s*(\w+)\s*=", line)
            cur = (m.group(1) if m else None, [])
            fields.append(cur)
        cur[1].append(line)
        depth += depth_change(line)
    return fields


def with_comma(field_lines):
    last = field_lines[-1].rstrip()
    if not last.endswith(","):
        field_lines = field_lines[:-1] + [last + ","]
    return field_lines


def req_lines(reqs, indent):
    out = []
    for i, (code, level) in enumerate(reqs):
        comma = "," if i < len(reqs) - 1 else ""
        out.append(f"{indent}{{ SKID.{code}, {level} }}{comma}")
    return out


def edit_skillinfo(text, changes, max_levels):
    """changes: code -> (default reqs, {JOBID name: reqs}, JOBID names we manage);
    max_levels: code -> raised max level."""
    lines = text.split("\r\n")
    missing = []
    for code in sorted(set(changes) | set(max_levels)):
        found = find_block(lines, rf"\t\[SKID\.{code}\] = \{{\s*$")
        if not found:
            missing.append(code)
            continue
        first, last = found
        fields = block_fields(lines[first + 1:last])
        out = []
        old_max = None
        for name, flines in fields:
            if name == "MaxLv":
                old_max = int(re.search(r"\d+", flines[0]).group())
        new_max = max_levels.get(code, old_max)
        keep_jobs = []
        for name, flines in fields:
            if code in changes and name in ("_NeedSkillList", "NeedSkillList"):
                if name == "NeedSkillList":
                    keep_jobs = parse_job_reqs("\n".join(flines))
                continue
            if name == "MaxLv" and new_max and new_max > old_max:
                flines = [re.sub(r"\d+", str(new_max), flines[0], count=1)]
            elif (new_max and old_max and new_max > old_max and len(flines) == 1
                  and re.match(r"\s*\w+ = \{[\s\d.,-]+\},?\s*$", flines[0])):
                values = re.findall(r"-?[\d.]+", flines[0].split("=", 1)[1])
                if len(values) == old_max:
                    values += [values[-1]] * (new_max - old_max)
                    indent = re.match(r"\s*", flines[0]).group()
                    fname = flines[0].split("=")[0].strip()
                    flines = [f"{indent}{fname} = {{ {', '.join(values)} }},"]
            out.append(with_comma(flines))
        if code in changes:
            default, per_job, managed = changes[code][:3]
            if default:
                out.append(["\t\t_NeedSkillList = {"] + req_lines(default, "\t\t\t") + ["\t\t},"])
            jobs = dict(per_job)
            for job, reqs in keep_jobs:
                if job not in managed:  # e.g. Super Novice: not ours to change
                    jobs[job] = reqs
            if jobs:
                body = ["\t\tNeedSkillList = {"]
                items = list(jobs.items())
                for i, (job, reqs) in enumerate(items):
                    body.append(f"\t\t\t[JOBID.{job}] = {{")
                    body += req_lines(reqs, "\t\t\t\t")
                    body.append("\t\t\t}" + ("," if i < len(items) - 1 else ""))
                body.append("\t\t},")
                out.append(body)
        flat = [line for f in out for line in f]
        if flat:
            flat[-1] = flat[-1].rstrip(",")
        lines[first + 1:last] = flat
    return "\r\n".join(lines), missing


def parse_job_reqs(text):
    """'[JOBID.JT_X] = { { SKID.A, 1 } }, ...' -> [(job, [(code, level)])]"""
    out, cur = [], None
    for m in re.finditer(r"\[JOBID\.(\w+)\]|SKID\.(\w+)\s*,\s*(\d+)", text):
        if m.group(1):
            cur = (m.group(1), [])
            out.append(cur)
        elif cur:
            cur[1].append((m.group(2), int(m.group(3))))
    return out


def edit_treeview(text, layouts):
    """layouts: JOBID name -> {slot: code}; rewrites those jobs' tabs."""
    lines = text.split("\r\n")
    for job, slots in layouts.items():
        found = find_block(lines, rf"\t\[JOBID\.{job}\] = \{{")
        if not found:
            continue
        first, last = found
        body = []
        items = sorted(slots.items())
        for i, (slot, code) in enumerate(items):
            body.append(f"\t\t[{slot}] = SKID.{code}" + ("," if i < len(items) - 1 else ""))
        closing = lines[last]
        lines[first:last + 1] = [f"\t[JOBID.{job}] = {{"] + body + [closing]
    return "\r\n".join(lines)


def read_treeview(text):
    """JOBID name -> {slot: code}"""
    lines = text.split("\r\n")
    out = {}
    for i, line in enumerate(lines):
        m = re.match(r"\t\[JOBID\.(\w+)\] = \{", line)
        if not m:
            continue
        found = find_block(lines[i:], r"\t\[JOBID\.")
        block = "\n".join(lines[i:i + found[1] + 1])
        out[m.group(1)] = {int(s): c for s, c in re.findall(r"\[(\d+)\]\s*=\s*SKID\.(\w+)", block)}
    return out


def write_client(client_dir, sources, jobs, ra_skills, ra_trees, finals, desired,
                 leaves, top):
    def effective(own_of, job, code):
        entry = own_of(job).get(code)
        if entry is None:
            for parent in ra_trees[job]["inherit"]:
                if code in own_of(parent):
                    entry = own_of(parent)[code]
        return entry if entry and entry["max"] > 0 else None

    def classic(job):
        return ra_trees[job]["tree"]

    def new(job):
        return finals.get(job, ra_trees[job]["tree"])

    # Skills whose tree entry changed for any job, and each job's prerequisites.
    codes = set()
    for job in jobs:
        codes |= set(classic(job)) | set(new(job))
        for parent in ra_trees[job]["inherit"]:
            codes |= set(classic(parent)) | set(new(parent))
    changes = {}
    for code in sorted(codes):
        before = [effective(classic, j, code) for j in jobs]
        after = [effective(new, j, code) for j in jobs]
        if before == after:
            continue
        per_job = {j: tuple(e["req"].items()) for j, e in zip(jobs, after) if e}
        having = [jid for job in per_job for jid in job_ids(job)]
        if not per_job:
            changes[code] = ([], {}, managed_ids(jobs), having)
            continue
        counts = Counter(per_job.values())
        default = max(counts, key=lambda r: (counts[r], -list(per_job.values()).index(r)))
        others = {}
        for job, reqs in per_job.items():
            if reqs != default:
                for jid in job_ids(job):
                    others[jid] = list(reqs)
        changes[code] = (list(default), others, managed_ids(jobs), having)
    max_levels = top

    # Each job's tab: drop skills it lost or that now show on a parent's tab,
    # add the ones its sheet tab lists, in free slots. The client draws Novice
    # on the same grid as each 1st job, so their new skills avoid each
    # other's slots (only Phase 3 jobs are counted; Super Novice, Gunslinger,
    # Ninja and Taekwon share that grid too and may overlap until their phase).
    def layouts_for(treeview):
        current = read_treeview(treeview)
        kept = {}
        for job in jobs:
            slots = dict(current.get(CLIENT_JOBID[job], {}))
            parents = ra_trees[job]["inherit"]
            for slot, code in list(slots.items()):
                shown_upstream = (code not in desired[job]
                                  and any(code in desired.get(p, {}) for p in parents))
                if (effective(new, job, code) is None or code in leaves[job]
                        or shown_upstream):
                    del slots[slot]
            kept[job] = slots
        first_jobs = [j for j in jobs if ra_trees[j]["inherit"] == ["Novice"]]
        order = ["Novice"] + [j for j in jobs if j != "Novice"]
        result = {}
        for job in order:
            slots = kept[job]
            if job == "Novice":
                reserved = set().union(*(kept[j] for j in first_jobs))
            elif job in first_jobs:
                reserved = set(kept["Novice"])
            else:
                reserved = set()
            for code in desired[job]:
                if code not in slots.values():
                    slot = next(s for s in range(200) if s not in slots and s not in reserved)
                    slots[slot] = code
            jid = CLIENT_JOBID[job]
            if slots != current.get(jid, {}):
                result[jid] = slots
        return result

    # The client names a few skills differently (BA_FROSTJOKER is BA_FROSTJOKE),
    # so names go through the numeric skill ID in its skillid.lub.
    ra_by_id = {s["Id"]: code for code, s in ra_skills.items()}

    def namers(folder):
        text = sources.get((folder, "skillid.lub"), "")
        client_by_id = {int(i): n for n, i in re.findall(r"^\s*(\w+)\s*=\s*(\d+)", text, re.M)}
        to_client = {code: client_by_id.get(s["Id"], code) for code, s in ra_skills.items()}
        to_ra = {n: ra_by_id.get(i, n) for i, n in client_by_id.items()}
        return (lambda c: to_client.get(c, c)), (lambda c: to_ra.get(c, c))

    def reqs_to(conv, reqs):
        return [(conv(r), lv) for r, lv in reqs]

    written = []
    for (folder, name), text in sorted(sources.items()):
        to_client, to_ra = namers(folder)
        client_changes = {
            to_client(code): (reqs_to(to_client, default),
                              {j: reqs_to(to_client, r) for j, r in others.items()},
                              managed, having)
            for code, (default, others, managed, having) in changes.items()}
        client_max = {to_client(code): lv for code, lv in max_levels.items()}
        if name == "skilldescript.lub":
            jobid_text = sources.get((folder, "jobinheritlist.lub"), "")
            job_numbers = {j: int(n) for j, n in re.findall(r"(JT_\w+)\s*=\s*(\d+)", jobid_text)}
            new_text = edit_descript(text, client_changes, client_max, job_numbers)
        elif name == "skillinfolist.lub":
            new_text, missing = edit_skillinfo(text, client_changes, client_max)
            for code in missing:
                print(f"  note: {code} is not in the client's {folder}/{name}")
        elif name == "skilltreeview.lub":
            ra_text = re.sub(r"SKID\.(\w+)", lambda m: "SKID." + to_ra(m.group(1)), text)
            layouts = {jid: {slot: to_client(code) for slot, code in slots.items()}
                       for jid, slots in layouts_for(ra_text).items()}
            new_text = edit_treeview(text, layouts)
        else:
            continue
        out = os.path.join(client_dir, *CLIENT_PATH.format(dir=folder, file=name).split("\\"))
        os.makedirs(os.path.dirname(out), exist_ok=True)
        with open(out, "w", encoding="latin-1", newline="") as f:
            f.write(new_text)
        written.append(out)
    print(f"Client: {len(changes)} skills with new prerequisites or trees")
    for path in written:
        print(f"  {path}")


def edit_descript(text, changes, max_levels, job_numbers):
    """Rewrite the "Max Level" and "Requirement" lines of skill descriptions.

    A requirement line may start with a job ID ("14^CC3399Requirement: ...")
    and then shows only to that job. Lines for jobs this tool doesn't manage
    (Super Novice...) are kept; if any are, or jobs need different
    requirements, the new lines get job IDs too."""
    lines = text.split("\r\n")
    names = {}
    for i, line in enumerate(lines[:-1]):
        m = re.match(r"\t\[SKID\.(\w+)\] = \{", line)
        n = re.match(r'\s*"(.*?)",?\s*$', lines[i + 1]) if m else None
        if n:
            names[m.group(1)] = n.group(1)

    def req_text(reqs):
        return "Requirement: " + ", ".join(f"{names.get(c, c)} {lv}" for c, lv in reqs)

    for code in sorted(set(changes) | set(max_levels)):
        found = find_block(lines, rf"\t\[SKID\.{code}\] = \{{\s*$")
        if not found:
            continue
        first, last = found
        body = lines[first + 1:last]
        if code in max_levels:
            body = [re.sub(r'"Max Level: \d+', f'"Max Level: {max_levels[code]}', b) for b in body]
        if code in changes:
            default, others, managed, having = changes[code]
            managed_nums = {job_numbers[j] for j in managed if j in job_numbers}
            out, kept_prefixed, insert_at, i = [], False, None, 0
            while i < len(body):
                m = re.match(r'\s*"(\d*)\^\w{6}Requirement:', body[i])
                if not m:
                    out.append(body[i])
                    i += 1
                    continue
                j = i
                while "^000000" not in body[j] and j < len(body) - 1:
                    j += 1
                if m.group(1) and int(m.group(1)) not in managed_nums:
                    out += body[i:j + 1]
                    kept_prefixed = True
                elif insert_at is None:
                    insert_at = len(out)
                i = j + 1
            if insert_at is None:
                insert_at = next((k + 1 for k, b in enumerate(out) if '"Max Level:' in b), 1)
            new = []
            if others or kept_prefixed:
                for jid in having:
                    reqs = others.get(jid, default)
                    if reqs and jid in job_numbers:
                        new.append(f'\t\t"{job_numbers[jid]}^CC3399{req_text(reqs)}^000000",')
            elif default:
                new.append(f'\t\t"^CC3399{req_text(default)}^000000",')
            out[insert_at:insert_at] = new
            body = out
        lines[first + 1:last] = body
    return "\r\n".join(lines)


def job_ids(job):
    base = CLIENT_JOBID[job]
    return [base, base + "_H", base + "_B"]


def managed_ids(jobs):
    return {jid for job in jobs for jid in job_ids(job)}


def write_import(path, header_type, version, entries):
    with open(path, encoding="utf-8") as f:
        keep = f.read()
    # Keep rAthena's file as it is (job_stats.yml has 136k lines of commented
    # examples after its Header) and replace only our block at the end.
    keep = keep.split("\n# [RO Sandbox] Generated")[0].rstrip("\n")
    out = [keep, "", "# [RO Sandbox] Generated by tools/sandbox/skills2yaml.py "
           "from the RO_Skill_Trees sheet.", "# Don't edit by hand: "
           "change the sheet and run the script again.", ""]
    if not re.search(r"^Header:", keep, re.M):
        out += ["Header:", f"  Type: {header_type}", f"  Version: {version}", ""]
    if entries:
        out.append("Body:")
        for e in entries:
            out += e
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
