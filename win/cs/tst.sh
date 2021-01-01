#!/bin/sh
# tst.sh

rm -f *.log

./bld.sh
if [ "$?" -ne 0 ]; then exit 1; fi

IP=`cat ../ip.txt`

. ../lbm.sh

date

../win_lbmrd.bat &

../win_lbmtmon.bat &

../win_lbmwrcv.bat &

dotnet run --project Tmon/TmonExample/TmonExample.csproj $* >TmonExample.log

sleep 8

taskkill /F /IM lbmtmon.exe
taskkill /F /IM lbmrd.exe

wait

./chk.sh
exit $?
