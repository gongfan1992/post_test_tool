#!/bin/sh
body_type="Poseidon"
serial='PSD1031013c0034'
console_save=./log/${serial}-post-test.console
src_test_times=./clib/$body_type-times
test_times=./log/${serial}-${body_type}-times
#if [ ! -f $test_times ]
#then
    cp $src_test_times $test_times 
#fi
config_file=./config/post-test.${body_type}
do_all_times=`cat $config_file | grep "post_test_times" | awk '{print $2}'`
curr_time=1
total_times=1
errors=0

if [ "$do_all_times" == "" ]
then
    total_times=1
elif [ "$do_all_times" == "-1" ]
then
    total_times=10
else
    total_times=$do_all_times
fi
run_time=1

##############################################
# Start to test
#
##############################################
./clib/gpio_led --passes -1 --mode 1 & 
led_pid=`ps -elf | grep "gpio_led" | grep "passes" | awk '{print $4}'`

#awk '{if(NR!=1){$2=0;$3=0;}printf $0"\n"}' $test_times > .$serial-test_times_tmp ; cp -f .$serial-test_times_tmp $test_times ; rm -f .$serial-test_times_tmp


while [ "$run_time" -le "$total_times" ];
do
    #############################################
    #  insmod pcie driver
    #############################################
#    ./script/mod.sh -i 2>&1
    echo "[-----------------------Start $curr_time times TEST -------------------]" | tee -a $console_save >&2
    echo -ne "\r" >&2
    while read newline;
    do
        module_name=`echo $newline | awk -F ']' '{print $1}' | sed 's/\[//g'`
        exe=`echo $newline | awk '{print $2}' | awk -F '=' '{print $1}' | sed 's/_do//g'`
        do_or_not=`echo $newline | awk -F '=' '{print $2}' | awk '{print $1}'` 
        if [ "$do_or_not" == "do" ]
        then
            command="./basic_script/$exe.sh"
            IFS=
            test_module="[Test-Module]: $module_name              "
            add_space=`expr 50 - ${#test_module}`
            space=" "
            while [ $add_space -ne 0  ] 
            do  
                test_module="$test_module""$space"
                add_space=`expr $add_space - 1`
            done
            echo -n $test_module | tee -a $console_save >&2
            IFS=' '
            #echo $command,$exe
         	$command  2>&1
            result=$?
            new_succ_times=`cat $test_times | grep "$exe" | awk '{print $2}'`
            new_fail_times=`cat $test_times | grep "$exe" | awk '{print $3}'`
            if [ $result -eq 0  ]
            then
                new_succ_times=`cat $test_times | grep "$exe" | awk '{print $2=$2+1}'`
                new_line=`cat $test_times | grep "$exe" | awk '{printf("%-15s%d\t%d\t\n", $1, $2+1, $3)}'`
            else
		errors=`expr $errors + 1` 
                new_fail_times=`cat $test_times | grep "$exe" | awk '{print $3=$3+1}'`
                new_line=`cat $test_times | grep "$exe" | awk '{printf("%-15s%d\t%d\t\n", $1, $2, $3+1)}'`
            fi
            sed -i "/$exe/s/.*/$new_line/" $test_times  

            if [ -f "./basic_script/${exe}_info.sh" ]
            then
                if [ $new_fail_times -ne 0 ]
                then
                    echo -en  "\033[0;32;1mPassed\033[0m:$new_succ_times | \033[0;31;1mFailed\033[0m:$new_fail_times"  | tee -a $console_save >&2
                    echo -ne "\r" >&2
                else
                    echo -en  "\033[0;32;1mPassed\033[0m:$new_succ_times | \033[0;32;1mFailed\033[0m:$new_fail_times"  | tee -a $console_save >&2
                    echo -ne "\r" >&2
                fi
                ./basic_script/${exe}_info.sh  | tee -a $console_save >&2
            else
                if [ $new_fail_times -ne 0 ]
                then
                    echo -e  "\033[0;32;1mPassed\033[0m:$new_succ_times | \033[0;31;1mFailed\033[0m:$new_fail_times"  | tee -a $console_save >&2
                    echo -ne "\r" >&2
                else
                    echo -e  "\033[0;32;1mPassed\033[0m:$new_succ_times | \033[0;32;1mFailed\033[0m:$new_fail_times"  | tee -a $console_save >&2
                    echo -ne "\r" >&2
                fi
            fi
        fi
    done < $config_file
    curr_time=`expr $curr_time + 1`
    if [ "$do_all_times" != "-1" ]
    then
        run_time=$curr_time
    fi
#    ./script/mod.sh -r 2>&1
done

#######################################################
# If test succeeds, light up leds
######################################################
echo "errors = $errors"
if [ $errors -ne 0 ]
then
    kill -9 $led_pid
    ./clib/gpio_led --passes -1 --mode 0 & 
else
    kill -9 $led_pid
    ./clib/gpio_led --passes 1 --mode 0 & 
    
fi

cat $console_save >&2



