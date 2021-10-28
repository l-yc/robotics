#include <ev3.h>
#include <thread>
#include <cmath>
#include <cassert>
#include "pid.h"

const int sen2to4 = 68;
const int arc90 = 442; //446; //436;
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
        EAST, SOUTH, WEST, NORTH, NONE
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

    void beep() {
        std::thread aaaaaaaaaaaaaaaaaaa([]() {
            { PlayToneEx(880, 100, 100); Wait(150); // beep
                PlayToneEx(880, 100, 100); Wait(150);
                PlayToneEx(880, 100, 100); Wait(100); }
            });
        aaaaaaaaaaaaaaaaaaa.detach();
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

    problem::color getColor(RGB rgb) {
        // g 0.7 r 2.0 y 3
        if ((double) rgb.red / (rgb.blue + rgb.green) > 2.0)
            return problem::color::RED;
        else if ((double) rgb.green / (rgb.red + rgb.blue) > 0.7)
            return problem::color::GREEN;
        else if ((double) (rgb.red + rgb.green) / rgb.blue > 3.0)
            return problem::color::YELLOW;
        else
            return problem::color::BLUE;
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
        //`:`jOnForDurationUnregulated(OUT_A, 25, 2000); // maximise tension
        //`:`jRotateMotor(OUT_A, 100, 115); //120); // turn
        //`:`jOnForDurationUnregulated(OUT_A, -2, 1000); // jam backwards
    }

    void resetClaw() {
        OnForDurationUnregulated(OUT_A, 30, 3000); // lock
        for (int i = 0; i < 2; ++i) operation::turnClaw();
    }

    void collectOne() {
        OnForDurationUnregulated(OUT_A, 25, 2000); // maximise tension
        RotateMotor(OUT_A, -100, 700); // open
            //RotateMotor(OUT_D, 5, 48); // lower
            //RotateMotor(OUT_D, 3, 42); // lower
            RotateMotor(OUT_D, 25, 50); // lower
            //ResetRotationCount(OUT_D);
            
            //while (abs(MotorRotationCount(OUT_D)) < 53) {
            //    int err = 53 - abs(MotorRotationCount(OUT_D));
            //    OnFwdReg(OUT_D, std::min(30.0, 3*exp(0.2*err)));
            //}
            //OnFwdReg(OUT_D, 1);
        RotateMotor(OUT_A, 100, 600); // close
        OnForDurationUnregulated(OUT_A, 25, 2000); // maximise tension
        //OnForDurationUnregulated(OUT_A, -2, 1000); // jam backwards
        //turnClaw();

        //PlayToneEx(880, 2000, 100);
        //Wait(2000);
        OnFwdReg(OUT_D, -25);    // lift up again
        //Wait(2000);
    }

    void setClawOrientation(problem::orientation startPos, problem::orientation endPos) {
        int diff = (static_cast<int>(endPos) - static_cast<int>(startPos) + 4) % 4;
        for (int i = 0; i < diff; ++i) {
            turnClaw();
        }
    }

    void depositOne(problem::orientation startPos, problem::orientation endPos, problem::color color) {
        if (endPos == problem::orientation::NONE) {
            beep();
            endPos = problem::orientation::NORTH;
        }

        int diff = (static_cast<int>(endPos) - static_cast<int>(startPos) + 4) % 4;
        //for (int i = 0; i < diff; ++i) {
        //    turnClaw();
        //}

        if (endPos == problem::orientation::WEST) {
            if (color == problem::color::RED or color == problem::color::YELLOW)
                RotateMotor(OUT_C, -20, 10);    // adjustment
            else //(color == problem::color::BLUE or color == problem::color::GREEN)
                RotateMotor(OUT_B, -20, 10);

            //Wait(2000);
            RotateMotor(OUT_D, 12, 43);
            RotateMotor(OUT_BC, -12, 60);
            RotateMotor(OUT_A, -100, 700); // open
        } else if (endPos == problem::orientation::EAST) {
            if (color == problem::color::RED or color == problem::color::YELLOW)
                RotateMotor(OUT_C, -20, 10);    // adjustment
            else //(color == problem::color::BLUE or color == problem::color::GREEN)
                RotateMotor(OUT_B, -20, 10);

            RotateMotor(OUT_BC, -12, 90);
            RotateMotor(OUT_D, 12, 43);
            RotateMotor(OUT_BC, 12, 80);
            RotateMotor(OUT_A, -100, 700); // open
        } else {
            //Wait(2000);
            RotateMotor(OUT_D, 12, 45);
            RotateMotor(OUT_BC, -12, 65);
            if (color == problem::color::RED or color == problem::color::YELLOW) { // wobble
                operation::rotateSync(12, -12, 30, 40);
                operation::rotateSync(-12, 12, 30, 40);
            }
            else { //(color == problem::color::BLUE or color == problem::color::GREEN)
                //operation::rotateSync(-12, 12, 500);
                //operation::rotateSync(12, -12, 500);
                operation::rotateSync(-12, 12, 30, 40);
                operation::rotateSync(12, -12, 30, 40);
            }
            RotateMotor(OUT_A, -100, 700); // open
        }
        OnFwdReg(OUT_D, -25);
        Wait(1000);
        RotateMotor(OUT_A, 100, 700); // close

        OnForDurationUnregulated(OUT_A, -2, 1000); // jam backwards

        if (diff % 2 == 1) {
            std::thread a([]() {
                turnClaw();
            });
            a.detach();
        }
        //Wait(2000);
    }

    void grabCable() {
        OnFwdReg(OUT_D, 10); Wait(1000); // slam down the claw
        { // move back to reach the cable
            for (int elapsed = 0; elapsed < 900; ++elapsed) {
                OnFwdReg(OUT_BC, std::min(15.0, elapsed*0.1));
                Wait(1);
            }
            Off(OUT_BC);
        }
        OnFwdReg(OUT_D, -100); Wait(500); // lift at lower power // c_EXPERIMENTAL_02 : change from -25 to -100
        OnForDuration(OUT_BC, 35, 1500);// ram backwards
    }

    void dropCable() {
        { // deposit
            { // move until deposit point
                ResetRotationCount(OUT_B);
                for (int elapsed = 0; MotorRotationCount(OUT_B) < 320; ++elapsed) {
                    OnFwdReg(OUT_BC, std::min(15.0, elapsed*0.1));
                    Wait(1);
                }
                Off(OUT_BC);
                //RotateMotor(OUT_BC, 12, 300);
            }
            OnForDuration(OUT_D, 20, 1000);
        }
    }
}

namespace tests {
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

    void ExpoRampTest2() {
        //// a*(x+240)*(x-b) = a*(x-(-240+b)/2)^2 + 100;
        //// a*240*-b = 12
        ////     wolfram alpha: a = -0.0065, b = 7.67
        ////     double a = -0.0065, b = 7.67;

        //ResetRotationCount(OUT_B);
        //while (MotorRotationCount(OUT_B) > -240) {
        //    int x = -abs(MotorRotationCount(OUT_B));
        //    OnFwdReg(OUT_BC, std::max(-40.0, a*(x+240)*(x-b)));   // clip at -40
        //    if (MotorRotationCount(OUT_B) > 240) {
        //        OnFwdReg(OUT_D, -12);
        //    }
        //}
        //Off(OUT_BC);
    }
}

namespace solution {
    // robot states
    //
    problem::orientation deviceOrientation[4];
    int nodeColor[2][3] = {-1};

    // robot subroutines
    // 
    void p_moveToY0FromRed() {
        RotateMotor(OUT_BC, 20, 100);   // move back, but dont hit the white block
        RotateMotor(OUT_C, 20, arc90);  // turn
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
        { // trace back to corner
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * 1;

                OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
        RotateMotor(OUT_C, -20, arc90); // turn at the corner to face south
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
        RotateMotor(OUT_BC, 20, 100);   // move back, but dont hit white node
        RotateMotor(OUT_B, 20, arc90); // turn
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

        RotateMotor(OUT_C, -20, arc90); // turn for line trace
        { // trace back to the corner
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * 1; // c_EXPERIMENTAL_01 : switch from left to right

                OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
        RotateMotor(OUT_B, -20, arc90); // turn at the corner to face north
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
        double Ku = 0.5, Tu = 0.78;
        PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
        { // skip the current line
            ResetRotationCount(OUT_B);
            while (abs(MotorRotationCount(OUT_B)) < 120) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * 1; // c_EXPERIMENTAL_01 : switch from left to right

                OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
        { // move until next line
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * 1; // c_EXPERIMENTAL_01 : switch from left to right

                OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);
        }
        //{ // move until half past next line   // c_EXPERIMENTAL_03 : commented this
        //    while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) < 35) {
        //        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
        //        double ret = pid.calculate(50, val) * -1;

        //        OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
        //        OnFwdReg(OUT_C, -20+ret/2);  // the two motors
        //        Wait(1);
        //    }
        //    Off(OUT_B); Off(OUT_C);
        //}
    }

    // solution subroutines
    //
    void s00_init() {
        //OutputPolarity(OUT_B, -1);
        //OutputPolarity(OUT_C, -1);
        
        //SetSensor(IN_1, EV3Color);
        //SetSensor(IN_2, EV3Color);
        //SetSensor(IN_3, EV3Color);
        //SetSensor(IN_4, EV3Color);

        for (int i = 0; i < 4; ++i) deviceOrientation[i] = problem::orientation::NONE;
        operation::resetClaw();
    }

    void s01_scanBlocks() {
        RotateMotor(OUT_BC, -20, 170);
        operation::rotateSync(-12, 12, arc90/2, arc90/2);
        OnForDuration(OUT_BC, 15, 1500);
        std::thread a([]() {
                RotateMotor(OUT_BC, -30, 1000);
            });
        problem::color colAtPos[4];
        for (int i = 0; i < 4; ++i) {
            int rawColor = -1;
            while (rawColor < 2 or rawColor > 5) {
                rawColor = ReadEV3ColorSensor(IN_1);
                Wait(1);
            }

            problem::color color;
            switch (rawColor) {
                case 2:
                    color = problem::color::BLUE;
                    break;
                case 3:
                    {
                        RGB rgb = ReadEV3ColorSensorRGB(IN_1);
                        color = (rgb.blue >= rgb.green ? problem::color::BLUE : problem::color::GREEN);
                    }
                    break;
                case 4:
                    color = problem::color::YELLOW;
                    break;
                case 5:
                    color = problem::color::RED;
                    break;
            }
            
            colAtPos[i] = color;// MARK: debug
            deviceOrientation[static_cast<int>(color)] = static_cast<problem::orientation>(i);
            //std::thread b([color]() {
            //        for (int j = 0; j < static_cast<int>(color); ++j) {
            //            PlayToneEx(440, 100, 100); Wait(150);
            //        }
            //    });
            //b.detach();
        }
        a.join();

        bool ok = true;
        for (int j = 0; j < 4; ++j) for (int k = j+1; k < 4; ++k) ok &= (colAtPos[j] != colAtPos[k]);
        if (ok) { operation::beep(); Wait(2000); }

        // now connect with s02_scanNodes
        // MARK: reach the corner
        {
            //double Ku = 0.5, Tu = 0.78;
            //PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                //int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                //double ret = pid.calculate(50, val) * -1;

                //OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                //OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                OnFwdReg(OUT_BC, -20);
                Wait(1);
            }
            Off(OUT_BC);
        }
        RotateMotor(OUT_BC, -20, unit);   // move forward a little to trace on right side
        RotateMotor(OUT_C, -20, arc90);
        { // MARK: go 4 intersections forward
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            {
                ResetRotationCount(OUT_B);
                while (abs(MotorRotationCount(OUT_B)) < 2000) {
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * 1;

                    OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                    Wait(1);
                }
                PlayToneEx(1000, 100, 100);
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * 1;

                    OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                    Wait(1);
                }
            }
            Off(OUT_BC);
        }
    }

    void s02_scanNodes() { // starts at the intersection in col 1
        { // MARK: rotate to prepare for line trace
            RotateMotor(OUT_C, -20, arc90-10); // turn
            Wait(200);  // avoid slipping
            //{ // ram back against wall
            //    int elapsed = 0; 
            //    for (; elapsed*0.2 < 35; ++elapsed) {
            //        OnFwdReg(OUT_BC, (elapsed*0.2));
            //        Wait(1);
            //    }
            //    while (elapsed < 1500) {
            //        Wait(1);
            //        ++elapsed;
            //    }
            //    Off(OUT_BC);
            //}
        }
        { // MARK: scan column 1
            for (int elapsed = 0; elapsed*0.3 < 40; ++elapsed) {    // ramp up power
                OnFwdReg(OUT_BC, -(elapsed*0.3));
                Wait(1);
            }
            for (int i = 0; i < 3; ++i) { // visit all 3 intersections
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
        }
        { // MARK: travel to column 0
            { // accelerate backwards to ram against wall
                int elapsed = 0; 
                for (; -40+elapsed*0.2 < 35; ++elapsed) {    // go to next
                    OnFwdReg(OUT_BC, (-40+elapsed*0.2));
                    Wait(1);
                }
                while (elapsed < 4000) {
                    Wait(1);
                    ++elapsed;
                }
                Off(OUT_BC);
            }
            RotateMotor(OUT_B, -20, arc90); // turn for line tracing
            { // line trace to next intersection
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
        }
        { // MARK: align against wall
            RotateMotor(OUT_C, -20, arc90-10); // align against wall
            Wait(200);
            { // ram back against wall with ramping
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
        }
        { // MARK: scan column 0
            RotateMotor(OUT_BC, -20, 150);
            //for (int elapsed = 0; elapsed*0.2 < 40; ++elapsed) {    // ramp up power
            //    OnFwdReg(OUT_BC, -(elapsed*0.2));
            //    Wait(1);
            //}
            for (int i = 0; i < 3; ++i) { // visit all 3 intersections
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
        Off(OUT_BC); // make sure to stop the robot
    }

    void s03_depositSet1() {
        bool blueDone = false, redDone = false; // flags for the logic
        if (nodeColor[1][2] == 0) { // definitely deposit in blue
            { // MARK: collect
                OnFwdReg(OUT_D, -25);
                OnFwdReg(OUT_BC, 12);
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 35) Wait(1);
                Off(OUT_BC);

                RotateMotor(OUT_C, -20, arc90);

                //{
                //    double Ku = 0.5, Tu = 0.78;
                //    PID pid = PID(0.001, 50, -50, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                //    ResetRotationCount(OUT_B);
                //    while (abs(MotorRotationCount(OUT_B)) < 50) {
                //        int val = ReadEV3ColorSensorLight(IN_3, ReflectedLight);
                //        double ret = pid.calculate(60, val) * -1;   // 52 vs 90

                //        OnFwdReg(OUT_B, -10-ret/2);  // use the ret val as difference between
                //        OnFwdReg(OUT_C, -10+ret/2);  // the two motors
                //        Wait(1);
                //    }
                //    Off(OUT_B); Off(OUT_C);
                //}
                operation::collectOne();
            }

            { // MARK: deposit blue
                OnFwdReg(OUT_D, -25);
                RotateMotor(OUT_BC, -20, 400);

                std::thread setClawOrientationThread([]() {
                        operation::setClawOrientation(problem::orientation::EAST,
                                deviceOrientation[static_cast<int>(problem::color::BLUE)]);
                    });
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {    // c_MARK_threshold
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(15*4, val*4) * -1;

                    OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                    Wait(1);
                }
                Off(OUT_B); Off(OUT_C);

                setClawOrientationThread.join();
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
                    double ret = pid.calculate(50, val) * 1; // c_EXPERIMENTAL_01 : switch from left to right

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
        operation::beep();
        if (nodeColor[1][1] == 0) { // if not blueDone, do blue, else red
            { // MARK: collect
                OnFwdReg(OUT_D, -25);

                RotateMotor(OUT_B, -20, arc90);
                //{
                //    double Ku = 0.5, Tu = 0.78;
                //    PID pid = PID(0.001, 50, -50, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                //    ResetRotationCount(OUT_B);
                //    while (abs(MotorRotationCount(OUT_B)) < 50) {
                //        int val = ReadEV3ColorSensorLight(IN_3, ReflectedLight);
                //        double ret = pid.calculate(60, val) * -1;   // 52 vs 90

                //        OnFwdReg(OUT_B, -10-ret/2);  // use the ret val as difference between
                //        OnFwdReg(OUT_C, -10+ret/2);  // the two motors
                //        Wait(1);
                //    }
                //    Off(OUT_B); Off(OUT_C);
                //}
                operation::collectOne();
            }

            { // MARK: move until intersection
                OnFwdReg(OUT_D, -25);   // lift
                RotateMotor(OUT_BC, -20, 175);  // move past white blank on field

                //operation::beep();

                { // move until intersection
                    OnFwdReg(OUT_BC, -20);
                    while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) Wait(1);
                    Off(OUT_B); Off(OUT_C);
                }

                //operation::beep();
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

                    operation::beep();
                    std::thread setClawOrientationThread([]() {
                            operation::setClawOrientation(problem::orientation::EAST,
                                    deviceOrientation[static_cast<int>(problem::color::BLUE)]);
                        });

                    { // move to blue region
                        double Ku = 0.5, Tu = 0.78;
                        PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                        while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) { // 5-6 vs 44 c_MARK_threshold
                            int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                            double ret = pid.calculate(50, val) * -1;   // left side of line

                            OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                            OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                            Wait(1);
                        }
                        Off(OUT_B); Off(OUT_C);
                    }

                    setClawOrientationThread.join();
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

                    operation::beep();
                    std::thread setClawOrientationThread([]() {
                            operation::setClawOrientation(problem::orientation::EAST,
                                    deviceOrientation[static_cast<int>(problem::color::RED)]);
                        });

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

                    setClawOrientationThread.join();
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
        operation::beep();
        if (nodeColor[1][0] == 0) { // definitely deposit in red
            { // MARK: collect
                OnFwdReg(OUT_D, -25);

                RotateMotor(OUT_B, -20, arc90);
                //{
                //    double Ku = 0.5, Tu = 0.78;
                //    PID pid = PID(0.001, 50, -50, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                //    ResetRotationCount(OUT_B);
                //    while (abs(MotorRotationCount(OUT_B)) < 50) {
                //        int val = ReadEV3ColorSensorLight(IN_3, ReflectedLight);
                //        double ret = pid.calculate(60, val) * -1;   // 52 vs 90

                //        OnFwdReg(OUT_B, -10-ret/2);  // use the ret val as difference between
                //        OnFwdReg(OUT_C, -10+ret/2);  // the two motors
                //        Wait(1);
                //    }
                //    Off(OUT_B); Off(OUT_C);
                //}
                operation::collectOne();
            }

            { // MARK: deposit red
                OnFwdReg(OUT_D, -25);
                RotateMotor(OUT_BC, -20, 400);

                std::thread setClawOrientationThread([]() {
                        operation::setClawOrientation(problem::orientation::EAST,
                                deviceOrientation[static_cast<int>(problem::color::RED)]);
                    });
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

                setClawOrientationThread.join();
                operation::depositOne(problem::orientation::EAST,
                        deviceOrientation[static_cast<int>(problem::color::RED)],
                        problem::color::RED);
            }

            redDone = true;

            { // MARK: go to y0 facing south
                p_moveToY0FromRed();
            }
        }
        OnForDuration(OUT_D, 20, 500); // smash the claw down
    }

    void s04_depositSet2() {
        { // MARK: adjust robot to face east at intersection
            OnForDuration(OUT_BC, 35, 1500);    // ram backwards against wall
            OnFwdReg(OUT_BC, -20);
            while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) Wait(1); // move until y-1
            Off(OUT_BC);
            RotateMotor(OUT_C, -20, arc90); // rotate to face east
            Wait(100);
        }
        { // MARK: grab the cable and position for node grabbing
            operation::grabCable();

            { // reach intersection
                OnFwdReg(OUT_BC, -20);
                while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) Wait(1);
                Off(OUT_BC);
            }
            RotateMotor(OUT_BC, -20, unit); // guarantee left side tracing

            RotateMotor(OUT_B, -20, arc90); // turn and ram again   c_RAM_CABLE
            OnFwdReg(OUT_D, -75);
            OnForDurationUnregulated(OUT_BC, 35, 1800);

            RotateMotor(OUT_BC, -20, 120); // move onto the line for PID
        }
        operation::beep();
        for (int i = 0; i < 2; ++i) { // MARK: grab the first available node
            { // PID to next intersection
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) { // get to the next line
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * -1;

                    OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                    Wait(1);
                }
                Off(OUT_B); Off(OUT_C);
            }

            operation::beep(); Wait(2000);
            RotateMotor(OUT_BC, -20, 80); // skip current intersection
            if (nodeColor[0][i] == 0) { // if black here
                operation::beep(); operation::beep(); Wait(2000);

                { // move to the next intersection
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) { // get to the next line
                        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                        double ret = pid.calculate(50, val) * -1;

                        OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                        Wait(1);
                    }
                    Off(OUT_B); Off(OUT_C);
                }
                { // align the robot properly
                    RotateMotor(OUT_BC, -20, unit); //0.5*unit);
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
                    RotateMotor(OUT_BC, 12, 130);
                    OnFwdReg(OUT_D, -25);   // lift with higher power as it is heavier
                    //RotateMotor(OUT_BC, 20, 80); // go back on the line
                }
                { // return to reference point
                    RotateMotor(OUT_C, -20, arc90); // turn 
                    OnFwdReg(OUT_D, -75);
                    OnForDurationUnregulated(OUT_BC, 35, (i+1)*3000); // ram back against wall c_RAM_CABLE
                }
                break;
            }
        }
        operation::beep();
        { // MARK: deposit cable
            // Move to the next column
            //RotateMotor(OUT_BC, -20, 120);
            RotateMotor(OUT_C, -20, arc90);
            {
                OnFwdReg(OUT_BC, -20);
                while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) Wait(1);
                Off(OUT_BC);
            }

            // Move to the center row
            RotateMotor(OUT_B, -20, arc90);
            OnForDuration(OUT_BC, 20, 2000);
            RotateMotor(OUT_BC, -20, 120); // move onto the line for PID
            for (int i = 0; i < 2; ++i) {
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
            }

            // Rotate to position robot
            RotateMotor(OUT_C, -20, arc90);
            operation::rotateSync(-20, 20, arc90, arc90);

            operation::dropCable();
        }
        { // MARK: travel to yellow    
            OnFwdReg(OUT_BC, -20); // move away from the cable
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1); // move to intersection
            Off(OUT_BC);
            OnFwdReg(OUT_D, -25); // lift claw

            operation::beep(); Wait(2000);

            RotateMotor(OUT_C, 20, arc90);  // rotate, ram and rotate again for PID
            OnForDuration(OUT_BC, 35, 3000);
            RotateMotor(OUT_B, -20, arc90+10);
            RotateMotor(OUT_BC, -20, 120);  // move forward to guarantee move past the 1st line

            operation::beep(); Wait(2000);

            for (int i = 0; i < 3; ++i) { // go to wall
                { // go to next intersection
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                        double ret = pid.calculate(50, val) * 1;

                        OnFwdReg(OUT_B, -25-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -25+ret/2);  // the two motors

                        Wait(1);
                    }
                }
                if (i < 2) { // skip past current line
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    ResetRotationCount(OUT_B);
                    while (abs(MotorRotationCount(OUT_B)) < 160) {
                        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                        double ret = pid.calculate(50, val) * 1;

                        OnFwdReg(OUT_B, -25-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -25+ret/2);  // the two motors

                        Wait(1);
                    }
                }
                Off(OUT_BC);
                    std::thread a([]() {
                        operation::beep();
                    });
                    a.detach();
                    //Wait(1000);
            }
            Off(OUT_BC);

            RotateMotor(OUT_C, -20, arc90); // rotate
            OnForDuration(OUT_BC, 35, 1500); // ram
            RotateMotor(OUT_BC, -20, 120); // move forward for PID

            operation::beep(); Wait(2000);

            { // MARK: go to next intersection and rotate
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) { // reach line
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * 1;

                    OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -15+ret/2);  // the two motors

                    Wait(1);
                }
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) < 35) { // until half on white
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * 1;

                    OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -15+ret/2);  // the two motors

                    Wait(1);
                }
                Off(OUT_BC);
                RotateMotor(OUT_C, -20, arc90);
                std::thread a([]() {
                        operation::beep();
                        });
                a.detach();
            }
        }
        { // MARK: deposit yellow
            OnFwdReg(OUT_D, -25);

            std::thread setClawOrientationThread([]() {
                    operation::setClawOrientation(problem::orientation::EAST,
                            deviceOrientation[static_cast<int>(problem::color::YELLOW)]);
                });
            double Ku = 0.5, Tu = 0.78;
            PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
            while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) < 70) {
                int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                double ret = pid.calculate(50, val) * 1;

                OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                OnFwdReg(OUT_C, -15+ret/2);  // the two motors
                Wait(1);
            }
            Off(OUT_B); Off(OUT_C);

            setClawOrientationThread.join();
            operation::depositOne(problem::orientation::EAST,
                    deviceOrientation[static_cast<int>(problem::color::YELLOW)],
                    problem::color::YELLOW);
        }
        { // MARK: go to y+1 facing east
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

            RotateMotor(OUT_C, -20, arc90); // turn and line trace

            PlayToneEx(1000, 2000, 100); Wait(2000);

            for (int i = 0; i < 5; ++i) { // reach the corner
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) {
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * -1;

                    OnFwdReg(OUT_B, -25-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -25+ret/2);  // the two motors
                    Wait(1);
                }
                if (i < 4) {
                    ResetRotationCount(OUT_B);
                    while (abs(MotorRotationCount(OUT_B)) < 120) {
                        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                        double ret = pid.calculate(50, val) * -1;

                        OnFwdReg(OUT_B, -25-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -25+ret/2);  // the two motors
                        Wait(1);
                    }
                }
                std::thread a([]() {
                        operation::beep();
                        });
                a.detach();
            }
            Off(OUT_B); Off(OUT_C);
            PlayToneEx(440, 2000, 100); Wait(2000);
            RotateMotor(OUT_B, -20, arc90); // turn again to face north
            OnForDuration(OUT_D, 20, 1000); // lower the claw

            { // ram back against wall
                int elapsed = 0; 
                for (; elapsed*0.2 < 35; ++elapsed) {
                    OnFwdReg(OUT_BC, (elapsed*0.2));
                    Wait(1);
                }
                while (elapsed < 1000) {
                    Wait(1);
                    ++elapsed;
                }
                Off(OUT_BC);
            }

            { // MARK: adjust robot to face east at intersection
                OnFwdReg(OUT_BC, -20);
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1); // move until y+1
                Off(OUT_BC);
                RotateMotor(OUT_B, -20, arc90); // rotate to face east
                Wait(100);
            }
        }
    }

    void s05_depositSet3() {
        { // MARK: grab the cable and position for node grabbing
            operation::grabCable();

            { // reach intersection
                OnFwdReg(OUT_BC, -20);
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1);
                Off(OUT_BC);
            }

            RotateMotor(OUT_C, -20, arc90); // turn
            //OnFwdReg(OUT_D, -75); // c_EXPERIMENTAL_02 : commented this out
            OnForDurationUnregulated(OUT_BC, 30, 1000); // ram again c_RAM_CABLE

            RotateMotor(OUT_BC, -20, 140); // move onto the line for PID
        }
        operation::beep();
        for (int i = 2; i > 0; --i) { // MARK: grab the first available node
            { // PID to next intersection
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) { // get to the next line
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * 1;

                    OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                    Wait(1);
                }
                Off(OUT_B); Off(OUT_C);
            }

            operation::beep(); Wait(2000);
            RotateMotor(OUT_BC, -20, 80); // skip current intersection
            if (nodeColor[0][i] == 0) { // if black here
                operation::beep(); operation::beep(); Wait(2000);

                { // move to the next intersection
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    while (ReadEV3ColorSensorLight(IN_3, ReflectedLight) > 20) { // get to the next line
                        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                        double ret = pid.calculate(50, val) * 1;

                        OnFwdReg(OUT_B, -20-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -20+ret/2);  // the two motors
                        Wait(1);
                    }
                    Off(OUT_B); Off(OUT_C);
                }
                { // align the robot properly
                    RotateMotor(OUT_BC, -20, 0.5*unit);
                    RotateMotor(OUT_B, 20, arc90);
                    while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                        OnFwdReg(OUT_B, 15);
                        Wait(10);
                    }
                    Off(OUT_B);
                }
                { // collection
                    RotateMotor(OUT_BC, -20, 130); // move foward
                    operation::collectOne();
                    RotateMotor(OUT_BC, 12, 130);
                    OnFwdReg(OUT_D, -25);   // lift with higher power as it is heavier
                    //RotateMotor(OUT_BC, 20, 80); // go back on the line
                }
                { // return to reference point
                    RotateMotor(OUT_B, -20, arc90); // turn 
                    OnFwdReg(OUT_D, -75);
                    OnForDurationUnregulated(OUT_BC, 35, (i+1)*2000); // ram back against wall c_RAM_CABLE
                }
                break;
            }
        }
        operation::beep();
        { // MARK: travel to yellow-green cable deposit point
            RotateMotor(OUT_B, -20, arc90+10);

            operation::beep(); // Wait(2000);

            for (int i = 0; i < 5; ++i) { // go to wall
                { // go to next intersection
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                        double ret = pid.calculate(50, val) * 1;

                        OnFwdReg(OUT_B, -25-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -25+ret/2);  // the two motors

                        Wait(1);
                    }
                }
                if (i < 4) { // skip past current line
                    double Ku = 0.5, Tu = 0.78;
                    PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                    ResetRotationCount(OUT_B);
                    while (abs(MotorRotationCount(OUT_B)) < 120) {
                        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                        double ret = pid.calculate(50, val) * 1;

                        OnFwdReg(OUT_B, -25-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -25+ret/2);  // the two motors

                        Wait(1);
                    }
                    std::thread a([]() {
                            operation::beep();
                            });
                    a.detach();
                }
            }
            Off(OUT_BC);

            RotateMotor(OUT_C, -20, arc90); // rotate
            OnFwdReg(OUT_D, -75);
            OnForDurationUnregulated(OUT_BC, 30, 1500); // ram c_RAM_CABLE
            RotateMotor(OUT_BC, -20, 120); // move forward for PID

            operation::beep(); Wait(2000);

            { // MARK: go to 2nd next intersection and rotate
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 20, -20, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                for (int i = 0; i < 2; ++i) {
                    while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) { // reach line
                        int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                        double ret = pid.calculate(50, val) * 1;

                        OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                        OnFwdReg(OUT_C, -15+ret/2);  // the two motors

                        Wait(1);
                    }
                    if (i < 1) {
                        ResetRotationCount(OUT_B);
                        while (abs(MotorRotationCount(OUT_B)) < 150) {
                            int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                            double ret = pid.calculate(50, val) * 1;

                            OnFwdReg(OUT_B, -15-ret/2);  // use the ret val as difference between
                            OnFwdReg(OUT_C, -15+ret/2);  // the two motors

                            Wait(1);
                        }
                    }
                    std::thread a([]() {
                            operation::beep();
                            });
                    a.detach();
                }
                Off(OUT_BC);
            }
        }
        { // MARK: deposit cable
            // Rotate to position robot
            RotateMotor(OUT_C, -20, arc90);
            operation::rotateSync(-20, 20, arc90, arc90);

            operation::dropCable();
        }
        { // MARK: move to green
            OnFwdReg(OUT_BC, -20); // move away from the cable
            while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) Wait(1); // move to intersection
            Off(OUT_BC);
            OnFwdReg(OUT_D, -25); // lift claw
            operation::beep(); Wait(2000);

            RotateMotor(OUT_C, -20, arc90);  // rotate
            RotateMotor(OUT_BC, 20, 60);    // give PID more time to trace
            operation::beep(); Wait(2000);

            { // MARK: go to next intersection and rotate
                double Ku = 0.5, Tu = 0.78;
                PID pid = PID(0.001, 30, -30, 0.6*Ku, 1.2*Ku/Tu, 3.0*Ku*Tu/40.0);    // Classic
                while (ReadEV3ColorSensorLight(IN_2, ReflectedLight) > 20) {
                    int val = ReadEV3ColorSensorLight(IN_4, ReflectedLight);
                    double ret = pid.calculate(50, val) * 1;

                    OnFwdReg(OUT_B, -25-ret/2);  // use the ret val as difference between
                    OnFwdReg(OUT_C, -25+ret/2);  // the two motors

                    Wait(1);
                }
                Off(OUT_BC);
                RotateMotor(OUT_C, -20, arc90);
                std::thread a([]() {
                        operation::beep();
                        });
                a.detach();
            }
        }
        { // MARK: deposit green
            OnFwdReg(OUT_D, -25);

            std::thread setClawOrientationThread([]() {
                    operation::setClawOrientation(problem::orientation::EAST,
                            deviceOrientation[static_cast<int>(problem::color::GREEN)]);
                });
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

            setClawOrientationThread.join();
            operation::depositOne(problem::orientation::EAST,
                    deviceOrientation[static_cast<int>(problem::color::GREEN)],
                    problem::color::GREEN);
        }
    }

    void s06_goHome() {
        PlayToneEx(440, 10000, 100);    // signal go home
        RotateMotor(OUT_BC, 20, 120);   // move back a little
        RotateMotor(OUT_C, 20, arc90);  // turn to face north
        OnFwdReg(OUT_D, 20);            // slam the claw down to compress robot
        for (int elapsed = 0; elapsed < 1200; ++elapsed) { // ram against south wall
            // y=-0.000275x(x-1200)
            OnFwdReg(OUT_BC, std::min(100.0, -0.000275*elapsed*(elapsed-1200)));
            //OnFwdReg(OUT_BC, std::min(100.0, elapsed*0.1));
            Wait(1);
        }
        RotateMotor(OUT_C, -20, arc90-30);  // under turn to face northish-west
        OnForDuration(OUT_BC, 35, 3000); // ram back
    }
}

int main() {
    operation::monitorExit();
    // MARK debugging: hardcode data
    //{
    //    using namespace solution;
    //    deviceOrientation[static_cast<int>(problem::color::RED)] = problem::orientation::EAST;
    //    deviceOrientation[static_cast<int>(problem::color::BLUE)] = problem::orientation::WEST;
    //    deviceOrientation[static_cast<int>(problem::color::GREEN)] = problem::orientation::NORTH;
    //    deviceOrientation[static_cast<int>(problem::color::YELLOW)] = problem::orientation::SOUTH;
    //    nodeColor[0][0] = 0;
    //    nodeColor[0][1] = 1;
    //    nodeColor[0][2] = 0;
    //    nodeColor[1][0] = 0;
    //    nodeColor[1][1] = 1;
    //    nodeColor[1][2] = 0;

    //    OnFwdReg(OUT_D, -20); Wait(1000);
    //    operation::collectOne();
    //    exit(0);
    //}
    // MARK end debugging
    OnFwdReg(OUT_D, 20);
    ButtonWaitForPress(BTNCENTER);

    std::thread a([]() {
        solution::s00_init();
    });
    //PlayToneEx(440, 1000, 100);
    // MARK: scan blocks
    solution::s01_scanBlocks();
     
    // MARK: scan nodes
    solution::s02_scanNodes();

    //LcdClean(); LcdTextf(1,0,0,"%d %d %d | %d %d %d", solution::nodeColor[0][0],
    //                                                  solution::nodeColor[0][1],
    //                                                  solution::nodeColor[0][2],
    //                                                  solution::nodeColor[1][0],
    //                                                  solution::nodeColor[1][1],
    //                                                  solution::nodeColor[1][2]);
    //Wait(10000);

    a.join();
    // MARK: deposit 2 nodes to red blue
    solution::s03_depositSet1();
    
    // MARK: deposit 1 cable to red-blue and 1 node to green
    solution::s04_depositSet2();

    // MARK: depsit 1 cable to yellow-green and 1 node to yellow
    solution::s05_depositSet3();

    // MARK: end mission
    solution::s06_goHome();

    //PlayToneEx(440, 1000, 100);
    Wait(1000);
}
