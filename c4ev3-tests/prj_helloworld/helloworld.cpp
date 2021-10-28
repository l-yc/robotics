#include <ev3.h>

void sensortest() {
    setAllSensorMode(COL_REFLECT, COL_REFLECT, COL_REFLECT, COL_REFLECT);
    while (!ButtonIsDown(BTNEXIT)) {
        LcdClean();
        LcdPrintf(1, "%d", readSensor(IN_1));
        Wait(1000);
    }
}

void motortest() {
    OnFwdReg(OUT_A, 50);
    Wait(1000);
    OnRevReg(OUT_A, 50);
    Wait(1000);
    Off(OUT_A);
}

int main() {
    InitEV3();

    setAllSensorMode(COL_REFLECT, COL_REFLECT, COL_REFLECT, COL_REFLECT);
    while (!ButtonIsDown(BTNEXIT)) {
        int val = readSensor(IN_1);
        if (val < 50) {
            OnFwdReg(OUT_B, -30);
            OnFwdReg(OUT_C, -15);
        }
        else {
            OnFwdReg(OUT_B, -15);
            OnFwdReg(OUT_C, -30);
        }
    }
    //Wait(10000);
    //PlayToneEx(440, 1000, 100);
    //LcdPrintf(0, "Hello World!");
    FreeEV3();
}
