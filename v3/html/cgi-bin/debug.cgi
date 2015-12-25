#!/bin/bash

echo "Content-type: text/html"
echo
echo
cat <<HEADER
<!DOCTYPE html>
<head>
	<title>InfoWall -- Debug</title>
	<meta http-equiv="content-type" content="text/html;charset=utf-8" />
	<meta name="viewport" content="width=device-width" />
	<link rel="stylesheet" href="/css/forms.css" />
	<link rel="shortcut icon" href="/favicon.png" />
</head>

<body>
<pre>
HEADER


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
#service --status-all 2>/dev/null
service nginx status
service ufw status
service mysql status
service fcgiwrap status
echo -------------
echo "Temperatur:"
sensors | grep -i '[Ct][oe][rm][ep]' | sed -e 's/(.*//'
echo ------------
echo
#echo
#ping -c 2 fritz.box
#echo
#echo
#ping -c 2 google.com
#echo
#ping -c 2 heise.de
echo ------------
echo
echo ">-------------<"
echo VARIABLES
env
echo ">-------------<"
echo "end"
echo ">-------------<"
echo "vnstat"
vnstat
echo "</pre>"
echo "</body></html>"

echo "DEBUG executed" 1>&2
