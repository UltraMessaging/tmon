#!/bin/sh
# lbm.sh

# PATH is set up when UM is installed.

DOTNET_PATH=`find /cygdrive/c/Program\ Files/Informatica -name 'dotnet' -print | head -1`
BIN_PATH=`dirname "$DOTNET_PATH"`
export LBM_PLATFORM=`dirname "$BIN_PATH"` 

export LBMCS_DLL="c:\\\\Program Files\\\\Informatica\\\\UMS_6.13.1\\\\Win2k-x86_64\\\\bin\\\\dotnet\\\\lbmcs.dll"
