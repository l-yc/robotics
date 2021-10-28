void rampUpMotors(double maxPower, double acceleration) {
    double power = 0.0;
    for (int t = 0; power < maxPower; ++t) {
        power += acceleration;
        OnFwdReg(OUT_BC, power);
    }
}

