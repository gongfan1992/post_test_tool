#! /bin/sh
#
# [$1]: Board type to test.
# [$2]: Serial number of the board.
# [$3]: Detail log file.
# [$4]: Times to test.
# [$5]: Gpio pid.
#

console_save=$6
tmp_detail="/tmp/$1-$2-xgbe02-detail"

times=0
success=0
failed=0

lines=0

mpipe_errors="0-0"

while [ -f $1-$2.lock ]                                                                                
do
sleep 0.05
done
touch $1-$2.lock
echo -e "[Test-Module]: `date +'%Y-%m-%d %T'` MPIPE-XGBE02            \t0/$4" >> $console_save
rm -rf $1-$2.lock 

sleep 20
 
# Initialize the temp detail file.
echo -n "" > $tmp_detail

taskset -c 20,21 napa_mpipe_aging --interface xgbe2 --worker 1 2>&1 | tee -a $tmp_detail >> $3 &

# Monitor the detail log for success times or any error.
# We just stop the test if we get any error.
while [ $times -ne $4 ]
do
	# We don't think that the test will get 20 dots in 5 seconds.
	sleep 5

	# Monitor the failure first.
	# We can only get dot in detail log if there isn't any error.
	error=`cat $tmp_detail | grep -E "fialed|timeout|Can't|failed"`
	if [ "$error" != "" ]
	then
			# Update the times of timeout.
			mpipe_error=`cat $tmp_detail | tail -1 | grep "loop back test"`
			if [ "$mpipe_error" != "" ]
			then
				mpipe_error=`echo $mpipe_error | awk -F ":" '{print $5}'`
				mpipe_errors="`echo $mpipe_error | awk -F "-" '{print $2}'`-`echo $mpipe_error | awk -F "-" '{print $3}'`"
			fi

        	# Wait for the access of the log.
        	while [ -f $1-$2.lock ]
        	do
                	sleep 0.05
        	done
        	touch $1-$2.lock

        	failed=`expr $success + 1`
			error="[error]"
        	sed -ie "/MPIPE-XGBE02/c\[Test-Module]: `date +'%Y-%m-%d %T'` MPIPE-XGBE02            `echo -ne "\t\033[0;31;1m${failed}${error}\033[0m"`/$4    ${mpipe_errors}        error" $console_save
        	rm -rf $1-$2.lock
#			kill -9 $5 > /dev/null 2>&1
			exit 1		
	fi

	# Number of the lines is same with the times of success.
	lines_new=`cat $tmp_detail | awk '{print NR}' | tail -1`
	if [ "$lines_new" != "" ]
	then
		if [ $lines_new -gt $lines ]
		then
			success=`expr $success + 1`

			# Update the times of timeout.
			mpipe_error=`cat $tmp_detail | tail -1 | grep "loop back test"`
			if [ "$mpipe_error" != "" ]
			then
				mpipe_error=`echo $mpipe_error | awk -F ":" '{print $5}'`
				mpipe_errors="`echo $mpipe_error | awk -F "-" '{print $2}'`-`echo $mpipe_error | awk -F "-" '{print $3}'`"
			fi

			# Wait for the access of the log.
        	while [ -f $1-$2.lock ]
        	do
                sleep 0.05
        	done
        	touch $1-$2.lock

			if [ "$mpipe_errors" == "0-0" ]
			then
        		sed -ie "/MPIPE-XGBE02/c\[Test-Module]: `date +'%Y-%m-%d %T'` MPIPE-XGBE02             `echo -ne "\t\033[0;32;1m$success\033[0m"`/$4    ${mpipe_errors}" $console_save
			else
				sed -ie "/MPIPE-XGBE02/c\[Test-Module]: `date +'%Y-%m-%d %T'` MPIPE-XGBE02             `echo -ne "\t\033[0;31;1m$success\033[0m"`/$4    ${mpipe_errors}        error" $console_save
			fi
			rm -rf $1-$2.lock

			lines=$lines_new
			times=`expr $times + 1`
		fi
	fi

done

#sed -ie "/MPIPE-XGBE02/c\[Test-Module]: `date +'%Y-%m-%d %T'` MPIPE-XGBE02     `echo -ne "\t\033[0;32;1mFinished\033[0m"`/$4" $console_save

exit 0
