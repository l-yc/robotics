#include <ev3.h>

void motortest() {
    OnFwdReg(OUT_A, 50);
    Wait(1000);
    OnRevReg(OUT_A, 50);
    Wait(1000);
    Off(OUT_A);
}

int main() {
    InitEV3();

    //motortest();
    //ResetRotationCount(OUT_A);
    OnFwdReg(OUT_A, 25);
    while (!ButtonIsDown(BTNEXIT)) {
        LcdTextf(1, 0, 50, "A: %d", MotorRotationCount(OUT_A));
        //LcdTextf(1, 0, 50, "A: %d", OutputInstance.pMotor[OUT_A].TachoSensor);

        Wait(100);
    }
    Off(OUT_A);

    FreeEV3();
}
