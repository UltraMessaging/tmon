# source this file
# lbm.sh - point to your installation of UM

LBM_BASE=$HOME/UMS_6.5; export LBM_BASE
TARGET_PLATFORM=Darwin-10.8.0-x86_64; export TARGET_PLATFORM
LBM_PLATFORM=$LBM_BASE/$TARGET_PLATFORM; export LBM_PLATFORM
DYLD_LIBRARY_PATH=$LBM_PLATFORM/lib; export DYLD_LIBRARY_PATH
PATH="$LBM_PLATFORM/bin:$PATH"; export PATH

LBM_LICENSE_INFO="Product=LBM,UME,UMQ,UMDRO:Organization=UM RnD sford (RnD):Expiration-Date=never:License-Key=7BAD 694F FF7C 9FA6"; export LBM_LICENSE_INFO
