#!/bin/sh
# bld.sh

. lbm.sh

gcc -g -Wall -I$LBM_PLATFORM/include -I.. -L$LBM_PLATFORM/lib -llbm -lm -c ../tmon.c
if [ $? -ne 0 ]; then exit 1; fi

gcc -g -Wall -I$LBM_PLATFORM/include -I.. -L$LBM_PLATFORM/lib -llbm -lm -o tmon_example tmon.o ../tmon_example.c
if [ $? -ne 0 ]; then exit 1; fi

gcc -g -I$LBM_PLATFORM/include -I.. -L$LBM_PLATFORM/lib -llbm -lm -o lbmtmon tmon.o ../lbmtmon.c
if [ $? -ne 0 ]; then exit 1; fi

cp ../*.cfg .

# Get 10.29.3 interface address
IP=`ifconfig | sed -n -e '/ 10\.[0-9]*\.[0-9]*\.[0-9]*/d; / 127\.0\.0\.1/d; s/.*inet \([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\).*/\1/p'`
echo "$IP" >ip.txt

sed -i '' -e "s/interface [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/interface $IP/;s/daemon [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/daemon $IP/; s/epoll/select/" *.cfg