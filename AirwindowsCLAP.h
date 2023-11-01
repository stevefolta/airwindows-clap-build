#pragma once

#include <vector>
#include "clap/clap.h"

class AudioEffectX;

struct PluginEntry {
	clap_plugin_descriptor_t* descriptor = nullptr;
	AudioEffectX* (*create)() = nullptr;

	clap_plugin_descriptor_t* get_descriptor();
	};


extern std::vector<PluginEntry> plugins;

