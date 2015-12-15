#!/bin/sh
passes=10
device=/dev/ttyS0
./clib/uart_tile --dev $device --passes $passes
if [ $? -ne 0 ]
then
    exit 1
else
    exit 0
fi
