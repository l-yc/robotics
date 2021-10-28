#include <ev3.h>
#include <thread>

namespace operation {
    void monitorExit() {
        std::thread a([]() {
            while (!ButtonIsDown(BTNEXIT)) {
                Wait(100);
            }
            exit(0);
        });

        a.detach();
    }
}


int main() {
    operation::monitorExit();

    while (true) {
        RGB val1 = ReadEV3ColorSensorRGB(IN_1);
        RGB val2 = ReadEV3ColorSensorRGB(IN_2);
        RGB val3 = ReadEV3ColorSensorRGB(IN_3);
        RGB val4 = ReadEV3ColorSensorRGB(IN_4);
        LcdTextf(1, 0, 20, "IN 1 R %03d G %03d B %03d", val1.red, val1.green, val1.blue, val1.red+val1.green+val1.blue);
        LcdTextf(1, 0, 40, "IN 2 R %03d G %03d B %03d", val2.red, val2.green, val2.blue, val2.red+val2.green+val2.blue);
        LcdTextf(1, 0, 60, "IN 3 R %03d G %03d B %03d", val3.red, val3.green, val3.blue, val3.red+val3.green+val3.blue);
        LcdTextf(1, 0, 80, "IN 4 R %03d G %03d B %03d", val4.red, val4.green, val4.blue, val4.red+val4.green+val4.blue);
        Wait(10);
    }
}
