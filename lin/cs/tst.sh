#!/bin/sh
# tst.sh

function kill_em {
  echo kill `cat *.pid`
  kill `cat *.pid`
}

ulimit -c unlimited
rm -f *.log *.pid

./bld.sh
if [ "$?" -ne 0 ]; then exit 1; fi

IP=`cat ../ip.txt`

. ../lbm.sh

date

trap "kill_em; exit 1" 1 2 3 15

lbmrd -i $IP -p 12030 -L lbmrd.log -r 8388608 &
echo $! >lbmrd.pid

# For lbmtmon, use the same transport options in the config file.
TOPTS=`sed -n <application.cfg 's/context monitor_transport_opts //p'`
../lbmtmon --transport-opts="$TOPTS" >lbmtmon.log 2>&1 &
echo $! >lbmtmon.pid

# Receive raw monitoring messages
lbmwrcv -c mon.cfg -E -v -v "/29west/.*" >lbmwrcv.log 2>&1 &
echo $! >lbmwrcv.pid

dotnet run --project Tmon/TmonExample/TmonExample.csproj $* >TmonExample.log 2>&1 &
echo $! >TmonExample.pid
wait `cat TmonExample.pid`
rm TmonExample.pid

sleep 8

wait `cat lbmwrcv.pid`
rm lbmwrcv.pid

kill_em

wait

./chk.sh
exit $?
