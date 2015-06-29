#!/bin/sh
#make driver
cd src/driver
make
make install
cd ../..

#make pru_firmware
cd src/firmware
make
make install
cd ../..

#make dts
cd src/dts
make
make install
cd ../..
