#!/bin/sh
# bld.sh

cp ../*.cfg .

# Get 10.29.3 interface address
IP=`ipconfig | sed -n -e '/ 10\.[0-9]*\.[0-9]*\.[0-9]*/d; s/.*IPv4[^0-9]*\([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\).*/\1/p'`

sed -i'' -e "s/interface [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/interface $IP/;s/daemon [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/daemon $IP/; s/epoll/wincompport/" *.cfg
sed -i'' -e "s/-i [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/-i $IP/" win_lbmrd.bat
