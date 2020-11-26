#!/bin/sh
# tst.sh

rm -f *.log

./bld.sh
if [ $? -ne 0 ]; then exit 1; fi
IP=`cat ip.txt`

. ./lbm.sh

date

lbmrd -i $IP -p 12030 -L lbmrd.log -r 8388608 &
LBMRD_PID=$!

./lbmtmon --transport-opts="config=mon.cfg" >lbmtmon.log &
LBMMON_PID=$!

# Receive raw monitoring messages
lbmwrcv -c mon.cfg -E -v -v "/29west/.*" >lbmwrcv.log &
LBMRCV_PID=$!

./tmon_example sford.cfg >tmon_example.log
RCV_STATS_PID=$!

sleep 8

kill $LBMMON_PID
kill $LBMRD_PID

wait
