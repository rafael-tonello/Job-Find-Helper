#!/bin/bash
title=$1
company=$2
place=$3
url=$4
logo=$5

sendToTelegram.sh "Job oportunity: $title\nCompany: $company\nPlace: $place\nMore info: $url"

