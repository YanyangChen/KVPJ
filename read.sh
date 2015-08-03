#(while sleep 1;do awk '/./{line=$0} END{print line}' char-array.txt;done)

awk '/./{line=$0} END{print line}' char-array.txt
