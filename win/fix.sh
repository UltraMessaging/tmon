#!/bin/sh
# fix.sh

if [ -d "win" ]; then :
  cd win
fi

cp ../*.cfg .

# Get the Windows interface IP address.
IP=`ipconfig | sed -n -e '/ 10\.[0-9]*\.[0-9]*\.[0-9]*/d; s/.*IPv4[^0-9]*\([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*\).*/\1/p'`

# Update the config files for this IP address.
sed -i.bak -e "s/interface [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/interface $IP/;s/daemon [0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/daemon $IP/; s/epoll/wincompport/" *.cfg

echo 'lbmrd -i '"$IP"' -p 12030 -L lbmrd.log -r 8388608' >win_lbmrd.bat

# For lbmtmon, use the same transport options in the config file.
TOPTS=`sed -n <application.cfg 's/context monitor_transport_opts //p'`

echo '"c:\users\sford\documents\visual studio 2013\Projects\tmon\x64\Debug\lbmtmon.exe" --transport-opts="'"$TOPTS"'" >lbmtmon.log' >win_lbmtmon.bat

echo 'lbmwrcv -c mon.cfg -E -v -v "/29west/.*" >lbmwrcv.log' >win_lbmwrcv.bat

echo '"c:\users\sford\documents\visual studio 2013\Projects\tmon\x64\Debug\tmon_example.exe" application.cfg >tmon_example.log' >win_tmon_example.bat

chmod +x *.bat
