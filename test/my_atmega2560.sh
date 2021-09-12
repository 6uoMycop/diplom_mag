#!/bin/sh
DEVICE=/dev/ttyACM1
DIR=`dirname $0`

make
avrdude -cstk500v2 -p atmega2560 -P $DEVICE -b 115200 -D -U flash:w:$DIR/my.hex -v
stty -F $DEVICE raw icanon eof \^d 38400
cat < $DEVICE
