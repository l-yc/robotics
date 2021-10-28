#include <ev3.h>
#include <cassert>
#include <algorithm>
using namespace std;

//int SetSensorMode(int sensorPort, int name)
//{
//	static DEVCON devCon;
//
//	if (!g_analogSensors)
//		InitSensors();
//
//	if (sensorPort < 0 || sensorPort >= INPUTS)
//		return -1;
//
//	sensor_setup_NAME[sensorPort] = name;
//	// Setup of Input
//    devCon.Connection[sensorPort] 	= CONN_INPUT_UART;
//    devCon.Type[sensorPort] 		= COL_TYPE;
//    devCon.Mode[sensorPort] 		= COL_COLOR_RGB_MODE;
//
//	return 0;
//}

int main() {
    InitEV3();

    //SetSensorMode(IN_2, 2);
    //SetSensorMode(IN_2, COL_REFLECT);
    SetAllSensorMode(COL_COLOR_RGB, COL_COLOR_RGB, COL_COLOR_RGB, COL_COLOR_RGB);
    //SetAllSensorMode(COL_REFLECT, COL_REFLECT, COL_REFLECT, COL_REFLECT);
    //PlayToneEx(440, 1000, 100);
    int idx = 0;
    while (!ButtonIsDown(BTNEXIT)) {
        //int val = ReadSensor(IN_2);
        //int r = (val & 0x00FF0000) >> 16;
        //int g = (val & 0x0000FF00) >> 8;
        //int b = (val & 0x000000FF);
        int r = ReadSensorRed(IN_1);
        int g = ReadSensorGreen(IN_1);
        int b = ReadSensorBlue(IN_1);
        //assert(r >= 0 and r < 256);
        //assert(g >= 0 and g < 256);
        //assert(b >= 0 and b < 256);
        LcdTextf(1, 0, 50, "%03d %03d %03d", r, g, b);
        
        
        //int sensorPort = IN_2;
        //uint64_t* data = (uint64_t*) ReadSensorData(sensorPort);
        //if (!data) LcdTextf(1, 0, 50, "oof");
        //else {
        //    int32_t temp = 0;
        //    /**
        //         * The first 6 bytes in data are the colors: 2 byte for each color.
        //         * The range of each color value is from 0 to 1023. We convert those
        //         * values in 3 bytes (0-255), to be able to return it as a int
        //         */
        //    temp = 0;
        //    int r = (int) (*data) & 0xFFFF;
        //    int g = (int) ((*data) >> 16) & 0xFFFF;
        //    int b = (int) ((*data) >> 32) & 0xFFFF;
        //    LcdTextf(1, 0, 50, "%03d %03d %03d", r, g, b);
        //}

        Wait(100);
        LcdClean();
    }
    //LcdPrintf(0, "Hello World!");
    //Wait(5000);

    FreeEV3();
}
