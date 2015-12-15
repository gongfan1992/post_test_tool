#!/bin/sh
#gpio28=`./clib/gpio_single --pin 28 --input | awk '{print $2}'`
#gpio29=`./clib/gpio_single --pin 29 --input | awk '{print $2}'`
#gpio_sum=`expr $gpio28 + $gpio29`
#if [ $gpio_sum -ne 0 ]
#then
#    exit 0
#fi
#serial=`cat /sys/hypervisor/board/board_serial`

gpio_sum=0

if [ $gpio_sum -ne 0 ]
then
	serial='PSD1031013c0034'
	detail=./log/${serial}-post-test.detail
	echo "############################### Start `date` ##################################" >> $detail
	./basic_script/do_post_test.sh >> $detail
	echo "############################### End   `date` ##################################" >> $detail
else
	./box_script/board-test.tile
fi


