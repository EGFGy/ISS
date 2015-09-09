#!/bin/bash

SERVER_NAME="/tmp/server_name.txt"
printf "server_name " > $SERVER_NAME
printf "%s;\n" $(curl -s checkip.dyndns.org | sed -e 's/.*Current IP Address: //' -e 's/<.*$//' ) >> $SERVER_NAME
