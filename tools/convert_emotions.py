#!/usr/bin/env python3

"""
Simple scripts which replaces the old emotion script constants to the new
script constants.
The actual replacement list is in 'emotion_dict'.

Related pull request: https://github.com/rathena/rathena/pull/2527
"""

import fileinput
import re
import os
import collections

convert_folders =  ["../npc", "../src"]
parse_dict_file = '../src/map/script_constants.hpp'
wl_file_extensions = ['.hpp', '.h', '.cpp', '.c', '.txt' ]
script_file_extensions = ['.txt']
bl_files = ['script_constants.hpp']
BACKUP_EXT = '.bak'
"""
This is the old emotion_dict, which is now parsed from script_constants.hpp.
emotion_dict_old = collections.OrderedDict([
('E_GASP', 'ET_SURPRISE'),
('E_WHAT', 'ET_QUESTION'),
('E_HO', 'ET_DELIGHT'),
('E_LV', 'ET_THROB'),
('E_SWT', 'ET_SWEAT'),
('E_IC', 'ET_AHA'),
('E_AN', 'ET_FRET'),
('E_AG', 'ET_ANGER'),
('E_CASH', 'ET_MONEY'),
('E_DOTS', 'ET_THINK'),
('E_SCISSORS', 'ET_SCISSOR'),
('E_ROCK', 'ET_ROCK'),
('E_PAPER', 'ET_WRAP'),
('E_KOREA', 'ET_FLAG'),
('E_LV2', 'ET_BIGTHROB'),
('E_THX', 'ET_THANKS'),
('E_WAH', 'ET_KEK'),
('E_SRY', 'ET_SORRY'),
('E_HEH', 'ET_SMILE'),
('E_SWT2', 'ET_PROFUSELY_SWEAT'),
('E_HMM', 'ET_SCRATCH'),
('E_NO1', 'ET_BEST'),
('E_NO', 'ET_STARE_ABOUT'),
('E_OMG', 'ET_HUK'),
('E_OH', 'ET_O'),
('E_X', 'ET_X'),
('E_HLP', 'ET_HELP'),
('E_GO', 'ET_GO'),
('E_SOB', 'ET_CRY'),
('E_GG', 'ET_KIK'),
('E_KIS', 'ET_CHUP'),
('E_KIS2', 'ET_CHUPCHUP'),
('E_PIF', 'ET_HNG'),
('E_OK', 'ET_OK'),
('E_MUTE', 'ET_CHAT_PROHIBIT'),
('E_INDONESIA', 'ET_INDONESIA_FLAG'),
('E_BZZ', 'ET_STARE'),
('E_RICE', 'ET_HUNGRY'),
('E_AWSM', 'ET_COOL'),
('E_MEH', 'ET_MERONG'),
('E_SHY', 'ET_SHY'),
('E_PAT', 'ET_GOODBOY'),
('E_MP', 'ET_SPTIME'),
('E_SLUR', 'ET_SEXY'),
('E_COM', 'ET_COMEON'),
('E_YAWN', 'ET_SLEEPY'),
('E_GRAT', 'ET_CONGRATULATION'),
('E_HP', 'ET_HPTIME'),
('E_PHILIPPINES', 'ET_PH_FLAG'),
('E_MALAYSIA', 'ET_MY_FLAG'),
('E_SINGAPORE', 'ET_SI_FLAG'),
('E_BRAZIL', 'ET_BR_FLAG'),
('E_FLASH', 'ET_SPARK'),
('E_SPIN', 'ET_CONFUSE'),
('E_SIGH', 'ET_OHNO'),
('E_DUM', 'ET_HUM'),
('E_LOUD', 'ET_BLABLA'),
('E_OTL', 'ET_OTL'),
('E_DICE1', 'ET_DICE1'),
('E_DICE2', 'ET_DICE2'),
('E_DICE3', 'ET_DICE3'),
('E_DICE4', 'ET_DICE4'),
('E_DICE5', 'ET_DICE5'),
('E_DICE6', 'ET_DICE6'),
('E_INDIA', 'ET_INDIA_FLAG'),
('E_LUV', 'ET_LUV'),
('E_RUSSIA', 'ET_FLAG8'),
('E_VIRGIN', 'ET_FLAG9'),
('E_MOBILE', 'ET_MOBILE'),
('E_MAIL', 'ET_MAIL'),
('E_CHINESE', 'ET_ANTENNA0'),
('E_ANTENNA1', 'ET_ANTENNA1'),
('E_ANTENNA2', 'ET_ANTENNA2'),
('E_ANTENNA3', 'ET_ANTENNA3'),
('E_HUM', 'ET_HUM2'),
('E_ABS', 'ET_ABS'),
('E_OOPS', 'ET_OOPS'),
('E_SPIT', 'ET_SPIT'),
('E_ENE', 'ET_ENE'),
('E_PANIC', 'ET_PANIC'),
('E_WHISP', 'ET_WHISP'),
('E_YUT1', 'ET_YUT1'),
('E_YUT2', 'ET_YUT2'),
('E_YUT3', 'ET_YUT3'),
('E_YUT4', 'ET_YUT4'),
('E_YUT5', 'ET_YUT5'),
('E_YUT6', 'ET_YUT6'),
('E_YUT7', 'ET_YUT7')
])
"""
def parse_emotion_dict(filepath):
    ret_list = []
    with fileinput.FileInput(filepath) as fiFile:
        for line in fiFile:
            found = re.search('"(E_[A-Z_0-9]+)"\s*,\s*(ET_[A-Z_0-9]+)\s*', line)
            if found:
                ret_list.append((found.group(1), found.group(2)))
    return ret_list

emotion_dict = collections.OrderedDict(parse_emotion_dict(parse_dict_file))

emotion_array = [val for val in emotion_dict.values()]
pattern_oldconst = re.compile(r'\b(' + '|'.join(emotion_dict.keys()) + r')\b', re.IGNORECASE)
pattern_value = re.compile(r'\b(' + '|'.join(["emotion\s+%d+"%i for i in range(len(emotion_array))]) + r')\b', re.IGNORECASE)

def revert_to_backup(filename):
   os.rename(filename+BACKUP_EXT, filename)
   
def apply_substitutions(new_line, is_script):
    remove_backup = True # only keep backup if the original file changed
    rpl_cnt = 0
    # E_GASP -> ET_SURPRISE
    new_line, rpl_cnt = pattern_oldconst.subn(lambda x: emotion_dict[x.group().upper()], new_line)
    remove_backup = False if rpl_cnt > 0 else remove_backup
    if is_script: # script only replacements
        # 0 -> ET_SURPRISE
        new_line, rpl_cnt = pattern_value.subn(lambda x: 'emotion '+emotion_array[int(x.group().split()[-1])], new_line)
        remove_backup = False if rpl_cnt > 0 else remove_backup
        # emotion e,0,"Record player#e152a01"; -> emotion e, getnpcid(0,"Record player#e152a01");
        new_line, rpl_cnt = re.subn(r"emotion\s+([^,]+)\s*,\s*0\s*,\s*([^;]+);",
                           "emotion \g<1>, getnpcid(0, \g<2>);", new_line)
        remove_backup = False if rpl_cnt > 0 else remove_backup
        # emotion e,1; -> emotion e, playerattached();
        new_line, rpl_cnt = re.subn(r"emotion\s+([^,]+)\s*,\s*1\s*;",
                           "emotion \g<1>, playerattached();", new_line)
        remove_backup = False if rpl_cnt > 0 else remove_backup
        # unitemote <id>,<emotion>; -> emotion <emotion>, <id>;
        new_line, rpl_cnt = re.subn(r"unitemote\s+([^,]+)\s*,\s*([^,;]+)\s*;",
                           "emotion \g<2>, \g<1>;", new_line)
        remove_backup = False if rpl_cnt > 0 else remove_backup

    return new_line, remove_backup

def replace_emoticons_in_file(filename):
    is_script = True if any([filename.endswith(script_ext) for script_ext in script_file_extensions]) else False
    remove_backup = True
    with fileinput.FileInput(filename, inplace=True, backup=BACKUP_EXT) as fiFile:
        try:
            for line in fiFile:
                new_line, rm_backup = apply_substitutions(line, is_script)
                if not rm_backup:
                    remove_backup = False
                print(new_line, end='')
            if remove_backup:
                os.remove(filename+BACKUP_EXT)
        except UnicodeDecodeError:
            # Encoding error, reapply the backup
            revert_to_backup(filename)


fileiter = (os.path.join(root, f)
    for conv_folder in convert_folders 
    for root, _, files in os.walk(conv_folder)
    for f in files
    if any([f.endswith(wl) for wl in wl_file_extensions])
    if not any([bl in f for bl in bl_files])
    )

for f in fileiter:
    print("Updating file", f)
    replace_emoticons_in_file(f)
    
