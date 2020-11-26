#!/bin/sh
# boxlab.sh

if [ ! -d "win" ]; then :
  echo "Must run win/boxlab.sh from the tmon directory." >&2
  exit 1
fi

ILINE=""

while [ "$ILINE" != "q" ]; do :
  echo 'rsync -a --exclude "lin" --exclude "x" --exclude "*.x" --exclude "x.*" -zq orion.29west.com:backup_exclude/labbox/src/tmon/ ./'

  rsync -a --exclude "lin" --exclude "x" --exclude "*.o" --exclude "x.*" -zq orion.29west.com:backup_exclude/labbox/src/tmon/ ./

  ./win/cp_files.sh

  echo -n "boxlab.sh (return or q for quit): "
  read ILINE
done
