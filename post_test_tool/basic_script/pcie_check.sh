#! /bin/sh
#
board=`lspci | grep "HighPoint"`
if [ "$board" != "" ]
then
	echo "pcie HighPoint link up"
	exit 0
else
	echo "pcie HighPoint link failed" 
	exit 1
fi


 
