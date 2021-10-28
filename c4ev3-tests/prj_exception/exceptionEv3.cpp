#include <ev3.h>
#include <exception>

int main() {
    InitEV3();

    PlayTone(440, 1000);
    Wait(1000);
    PlayTone(220, 1000);
    try {
        int *blah = new int[1238908123];
    }
    catch (const std::exception & e) {
        //std::cerr << e.what() << std::endl;
        LcdPrintf(0, e.what());
    }
    Wait(5000);

    FreeEV3();
}
