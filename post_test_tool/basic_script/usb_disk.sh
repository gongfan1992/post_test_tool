#!/bin/sh
dev=sda
find_dev=0
for block in `ls /sys/class/block`
do
	if [ "$dev" == "$block" ]
	then
		find_dev=1
		break
	fi
done

if [ $find_dev -eq 0 ]
then
	echo "[USB-DISK-`date +'%Y-%m-%d %T'`]: Can't find the usb disk $dev."
	exit 1
fi

mkdir /mnt/sda1
mount /dev/sda1 /mnt/sda1
if [ $? -ne 0 ]
then
    echo "[USB-DISK-`date +'%Y-%m-%d %T'`]: Mount /dev/sda1 failed"
    exit 1
fi
dd if=/dev/urandom of=/opt/test_src bs=1024k count=100
if [ $? -ne 0 ]
then
    echo "[USB-DISK-`date +'%Y-%m-%d %T'`]: Write the usb disk failed."
    umount /mnt/sda1
    rm -rf /mnt/sda1
    sync
    exit 1
fi
dd if=/opt/test_src of=/mnt/sda1/post.data bs=1024k count=100
if [ $? -ne 0 ]
then
    echo "[USB-DISK-`date +'%Y-%m-%d %T'`]: Write the usb disk failed."
    rm -rf /opt/test_src
    umount /mnt/sda1
    rm -rf /mnt/sda1
    sync
    exit 1
fi
sync
dd if=/mnt/sda1/post.data of=/opt/test_dst bs=1024k count=100
if [ $? -ne 0 ]
then
    echo "[USB-DISK-`date +'%Y-%m-%d %T'`]: Read the usb disk failed."
    rm -rf /mnt/sda1/post.data
    rm -rf /opt/test_src
    umount /mnt/sda1
    rm -rf /mnt/sda1
    sync
    exit 1
fi

diff /opt/test_src /opt/test_dst
if [ $? -ne 0  ]   
then                 
    echo "[DISK-`date +'%Y-%m-%d %T'`]: Check the data failed."
    rm -rf /mnt/sda1/post.data
    rm -rf /opt/test_src
    rm -rf /opt/test_dst
    umount /mnt/sda1
    rm -rf /mnt/sda1
    sync
    exit 1           
else                 
    echo "[DISK-`date +'%Y-%m-%d %T'`]: Check the data success."
fi  
rm -rf /mnt/sda1/post.data
rm -rf /opt/test_src
rm -rf /opt/test_dst
umount /mnt/sda1
rm -rf /mnt/sda1
sync
exit 0
