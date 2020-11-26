#!/bin/sh
# tst_loss.sh

rm *.log

./bld.sh
if [ $? -ne 0 ]; then exit 1; fi

. ./lbm.sh

date

lbmrd -i 10.29.3.43 -p 12030 -L lbmrd.log -r 8388608 &
LBMRD_PID=$!

./lbmtmon --transport-opts="config=mon.cfg" >lbmtmon.log &
LBMMON_PID=$!

# Receive raw monitoring messages
lbmwrcv -c mon.cfg -E -v -v "/29west/.*" >lbmwrcv.log &
LBMRCV_PID=$!

LBTRM_SRC_LOSS_RATE=50 ./tmon_example sford.cfg 2 >tmon_example.log
RCV_STATS_PID=$!

sleep 8

kill $LBMMON_PID
kill $LBMRD_PID

wait