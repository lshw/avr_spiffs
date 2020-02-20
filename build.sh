#!/bin/bash
mkdir -p /tmp/build
chmod 777 /tmp/build
arduino=/home/liushiwei/arduino
home=/home/liushiwei
sketchbook=.

touch /tmp/build/info.log
$arduino/arduino-builder -dump-prefs -logger=machine -hardware "$arduino/hardware" -hardware "$home/.arduino15/packages" -tools "$arduino/tools-builder" -tools "$arduino/hardware/tools/avr" -tools "$home/.arduino15/packages" -built-in-libraries "$arduino/libraries" -libraries "$home/sketchbook/libraries" -fqbn=arduino:avr:mega:cpu=atmega2560l737 -ide-version=10609 -build-path "/tmp/build" -warnings=none -prefs=build.warn_data_percentage=75 -verbose "$sketchbook/avr_spiffs.ino"
$arduino/arduino-builder -compile -logger=machine -hardware "$arduino/hardware" -hardware "$home/.arduino15/packages" -tools "$arduino/tools-builder" -tools "$arduino/hardware/tools/avr" -tools "$home/.arduino15/packages" -built-in-libraries "$arduino/libraries" -libraries "$home/sketchbook/libraries" -fqbn=arduino:avr:mega:cpu=atmega2560l737 -ide-version=10609 -build-path "/tmp/build" -warnings=none -prefs=build.warn_data_percentage=75 -verbose "$sketchbook/avr_spiffs.ino" |tee /tmp/build/info.log

if [ $? == 0 ] ; then
grep "Global vari" /tmp/build/info.log |awk -F[ '{printf $2}'|tr -d ']'|awk -F' ' '{print "内存：使用"$1"字节,"$3"%,剩余:"$4"字节"}'
grep "Sketch uses" /tmp/build/info.log |awk -F[ '{printf $2}'|tr -d ']'|awk -F' ' '{print "ROM：使用"$1"字节,"$3"%"}'

fi
