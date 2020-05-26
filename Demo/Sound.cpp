#include "Sound.h"

Bat::ISoundEngine* snd;

void InitializeSound()
{
	snd = Bat::Audio::CreateSoundPlaybackDevice();
}
