#include "Koralfx-Modules.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/koralfx/Koralfx-Modules";
	p->manual = "https://github.com/koralfx/Koralfx-Modules/blob/master/README.md";
	
    p->addModel(modelBeatovnik);
    p->addModel(modelMixovnik);
    p->addModel(modelNullovnik4);
    p->addModel(modelNullovnik6);
    p->addModel(modelPresetovnik);
    p->addModel(modelQuantovnik);
	p->addModel(modelScorovnik);

}
