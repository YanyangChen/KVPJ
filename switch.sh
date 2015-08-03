
read ON
#COUNT=1;
if [ "$ON" == "activate" ]; 
then
#kill -18 ./Sample-NiSimpleSkeleto

	#if [$COUNT = 1];
	#then
	#./c.sh
cd /home/chen/kinect/OpenNI/Samples/Bin/x64-Release
./Sample-NiSimpleSkeleto 
echo $! >> numbers.txt
echo "activateddd" >> char-array.txt
	#COUNT=0;
	#fi


#elif [ "$ON" == "stop" ]; 
#then
#kill -19 ./Sample-NiSimpleSkeleto


else
echo "doing nothing"
fi
