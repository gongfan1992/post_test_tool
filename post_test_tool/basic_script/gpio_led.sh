#!/bin/sh
passes=10
mode=1
./clib/gpio_led --passes 10 --mode $mode
if [ $? -ne 0 ]
then
    exit 1
else
    exit 0
fi
