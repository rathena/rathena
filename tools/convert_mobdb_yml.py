#! /usr/bin/python

import os
import sys
import re
import shutil
import datetime

MAX_MOB_DROP = 10
MAX_MVP_DROP = 3
MAX_STEAL_DROP = 7
MOB_DB_TXT = "mob_db.txt"
MOB_DB_YAML = "mob_db.yml"
MOB_DROP_TXT = "mob_drop.txt"
MOB_DROP_YAML = "mob_drop.yml"
ITEM_DB_TXT = "item_db.txt"
item_db = {}

def read_item_db(path):
    """Read the item databases from path, fill item database"""
    global item_db
    comma_split = re.compile(",")
    itemdbtxt = path + ITEM_DB_TXT

    with open(itemdbtxt, "r") as db:
        for line in db:
            entry = {}
            line = line.strip()
            if len(line) < 5:
                continue
            if line[:2] == "//":
                continue
            fields = [a.strip() for a in comma_split.split(line)]
            if len(fields) < 22:
                continue
            if fields[0] == 'ID':
                continue
            item_db[fields[0]] = fields[1]

def read_mob_db(path):
    """Read the databases from path, return a database"""
    comma_split = re.compile(",")
    flen = 31 + 2 * (MAX_MOB_DROP + MAX_MVP_DROP)
    mobdbtxt = path + MOB_DB_TXT
    mobdroptxt = path + MOB_DROP_TXT
    is_import = "import" in mobdbtxt

    mobdb = []

    with open(mobdbtxt, "r") as db:
        for line in db:
            entry = {}
            line = line.strip()
            if len(line) < 5:
                continue
            if line[:2] == "//":
                entry['comment'] = 1
                line = line[2:]
            line = line.partition("//")[0].strip()
            fields = [a.strip() for a in comma_split.split(line)]
            if len(fields) != flen:
                continue
            if fields[0] == 'ID':
                continue
            if is_import:
                entry['overwrite'] = True
            entry['mob_id'] = int(fields[0])
            entry['sprite_name'] = fields[1]
            entry['kROName'] = fields[2]
            entry['iROName'] = fields[3]
            entry['LV'] = fields[4]
            entry['HP'] = fields[5]
            entry['SP'] = fields[6]
            entry['EXP'] = fields[7]
            entry['JEXP'] = fields[8]
            entry['Range'] = fields[9] + "~" + fields[20] + "~" + fields[21]
            entry['ATK'] = fields[10] + "~"
            if fields[11] != "":
                entry['ATK'] += fields[11]
            else:
                entry['ATK'] += fields[10]
            entry['DEF'] = fields[12]
            entry['MDEF'] = fields[13]
            entry['Stats'] = {}
            entry['Stats']['str'] = fields[14]
            entry['Stats']['agi'] = fields[15]
            entry['Stats']['vit'] = fields[16]
            entry['Stats']['int'] = fields[17]
            entry['Stats']['dex'] = fields[18]
            entry['Stats']['luk'] = fields[19]
            entry['Size'] = fields[22]
            entry['Race'] = fields[23]
            entry['Element'] = fields[24]
            entry['Mode'] = fields[25]
            entry['Speed'] = fields[26]
            entry['aDelay'] = fields[27]
            entry['aMotion'] = fields[28]
            entry['dMotion'] = fields[29]
            entry['MEXP'] = fields[30]
            entry['drops'] = []
            entry['mvp_drops'] = []
            for i in range(MAX_MVP_DROP):
                idx = 31 + (i * 2)
                if fields[idx] == '0':
                    continue
                it = {}
                it['nameid'] = fields[idx]
                it['rate'] = fields[idx + 1]
                entry['mvp_drops'].append(it)
            for i in range(MAX_MOB_DROP):
                idx = 31 + (i + MAX_MVP_DROP) * 2
                if fields[idx] == '0':
                    continue
                it = {}
                it['nameid'] = fields[idx]
                it['rate'] = fields[idx + 1]
                if i >= MAX_STEAL_DROP:
                    it['steal_protected'] = True
                entry['drops'].append(it)
            # print(entry)
            mobdb.append(entry)

    with open(mobdroptxt, "r") as db:
        for line in db:
            mob_comment = 0
            line = line.strip()
            drop = {}
            if len(line) < 5:
                continue
            if line[:2] == "//":
                mob_comment = 1
                line = line[2:]
            line = line.partition("//")[0]
            fields = [a.strip() for a in comma_split.split(line)]
            if len(fields) < 3 or len(fields) > 5:
                continue
            if fields[0] == '<mobid>':
                continue
            mob_id = int(fields[0])
            drop['nameid'] = fields[1]
            drop['rate'] = fields[2]
            drop['rog'] = fields[3] if len(fields) > 3 else "RDMOPTG_None"
            flag = int(fields[4]) if len(fields) > 4 else 0
            drop['steal_protected'] = flag & 0x1
            mob = None
            for mobe in mobdb:
                if mobe['mob_id'] != mob_id:
                    continue
                if mobe.has_key('comment') != mob_comment:
                    continue
                mob = mobe
            if mob is None:
                mob = {}
                mob['mob_id'] = mob_id
                mob['mvp_drops'] = []
                mob['drops'] = []
                mobdb.append(mob)

            if flag & 0x2:
                drops = mob['mvp_drops']
            else:
                drops = mob['drops']
            drop['replace'] = True
            drops.append(drop)
    return mobdb


def write_drop(md, drop, comment):
    md.write(comment + "      -\n")
    if drop['nameid'] in item_db:
        md.write(comment + "        AegisName: " + item_db[drop['nameid']] + "\n")
    else:
        md.write(comment + "        ItemId: " + drop['nameid'] + "\n")
    md.write(comment + "        Rate: " + drop['rate'] + "\n")
    if drop.has_key('steal_protected') and drop['steal_protected']:
        md.write(comment + "        StealProtected: true\n")
    if drop.has_key('rog') and drop['rog'] != "RDMOPTG_None":
        md.write(comment + '        RandomOptionGroup: {0}\n'.format(drop['rog']))
    if drop.has_key('replace') and drop['replace']:
        md.write(comment + "        Replace: true\n")

def convert_size(size):
    sizes = ["SZ_SMALL", "SZ_MEDIUM", "SZ_LARGE"]
    return sizes[int(size)]

def convert_race(race):
    races = ["FORMLESS", "UNDEAD", "BRUTE", "PLANT", "INSECT", "FISH", "DEMON", "DEMIHUMAN", "ANGEL", "DRAGON", "PLAYER"]
    return "RC_" + races[int(race)]

def convert_element(element):
    eles = ["NEUTRAL", "WATER" , "EARTH" , "FIRE" , "WIND" , "POISON" , "HOLY" , "DARK" , "GHOST" , "UNDEAD"]
    return "ELE_" + eles[int(element) % 20] + " " + str(int(element) // 20)

def write_mode(md, mode, comment):
    modes = ["MD_CANMOVE", "MD_LOOTER", "MD_AGGRESSIVE", "MD_ASSIST", "MD_CASTSENSOR_IDLE"]
    modes.extend(["MD_NORANDOM_WALK", "MD_NOCAST_SKILL", "MD_CANATTACK", "MD_NONE"])
    modes.extend(["MD_CASTSENSOR_CHASE", "MD_CHANGECHASE", "MD_ANGRY", "MD_CHANGETARGET_MELEE"])
    modes.extend(["MD_CHANGETARGET_CHASE", "MD_TARGETWEAK", "MD_RANDOMTARGET", "MD_IGNOREMELEE"])
    modes.extend(["MD_IGNOREMAGIC", "MD_IGNORERANGED", "MD_MVP", "MD_IGNOREMISC"])
    modes.extend(["MD_KNOCKBACK_IMMUNE", "MD_TELEPORT_BLOCK", "MD_NONE", "MD_FIXED_ITEMDROP"])
    modes.extend(["MD_DETECTOR", "MD_STATUS_IMMUNE", "MD_SKILL_IMMUNE"])
    mob = []
    md.write(comment + "    Mode:\n")
    for i, m in enumerate(modes):
        if mode & 1 << i:
            md.write(comment + "        {0}: on\n".format(m))




def write_stats(md, stats, comment):
    md.write(comment + "    Stats:" + "\n")
    md.write(comment + "        Str: " + stats['str'] + "\n")
    md.write(comment + "        Agi: " + stats['agi'] + "\n")
    md.write(comment + "        Vit: " + stats['vit'] + "\n")
    md.write(comment + "        Int: " + stats['int'] + "\n")
    md.write(comment + "        Dex: " + stats['dex'] + "\n")
    md.write(comment + "        Luk: " + stats['luk'] + "\n")


def write_db(path, mob_db):
    mobdbyaml = path + MOB_DB_YAML

    with open(mobdbyaml, "w") as md:
        md.write(MOBDBHEADER)
        md.write("Mobs:\n")
        for entry in mob_db:
            if entry.has_key('comment'):
                comment = "#"
            else:
                comment = ""
            md.write(comment + "  -\n")
            md.write(comment + "    MobId: " + str(entry['mob_id']) + "\n")
            if entry.has_key('sprite_name'):
                md.write(comment + "    Name: " + entry['iROName'] + "\n")
                if entry['iROName'] != entry['kROName']:
                    md.write(comment + "    kROName: " + entry['kROName'] + "\n")
                md.write(comment + "    Sprite: " + entry['sprite_name'] + "\n")
                md.write(comment + "    LV: " + entry['LV'] + "\n")
                md.write(comment + "    HP: " + entry['HP'] + "\n")
                md.write(comment + "    SP: " + entry['SP'] + "\n")
                md.write(comment + "    EXP: " + entry['EXP'] + "\n")
                md.write(comment + "    JEXP: " + entry['JEXP'] + "\n")
                md.write(comment + "    Range: " + entry['Range'] + "\n")
                md.write(comment + "    ATK: " + entry['ATK'] + "\n")
                md.write(comment + "    DEF: " + entry['DEF'] + "\n")
                md.write(comment + "    MDEF: " + entry['MDEF'] + "\n")
                write_stats(md, entry['Stats'], comment)
                md.write(comment + "    Size: " + convert_size(entry['Size']) + "\n")
                md.write(comment + "    Race: " + convert_race(entry['Race']) + "\n")
                md.write(comment + "    Element: " + convert_element(entry['Element']) + "\n")
                write_mode(md, int(entry['Mode'], 16), comment)
                md.write(comment + "    Speed: " + entry['Speed'] + "\n")
                md.write(comment + "    aDelay: " + entry['aDelay'] + "\n")
                md.write(comment + "    aMotion: " + entry['aMotion'] + "\n")
                md.write(comment + "    dMotion: " + entry['dMotion'] + "\n")
                if entry['MEXP'] != "0":
                    md.write(comment + "    MEXP: " + entry['MEXP'] + "\n")
                if "overwrite" in entry and entry['overwrite']:
                    md.write(comment + "    Overwrite: true\n")
            if len(entry['drops']) > 0:
                md.write(comment + "    NormalDrop:\n")
                for drop in entry['drops']:
                    write_drop(md, drop, comment)

            if len(entry['mvp_drops']) > 0:
                md.write(comment + "    MvPDrop:\n")
                for drop in entry['mvp_drops']:
                    write_drop(md, drop, comment)


def printUsage():
    print("convert_mobdb_yml.py database-path [...]")
    print("\tExample Usage: convert_mobdb_yml.py ../db/pre-re/ ../db/import/")
    print("This will convert mob_db.txt + mob_drop.txt -> mob_db.yml")


def main():
    if len(sys.argv) <= 1:
        printUsage()
        exit(0)
    curtime = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
    for path in sys.argv[1:]:
        files = [path + MOB_DB_TXT, path + MOB_DROP_TXT]
        for file in files:
            if not os.path.exists(file):
                print("File {0} does not exist!".format(file))
                print("Exiting without changes...")
                exit(0)
        files = [path + MOB_DB_YAML]
        for file in files:
            if os.path.exists(file):
                c = raw_input("Overwrite {0}? (y/n): ".format(file))
                if c[0] != 'y' and c[0] != 'Y':
                    exit(0)
                shutil.copy2(file, file + ".old" + curtime)
        read_item_db(path)
        db = read_mob_db(path)
        print(str(len(db)) + " entries read in path " + path)
        write_db(path, db)

MOBDBHEADER = """
# For a full explanation of this format, check doc/mob_db.txt
#  -
#    MobId: <int>
#    Name: <string>
#    kROName: <string>
#    Sprite: <string>
#    LV: <int>
#    HP: <int>
#    SP: <int>
#    EXP: <int>
#    JEXP: <int>
#    Range: <int>~<int>~<int>
#    ATK: <int>{~<int>}
#    DEF: <int>
#    MDEF: <int>
#    Stats:
#        Str: <int>
#        Agi: <int>
#        Vit: <int>
#        Int: <int>
#        Dex: <int>
#        Luk: <int>
#    Size: <string>
#    Race: <string>
#    Element: <string> <int>
#    Mode:
#        [MD_EXAMPLE: <bool | ~ | null>]
#    Speed: <int>
#    aDelay: <int>
#    aMotion: <int>
#    dMotion: <int>
#    MEXP: <int>
#    NormalDrop:
#      -
#        ItemId: <int>
#        AegisName: <string>
#        Index: <int>
#        Rate: <int>
#        StealProtected: <bool>
#        RandomOptionGroup: <string>
#    MvPDrop:
#      -
#        ItemId: <int>
#        AegisName: <string>
#        Index: <int>
#        Rate: <int>
#        StealProtected: <bool>
#        RandomOptionGroup: <string>

"""

if __name__ == "__main__":
    main()
