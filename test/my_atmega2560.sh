#!/bin/sh
DEVICE=/dev/ttyACM0
DIR=`dirname $0`

make
avrdude -c avrisp -p atmega2560 -P $DEVICE -b 115200 -U flash:w:$DIR/my.hex -v
#stty -F $DEVICE raw icanon eof \^d 38400
#cat < $DEVICE
