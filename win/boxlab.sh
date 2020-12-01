#!/bin/sh
# boxlab.sh

CURDIR=`pwd`
SUBDIR=`basename $CURDIR`

if [ "$SUBDIR" = "win" ]; then :
  # Inside "win" directory, go to main directory.
  cd ..
  CURDIR=`pwd`
  SUBDIR=`basename $CURDIR`
fi

ROOT=`dirname $CURDIR`
WORKSPACE=`basename $ROOT`
# echo "CURDIR=$CURDIR, SUBDIR=$SUBDIR, ROOT=$ROOT, WORKSPACE=$WORKSPACE"

ILINE=""
while [ "$ILINE" != "q" ]; do :
  rsync -a --exclude "lin" --exclude "x" --exclude "*.o" --exclude "x.*" -zq orion.29west.com:backup_exclude/labbox/$WORKSPACE/$SUBDIR/ ./

  ./win/cp_files.sh

  echo -n "boxlab.sh (return or q for quit): "
  read ILINE
done
