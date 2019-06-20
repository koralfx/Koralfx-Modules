#include "Koralfx-Modules.hpp"

// The pluginInstance-wide instance of the Plugin class
Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;
	
    p->addModel(modelBeatovnik);
    p->addModel(modelMixovnik);
    p->addModel(modelNullovnik4);
    p->addModel(modelNullovnik6);
    p->addModel(modelPresetovnik);
    p->addModel(modelQuantovnik);
	p->addModel(modelScorovnik);

}
