#!/bin/bash
clear
./netempmon \
    -c "./telegramsend.sh \"#title#\" \"#company#\" \"#location#\" \"#url#\" \"#logo#\"" \
    -c "./notifysend.sh \"#title#\" \"#company#\" \"#location#\" \"#url#\" \"#logo#\"" \
    -u "https://www.net-empregos.com/pesquisa-empregos.asp?chaves=programador&cidade=&categoria=0&zona=0&tipo=0" \
    -u "https://www.net-empregos.com/pesquisa-empregos.asp?chaves=desenvolvedor&cidade=&categoria=0&zona=0&tipo=0" \
    -u "https://www.net-empregos.com/pesquisa-empregos.asp?chaves=c%2B%2B&cidade=&categoria=0&zona=0&tipo=0" \
    -u "https://www.net-empregos.com/pesquisa-empregos.asp?chaves=.net&cidade=&categoria=0&zona=0&tipo=0"

