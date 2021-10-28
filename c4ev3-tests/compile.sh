#!/bin/sh
#../arm-2009q1/bin/arm-none-linux-gnueabi-gcc helloworld.c -L/home/lyc/Downloads/tmp/ev3/c4ev3/ev3duder/EV3-API/API -lev3api -I /home/lyc/Downloads/tmp/ev3/c4ev3/ev3duder/EV3-API/API -Os -o helloworld -std=c99
echo "Source $1, Output $2"
#../arm-2009q1/bin/arm-none-linux-gnueabi-g++ $1 -L/home/lyc/EV3/ev3duder/EV3-API/API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/API -std=c++11 -static -Os -o $2
#../arm-2009q1/bin/arm-none-linux-gnueabi-g++ $1 -L/home/lyc/EV3/ev3duder/EV3-API/API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/API -std=c++0x -static -Os -o $2 -lpthread
../arm-2009q1/bin/arm-none-linux-gnueabi-g++ $1 -L/home/lyc/EV3/ev3duder/EV3-API/API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/API -std=c++0x -static -Os -o $2

#arm-linux-gnu-cpp $1 -I/usr/arm-linux-gnu/include -L/home/lyc/EV3/ev3duder/EV3-API/API -lev3api -I/home/lyc/EV3/ev3duder/EV3-API/API -static -Os -o $2 -std=c++0x
#arm-linux-gnu-cpp $1 -I/usr/arm-linux-gnu/include -L/home/lyc/EV3/ev3duder/EV3-API/API -lev3api -I/home/lyc/EV3/ev3duder/EV3-API/API -static -Os -o $2 -std=c++11 -static-libstdc++

#arm-linux-gnu-cpp $1 -L/home/lyc/Downloads/tmp/ev3/c4ev3/ev3duder/EV3-API/API -lev3api -I /home/lyc/Downloads/tmp/ev3/c4ev3/ev3duder/EV3-API/API -static -Os -o $2 -I /usr/arm-linux-gnu/include -I /usr/arm-linux-gnu/sys-root/usr/include
#arm-linux-gnu-cpp $1 -L/home/lyc/Downloads/tmp/ev3/c4ev3/ev3duder/EV3-API/API -lev3api -I /home/lyc/Downloads/tmp/ev3/c4ev3/ev3duder/EV3-API/API -static -lpthread -Os -o $2 -I /usr/arm-linux-gnu/include -I /usr/arm-linux-gnu/sys-root/usr/include -std=c++11
