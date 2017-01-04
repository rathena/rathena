#!/bin/sh
#source var/function
. ./function.sh
echo "My pkg path is $PKG_PATH"

check_inst_right
read -p "WARNING: This script is experimental. Press Ctrl+C to cancel or Enter to continue." readEnterKey
case $1 in
	'bin')
		echo "Starting binary cleanup"
		rm -rf $PKG_PATH/bin/*
		echo "Binary files have been deleted"
	;;
	'all')
		echo "Starting uninstall"
		rm -rf $PKG_PATH
		rm -rf /usr/bin/$PKG
		echo "Uninstallation has succeed"
	;;
	*)
		echo "Usage: Please enter a target './uninstall { all | bin }'"
esac
