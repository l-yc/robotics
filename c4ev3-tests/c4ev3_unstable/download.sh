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
/home/lyc/EV3/buildroot/output/host/bin/arm-none-linux-gnueabi-g++ $sourceFile -L/home/lyc/EV3/ev3duder/EV3-API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/src -std=c++11 -static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -Os -o $executableFile
#../arm-2009q1/bin/arm-none-linux-gnueabi-g++ $sourceFile -L/home/lyc/EV3/ev3duder/EV3-API/API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/API -std=c++0x -static -Os -o $executableFile

ev3 exec "rm -r /home/root/lms2012/prjs/$projectFolder/"

ev3 up $executableFile /home/root/lms2012/prjs/$projectFolder/$executableFile
ev3 mkrbf /home/root/lms2012/prjs/$projectFolder/$executableFile $executableFile.rbf
ev3 up $executableFile.rbf /home/root/lms2012/prjs/$projectFolder/$executableFile.rbf
#echo "/home/root/lms2012/prjs/$projectFolder/$executableFile"
#ev3 exec "/home/root/lms2012/prjs/$projectFolder/$executableFile"
