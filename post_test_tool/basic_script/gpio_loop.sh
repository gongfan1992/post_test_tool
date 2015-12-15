#!/bin/sh
passes=10
./clib/gpio_loop --passes $passes
if [ $? -ne 0 ]
then
    exit 1
else
    exit 0
fi
