#include <ev3.h>
//#include <ev3_sound.h>

#define LMS_SOUND_DEVICE          "lms_sound"           /*!< SOUND device name */
#define LMS_SOUND_DEVICE_NAME     "/dev/lms_sound"      /*!< SOUND device */

// sound commands
#define SOUND_CMD_BREAK   0
#define SOUND_CMD_TONE    1
#define SOUND_CMD_PLAY    2
#define SOUND_CMD_REPEAT  3
#define SOUND_CMD_SERVICE 4

int WriteToSoundDevice(uint8_t * bytes, int num_bytes)
{
	ssize_t result = -1;
	int sndHandle = open(LMS_SOUND_DEVICE_NAME, O_WRONLY);
	if (sndHandle >= 0)
	{
		// for some reason write is not returning num_bytes -
		// it usually returns zero
		result = write(sndHandle, bytes, num_bytes);
//printf("result = %zd\n", result);
		close(sndHandle);
//    if (result >= 0)
//      return num_bytes;
	}
	return (int)result;
}

void PlayToneEx2(unsigned short frequency, unsigned short duration, uint8_t volume)
{
	//if (!SoundInitialized())
	//	return;

	//// sound system must not be muted to play a sound
	//if (SoundInstance.SoundMuted != 0)
	//	return;

	//(*SoundInstance.pSound).Busy = true;
	uint8_t SoundData[6];
	SoundData[0] = SOUND_CMD_TONE;
	SoundData[1] = (uint8_t)((volume*13)/100);
	SoundData[2] = (uint8_t)(frequency);
	SoundData[3] = (uint8_t)(frequency >> 8);
	SoundData[4] = (uint8_t)(duration);
	SoundData[5] = (uint8_t)(duration >> 8);
	//SoundInstance.SoundState = SOUND_STATE_TONE;
	WriteToSoundDevice(SoundData, sizeof(SoundData)); // write 6 bytes
}

int main() {
    InitEV3();

    PlayToneEx(440, 1000, 100);
    //PlayTone(440, 1000);
    LcdPrintf(0, "Hello World!");
    Wait(5000);

    FreeEV3();
}
