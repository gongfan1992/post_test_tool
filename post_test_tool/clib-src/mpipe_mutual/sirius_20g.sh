mpipe-stat -v xgbe1 
mpipe-stat -v xgbe2
taskset -c 1-3 ./mpipe_mutual --sender xgbe1 --recver xgbe1 --second 21600 > /mnt/xgbe1.log &
taskset -c 4-6 ./mpipe_mutual --sender xgbe2 --recver xgbe2 --second 21600 > /mnt/xgbe2.log &
tail -f /mnt/xgbe1.log
