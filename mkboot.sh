#!/bin/bash -p
#
# (c) 2013, Amfeltec Corp.
#

ROOT=
TMPFILE=/tmp/tmp.$$
INITD=/etc/init.d
MODULES_FILE=/etc/dahdi/modules
INIT_FILE=/etc/dahdi/init.conf

system=redhat  # assume redhat
if [ -f /etc/debian_version ]; then
    system=debian
fi

if [ -d /etc/rc.d/init.d ]; then
	INITD=/etc/rc.d/init.d
elif [ -d  /etc/init.d ]; then
	INITD=/etc/init.d
else
	echo "Error: No /etc/init.d directory found!"
	exit 1
fi

err=1;
SNAME="dahdi"
CNAME="Dahdi"


# first, create an empty module file if it does not exist
touch $MODULES_FILE

# Just add a couple of strings to dahdi/modules:
if [ -f $MODULES_FILE ]; then

	if grep -Fxq "amf_usb" "$MODULES_FILE"
	then
		echo	
		echo "-----------------------------------------------------"
		echo "Amfeltec USB Device Boot is already configured!"
		echo "Please check file /etc/dahdi/modules"
		echo
		err=1;
	else
		echo  >> $MODULES_FILE
		echo "# Amfeltec USB Driver" >> $MODULES_FILE
		echo "amf_usb" >> $MODULES_FILE
		err=0;
	fi
fi

# Just add a couple of strings to dahdi/init.conf:
if [ -f $INIT_FILE ]; then

	if grep -Fxq "DAHDI_UNLOAD_MODULES=\"amf_usb\"" "$INIT_FILE"
	then
		echo	
		echo "-----------------------------------------------------"
		echo "Amfeltec USB Device Unload is already configured!"
		echo "Please check file /etc/dahdi/init.conf"
		echo
		err=2;
	else
		echo  >> $INIT_FILE
		echo "# Amfeltec USB Driver" >> $INIT_FILE
		echo "DAHDI_UNLOAD_MODULES=\"amf_usb\"" >> $INIT_FILE
		err=0;
	fi
fi


if [ $err -eq 0 ]; then
	echo
	echo "Amfeltec USB driver Boot Setup Complete!"
	echo "-----------------------------------------------------"
	echo
	echo "The amf_usb module was added to /etc/dahdi/modules and /etc/dahdi/init.conf lists."
	echo
	echo "Simply start $SNAME using: $INITD/$SNAME start or service dahdi start"
	echo
	err=0
fi

if [ $err -eq 3 ]; then
	echo
	echo "No DAHDI system files found: $INIT_FILE $MODULES_FILE"
	echo "-----------------------------------------------------"
	echo
	echo
	err=0
fi


exit $err

