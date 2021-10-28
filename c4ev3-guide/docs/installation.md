# Installing c4ev3

## Prerequisites
- Linux/Unix-like System
    - MacOS Users: To follow some of the commands, you might need to install brew.
    - Windows Users: Install windows subsystem for linux.
- No phobia of terminals

## Getting the dependencies

First, we need to install the GCC Arm Toolchain in order to compile code for the EV3. We have 2 options:

- Use system repository (Not successfully tested)
- Install the CodeSourcery Toolchain

For simplicity, since the CodeSourcery Toolchain also provides the build environment, we'll download it. Open up a terminal and paste in the following:
```bash
wget -c http://www.codesourcery.com/sgpp/lite/arm/portal/package4571/public/arm-none-linux-gnueabi/arm-2009q1-203-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2
mkdir ~/CodeSourcery
tar -jxvf ~/arm-2009q1-203-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2 -C ~/CodeSourcery/
echo export PATH=~/CodeSourcery/arm-2009q1/bin/:$PATH >> ~/.bashrc && . ~/.bashrc
```

Next, we'll need to get the program uploader and EV3-API.
```bash
git clone https://github.com/c4ev3/ev3duder.git
cd ev3duder
```

We'll build the program downloader/uploader:
```bash
make
sudo make install
```

Now, we'll build the API.
```bash
cd EV3-API/API
make
```

Open up the Makefile using your favourite text editor and paste in the following
(Note: on some versions of make, spaces are not accepted in replacement of tabs. Replace the indentation with tabs for the Makefile to work)
```Makefile
# Make sure it's added to your path!
override PREFIX = arm-none-linux-gnueabi- 
CC = $(PREFIX)gcc
AR = $(PREFIX)ar
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

.DEFAULT: libev3api.a
libev3api.a: $(OBJS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) -Os -isystem. -c $<

#ifeq ($(OS),Windows_NT)
#RM = del /Q
#endif

.PHONY: clean
clean:
	$(RM) *.o *.a *.d
```

Now, let's build the project.
```bash
make
```

You should now see `libev3api.a` in your current directory. Proceed to the next section to see how to compile and run programs.
