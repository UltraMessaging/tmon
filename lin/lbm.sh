#!/bin/sh
# lbm.sh

if [ ! -f "lbm.relpath" ]; then :
  echo -n "Product (UMS, UMP, UMQ): "
  read PROD
  echo -n "Version (6.5, 6.13.1, etc): "
  read VERS
  lbm_init.sh $PROD $VERS
fi

. $HOME/lbm.sh

export LBMCS_DLL="$LBM_PLATFORM/bin/dotnet/lbmcs.dll"
