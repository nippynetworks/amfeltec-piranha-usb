#!/bin/bash

modprobe -r amf_usb 2>/dev/null
modprobe -r cp2101 2>/dev/null
modprobe -r cp210X 2>/dev/null

if [ -e /sys/module/amf_usb ]; then

	echo
	echo "Warning: Amfeltec driver is loaded"
	echo "         Please remove Amfeltec driver from the kernel"
	echo
	echo "         eg: service dahdi stop"
	echo
	exit 1
fi

trap '' 2

modprobe amf_usb firmwareupdate=1 2>/dev/null

./usb_fw_up "-fwupdate"

modprobe -r amf_usb 2> /dev/null


