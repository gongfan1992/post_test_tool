#! /bin/sh

total=`cat /proc/meminfo | grep MemTotal | sed 's/MemTotal:\s*\(.*\) kB/\1/g'`
size=`expr $total - 2 \* 1024 \* 1024`
size=`expr $size \* 512`
passes=20
taskset -c 5-20 memtest --passes $passes --size $size 

if [ $? -ne 0 ]
then
	echo "[MEM-CPU-`date +'%Y-%m-%d %T'`]: MEM-CPU test failed."
	exit 1
else
	
	echo "[MEM-CPU-`date +'%Y-%m-%d %T'`]: MEM-CPU test success."
	exit 0
fi


