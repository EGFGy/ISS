#!/bin/bash

# Diese bash-Skript dient nur zum Herunterladen des aktuellen Vertretungsplans
# und ist nur im Falle unserer Schule gültig, es hat sonst keinen Nutzen.

DOW=$(($(date +%-w)-1))
days=(mo di mi do fr)
base_url="http://vertretungsplan.egf-online.de/Schuelerplanjoomla/schuelerplan_"

echo "Tag: "${days[$DOW]}
if (( $DOW < 5)); then
	URL=$base_url${days[$DOW]}".htm"
else
	URL=$base_url${days[0]}".htm"
fi
echo "URL: "$URL

#wget -O ${days[$DOW]}.htm $URL
wget -O plan.html $URL
