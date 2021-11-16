4CrAM Database ReadMe
================================
This is a seprate database created for the entry and loading of data for things
related to the 4CrAM project. Item, monster, and skill related stuff is placed
here to keep things seprate from rAthena's database files and also make it easier
for development reasons.

2 type of files are stored in here. TXT and YML files. The TXT files are used for
the adding/editing of different item/mob/skill entrys. Once done, the csv2yml
tool can be ran to update these entrys into the YML files in this database.
The map server will then read these YML files on server startup.

Note that the csv2yaml will not apply these changes when ran from the root folder.
To apply them, you must copy the csv2yaml.exe to the folder this readme is in
and then run it. Running it from the root will only cause it to read the YML
files in this folders db subfolder.

The skill-backup folder is a backup of the TXT files that were used in the past
and are for reference use.
