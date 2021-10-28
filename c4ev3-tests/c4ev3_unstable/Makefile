#TASK:=$(notdir $(shell pwd))
PROJECT=c4ev3_unstable
DOWNLOADSCRIPT=./download.sh
RUNSCRIPT=./run.sh

dr: main.cpp
	$(DOWNLOADSCRIPT) main.cpp $(PROJECT)
	$(RUNSCRIPT) main.cpp $(PROJECT)

d: main.cpp
	$(DOWNLOADSCRIPT) main.cpp $(PROJECT)

r::
	$(RUNSCRIPT) main.cpp $(PROJECT)

clean::
