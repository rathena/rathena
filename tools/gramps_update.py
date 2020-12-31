import io
import os
import re

# Target update variables
mobs_low = [1155, 1257]
mobs_mid = [1366, 1771]
mobs_high = [2021, 2363]
map_low = 'epay_dun00l'
map_mid = 'egef_dun03m'
map_high = 'eabyss_03h'
spawn_quantity = 50
script_path = os.path.dirname(os.path.abspath(__file__)) + '/../npc/custom/quests/gramps.txt'

# Note: mobs and quests are mapped by index order
mobs = [
    1193, 1106, 1410, 1417, 1370, 1374, 2083, 2084, 1778,
    1777, 1752, 1753, 2085, 2086, 1402, 1403, 1504, 1505,
    1713, 1716, 1614, 1615, 1714, 1717, 1657, 1654, 1652,
    1655, 1656, 1653, 1316, 1314, 1736, 1737, 1987, 2024,
    1163, 1208, 1780, 1781, 2019, 2021, 1622, 1617, 1670,
    1671, 1672, 1673, 1838, 1837, 1627, 1782, 1769, 1770,
    1638, 1635, 1637, 1634, 1639, 1636, 1613, 1619, 1676,
    1677, 1678, 1679, 1867, 1870, 1253, 1194, 1215, 1162,
    1699, 1698, 1986, 2092, 1133, 1134, 1135, 1833, 1831,
    1143, 1109, 1194, 1297, 1384, 1385, 1773, 1774, 1196,
    1257, 2015, 1993, 1509, 1510, 1836, 1680, 1256, 1148,
    1989, 1282, 1390, 1715, 1668, 1988, 2133, 2132, 1718,
    1992, 2137, 2136, 1318, 1315, 1367, 1366, 1295, 1771,
    1772, 1865, 1864, 1883, 1102, 1379, 1206, 1703, 1201,
    1735, 1155, 1884, 1995, 1775, 1682, 1132, 1408, 1866,
    1213, 1319, 2013, 1628, 1377, 2134, 1776, 1197, 1117,
    1219, 1401, 1376, 1406, 1869, 1796, 1797, 1375, 1405,
    2071, 1198, 1098, 1512, 1513, 1309, 2153, 1291, 1999,
    1154, 2315, 2313, 1386, 2365, 2366, 1199, 2363, 2370,
    2309, 2310, 2364, 1692, 1255, 2204, 1305, 2369, 2367,
    2073, 2199
]

quests = [
    60443, 60445, 60451, 60452, 60453, 60454, 60455, 60456, 60459,
    60460, 60461, 60462, 60463, 60464, 60465, 60466, 60467, 60468,
    60469, 60470, 60476, 60477, 60478, 60479, 60480, 60481, 60482,
    60483, 60484, 60485, 60488, 60489, 60490, 60491, 60492, 60493,
    60501, 60502, 60503, 60504, 60505, 60506, 60507, 60508, 60509,
    60510, 60511, 60512, 60513, 60514, 60515, 60516, 60517, 60518,
    60519, 60520, 60521, 60522, 60523, 60524, 60525, 60526, 60527,
    60528, 60529, 60530, 60531, 60532, 60535, 60536, 60537, 60538,
    60539, 60540, 60541, 60542, 60543, 60544, 60545, 60546, 60547,
    60549, 60550, 60551, 60552, 60553, 60554, 60555, 60556, 60557,
    60558, 60559, 60560, 60561, 60562, 60565, 60570, 60571, 60572,
    60573, 60575, 60576, 60579, 60580, 60581, 60582, 60583, 60584,
    60585, 60586, 60587, 60588, 60589, 60590, 60591, 60592, 60593,
    60594, 60595, 60596, 60601, 60602, 60603, 60604, 60605, 60608,
    60609, 60610, 60611, 60612, 60614, 60815, 60816, 60817, 60818,
    60820, 60821, 60822, 60824, 60825, 60826, 60827, 60828, 60830,
    60831, 60832, 60833, 60834, 60835, 60836, 60837, 60838, 60839,
    60840, 60841, 60842, 60843, 60844, 60845, 60846, 60848, 60849,
    60851, 60853, 60854, 60855, 60856, 60857, 60858, 60859, 60860,
    60861, 60862, 60863, 60864, 60865, 60866, 60867, 60868, 60869,
    62832, 62833
]

# Uses kName value from mob_db_re
mob_names = {
    1098: 'Anubis',
    1102: 'Bathory',
    1106: 'Desert Wolf',
    1109: 'Deviruchi',
    1117: 'Evil Druid',
    1132: 'Khalitzburg',
    1133: 'Kobold',
    1134: 'Kobold',
    1135: 'Kobold',
    1143: 'Marionette',
    1148: 'Medusa',
    1154: 'Pasana',
    1155: 'Petite',
    1162: 'Rafflesia',
    1163: 'Raydric',
    1193: 'Alarm',
    1194: 'Arclouse',
    1196: 'Skeleton Prisoner',
    1197: 'Zombie Prisoner',
    1198: 'Dark Priest',
    1199: 'Punk',
    1201: 'Rybio',
    1206: 'Anolian',
    1208: 'Wander Man',
    1213: 'High Orc',
    1215: 'Stem Worm',
    1219: 'Knight of Abyss',
    1253: 'Gargoyle',
    1255: 'Neraid',
    1256: 'Pest',
    1257: 'Injustice',
    1282: 'Kobold Archer',
    1291: 'Wraith Dead',
    1295: 'Owl Baron',
    1297: 'Ancient Mummy',
    1305: 'Ancient Worm',
    1309: 'Gajomart',
    1314: 'Permeter',
    1315: 'Assaulter',
    1316: 'Solider',
    1318: 'Heater',
    1319: 'Freezer',
    1366: 'Lava Golem',
    1367: 'Blazer',
    1370: 'Succubus',
    1374: 'Incubus',
    1375: 'The Paper',
    1376: 'Harpy',
    1377: 'Elder',
    1379: 'Nightmare Terror',
    1384: 'Deleter',
    1385: 'Deleter',
    1386: 'Sleeper',
    1390: 'Violy',
    1401: 'Shinobi',
    1402: 'Poison Toad',
    1403: 'Antique Firelock',
    1405: 'Tengu',
    1406: 'Kapha',
    1408: 'Bloody Butterfly',
    1410: 'Live Peach Tree',
    1417: 'Zipper Bear',
    1504: 'Dullahan',
    1505: 'Loli Ruri',
    1509: 'Lude',
    1510: 'Hylozoist',
    1512: 'Hyegun',
    1513: 'Civil Servant',
    1613: 'Metaling',
    1614: 'Mineral',
    1615: 'Obsidian',
    1617: 'Waste Stove',
    1619: 'Porcellio',
    1622: 'Teddy Bear',
    1627: 'Anopheles',
    1628: 'Mole',
    1634: 'Seyren',
    1635: 'Eremes',
    1636: 'Harword',
    1637: 'Magaleta',
    1638: 'Shecil',
    1639: 'Katrinn',
    1652: 'Ygnizem',
    1653: 'Whikebain',
    1654: 'Armaia',
    1655: 'Erend',
    1656: 'Kavac',
    1657: 'Rawrel',
    1668: 'Archdam',
    1670: 'Dimik',
    1671: 'Dimik',
    1672: 'Dimik',
    1673: 'Dimik',
    1676: 'Venatu',
    1677: 'Venatu',
    1678: 'Venatu',
    1679: 'Venatu',
    1680: 'Hill Wind',
    1682: 'Removal',
    1692: 'Breeze',
    1698: 'Deathword',
    1699: 'Ancient Mimic',
    1703: 'Solace',
    1713: 'Acidus',
    1714: 'Ferus',
    1715: 'Novus',
    1716: 'Acidus',
    1717: 'Ferus',
    1718: 'Novus',
    1735: 'Alicel',
    1736: 'Aliot',
    1737: 'Aliza',
    1752: 'Skogul',
    1753: 'Frus',
    1769: 'Agav',
    1770: 'Echio',
    1771: 'Vanberk',
    1772: 'Isilla',
    1773: 'Hodremlin',
    1774: 'Seeker',
    1775: 'Snowier',
    1776: 'Siroma',
    1777: 'Ice Titan',
    1778: 'Gazeti',
    1780: 'Muscipular',
    1781: 'Drosera',
    1782: 'Roween',
    1796: 'Aunoe',
    1797: 'Fanat',
    1831: 'Salamander',
    1833: 'Kasa',
    1836: 'Magmaring',
    1837: 'Imp',
    1838: 'Knocker',
    1864: 'Zombie Slaughter',
    1865: 'Ragged Zombie',
    1866: 'Hell Poodle',
    1867: 'Banshee',
    1869: 'Flame Skull',
    1870: 'Necromancer',
    1883: 'Uzhas',
    1884: 'Mavka',
    1986: 'Tatacho',
    1987: 'Centipede',
    1988: 'Nepenthes',
    1989: 'Hillslion',
    1992: 'Cornus',
    1993: 'Naga',
    1995: 'Pinguicula',
    1999: 'Centipede Larva',
    2013: 'Draco',
    2015: 'Dark Pinguicula',
    2019: 'Ancient Tree',
    2021: 'Phylla',
    2024: 'Bradium Golem',
    2071: 'Headless Mule',
    2073: 'Toucan',
    2083: 'Scaraba',
    2084: 'Scaraba',
    2085: 'Antler Scaraba',
    2086: 'Rake Scaraba',
    2092: 'Dolomedes',
    2132: 'Pom Spider',
    2133: 'Angra Mantis',
    2134: 'Parus',
    2136: 'Little Fatum',
    2137: 'Miming',
    2153: 'Cendrawasih',
    2199: 'Siorava',
    2204: 'Sedora',
    2309: 'Bungisngis',
    2310: 'Engkanto',
    2313: 'Tikbalang',
    2315: 'Wakwak',
    2363: 'Menblatt',
    2364: 'Petal',
    2365: 'Cenere',
    2366: 'Antique Book',
    2367: 'Blue Lichtern',
    2369: 'Red Lichtern',
    2370: 'Green Lichtern',
}

quest_low = [quests[mobs.index(mobs_low[0])], quests[mobs.index(mobs_low[1])]]
quest_mid = [quests[mobs.index(mobs_mid[0])], quests[mobs.index(mobs_mid[1])]]
quest_high = [quests[mobs.index(mobs_high[0])], quests[mobs.index(mobs_high[1])]]

turn_low = f'{quest_low[0]}, {mobs_low[0]}, {quest_low[1]}, {mobs_low[1]}'
turn_mid = f'{quest_mid[0]}, {mobs_mid[0]}, {quest_mid[1]}, {mobs_mid[1]}'
turn_high = f'{quest_high[0]}, {mobs_high[0]}, {quest_high[1]}, {mobs_high[1]}'
gramps_turn = f'{turn_low}, {turn_mid}, {turn_high}'

def generate_spawns(map_name, targets, spawn):
    return [f'{map_name},0,0,0,0\tmonster\t{mob_names[mob_id]}\t{mob_id},{spawn},0,0,0' for mob_id in targets]

spawns_low = generate_spawns(map_low, mobs_low, spawn_quantity)
spawns_mid = generate_spawns(map_mid, mobs_mid, spawn_quantity)
spawns_high = generate_spawns(map_high, mobs_high, spawn_quantity)

# Used to make different replacements using the same matching pattern.
# Warning: Can only handle replacing a matching number of spawns by gramps level
match_count = 0
level = ''
def spawn_repl(match):
    global match_count, level
    if level == 'l':
        repl = f'{map_low}{match.group(2)}{mob_names[mobs_low[match_count]]}\t{mobs_low[match_count]},{spawn_quantity},0,0,0'
    elif level == 'm':
        repl = f'{map_mid}{match.group(2)}{mob_names[mobs_mid[match_count]]}\t{mobs_mid[match_count]},{spawn_quantity},0,0,0'
    elif level == 'h':
        repl = f'{map_high}{match.group(2)}{mob_names[mobs_high[match_count]]}\t{mobs_high[match_count]},{spawn_quantity},0,0,0'
    match_count += 1
    return repl

def replace_spawn(lv, text):
    global match_count, level
    match_count = 0
    level = lv
    return re.sub(rf'(.*{lv})(,0,0,0,0\tmonster\t)(.*)', spawn_repl, text)

# Replace strings in gramps script
with open(script_path) as f:
    replaced = re.sub(r'(setarray \.Gramps_Turn, )(.*)(;)', rf'\g<1>{gramps_turn}\g<3>', f.read())
    replaced = replace_spawn('l', replaced)
    replaced = replace_spawn('m', replaced)
    replaced = replace_spawn('h', replaced)

with open(script_path, 'w') as f:
    f.write(replaced)
