#!/bin/bash
if [ -x lib/rtu.sh ] ; then
cd lib
fi
killall minicom 2>/dev/null
killall avrdude 2>/dev/null
avrdude  -v  -c avrdude.conf -patmega2560 -cwiring -x snooze=250 -P/dev/ttyUSB0 -b115200 -D -Uflash:w:/tmp/build/avr_spiffs.ino.hex:i
