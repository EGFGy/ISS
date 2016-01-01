#!/bin/bash

CPU=$(lscpu | grep -i '^CPU(s):' | sed -e 's/CPU(s):[ ]*//')
w_CPU=$CPU"00px"
Min_1=$(uptime | sed -e 's/^.*load average: //' -e 's/, / /g' -e 's/[.,]//g' | cut -d' ' -f 1)
Min_5=$(uptime | sed -e 's/^.*load average: //' -e 's/, / /g' -e 's/[.,]//g' | cut -d' ' -f 2)
Min_15=$(uptime | sed -e 's/^.*load average: //' -e 's/, / /g' -e 's/[.,]//g' | cut -d' ' -f 3)
UP=$(uptime)

echo "content-type: text/html"
echo ""
cat <<EOH
<!DOCTYPE html>
<html lang="de"><head>
<meta name="viewport" content="width=device-width, initial-scale=1,  maximum-scale=1, user-scalable=no">
	<title>InfoWall -- Load</title>
	<meta http-equiv="content-type" content="text/html;charset=utf-8" />
	<link rel="shortcut icon" href="/favicon.png" />
	<style>
		.bar{
			width: $w_CPU;
			background-color: #000;
			border: 1px solid black;
			display: block;
		}
		.level{
			background-color: #5F5;
			display: block;
		}
		b{
			color: #00F;
		}
	</style>
</head>

EOH
echo "<body>"
#echo $Min_1
#echo $Min_5
#echo $Min_15
w_1=$Min_1"px"
w_5=$Min_5"px"
w_15=$Min_15"px"
echo "<h2>CPU_Auslastung</h2>"
echo "<span class='bar'><span class='level' style='width: $w_1;'><b>1Min</b></span></span><br>"
echo "<span class='bar'><span class='level' style='width: $w_5;'><b>5Min</b></span></span><br>"
echo "<span class='bar'><span class='level' style='width: $w_15;'><b>15Min</b></span></span><br>"
echo "<em>$UP</em>"
echo "</body>"
