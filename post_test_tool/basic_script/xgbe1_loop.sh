#!/bin/sh
./clib/mpipe_mutual --sender xgbe1 --recver xgbe1 --second 10 --config ./clib/generator.conf
if [ $? -ne 0 ]
then
    exit 1
else
    exit 0
fi
