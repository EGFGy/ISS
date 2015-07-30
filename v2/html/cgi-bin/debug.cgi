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

echo --------------
echo VARIABLES

env
echo end
