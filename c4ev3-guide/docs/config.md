# Config
# Get the toolchain (the less cancer way, I gave up using system package)
```bash
$ wget -c http://www.codesourcery.com/sgpp/lite/arm/portal/package4571/public/arm-none-linux-gnueabi/arm-2009q1-203-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2
$ mkdir ~/CodeSourcery
$ tar -jxvf ~/arm-2009q1-203-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2 -C ~/CodeSourcery/
$ echo export PATH=~/CodeSourcery/arm-2009q1/bin/:$PATH >> ~/.bashrc && . ~/.bashrc
```

# Build the EV3-API next (please don't blindly copy stuff)
```
cd ev3duder/EV3-API/API
make
```
Note the libev3api.a file created. Note down its path, then when compiling (e.g. test.c)

arm-linux-blah test.c -L/path/to/lib -lev3api -I/path/to/lib -o test

(L for linker, l for library, I for includes)


# Uploading the executable
Refer to upload.sh
