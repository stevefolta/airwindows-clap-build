#pragma once

#include "clap/clap.h"
#include <set>
#include <string>
#include <vector>
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
		AudioEffectX(audioMasterCallback audio_master, int num_programs, int num_parameters_in);
		virtual bool init();
		virtual ~AudioEffectX() {}

		void setNumInputs(int new_num_inputs) { num_inputs = new_num_inputs; }
		void setNumOutputs(int new_num_outputs) { num_outputs = new_num_outputs; }
		void setUniqueID(int id) {}
		void canProcessReplacing() { can_process_replacing = true; }
		void canDoubleReplacing() { can_double_replacing = true; }
		void programsAreChunks(bool) {}
		double getSampleRate() { return sample_rate; }

		virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) = 0;
		virtual void processDoubleReplacing(double** inputs, double** outputs, VstInt32 sampleFrames) = 0;
		virtual VstInt32 getChunk(void** data, bool isPreset) { return 0; }
		virtual VstInt32 setChunk(void* data, VstInt32 byteSize, bool isPreset) { return 0; }
		virtual void setParameter(VstInt32 index, float value) {}
		virtual float getParameter(VstInt32 index) { return 0.0; }
		virtual void getParameterName(VstInt32 index, char* text) { text[0] = 0; }
		virtual void getParameterDisplay(VstInt32 index, char* text) { text[0] = 0; }
		virtual void getParameterLabel(VstInt32 index, char* text) { text[0] = 0; }
		virtual VstInt32 canDo(char* text) { return 0; }
		virtual bool getEffectName(char* name) = 0;
		virtual VstPlugCategory getPlugCategory() { return kPlugCategEffect; }
		virtual bool getProductString(char* text) { return false; }
		virtual bool getVendorString(char* text) { return false; }

		void fill_descriptor(clap_plugin_descriptor_t* descriptor, std::set<std::string>* strings);
		bool activate(double sample_rate, uint32_t min_frames, uint32_t max_frames);
		void deactivate();
		bool start_processing();
		void stop_processing();
		void reset();
		clap_process_status process(const clap_process_t* process);
		const void* get_extension(const char* id);
		void main_thread_tick();

		virtual uint32_t num_audio_ports(bool is_input);
		virtual bool get_audio_port_info(uint32_t index, bool is_input, clap_audio_port_info_t* info_out);
		virtual bool get_param_info(uint32_t param_index, clap_param_info_t* param_info_out);
		virtual void flush_parameters(const clap_input_events_t* in, const clap_output_events_t* out);
		virtual bool save_state(const clap_ostream_t* stream);
		virtual bool load_state(const clap_istream_t* stream);

		clap_plugin_t clap_plugin;

		static AudioEffectX* of(const clap_plugin_t* clap_plugin) {
			return (AudioEffectX*) clap_plugin->plugin_data;
			}

		int num_parameters = 0;

	protected:
		int num_inputs = 0, num_outputs = 0;
		bool can_process_replacing = false, can_double_replacing = false;
		double sample_rate = 44100;
		std::vector<double> param_values;
		std::vector<double> param_mods;

		static void float2string(double value, char* dest, int size);
		static void int2string(VstInt32 value, char* dest, int size);
		static void dB2string(double value, char* dest, int size);

		void process_event(const clap_event_header_t* event);
	};

