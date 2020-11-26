#!/bin/sh
# tst_valgrind.sh

rm -f *.log

./bld.sh
if [ $? -ne 0 ]; then exit 1; fi
IP=`cat ip.txt`

. $HOME/lbm.sh

date

valgrind --leak-check=full ./tmon_example sford.cfg
