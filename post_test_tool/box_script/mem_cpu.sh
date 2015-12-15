#! /bin/sh
#
# [$1]: Board type to test.
# [$2]: Serial number of the board.
# [$3]: Detail log file.
# [$4]: Times to test.
# [$5]: Gpio pid.
#

console_save=$6
tmp_detail="/tmp/$1-$2-mdetail"

times=0
success=0
failed=0

success_print=0

ecc_errors="0-0"

while [ -f $1-$2.lock ]                                                                                
do
sleep 0.05
done
touch $1-$2.lock
echo -e "[Test-Module]: `date +'%Y-%m-%d %T'` MEMORY&CPU      \t\t0/$4" >> $console_save
rm -rf $1-$2.lock 

sleep 20

total=`cat /proc/meminfo | grep MemTotal | sed 's/MemTotal:\s*\(.*\) kB/\1/g'`
size=`expr $total - 2 \* 1024 \* 1024`
size=`expr $size \* 512`

# Get the expected times to test.
if [ $4 -ne -1 ]
then
	passes=`expr $4 \* 20`
else
	passes=100000000
fi
 
# Initialize the temp detail file.
echo -n "" > $tmp_detail

nodes=`cat /proc/tile/memory | grep "1_node"`
if [ "$nodes" == "" ]
then
	error="[Node Miss]"
	# Wait for the access of the log.
	while [ -f $1-$2.lock ]
	do
		sleep 0.05
	done

	touch $1-$2.lock
	sed -ie "/MEMORY&CPU/c\[Test-Module]: `date +'%Y-%m-%d %T'` MEMORY&CPU      `echo -ne "\t\t\033[0;31;1m0${error}\033[0m"`/$4" $console_save
	rm -rf $1-$2.lock
#	kill -9 $5 > /dev/null 2>&1
	exit 1
fi

taskset -c 5-20 memtest --passes $passes --size $size 2>&1 | tee -a $tmp_detail >> $3 &

# Monitor the detail log for success times or any error.
# We just stop the test if we get any error.
while [ $times -ne $4 ]
do
	# We don't think that the test will get 20 dots in 5 seconds.
	sleep 5

	# Monitor the failure first.
	# We can only get dot in detail log if there isn't any error.
	error=`cat $tmp_detail | sed '/Testing/d' | sed '/Start/d' | grep "[a-z,A-Z]"`
	if [ "$error" != "" ]
	then
        	# Wait for the access of the log.
        	while [ -f $1-$2.lock ]
        	do
                	sleep 0.05
        	done
        	touch $1-$2.lock

        	failed=`expr $success + 1`
		error="[error]"
        	sed -ie "/MEMORY&CPU/c\[Test-Module]: `date +'%Y-%m-%d %T'` MEMORY&CPU      `echo -ne "\t\t\033[0;31;1m${failed}${error}\033[0m"`/$4    ${total}KB ${ecc_errors}" $console_save
        	rm -rf $1-$2.lock
		killall memtest > /dev/null 2>&1
		kill -9 $5 > /dev/null 2>&1
		exit 1		
	fi

#	# Check the ecc error.
#	ecc_error=`find /sys/devices/system/edac/mc -name "ch*_ce_count" | xargs cat`
#	ecc_slot=0
#	for ecc in `echo $ecc_error`
#	do
#		if [ $ecc -ne 0 ]
#		then	
#        		# Wait for the access of the log.
#        		while [ -f $1-$2.lock ]
#        		do
#                		sleep 0.05
#        		done
#        		touch $1-$2.lock
#
#        		failed=`expr $success + 1`
#			error="[ecc_error:${ecc_slot}]"
#        		sed -ie "/MEMORY/c\[Test-Module]: `date +'%Y-%m-%d %T'` MEMORY      `echo -ne "\t\t\033[0;31;1m${failed}${error}\033[0m"`/$4    ${total}KB" $console_save
#        		rm -rf $1-$2.lock
#			killall memtest > /dev/null 2>&1
#			kill -9 $5 > /dev/null 2>&1
#			exit 1			
#		fi
#		
#		ecc_slot=`expr $ecc_slot + 1`
#	done
	

	# Number of the dots is same with the times of success.
	dots=`cat $tmp_detail | awk -F "" '{for(i=0;i<=NF;i++)a[$i]++}END{for(j in a)if(j==".")print a[j]}'`

	# Each 20 times of success we will print.
	#if [ $((${dots}%20)) -eq 0 ] && [ $dots -gt $dots_print ]
	dot_mod=`expr $dots / 20`
	if [ $dot_mod -gt $success_print ]
	then
		success=$dot_mod

		# Wait for the access of the log.
        	while [ -f $1-$2.lock ]
        	do
                	sleep 0.05
        	done
        	touch $1-$2.lock

            	ecc_error=`find /sys/devices/system/edac/mc -name "ch*_ce_count" | xargs cat`
            	ecc_errors="`echo $ecc_error | awk '{print $1}'`-`echo $ecc_error | awk '{print $2}'`"
				if [ "$ecc_errors" != "0-0" ]
				then
        			sed -ie "/MEMORY&CPU/c\[Test-Module]: `date +'%Y-%m-%d %T'` MEMORY&CPU      `echo -ne "\t\t\033[0;31;1m$success\033[0m"`/$4    ${total}KB ${ecc_errors}        error" $console_save
				else
        			sed -ie "/MEMORY&CPU/c\[Test-Module]: `date +'%Y-%m-%d %T'` MEMORY&CPU      `echo -ne "\t\t\033[0;32;1m$success\033[0m"`/$4    ${total}KB ${ecc_errors}" $console_save
				fi
		rm -rf $1-$2.lock

		success_print=$success
		times=`expr $times + 1`
	fi

done

#sed -ie "/MEMORY&CPU/c\[Test-Module]: `date +'%Y-%m-%d %T'` MEMORY&CPU      `echo -ne "\t\t\033[0;32;1mFinished\033[0m"`/$4" $console_save

exit 0
