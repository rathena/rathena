#!/bin/bash

athena_dir="/home/athena/658/"

while [ true ] ; do

if [ ` ps fauxw | grep map-server | grep -v grep | wc -l ` -eq 0 ];then
        #echo `date` " -- map-server crashed - restarting"
        echo `date` " -- map-server crashed - restarting" >> /var/log/athena_status.log
        killall -9 map-server
        cd $athena_dir
        nohup ./map-server ./conf/map_athena.conf ./inter_athena.conf &
        sleep 240
        #sleep 40 #for fast pc's remove the "#" at the beginning of the line and delete the line above
fi


if [ ` ps fauxw | grep map-server | grep -v grep | awk '{print $3}' | awk 'BEGIN{FS="."} {print $1}' ` -gt 10 ];then
        #echo `date` " -- mapserver cpuload over 10 - restarting"
        echo `date` " -- mapserver cpuload over 10 - restarting" >> /var/log/athena_status.log
        killall -9 map-server
        cd $athena_dir
        nohup ./map-server ./conf/map_athena.conf ./inter_athena.conf &
        sleep 240
        #sleep 40 #for fast pc's remove the "#" at the beginning of the line and delete the line above
        #echo `date` " -- restarted"
        echo `date` " -- restarted" >> /var/log/athena_status.log
fi

if [ ` ps fauxw | grep char-server | grep -v grep | wc -l ` -eq 0 ];then
        #echo `date` " -- char server crashed - restarting"
        echo `date` " -- char server crashed - restarting" >> /var/log/athena_status.log
        killall -9 char-server
        cd $athena_dir
        nohup ./char-server ./conf/char_athena.conf ./conf/inter_athena.conf &
        #echo `date` " -- restarted"
        echo `date` " -- restarted" >> /var/log/athena_status.log

fi

if [ ` ps fauxw | grep login-server | grep -v grep | wc -l ` -eq 0 ];then
        #echo `date` " -- login server crashed - restarting"
        echo `date` " -- login server crashed - restarting" >> /var/log/athena_status.log
        killall -9 login-server
        cd $athena_dir
        nohup ./login-server ./conf/login_athena.conf &
        #echo `date` " -- restarted"
        echo `date` " -- restarted" >> /var/log/athena_status.log

fi


#echo `date` " -- everything is fine"
echo `date` " -- everything is fine" >> /var/log/athena_status.log
sleep 30
done
