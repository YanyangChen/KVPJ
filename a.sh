#this shell file is written to trigger voice control
#source files stored in /home/chen/Voicecontrol/pocketsphinx-5prealpha/src/programs/continuous.c
#modify the c file and then command "sudo make" in the directory if needed
#dictionary stored in /usr/local/share/pocketsphinx/model/en-us/cmudict-en-us.dict
#to modify the dictionary, change the .dict content in dna and command: sudo cp cmudict-en-us.dict /usr/local/share/pocketsphinx/model/en-us 
#to modify the whole program, go to the pocketsphinks file directory and input the following command:
# ./configure
# sudo make
# sudo make install


#more detailed tutorial guide: http://cmusphinx.sourceforge.net/wiki/tutoriallm

pocketsphinx_continuous -inmic yes
