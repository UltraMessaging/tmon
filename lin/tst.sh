#!/bin/sh
# tst.sh

rm -f *.log *.pid

./bld.sh
if [ $? -ne 0 ]; then exit 1; fi

IP=`cat ip.txt`

. ./lbm.sh

date

lbmrd -i $IP -p 12030 -L lbmrd.log -r 8388608 &
echo $! >lbmrd.pid

# For lbmtmon, use the same transport options in the config file.
TOPTS=`sed -n <application.cfg 's/context monitor_transport_opts //p'`
./lbmtmon --transport-opts="$TOPTS" >lbmtmon.log &
echo $! >lbmtmon.pid

# Receive raw monitoring messages
lbmwrcv -c mon.cfg -E -v -v "/29west/.*" >lbmwrcv.log &
echo $! >lbmwrcv.pid

./tmon_example application.cfg >tmon_example.log &
echo $! >tmon_example.pid
wait `cat tmon_example.pid`

sleep 8

kill `cat lbmtmon.pid`
kill `cat lbmrd.pid`

wait
