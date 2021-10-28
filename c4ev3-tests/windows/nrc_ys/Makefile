#TASK:=$(notdir $(shell pwd))
PROJECT=nrc_ys
DOWNLOADSCRIPT=./download.sh
RUNSCRIPT=./run.sh
COMPILER=/home/lyc/EV3/buildroot/output/host/bin/arm-none-linux-gnueabi-g++
CFLAGS=-L/home/lyc/EV3/ev3duder/EV3-API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/src -std=c++11 -static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -Os

test: test.cpp
	$(DOWNLOADSCRIPT) test.cpp $(PROJECT)
	$(RUNSCRIPT) test.cpp $(PROJECT)

d: main.cpp
	$(COMPILER) main.cpp pid.cpp $(CFLAGS) -o main
	ev3 exec "rm /home/root/lms2012/prjs/$(PROJECT)/main"

	ev3 up main /home/root/lms2012/prjs/$(PROJECT)/main
	ev3 mkrbf /home/root/lms2012/prjs/$(PROJECT)/main main.rbf
	ev3 up main.rbf /home/root/lms2012/prjs/$(PROJECT)/main.rbf
	#./beep.sh
	ffplay ./nyan/nyan.mp3 -nodisp

#echo "/home/root/lms2012/prjs/$projectFolder/$executableFile"
#ev3 exec "/home/root/lms2012/prjs/$projectFolder/$executableFile"
#dr: main.cpp
#	$(DOWNLOADSCRIPT) main.cpp $(PROJECT)
#	$(RUNSCRIPT) main.cpp $(PROJECT)
#
#d: main.cpp
#	$(DOWNLOADSCRIPT) main.cpp $(PROJECT)

r::
	$(RUNSCRIPT) main.cpp $(PROJECT)

clean::
