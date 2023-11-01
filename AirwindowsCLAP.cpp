#include "AirwindowsCLAP.h"
#include "AudioEffectX.h"
#include "clap/clap.h"
#include <stdlib.h>


clap_plugin_descriptor_t* PluginEntry::get_descriptor()
{
	if (!descriptor) {
		auto plugin = create();
		descriptor = (clap_plugin_descriptor_t*) malloc(sizeof(clap_plugin_descriptor_t));
		plugin->fill_descriptor(descriptor);
		delete plugin;
		}
	return descriptor;
}


#define PLUGIN(clap_plugin) ((AudioEffectX*) clap_plugin->plugin_data)
clap_plugin_t clap_plugin_template = {
	.init = [](const clap_plugin_t* plugin) -> bool { return true; },
	.destroy = [](const clap_plugin_t* clap_plugin) { delete PLUGIN(clap_plugin); },
	};

const clap_plugin_t* create_plugin(const clap_plugin_factory_t* factory, const clap_host_t* host, const char* plugin_id)
{
	for (auto& entry: plugins) {
		auto descriptor = entry.get_descriptor();
		if (strcmp(plugin_id, descriptor->id)) {
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

