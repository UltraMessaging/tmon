#!/bin/sh
# tst.sh

./fix.sh
rm -f *.log

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

# We *don't* want to see this string.
if egrep "^Unrecognized" lbmtmon.log >/dev/null
then echo "Check 13: FAIL" >>tst.log
else echo "Check 13: ok" >>tst.log
fi

OKS=`egrep ": ok" tst.log | wc -l`
echo "$OKS checks ok"

ERRS=`egrep ": FAIL" tst.log | wc -l`
if [ "$ERRS" -gt 0 ]; then :
  echo "ERROR: $ERRS checks failed! (see tst.log)" >&2
  exit 1
fi
