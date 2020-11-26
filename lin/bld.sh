#!/bin/sh
# bld.sh

if [ ! -f "lbm.relpath" ]; then :
  echo -n "Product (UMS, UMP, UMQ): "
  read PROD
  echo -n "Version (6.5, 6.13.1, etc): "
  read VERS
  skeleton_init.sh $PROD $VERS
fi

. $HOME/lbm.sh

gcc -g -Wall -I$LBM_PLATFORM/include -I../c -L$LBM_PLATFORM/lib -llbm -lm -c ../c/tmon.c
if [ $? -ne 0 ]; then exit 1; fi

gcc -g -Wall -I$LBM_PLATFORM/include -I../c -L$LBM_PLATFORM/lib -llbm -lm -o tmon_example tmon.o ../c/tmon_example.c
if [ $? -ne 0 ]; then exit 1; fi

gcc -g -I$LBM_PLATFORM/include -I../c -L$LBM_PLATFORM/lib -llbm -lm -o lbmtmon tmon.o ../c/lbmtmon.c
if [ $? -ne 0 ]; then exit 1; fi

cp ../*.cfg .

# Get 10.29.3 interface address
IP=`ifconfig | sed -n -e 's/.*inet \(10\.29\.3\.[0-9]*\).*/\1/p'`
echo "$IP" >ip.txt

sed -i'' -e "s/interface [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/interface $IP/;s/daemon [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/daemon $IP/" *.cfg
