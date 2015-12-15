#!/bin/sh

dd if=/dev/urandom of=/opt/test_src_spi bs=1024k count=10
if [ $? -ne 0 ]
then
    echo "[GPIO_CPLD_SPI-`date +'%Y-%m-%d %T'`]: Write the gpio_cpld_spi failed."
    sync
    exit 1
fi
dd if=/opt/test_src_spi of=/home/post.data bs=1024k count=10
if [ $? -ne 0 ]
then
    echo "[GPIO_CPLD_SPI-`date +'%Y-%m-%d %T'`]: Write the gpio_cpld_spi failed."
    rm -rf /opt/test_src_spi
    rm -rf /home/post.data
    sync
    exit 1
fi
sync
dd if=/home/post.data of=/opt/test_dst_spi bs=1024k count=10
if [ $? -ne 0 ]
then
    echo "[GPIO_CPLD_SPI-`date +'%Y-%m-%d %T'`]: Read the gpio_cpld_spi failed."
    rm -rf /home/post.data
    rm -rf /opt/test_src_soi
    sync
    exit 1
fi

diff /opt/test_src_spi /opt/test_dst_spi
if [ $? -ne 0  ]   
then                 
    echo "[GPIO_CPLD_SPI-`date +'%Y-%m-%d %T'`]: Check the gpio_cpld_spi failed."
    rm -rf /home/post.data
    rm -rf /opt/test_src_spi
    rm -rf /opt/test_dst_spi
    sync
    exit 1           
else                 
    echo "[GPIO_CPLD_SPI-`date +'%Y-%m-%d %T'`]: Check the gpio_cpld_spi success."
fi  
rm -rf /home/post.data
rm -rf /opt/test_src_spi
rm -rf /opt/test_dst_spi
sync
exit 0
