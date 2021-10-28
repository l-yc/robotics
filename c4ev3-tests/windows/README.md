# C4EV3 For Windows

Not really sure how everything will work but windows but I'll try to gather what I can here

Stuff you'll probably need to follow this:

- GNU Make (I think it can come with MinGW)
- Git

## Toolchain

[Toolchain Supporting C++ Download For WIndows](https://github.com/c4ev3/C4EV3.Toolchain/releases/download/2018-05-15/c4ev3-gcc-2018-05-15-windows.zip) - Apparently not working

According to the gitter chat this hacky workaround may work with the [Codesourcery Toolchain](http://www.codesourcery.com/sgpp/lite/arm/portal/package4571/public/arm-none-linux-gnueabi/arm-2009q1-203-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2):

- Upload the elf file.
- Grab the libstdc++.so.6.0.25 file from one of the toolchain releases.
- Upload the libstdc++.so.6.0.25 as libstdc++.so.6.
- Use "LD_PRELOAD=[libstdc++.so.6 location on ev3] [elf file location on ev3]" as remote path to create the rbf file.
- Upload the rbf file

If you are running a linux virtual machine:

- **Anything**: [This](https://github.com/c4ev3/C4EV3.Toolchain/releases/download/C4EV3.Toolchain-2019.08.0/C4EV3.Toolchain-v2019.08.0-arm-c4ev3-linux-uclibceabi-gcc-8.2.1-uclibc-ng-1.0.31-binutils-2.31.1-kernel-2.6.33-rc4-sanitized_linux-amd64.tar.xz) is the latest build released by the C4EV3 Developer. I didn't test it yet but you may want to try this first since it w

- **Ubuntu**: If I'm not wrong, this should be much easier because you can use the built in system cross compiler
- **Fedora**: (what I'm using) The system cross compiler doesn't support C++11, so what I did is to use [buildroot](http://parangninja.sytes.net:8080/installation/#a-note-about-c11) (the rest of the page may be useful as well but stuff there may be outdated)

## DUder

```bash
git clone https://github.com/c4ev3/ev3duder.git
cd ev3duder
make
sudo make install	# not sure how this will work for windows
```

## Library

```bash
# while in ev3duder directory
git clone --branch develop git@github.com:simonedegiacomi/EV3-API.git
```

Replace the `CROSS_COMPILE` macro in the Makefile with the compiler you just installed

```bash
make
```

## Compilation/Uploading

Refer to the Makefile in `nrc_ys` folder. You will need to change the following:

- `COMPILER`: Replace with the compiler you just installed
- `CFLAGS`: Replace the path after `-L` with the path to the `EV3-API` folder and the path after `-I` with the path to the `EV3-API/src` folder

To build and download a program, just type `make d` in the directory.

Other scripts probably won't work out of the box unless you are using linux and tweak them, but it's quite pointless since running from computer is quite slow anyways.

## Editing
Just edit the `main.cpp` file. If you need to test anything, you can just copy to the main function.
