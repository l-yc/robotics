#include <ev3.h>
#include <thread>
//using namespace std;

int main() {
    InitEV3();

    //try {
        PlayTone(440, 1000);
        Wait(1000);
        std::thread a([]() {
            // Do Something
            Wait(1000);
            LcdPrintf(0, "Hello World!");
        });
        std::thread b([]() {
            // Do Something
            LcdPrintf(0, "Bye World!");
            Wait(1000);
        });

        a.join();
        b.join();
        LcdPrintf(0, "threads done");
    //} catch (std::exception e) {
    //    LcdClean();
    //    LcdPrintf(0, e.what());
    //}
    Wait(10000);

    FreeEV3();
}
