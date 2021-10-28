# Better API Documentation
than the command.pdf provided.

I'll just provide some sample code here for your reference

## Template Code for any new program
```c
#include "ev3.h"

int main() {
    InitEV3();

    while (!ButtonIsDown(BTNEXIT)) {
        // your code here

        Wait(100);
    }
    FreeEV3();
}

```

## Sensors

### Light Sensor Reflected Light Example
```c
#include <ev3.h>

int main() {
    InitEV3();

    setAllSensorMode(COL_REFLECT, COL_REFLECT, COL_REFLECT, COL_REFLECT);
    for (int i = 0; i < 10; ++i) {
        LcdClean();
        LcdPrintf(1, "%d", readSensor(IN_1));
        Wait(1000);
    }

    FreeEV3();
}
```

## Motors

### Large Motor Example
```c

```

## LCD

### Printing Sensor Value Example
```c
#include <ev3.h>

int main() {
    InitEV3();

    setAllSensorMode(COL_REFLECT, COL_REFLECT, COL_REFLECT, COL_REFLECT);
    while (!ButtonIsDown(BTNEXIT)) {
        int val = readSensor(IN_1);
        LcdTextf(1, 0, 0, "sensor %d\n", val);

        Wait(1000);
    }

    FreeEV3();
}
```
