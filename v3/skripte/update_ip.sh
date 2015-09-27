#!/bin/bash

SERVER_NAME="/tmp/server_name.txt"
printf "server_name $(dig +short myip.opendns.com @resolver1.opendns.com);" > $SERVER_NAME
