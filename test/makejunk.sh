#!/bin/bash

if [ $1 == "-h" ]; then
	echo
	echo "Usage: makejunk.sh [n] [o] [y/n]"
	echo "n:   Number of calls to wget"
	echo "o:   output File for speeds [MB/s]"
	echo "y/n: start gnuplot after wget-loop finishes?"
	echo
	echo "ALL options must be specified"
	exit
fi

for ((i=0;$i<$1;i++))do
	wget --no-check-certificate keller-pc -O /dev/null 2>>/tmp/crappy
done
grep "MB\/s" /tmp/crappy | cut -d " " -f 3 | sed -e 's/(//' > $2

if [ $3 == "n"  ]; then
	echo "not starting gnuplot"
else
	if [ $3 == "y" ]; then
		gnuplot -e "set term x11 persist;set grid ;p '$2' w l"
	else
		echo "You have wrong Newton, light is wave!"
	fi
fi
