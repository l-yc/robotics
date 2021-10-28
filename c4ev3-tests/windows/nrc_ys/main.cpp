#include <ev3.h>
#include <thread>
#include <cmath>
#include <cassert>
#include "pid.h"

const int sen2to4 = 68;
const int arc90 = 440; //446; //436;
const int unit = 28;
void OnForDuration(int output, int power, int millis) {
    OnFwdReg(output, power);
    Wait(millis);
    Off(output);
}

void OnForDurationUnregulated(int output, int power, int millis) {
    OnFwdEx(output, power, RESET_NONE);
    Wait(millis);
    Off(output);
}

void OnFwdUnreg(int motor, double power) {
    if (power > 100) power = 100;
    if (power < -100) power = -100;
    OnFwdEx(motor, power, RESET_NONE);
}

namespace problem {
    enum class orientation {
        EAST, SOUTH, WEST, NORTH
    };

    enum class color {
        RED, BLUE, GREEN, YELLOW
    };

}

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

    void linetrace(int sensor, int threshold, int high, int low, bool verbose = false) {
        int val = ReadEV3ColorSensorLight(sensor, ReflectedLight);
        if (val > threshold) {
            OnFwdReg(OUT_B, high);
            OnFwdReg(OUT_C, low);
        } else {
            OnFwdReg(OUT_B, low);
            OnFwdReg(OUT_C, high);
        }

        if (verbose) {
            LcdClean(); LcdTextf(0, 0, 0, "VAL: %03d", val);
        }
    }

    void ptrace(int sensor, int threshold, int avg, double aggression) {
        int val = ReadEV3ColorSensorLight(sensor, ReflectedLight);
        int err = val-threshold;

        OnFwdReg(OUT_B, avg + aggression * err);
        OnFwdReg(OUT_C, avg - aggression * err);
    }

    void pStageTrace(int sensor, int threshold, int avg, double aggressionHi, double aggressionLo, int dirn) {
        int val = ReadEV3ColorSensorLight(sensor, ReflectedLight);
        int err = abs(val-threshold);

        if (dirn == -1) {
            if (val < threshold) {
                OnFwdReg(OUT_C, avg - aggressionLo * err);
                OnFwdReg(OUT_B, avg + aggressionLo * err);
            } else {
                OnFwdReg(OUT_C, avg + aggressionHi * err);
                OnFwdReg(OUT_B, avg - aggressionHi * err);
            }
        } else {
            if (val < threshold) {
                OnFwdReg(OUT_B, avg - aggressionLo * err);
                OnFwdReg(OUT_C, avg + aggressionLo * err);
            } else {
                OnFwdReg(OUT_B, avg + aggressionHi * err);
                OnFwdReg(OUT_C, avg - aggressionHi * err);
            }
        }
    }

    int getColor(RGB rgb) {
        return static_cast<int>(problem::color::RED);
    }

    void rotateSync(int powerB, int powerC, int degreeB, int degreeC) {
        std::thread a([=]() {
            RotateMotor(OUT_B, powerB, degreeB);
        });
        std::thread b([=]() {
            RotateMotor(OUT_C, powerC, degreeC);
        });

        a.join();
        b.join();
    }

    void rotateSync(int powerB, int powerC, int time) {
        std::thread a([=]() {
            OnForDuration(OUT_B, powerB, time);
        });
        std::thread b([=]() {
            OnForDuration(OUT_C, powerC, time);
        });

        a.join();
        b.join();
    }

    void turnClaw() {
        OnForDurationUnregulated(OUT_A, 25, 2000); // maximise tension
        RotateMotor(OUT_A, 100, 120); // turn
        OnForDurationUnregulated(OUT_A, -2, 1000); // jam backwards
    }

    void resetClaw() {
        OnForDurationUnregulated(OUT_A, 30, 3000); // lock
        for (int i = 0; i < 2; ++i) operation::turnClaw();
    }

    void collectOne() {
        OnForDurationUnregulated(OUT_A, 25, 2000); // maximise tension
        RotateMotor(OUT_A, -100, 700); // open
            RotateMotor(OUT_D, 5, 48); // lower
        RotateMotor(OUT_A, 100, 600); // close
        OnForDurationUnregulated(OUT_A, 25, 2000); // maximise tension
        //OnForDurationUnregulated(OUT_A, -2, 1000); // jam backwards
        //turnClaw();

        //PlayToneEx(880, 2000, 100);
        //Wait(2000);
        OnFwdReg(OUT_D, -20);    // lift up again
        //Wait(2000);
    }

    void depositOne(problem::orientation startPos, problem::orientation endPos, problem::color color) {
        int diff = (static_cast<int>(endPos) - static_cast<int>(startPos) + 4) % 4;
        for (int i = 0; i < diff; ++i) {
            turnClaw();
        }

        if (endPos == problem::orientation::WEST) {
            if (color == problem::color::RED) RotateMotor(OUT_C, -20, 10);    // adjustment
            else if (color == problem::color::BLUE) RotateMotor(OUT_B, -20, 10);

            Wait(2000);
            RotateMotor(OUT_D, 1, 43);
            RotateMotor(OUT_BC, -12, 60);
            RotateMotor(OUT_A, -100, 700); // open
        } else if (endPos == problem::orientation::EAST) {
            if (color == problem::color::RED) RotateMotor(OUT_C, -20, 10);    // adjustment
            else if (color == problem::color::BLUE) RotateMotor(OUT_B, -20, 10);

            RotateMotor(OUT_BC, -12, 90);
            RotateMotor(OUT_D, 1, 43);
            RotateMotor(OUT_BC, 12, 80);
            RotateMotor(OUT_A, -100, 700); // open
        } else {
            Wait(2000);
            RotateMotor(OUT_D, 1, 45);
            RotateMotor(OUT_BC, -12, 65);
            operation::rotateSync(-12, 12, 500);
            operation::rotateSync(12, -12, 500);
            //operation::rotateSync(-12, 12, 30, 30);
            //operation::rotateSync(12, -12, 30, 30);
            RotateMotor(OUT_A, -100, 700); // open
        }
        OnFwdReg(OUT_D, -20);
        Wait(3000);
        RotateMotor(OUT_A, 100, 700); // close

        OnForDurationUnregulated(OUT_A, -2, 1000); // jam backwards

        if (diff % 2 == 1) turnClaw();
        //Wait(2000);
    }

    void beep() {
        { PlayToneEx(880, 100, 100); Wait(150); // beep
            PlayToneEx(880, 100, 100); Wait(150);
            PlayToneEx(880, 100, 100); Wait(100); }
    }
}

namespace tests {
    void DepositTest() {
        OnFwdReg(OUT_D, -20);
        while (ReadEV3ColorSensorRGB(IN_3).blue > 120) {
            //operation::ptrace(IN_4, 20, -15, -0.07);
            int sensor = IN_4;
            int threshold = 20;
            int avg = -15;
            double aggressionHi = 0.07;
            double aggressionLo = 0.07;
            int dirn = 1;

            int val = ReadEV3ColorSensorLight(sensor, ReflectedLight);
            int err = abs(val-threshold);

            if (dirn == -1) {
                if (val < threshold) {
                    OnFwdReg(OUT_C, avg - aggressionLo * err);
                    OnFwdReg(OUT_B, avg + aggressionLo * err);
                } else {
                    OnFwdReg(OUT_C, avg + aggressionHi * err);
                    OnFwdReg(OUT_B, avg - aggressionHi * err);
                }
            } else {
                if (val < threshold) {
                    OnFwdReg(OUT_B, avg - aggressionLo * err);
                    OnFwdReg(OUT_C, avg + aggressionLo * err);
                } else {
                    OnFwdReg(OUT_B, avg + aggressionHi * err);
                    OnFwdReg(OUT_C, avg - aggressionHi * err);
                }
            }
        }
        Off(OUT_B); Off(OUT_C);

        problem::orientation o = problem::orientation::NORTH;
        if (o == problem::orientation::WEST) {
            RotateMotor(OUT_BC, -12, 90);
            RotateMotor(OUT_D, 1, 43);
            RotateMotor(OUT_BC, 12, 80);
            RotateMotor(OUT_A, -100, 700); // open
        } else if (o == problem::orientation::EAST) {
            RotateMotor(OUT_C, -20, 10);    // adjustment
            Wait(2000);
            RotateMotor(OUT_D, 1, 43);
            RotateMotor(OUT_BC, -12, 60);
            RotateMotor(OUT_A, -100, 700); // open
        } else {
            Wait(2000);
            RotateMotor(OUT_D, 1, 45);
            RotateMotor(OUT_BC, -12, 65);
            operation::rotateSync(-12, 12, 500);
            operation::rotateSync(12, -12, 500);
            //operation::rotateSync(-12, 12, 30, 30);
            //operation::rotateSync(12, -12, 30, 30);
            RotateMotor(OUT_A, -100, 700); // open
        }
        OnFwdReg(OUT_D, -20);
        Wait(3000);
        RotateMotor(OUT_A, 100, 720); // close
    }

    void PIDTraceTest(int sensor, int threshold, int dirn = -1, bool verbose = true) {
        // -1 for left, 1 for right

        // dt=0.1, max=100, min=0, Kp=0.1, Ki=0.5, Kd=0.01
        double Ku = 0.5, Tu = 0.78;
        PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
        //PID pid = PID(0.001, 20, -20, Ku/5.0, (2.0/5.0)*Ku/Tu, Ku*Tu/15.0);    // No overshoot
        while (true) {
            int val = ReadEV3ColorSensorLight(sensor, ReflectedLight);
            double ret = pid.calculate(threshold, val) * dirn;

            OnFwdReg(OUT_B, -50-ret/2);  // use the ret val as difference between
            OnFwdReg(OUT_C, -50+ret/2);  // the two motors

            if (verbose) {
                //LcdClean(); LcdTextf(1, 20, 50, "VAL %03d RET %d", val, (int)ret);
                TermPrintln("VAL %03d RET %.10f", val, ret);
            }
            Wait(1);
        }
    }

    void ExpoRampTest(double maxPower, double totalDegrees) {
        // -1 for left, 1 for right

        double rampUpScale = 0.15;
        int millisUpElapsed = 0;
        double power = 0;
        ResetRotationCount(OUT_B);
        while (power < maxPower) {
            power = exp(millisUpElapsed * rampUpScale);

            OnFwdUnreg(OUT_B, -power);
            OnFwdUnreg(OUT_C, -power*0.92);

            Wait(1);
            ++millisUpElapsed;
        }

        while (-MotorRotationCount(OUT_B) < totalDegrees - 3*maxPower) {
            OnFwdUnreg(OUT_B, -maxPower);
            OnFwdUnreg(OUT_C, -maxPower*0.92);

            Wait(1);
        }

            LcdClean(); 
            LcdTextf(1,20,20,"a %d\n", -MotorRotationCount(OUT_B));

        double rampDownScale = 0.05;
        int millisDownElapsed = 0;
        while (-MotorRotationCount(OUT_B) < totalDegrees) {
            assert(-MotorRotationCount(OUT_B) >= 0);
            power = exp((double)millisUpElapsed * rampUpScale - (double)millisDownElapsed * rampDownScale) + 10;

            OnFwdUnreg(OUT_B, -power);
            OnFwdUnreg(OUT_C, -power*0.92);

            Wait(1);
            ++millisDownElapsed;
        }

        Off(OUT_B); Off(OUT_C);
    }

    void jonsIdeaDraft() {
        double Ku = 0.5, Tu = 0.78;
        PID pid = PID(0.001, 15, -15, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
        //PID pid = PID(0.001, 20, -20, Ku/5.0, (2.0/5.0)*Ku/Tu, Ku*Tu/15.0);    // No overshoot
        while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
            int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
            double ret = pid.calculate(50, val) * 1;

            OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
            OnFwdReg(OUT_C, -15+ret/2);  // the two motors
            Wait(1);
        }
        Off(OUT_B); Off(OUT_C);

        RotateMotor(OUT_C, -20, arc90-10); // align against wall
        OnForDuration(OUT_BC, 35, 1500);

        int nodeColor[2][6] = {-1};
        for (int elapsed = 0; elapsed*0.2 < 40; ++elapsed) {    // go to next
            OnFwdReg(OUT_BC, -(elapsed*0.2));
            Wait(1);
        }
        for (int i = 0; i < 3; ++i) {
            OnFwdReg(OUT_BC, -40);
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1);
            ResetRotationCount(OUT_B);
            while (abs(MotorRotationCount(OUT_B)) < 80) Wait(1);

            if (ReadEV3ColorSensorLight(IN_1, ReflectedLight) < 15) {   // check colour
                nodeColor[0][i] = 0;
                std::thread a([]() {
                        PlayToneEx(261, 100, 100); Wait(150);
                        PlayToneEx(261, 100, 100); Wait(100);
                        });

                a.detach();
            } else {
                nodeColor[0][i] = 1;
                std::thread a([]() {
                        PlayToneEx(261, 100, 100); Wait(100);
                        });

                a.detach();
            }
        }
        OnFwdReg(OUT_BC, -20);
        while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) Wait(1);
        ResetRotationCount(OUT_B);
        while (abs(MotorRotationCount(OUT_B)) < 150) Wait(1);
        Off(OUT_BC);
        operation::rotateSync(12, -12, arc90/2, arc90/2);

        OnFwdReg(OUT_BC, 12);   // collect fibre optic
        ResetRotationCount(OUT_B);
        while (abs(MotorRotationCount(OUT_B)) < 240) Wait(1);
        Off(OUT_BC);
        OnFwdReg(OUT_D, -12);
        Wait(1000);

        OnForDuration(OUT_BC, 20, 1000);    // adjust to next row
        OnFwdReg(OUT_BC, -20);
        for (int i = 0; i < 2; ++i) {
            if (i > 0) {
                ResetRotationCount(OUT_B);
                while (abs(MotorRotationCount(OUT_B)) < 2*unit) Wait(1);
            }
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1);
        }
        Off(OUT_BC);
        RotateMotor(OUT_B, -12, arc90);
        OnForDuration(OUT_BC, -35, 1000);
    }
}

namespace solution {
    // robot states
    //
    problem::orientation deviceOrientation[4];
    int nodeColor[2][6] = {-1};

    // robot subroutines
    // 
    void p_moveToY0FromRed() {
        RotateMotor(OUT_BC, 20, 300);   // move back and turn
        RotateMotor(OUT_C, 20, arc90);

        { // ram back against wall
            int elapsed = 0; 
            for (; elapsed*0.2 < 35; ++elapsed) {
                OnFwdReg(OUT_BC, (elapsed*0.2));
                Wait(1);
            }
            while (elapsed < 1500) {
                Wait(1);
                ++elapsed;
            }
            Off(OUT_BC);
        }

        RotateMotor(OUT_B, -20, arc90); // turn and line trace
        {
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 10) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * 1;

                OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
        RotateMotor(OUT_C, -20, arc90); // turn again to face south

        { // currently north of y0. move south to reach y0
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * 1;

                OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
    }

    void p_moveToY2FromBlue() {
        RotateMotor(OUT_BC, 20, 300);   // move back and turn
        RotateMotor(OUT_B, 20, arc90);

        { // ram back against wall
            int elapsed = 0; 
            for (; elapsed*0.2 < 35; ++elapsed) {
                OnFwdReg(OUT_BC, (elapsed*0.2));
                Wait(1);
            }
            while (elapsed < 1500) {
                Wait(1);
                ++elapsed;
            }
            Off(OUT_BC);
        }

        RotateMotor(OUT_C, -20, arc90); // turn and line trace
        {
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 10) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * -1;

                OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
        RotateMotor(OUT_B, -20, arc90); // turn again to face north

        { // currently south of y2. move north to reach y2
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * -1;

                OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
    }

    void p_moveOneUnitNorth() { // starts on a black line
        { // skip the current line
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            ResetRotationCount(OUT_B);
            while (abs(MotorRotationCount(OUT_B)) < 120) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * -1;

                OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
        { // move until next line
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * -1;

                OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
        { // move until half past next line
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) < 35) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * -1;

                OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
    }

    // solution subroutines
    //
    void s00_init() {
        //OutputPolarity(OUT_B, -1);
        //OutputPolarity(OUT_C, -1);
        
        SetSensor(IN_1, EV3Color);
        SetSensor(IN_2, EV3Color);
        SetSensor(IN_3, EV3Color);
        SetSensor(IN_4, EV3Color);

        operation::resetClaw();
    }

    void s01_scanBlocks() {
        for (int i = 0; i < 4; ++i) {
            RotateMotor(OUT_BC, -20, 60 + i*5);

            while (ReadEV3ColorSensorLight(IN_1, ReflectedLight) < 7) { // TODO: replace with a legit threshold for blocks
                operation::linetrace(IN_4, 90, -15, -12);
                LcdClean();
                LcdTextf(1, 0, 0, "AAAAA %d ", ReadEV3ColorSensorLight(IN_1, ReflectedLight));
                Wait(10);
            }
            Off(OUT_B); Off(OUT_C);
            PlayToneEx(880, 1000, 100);

            RGB val = ReadEV3ColorSensorRGB(IN_1);
            int color = operation::getColor(val);
            deviceOrientation[color] = static_cast<problem::orientation>(i);
            Wait(1000);
        }
    }

    void s02_scanNodes() {
        {
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 15, -15, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            //PID pid = PID(0.001, 20, -20, Ku/5.0, (2.0/5.0)*Ku/Tu, Ku*Tu/15.0);    // No overshoot
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * 1;

                OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }

        RotateMotor(OUT_C, -20, arc90-10); // align against wall
        Wait(200);
        //OnForDuration(OUT_BC, 35, 1500);
        {
            int elapsed = 0; 
            for (; elapsed*0.2 < 35; ++elapsed) {
                OnFwdReg(OUT_BC, (elapsed*0.2));
                Wait(1);
            }
            while (elapsed < 1500) {
                Wait(1);
                ++elapsed;
            }
            Off(OUT_BC);
        }

        for (int elapsed = 0; elapsed*0.2 < 40; ++elapsed) {    // go to next
            OnFwdReg(OUT_BC, -(elapsed*0.2));
            Wait(1);
        }
        for (int i = 0; i < 3; ++i) {
            //OnFwdReg(OUT_BC, -40);
            //while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1);
            //ResetRotationCount(OUT_B);
            //while (abs(MotorRotationCount(OUT_B)) < 60) Wait(1);
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                operation::ptrace(IN_4, 50, -15, -0.07);
            }
            ResetRotationCount(OUT_B);
            while (abs(MotorRotationCount(OUT_B)) < 60) {
                operation::ptrace(IN_4, 50, -15, -0.07);
            }

            LcdTextf(1,20,i*20, "VAL %03d", ReadEV3ColorSensorLight(IN_1, ReflectedLight));
            if (ReadEV3ColorSensorLight(IN_1, ReflectedLight) < 15) {   // check colour
                nodeColor[1][i] = 0;
                std::thread a([]() {
                        PlayToneEx(261, 100, 100); Wait(150);
                        PlayToneEx(261, 100, 100); Wait(100);
                        });

                a.detach();
            } else {
                nodeColor[1][i] = 1;
                std::thread a([]() {
                        PlayToneEx(261, 100, 100); Wait(100);
                        });

                a.detach();
            }
        }
        {
            int elapsed = 0; 
            for (; -40+elapsed*0.2 < 20; ++elapsed) {    // go to next
                OnFwdReg(OUT_BC, (-40+elapsed*0.2));
                Wait(1);
            }
            while (elapsed < 5000) {
                Wait(1);
                ++elapsed;
            }
            Off(OUT_BC);
        }
        RotateMotor(OUT_B, -20, arc90);
        {
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 15, -15, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            //PID pid = PID(0.001, 20, -20, Ku/5.0, (2.0/5.0)*Ku/Tu, Ku*Tu/15.0);    // No overshoot
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * -1;

                OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }

        RotateMotor(OUT_C, -20, arc90-10); // align against wall
        Wait(200);
        //OnForDuration(OUT_BC, 35, 1500);
        {
            int elapsed = 0; 
            for (; elapsed*0.2 < 35; ++elapsed) {
                OnFwdReg(OUT_BC, (elapsed*0.2));
                Wait(1);
            }
            while (elapsed < 1500) {
                Wait(1);
                ++elapsed;
            }
            Off(OUT_BC);
        }

        for (int elapsed = 0; elapsed*0.2 < 40; ++elapsed) {    // go to next
            OnFwdReg(OUT_BC, -(elapsed*0.2));
            Wait(1);
        }
        for (int i = 0; i < 3; ++i) {
            //OnFwdReg(OUT_BC, -40);
            //while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1);
            //ResetRotationCount(OUT_B);
            //while (abs(MotorRotationCount(OUT_B)) < 60) Wait(1);
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                operation::ptrace(IN_4, 50, -15, -0.07);
            }
            ResetRotationCount(OUT_B);
            while (abs(MotorRotationCount(OUT_B)) < 60) {
                operation::ptrace(IN_4, 50, -15, -0.07);
            }

            LcdTextf(1,20,i*20, "VAL %03d", ReadEV3ColorSensorLight(IN_1, ReflectedLight));
            if (ReadEV3ColorSensorLight(IN_1, ReflectedLight) < 15) {   // check colour
                nodeColor[0][i] = 0;
                std::thread a([]() {
                        PlayToneEx(261, 100, 100); Wait(150);
                        PlayToneEx(261, 100, 100); Wait(100);
                        });

                a.detach();
            } else {
                nodeColor[0][i] = 1;
                std::thread a([]() {
                        PlayToneEx(261, 100, 100); Wait(100);
                        });

                a.detach();
            }
        }
    }

    void s03_depositSet1() {
        bool blueDone = false, redDone = false;

        if (nodeColor[1][2] == 0) { 
            { // MARK: collect
                OnFwdReg(OUT_D, -20);
                OnFwdReg(OUT_BC, 12);
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 35) Wait(1);
                Off(OUT_BC);

                RotateMotor(OUT_C, -20, arc90);

                {
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 50, -50, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    ResetRotationCount(OUT_B);
                    while (abs(MotorRotationCount(OUT_B)) < 50) {
                        int val = ReadEV3ColorSensorLight(IN_3, ReflectedLight);
                        double ret = pid.calculate(60, val) * -1;   // 52 vs 90

                        OnFwdReg(OUT_B, -10-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -10+ret/2);  // the two motors
                        Wait(1);
                    }
                    Off(OUT_B); Off(OUT_C);
                }
                operation::collectOne();
            }

            { // MARK: deposit blue
                OnFwdReg(OUT_D, -20);
                RotateMotor(OUT_BC, -20, 400);

                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 10) {
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(15*4, val*4) * -1;

                    OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                    Wait(1);
                }
                Off(OUT_B); Off(OUT_C);

                operation::depositOne(problem::orientation::EAST,
                        deviceOrientation[static_cast<int>(problem::color::BLUE)],
                        problem::color::BLUE);
            }

            blueDone = true;

            { // MARK: go back to y2
                p_moveToY2FromBlue();
            }

            { // MARK: go to y1
                p_moveOneUnitNorth();
            }
        } else {
            { // MARK: move forward and rotate 180 deg
                RotateMotor(OUT_BC, -20, 250); 
                operation::rotateSync(20, -20, arc90, arc90);
            }

            { // MARK: go to y2
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * -1;

                    OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                    Wait(1);
                }
                Off(OUT_B); Off(OUT_C);
            }

            { // MARK: go to y1
                p_moveOneUnitNorth();
            }
        }

        { PlayToneEx(880, 100, 100); Wait(150);
            PlayToneEx(880, 100, 100); Wait(150);
            PlayToneEx(880, 100, 100); Wait(100); }

        if (nodeColor[1][1] == 0) {
            { // MARK: collect
                OnFwdReg(OUT_D, -20);

                RotateMotor(OUT_B, -20, arc90);
                {
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 50, -50, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    ResetRotationCount(OUT_B);
                    while (abs(MotorRotationCount(OUT_B)) < 50) {
                        int val = ReadEV3ColorSensorLight(IN_3, ReflectedLight);
                        double ret = pid.calculate(60, val) * -1;   // 52 vs 90

                        OnFwdReg(OUT_B, -10-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -10+ret/2);  // the two motors
                        Wait(1);
                    }
                    Off(OUT_B); Off(OUT_C);
                }
                operation::collectOne();
            }

            { // MARK: move until intersection
                OnFwdReg(OUT_D, -20);   // lift
                RotateMotor(OUT_BC, -20, 175);  // move past white blank on field

                { PlayToneEx(880, 100, 100); Wait(150);
                    PlayToneEx(880, 100, 100); Wait(150);
                    PlayToneEx(880, 100, 100); Wait(100); }

                { // move until intersection
                    OnFwdReg(OUT_BC, -20);
                    while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) Wait(1);
                    Off(OUT_B); Off(OUT_C);
                }

                { PlayToneEx(880, 100, 100); Wait(150);
                    PlayToneEx(880, 100, 100); Wait(150);
                    PlayToneEx(880, 100, 100); Wait(100); }
            }

            if (!blueDone) {
                { // MARK: deposit blue
                    { // turn to the line leading to blue
                        RotateMotor(OUT_B, -20, arc90-120);
                        OnFwdReg(OUT_B, -20);
                        while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1);
                        Off(OUT_B);
                        RotateMotor(OUT_C, -20, arc90+15);
                    }

                    { PlayToneEx(880, 100, 100); Wait(150); // beep
                        PlayToneEx(880, 100, 100); Wait(150);
                        PlayToneEx(880, 100, 100); Wait(100); }

                    { // move to blue region
                        double Ku = 0.5, Tu = 0.78;
                        PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                        while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 10) { // 5-6 vs 44
                            int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                            double ret = pid.calculate(50, val) * -1;   // left side of line

                            OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                            OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                            Wait(1);
                        }
                        Off(OUT_B); Off(OUT_C);
                    }

                    operation::depositOne(
                            problem::orientation::EAST, 
                            deviceOrientation[static_cast<int>(problem::color::BLUE)],
                            problem::color::BLUE);
                }

                blueDone = true;

                { // MARK: go back to y2
                    p_moveToY2FromBlue();
                }

                { // MARK: go to y1
                    p_moveOneUnitNorth();
                }

                { // MARK: go to y0
                    p_moveOneUnitNorth();
                }
            } else {
                { // MARK: deposit red
                    { // turn to the line leading to red
                        RotateMotor(OUT_C, -20, arc90-120);
                        OnFwdReg(OUT_C, -20);
                        while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) Wait(1);
                        Off(OUT_C);
                        RotateMotor(OUT_B, -20, arc90+15);
                    }

                    { PlayToneEx(880, 100, 100); Wait(150); // beep
                        PlayToneEx(880, 100, 100); Wait(150);
                        PlayToneEx(880, 100, 100); Wait(100); }

                    { // move to red region
                        double Ku = 0.5, Tu = 0.78;
                        PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                        while (ReadEV3ColorSensorRGB(IN_3).blue > 120) {
                            int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                            double ret = pid.calculate(50, val) * 1;    // right side of line

                            OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                            OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                            Wait(1);
                        }
                        Off(OUT_B); Off(OUT_C);
                    }

                    operation::depositOne(
                            problem::orientation::EAST, 
                            deviceOrientation[static_cast<int>(problem::color::RED)],
                            problem::color::RED);
                }

                redDone = true;

                { // MARK: go to y0 facing south
                    p_moveToY0FromRed();
                }
            }
        } else {
            { // MARK: go to y0
                p_moveOneUnitNorth();
            }
        }

        { PlayToneEx(880, 100, 100); Wait(150);
            PlayToneEx(880, 100, 100); Wait(150);
            PlayToneEx(880, 100, 100); Wait(100); }

        if (nodeColor[1][0] == 0) {
            { // MARK: collect
                OnFwdReg(OUT_D, -20);

                RotateMotor(OUT_B, -20, arc90);
                {
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 50, -50, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    ResetRotationCount(OUT_B);
                    while (abs(MotorRotationCount(OUT_B)) < 50) {
                        int val = ReadEV3ColorSensorLight(IN_3, ReflectedLight);
                        double ret = pid.calculate(60, val) * -1;   // 52 vs 90

                        OnFwdReg(OUT_B, -10-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -10+ret/2);  // the two motors
                        Wait(1);
                    }
                    Off(OUT_B); Off(OUT_C);
                }
                operation::collectOne();
            }

            { // MARK: deposit red
                OnFwdReg(OUT_D, -20);
                RotateMotor(OUT_BC, -20, 400);

                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorRGB(IN_3).blue > 120) {
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * 1;

                    OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                    Wait(1);
                }
                Off(OUT_B); Off(OUT_C);

                operation::depositOne(problem::orientation::EAST,
                        deviceOrientation[static_cast<int>(problem::color::RED)],
                        problem::color::RED);
            }

            redDone = true;

            { // MARK: go to y0 facing south
                p_moveToY0FromRed();
            }
        }
    }
}

int main() {
    operation::monitorExit();
    // MARK debugging: hardcode data
    using namespace solution;
    deviceOrientation[static_cast<int>(problem::color::RED)] = problem::orientation::EAST;
    deviceOrientation[static_cast<int>(problem::color::BLUE)] = problem::orientation::EAST;
    deviceOrientation[static_cast<int>(problem::color::GREEN)] = problem::orientation::NORTH;
    deviceOrientation[static_cast<int>(problem::color::YELLOW)] = problem::orientation::SOUTH;
    nodeColor[0][0] = 0;
    nodeColor[0][1] = 1;
    nodeColor[0][2] = 0;
    nodeColor[1][0] = 0;
    nodeColor[1][1] = 1;
    nodeColor[1][2] = 0;

    // MARK end debugging

    //solution::s00_init();
    //PlayToneEx(440, 1000, 100);
    //solution::s01_scanBlocks();
     
    // MARK: scan nodes
    //solution::s02_scanNodes();

    // MARK: deposit 2 nodes to red blue
    //solution::s03_depositSet1();
    
    // MARK: deposit 1 cable to red-blue and 1 node to green
    { // Adjust robot to face east
        OnForDuration(OUT_BC, 35, 2000);
        OnFwdReg(OUT_BC, -20);
        while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 10) Wait(1);
        Off(OUT_BC);
        RotateMotor(OUT_C, -20, arc90);
        Wait(100);
    }

    { // grab the cable and position for node grabbing
        { // move back to reach cable
            ResetRotationCount(OUT_B);
            for (int elapsed = 0; MotorRotationCount(OUT_B) < 370; ++elapsed) {
                OnFwdReg(OUT_BC, std::min(15.0, elapsed*0.1));
                Wait(1);
            }
            //RotateMotor(OUT_BC, 15, 370); 
        }
        RotateMotor(OUT_D, -30, 60);    // grab
        RotateMotor(OUT_BC, -20, 120);  // move a bit forward
        OnFwdReg(OUT_D, -50);           // lift higher
        Wait(1000);
        OnForDuration(OUT_BC, 35, 2000);// ram backwards

        { // reach intersection
            OnFwdReg(OUT_BC, -20);
            while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 10) Wait(1);
            Off(OUT_BC);
        }

        RotateMotor(OUT_B, -20, arc90); // turn and ram again
        OnForDuration(OUT_BC, 35, 2000);

        RotateMotor(OUT_BC, -20, 120); // move onto the line for PID
    }

    operation::beep();

    for (int i = 0; i < 2; ++i) {   // grab the first available node
        { // PID to next intersection
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 10) { // get to the next line
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * -1;

                OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }

        if (nodeColor[0][i] == 0) { // if black here
            RotateMotor(OUT_BC, -20, 120); // skip current intersection

            { // move to the next intersection
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 10) { // get to the next line
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * -1;

                    OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                    Wait(1);
                }
                Off(OUT_B); Off(OUT_C);
            }
            { // align the robot properly
                RotateMotor(OUT_BC, -20, 0.5*unit);
                RotateMotor(OUT_C, 20, arc90);
                while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) {
                    OnFwdReg(OUT_C, 15);
                    Wait(10);
                }
                Off(OUT_C);
            }
            { // collection
                RotateMotor(OUT_BC, -20, 130); // move foward
                operation::collectOne();
                OnFwdReg(OUT_D, -50);   // lift with higher power as it is heavier
                RotateMotor(OUT_BC, 20, 130); // go back on the line
            }
            { // return to reference point
                RotateMotor(OUT_C, -20, arc90); // turn and ram back against wall
                OnForDuration(OUT_BC, 35, (i+1)*2000);
            }
            break;
        }
    }

    operation::beep();

    { // Deposit cable
        // Move to the next column
        //RotateMotor(OUT_BC, -20, 120);
        RotateMotor(OUT_C, -20, arc90);
        {
            OnFwdReg(OUT_BC, -20);
            while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 10) Wait(1);
            Off(OUT_BC);
        }

        // Move to the center row
        RotateMotor(OUT_B, -20, arc90);
        OnForDuration(OUT_BC, 20, 2000);
        RotateMotor(OUT_BC, -20, 120); // move onto the line for PID
        for (int i = 0; i < 2; ++i) {
            p_moveOneUnitNorth();
        }

        // Rotate to position robot
        RotateMotor(OUT_C, -20, arc90);
        operation::rotateSync(-20, 20, arc90, arc90);

        { // deposit
            { // move until deposit point
                ResetRotationCount(OUT_B);
                for (int elapsed = 0; MotorRotationCount(OUT_B) < 200; ++elapsed) {
                    OnFwdReg(OUT_BC, std::min(15.0, elapsed*0.1));
                    Wait(1);
                }
                //RotateMotor(OUT_BC, 12, 300);
            }
            Off(OUT_D);
            Wait(1000);
        }
    }

    // a*(x+240)*(x-b) = a*(x-(-240+b)/2)^2 + 100;
    // a*240*-b = 12
    // wolfram alpha: a = -0.0065, b = 7.67
    //double a = -0.0065, b = 7.67;

    //ResetRotationCount(OUT_B);
    //while (MotorRotationCount(OUT_B) > -240) {
    //    int x = -abs(MotorRotationCount(OUT_B));
    //    OnFwdReg(OUT_BC, std::max(-40.0, a*(x+240)*(x-b)));   // clip at -40
    //    if (MotorRotationCount(OUT_B) > 240) {
    //        OnFwdReg(OUT_D, -12);
    //    }
    //}
    //Off(OUT_BC);

    //PlayToneEx(440, 1000, 100);
    Wait(1000);
}
