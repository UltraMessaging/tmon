#!/bin/sh
# bld.sh

if [ ! -d Tmon ]; then :
  ./init_proj.sh
  if [ "$?" -ne 0 ]; then exit 1; fi
fi

# Build C code and create config files.
cd ..
./bld.sh
if [ "$?" -ne 0 ]; then exit; fi
cd cs

. ../lbm.sh

cp ../../cs/Tmon/*.cs Tmon/Tmon/
cp ../../cs/TmonExample/*.cs Tmon/TmonExample/

rm -f *.cfg
cp ../*.cfg .
chmod -w *.cfg

cd Tmon
dotnet build Tmon.sln
exit $?
