#!/bin/sh
#source var/function
. ./function.sh
echo "My pkg path is $PKG_PATH"

check_inst_right
read -p "WARNING: This target dis experimental. Press Ctrl+C to cancel or Enter to continue." readEnterKey
case $1 in
	'bin')
		echo "Starting binary cleanup"
		rm -rf $PKG_PATH/bin/*
		echo "Binary file was deleted"
	;;
	'all')
		echo "Starting uninstalling "
		rm -rf $PKG_PATH
		rm -rf /usr/bin/$PKG
		echo "Uninstallation succed"
	;;
	'*')
		echo "Please enter a target usage './uninstall { all | bin }'"
esac
