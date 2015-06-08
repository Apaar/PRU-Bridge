#!/bin/bash

cd driver
make

#temporary script to help while testing driver :)
cp pru_bridge.ko ../../lib/modules/3.8.13-bone71/kernel/drivers/remoteproc
depmod -a
echo BB-PRUBRIDGE > /sys/devices/bone_capemgr.*/slots
modprobe pru_bridge
