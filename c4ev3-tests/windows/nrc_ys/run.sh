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

echo "Running compiled $sourceFile, Project Folder $projectFolder"
echo "/home/root/lms2012/prjs/$projectFolder/$executableFile"
ev3 exec "/home/root/lms2012/prjs/$projectFolder/$executableFile"
