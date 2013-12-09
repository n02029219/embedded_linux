#!/bin/bash
# run_pi_opencv.sh
# script to run embedded linux OpenCV project on the pi, and collect the output file

echo "THIS SCRIPT DOES NOT WORK!"
echo "ONCE SSH HAS BEEN PERFORMED, SCRIPT STOPS"

EXTERNAL_PI_IP=24.168.34.221
INTERNAL_PI_IP=192.168.2.100
PROJECT_DIR=~/embedded_linux/project

# FLAGS:
# -c: copy project folder and re-install
# -x: ssh/scp from an external LAN, use external IP
# <parameter>: command-line argument to run opencv with

CFLAG=0
XFLAG=0
ARG=""

# process arguments
while [[ "$1" != "" ]]; do
	case $1 in 
		-x )	echo "-x raised: external network"
				XFLAG=1
				;;
		-c ) 	echo "-c raised: copy project first"
				CFLAG=1
				;;
		* )		echo "argument: " $1
				ARG=$1
				;;
	esac
	shift
done

# copy project if -c flag raised, then run
if [[ $CFLAG = 1 ]]; then
	cd ~/scripts
	if ["$XFLAG" -e 1]; then
		./copy_project.sh -x
	else
		./copy_project.sh
	fi
	echo "-c: Project directory copied"
fi

# ssh into pi
if [[ $XFLAG = 1 ]]; then
	ssh pi@${EXTERNAL_PI_IP}
	echo "-x: pi accessed from external network"
else
	ssh pi@${INTERNAL_PI_IP}
	echo "pi accessed from local network"
fi

# if -c raised, first install project
if [$CFLAG = 1]; then
	cd ${PROJECT_DIR}
	make install
	echo "-c: project installed"
fi

# run program
cd ${PROJECT_DIR}
./opencv $ARG
echo "project run"

# get output
logout
cd ~/scripts
if [[ $XFLAG = 1 ]]; then
	./get_opencv_output.sh -x
else
	./get_opencv_output.sh
fi
echo "output file retrieved"
echo "done."
