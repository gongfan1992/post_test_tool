#!/bin/sh
./clib/mpipe_mutual --sender xgbe2 --recver xgbe2 --second 10 --config ./clib/generator.conf
if [ $? -ne 0 ]
then
    exit 1
else
    exit 0
fi
