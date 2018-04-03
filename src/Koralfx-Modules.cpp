#include "Koralfx-Modules.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// The "slug" is the unique identifier for your plugin and must never change after release, so choose wisely.
	// It must only contain letters, numbers, and characters "-" and "_". No spaces.
	// To guarantee uniqueness, it is a good idea to prefix the slug by your name, alias, or company name if available, e.g. "MyCompany-MyPlugin".
	// The ZIP package must only contain one folder, with the name equal to the plugin's slug.

	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/koralfx/Koralfx";
	p->manual = "https://github.com/koralfx/Koralfx/blob/master/README.md";




	p->addModel(modelQuantovnik);
    p->addModel(modelMixovnik);
    p->addModel(modelBeatovnik);
    p->addModel(modelScorovnik);
    p->addModel(modelNullovnik4);
    p->addModel(modelNullovnik6);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
