#include "Plateovnik4.hpp"

Plateovnik4::Plateovnik4() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Plateovnik4::step() {
}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *Plateovnik4::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    return rootJ;
}

void Plateovnik4::fromJson(json_t *rootJ) {
	json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
	panelStyle = json_integer_value(j_panelStyle);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Plateovnik4Widget::Plateovnik4Widget(Plateovnik4 *module) : ModuleWidget(module){

	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelWidget *panel = new DynamicPanelWidget();
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Plateovnik4-Dark.svg")));
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Plateovnik4-Light.svg")));
		box.size = panel->box.size;
		panel->mode = &module->panelStyle;
		addChild(panel);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Context Menu ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct Plateovnik4PanelStyleItem : MenuItem {
    Plateovnik4* module;
    int panelStyle;
    void onAction(EventAction &e) override {
        module->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (module->panelStyle == panelStyle) ? "✔" : "";
        MenuItem::step();
    }
};

void Plateovnik4Widget::appendContextMenu(Menu *menu) {
    Plateovnik4 *module = dynamic_cast<Plateovnik4*>(this->module);
    assert(module);

    // Panel style
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Frame of mind"));
    menu->addChild(construct<Plateovnik4PanelStyleItem>(&MenuItem::text, "Dark Calm Night",
    	&Plateovnik4PanelStyleItem::module, module, &Plateovnik4PanelStyleItem::panelStyle, 0));
    menu->addChild(construct<Plateovnik4PanelStyleItem>(&MenuItem::text, "Happy Bright Day",
    	&Plateovnik4PanelStyleItem::module, module, &Plateovnik4PanelStyleItem::panelStyle, 1));

}

////////////////////////////////////////////////////////////////////////////////////////////////////

Model *modelPlateovnik4 = Model::create<Plateovnik4, Plateovnik4Widget>("KoralfxVCV", "Plateovnik4", "Plateovnik 4",
	UTILITY_TAG);
