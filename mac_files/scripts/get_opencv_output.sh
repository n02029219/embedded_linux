#!/bin/bash
# get_opencv_output.sh
# script to scp an image file FROM the embedded linux project's folder
# if “-x” flag added, will access pi from external network

DIR_CPU=~/opencv_output
IMG_PI=embedded_linux/project/images/output.png
EXTERNAL_IP=24.168.34.221
INTERNAL_IP=192.168.2.100

mkdir -p ${DIR_CPU}
if [[ $1 = "-x" ]]; then
    echo "-x: Getting output from outside LAN"
    scp pi@${EXTERNAL_IP}:~/${IMG_PI} ${DIR_CPU}
else
    echo "Getting output from inside LAN"
    scp pi@${INTERNAL_IP}:~/${IMG_PI} ${DIR_CPU}
fi
echo “done”