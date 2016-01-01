#!/bin/bash

if [ -z "$1" ]; then
	echo "ALL options must be specified"
        exit
fi

if [ $1 == "-h" ]; then
	echo
	echo "Usage: makejunk.sh [URL] [n] [o] [y/n]"
	echo "URL: URL to call"
	echo "n:   Number of calls to wget"
	echo "o:   output File for speeds [MB/s]"
	echo "y/n: start gnuplot after wget-loop finishes?"
	echo
	echo "ALL options must be specified"
	exit
fi

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ]; then
        echo "ALL options must be specified"
        exit
fi

tempo=tempo.file
rm $tempo
echo "Request progress:"

for ((i=0;$i <= $(($2));i++))do
	wget --no-check-certificate $1 -O /dev/null 2>> $tempo
	printf "#"
done

echo " done"

grep "MB\/s" $tempo | cut -d " " -f 3 | sed -e 's/(//' > $3

rm $tempo

if [ $4 == "n"  ]; then
	echo "not starting gnuplot"
else
	if [ $4 == "y" ]; then
		gnuplot -e "set term x11 persist;set grid ;p '$3' w l"
	else
		echo "Newton you have wrong, light is wave!"
	fi
fi
