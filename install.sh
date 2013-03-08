#!/bin/sh
#source var/function
. ./function.sh

check_inst_right
check_files
read -p "WARNING: This target dis experimental. Press Ctrl+C to cancel or Enter to continue." readEnterKey
mkdir -p $PKG_PATH/bin/
mkdir -p $PKG_PATH/etc/$PKG/
mkdir -p $PKG_PATH/var/$PKG/

#we copy all file into opt/ dir and treat dir like normal unix arborescence
rsync -r --exclude .svn db/ $PKG_PATH/var/$PKG/db
rsync -r --exclude .svn log/ $PKG_PATH/var/$PKG/log
rsync -r --exclude .svn conf/ $PKG_PATH/etc/$PKG/conf
rsync -r --exclude .svn npc/ $PKG_PATH/npc
cp athena-start $PKG_PATH/	
mv *-server* $PKG_PATH/bin/

ln -fs $PKG_PATH/var/$PKG/db/ $PKG_PATH/db
ln -fs $PKG_PATH/var/$PKG/log/ $PKG_PATH/log
ln -fs $PKG_PATH/etc/$PKG/conf/ $PKG_PATH/conf
ln -fs $PKG_PATH/athena-start /usr/bin/$PKG
for f in $(ls $PKG_PATH/bin/) ; do ln -fs $PKG_PATH/bin/$f $PKG_PATH/$f; done
echo "Installation is done you can now control server with '$PKG start'"
