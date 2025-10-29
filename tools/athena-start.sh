#!/bin/sh

#source var/function
. ./function.sh
inst_launch_workaround

PATH=./:$PATH
LOG_DIR="./log"

print_start() {
	#    more << EOF
	echo "rAthena is starting..."
	echo "            (c) 2021 rAthena Project"
	echo ""
	echo ""
	echo "checking..."
	#EOF
}

get_status(){
	PIDFILE=.$1.pid
	if [ -e ${PIDFILE} ]; then
		PSRUN=$(pgrep -F ${PIDFILE})
	fi
	#return ${PSRUN} #seems to cause an issue for some os
}

#checking if already started, launch and mark in log
start_serv(){ 
	get_status $1
	if [ $2 ]; then #is logging on ?
		LOGFILE="$LOG_DIR/$1.launch.log"
		LOGRUN="$LOG_DIR/$1.log"
		FIFO="$1_fifo"
		echo "stat_serv, log is enabled"
		echo "My logfile=${LOGFILE}"
		if [ -z ${PSRUN} ]; then
			if [ -e ./${FIFO} ]; then rm "$FIFO"; fi
			mkfifo "$FIFO"; tee "$LOGRUN" < "$FIFO" & "./$1" > "$FIFO" 2>&1 & PID=$!
			#"./$1" > >(tee "$LOGRUN") 2>&1 & PID=$! #bash only
			echo "$PID" > .$1.pid
			echo "Server '$1' started at $(date +"%m-%d-%H:%M-%S")" | tee ${LOGFILE}
		else
			echo "Cannot start '$1', because it is already running p${PSRUN}" | tee ${LOGFILE}
		fi
	else
		if [ -z ${PSRUN} ]; then
			./$1&
			echo "$!" > .$1.pid
			echo "Server '$1' started at $(date +"%m-%d-%H:%M-%S")"
		else
			echo "Cannot start '$1', because it is already running p${PSRUN}"
		fi
	fi
}

watch_serv(){
	ulimit -Sc unlimited

	#now checking status and looping
	count=0;
	while true; do
		for i in ${L_SRV} ${C_SRV} ${M_SRV} ${W_SRV}
		do
			LOGFILE="$LOG_DIR/$i.launch.log"
			LOGRUN="$LOG_DIR/$i.log"
			FIFO=$i"_fifo"

			get_status ${i}
			#echo "Echo id of $i is ${PSRUN}"
			if [ -z ${PSRUN} ]; then
				count=$((count+1))
				#echo "fifo=$FIFO"
				echo "server '$i' is down"
				echo "server '$i' is down" >> ${LOGFILE}
				echo "restarting server at time at $(date +"%m-%d-%H:%M-%S")"
				echo "restarting server at time at $(date +"%m-%d-%H:%M-%S")" >> ${LOGFILE}
				if [ -e $FIFO ]; then rm $FIFO; fi
				mkfifo "$FIFO"; tee "$LOGRUN" < "$FIFO" & "./$i" > "$FIFO" 2>&1 & PID=$!
				echo "$PID" > .$i.pid
				if [ $2 ] && [ $2 -lt $count ]; then break; fi
			fi
		done
		sleep $1
	done
}

restart(){
	$0 stop
	if [ $1 ]; then sleep $1; fi
	for i in ${L_SRV} ${C_SRV} ${M_SRV} ${W_SRV}
	do
		FIFO="$1_fifo"
		while true; do
			get_status ${i}
			if [ ${PSRUN} ]; then echo "'${i}' is still running p${PSRUN} waiting for the process to end"; sleep 2;
			else 
				if [ -e ./${FIFO} ]; then rm "$FIFO"; fi
				break
			fi
		done
	done
	$0 start
}

case $1 in
	'start')
		print_start
		check_files
		echo "Check complete."
		echo "Looks like a good, nice rAthena!"
		if [ "$2" = "--enlog" ]; then
		 ENLOG=1
		 if [ ! -d "$LOG_DIR" ]; then mkdir -p $LOG_DIR; fi
		 echo "Logging is enabled in $LOG_DIR"
		else
		 echo "Logging is disabled"
		fi
		for i in ${L_SRV} ${C_SRV} ${M_SRV} ${W_SRV}
		do
			start_serv $i $ENLOG
		done
		echo "rAthena was started."
	;;
	'watch')
		if [ ! -d "$LOG_DIR" ]; then mkdir -p $LOG_DIR; fi
		if [ -z $2 ]; then Restart_count=10; else Restart_count=$2; fi
		if [ -z $3 ]; then Restart_sleep=3; else Restart_sleep=$3; fi
		echo "Going to watch rAthena for restart_count = $Restart_count, restart_sleep = $Restart_sleep"
		for i in ${L_SRV} ${C_SRV} ${M_SRV} ${W_SRV}
		do
			start_serv $i 1
		done
		watch_serv $Restart_count $Restart_sleep
		echo "Watching rAthena now."
	;;
	'stop')
		for i in ${W_SRV} ${M_SRV} ${C_SRV} ${L_SRV}
		do
			PIDFILE=.${i}.pid
			if [ -e ./${PIDFILE} ]; then
				kill $(cat ${PIDFILE})
				
				while true; do
					get_status ${i}
					if [ ${PSRUN} ]; then echo "'${i}' is still running p${PSRUN} waiting for the process to end"; sleep 2;
					else
						break
					fi
				done
				
				rm ${PIDFILE}
			fi
		done
	;;
	'restart')
		 restart "$@"
	;;
	'status')
		for i in ${L_SRV} ${C_SRV} ${M_SRV} ${W_SRV}
		do
			get_status ${i}
			if [ ${PSRUN} ]; then echo "'${i}' is running p${PSRUN}"; else echo "'${i}' seems to be down"; fi
		done
	;;
	'val_runonce')
		for i in ${L_SRV} ${C_SRV} ${M_SRV} ${W_SRV}
		do
			valgrind --leak-check=full --show-leak-kinds=all ./$i --run-once > "log/$i.runonce.leak"
		done
	;;
	'valchk')
		for i in ${L_SRV} ${C_SRV} ${M_SRV} ${W_SRV}
		do
			valgrind --leak-check=full --show-leak-kinds=all ./$i > "log/$i.runonce.leak"
		done
	;;
	'help')
		case $2 in
			'start')
				echo "syntax: 'start {--enlog}'"
				echo "This option will start the servers"
				echo "--enlog will write all terminal output into a log/\$servname.log file"
			;;
			'stop')
				echo "This option will shut the servers down"
			;;
			'restart')
				echo "syntax: 'restart {<delay>}'"
				echo "This option will wait for the given delay and will attempt to restart the servers afterwards"
				echo "Note: Even if the delay is over it will wait until the pid is finished before attempting to restart the servers"
			;;
			'status')
				echo "syntax: 'status'"
				echo "This option will let you know whether the server are running or not"
				echo "Note: This option is based on PID and requires that you have launched the servers with this script too"
				echo "If this was not the case please use something like 'ps ax | grep server' to check their status"
			;;
			'watch')
				echo "syntax: 'watch {<restart_interval> <restart_count>}'"
				echo "The watch option allows you to automatically restart the servers when one of them was stopped"
				echo "<restart_interval> delay in seconds before rechecking if a server is down (default 10) "
				echo "<restart_count> how many times the servers should be restarted (default 3), (-1=indefinitly)"
			;;
			'val_runonce')
				echo "syntax: 'val_runonce'"
				echo "This option will run valgrind with run-once to check the servers"
			;;
			'valchk')
				echo "syntax: 'valchk'"
				echo "This option will run valgrind with the servers"
			;;
			*)
				echo "Please specify a command you would like more info on { start | stop | restart | status | watch }"
				read -p "Enter a valid command: " readEnterKey
				$0 "help" $readEnterKey
			;;
		esac
	;;
	*)
		echo "Usage: athena-start { start | stop | restart | status | watch | help | val_runonce | valchk }"
		read -p "Enter a valid option: " readEnterKey
		$0 $readEnterKey
	;;
esac


