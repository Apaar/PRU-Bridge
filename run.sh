#!/bin/bash

echo BB-PRUBRIDGE > /sys/devices/bone_capemgr.*/slots
modprobe pru_bridge

