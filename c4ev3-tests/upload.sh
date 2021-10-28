#!/bin/sh
echo "Project $1, Program $2"
ev3 up $2 ../prjs/$1/$2
ev3 mkrbf ../prjs/$1/$2 $2.rbf
ev3 up $2.rbf ../prjs/$1/$2.rbf
ev3 run ../prjs/$1/$2.rbf

#ev3 up prj_multithread/multithread ../prjs/aaah/aaa
#ev3 mkrbf ../prjs/aaah/aaa prj_multithread/multithread.rbf
#ev3 up prj_multithread/multithread.rbf ../prjs/aaah/aaa.rbf
#ev3 run ../prjs/aaah/aaa.rbf
