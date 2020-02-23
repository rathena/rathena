import yaml
import os
import sys
import re
import shutil
import datetime

ITEM_DB_YML = "item_db.yml"
ITEM_DB_SQL = "item_db"

def q(s):
    return '"' + s + '"'

def write_sql(files, item_db, is_import):
    """Write .sql files

    files[0] is item_db.sql
    """
    tables = [os.path.basename(f).split('.')[0] for f in files]
    if is_import:
        tables = [t.replace("2", "") for t in tables]
    with open(files[0], 'w') as md:
        header = SQLHEADER
        md.write(header.format(tables[0]))
        for entry in item_db:
            fields = []
            fields.append(("id", entry['Id']))
            fields.append(("name_english", q(entry['Name'])))
            fields.append(("name_aegis", q(entry['AegisName'])))
            if "Type" in entry:
                fields.append(("type", q(entry['Type'])))
            if "SubType" in entry:
                fields.append(("subtype", q(entry['SubType'])))
            if "Buy" in entry:
                fields.append(("price_buy", entry['Buy']))
            if "Sell" in entry:
                fields.append(("price_sell", entry['Sell']))
            if "Weight" in entry:
                fields.append(("weight", entry['Weight']))
            if "Attack" in entry:
                fields.append(("attack", entry['Attack']))
            if "MagicAttack" in entry:
                fields.append(("magic_attack", entry['MagicAttack']))
            if "Defense" in entry:
                fields.append(("defense", entry['Defense']))
            if "Range" in entry:
                fields.append(("range", entry['Range']))
            if "Slots" in entry:
                fields.append(("slots", entry['Slots']))
            if "Job" in entry:
                j = ""
                for jobs in entry['Job']:
                    j += "|" + jobs
                fields.append(("equip_jobs", q(j)))
            if "Class" in entry:
                c = ""
                for classes in entry['Class']:
                    c += "|" + classes
                fields.append(("equip_upper", q(c)))
            if "Gender" in entry:
                fields.append(("equip_genders", q(entry['Gender'])))
            if "Location" in entry:
                l = ""
                for locations in entry['Location']:
                    l += "|" + locations
                fields.append(("equip_locations", q(l)))
            if "WeaponLevel" in entry:
                fields.append(("weapon_level", entry['WeaponLevel']))
            if "EquipLevelMin" in entry:
                fields.append(("equip_level_min", entry['EquipLevelMin']))
            if "EquipLevelMax" in entry:
                fields.append(("equip_level_max", entry['EquipLevelMax']))
            if "Refineable" in entry:
                fields.append(("refineable", entry['Refineable']))
            if "View" in entry:
                fields.append(("view", entry['View']))
            if "Flags" in entry:
                f = entry['Flags']
                if "BuyingStore" in f:
                    fields.append(("flag_buyingstore", 1 if f['BuyingStore'] > 0 else 0))
                if "DeadBranch" in f:
                    fields.append(("flag_deadbranch", 1 if f['DeadBranch'] > 0 else 0))
                if "Container" in f:
                    fields.append(("flag_container", 1 if f['Container'] > 0 else 0))
                if "Guid" in f:
                    fields.append(("flag_guid", 1 if f['Guid'] > 0 else 0))
                if "BindOnEquip" in f:
                    fields.append(("flag_bindonequip", 1 if f['BindOnEquip'] > 0 else 0))
                if "DropAnnounce" in f:
                    fields.append(("flag_dropannounce", 1 if f['DropAnnounce'] > 0 else 0))
                if "NoConsume" in f:
                    fields.append(("flag_noconsume", 1 if f['NoConsume'] > 0 else 0))
                if "DropEffect" in f:
                    fields.append(("flag_dropeffect", 1 if f['DropEffect'] > 0 else 0))
            if "Delay" in entry:
                d = entry['Delay']
                if "Duration" in d:
                    fields.append(("delay_duration", d['Duration']))
                if "Status" in d:
                    fields.append(("delay_status", q(d['Status'])))
            if "Stack" in entry:
                s = entry['Stack']
                if "Amount" in s:
                    fields.append(("stack_amount", s['Amount']))
                if "Inventory" in s:
                    fields.append(("stack_inventory", 1 if s['Inventory'] > 0 else 0))
                if "Cart" in s:
                    fields.append(("stack_cart", 1 if s['Cart'] > 0 else 0))
                if "Storage" in s:
                    fields.append(("stack_storage", 1 if s['Storage'] > 0 else 0))
                if "GuildStorage" in s:
                    fields.append(("stack_guildstorage", 1 if s['GuildStorage'] > 0 else 0))
            if "Nouse" in entry:
                n = entry['NoUse']
                if "Override" in n:
                    fields.append(("nouse_override", n['Override']))
                if "Sitting" in n:
                    fields.append(("nouse_sitting", 1 if n['Sitting'] > 0 else 0))
            if "Trade" in entry:
                t = entry['Trade']
                if "Override" in t:
                    fields.append(("trade_override", t['Override']))
                if "NoDrop" in t:
                    fields.append(("trade_nodrop", 1 if t['NoDrop'] > 0 else 0))
                if "NoTrade" in t:
                    fields.append(("trade_notrade", 1 if t['NoTrade'] > 0 else 0))
                if "TradePartner" in t:
                    fields.append(("trade_tradepartner", 1 if t['TradePartner'] > 0 else 0))
                if "NoSell" in t:
                    fields.append(("trade_nosell", 1 if t['NoSell'] > 0 else 0))
                if "NoCart" in t:
                    fields.append(("trade_nocart", 1 if t['NoCart'] > 0 else 0))
                if "NoStorage" in t:
                    fields.append(("trade_nostorage", 1 if t['NoStorage'] > 0 else 0))
                if "NoGuildStorage" in t:
                    fields.append(("trade_noguildstorage", 1 if t['NoGuildStorage'] > 0 else 0))
                if "NoMail" in t:
                    fields.append(("trade_nomail", 1 if t['NoMail'] > 0 else 0))
                if "NoAuction" in t:
                    fields.append(("trade_noauction", 1 if t['NoAuction'] > 0 else 0))
            if "Script" in entry:
                fields.append(("script", q(entry['Script'])))
            if "EquipScript" in entry:
                fields.append(("equip_script", q(entry['EquipScript'])))
            if "UnEquipScript" in entry:
                fields.append(("unequip_script", q(entry['UnEquipScript'])))
            if len(fields) < 3:
                continue
            keys = ["`" + k + "`" for k, v in fields]
            vals = [str(v) for k, v in fields]
            s = "INSERT INTO " + tables[0] + "(" + ", ".join(keys) + ") "
            s += "VALUES \n\t(" + ", ".join(vals) + ") \n\tON DUPLICATE KEY UPDATE "
            s += ", ".join([k + "=VALUES(" + k + ")" for k in keys[1:]])
            md.write(s + ";\n\n")

def read_item_yml(path):
    with open(path + ITEM_DB_YML, 'r') as mdb:
        db = yaml.load(mdb)
    return db

def printUsage():
    print("convert_item_yaml_to_sql.py server-type sql-path database-path import-path")
    print("\tExample Usage: convert_item_yaml_to_sql.py pre-re ../sql-files/ ../db/pre-re/ ../db/import/")
    print("This will convert item_db.yml -> item_db.sql")

def main():
    if len(sys.argv) <= 3:
        printUsage()
        exit(0)

    sql_path = sys.argv[2]
    is_renewal = "pre" not in sys.argv[1]
    curtime = datetime.datetime.now().strftime('%Y%m%d%H%M%S')

    for path in sys.argv[3:]:
        is_import = "import" in path
        file = path + ITEM_DB_YML
        if not os.path.exists(file):
            print("File {0} does not exist!".format(file))
            print("Exiting without changes...")
            exit(0)
        suffix = ""
        if is_import:
            suffix += "2"
        if is_renewal:
            suffix += "_re"
        files = [sql_path + ITEM_DB_SQL + suffix + ".sql"]
        for file in files:
            if os.path.exists(file):
                c = raw_input("Overwrite {0}? (y/n): ".format(file))
                if c[0] != 'Y' and c[0] != 'y':
                    exit(0)
                shutil.copy2(file, file + ".old." + curtime)
        db = read_item_yml(path)['Body']
        if db is None:
            db = []
        print(str(len(db)) + " entries read in path " + path)
        write_sql(files, db, is_import)

SQLHEADER = """#
# Table structure for table `{0}`
#

DROP TABLE IF EXISTS `{0}`;
CREATE TABLE `{0}` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `name_english` varchar(50) NOT NULL DEFAULT '',
  `name_aegis` varchar(50) NOT NULL DEFAULT '',
  `type` varchar(20) NOT NULL DEFAULT 'Etc',
  `subtype` varchar(20) NOT NULL DEFAULT '',
  `price_buy` mediumint(8) unsigned DEFAULT '0',
  `price_sell` mediumint(8) unsigned DEFAULT '0',
  `weight` smallint(5) unsigned NOT NULL DEFAULT '0',
  `attack` smallint(5) unsigned DEFAULT '0',
  `defense` smallint(5) unsigned DEFAULT '0',
  `range` tinyint(2) unsigned DEFAULT '0',
  `slots` tinyint(2) unsigned DEFAULT '0',
  `equip_jobs` text,
  `equip_upper` text,
  `equip_genders` varchar(10) NOT NULL DEFAULT 'Both',
  `equip_locations` text,
  `weapon_level` tinyint(1) unsigned DEFAULT '0',
  `equip_level_min` tinyint(3) unsigned DEFAULT '0',
  `equip_level_max` tinyint(3) unsigned DEFAULT '0',
  `refineable` tinyint(1) unsigned DEFAULT '0',
  `view` smallint(5) unsigned DEFAULT '0',
  `flag_buyingstore` tinyint(1) unsigned DEFAULT '0',
  `flag_deadbranch` tinyint(1) unsigned DEFAULT '0',
  `flag_container` tinyint(1) unsigned DEFAULT '0',
  `flag_guid` tinyint(1) unsigned DEFAULT '0',
  `flag_bindonequip` tinyint(1) unsigned DEFAULT '0',
  `flag_dropannounce` tinyint(1) unsigned DEFAULT '0',
  `flag_noconsume` tinyint(1) unsigned DEFAULT '0',
  `flag_dropeffect` tinyint(1) unsigned DEFAULT '0',
  `delay_duration` bigint(20) unsigned DEFAULT '0',
  `delay_status` varchar(30) NOT NULL DEFAULT 'None',
  `stack_amount` smallint(5) unsigned DEFAULT '0',
  `stack_inventory` tinyint(1) unsigned DEFAULT '0',
  `stack_cart` tinyint(1) unsigned DEFAULT '0',
  `stack_storage` tinyint(1) unsigned DEFAULT '0',
  `stack_guildstorage` tinyint(1) unsigned DEFAULT '0',
  `nouse_override` smallint(5) unsigned DEFAULT '0',
  `nouse_sitting` tinyint(1) unsigned DEFAULT '0',
  `trade_override` smallint(5) unsigned DEFAULT '0',
  `trade_nodrop` tinyint(1) unsigned DEFAULT '0',
  `trade_notrade` tinyint(1) unsigned DEFAULT '0',
  `trade_tradepartner` tinyint(1) unsigned DEFAULT '0',
  `trade_nosell` tinyint(1) unsigned DEFAULT '0',
  `trade_nocart` tinyint(1) unsigned DEFAULT '0',
  `trade_nostorage` tinyint(1) unsigned DEFAULT '0',
  `trade_noguildstorage` tinyint(1) unsigned DEFAULT '0',
  `trade_nomail` tinyint(1) unsigned DEFAULT '0',
  `trade_noauction` tinyint(1) unsigned DEFAULT '0',
  `script` text,
  `equip_script` text,
  `unequip_script` text,
  PRIMARY KEY (`id`),
  UNIQUE INDEX `UniqueAegisName` (`name_english`)
) ENGINE=MyISAM;

"""

if __name__ == "__main__":
    main()
