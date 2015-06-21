#!/bin/bash

echo "Content-type: text/plain"
echo ""
echo "SELECT name FROM Benutzer" | mysql -pweb_pass -u web_user base4

