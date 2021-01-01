#!/bin/sh
# bld.sh

. ./lbm.sh

rm -f tmon.o tmon_example lbmtmon

gcc -g -Wall -I$LBM_PLATFORM/include -I../c -L$LBM_PLATFORM/lib -llbm -lm -c ../c/tmon.c
if [ $? -ne 0 ]; then exit 1; fi

gcc -g -Wall -I$LBM_PLATFORM/include -I../c -L$LBM_PLATFORM/lib -llbm -lm -o tmon_example tmon.o ../c/tmon_example.c
if [ $? -ne 0 ]; then exit 1; fi

gcc -g -I$LBM_PLATFORM/include -I../c -L$LBM_PLATFORM/lib -llbm -lm -o lbmtmon tmon.o ../c/lbmtmon.c
if [ $? -ne 0 ]; then exit 1; fi

rm -f *.cfg
cp ../*.cfg .

# Get 10.29.3 interface address
IP=`ifconfig | sed -n -e 's/.*inet \(10\.29\.3\.[0-9]*\).*/\1/p'`
echo "$IP" >ip.txt

sed -i.bak -e "s/interface [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/interface $IP/;s/daemon [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/daemon $IP/" *.cfg

chmod -w *.cfg
