#!/bin/sh
# cp_files.sh

if [ "$SUBDIR" = "win" ]; then :
  # Inside "win" directory, go to main directory.
  cd ..
  CURDIR=`pwd`
  SUBDIR=`basename $CURDIR`
fi

cp c/tmon_example.c c/tmon.c c/tmon.h /cygdrive/c/users/sford/Documents/Visual\ Studio\ 2013/Projects/tmon/tmon_example/

cp c/lbmtmon.c c/tmon.c c/getopt.c c/tmon.h c/monmodopts.h c/replgetopt.h /cygdrive/c/users/sford/Documents/Visual\ Studio\ 2013/Projects/tmon/lbmtmon/
