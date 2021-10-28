# Compiling & Uploading
## Your first project
Using your favourite text editor, paste the following into helloworld.c:
```c
#include <ev3.h>

int main() {
    InitEV3();
    LcdPrintf(1, "Hello World!");   // prints black text on white background
    Wait(10000);
    FreeEV3();
}
```

Save the file and open up a terminal. Locate the path of the library that you have created earlier. Now type in the following to build the program:
```bash
# To link the library statically (*.a)
# -L: path to library
# -l: name of library

# To include the headers
# -I: path to header files

# Compilation flags
# -std=c99: enable C99 features
# -Os: optimise for size
arm-none-linux-gnueabi-gcc yourprogram.c -L/path/to/lib -lev3api -I/path/to/lib -std=c99 -Os -o yourprogram
```

Finally, to upload the compiled program to your ev3, enter the following into a terminal. You may save it as a script to ease the process in the future.
```bash
#!/bin/sh
ev3 up ./helloworld ../prjs/c4ev3_helloworld/helloworld
ev3 mkrbf ../prjs/c4ev3_helloworld/helloworld helloworld.rbf
ev3 up ./helloworld.rbf ../prjs/c4ev3_helloworld/helloworld.rbf
ev3 run ../prjs/c4ev3_helloworld/helloworld.rbf
```

Congrats if you made it this far :)
