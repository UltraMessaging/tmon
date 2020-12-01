#!/bin/sh
# tst_loss.sh

rm -f *.log

./bld.sh
if [ $? -ne 0 ]; then exit 1; fi
IP=`cat ip.txt`

. $HOME/lbm.sh

date

lbmrd -i $IP -p 12030 -L lbmrd.log -r 8388608 &
LBMRD_PID=$!

TOPTS=`sed -n <application.cfg 's/context monitor_transport_opts //p'`
./lbmtmon --transport-opts="$TOPTS" >lbmtmon.log &
LBMMON_PID=$!

# Receive raw monitoring messages
lbmwrcv -c mon.cfg -E -v -v "/29west/.*" >lbmwrcv.log &
LBMRCV_PID=$!

LBTRM_SRC_LOSS_RATE=50 ./tmon_example application.cfg 2 >tmon_example.log
RCV_STATS_PID=$!

sleep 8

kill $LBMMON_PID
kill $LBMRD_PID

wait

rm -f tst.log

if egrep "^Source statistics received from tmon_example_1" lbmtmon.log >/dev/null
then echo "Check 1: ok" >>tst.log
else echo "Check 1: FAIL" >>tst.log
fi

if egrep "^Receiver statistics received from tmon_example_1" lbmtmon.log >/dev/null
then echo "Check 2: ok" >>tst.log
else echo "Check 2: FAIL" >>tst.log
fi

if egrep "^Context statistics received from tmon_example_1" lbmtmon.log >/dev/null
then echo "Check 3: ok" >>tst.log
else echo "Check 3: FAIL" >>tst.log
fi

if egrep "^Context statistics received from tmon_example_1" lbmtmon.log >/dev/null
then echo "Check 3: ok" >>tst.log
else echo "Check 3: FAIL" >>tst.log
fi

if egrep "^Receiver create" lbmtmon.log >/dev/null
then echo "Check 4: ok" >>tst.log
else echo "Check 4: FAIL" >>tst.log
fi

if egrep "topic=src1" lbmtmon.log >/dev/null
then echo "Check 5: ok" >>tst.log
else echo "Check 5: FAIL" >>tst.log
fi

if egrep "^Source create" lbmtmon.log >/dev/null
then echo "Check 6: ok" >>tst.log
else echo "Check 6: FAIL" >>tst.log
fi

if egrep "^Conn create" lbmtmon.log >/dev/null
then echo "Check 7: ok" >>tst.log
else echo "Check 7: FAIL" >>tst.log
fi

if egrep "^BOS" lbmtmon.log >/dev/null
then echo "Check 8: ok" >>tst.log
else echo "Check 8: FAIL" >>tst.log
fi

if egrep "^Conn delete" lbmtmon.log >/dev/null
then echo "Check 9: ok" >>tst.log
else echo "Check 9: FAIL" >>tst.log
fi

if egrep "^Receiver delete" lbmtmon.log >/dev/null
then echo "Check 10: ok" >>tst.log
else echo "Check 10: FAIL" >>tst.log
fi

if egrep "^Source delete" lbmtmon.log >/dev/null
then echo "Check 11: ok" >>tst.log
else echo "Check 11: FAIL" >>tst.log
fi

if egrep "^EOS" lbmtmon.log >/dev/null
then echo "Check 12: ok" >>tst.log
else echo "Check 12: FAIL" >>tst.log
fi

if egrep "^Loss" lbmtmon.log >/dev/null
then echo "Check 13: ok" >>tst.log
else echo "Check 13: FAIL" >>tst.log
fi

# We *don't* want to see this string.
if egrep "^Unrecognized" lbmtmon.log >/dev/null
then echo "Check 14: FAIL" >>tst.log
else echo "Check 14: ok" >>tst.log
fi

OKS=`egrep ": ok" tst.log | wc -l`
echo "$OKS checks ok"

ERRS=`egrep ": FAIL" tst.log | wc -l`
if [ "$ERRS" -gt 0 ]; then :
  echo "ERROR: $ERRS checks failed! (see tst.log)" >&2
  exit 1
fi
