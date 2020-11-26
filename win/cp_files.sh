#!/bin/sh
# cp_files.sh

if [ ! -d "win" ]; then :
  echo "Must run win/boxlab.sh from the tmon directory." >&2
  exit 1
fi

cp tmon_example.c tmon.c tmon.h /cygdrive/c/users/sford/Documents/Visual\ Studio\ 2013/Projects/tmon/tmon_example/

cp lbmtmon.c tmon.c getopt.c tmon.h monmodopts.h replgetopt.h /cygdrive/c/users/sford/Documents/Visual\ Studio\ 2013/Projects/tmon/lbmtmon/
