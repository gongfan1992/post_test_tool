#!/bin/sh
ifconfig xgbe1 down
ifconfig xgbe2 down
ifconfig gbe4 down
lengths="64 65 128 129 256 257 512 513 1024 1025 1028 1029 1500 1501"
gbe="gbe5 gbe6 gbe7 gbe8"
seconds=$2
if [[ $1 == "SGM" ]] ;then
	for pktlen in $lengths
	do
		echo -n "sender gbe4 recver gbe4 pktlen:$pktlen "
		./mpipe_mutual --sender gbe4 --recver gbe4 --second $seconds --timeout 100000000  --pktlen $pktlen | grep "SUCCESSED" > /dev/null 2>&1
	    if [ $? -eq 0 ]
    	then                                                                                                                                                                               
        	echo -e '\033[0;32;1mPassed\033[0m' | tee -a $console_save >&2
    	else
        	echo -e '\033[0;31;1mFailed\033[0m' | tee -a $console_save >&2
    	fi  	
	done
elif [[ $1 == "TOP" ]] ;then
	for pktlen in $lengths
	do
		echo -n "sender xgbe1 recver xgbe2 pktlen:$pktlen "
		./mpipe_mutual --sender xgbe1 --recver xgbe2 --second $seconds --timeout 100000000  --pktlen $pktlen | grep "SUCCESSED" > /dev/null 2>&1
	    if [ $? -eq 0 ]
    	then                                                                                                                                                                               
        	echo -e '\033[0;32;1mPassed\033[0m' | tee -a $console_save >&2
    	else
        	echo -e '\033[0;31;1mFailed\033[0m' | tee -a $console_save >&2
    	fi  	
	done
	for pktlen in $lengths
	do
		echo -n "sender xgbe2 recver xgbe1 pktlen:$pktlen "
		./mpipe_mutual --sender xgbe2 --recver xgbe1 --second $seconds --timeout 100000000  --pktlen $pktlen | grep "SUCCESSED" > /dev/null 2>&1
	    if [ $? -eq 0 ]
    	then                                                                                                                                                                               
        	echo -e '\033[0;32;1mPassed\033[0m' | tee -a $console_save >&2
    	else
        	echo -e '\033[0;31;1mFailed\033[0m' | tee -a $console_save >&2
    	fi  	
	done
elif [[ $1 == "BGM4" ]] ;then
	for gbek in $gbe
	do
		for pktlen in $lengths
		do
			echo -n "sender xgbe5 recver xgbe5 pktlen:$pktlen "
			./mpipe_mutual --sender $gbek --recver $gbek --second $seconds --timeout 100000000  --pktlen $pktlen | grep "SUCCESSED" > /dev/null 2>&1
	    	if [ $? -eq 0 ]
    		then                                                                                                  	
			echo -e '\033[0;32;1mPassed\033[0m' | tee -a $console_save >&2
    		else
        		echo -e '\033[0;31;1mFailed\033[0m' | tee -a $console_save >&2
    		fi  	
		done
	done
else 

	for pktlen in $lengths
	do
		echo -n "sender xgbe1 recver xgbe1 pktlen:$pktlen "
		./mpipe_mutual --sender xgbe1 --recver xgbe1 --second $seconds --timeout 100000000  --pktlen $pktlen | grep "SUCCESSED" > /dev/null 2>&1
	    if [ $? -eq 0 ]
    	then                                                                                                                                                                               
        	echo -e '\033[0;32;1mPassed\033[0m' | tee -a $console_save >&2
    	else
        	echo -e '\033[0;31;1mFailed\033[0m' | tee -a $console_save >&2
    	fi  	
	done
	for pktlen in $lengths
	do
		echo "sender xgbe2 recver xgbe2 pktlen:$pktlen "
		./mpipe_mutual --sender xgbe2 --recver xgbe2 --second $seconds --timeout 100000000  --pktlen $pktlen | grep "SUCCESSED" > /dev/null 2>&1
	    if [ $? -eq 0 ]
    	then                                                                                                                                                                               
        	echo -e '\033[0;32;1mPassed\033[0m' | tee -a $console_save >&2
    	else
        	echo -e '\033[0;31;1mFailed\033[0m' | tee -a $console_save >&2
    	fi  	
	done
fi
