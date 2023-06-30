#!/bin/bash
title=$1
company=$2
place=$3
url=$4
logo=$5

randomid=$((1 + $RANDOM % 10))


#download logo
curl -sS $logo --output /tmp/$randomid
notify-send --app-name="NetEmrpegos monitor" --icon="/tmp/$randomid" "$title" "More info: $url\nCompany: $company\nPlace: $place"

if [[ $company == *"yld"* ]]; then
    play /media/veracrypt/Sounds/Effects/alert1.ogg
fi
