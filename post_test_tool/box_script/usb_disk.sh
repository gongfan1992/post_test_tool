#! /bin/sh
#
# [$1]: Board type to test.
# [$2]: Serial number of the board.
# [$3]: Detail log file.
# [$4]: Times to test.
# [$5]: Gpio pid.
#

console_save=$6

success0=0
failed0=0

times=0

while [ -f $1-$2.lock ]                                                                                
do
sleep 0.05
done
touch $1-$2.lock
echo -e "[Test-Module]: `date +'%Y-%m-%d %T'` USB_DISK             \t0/$4" >> $console_save
rm -rf $1-$2.lock 

sleep 20


while [ $times -ne $4 ]
do
	if [ $failed0 -eq 0 ]
	then
	        USB_DISK 1>>$3 2>&1
		if [ $? -eq 0 ] 
		then
			# Wait for the access of the log.
        	while [ -f $1-$2.lock ]
        	do
                	sleep 0.05
        	done
        	touch $1-$2.lock

			success0=`expr $success0 + 1`
        	sed -ie "/USB_DISK/c\[Test-Module]: `date +'%Y-%m-%d %T'` USB_DISK             `echo -ne "\t\033[0;32;1m$success0\033[0m"`/$4" $console_save
			
			rm -rf $1-$2.lock
		else
			# Update the failed counter then we wouldn't start test on this channel later.
			failed0=`expr $success0 + 1`
			
        	# Wait for the access of the log.
        	while [ -f $1-$2.lock ]
        	do
                	sleep 0.05
        	done
        	touch $1-$2.lock

        	sed -ie "/USB_DISK/c\[Test-Module]: `date +'%Y-%m-%d %T'` USB_DISK             `echo -ne "\t\033[0;31;1m$failed0\033[0m"`/$4        error" $console_save
        	rm -rf $1-$2.lock
#			kill -9 $5 > /dev/null 2>&1
		fi 
	fi


	times=$(expr $times + 1)
done

exit 0
