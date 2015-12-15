#!/bin/sh
passes=10
i2c=/dev/i2c-0
slave=0x51
bytes=100
./clib/rom_write --i2c $i2c --slave $slave --word_offset --offset 0x100 --bytes $bytes --passes $passes
if [ $? -ne 0 ]
then
    exit 1
else
    exit 0
fi
