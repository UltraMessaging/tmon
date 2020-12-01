# labbox.sh

CURDIR=`pwd`
SUBDIR=`basename $CURDIR`

if [ "$SUBDIR" = "mac" ]; then :
  # Inside "mac" directory, go to main directory.
  cd ..
  CURDIR=`pwd`
  SUBDIR=`basename $CURDIR`
fi

ROOT=`dirname $CURDIR`
WORKSPACE=`basename $ROOT`
labbox backup_exclude/labbox/$WORKSPACE/$SUBDIR
