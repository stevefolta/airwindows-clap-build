#include "AudioEffectX.h"

#include <stdio.h>


void AudioEffectX::float2string(double value, char* dest, int size)
{
	snprintf(dest, size, "%g", value);
}


int AudioEffectX::getSampleRate()
{
	//*** TODO
	return 44100;
	/***/
}


void AudioEffectX::fill_descriptor(clap_plugin_descriptor_t* descriptor)
{
	//*** TODO
	/***/
}



