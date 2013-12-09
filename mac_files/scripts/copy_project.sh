#!/bin/bash
# copy_project.sh
# script to copy opencv project folder onto the pi

if [[ $1 = "-x" ]]; then
	echo "-x: Copying project directory to Pi from outside LAN"
	scp -r ~/Documents/SUNY/Fall_2013/Embedded_Linux/project pi@24.168.34.221:~/embedded_linux
else
	echo "Copying project directory to Pi from inside LAN"
	scp -r ~/Documents/SUNY/Fall_2013/Embedded_Linux/project pi@192.168.2.100:~/embedded_linux
fi

echo "done"