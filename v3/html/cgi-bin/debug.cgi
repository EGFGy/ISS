#!/bin/bash

echo "Content-type: text/plain"
echo
echo

echo QueryString:
echo $QUERY_STRING

echo --------------
echo POST:
read post_dataz
echo $post_dataz
echo -------------
echo
echo "status:"
echo -------------
uptime
echo
service --status-all 2>/dev/null
echo -------------
echo "Temperatur:"
sensors | grep -i '[Ct][oe][rm][ep]' | sed -e 's/(.*//'
echo ------------
echo
echo
ping -c 2 fritz.box
echo
echo
ping -c 2 google.com
echo
ping -c 2 heise.de
echo ------------
echo
echo ">-------------<"
echo VARIABLES
env
echo ">-------------<"
echo end
