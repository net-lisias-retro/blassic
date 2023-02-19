#!/bin/sh
# Simple demo of blassic from command line.
#

i=0
while [ ! $i = 10 ]
do
	i=`./blassic -e "print $i + 1" `
	echo $i
done
