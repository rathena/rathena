#!/bin/sh
echo "============================================"
echo "=       map server status checker...       ="
echo "============================================"
./map-server.exe &
sleep 40

while [ 0 ]
do
	pcpu=` top -n 1| grep map-server | awk '{print $9}' | awk 'BEGIN{FS="."} {print $1}' ` 
	if [ "$pcpu" -gt 80 ];then
		echo "============================================"
		echo "map server is more than 80% (now $pcpu%)"
		echo "============================================"
		ppid=` ps -a | grep map-server | awk '{print $1}' `
		kill $ppid
		./map-server.exe &
		sleep 40
	else
		pmapct=` ps -a| grep map-server | wc -l `
		if [ "$pmapct" -eq 0 ];then
			echo "============================================"
			echo "map server is not running..."
			echo "restart map server..."
			echo "============================================"
			./map-server.exe &
			sleep 40
			#echo "test"
		else
			echo "map server is ok (now $pcpu%)..."
			sleep 5
		fi
	fi
done