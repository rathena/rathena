#/bin/sh
#Hi my naem is Kirt and I liek anime

ulimit -Sc unlimited

while [ 1 ] ; do
if [ -f .stopserver ] ; then
echo server marked down >> servlog.txt
else
echo restarting server at time at `date +"%m-%d-%H:%M-%S"`>> startlog.txt
./map-server
fi

sleep 5

done
