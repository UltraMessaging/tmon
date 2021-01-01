#!/bin/sh
# clean.sh

rm -rf */lbm.platform */lbm.relpath */*.o */tmon_example */lbmtmon
for D in win lin mac; do :
  rm -rf $D/cs/Tmon $D/*.cfg $D/cs/*.cfg
done
rm -f `find . -name 'ip.txt' -print`
rm -f `find . -name '*.log' -print`
rm -f `find . -name '*.o' -print`
rm -rf `find . -name '*.dSYM' -print`
rm -rf `find . -name '.DS_Store' -print`
rm -f `find . -name '*.pid' -print`
rm -f `find . -name '*.tst' -print`
rm -f `find . -name '*.bak' -print`
