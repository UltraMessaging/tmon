#!/bin/sh
# tst.sh

./bld.sh
rm *.log

date

./win_lbmrd.bat &

./win_lbmtmon.bat &

# Receive raw monitoring messages
./win_lbmwrcv.bat &

./win_tmon_example.bat

sleep 8

taskkill /F /IM lbmtmon.exe
taskkill /F /IM lbmrd.exe

wait
