#!/bin/sh
echo "THIS MUST BE RAN IN THE prj_***/ FOLDER."

sourceFile=$1
executableFile="${sourceFile%.*}"
projectFolder=$2

if [[ $sourceFile == "" || $projectFolder == "" ]]
then
    echo "NO SOURCE FILE OR PROJECT FOLDER SPECIFIED"
    exit 0;
fi

echo "Source $sourceFile, Project Folder $projectFolder"
/home/lyc/EV3/buildroot/output/host/bin/arm-none-linux-gnueabi-g++ $sourceFile -L/home/lyc/EV3/ev3duder/EV3-API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/src -std=c++11 -static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -Os -s -o $executableFile
# uncomment the line below to use the old API
#/home/lyc/EV3/buildroot/output/host/bin/arm-none-linux-gnueabi-g++ $sourceFile -L/home/lyc/EV3/ev3duder/EV3-API/API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/API -std=c++11 -static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -Os -o $executableFile

ev3 exec "rm /media/card/$projectFolder/$executableFile"

ev3 up $executableFile /media/card/$projectFolder/$executableFile
ev3 mkrbf /media/card/$projectFolder/$executableFile $executableFile.rbf
ev3 up $executableFile.rbf /media/card/$projectFolder/$executableFile.rbf
#echo "/home/root/lms2012/prjs/$projectFolder/$executableFile"
#ev3 exec "/home/root/lms2012/prjs/$projectFolder/$executableFile"
