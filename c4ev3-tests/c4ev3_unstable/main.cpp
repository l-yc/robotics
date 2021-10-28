#include <ev3.h>
#include <thread>

namespace operation {
    void monitorExit() {
        std::thread a([]() {
            while (!ButtonIsDown(BTNEXIT)) {
                Wait(100);
            }
            LcdExit();

            exit(0);
        });

        a.detach();
    }
}

//namespace test {
//    void ptrace() {
//        SetSensorMode(IN_2, COL_REFLECT);
//        OutputPolarity(OUT_B, -1);
//        while (!ButtonIsDown(BTNEXIT)) {
//            int val = readSensor(IN_2);
//
//            LcdClean();
//            LcdTextf(1,0,0,"VALUE: %03d", val);
//
//            if (val > 50) {
//                OnFwdReg(OUT_B, -20);
//                OnFwdReg(OUT_C, 10);
//            } else {
//                OnFwdReg(OUT_B, -10);
//                OnFwdReg(OUT_C, 20);
//            }
//            Wait(10);
//        }
//
//        Off(OUT_B);
//        Off(OUT_C);
//    }
//}

int main() {
    InitEV3();

    operation::monitorExit();

    PlayToneEx(440,1000,100);
    Wait(500);

    //SetSensor(IN_2, EV3Color);
    SetSensor(IN_1, HTColorV2);

    LcdTextf(1,0,50,"Hello World!\n");
    LcdUpdate();
    Wait(1000);
    //while (1) {
    //    int val = ReadEV3ColorSensorLight(IN_2, ReflectedLight);
    //    LcdClean();
    //    LcdTextf(1,0,0,"VAL: %03d\n", val);
    //    LcdUpdate();
    //    Wait(100);
    //}
    
    //while (1) {
    //    RGB val = ReadEV3ColorSensorRGB(IN_2);
    //    LcdClean();
    //    LcdTextf(1,0,0,"VAL: %04d %04d %04d\n", val.red, val.green, val.blue);
    //    LcdUpdate();
    //    Wait(100);
    //}

    while (1) {
        RGBA val = ReadHTColorSensorV2RGBA(IN_1, HTColorSensorDefaultMode);
        LcdClean();
        LcdTextf(1,0,50,"VAL: %04d %04d %04d %04d\n", val.red, val.green, val.blue, val.white);
        LcdUpdate();
        Wait(100);
    }

    //SetSensorMode(IN_2, NXT_IR_SEEKER); // just to test iic
    //while (1) {
    //    //uint64_t* val = ReadSensorData(IN_2);
    //    uint8_t* val = (uint8_t*) ReadSensorData(IN_2);

    //    LcdClean();
    //    for (int i = 0; i < 4; ++i) LcdTextf(1,i*40,0,"%03d", val[i]);
    //    for (int i = 4; i < 8; ++i) LcdTextf(1,(i-4)*40,20,"%03d", val[i]);
    //    Wait(100);
    //}

    FreeEV3();
}

