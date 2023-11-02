#include "AirwindowsCLAP.h"
#include "AudioEffectX.h"
#include "clap/clap.h"
#include <stdlib.h>
#include <iostream>


static std::set<std::string> interned_strings;

clap_plugin_descriptor_t* PluginEntry::get_descriptor()
{
	if (descriptor == nullptr) {
		auto plugin = create();
		descriptor = (clap_plugin_descriptor_t*) malloc(sizeof(clap_plugin_descriptor_t));
		memset(descriptor, 0, sizeof(clap_plugin_descriptor_t));
		plugin->fill_descriptor(descriptor, &interned_strings);
		delete plugin;
		}
	return descriptor;
}


clap_plugin_t clap_plugin_template = {
	.init = [](const clap_plugin_t* clap_plugin) -> bool { return AudioEffectX::of(clap_plugin)->init(); },
	.destroy = [](const clap_plugin_t* clap_plugin) { delete AudioEffectX::of(clap_plugin); },
	.activate = [](const clap_plugin_t* clap_plugin, double sample_rate, uint32_t min_frames, uint32_t max_frames) -> bool {
		return AudioEffectX::of(clap_plugin)->activate(sample_rate, min_frames, max_frames);
		},
	.deactivate = [](const clap_plugin_t* clap_plugin) {
		AudioEffectX::of(clap_plugin)->deactivate();
		},
	.start_processing = [](const clap_plugin_t* clap_plugin) -> bool {
		return AudioEffectX::of(clap_plugin)->start_processing();
		},
	.stop_processing = [](const clap_plugin_t* clap_plugin) {
		AudioEffectX::of(clap_plugin)->stop_processing();
		},
	.reset = [](const clap_plugin_t* clap_plugin) {
		AudioEffectX::of(clap_plugin)->reset();
		},
	.process = [](const clap_plugin* clap_plugin, const clap_process_t* process) -> clap_process_status {
		return AudioEffectX::of(clap_plugin)->process(process);
		},
	.get_extension = [](const clap_plugin* clap_plugin, const char* id) -> const void* {
		return AudioEffectX::of(clap_plugin)->get_extension(id);
		},
	.on_main_thread = [](const clap_plugin* clap_plugin) {
		AudioEffectX::of(clap_plugin)->main_thread_tick();
		},
	};

const clap_plugin_t* create_plugin(const clap_plugin_factory_t* factory, const clap_host_t* host, const char* plugin_id)
{
	for (auto& entry: plugins) {
		auto descriptor = entry.get_descriptor();
		if (strcmp(plugin_id, descriptor->id) == 0) {
			auto plugin = entry.create();
			plugin->clap_plugin = clap_plugin_template;
			plugin->clap_plugin.desc = descriptor;
			plugin->clap_plugin.plugin_data = plugin;
			return &plugin->clap_plugin;
			}
		}
	return nullptr;
}

static const clap_plugin_factory_t factory = {
	.get_plugin_count = [](const clap_plugin_factory_t* factory) -> uint32_t {
		return plugins.size();
		},
	.get_plugin_descriptor = [](const clap_plugin_factory_t* factory, uint32_t index) -> const clap_plugin_descriptor_t* {
		if (index >= plugins.size())
			return nullptr;
		return plugins[index].get_descriptor();
		},
	.create_plugin = create_plugin,
	};

bool init(const char* plugin_path)
{
	return true;
}

void deinit()
{
	for (auto& plugin: plugins) {
		free(plugin.descriptor);
		plugin.descriptor = nullptr;
		}
	interned_strings.clear();
}

extern "C" const clap_plugin_entry_t clap_entry = {
	.clap_version = CLAP_VERSION_INIT,
	.init = init,
	.deinit = deinit,
	.get_factory = [](const char* factory_id) -> const void* {
		if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0)
			return &factory;
		return nullptr;
		},
	};

#ifdef DEBUG_UNLOADING
extern "C" __attribute__((destructor)) void at_unload()
{
  std::cerr << "- Unloading." << std::endl << std::endl;
}
#endif

