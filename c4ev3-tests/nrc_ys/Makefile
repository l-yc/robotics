#TASK:=$(notdir $(shell pwd))
PROJECT=run2019
DOWNLOADSCRIPT=./download.sh
RUNSCRIPT=./run.sh
COMPILER=/home/lyc/EV3/buildroot/output/host/bin/arm-none-linux-gnueabi-g++
CFLAGS=-L/home/lyc/EV3/ev3duder/EV3-API -lev3api -I /home/lyc/EV3/ev3duder/EV3-API/src -std=c++11 -static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -Os -s 

test: test.cpp
	$(DOWNLOADSCRIPT) test.cpp $(PROJECT)
	$(RUNSCRIPT) test.cpp $(PROJECT)

d: run2019.cpp
	$(COMPILER) run2019.cpp pid.cpp $(CFLAGS) -o run2019
	ev3 exec "rm /home/root/lms2012/prjs/$(PROJECT)"

	ev3 up run2019 /home/root/lms2012/prjs/$(PROJECT)/run2019
	ev3 mkrbf /home/root/lms2012/prjs/$(PROJECT)/run2019 run2019.rbf
	ev3 up run2019.rbf /home/root/lms2012/prjs/$(PROJECT)/run2019.rbf
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
