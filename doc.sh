#!/bin/sh
# doc.sh

TMON_ROOT=`pwd`

rm -rf html
mkdir html

# Pass 1 (ignore errors and html output)
cd $TMON_ROOT/manual
rm -rf html
doxygen >manual1.log 2>&1

# cd $TMON_ROOT/java
# rm -rf html
# doxygen >java1.log 2>&1

cd $TMON_ROOT/c
rm -rf html
doxygen >c1.log 2>&1

# Second pass (pay attention to errors)
rm -rf html
cd $TMON_ROOT/manual
doxygen >manual2.log 2>&1
mv html $TMON_ROOT/html/manual
sed <manual2.log '/^[A-Za-z][a-z]*ing /d;/^lookup cache used/d;/^finished/d;/^Add /d;/^Search /d'

# rm -rf html
# cd $TMON_ROOT/java
# doxygen >java2.log 2>&1
# mv html $TMON_ROOT/html/java
# sed <java2.log '/^[A-Za-z][a-z]*ing /d;/^lookup cache used/d;/^finished/d;/^Add /d;/^Search /d'

rm -rf html
cd $TMON_ROOT/c
doxygen >c2.log 2>&1
mv html $TMON_ROOT/html/c
sed <c2.log '/^[A-Za-z][a-z]*ing /d;/^lookup cache used/d;/^finished/d;/^Add /d;/^Search /d'
