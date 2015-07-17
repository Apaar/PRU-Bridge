#!/bin/bash

cd driver
make
cp pru_bridge.ko /lib/modules/3.8.13-bone70/kernel/drivers/remoteproc
depmod -a
echo BB-PRUBRIDGE > /sys/devices/bone_capemgr.*/slots
modprobe pru_bridge

