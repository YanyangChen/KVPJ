#!/bin/bash 
read SWITCH
read PID
#NUM=awk '/./{line=$0} END{print line}' char-array.txt

if [ "$SWITCH" == "activate" ]; 
then
kill -18 "$PID"
./Desktop/shared_memory/clienta
#./c.sh
echo "activateddd" >> char-array.txt


elif [ "$SWITCH" == "stop" ]; 
then
kill -19 "$PID"
./Desktop/shared_memory/clients
echo "stopped" >> char-array.txt

else
echo "waiting"
fi
