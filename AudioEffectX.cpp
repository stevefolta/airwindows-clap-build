#include "AudioEffectX.h"

#include <vector>
#include <stdio.h>
#include <math.h>


void AudioEffectX::float2string(double value, char* dest, int size)
{
	snprintf(dest, size, "%g", value);
}

void AudioEffectX::int2string(VstInt32 value, char* dest, int size)
{
	snprintf(dest, size, "%d", value);
}

void AudioEffectX::dB2string(double value, char* dest, int size)
{
	// Is this right?  Why can't Wikipedia tell us what a digital dB is?
	snprintf(dest, size, "%g", 20 * log10(value));
}



static const char* const default_features[] = {
	CLAP_PLUGIN_FEATURE_AUDIO_EFFECT,
	nullptr,
	};

void AudioEffectX::fill_descriptor(clap_plugin_descriptor_t* descriptor, std::set<std::string>* strings)
{
	auto intern_string = [&](std::string str) {
		return strings->insert(str).first->c_str();
		};

	descriptor->clap_version = CLAP_VERSION_INIT;

	char name[kVstMaxProductStrLen];
	getEffectName(name);
	descriptor->name = intern_string(name);
	descriptor->id = intern_string("com.airwindows." + std::string(name));
	if (getVendorString(name))
		descriptor->vendor = intern_string(name);
	//*** TODO: URL.  Requires converting to lowercase.
	//*** TODO: better features.
	descriptor->features = default_features;
}


bool AudioEffectX::activate(double sample_rate_in, uint32_t min_frames, uint32_t max_frames)
{
	sample_rate = sample_rate_in;
	return true;
}

void AudioEffectX::deactivate()
{
}


bool AudioEffectX::start_processing()
{
	return true;
}

void AudioEffectX::stop_processing()
{
}


void AudioEffectX::reset()
{
	/***/
}


clap_process_status AudioEffectX::process(const clap_process_t* process)
{
	const uint32_t num_frames = process->frames_count;
	const uint32_t num_events = process->in_events->size(process->in_events);
	uint32_t cur_event = 0;
	uint32_t next_event_frame = num_events > 0 ? 0 : num_frames;

	// Process audio and handle events.
	for (uint32_t cur_frame = 0; cur_frame < num_frames; ) {
		// Handle events at this frame (and/or update next_event_frame).
		while (cur_event < num_events && next_event_frame <= cur_frame) {
			const clap_event_header_t* event = process->in_events->get(process->in_events, cur_event);
			if (event->time > cur_frame) {
				next_event_frame = event->time;
				break;
				}

			process_event(event);

			cur_event += 1;
			if (cur_event == num_events) {
				next_event_frame = num_frames;
				break;
				}
			}

		// Process.
		if (process->audio_inputs[0].data64 && process->audio_outputs[0].data64)
			processDoubleReplacing(process->audio_inputs[0].data64, process->audio_outputs[0].data64, next_event_frame - cur_frame);
		else
			processReplacing(process->audio_inputs[0].data32, process->audio_outputs[0].data32, next_event_frame - cur_frame);
		cur_frame = next_event_frame;
		}

	return CLAP_PROCESS_CONTINUE;
}


void AudioEffectX::process_event(const clap_event_header_t* event)
{
	if (event->space_id != CLAP_CORE_EVENT_SPACE_ID)
		return;

	if (event->type == CLAP_EVENT_PARAM_VALUE) {
		auto param_event = (const clap_event_param_value_t*) event;
		setParameter(param_event->param_id, param_event->value);
		}
}


static const clap_plugin_audio_ports_t audio_ports_extension = {
	.count = [](const clap_plugin_t* clap_plugin, bool is_input) -> uint32_t {
		return AudioEffectX::of(clap_plugin)->num_audio_ports(is_input);
		},
	.get = [](const clap_plugin_t* clap_plugin, uint32_t index, bool is_input, clap_audio_port_info_t* info_out) -> bool {
		return AudioEffectX::of(clap_plugin)->get_audio_port_info(index, is_input, info_out);
		}
	};

static const std::vector<clap_audio_port_info_t> audio_in_ports = {
	{
		.id = 0,
		.name = "in",
		.flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS,
		.channel_count = 2,
		.port_type = CLAP_PORT_STEREO,
		.in_place_pair = 0,
		},
	};
static const std::vector<clap_audio_port_info_t> audio_out_ports = {
	{
		.id = 0,
		.name = "out",
		.flags = CLAP_AUDIO_PORT_IS_MAIN | CLAP_AUDIO_PORT_SUPPORTS_64BITS,
		.channel_count = 2,
		.port_type = CLAP_PORT_STEREO,
		.in_place_pair = 0,
		},
	};

uint32_t AudioEffectX::num_audio_ports(bool is_input)
{
	return (is_input ? audio_in_ports.size() : audio_out_ports.size());
}

bool AudioEffectX::get_audio_port_info(uint32_t index, bool is_input, clap_audio_port_info_t* info_out)
{
	if (is_input && index < audio_in_ports.size()) {
		*info_out = audio_in_ports[index];
		return true;
		}
	else if (!is_input && index < audio_out_ports.size()) {
		*info_out = audio_out_ports[index];
		return true;
		}
	return false;
}

static const clap_plugin_params_t params_extension = {
	.count = [](const clap_plugin_t* clap_plugin) -> uint32_t {
		return AudioEffectX::of(clap_plugin)->num_parameters;
		},
	.get_info = [](const clap_plugin_t* clap_plugin, uint32_t param_index, clap_param_info_t* param_info_out) -> bool {
		return AudioEffectX::of(clap_plugin)->get_param_info(param_index, param_info_out);
		},
	.get_value = [](const clap_plugin_t* clap_plugin, clap_id param_id, double* value_out) -> bool {
		*value_out = AudioEffectX::of(clap_plugin)->getParameter(param_id);
		return true;
		},
	.value_to_text = [](const clap_plugin_t* clap_plugin, clap_id param_id, double value, char* out, uint32_t capacity) -> bool {
		// Not quite right; getParameterDisplay() will get the plugin's current
		// value, not "value".
		AudioEffectX::of(clap_plugin)->getParameterDisplay(param_id, out);
		return true;
		},
	.text_to_value = [](const clap_plugin_t* clap_plugin, clap_id param_id, const char* value_text, double* value_out) -> bool {
		char* end_ptr = nullptr;
		*value_out = strtod(value_text, &end_ptr);
		return end_ptr != value_text;
		},
	.flush = [](const clap_plugin_t* clap_plugin, const clap_input_events_t* in, const clap_output_events_t* out) {
		AudioEffectX::of(clap_plugin)->flush_parameters(in, out);
		}
	};

void AudioEffectX::flush_parameters(const clap_input_events_t* in, const clap_output_events_t* out)
{
	auto num_events = in->size(in);
	for (int i = 0; i < num_events; ++i)
		process_event(in->get(in, i));
}


bool AudioEffectX::get_param_info(uint32_t param_index, clap_param_info_t* param_info_out)
{
	param_info_out->id = param_index;
	param_info_out->flags = CLAP_PARAM_IS_AUTOMATABLE | CLAP_PARAM_REQUIRES_PROCESS;
	param_info_out->cookie = nullptr;
	getParameterName(param_index, param_info_out->name);
	param_info_out->module[0] = 0;
	// How should we get these?  Does Airwindows ever use anything other than 0.0 - 1.0?
	param_info_out->min_value = 0.0;
	param_info_out->max_value = 1.0;
	param_info_out->default_value = 0.5;
	return true;
}


static const clap_plugin_state_t state_extension = {
	.save = [](const clap_plugin_t* clap_plugin, const clap_ostream_t* stream) -> bool {
		return AudioEffectX::of(clap_plugin)->save_state(stream);
		},
	.load = [](const clap_plugin_t* clap_plugin, const clap_istream_t* stream) -> bool {
		return AudioEffectX::of(clap_plugin)->load_state(stream);
		},
	};

bool AudioEffectX::save_state(const clap_ostream_t* stream)
{
	void* chunk = nullptr;
	auto num_bytes = getChunk(&chunk, false);

	const char* bytes = (char*) chunk;
	int64_t bytes_left = num_bytes;
	while (bytes_left > 0) {
		auto bytes_written = stream->write(stream, bytes, bytes_left);
		if (bytes_written < 0)
			break;
		bytes += bytes_written;
		bytes_left -= bytes_written;
		}

	free(chunk);
	return bytes_left == 0;
}

bool AudioEffectX::load_state(const clap_istream_t* stream)
{
	// Read the whole chunk.
	std::vector<char> buffer(256);
	std::vector<char> chunk;
	while (true) {
		auto bytes_read = stream->read(stream, buffer.data(), buffer.size());
		if (bytes_read == 0)
			break;
		else if (bytes_read < 0)
			return false;
		chunk.insert(chunk.end(), buffer.begin(), std::next(buffer.begin(), bytes_read));
		}

	// Airwindows plugs don't check the size of the data (they don't trust it).
	// The validator requires that a size of zero return false anyway.
	if (chunk.size() == 0)
		return false;
	setChunk(chunk.data(), chunk.size(), false);
	return true;
}


const void* AudioEffectX::get_extension(const char* id)
{
	if (strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0)
		return &audio_ports_extension;
	else if (strcmp(id, CLAP_EXT_PARAMS) == 0)
		return &params_extension;
	else if (strcmp(id, CLAP_EXT_STATE) == 0)
		return &state_extension;
	return nullptr;
}


void AudioEffectX::main_thread_tick()
{
	/***/
}



