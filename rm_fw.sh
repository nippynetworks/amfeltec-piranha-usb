#!/bin/sh

for f in `locate USB_FX  | grep -v svn`
do
	rm -f  $f
#	echo `basename $f`
	ls $f
done
