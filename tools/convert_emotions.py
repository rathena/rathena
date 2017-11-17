#!/usr/bin/env python3

"""
Simple scripts which replaces the old emotion script constants to the new
script constants.
The actual replacement list is in 'emotion_dict'.

Related pull request: https://github.com/rathena/rathena/pull/2527
Note: This only applies the constant changes. You still have to check your scripts
for 'unitemote' (command was removed) and 'emotion' using the third parameter ('target name').
"""

import fileinput
import re
import os

convert_folders =  ["../npc", "../src"]
wl_file_extensions = ['.hpp', '.h', '.cpp', '.c', '.txt' ]
bl_files = ['script_constants.hpp']
BACKUP_EXT = '.bak'
emotion_dict = {
'E_GASP': 'ET_SURPRISE',
'E_WHAT': 'ET_QUESTION',
'E_HO': 'ET_DELIGHT',
'E_LV': 'ET_THROB',
'E_SWT': 'ET_SWEAT',
'E_IC': 'ET_AHA',
'E_AN': 'ET_FRET',
'E_AG': 'ET_ANGER',
'E_CASH': 'ET_MONEY',
'E_DOTS': 'ET_THINK',
'E_SCISSORS': 'ET_SCISSOR',
'E_ROCK': 'ET_ROCK',
'E_PAPER': 'ET_WRAP',
'E_KOREA': 'ET_FLAG',
'E_LV2': 'ET_BIGTHROB',
'E_THX': 'ET_THANKS',
'E_WAH': 'ET_KEK',
'E_SRY': 'ET_SORRY',
'E_HEH': 'ET_SMILE',
'E_SWT2': 'ET_PROFUSELY_SWEAT',
'E_HMM': 'ET_SCRATCH',
'E_NO1': 'ET_BEST',
'E_NO': 'ET_STARE_ABOUT',
'E_OMG': 'ET_HUK',
'E_OH': 'ET_O',
'E_X': 'ET_X',
'E_HLP': 'ET_HELP',
'E_GO': 'ET_GO',
'E_SOB': 'ET_CRY',
'E_GG': 'ET_KIK',
'E_KIS': 'ET_CHUP',
'E_KIS2': 'ET_CHUPCHUP',
'E_PIF': 'ET_HNG',
'E_OK': 'ET_OK',
'E_MUTE': 'ET_CHAT_PROHIBIT',
'E_INDONESIA': 'ET_INDONESIA_FLAG',
'E_BZZ': 'ET_STARE',
'E_RICE': 'ET_HUNGRY',
'E_AWSM': 'ET_COOL',
'E_MEH': 'ET_MERONG',
'E_SHY': 'ET_SHY',
'E_PAT': 'ET_GOODBOY',
'E_MP': 'ET_SPTIME',
'E_SLUR': 'ET_SEXY',
'E_COM': 'ET_COMEON',
'E_YAWN': 'ET_SLEEPY',
'E_GRAT': 'ET_CONGRATULATION',
'E_HP': 'ET_HPTIME',
'E_PHILIPPINES': 'ET_PH_FLAG',
'E_MALAYSIA': 'ET_MY_FLAG',
'E_SINGAPORE': 'ET_SI_FLAG',
'E_BRAZIL': 'ET_BR_FLAG',
'E_FLASH': 'ET_SPARK',
'E_SPIN': 'ET_CONFUSE',
'E_SIGH': 'ET_OHNO',
'E_DUM': 'ET_HUM',
'E_LOUD': 'ET_BLABLA',
'E_OTL': 'ET_OTL',
'E_DICE1': 'ET_DICE1',
'E_DICE2': 'ET_DICE2',
'E_DICE3': 'ET_DICE3',
'E_DICE4': 'ET_DICE4',
'E_DICE5': 'ET_DICE5',
'E_DICE6': 'ET_DICE6',
'E_INDIA': 'ET_INDIA_FLAG',
'E_LUV': 'ET_LUV',
'E_RUSSIA': 'ET_FLAG8',
'E_VIRGIN': 'ET_FLAG9',
'E_MOBILE': 'ET_MOBILE',
'E_MAIL': 'ET_MAIL',
'E_CHINESE': 'ET_ANTENNA0',
'E_ANTENNA1': 'ET_ANTENNA1',
'E_ANTENNA2': 'ET_ANTENNA2',
'E_ANTENNA3': 'ET_ANTENNA3',
'E_HUM': 'ET_HUM2',
'E_ABS': 'ET_ABS',
'E_OOPS': 'ET_OOPS',
'E_SPIT': 'ET_SPIT',
'E_ENE': 'ET_ENE',
'E_PANIC': 'ET_PANIC',
'E_WHISP': 'ET_WHISP',
'E_YUT1': 'ET_YUT1',
'E_YUT2': 'ET_YUT2',
'E_YUT3': 'ET_YUT3',
'E_YUT4': 'ET_YUT4',
'E_YUT5': 'ET_YUT5',
'E_YUT6': 'ET_YUT6',
'E_YUT7': 'ET_YUT7',
'E_MAX': 'ET_MAX'
}

pattern = re.compile(r'\b(' + '|'.join(emotion_dict.keys()) + r')\b', re.IGNORECASE)

def revert_to_backup(filename):
   os.rename(filename+BACKUP_EXT, filename)

def replace_emoticons_in_file(filename):
    remove_backup = True # only keep backup if the original file changed
    with fileinput.FileInput(filename, inplace=True, backup=BACKUP_EXT) as fiFile:
        try:
            for line in fiFile:
                new_line, rpl_cnt = pattern.subn(lambda x: emotion_dict[x.group().upper()], line)
                print(new_line, end='')
                if rpl_cnt > 0:
                    remove_backup = False
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
 
 
