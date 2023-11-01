#pragma once

#include "clap/clap.h"
#include <string.h>
#include <stdint.h>


typedef void* audioMasterCallback;
typedef int VstPlugCategory;
typedef int32_t VstInt32;
typedef class AudioEffectX AudioEffect;

#define vst_strncpy strncpy

enum {
	kPlugCategEffect = 0,
	kVstMaxProgNameLen = 64,
	kVstMaxParamStrLen = 64,
	kVstMaxProductStrLen = 64,
	kVstMaxVendorStrLen = 64,
	};


class AudioEffectX {
	public:
		AudioEffectX(audioMasterCallback audio_master, int num_programs, int num_parameters);

		void setNumInputs(int new_num_inputs) { num_inputs = new_num_inputs; }
		void setNumOutputs(int new_num_outputs) { num_outputs = new_num_outputs; }
		void setUniqueID(int id) {}
		void canProcessReplacing() { can_process_replacing = true; }
		void canDoubleReplacing() { can_double_replacing = true; }
		void programsAreChunks(bool) {}
		int getSampleRate();

		void fill_descriptor(clap_plugin_descriptor_t* descriptor);

		clap_plugin_t clap_plugin;

	protected:
		int num_inputs = 0, num_outputs = 0;
		bool can_process_replacing = false, can_double_replacing = false;

		static void float2string(double value, char* dest, int size);
	};

