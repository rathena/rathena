import yaml
import os
import sys
import re
import shutil

MOB_DB_YML = "mob_db.yml"
MOB_DB_SQL = "mob_db"
MOB_DROP_SQL = "mob_drop"
ITEM_DB_TXT = "item_db.txt"
item_db = {}
orig_mob_db = {}


def q(s):
    return '"' + s + '"'

def getMode(ms):
    modes = ["MD_CANMOVE", "MD_LOOTER", "MD_AGGRESSIVE", "MD_ASSIST", "MD_CASTSENSOR_IDLE"]
    modes.extend(["MD_NORANDOM_WALK", "MD_NOCAST_SKILL", "MD_CANATTACK", "MD_NONE"])
    modes.extend(["MD_CASTSENSOR_CHASE", "MD_CHANGECHASE", "MD_ANGRY", "MD_CHANGETARGET_MELEE"])
    modes.extend(["MD_CHANGETARGET_CHASE", "MD_TARGETWEAK", "MD_RANDOMTARGET", "MD_IGNOREMELEE"])
    modes.extend(["MD_IGNOREMAGIC", "MD_IGNORERANGED", "MD_MVP", "MD_IGNOREMISC"])
    modes.extend(["MD_KNOCKBACK_IMMUNE", "MD_TELEPORT_BLOCK", "MD_NONE", "MD_FIXED_ITEMDROP"])
    modes.extend(["MD_DETECTOR", "MD_STATUS_IMMUNE", "MD_SKILL_IMMUNE"])
    ret = 0
    if ms:
        for mode in ms:
            i = modes.index(mode)
            ret += 1 << i
    return hex(ret)


def write_sql(files, mob_db, is_import):
    """Write .sql files

    Files[0] is mob_db.sql
    Files[1] is mob_drop.sql
    mob_db is the yaml loaded

    INSERT INTO mob_db (id, b, c) VALUES (1, 2, 3)
      ON DUPLICATE KEY UPDATE  b=VALUES(b), c=VALUES(c)
    """
    tables = [os.path.basename(f).split('.')[0] for f in files]
    if is_import:
        tables = [t.replace("2", "") for t in tables]
    with open(files[0], 'w') as md:
        header = SQLHEADER if not is_import else SQLHEADERIMPORT
        md.write(header.format(tables[0]))
        for entry in mob_db:
            fields = []
            fields.append(("ID", entry['MobId']))
            if "Sprite" in entry:
                fields.append(("Sprite", q(entry['Sprite'])))
            if "Name" in entry:
                fields.append(("iName", q(entry['Name'])))
                if "kROName" not in entry:
                    fields.append(("kName", q(entry['Name'])))
            if "kROName" in entry:
                fields.append(("kName", q(entry['kROName'])))
            if "LV" in entry:
                fields.append(("LV", entry['LV']))
            if "HP" in entry:
                fields.append(("HP", entry['HP']))
            if "SP" in entry:
                fields.append(("SP", entry['SP']))
            if "EXP" in entry:
                fields.append(("EXP", entry['EXP']))
            if "JEXP" in entry:
                fields.append(("JEXP", entry['JEXP']))
            if "Range" in entry:
                r = entry['Range'].split("~")
                fields.append(("Range1", r[0]))
                fields.append(("Range2", r[1]))
                fields.append(("Range3", r[2]))
            if "ATK" in entry:
                a = entry['ATK'].split("~")
                fields.append(("ATK1", a[0]))
                if len(a) == 1:
                    fields.append(("ATK2", a[0]))
                else:
                    fields.append(("ATK2", a[1]))
            if "DEF" in entry:
                fields.append(("DEF", entry['DEF']))
            if "MDEF" in entry:
                fields.append(("MDEF", entry['MDEF']))
            if "Stats" in entry:
                s = entry['Stats']
                if "Str" in s:
                    fields.append(("STR", s['Str']))
                if "Dex" in s:
                    fields.append(("DEX", s['Dex']))
                if "Agi" in s:
                    fields.append(("AGI", s['Agi']))
                if "Int" in s:
                    fields.append(("INT", s['Int']))
                if "Vit" in s:
                    fields.append(("VIT", s['Vit']))
                if "Luk" in s:
                    fields.append(("LUK", s['Luk']))
            if "Size" in entry:
                sizes = ["SZ_SMALL", "SZ_MEDIUM", "SZ_LARGE"]
                fields.append(("Scale", sizes.index(entry['Size'])))
            if "Race" in entry:
                races = ["FORMLESS", "UNDEAD", "BRUTE", "PLANT", "INSECT", "FISH", "DEMON", "DEMIHUMAN", "ANGEL", "DRAGON", "PLAYER"]
                fields.append(("Race", races.index(entry['Race'][3:])))
            if "Element" in entry:
                eles = ["NEUTRAL", "WATER" , "EARTH" , "FIRE" , "WIND" , "POISON" , "HOLY" , "DARK" , "GHOST" , "UNDEAD"]
                e = entry['Element'].split(" ")
                ele = eles.index(e[0][4:])
                ele += int(e[1]) * 20
                fields.append(("Element", ele))
            if "Mode" in entry:
                mode = getMode(entry['Mode'])
                fields.append(("Mode", mode))
            if "Speed" in entry:
                fields.append(("Speed", entry['Speed']))
            if "aDelay" in entry:
                fields.append(("aDelay", entry['aDelay']))
            if "aMotion" in entry:
                fields.append(("aMotion", entry['aMotion']))
            if "dMotion" in entry:
                fields.append(("dMotion", entry['dMotion']))
            if "MEXP" in entry:
                fields.append(("MEXP", entry['MEXP']))
            if len(fields) < 2:
                continue
            keys = ["`" + k + "`" for k, v in fields]
            vals = [str(v) for k, v in fields]
            s = "INSERT INTO " + tables[0] + "(" + ", ".join(keys) + ") "
            s += "VALUES \n\t(" + ", ".join(vals) + ") \n\tON DUPLICATE KEY UPDATE "
            s += ", ".join([k + "=VALUES(" + k + ")" for k in keys[1:]])
            md.write(s + ";\n\n")

    with open(files[1], "w") as md:
        header = SQLDROPHEADER if not is_import else SQLHEADERIMPORT
        md.write(header.format(tables[1]))
        for entry in mob_db:
            dtypes = ["NormalDrop", "MvPDrop"]
            for dtype in dtypes:
                if dtype in entry:
                    mob_id = entry['MobId']
                    for count, drop in enumerate(entry[dtype]):
                        nameid = 0
                        idx = -1
                        rate = 0
                        steal_protect = "FALSE"
                        rog = "NULL"
                        if "AegisName" in drop:
                            nameid = item_db[drop['AegisName']]
                        if "ItemId" in drop:
                            nameid = drop['ItemId']
                        if "Index" in drop:
                            idx = drop['Index']
                        elif "Replace" in drop:
                            if mob_id in orig_mob_db:
                                mb = orig_mob_db[mob_id]
                            else:
                                mb = entry
                            if dtype in mb:
                                for c, d in enumerate(mb[dtype]):
                                    if "AegisName" in d:
                                        nid = item_db[d['AegisName']]
                                    if "ItemId" in drop:
                                        nid = d['ItemId']
                                    if nameid == nid:
                                        idx = c
                                        break
                            else:
                                idx = 0
                            if idx == -1:
                                idx = len(mb[dtype])
                        else:
                            idx = count
                        type = dtypes.index(dtype)
                        if "Rate" in drop:
                            rate = drop['Rate']
                        if "RandomOptionGroup" in drop:
                            rog = "'" + drop['RandomOptionGroup'] + "'"
                        if "StealProtected" in drop and drop['StealProtected']:
                            steal_protect = "TRUE"
                        keys = ["mobid", "dtype", "index", "nameid", "rate", "stealProtected", "randomOptionGroup"]
                        keys = ["`" + k + "`" for k in keys]
                        s = "INSERT INTO " + tables[1] +"(" + ", ".join(keys) + ") VALUES \n\t("
                        s += ", ".join([str(mob_id), str(type), str(idx), str(nameid), str(rate), steal_protect, rog])
                        s += ") \n\tON DUPLICATE KEY UPDATE "
                        s += ", ".join([k + "=VALUES("+k+")" for k in keys])
                        md.write(s + ";\n\n")





def read_mob_yml(path):
    with open(path + MOB_DB_YML, 'r') as mdb:
        db = yaml.load(mdb)
    return db


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
            item_db[fields[1]] = fields[0]


def printUsage():
    print("convert_mob_sql.py server-type sql-path database-path import-path")
    print("\tExample Usage: convert_mob_sql.py pre-re ../sql-files/ ../db/pre-re/ ../db/import/")
    print("This will convert mob_db.yml -> mob_db.sql + mob_drop.sql")


def main():
    if len(sys.argv) <= 3:
        printUsage()
        exit(0)

    sql_path = sys.argv[2]
    is_renewal = "pre" not in sys.argv[1]

    for path in sys.argv[3:]:
        is_import = "import" in path
        file = path + MOB_DB_YML
        if not os.path.exists(file):
            print("File {0} does not exist!".format(file))
            print("Exiting without changes...")
            exit(0)
        suffix = ""
        if is_import:
            suffix += "2"
        if is_renewal:
            suffix += "_re"
        files = [sql_path + MOB_DB_SQL + suffix + ".sql",
                 sql_path + MOB_DROP_SQL + suffix + ".sql"]
        for file in files:
            if os.path.exists(file):
                c = raw_input("Overwrite {0}? (y/n): ".format(file))
                if c[0] != 'Y' and c[0] != 'y':
                    exit(0)
                #shutil.copy2(file, file + ".old")
        read_item_db(path)
        db = read_mob_yml(path)['Mobs']
        if db is None:
            db = []
        print(str(len(db)) + " entries read in path " + path)
        write_sql(files, db, is_import)
        for entry in db:
            orig_mob_db[entry['MobId']] = entry

SQLDROPHEADER = """#
# Table structure for table `{0}`
#

DROP TABLE IF EXISTS `{0}`;

CREATE TABLE `{0}` (
    `mobid` mediumint(9) unsigned NOT NULL default '0',
    `dtype` smallint(5) unsigned NOT NULL default '0',
    `index` smallint(5) unsigned NOT NULL,
    `nameid` smallint(5) unsigned NOT NULL default '0',
    `rate` smallint(9) unsigned NOT NULL default '0',
    `stealProtected` bool default false,
    `randomOptionGroup` text,

    PRIMARY KEY (`mobId`, `dtype`, `index`)

) ENGINE=MyISAM;

"""
SQLHEADERIMPORT = """# 
# This is the overwrites for the sql database.
#

#INSERT INTO `{0}` (mobid, a, b, c) VALUES (1001, 1, 2, 3)
# ON DUPICLATE KEY UPDATE a=VALUES(a), b=VALUES(b), c=VALUES(c);

"""
SQLHEADER = """#
# Table structure for table `{0}`
#

DROP TABLE IF EXISTS `{0}`;

CREATE TABLE `{0}` (
	`ID` mediumint(9) unsigned NOT NULL default '0',
    `Sprite` text,
	`kName` text,
	`iName` text,
	`LV` tinyint(6) unsigned NOT NULL default '0',
	`HP` int(9) unsigned NOT NULL default '0',
	`SP` mediumint(9) unsigned NOT NULL default '0',
	`EXP` mediumint(9) unsigned NOT NULL default '0',
	`JEXP` mediumint(9) unsigned NOT NULL default '0',
	`Range1` tinyint(4) unsigned NOT NULL default '0',
	`ATK1` smallint(6) unsigned NOT NULL default '0',
	`ATK2` smallint(6) unsigned NOT NULL default '0',
	`DEF` smallint(6) unsigned NOT NULL default '0',
	`MDEF` smallint(6) unsigned NOT NULL default '0',
	`STR` smallint(6) unsigned NOT NULL default '0',
	`AGI` smallint(6) unsigned NOT NULL default '0',
	`VIT` smallint(6) unsigned NOT NULL default '0',
	`INT` smallint(6) unsigned NOT NULL default '0',
	`DEX` smallint(6) unsigned NOT NULL default '0',
	`LUK` smallint(6) unsigned NOT NULL default '0',
	`Range2` tinyint(4) unsigned NOT NULL default '0',
	`Range3` tinyint(4) unsigned NOT NULL default '0',
	`Scale` tinyint(4) unsigned NOT NULL default '0',
	`Race` tinyint(4) unsigned NOT NULL default '0',
	`Element` tinyint(4) unsigned NOT NULL default '0',
	`Mode` int(11) unsigned NOT NULL default '0',
	`Speed` smallint(6) unsigned NOT NULL default '0',
	`aDelay` smallint(6) unsigned NOT NULL default '0',
	`aMotion` smallint(6) unsigned NOT NULL default '0',
	`dMotion` smallint(6) unsigned NOT NULL default '0',
	`MEXP` mediumint(9) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM;

"""


if __name__ == "__main__":
    main()
