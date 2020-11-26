# labbox.sh

CURDIR=`pwd`
SUBDIR=`basename $CURDIR`
ROOT=`dirname $CURDIR`
WORKSPACE=`basename $ROOT`
labbox backup_exclude/labbox/$WORKSPACE/$SUBDIR
