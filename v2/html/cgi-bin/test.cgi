#!/bin/bash

cutycapt --url=http://jufo-log2.fritz.box/cgi-bin/Diagramm.cgi --out=/tmp/test.jpg
#echo "lol" > /tmp/lol

printf "Content-type: text/html\n\n";
printf "<p>Hello, world!</p>\n";
printf "<a href='/image.jpg'>image</a>"
